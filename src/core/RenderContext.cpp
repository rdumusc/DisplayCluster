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
#include "log.h"

#include <stdexcept>

#include <boost/foreach.hpp>

RenderContext::RenderContext(const WallConfiguration& configuration)
    : activeGLWindowIndex_(-1)
{
    setupOpenGLWindows(configuration);
}

RenderContext::~RenderContext()
{
}

void RenderContext::setBackgroundColor(const QColor& color)
{
    BOOST_FOREACH(GLWindowPtr glWindow, glWindows_)
    {
        glWindow->setBackgroundColor(color);
    }
}

void RenderContext::setupOpenGLWindows(const WallConfiguration& configuration)
{
    for(int i=0; i<configuration.getScreenCount(); ++i)
    {
        const QPoint screenIndex = configuration.getGlobalScreenIndex(i);
        const QRectF normalizedCoordinates = configuration.getNormalizedScreenRect(screenIndex);

        const QRect windowRect = QRect(configuration.getScreenPosition(i),
                                       QSize(configuration.getScreenWidth(),
                                             configuration.getScreenHeight()));

        // share OpenGL context from the first GLWindow
        GLWindow* shareWidget = (i==0) ? 0 : glWindows_[0].get();

        GLWindowPtr glw;
        try
        {
            glw.reset(new GLWindow(normalizedCoordinates, windowRect, shareWidget));
        }
        catch (const std::runtime_error& e)
        {
            put_flog(LOG_FATAL, "Error creating a GLWindow: '%s'", e.what());
            throw std::runtime_error("Failed creating the GLWindows.");
        }
        glWindows_.push_back(glw);

        if(configuration.getFullscreen())
            glw->showFullScreen();
        else
            glw->show();
    }
}

GLWindowPtr RenderContext::getGLWindow(const int index) const
{
    return glWindows_[index];
}

size_t RenderContext::getGLWindowCount() const
{
    return glWindows_.size();
}

int RenderContext::getActiveGLWindowIndex() const
{
    return activeGLWindowIndex_;
}

void RenderContext::renderText(const int x, const int y, const QString& str,
                               const QFont& font)
{
    activeGLWindow_->renderText(x, y, str, font);
}

void RenderContext::renderText(const double x, const double y, const double z,
                               const QString& str, const QFont& font)
{
    activeGLWindow_->renderText(x, y, z, str, font);
}

void RenderContext::addRenderable(RenderablePtr renderable)
{
    BOOST_FOREACH(GLWindowPtr glWindow, glWindows_)
    {
        glWindow->addRenderable(renderable);
    }
}

bool RenderContext::isRegionVisible(const QRectF& region) const
{
    BOOST_FOREACH(GLWindowPtr glWindow, glWindows_)
    {
        if(glWindow->isRegionVisible(region))
            return true;
    }
    return false;
}

void RenderContext::updateGLWindows()
{
    activeGLWindowIndex_ = 0;
    BOOST_FOREACH(GLWindowPtr glWindow, glWindows_)
    {
        activeGLWindow_ = glWindow;
        glWindow->updateGL();
        ++activeGLWindowIndex_;
    }
}

void RenderContext::swapBuffers()
{
    BOOST_FOREACH(GLWindowPtr glWindow, glWindows_)
    {
        glWindow->makeCurrent();
        glWindow->swapBuffers();
    }
}
