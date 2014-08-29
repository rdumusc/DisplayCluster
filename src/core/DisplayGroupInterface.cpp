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

#include "DisplayGroupInterface.h"

#include "DisplayGroup.h"
#include "ContentWindow.h"

DisplayGroupInterface::DisplayGroupInterface(DisplayGroupPtr displayGroup)
    : displayGroup_(displayGroup)
{
    // copy all members from displayGroup
    if(displayGroup)
        contentWindows_ = displayGroup->contentWindows_;

    // connect signals from this to slots on the DisplayGroup
    // use queued connections for thread-safety
    connect(this, SIGNAL(contentWindowAdded(ContentWindowPtr, DisplayGroupInterface *)), displayGroup.get(), SLOT(addContentWindow(ContentWindowPtr, DisplayGroupInterface *)), Qt::QueuedConnection);
    connect(this, SIGNAL(contentWindowRemoved(ContentWindowPtr, DisplayGroupInterface *)), displayGroup.get(), SLOT(removeContentWindow(ContentWindowPtr, DisplayGroupInterface *)), Qt::QueuedConnection);
    connect(this, SIGNAL(contentWindowMovedToFront(ContentWindowPtr, DisplayGroupInterface *)), displayGroup.get(), SLOT(moveContentWindowToFront(ContentWindowPtr, DisplayGroupInterface *)), Qt::QueuedConnection);

    // connect signals on the DisplayGroup to slots on this
    // use queued connections for thread-safety
    connect(displayGroup.get(), SIGNAL(contentWindowAdded(ContentWindowPtr, DisplayGroupInterface *)), this, SLOT(addContentWindow(ContentWindowPtr, DisplayGroupInterface *)), Qt::QueuedConnection);
    connect(displayGroup.get(), SIGNAL(contentWindowRemoved(ContentWindowPtr, DisplayGroupInterface *)), this, SLOT(removeContentWindow(ContentWindowPtr, DisplayGroupInterface *)), Qt::QueuedConnection);
    connect(displayGroup.get(), SIGNAL(contentWindowMovedToFront(ContentWindowPtr, DisplayGroupInterface *)), this, SLOT(moveContentWindowToFront(ContentWindowPtr, DisplayGroupInterface *)), Qt::QueuedConnection);

    // destruction
    connect(displayGroup.get(), SIGNAL(destroyed(QObject *)), this, SLOT(deleteLater()));
}

DisplayGroupPtr DisplayGroupInterface::getDisplayGroup()
{
    return displayGroup_.lock();
}

ContentWindowPtrs DisplayGroupInterface::getContentWindows()
{
    return contentWindows_;
}

ContentWindowPtr DisplayGroupInterface::getContentWindow(const QUuid& id) const
{
    for(size_t i=0; i<contentWindows_.size(); ++i)
    {
        if( contentWindows_[i]->getID() == id )
            return contentWindows_[i];
    }

    return ContentWindowPtr();
}

void DisplayGroupInterface::setContentWindows(ContentWindowPtrs contentWindows)
{
    // remove existing content window managers
    clear();

    // add new content window managers
    for(unsigned int i=0; i<contentWindows.size(); i++)
        addContentWindow(contentWindows[i]);
}

void DisplayGroupInterface::clear()
{
    while(!contentWindows_.empty())
        removeContentWindow(contentWindows_[0]);
}

void DisplayGroupInterface::addContentWindow(ContentWindowPtr contentWindow, DisplayGroupInterface * source)
{
    if(source == this)
    {
        return;
    }

    contentWindows_.push_back(contentWindow);

    if(source == NULL || dynamic_cast<DisplayGroup *>(this) != NULL)
    {
        if(source == NULL)
        {
            source = this;
        }

        emit(contentWindowAdded(contentWindow, source));
    }
}

void DisplayGroupInterface::removeContentWindow(ContentWindowPtr contentWindow, DisplayGroupInterface * source)
{
    if(source == this)
    {
        return;
    }

    Event closeEvent;
    closeEvent.type = Event::EVT_CLOSE;
    contentWindow->setEvent( closeEvent );

    // find vector entry for content window manager
    ContentWindowPtrs::iterator it = find(contentWindows_.begin(),
                                                 contentWindows_.end(), contentWindow);

    if(it != contentWindows_.end())
    {
        // we found the entry
        // now, remove it
        contentWindows_.erase(it);
    }

    if(source == NULL || dynamic_cast<DisplayGroup *>(this) != NULL)
    {
        if(source == NULL)
        {
            source = this;
        }

        emit(contentWindowRemoved(contentWindow, source));
    }
}

void DisplayGroupInterface::moveContentWindowToFront(ContentWindowPtr contentWindow, DisplayGroupInterface * source)
{
    if(source == this)
    {
        return;
    }

    // find vector entry for content window manager
    ContentWindowPtrs::iterator it;

    it = find(contentWindows_.begin(), contentWindows_.end(), contentWindow);

    if(it != contentWindows_.end())
    {
        // we found the entry
        // now, move it to end of the list (last item rendered is on top)
        contentWindows_.erase(it);
        contentWindows_.push_back(contentWindow);
    }

    if(source == NULL || dynamic_cast<DisplayGroup *>(this) != NULL)
    {
        if(source == NULL)
        {
            source = this;
        }

        emit(contentWindowMovedToFront(contentWindow, source));
    }
}
