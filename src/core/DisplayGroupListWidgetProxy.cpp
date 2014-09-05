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

#include "DisplayGroupListWidgetProxy.h"

#include "ContentWindow.h"
#include "Content.h"
#include "ContentWindowListWidgetItem.h"

#include <QListWidget>

DisplayGroupListWidgetProxy::DisplayGroupListWidgetProxy(DisplayGroupPtr displayGroup)
    : DisplayGroupInterface(displayGroup)
    , listWidget_(new QListWidget())
{
    connect(listWidget_, SIGNAL(itemClicked(QListWidgetItem * )), this, SLOT(moveListWidgetItemToFront(QListWidgetItem *)));
}

DisplayGroupListWidgetProxy::~DisplayGroupListWidgetProxy()
{
    delete listWidget_;
}

QListWidget* DisplayGroupListWidgetProxy::getListWidget()
{
    return listWidget_;
}

void DisplayGroupListWidgetProxy::addContentWindow(ContentWindowPtr contentWindow, DisplayGroupInterface * source)
{
    DisplayGroupInterface::addContentWindow(contentWindow, source);

    if(source != this)
    {
        // for now, just clear and refresh the entire list, since this is just a read-only interface
        // later this could be modeled after DisplayGroupGraphicsViewProxy if we want to expand the interface
        refreshListWidget();
    }
}

void DisplayGroupListWidgetProxy::removeContentWindow(ContentWindowPtr contentWindow, DisplayGroupInterface * source)
{
    DisplayGroupInterface::removeContentWindow(contentWindow, source);

    if(source != this)
    {
        refreshListWidget();
    }
}

void DisplayGroupListWidgetProxy::moveContentWindowToFront(ContentWindowPtr contentWindow, DisplayGroupInterface * source)
{
    DisplayGroupInterface::moveContentWindowToFront(contentWindow, source);

    if(source != this)
    {
        refreshListWidget();
    }
}

void DisplayGroupListWidgetProxy::moveListWidgetItemToFront(QListWidgetItem* item)
{
    ContentWindowListWidgetItem * contentWindowItem = dynamic_cast<ContentWindowListWidgetItem *>(item);

    if(contentWindowItem)
        contentWindowItem->moveToFront();
}

void DisplayGroupListWidgetProxy::refreshListWidget()
{
    listWidget_->clear();

    for(unsigned int i=0; i<contentWindows_.size(); i++)
    {
        // add to list view
        ContentWindowListWidgetItem * newItem = new ContentWindowListWidgetItem(contentWindows_[i]);
        newItem->setText(contentWindows_[i]->getContent()->getURI());

        listWidget_->insertItem(0, newItem);
    }
}
