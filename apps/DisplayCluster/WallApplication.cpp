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

#include "log.h"
#include "CommandLineParameters.h"

#include "MPIChannel.h"
#include "RenderController.h"
#include "WallFromMasterChannel.h"
#include "WallToMasterChannel.h"
#include "WallToWallChannel.h"
#include "WallWindow.h"

#include "configuration/WallConfiguration.h"

#include "PixelStreamProvider.h"

#include <stdexcept>

#include <boost/bind.hpp>
#include <QOpenGLContext>
#if QT_VERSION >= 0x050300
#  include <QOpenGLFunctions>
#else
#  include <QOpenGLFunctions_1_0>
#endif

WallApplication::WallApplication( int& argc_, char** argv_,
                                  MPIChannelPtr worldChannel,
                                  MPIChannelPtr wallChannel )
    : QApplication( argc_, argv_ )
    , _wallChannel( new WallToWallChannel( wallChannel ))
    , _renderedFrames( 0 )
{
    CommandLineParameters options( argc_, argv_ );
    if( options.getHelp( ))
        options.showSyntax();

    if ( !createConfig( options.getConfigFilename(), worldChannel->getRank( )))
        throw std::runtime_error(" WallApplication: initialization failed." );

    initWallWindow();
    initMPIConnection( worldChannel );
    startRendering();
}

WallApplication::~WallApplication()
{
    _mpiReceiveThread.quit();
    _mpiReceiveThread.wait();

    _mpiSendThread.quit();
    _mpiSendThread.wait();
}

bool WallApplication::createConfig( const QString& filename, const int rank )
{
    try
    {
        _config.reset( new WallConfiguration( filename, rank ));
    }
    catch( const std::runtime_error& e )
    {
        put_flog( LOG_FATAL, "Could not load configuration. '%s'", e.what( ));
        return false;
    }
    return true;
}

void WallApplication::initWallWindow()
{
    try
    {
        _window = new WallWindow( *_config );
    }
    catch( const std::runtime_error& e )
    {
        put_flog( LOG_FATAL, "Error creating WallWindow: '%s'",
                  e.what( ));
        throw std::runtime_error( "WallApplication: initialization failed." );
    }

    _renderController.reset( new RenderController( *_window ));
}

void WallApplication::initMPIConnection( MPIChannelPtr worldChannel )
{
    _fromMasterChannel.reset( new WallFromMasterChannel( worldChannel ));
    _toMasterChannel.reset( new WallToMasterChannel( worldChannel ));

    _fromMasterChannel->moveToThread( &_mpiReceiveThread );
    _toMasterChannel->moveToThread( &_mpiSendThread );

    connect( _fromMasterChannel.get(), SIGNAL( receivedQuit( )),
             _renderController.get(), SLOT( updateQuit( )));

    connect( _fromMasterChannel.get(), SIGNAL( received( DisplayGroupPtr )),
             _renderController.get(), SLOT( updateDisplayGroup( DisplayGroupPtr )));

    connect( _fromMasterChannel.get(), SIGNAL( received( OptionsPtr )),
             _renderController.get(), SLOT( updateOptions( OptionsPtr )));

    connect( _fromMasterChannel.get(), SIGNAL( received( MarkersPtr )),
             _renderController.get(), SLOT( updateMarkers( MarkersPtr )));

    connect( _fromMasterChannel.get(),
             SIGNAL( received( deflect::FramePtr )),
             &_window->getPixelStreamProvider(),
             SLOT( setNewFrame( deflect::FramePtr )));

    if( _wallChannel->getRank() == 0 )
    {
        connect( &_window->getPixelStreamProvider(),
                 SIGNAL( requestFrame( QString )),
                 _toMasterChannel.get(), SLOT( sendRequestFrame( QString )));
    }

    connect( _fromMasterChannel.get(), SIGNAL( receivedQuit( )),
             _toMasterChannel.get(), SLOT( sendQuit( )));

    connect( &_mpiReceiveThread, SIGNAL( started( )),
             _fromMasterChannel.get(), SLOT( processMessages( )));

    _mpiReceiveThread.start();
    _mpiSendThread.start();
}

void WallApplication::startRendering()
{
    // setup connection so renderFrame() will be called continuously.
    // Must be a queued connection to avoid infinite recursion.
    connect( this, SIGNAL( frameFinished( )),
             this, SLOT( renderFrame( )), Qt::QueuedConnection );

    // swap sync; afterRendering signal is emitted before swapBuffers
    connect( _window, &WallWindow::afterRendering, [this]()
    {
        auto gl = _window->openglContext();
#if QT_VERSION >= 0x050300
        gl->functions()->glFinish();
#else
        auto funcs = gl->versionFunctions< QOpenGLFunctions_1_0 >();
        funcs->initializeOpenGLFunctions();
        funcs->glFinish();
#endif
        if( _renderedFrames == 0 )
            _wallChannel->globalBarrier();
        ++_renderedFrames;
    });

    // trigger new renderloop after rendering & swap or quit application
    connect( _window, &WallWindow::frameSwapped, [this]()
    {
        auto gl = _window->openglContext();
#if QT_VERSION >= 0x050300
        gl->functions()->glFlush();
#else
        auto funcs = gl->versionFunctions< QOpenGLFunctions_1_0 >();
        funcs->initializeOpenGLFunctions();
        funcs->glFlush();
#endif
        // expose event also causes swap, but don't trigger new frames in that
        // case to not messup/deadlock swap- and object-sync
        if( _renderedFrames == 1 )
        {
            if( _renderController->quitRendering( ))
                _window->deleteLater();
           else
                emit frameFinished();
        }
    });

    renderFrame();
}

void WallApplication::renderFrame()
{
    _renderedFrames = 0;
    _wallChannel->synchronizeClock();
    _renderController->preRenderUpdate( *_wallChannel );
    _window->update();
}
