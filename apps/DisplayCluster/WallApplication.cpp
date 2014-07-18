/*********************************************************************/
/* Copyright (c) 2014, EPFL/Blue Brain Project                       */
/*                     Raphael Dumusc <raphael.dumusc@epfl.ch>       */
/* All rights reserved.                                              */
/*                                                                   */
/* Redistribution and use in source and binary forms, with or        */
/* without modification, are permitted provided that the following   */
/* conditions are met:                                               */
/*                                                                   */
/*   1. Redistributions of source code must retain the above         */
/*      copyright notice, this list of conditions and the following  */
/*      disclaimer.                                                  */
/*                                                                   */
/*   2. Redistributions in binary form must reproduce the above      */
/*      copyright notice, this list of conditions and the following  */
/*      disclaimer in the documentation and/or other materials       */
/*      provided with the distribution.                              */
/*                                                                   */
/*    THIS  SOFTWARE IS PROVIDED  BY THE  UNIVERSITY OF  TEXAS AT    */
/*    AUSTIN  ``AS IS''  AND ANY  EXPRESS OR  IMPLIED WARRANTIES,    */
/*    INCLUDING, BUT  NOT LIMITED  TO, THE IMPLIED  WARRANTIES OF    */
/*    MERCHANTABILITY  AND FITNESS FOR  A PARTICULAR  PURPOSE ARE    */
/*    DISCLAIMED.  IN  NO EVENT SHALL THE UNIVERSITY  OF TEXAS AT    */
/*    AUSTIN OR CONTRIBUTORS BE  LIABLE FOR ANY DIRECT, INDIRECT,    */
/*    INCIDENTAL,  SPECIAL, EXEMPLARY,  OR  CONSEQUENTIAL DAMAGES    */
/*    (INCLUDING, BUT  NOT LIMITED TO,  PROCUREMENT OF SUBSTITUTE    */
/*    GOODS  OR  SERVICES; LOSS  OF  USE,  DATA,  OR PROFITS;  OR    */
/*    BUSINESS INTERRUPTION) HOWEVER CAUSED  AND ON ANY THEORY OF    */
/*    LIABILITY, WHETHER  IN CONTRACT, STRICT  LIABILITY, OR TORT    */
/*    (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY WAY OUT    */
/*    OF  THE  USE OF  THIS  SOFTWARE,  EVEN  IF ADVISED  OF  THE    */
/*    POSSIBILITY OF SUCH DAMAGE.                                    */
/*                                                                   */
/* The views and conclusions contained in the software and           */
/* documentation are those of the authors and should not be          */
/* interpreted as representing official policies, either expressed   */
/* or implied, of The University of Texas at Austin.                 */
/*********************************************************************/

#include "WallApplication.h"

#include "MPIChannel.h"
#include "WallToWallChannel.h"
#include "configuration/WallConfiguration.h"
#include "Options.h"
#include "RenderContext.h"
#include "Factories.h"
#include "PixelStreamFrame.h"
#include "GLWindow.h"
#include "TestPattern.h"
#include "DisplayGroupManager.h"
#include "ContentWindowManager.h"
#include "DisplayGroupRenderer.h"
#include "MarkerRenderer.h"

#include <boost/foreach.hpp>
#include <boost/make_shared.hpp>

WallApplication::WallApplication(int& argc_, char** argv_, MPIChannelPtr mpiChannel)
    : Application(argc_, argv_)
    , wallToMasterChannel_(mpiChannel)
    , wallToWallChannel_(new WallToWallChannel(mpiChannel))
{
    WallConfiguration* config = new WallConfiguration(getConfigFilename(),
                                                      mpiChannel->getRank());
    g_configuration = config;

    initRenderContext(config);
    setupTestPattern(config, mpiChannel->getRank());
    initMPIConnection();

    wallToMasterChannel_->setFactories(factories_);

    // setup connection so renderFrame() will be called continuously.
    // Must be a queued connection to avoid infinite recursion.
    connect(this, SIGNAL(frameFinished()),
            this, SLOT(renderFrame()), Qt::QueuedConnection);
    renderFrame();
}

WallApplication::~WallApplication()
{
    // Must be done before destructing the GLWindows to release GL objects
    factories_->clear();
}

