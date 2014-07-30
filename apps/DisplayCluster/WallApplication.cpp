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
#include "WallFromMasterChannel.h"
#include "WallToMasterChannel.h"
#include "WallToWallChannel.h"
#include "configuration/WallConfiguration.h"
#include "RenderContext.h"
#include "Factories.h"
#include "GLWindow.h"
#include "TestPattern.h"
#include "DisplayGroupManager.h"
#include "DisplayGroupRenderer.h"
#include "MarkerRenderer.h"

#include <boost/make_shared.hpp>
#include <boost/bind.hpp>

WallApplication::WallApplication(int& argc_, char** argv_, MPIChannelPtr worldChannel, MPIChannelPtr wallChannel)
    : Application(argc_, argv_)
    , wallChannel_(new WallToWallChannel(wallChannel))
    , syncQuit_(false)
    , syncDisplayGroup_(boost::make_shared<DisplayGroupManager>())
{
    WallConfiguration* config = new WallConfiguration(getConfigFilename(),
                                                      worldChannel->getRank());
    g_configuration = config;
    syncOptions_.setCallback(boost::bind(&Configuration::setOptions, config, _1));

    initRenderContext(config);
    setupTestPattern(config, worldChannel->getRank());
    initMPIConnection(worldChannel);
    startRendering();
}

WallApplication::~WallApplication()
{
    // Must be done before destructing the GLWindows to release GL objects
    factories_->clear();

    mpiReceiveThread_.quit();
    mpiReceiveThread_.wait();

    mpiSendThread_.quit();
    mpiSendThread_.wait();
}

void WallApplication::initRenderContext(const WallConfiguration* config)
{
    renderContext_.reset(new RenderContext(config));
    factories_.reset(new Factories(boost::bind(&WallApplication::onNewObject, this, _1)));

    DisplayGroupRendererPtr displayGroupRenderer(new DisplayGroupRenderer(factories_));
    MarkerRendererPtr markerRenderer(new MarkerRenderer());

    syncDisplayGroup_.setCallback(boost::bind(&DisplayGroupRenderer::setDisplayGroup,
                                               displayGroupRenderer.get(), _1));

    syncMarkers_.setCallback(boost::bind(&MarkerRenderer::setMarkers,
                                          markerRenderer.get(), _1));

    renderContext_->addRenderable(displayGroupRenderer);
    renderContext_->addRenderable(markerRenderer);
}

void WallApplication::onNewObject(FactoryObject& object)
{
    object.setRenderContext(renderContext_.get());

    PixelStream* pixelStream = dynamic_cast< PixelStream* >(&object);
    // only one process needs to request new frames
    if(pixelStream && wallChannel_->getRank() == 0)
    {
        connect(pixelStream, SIGNAL(requestFrame(const QString)),
                toMasterChannel_.get(), SLOT(sendRequestFrame(const QString)));
    }
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

void WallApplication::initMPIConnection(MPIChannelPtr worldChannel)
{
    fromMasterChannel_.reset(new WallFromMasterChannel(worldChannel));
    toMasterChannel_.reset(new WallToMasterChannel(worldChannel));

    fromMasterChannel_->moveToThread(&mpiReceiveThread_);
    toMasterChannel_->moveToThread(&mpiSendThread_);

    connect(fromMasterChannel_.get(), SIGNAL(received(DisplayGroupManagerPtr)),
            this, SLOT(updateDisplayGroup(DisplayGroupManagerPtr)));

    connect(fromMasterChannel_.get(), SIGNAL(received(OptionsPtr)),
            this, SLOT(updateOptions(OptionsPtr)));

    connect(fromMasterChannel_.get(), SIGNAL(received(MarkersPtr)),
            this, SLOT(updateMarkers(MarkersPtr)));

    connect(fromMasterChannel_.get(), SIGNAL(received(PixelStreamFramePtr)),
            factories_.get(), SLOT(updatePixelStream(PixelStreamFramePtr)));

    connect(fromMasterChannel_.get(), SIGNAL(receivedQuit()),
            this, SLOT(updateQuit()));

    connect(fromMasterChannel_.get(), SIGNAL(receivedQuit()),
            toMasterChannel_.get(), SLOT(sendQuit()));

    connect(&mpiReceiveThread_, SIGNAL(started()),
            fromMasterChannel_.get(), SLOT(processMessages()));

    mpiReceiveThread_.start();
    mpiSendThread_.start();
}

void WallApplication::startRendering()
{
    // setup connection so renderFrame() will be called continuously.
    // Must be a queued connection to avoid infinite recursion.
    connect(this, SIGNAL(frameFinished()),
            this, SLOT(renderFrame()), Qt::QueuedConnection);
    renderFrame();
}

void WallApplication::renderFrame()
{
    preRenderUpdate();

    renderContext_->updateGLWindows();
    wallChannel_->globalBarrier();
    renderContext_->swapBuffers();

    postRenderUpdate();

    emit(frameFinished());
}

void WallApplication::preRenderUpdate()
{
    syncObjects();
    wallChannel_->synchronizeClock();
    factories_->preRenderUpdate(*syncDisplayGroup_.get(), *wallChannel_);
}

void WallApplication::syncObjects()
{
    const SyncFunction& versionCheckFunc =
        boost::bind( &WallToWallChannel::checkVersion, wallChannel_.get(), _1 );

    syncQuit_.sync(versionCheckFunc);
    if (syncQuit_.get())
        quit();
    syncDisplayGroup_.sync(versionCheckFunc);
    syncMarkers_.sync(versionCheckFunc);
    syncOptions_.sync(versionCheckFunc);
}

void WallApplication::postRenderUpdate()
{
    factories_->postRenderUpdate(*syncDisplayGroup_.get(), *wallChannel_);
    factories_->clearStaleFactoryObjects();
}

void WallApplication::updateQuit()
{
    syncQuit_.update(true);
}

void WallApplication::updateDisplayGroup(DisplayGroupManagerPtr displayGroup)
{
    syncDisplayGroup_.update(displayGroup);
}

void WallApplication::updateMarkers(MarkersPtr markers)
{
    syncMarkers_.update(markers);
}

void WallApplication::updateOptions(OptionsPtr options)
{
    syncOptions_.update(options);
}
