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

#include "MasterApplication.h"

#include "log.h"
#include "CommandLineParameters.h"
#include "MasterWindow.h"
#include "DisplayGroup.h"
#include "ContentFactory.h"
#include "configuration/MasterConfiguration.h"
#include "MasterToWallChannel.h"
#include "MasterFromWallChannel.h"
#include "Options.h"
#include "Markers.h"

#if ENABLE_TUIO_TOUCH_LISTENER
#  include "MultiTouchListener.h"
#  include "DisplayGroupGraphicsView.h" // Required to cast to QGraphicsView
#endif

#include "localstreamer/PixelStreamerLauncher.h"
#include "StateSerializationHelper.h"
#include "PixelStreamWindowManager.h"

#include "SessionCommandHandler.h"
#include "FileCommandHandler.h"
#include "WebbrowserCommandHandler.h"

#include "ws/WebServiceServer.h"
#include "ws/TextInputDispatcher.h"
#include "ws/TextInputHandler.h"
#include "ws/DisplayGroupAdapter.h"

#include <deflect/CommandHandler.h>
#include <deflect/NetworkListener.h>
#include <deflect/PixelStreamDispatcher.h>

#include <stdexcept>

MasterApplication::MasterApplication(int& argc_, char** argv_, MPIChannelPtr worldChannel)
    : QApplication(argc_, argv_)
    , masterToWallChannel_(new MasterToWallChannel(worldChannel))
    , masterFromWallChannel_(new MasterFromWallChannel(worldChannel))
    , markers_(new Markers)
{
    // don't create touch points for mouse events and vice versa
    setAttribute( Qt::AA_SynthesizeTouchForUnhandledMouseEvents, false );
    setAttribute( Qt::AA_SynthesizeMouseForUnhandledTouchEvents, false );

    CommandLineParameters options(argc_, argv_);
    if (options.getHelp())
        options.showSyntax();

    if (!createConfig(options.getConfigFilename()))
        throw std::runtime_error("MasterApplication: initialization failed.");

    displayGroup_.reset( new DisplayGroup( config_->getTotalSize( )));

    init();

    if(!options.getSessionFilename().isEmpty())
        StateSerializationHelper(displayGroup_).load(options.getSessionFilename());
}

MasterApplication::~MasterApplication()
{
    networkListener_.reset();

    masterToWallChannel_->sendQuit();

    mpiSendThread_.quit();
    mpiSendThread_.wait();

    mpiReceiveThread_.quit();
    mpiReceiveThread_.wait();

    webServiceServer_->stop();
    webServiceServer_->wait();
}

void MasterApplication::init()
{
    connect(this, SIGNAL(lastWindowClosed()), this, SLOT(quit()));

    masterWindow_.reset(new MasterWindow(displayGroup_, *config_));
    pixelStreamWindowManager_.reset(new PixelStreamWindowManager(*displayGroup_));

    initPixelStreamLauncher();
    startNetworkListener();
    startWebservice(config_->getWebServicePort());
    initMPIConnection();
    restoreBackground();

#if ENABLE_TUIO_TOUCH_LISTENER
    initTouchListener();
#endif
}

bool MasterApplication::createConfig(const QString& filename)
{
    try
    {
        config_.reset(new MasterConfiguration(filename));
    }
    catch (const std::runtime_error& e)
    {
        put_flog(LOG_FATAL, "Could not load configuration. '%s'", e.what());
        return false;
    }
    return true;
}

void MasterApplication::startNetworkListener()
{
    if (networkListener_)
        return;

    try
    {
        networkListener_.reset(new deflect::NetworkListener);
    }
    catch (const std::runtime_error& e)
    {
        put_flog(LOG_FATAL, "Could not start NetworkListener. '%s'", e.what());
        return;
    }

    deflect::PixelStreamDispatcher& dispatcher =
            networkListener_->getPixelStreamDispatcher();

    connect( &dispatcher, SIGNAL( openPixelStream( QString )),
             pixelStreamWindowManager_.get(),
             SLOT( openPixelStreamWindow( QString )));
    connect( &dispatcher, SIGNAL( deletePixelStream( QString )),
             pixelStreamWindowManager_.get(),
             SLOT( closePixelStreamWindow( QString )));
    connect( pixelStreamWindowManager_.get(),
             SIGNAL( pixelStreamWindowClosed( QString )),
             &dispatcher, SLOT( deleteStream( QString )));

    deflect::CommandHandler& handler = networkListener_->getCommandHandler();
    handler.registerCommandHandler(new FileCommandHandler(displayGroup_, *pixelStreamWindowManager_));
    handler.registerCommandHandler(new SessionCommandHandler(*displayGroup_));

    const QString& url = config_->getWebBrowserDefaultURL();
    handler.registerCommandHandler(new WebbrowserCommandHandler(
                                       *pixelStreamWindowManager_,
                                       *pixelStreamerLauncher_,
                                       url));
}

