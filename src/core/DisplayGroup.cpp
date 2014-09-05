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

#include "DisplayGroup.h"

#include "ContentWindow.h"

#include "log.h"
#include <boost/foreach.hpp>

DisplayGroup::DisplayGroup()
{
}

DisplayGroup::~DisplayGroup()
{
}

void DisplayGroup::addContentWindow(ContentWindowPtr contentWindow, DisplayGroupInterface * source)
{
    BOOST_FOREACH(ContentWindowPtr existingWindow, contentWindows_)
    {
        if (contentWindow->getID() == existingWindow->getID())
        {
            put_flog(LOG_WARN, "A window with the same id already exists!");
            return;
        }
    }

    DisplayGroupInterface::addContentWindow(contentWindow, source);

    if(source != this)
    {
        contentWindow->setDisplayGroup(shared_from_this());
        watchChanges(contentWindow);

        emit modified(shared_from_this());
    }
}

void DisplayGroup::watchChanges(ContentWindowPtr contentWindow)
{
    // Don't call sendDisplayGroup() on movedToFront() or destroyed() since it happens already
    connect(contentWindow.get(), SIGNAL(contentDimensionsChanged(int, int, ContentWindowInterface *)),
            this, SLOT(sendDisplayGroup()));
    connect(contentWindow.get(), SIGNAL(coordinatesChanged(QRectF, ContentWindowInterface *)),
            this, SLOT(sendDisplayGroup()));
    connect(contentWindow.get(), SIGNAL(positionChanged(double, double, ContentWindowInterface *)),
            this, SLOT(sendDisplayGroup()));
    connect(contentWindow.get(), SIGNAL(sizeChanged(double, double, ContentWindowInterface *)),
            this, SLOT(sendDisplayGroup()));
    connect(contentWindow.get(), SIGNAL(centerChanged(double, double, ContentWindowInterface *)),
            this, SLOT(sendDisplayGroup()));
    connect(contentWindow.get(), SIGNAL(zoomChanged(double, ContentWindowInterface *)),
            this, SLOT(sendDisplayGroup()));
    connect(contentWindow.get(), SIGNAL(windowStateChanged(ContentWindowInterface::WindowState, ContentWindowInterface *)),
            this, SLOT(sendDisplayGroup()));
    connect(contentWindow.get(), SIGNAL(contentModified()),
            this, SLOT(sendDisplayGroup()));
}

void DisplayGroup::removeContentWindow(ContentWindowPtr contentWindow, DisplayGroupInterface * source)
{
    DisplayGroupInterface::removeContentWindow(contentWindow, source);

    if(source != this)
    {
        // disconnect any existing connections with the window
        disconnect(contentWindow.get(), 0, this, 0);

        // set null display group in content window manager object
        contentWindow->setDisplayGroup(DisplayGroupPtr());

        emit modified(shared_from_this());
    }
}

void DisplayGroup::moveContentWindowToFront(ContentWindowPtr contentWindow, DisplayGroupInterface * source)
{
    DisplayGroupInterface::moveContentWindowToFront(contentWindow, source);

    if(source != this)
    {
        emit modified(shared_from_this());
    }
}

void DisplayGroup::setBackgroundContent(ContentPtr content)
{
    if (content)
    {
        backgroundContent_ = ContentWindowPtr(new ContentWindow(content));
        // set display group in content window manager object
        backgroundContent_->setDisplayGroup(shared_from_this());
        backgroundContent_->adjustSize( SIZE_FULLSCREEN );
        watchChanges(backgroundContent_);
    }
    else
    {
        backgroundContent_ = ContentWindowPtr();
    }

    emit modified(shared_from_this());
}

ContentWindowPtr DisplayGroup::getBackgroundContentWindow() const
{
    return backgroundContent_;
}

bool DisplayGroup::isEmpty() const
{
    return contentWindows_.empty();
}

ContentWindowPtr DisplayGroup::getActiveWindow() const
{
    if (isEmpty())
        return ContentWindowPtr();

    return contentWindows_.back();
}

void DisplayGroup::sendDisplayGroup()
{
    emit modified(shared_from_this());
}
