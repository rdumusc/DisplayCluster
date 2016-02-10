/*********************************************************************/
/* Copyright (c) 2015, EPFL/Blue Brain Project                       */
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

#include "WallWindow.h"

#include "QuickRenderer.h"

#include "DisplayGroupRenderer.h"
#include "Options.h"
#include "TestPattern.h"

#include "configuration/WallConfiguration.h"

#include "config.h"
#include "MovieProvider.h"
#if ENABLE_PDF_SUPPORT
#  include "PDFProvider.h"
#endif
#include "PixelStreamProvider.h"
#include "SVGProvider.h"
#include "TextureProvider.h"

#include <QOpenGLContext>
#include <QQmlEngine>
#include <QQuickRenderControl>
#include <QThread>

namespace
{
const QUrl QML_BACKGROUND_URL( "qrc:/qml/core/Background.qml" );
}

WallWindow::WallWindow( const WallConfiguration& config,
                        QQuickRenderControl* renderControl,
                        WallToWallChannel& wallChannel )
    : QQuickWindow( renderControl )
    , _displayGroupRenderer( nullptr )
    , _testPattern( nullptr )
    , _wallChannel( wallChannel )
    , _glContext( new QOpenGLContext )
    , _renderControl( renderControl )
    , _quickRenderer( new QuickRenderer( *this ))
    , _quickRendererThread( new QThread )
    , _qmlEngine( new QQmlEngine )
    , _qmlComponent( nullptr )
    , _rootItem( nullptr )
    , _rendererInitialized( false )
{
    const QPoint& screenIndex = config.getGlobalScreenIndex();
    const QRect& screenRect = config.getScreenRect( screenIndex );
    const QPoint& windowPos = config.getWindowPos();

    setFlags( Qt::FramelessWindowHint );
    setPosition( windowPos );
    resize( screenRect.size( ));

    if( config.getFullscreen( ))
    {
        setCursor( Qt::BlankCursor );
        showFullScreen();
    }
    else
        show();

    startQuick( config );
}

WallWindow::~WallWindow()
{
    _quickRenderer->stop();

    _quickRendererThread->quit();
    _quickRendererThread->wait();

    delete _displayGroupRenderer;
    delete _qmlComponent;
    delete _qmlEngine;
    delete _glContext;
    delete _quickRenderer;
}

void WallWindow::exposeEvent( QExposeEvent* )
{
    if( !_rendererInitialized )
    {
        // Initialize the renderer once the window is shown for correct GL context
        // realisiation

        QSurfaceFormat format_;
        format_.setDepthBufferSize( 16 );
        format_.setStencilBufferSize( 8 );

        _glContext->setFormat( format_ );
        _glContext->create();

    #if QT_VERSION >= 0x050500
        _renderControl->prepareThread( _quickRendererThread );
    #endif

        _glContext->moveToThread( _quickRendererThread );
        _quickRenderer->moveToThread( _quickRendererThread );
        _quickRendererThread->start();

        _quickRenderer->init();
        _wallChannel.globalBarrier();
        _rendererInitialized = true;
    }
}

void WallWindow::startQuick( const WallConfiguration& config )
{
    _qmlComponent = new QQmlComponent(_qmlEngine, QML_BACKGROUND_URL );
    QObject* rootObject_ = _qmlComponent->create();
    _rootItem = qobject_cast< QQuickItem* >( rootObject_ );
    _rootItem->setParentItem( contentItem( ));

    // behave like SizeRootObjectToView
    _rootItem->setWidth( width( ));
    _rootItem->setHeight( height( ));

    _qmlEngine->addImageProvider( MovieProvider::ID, new MovieProvider );
#if ENABLE_PDF_SUPPORT
    _qmlEngine->addImageProvider( PDFProvider::ID, new PDFProvider );
#endif
    _qmlEngine->addImageProvider( PixelStreamProvider::ID,
                              new PixelStreamProvider );
    _qmlEngine->addImageProvider( SVGProvider::ID, new SVGProvider );
    _qmlEngine->addImageProvider( TextureProvider::ID, new TextureProvider);

    const QPoint& screenIndex = config.getGlobalScreenIndex();
    const QRect& screenRect = config.getScreenRect( screenIndex );
    _displayGroupRenderer = new DisplayGroupRenderer( *this, screenRect );
    _testPattern = new TestPattern( config, _rootItem );
    _testPattern->setPosition( -screenRect.topLeft( ));

    connect( _quickRenderer, &QuickRenderer::frameSwapped,
             _displayGroupRenderer,
             &DisplayGroupRenderer::updateRenderedFrames );
}

DisplayGroupRenderer& WallWindow::getDisplayGroupRenderer()
{
    return *_displayGroupRenderer;
}

bool WallWindow::syncAndRender()
{
    if( !_rendererInitialized )
        return true;

    auto movieProvider = dynamic_cast< MovieProvider* >
            ( engine()->imageProvider( MovieProvider::ID ));
    auto textureProvider = dynamic_cast< TextureProvider* >
            ( engine()->imageProvider( TextureProvider::ID ));
    auto& pixelStreamProvider = getPixelStreamProvider();

    _wallChannel.synchronizeClock();
    movieProvider->update( _wallChannel );
    pixelStreamProvider.update( _wallChannel );
    bool needRedraw = textureProvider->needRedraw();

    _renderControl->polishItems();
    _quickRenderer->render();

    needRedraw = needRedraw || _displayGroupRenderer->needRedraw();
    return !_wallChannel.allReady( !needRedraw );
}

void WallWindow::setRenderOptions( OptionsPtr options )
{
    setColor( options->getShowTestPattern() ? Qt::black
                                            : options->getBackgroundColor( ));
    _testPattern->setVisible( options->getShowTestPattern( ));

    _displayGroupRenderer->setRenderingOptions( options );
}

void WallWindow::setDisplayGroup( DisplayGroupPtr displayGroup )
{
    _displayGroupRenderer->setDisplayGroup( displayGroup );
}

void WallWindow::setMarkers( MarkersPtr markers )
{
    _displayGroupRenderer->setMarkers( markers );
}

PixelStreamProvider& WallWindow::getPixelStreamProvider()
{
    auto pixelStreamProvider = dynamic_cast< PixelStreamProvider* >
            ( engine()->imageProvider( PixelStreamProvider::ID ));
    return *pixelStreamProvider;
}

QQmlEngine* WallWindow::engine() const
{
    return _qmlEngine;
}

QQuickItem*	WallWindow::rootObject() const
{
    return _rootItem;
}

QOpenGLContext& WallWindow::getOpenGLContext()
{
    return *_glContext;
}

QQuickRenderControl& WallWindow::getRenderControl()
{
    return *_renderControl;
}

WallToWallChannel& WallWindow::getWallChannel()
{
    return _wallChannel;
}
