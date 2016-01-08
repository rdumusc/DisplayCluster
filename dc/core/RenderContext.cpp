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
#include "log.h"
#include "TestPattern.h"
#include "WallWindow.h"

namespace
{
const QUrl QML_BACKGROUND_URL( "qrc:/qml/core/Background.qml" );
}

RenderContext::RenderContext( const WallConfiguration& configuration )
    : _window()
    , _wallSize( configuration.getTotalSize( ))
{
    const QPoint screenIndex = configuration.getGlobalScreenIndex();
    const QRect screenRect = configuration.getScreenRect( screenIndex );
    const QPoint windowPos = configuration.getWindowPos();

    _window.reset( new WallWindow( screenRect ));
    _window->setPosition( windowPos );
    _window->setTestPattern( TestPatternPtr( new TestPattern( configuration )));

    _window->setSource( QML_BACKGROUND_URL );

    if( configuration.getFullscreen( ))
        _window->showFullScreen();
    else
        _window->show();

    _window->createScene( screenRect.topLeft( ));
}

void RenderContext::updateWindow()
{
    _window->update();
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
