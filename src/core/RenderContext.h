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

class WallConfiguration;

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
    RenderContext(const WallConfiguration& configuration);

    /** Destructor. */
    ~RenderContext();

    /** Set the background color of all windows. */
    void setBackgroundColor(const QColor& color);

    /** Get a specific GL window. */
    GLWindowPtr getGLWindow(const int index=0) const;

    /** Get the count of GL windows. */
    size_t getGLWindowCount() const;

    /** Get the index of the active window during rendering. @deprecated */
    int getActiveGLWindowIndex() const;

    /** Render text. @see QGLWidget::renderText */
    void renderText(const int x, const int y, const QString & str,
                    const QFont& font);

    /** Render text. @see QGLWidget::renderText */
    void renderText(const double x, const double y, const double z,
                    const QString & str, const QFont& font);

    /** Add an object to be rendered. */
    void addRenderable(RenderablePtr renderable);

    /** Check if a region is visible. */
    bool isRegionVisible(const QRectF& region) const;

    /** Render GL objects on all windows. */
    void updateGLWindows();

    /** Swap GL buffers on all windows. */
    void swapBuffers();

private:
    void setupOpenGLWindows(const WallConfiguration& configuration);

    GLWindowPtrs glWindows_;
    GLWindowPtr activeGLWindow_;
    int activeGLWindowIndex_;
};

#endif
