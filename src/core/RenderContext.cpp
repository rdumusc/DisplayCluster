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

#include "configuration/WallConfiguration.h"
#include "GLWindow.h"
#include "TestPattern.h"
#include "WallWindow.h"
#include "log.h"

#include <stdexcept>

#include <boost/foreach.hpp>

#ifdef __APPLE__
    #include <OpenGL/glu.h>

    // glu functions deprecated in 10.9
#   pragma clang diagnostic ignored "-Wdeprecated-declarations"
#   pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#else
    #include <GL/glu.h>
#endif


RenderContext::RenderContext( const WallConfiguration& configuration )
    : scene_( QRectF( QPointF(), configuration.getTotalSize( )))
    , activeGLWindowIndex_( -1 )
{
    setupOpenGLWindows( configuration );
}

void RenderContext::setBackgroundColor( const QColor& color )
{
    scene_.setBackgroundColor( color );
}

void RenderContext::setupOpenGLWindows( const WallConfiguration& config )
{
    for( int i = 0; i < config.getScreenCount(); ++i )
    {
        const QPoint screenIndex = config.getGlobalScreenIndex( i );
        const QRect screenRect = config.getScreenRect( screenIndex );
        const QPoint windowPos = config.getWindowPos( i );

        visibleWallArea_ = visibleWallArea_.unite( screenRect );

        WallWindowPtr window( new WallWindow( &scene_, screenRect, windowPos ));
        window->setTestPattern( TestPatternPtr( new TestPattern( config, i )));
        windows_.push_back( window );

        // share OpenGL context from the first GLWindow
        QGLWidget* shareWidget = (i==0) ? 0 :
                             static_cast<QGLWidget*>( windows_[0]->viewport( ));
        try
        {
            // The window takes ownership of the QGLWidget
            window->setViewport( new GLWindow( shareWidget ));
        }
        catch( const std::runtime_error& e )
        {
            put_flog( LOG_FATAL, "Error creating a GLWindow: '%s'", e.what( ));
            throw std::runtime_error( "Failed creating the GLWindows." );
        }

        if( config.getFullscreen( ))
            window->showFullScreen();
        else
        {
            window->setWindowFlags( Qt::FramelessWindowHint );
            window->show();
        }
    }
}

int RenderContext::getActiveGLWindowIndex() const
{
    return activeGLWindowIndex_;
}

void RenderContext::renderTextInWindow( const int x, const int y,
                                        const QString& str, const QFont& font,
                                        const QColor& color )
{
    glPushAttrib( GL_ENABLE_BIT );
    glDisable( GL_DEPTH_TEST );
    scene_.painter_->setFont( font );
    scene_.painter_->setPen( color );
    scene_.painter_->drawText( x + scene_.painterRect_.x(),
                               y + scene_.painterRect_.y(), str );
    glPopAttrib();
}

void RenderContext::renderText( const double x, const double y, const double z,
                                const QString& str, const QFont& font,
                                const QColor& color )
{
    GLdouble model[4][4], proj[4][4];
    GLint view[4];
    glGetDoublev( GL_MODELVIEW_MATRIX, &model[0][0] );
    glGetDoublev( GL_PROJECTION_MATRIX, &proj[0][0] );
    glGetIntegerv( GL_VIEWPORT, &view[0] );
    GLdouble win_x = 0.0, win_y = 0.0, win_z = 0.0;
    gluProject( x, y, z, &model[0][0], &proj[0][0], &view[0],
                &win_x, &win_y, &win_z );
    win_y = scene_.painterRect_.height() - win_y;

    renderTextInWindow( qRound( win_x ), qRound( win_y ), str, font, color );
}

void RenderContext::addRenderable( RenderablePtr renderable )
{
    scene_.addRenderable( renderable );
}

bool RenderContext::isRegionVisible( const QRectF& region ) const
{
    return region.intersects( visibleWallArea_ );
}

void RenderContext::updateGLWindows()
{
    activeGLWindowIndex_ = 0;

    BOOST_FOREACH( WallWindowPtr window, windows_ )
    {
        window->viewport()->repaint();
        ++activeGLWindowIndex_;
    }
}

void RenderContext::swapBuffers()
{
    BOOST_FOREACH( WallWindowPtr window, windows_ )
    {
        static_cast<QGLWidget*>( window->viewport( ))->makeCurrent();
        static_cast<QGLWidget*>( window->viewport( ))->swapBuffers();
    }
}

WallGraphicsScene& RenderContext::getScene()
{
    return scene_;
}

QDeclarativeEngine& RenderContext::getQmlEngine()
{
    return engine_;
}

void RenderContext::displayFps( bool value )
{
    BOOST_FOREACH( WallWindowPtr window, windows_ )
    {
        window->setShowFps( value );
    }
}

void RenderContext::displayTestPattern( const bool value )
{
    BOOST_FOREACH( WallWindowPtr window, windows_ )
    {
        window->getTestPattern()->setVisible( value );
    }
    scene_.displayTestPattern( value );
}
