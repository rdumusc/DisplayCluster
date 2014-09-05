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

#ifndef DISPLAY_GROUP_INTERFACE_H
#define DISPLAY_GROUP_INTERFACE_H

#include "types.h"

#include <QObject>
#include <QUuid>
#include <boost/weak_ptr.hpp>

class DisplayGroupInterface : public QObject
{
    Q_OBJECT

public:

    DisplayGroupInterface() { }
    DisplayGroupInterface(DisplayGroupPtr displayGroup);

    DisplayGroupPtr getDisplayGroup();

    ContentWindowPtrs getContentWindows();
    ContentWindowPtr getContentWindow(const QUuid& id) const;

    // remove all current ContentWindows and add the vector of provided ContentWindows
    void setContentWindows(ContentWindowPtrs contentWindows);

public slots:
    /** Clear all ContentWindows */
    void clear();

    // these methods set the local copies of the state variables if source != this
    // they will emit signals if source == 0 or if this is a DisplayGroup object
    // the source argument should not be provided by users -- only by these functions
    virtual void addContentWindow(ContentWindowPtr contentWindow, DisplayGroupInterface* source = 0);
    virtual void removeContentWindow(ContentWindowPtr contentWindow, DisplayGroupInterface* source = 0);
    virtual void moveContentWindowToFront(ContentWindowPtr contentWindow, DisplayGroupInterface* source = 0);

signals:

    // emitting these signals will trigger updates on the corresponding DisplayGroup
    // as well as all other DisplayGroupInterfaces to that DisplayGroup
    void contentWindowAdded(ContentWindowPtr contentWindow, DisplayGroupInterface* source = 0);
    void contentWindowRemoved(ContentWindowPtr contentWindow, DisplayGroupInterface* source = 0);
    void contentWindowMovedToFront(ContentWindowPtr contentWindow, DisplayGroupInterface* source = 0);

protected:

    // optional: reference to DisplayGroup for non-DisplayGroup objects
    boost::weak_ptr<DisplayGroup> displayGroup_;

    // vector of all of its content window managers
    ContentWindowPtrs contentWindows_;
};

#endif