void MasterApplication::startWebservice(const int webServicePort)
{
    if (webServiceServer_)
        return;

    webServiceServer_.reset(new WebServiceServer(webServicePort));

    DisplayGroupAdapterPtr adapter(new DisplayGroupAdapter(displayGroup_));
    TextInputHandler* textInputHandler = new TextInputHandler(adapter);
    webServiceServer_->addHandler("/dcapi/textinput", dcWebservice::HandlerPtr(textInputHandler));

    textInputHandler->moveToThread(webServiceServer_.get());
    textInputDispatcher_.reset(new TextInputDispatcher(displayGroup_));
    textInputDispatcher_->connect(textInputHandler, SIGNAL(receivedKeyInput(char)),
                         SLOT(sendKeyEventToActiveWindow(char)));

    webServiceServer_->start();
}

void MasterApplication::restoreBackground()
{
    OptionsPtr options = masterWindow_->getOptions();
    options->setBackgroundColor( config_->getBackgroundColor( ));

    const QString& uri = config_->getBackgroundUri();
    if( !uri.isEmpty( ))
        options->setBackgroundContent( ContentFactory::getContent( uri ));
}

void MasterApplication::initPixelStreamLauncher()
{
    pixelStreamerLauncher_.reset(new PixelStreamerLauncher(*pixelStreamWindowManager_, *config_));

    pixelStreamerLauncher_->connect(masterWindow_.get(), SIGNAL(openWebBrowser(QPointF,QSize,QString)),
                                    SLOT(openWebBrowser(QPointF,QSize,QString)));
    pixelStreamerLauncher_->connect(masterWindow_.get(), SIGNAL(openDock(QPointF)),
                                    SLOT(openDock(QPointF)));
    pixelStreamerLauncher_->connect(masterWindow_.get(), SIGNAL(hideDock()),
                                    SLOT(hideDock()));
}

void MasterApplication::initMPIConnection()
{
    masterToWallChannel_->moveToThread( &mpiSendThread_ );
    masterFromWallChannel_->moveToThread( &mpiReceiveThread_ );

    connect( displayGroup_.get(), SIGNAL( modified( DisplayGroupPtr )),
             masterToWallChannel_.get(), SLOT( sendAsync( DisplayGroupPtr )),
             Qt::DirectConnection );

    connect( masterWindow_->getOptions().get(), SIGNAL( updated( OptionsPtr )),
             masterToWallChannel_.get(), SLOT( sendAsync( OptionsPtr )),
             Qt::DirectConnection );

    connect( markers_.get(), SIGNAL( updated( MarkersPtr )),
             masterToWallChannel_.get(), SLOT( sendAsync( MarkersPtr )),
             Qt::DirectConnection );

    connect( &networkListener_->getPixelStreamDispatcher(),
             SIGNAL( sendFrame( deflect::PixelStreamFramePtr )),
             masterToWallChannel_.get(),
             SLOT( send( deflect::PixelStreamFramePtr )));
    connect( &networkListener_->getPixelStreamDispatcher(),
             SIGNAL( sendFrame( deflect::PixelStreamFramePtr )),
             pixelStreamWindowManager_.get(),
             SLOT( updateStreamDimensions( deflect::PixelStreamFramePtr )));
    connect( networkListener_.get(),
             SIGNAL( registerToEvents( QString, bool, deflect::EventReceiver*)),
             pixelStreamWindowManager_.get(),
             SLOT( registerEventReceiver( QString, bool,
                                          deflect::EventReceiver* )));

    connect( pixelStreamWindowManager_.get(),
             SIGNAL( pixelStreamWindowClosed( QString )),
             networkListener_.get(), SLOT(onPixelStreamerClosed( QString )));
    connect( pixelStreamWindowManager_.get(),
             SIGNAL( eventRegistrationReply( QString, bool )),
             networkListener_.get(),
             SLOT( onEventRegistrationReply( QString, bool )));

    connect( masterFromWallChannel_.get(),
             SIGNAL( receivedRequestFrame( const QString )),
             &networkListener_->getPixelStreamDispatcher(),
             SLOT( requestFrame( const QString )));

    connect( &mpiReceiveThread_, SIGNAL( started( )),
             masterFromWallChannel_.get(), SLOT( processMessages( )));

    mpiSendThread_.start();
    mpiReceiveThread_.start();
}

#if ENABLE_TUIO_TOUCH_LISTENER
void MasterApplication::initTouchListener()
{
    QGraphicsView* graphicsView = masterWindow_->getGraphicsView();
    touchListener_.reset(new MultiTouchListener(graphicsView));
    connect(touchListener_.get(), SIGNAL(touchPointAdded(int,QPointF)),
            markers_.get(), SLOT(addMarker(int,QPointF)));
    connect(touchListener_.get(), SIGNAL(touchPointUpdated(int,QPointF)),
            markers_.get(), SLOT(updateMarker(int,QPointF)));
    connect(touchListener_.get(), SIGNAL(touchPointRemoved(int)),
            markers_.get(), SLOT(removeMarker(int)));
}
#endif