void WallApplication::initRenderContext(const WallConfiguration* config)
{
    renderContext_.reset(new RenderContext(config));
    factories_.reset(new Factories(*renderContext_));

    DisplayGroupRendererPtr displayGroupRenderer(new DisplayGroupRenderer(factories_));
    MarkerRendererPtr markerRenderer(new MarkerRenderer());

    connect(wallToMasterChannel_.get(), SIGNAL(received(MarkersPtr)),
            markerRenderer.get(), SLOT(setMarkers(MarkersPtr)));

    connect(wallToMasterChannel_.get(), SIGNAL(received(DisplayGroupManagerPtr)),
            displayGroupRenderer.get(), SLOT(setDisplayGroup(DisplayGroupManagerPtr)));

    renderContext_->addRenderable(displayGroupRenderer);
    renderContext_->addRenderable(markerRenderer);
}

void WallApplication::setupTestPattern(const WallConfiguration* config, const int rank)
{
    for (size_t i = 0; i < renderContext_->getGLWindowCount(); ++i)
    {
        GLWindowPtr glWindow = renderContext_->getGLWindow(i);
        RenderablePtr testPattern(new TestPattern(glWindow.get(),
                                                  config,
                                                  rank,
                                                  glWindow->getTileIndex()));
        glWindow->setTestPattern(testPattern);
    }
}

void WallApplication::initMPIConnection()
{
    connect(wallToMasterChannel_.get(), SIGNAL(received(DisplayGroupManagerPtr)),
            this, SLOT(updateDisplayGroup(DisplayGroupManagerPtr)));

    connect(wallToMasterChannel_.get(), SIGNAL(received(OptionsPtr)),
            this, SLOT(updateOptions(OptionsPtr)));

    connect(wallToMasterChannel_.get(), SIGNAL(received(PixelStreamFramePtr)),
            this, SLOT(processPixelStreamFrame(PixelStreamFramePtr)));

    connect(wallToMasterChannel_.get(), SIGNAL(receivedQuit()),
            this, SLOT(quit()));
}

void WallApplication::renderFrame()
{
    wallToMasterChannel_->receiveMessages(); // TODO make this an async task

    preRenderUpdate();

    renderContext_->updateGLWindows();
    wallToWallChannel_->globalBarrier();
    renderContext_->swapBuffers();

    postRenderUpdate();

    emit(frameFinished());
}

void WallApplication::preRenderUpdate()
{
    // synchronize clock right after receiving messages to ensure we have an
    // accurate time for rendering, etc. below
    wallToWallChannel_->synchronizeClock();

    ContentWindowManagerPtrs contentWindows = displayGroup_->getContentWindowManagers();

    BOOST_FOREACH(ContentWindowManagerPtr contentWindow, contentWindows)
    {
        // note that if we have multiple ContentWindowManagers corresponding to a single Content object,
        // we will call advance() multiple times per frame on that Content object...
        contentWindow->getContent()->preRenderUpdate(factories_, contentWindow, wallToWallChannel_);
    }
    ContentWindowManagerPtr backgroundWindow = displayGroup_->getBackgroundContentWindow();
    if (backgroundWindow)
        backgroundWindow->getContent()->preRenderUpdate(factories_, backgroundWindow, wallToWallChannel_);
}

void WallApplication::postRenderUpdate()
{
    ContentWindowManagerPtrs contentWindows = displayGroup_->getContentWindowManagers();

    BOOST_FOREACH(ContentWindowManagerPtr contentWindow, contentWindows)
    {
        // note that if we have multiple ContentWindowManagers corresponding to a single Content object,
        // we will call advance() multiple times per frame on that Content object...
        contentWindow->getContent()->postRenderUpdate(factories_, contentWindow, wallToWallChannel_);
    }
    ContentWindowManagerPtr backgroundWindow = displayGroup_->getBackgroundContentWindow();
    if (backgroundWindow)
        backgroundWindow->getContent()->postRenderUpdate(factories_, backgroundWindow, wallToWallChannel_);

    factories_->clearStaleFactoryObjects();
}

void WallApplication::updateDisplayGroup(DisplayGroupManagerPtr displayGroup)
{
    displayGroup_ = displayGroup;
}

void WallApplication::updateOptions(OptionsPtr options)
{
    g_configuration->setOptions(options);
}

void WallApplication::processPixelStreamFrame(PixelStreamFramePtr frame)
{
    Factory<PixelStream>& pixelStreamFactory = factories_->getPixelStreamFactory();
    pixelStreamFactory.getObject(frame->uri)->insertNewFrame(frame->segments);
}
