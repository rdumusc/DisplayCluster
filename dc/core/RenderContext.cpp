/*********************************************************************/
/* Copyright (c) 2011 - 2012, The University of Texas at Austin.     */
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

#include "RenderContext.h"

#include "config.h"
#include "configuration/WallConfiguration.h"
#include "log.h"
#include "TestPattern.h"
#include "WallWindow.h"

#include "MovieProvider.h"
#if ENABLE_PDF_SUPPORT
#  include "PDFProvider.h"
#endif
#include "PixelStreamProvider.h"
#include "SVGProvider.h"
#include "TextureProvider.h"

namespace
{
const QUrl QML_BACKGROUND_URL( "qrc:/qml/core/Background.qml" );
}

RenderContext::RenderContext( const WallConfiguration& configuration )
    : _wallSize( configuration.getTotalSize( ))
{
    setupOpenGLWindows( configuration );
}

const QRect& RenderContext::getVisibleWallArea() const
{
    return _visibleWallArea;
}

void RenderContext::setBackgroundColor( const QColor& color )
{
    for( WallWindowPtr window : _windows )
        window->setColor( color );
}

void RenderContext::setupOpenGLWindows( const WallConfiguration& config )
{
    for( int i = 0; i < config.getScreenCount(); ++i )
    {
        const QPoint screenIndex = config.getGlobalScreenIndex( i );
        const QRect screenRect = config.getScreenRect( screenIndex );
        const QPoint windowPos = config.getWindowPos( i );

        _visibleWallArea = _visibleWallArea.united( screenRect );

        WallWindowPtr window;

        if( i == 0 )
        {
            window.reset( new WallWindow );

            QQmlEngine* engine = window->engine();

            engine->addImageProvider( MovieProvider::ID, new MovieProvider );
#if ENABLE_PDF_SUPPORT
            engine->addImageProvider( PDFProvider::ID, new PDFProvider );
#endif
            engine->addImageProvider( PixelStreamProvider::ID,
                                      new PixelStreamProvider );
            engine->addImageProvider( SVGProvider::ID, new SVGProvider );
            engine->addImageProvider( TextureProvider::ID, new TextureProvider);
        }
        else
            window.reset( new WallWindow( _windows.front()->engine( )));

        window->setPosition( windowPos );
        window->resize( screenRect.size( ));
        _windows.push_back( window );
        window->setTestPattern( TestPatternPtr( new TestPattern( config, i )));

        window->setSource( QML_BACKGROUND_URL );

        if( config.getFullscreen( ))
            window->showFullScreen();
        else
            window->show();

        window->createScene( screenRect.topLeft( ));
    }
}

void RenderContext::updateGLWindows()
{
    for( WallWindowPtr window : _windows )
    {
        window->update();
    }

//    BOOST_FOREACH( WallWindowPtr window, windows_ )
//    {
//#ifdef __APPLE__
//        // Using Qt's update() mechanism on OSX. Blocking draw calls causes
//        // the app to never render anything and hang forever in swapBuffer().
//        window->viewport()->update();
//#else
//        window->setBlockDrawCalls( false );
//        window->viewport()->repaint();
//        window->setBlockDrawCalls( true );
//#endif
//    }
//    glFinish();
}

void RenderContext::swapBuffers()
{
//    BOOST_FOREACH( WallWindowPtr window, windows_ )
//    {
//        if( !window->isExposed( ))
//            continue;

//        QGLWidget* glContext = static_cast<QGLWidget*>( window->viewport( ));
//        glContext->makeCurrent();
//        glContext->swapBuffers();
//        glFlush();
//    }
}

const QSize& RenderContext::getWallSize() const
{
    return _wallSize;
}

MovieProvider& RenderContext::getMovieProvider()
{
    auto engine = _windows.front()->engine();
    auto movieProvider = engine->imageProvider( MovieProvider::ID );
    return dynamic_cast<MovieProvider&>( *movieProvider );
}

PixelStreamProvider& RenderContext::getPixelStreamProvider()
{
    auto engine = _windows.front()->engine();
    auto pixelStreamProvider = engine->imageProvider( PixelStreamProvider::ID );
    return dynamic_cast<PixelStreamProvider&>( *pixelStreamProvider );
}

WallWindowPtrs RenderContext::getWindows()
{
    return _windows;
}

void RenderContext::displayTestPattern( const bool value )
{
    for( WallWindowPtr window : _windows )
        window->getTestPattern()->setVisible( value );
}
