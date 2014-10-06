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

#ifndef GL_WINDOW_H
#define GL_WINDOW_H

#include <QGLWidget>
#include <QList>

#include "types.h"

/**
 * An OpenGL window used by Wall applications to render contents.
 */
class GLWindow : public QGLWidget
{
public:
    /**
     * Create a new window.
     * @param normalizedCoordinates The normalized window coordinates.
     * @param windowRect The position and dimensions for the window in pixels.
     * @param shareWidget An optional widget to share an existing GLContext.
     *                    A new GLContext is allocated if not provided.
     * @throw std::runtime_error if the initialization failed.
     */
    GLWindow(const QRectF& normalizedCoordinates, const QRect& windowRect,
             QGLWidget* shareWidget = 0);

    /** Destructor. */
    ~GLWindow();

    /** Add an object to be rendered. */
    void addRenderable(RenderablePtr renderable);

    /** Set the background color */
    void setBackgroundColor(const QColor& color);

    /**
     * Is the given region visible in this window.
     * @param region The region in normalized global screen space, i.e. top-left
     *        of tiled display is (0,0) and bottom-right is (1,1)
     * @return true if (partially) visible, false otherwise
     */
    bool isRegionVisible(const QRectF& region) const;

    /**
     * Get the region spanned by a unit rectangle {(0;0),(1;1)} in the current
     * GL view.
     * The region is in screen coordinates with the origin at the viewport's
     * top-left corner.
     * @param clampToViewportBorders Clamp to the visible part of the region.
     * @return The region in pixel units.
     * @deprecated
     */
    static QRectF getProjectedPixelRect(const bool clampToViewportBorders);

protected:
    /** @name Overloaded methods from QGLWidget */
    //@{
    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int w, int h) override;
    //@}

private:
    QColor backgroundColor_;
    QRectF normalizedCoordinates_;
    QList<RenderablePtr> renderables_;

    void clear(const QColor& clearColor);
    void setOrthographicView();
};

#endif
