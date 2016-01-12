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
#include "WallScene.h"

#include "config.h"
#include "MovieProvider.h"
#if ENABLE_PDF_SUPPORT
#  include "PDFProvider.h"
#endif
#include "PixelStreamProvider.h"
#include "SVGProvider.h"
#include "TextureProvider.h"

#include <QQmlEngine>

WallWindow::WallWindow( const QRect& wallArea )
    : QQuickView()
    , _wallArea( wallArea )
    , _blockUpdates( false )
    , _isExposed( false )
{
    setSurfaceType( QWindow::OpenGLSurface );
    setResizeMode( SizeRootObjectToView );
    setFlags( Qt::FramelessWindowHint );
    resize( wallArea.size( ));

    engine()->addImageProvider( MovieProvider::ID, new MovieProvider );
#if ENABLE_PDF_SUPPORT
    engine()->addImageProvider( PDFProvider::ID, new PDFProvider );
#endif
    engine()->addImageProvider( PixelStreamProvider::ID,
                              new PixelStreamProvider );
    engine()->addImageProvider( SVGProvider::ID, new SVGProvider );
    engine()->addImageProvider( TextureProvider::ID, new TextureProvider);
}

WallScene& WallWindow::getScene()
{
    return *_scene;
}

void WallWindow::createScene( const QPoint& pos )
{
    _scene = make_unique<WallScene>( *this, pos );
}

void WallWindow::setTestPattern( TestPatternPtr testPattern )
{
    _testPattern = testPattern;
}

TestPatternPtr WallWindow::getTestPattern()
{
    return _testPattern;
}

void WallWindow::setBlockDrawCalls( const bool enable )
{
    _blockUpdates = enable;
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
    setColor( options->getBackgroundColor( ));
    getTestPattern()->setVisible( options->getShowTestPattern( ));

    getScene().getDisplayGroupRenderer().setRenderingOptions( options );
}

void WallWindow::setDisplayGroup( DisplayGroupPtr displayGroup )
{
    getScene().setDisplayGroup( displayGroup );
}

void WallWindow::setMarkers( MarkersPtr markers )
{
    getScene().getDisplayGroupRenderer().setMarkers( markers );
}

PixelStreamProvider& WallWindow::getPixelStreamProvider()
{
    auto pixelStreamProvider = dynamic_cast< PixelStreamProvider* >
            ( engine()->imageProvider( PixelStreamProvider::ID ));
    return *pixelStreamProvider;
}

//bool WallWindow::isExposed() const
//{
//    return isExposed_;
//}

//void WallWindow::drawForeground( QPainter* painter, const QRectF& rect_ )
//{
//    if( testPattern_ && testPattern_->isVisible( ))
//    {
//        testPattern_->draw( painter, rect_ );
//        return;
//    }

//    QGraphicsView::drawForeground( painter, rect_ );
//}

//void WallWindow::paintEvent( QPaintEvent* event_ )
//{
//    if( blockUpdates_ )
//        return;

//    isExposed_ = true;

//    QGraphicsView::paintEvent( event_ );
//}
