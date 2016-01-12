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

#include <QQmlEngine>

namespace
{
const QUrl QML_BACKGROUND_URL( "qrc:/qml/core/Background.qml" );
}

WallWindow::WallWindow( const WallConfiguration& config )
    : QQuickView()
    , _displayGroupRenderer( nullptr )
    , _testPattern( nullptr )
{
    engine()->addImageProvider( MovieProvider::ID, new MovieProvider );
#if ENABLE_PDF_SUPPORT
    engine()->addImageProvider( PDFProvider::ID, new PDFProvider );
#endif
    engine()->addImageProvider( PixelStreamProvider::ID,
                              new PixelStreamProvider );
    engine()->addImageProvider( SVGProvider::ID, new SVGProvider );
    engine()->addImageProvider( TextureProvider::ID, new TextureProvider);

    const QPoint& screenIndex = config.getGlobalScreenIndex();
    const QRect& screenRect = config.getScreenRect( screenIndex );
    const QPoint& windowPos = config.getWindowPos();

    setPosition( windowPos );
    resize( screenRect.size( ));
    setSurfaceType( QWindow::OpenGLSurface );
    setResizeMode( SizeRootObjectToView );
    setFlags( Qt::FramelessWindowHint );

    setSource( QML_BACKGROUND_URL );

    _displayGroupRenderer = new DisplayGroupRenderer( *this,
                                                      screenRect.topLeft( ));
    _testPattern = new TestPattern( config, rootObject( ));
    _testPattern->setPosition( -screenRect.topLeft( ));

    if( config.getFullscreen( ))
    {
        setCursor( Qt::BlankCursor );
        showFullScreen();
    }
    else
        show();
}

DisplayGroupRenderer& WallWindow::getDisplayGroupRenderer()
{
    return *_displayGroupRenderer;
}

void WallWindow::preRenderUpdate( WallToWallChannel& wallChannel )
{
    auto movieProvider = dynamic_cast< MovieProvider* >
            ( engine()->imageProvider( MovieProvider::ID ));
    auto& pixelStreamProvider = getPixelStreamProvider();

    movieProvider->update( wallChannel );
    pixelStreamProvider.update( wallChannel );
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
