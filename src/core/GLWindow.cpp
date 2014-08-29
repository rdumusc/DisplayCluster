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

#include "GLWindow.h"

#include "Renderable.h"

#include <stdexcept>

#include <QtOpenGL>

#ifdef __APPLE__
    #include <OpenGL/glu.h>

    // glu functions deprecated in 10.9
#   pragma clang diagnostic ignored "-Wdeprecated-declarations"
#   pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#else
    #include <GL/glu.h>
#endif

GLWindow::GLWindow(const QRectF& normalizedCoordinates, const QRect& windowRect, QGLWidget* shareWidget)
  : QGLWidget(0, shareWidget)
  , backgroundColor_(Qt::black)
  , normalizedCoordinates_(normalizedCoordinates)
{
    setGeometry(windowRect);
    setCursor(Qt::BlankCursor);

    if(shareWidget && !isSharing())
        throw std::runtime_error("failed to share OpenGL context");

    setAutoBufferSwap(false);
}

GLWindow::~GLWindow()
{
}

void GLWindow::addRenderable(RenderablePtr renderable)
{
    renderables_.append(renderable);
}

void GLWindow::setBackgroundColor(const QColor& color)
{
    backgroundColor_ = color;
}

void GLWindow::initializeGL()
{
    // enable depth testing; disable lighting
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
}

void GLWindow::paintGL()
{
    clear(backgroundColor_);
    setOrthographicView();

    foreach (RenderablePtr renderable, renderables_)
    {
        if (renderable->isVisible())
            renderable->render();
    }
}

void GLWindow::resizeGL(int w, int h)
{
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    update();
}

void GLWindow::clear(const QColor& clearColor)
{
    glClearColor(clearColor.redF(), clearColor.greenF(),
                 clearColor.blueF(), clearColor.alpha());
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void GLWindow::setOrthographicView()
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    gluOrtho2D(normalizedCoordinates_.left(), normalizedCoordinates_.right(),
               normalizedCoordinates_.bottom(), normalizedCoordinates_.top());
    glPushMatrix();

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

bool GLWindow::isRegionVisible(const QRectF& region) const
{
    return normalizedCoordinates_.intersects(region);
}

QRectF GLWindow::getProjectedPixelRect(const bool clampToViewportBorders)
{
    // get four corners in object space (recall we're in normalized 0->1 dimensions)
    const double corners[4][3] =
    {
        {0.,0.,0.},
        {1.,0.,0.},
        {1.,1.,0.},
        {0.,1.,0.}
    };

    // get four corners in screen space
    GLdouble modelview[16];
    glGetDoublev(GL_MODELVIEW_MATRIX, modelview);

    GLdouble projection[16];
    glGetDoublev(GL_PROJECTION_MATRIX, projection);

    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);

    GLdouble xWin[4][3];

    for(size_t i=0; i<4; i++)
    {
        gluProject(corners[i][0], corners[i][1], corners[i][2], modelview, projection, viewport, &xWin[i][0], &xWin[i][1], &xWin[i][2]);

        const GLdouble viewportWidth = (GLdouble)viewport[2];
        const GLdouble viewportHeight = (GLdouble)viewport[3];

        // The GL coordinates system origin is at the bottom-left corner with
        // the y-axis pointing upwards. For the QRect, we want the origin at
        // the top of the viewport with the y-axis pointing downwards.
        xWin[i][1] = viewportHeight - xWin[i][1];

        if( clampToViewportBorders )
        {
            xWin[i][0] = std::min( std::max( xWin[i][0], 0. ), viewportWidth );
            xWin[i][1] = std::min( std::max( xWin[i][1], 0. ), viewportHeight );
        }
    }

    const QPointF topleft( xWin[0][0], xWin[0][1] );
    const QPointF bottomright( xWin[2][0], xWin[2][1] );
    return QRectF( topleft, bottomright );
}
