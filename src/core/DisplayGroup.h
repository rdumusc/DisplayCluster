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

#ifndef DISPLAY_GROUP_H
#define DISPLAY_GROUP_H

#include "types.h"

#include <boost/serialization/access.hpp>
#include <boost/enable_shared_from_this.hpp>

#include <QObject>
#include <QUuid>

/**
 * A collection of ContentWindows.
 *
 * Can be serialized and distributed to the Wall applications.
 */
class DisplayGroup : public QObject,
        public boost::enable_shared_from_this<DisplayGroup>
{
    Q_OBJECT

public:
    /** Constructor */
    DisplayGroup();

    /** Destructor */
    virtual ~DisplayGroup();

    /** Add a content window. */
    void addContentWindow( ContentWindowPtr contentWindow );

    /** Get the background content window. */
    ContentWindowPtr getBackgroundContentWindow() const;

    /**
     * Is the DisplayGroup empty.
     * @return true if the DisplayGroup has no ContentWindow, false otherwise.
     */
    bool isEmpty() const;

    /**
     * Get the active window.
     * @return A shared pointer to the active window. Can be empty if there is
     *         no Window available.
     * @see isEmpty().
     */
    ContentWindowPtr getActiveWindow() const;

    /** Get all windows. */
    ContentWindowPtrs getContentWindows() const;

    /** Get a single window by its id */
    ContentWindowPtr getContentWindow( const QUuid& id ) const;

    /**
     * Replace the content windows.
     * @param contentWindows The list of windows to set.
     */
    void setContentWindows( ContentWindowPtrs contentWindows );

public slots:
    /** Clear all ContentWindows */
    void clear();

    /**
     * Set the background content.
     * @param content The content to set.
     *                A null pointer removes the current background.
     */
    void setBackgroundContent( ContentPtr content );

    /** Remove a content window. */
    void removeContentWindow( ContentWindowPtr contentWindow );

    /** Move a content window to the front. */
    void moveContentWindowToFront( ContentWindowPtr contentWindow );

signals:
    /** Emitted whenever the DisplayGroup is modified */
    void modified( DisplayGroupPtr displayGroup );

    /** Emitted when a content window is added. */
    void contentWindowAdded( ContentWindowPtr contentWindow );

    /** Emitted when a content window is removed. */
    void contentWindowRemoved( ContentWindowPtr contentWindow );

    /** Emitted when a content window is moved to the front. */
    void contentWindowMovedToFront( ContentWindowPtr contentWindow );

private slots:
    void sendDisplayGroup();

private:
    friend class boost::serialization::access;

    template< class Archive >
    void serialize( Archive & ar, const unsigned int )
    {
        ar & contentWindows_;
        ar & backgroundContent_;
    }

    void watchChanges( ContentWindowPtr contentWindow );

    ContentWindowPtrs contentWindows_;
    ContentWindowPtr backgroundContent_;
};

#endif
