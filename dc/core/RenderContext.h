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

#ifndef RENDERCONTEXT_H
#define RENDERCONTEXT_H

#include "types.h"

#include <QRectF>
#include <QColor>
#include <QFont>

#include <QQmlEngine>
#include <QQuickItem>

/**
 * A render context composed of multiple GL windows.
 */
class RenderContext
{
public:
    /**
     * Create a new RenderContext and initialize the GLWindows.
     * @param configuration The configuration that describes the window settings
     * @throw std::runtime_error if the context initialization failed.
     */
    RenderContext( const WallConfiguration& configuration );

    /** Get the area of the wall which is visible in this context. */
    const QRect& getVisibleWallArea() const;

    /** Set the background color of all windows. */
    void setBackgroundColor( const QColor& color );

    /** Render GL objects on all windows. */
    void updateGLWindows();

    /** Swap GL buffers on all windows. */
    void swapBuffers();

    /** Get access to the scene object. */
    const QSize& getWallSize() const;

    /** Get the movie provider. */
    MovieProvider& getMovieProvider();

    /** Get the pixel stream provider. */
    PixelStreamProvider& getPixelStreamProvider();

    /** Display or hide the test pattern. */
    void displayTestPattern( bool value );

    /** Display or hide the fps counter. */
    void displayFps( bool value );

    /** Get access to the windows. */
    WallWindowPtrs getWindows();

private:
    void setupOpenGLWindows( const WallConfiguration& config );
    void setupVSync();

    WallWindowPtrs _windows;

    QSize _wallSize;
    QRect _visibleWallArea;
};

#endif
