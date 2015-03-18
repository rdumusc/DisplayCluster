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

RenderContext::RenderContext( const WallConfiguration& configuration )
    : scene_( QRectF( QPointF(), configuration.getTotalSize( )))
{
    setupOpenGLWindows( configuration );
    setupVSync();
}

const QRect& RenderContext::getVisibleWallArea() const
{
    return visibleWallArea_;
}

void RenderContext::setBackgroundColor( const QColor& color )
{
    scene_.setBackgroundBrush( color );
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

void RenderContext::setupVSync()
{
    BOOST_FOREACH( WallWindowPtr window, windows_ )
    {
        QGLWidget* glContext = static_cast<QGLWidget*>( window->viewport( ));
        glContext->makeCurrent();
        if( window != windows_.front( ))
            window->disableVSync();
    }
}

void RenderContext::updateGLWindows()
{
    BOOST_FOREACH( WallWindowPtr window, windows_ )
    {
        window->setBlockDrawCalls( false );
        window->viewport()->repaint();
        window->setBlockDrawCalls( true );
    }
    glFinish();
}

void RenderContext::swapBuffers()
{
    BOOST_FOREACH( WallWindowPtr window, windows_ )
    {
        QGLWidget* glContext = static_cast<QGLWidget*>( window->viewport( ));
        glContext->makeCurrent();
        glContext->swapBuffers();
        glFlush();
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

void RenderContext::displayFps( const bool value )
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
}
