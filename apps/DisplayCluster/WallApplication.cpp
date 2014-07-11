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

WallApplication::WallApplication(int& argc_, char** argv_, MPIChannelPtr mpiChannel)
    : Application(argc_, argv_, mpiChannel)
{
    WallConfiguration* config = new WallConfiguration(getConfigFilename(),
                                                      mpiChannel_->getRank());
    g_configuration = config;

    renderContext_.reset(new RenderContext(config));

    factories_.reset(new Factories(*renderContext_));
    displayGroupRenderer_.reset(new DisplayGroupRenderer(factories_));

    MarkerRendererPtr markerRenderer(new MarkerRenderer);

    if (mpiChannel_->getRank() == 1)
        mpiChannel_->setFactories(factories_);

    for (size_t i = 0; i < renderContext_->getGLWindowCount(); ++i)
    {
        GLWindowPtr glWindow = renderContext_->getGLWindow(i);
        glWindow->addRenderable(displayGroupRenderer_);
        glWindow->addRenderable(markerRenderer);

        RenderablePtr testPattern(new TestPattern(glWindow.get(),
                                                  config,
                                                  mpiChannel_->getRank(),
                                                  glWindow->getTileIndex()));
        glWindow->setTestPattern(testPattern);
    }

    connect(mpiChannel_.get(), SIGNAL(received(DisplayGroupManagerPtr)),
            this, SLOT(updateDisplayGroup(DisplayGroupManagerPtr)));

    connect(mpiChannel_.get(), SIGNAL(received(OptionsPtr)),
            this, SLOT(updateOptions(OptionsPtr)));

    connect(mpiChannel_.get(), SIGNAL(received(MarkersPtr)),
            markerRenderer.get(), SLOT(setMarkers(MarkersPtr)));

    connect(mpiChannel_.get(), SIGNAL(received(PixelStreamFramePtr)),
            this, SLOT(processPixelStreamFrame(PixelStreamFramePtr)));

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

void WallApplication::renderFrame()
{
    mpiChannel_->receiveMessages(); // TODO make this an async task

    preRenderUpdate();

    renderContext_->updateGLWindows();
    mpiChannel_->globalBarrier();
    renderContext_->swapBuffers();

    postRenderUpdate();

    emit(frameFinished());
}

void WallApplication::preRenderUpdate()
{
    // synchronize clock right after receiving messages to ensure we have an
    // accurate time for rendering, etc. below
    mpiChannel_->synchronizeClock();

    ContentWindowManagerPtrs contentWindows = displayGroup_->getContentWindowManagers();

    BOOST_FOREACH(ContentWindowManagerPtr contentWindow, contentWindows)
    {
        // note that if we have multiple ContentWindowManagers corresponding to a single Content object,
        // we will call advance() multiple times per frame on that Content object...
        contentWindow->getContent()->preRenderUpdate(factories_, contentWindow, mpiChannel_);
    }
    ContentWindowManagerPtr backgroundWindow = displayGroup_->getBackgroundContentWindow();
    if (backgroundWindow)
        backgroundWindow->getContent()->preRenderUpdate(factories_, backgroundWindow, mpiChannel_);
}

void WallApplication::postRenderUpdate()
{
    ContentWindowManagerPtrs contentWindows = displayGroup_->getContentWindowManagers();

    BOOST_FOREACH(ContentWindowManagerPtr contentWindow, contentWindows)
    {
        // note that if we have multiple ContentWindowManagers corresponding to a single Content object,
        // we will call advance() multiple times per frame on that Content object...
        contentWindow->getContent()->postRenderUpdate(factories_, contentWindow, mpiChannel_);
    }
    ContentWindowManagerPtr backgroundWindow = displayGroup_->getBackgroundContentWindow();
    if (backgroundWindow)
        backgroundWindow->getContent()->postRenderUpdate(factories_, backgroundWindow, mpiChannel_);

    factories_->clearStaleFactoryObjects();
}

void WallApplication::updateDisplayGroup(DisplayGroupManagerPtr displayGroup)
{
    displayGroup_ = displayGroup;
    displayGroupRenderer_->setDisplayGroup(displayGroup);
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
