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
#include "Coordinates.h"
#include "ContentWindow.h"

#include <boost/serialization/access.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/set.hpp>
#include <boost/enable_shared_from_this.hpp>

#include <QObject>
#include <QUuid>
#include <QRectF>

/**
 * The different versions of the xml serialized display group.
 */
enum DisplayGroupVersion
{
    INVALID_DISPLAYGROUP_VERSION = -1,
    FIRST_DISPLAYGROUP_VERSION = 0
};

/**
 * A collection of ContentWindows.
 *
 * Can be serialized and distributed to the Wall applications.
 */
class DisplayGroup : public Coordinates,
        public boost::enable_shared_from_this<DisplayGroup>
{
    Q_OBJECT
    Q_PROPERTY( bool showWindowTitles READ getShowWindowTitles
                WRITE setShowWindowTitles NOTIFY showWindowTitlesChanged )
    Q_PROPERTY( bool hasFocusedWindows READ hasFocusedWindows
                NOTIFY hasFocusedWindowsChanged )

public:
    /** Constructor */
    DisplayGroup( const QSizeF& size );

    /** Destructor */
    virtual ~DisplayGroup();

    /** Add a content window. */
    void addContentWindow( ContentWindowPtr contentWindow );

    /** @return true if window titles are visible. */
    bool getShowWindowTitles() const;

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
    const ContentWindowPtrs& getContentWindows() const;

    /** Get a single window by its id. */
    ContentWindowPtr getContentWindow( const QUuid& id ) const;

    /**
     * Replace the content windows.
     * @param contentWindows The list of windows to set.
     */
    void setContentWindows( ContentWindowPtrs contentWindows );

    /** Assignment operator. */
    DisplayGroup& operator=( const DisplayGroup& displayGroup );


    /** Are there focused windows. */
    bool hasFocusedWindows() const;

    /** Focus a window. */
    Q_INVOKABLE void focus( const QUuid& id );

    /** Unfocus a window. */
    Q_INVOKABLE void unfocus( const QUuid& id );

public slots:
    /** Clear all ContentWindows. */
    void clear();

    /** Enable/Disable the visibility of window titles. */
    void setShowWindowTitles( bool set );

    /** Remove a content window. */
    void removeContentWindow( const QUuid id );

    /** Remove a content window. */
    void removeContentWindow( ContentWindowPtr contentWindow );

    /** Remove a content window. */
    void moveContentWindowToFront( const QUuid id );

    /** Move a content window to the front. */
    void moveContentWindowToFront( ContentWindowPtr contentWindow );

signals:
    /** @name QProperty notifiers */
    //@{
    void showWindowTitlesChanged( bool set );
    //@}

    /** Emitted whenever the DisplayGroup is modified */
    void modified( DisplayGroupPtr displayGroup );

    /** Emitted when a content window is added. */
    void contentWindowAdded( ContentWindowPtr contentWindow );

    /** Emitted when a content window is removed. */
    void contentWindowRemoved( ContentWindowPtr contentWindow );

    /** Emitted when a content window is moved to the front. */
    void contentWindowMovedToFront( ContentWindowPtr contentWindow );

    /** Notifier for the hasFocusedWindows property. */
    void hasFocusedWindowsChanged();

private slots:
    void sendDisplayGroup();

private:
    friend class boost::serialization::access;

    /** No-argument constructor required for serialization. */
    DisplayGroup();

    template< class Archive >
    void serialize( Archive & ar, const unsigned int )
    {
        ar & showWindowTitles_;
        ar & contentWindows_;
        ar & focusedWindows_;
        ar & coordinates_;
    }

    /** Serialize for saving to an xml file */
    template< class Archive >
    void serialize_members_xml( Archive & ar, const unsigned int )
    {
        ar & boost::serialization::make_nvp( "showWindowTitles",
                                             showWindowTitles_ );
        ar & boost::serialization::make_nvp( "contentWindows",
                                             contentWindows_ );
        ar & boost::serialization::make_nvp( "coordinates", coordinates_ );
    }

    /** Loading from xml. */
    void serialize_for_xml( boost::archive::xml_iarchive& ar,
                            const unsigned int version)
    {
        serialize_members_xml( ar, version );
    }

    /** Saving to xml. */
    void serialize_for_xml( boost::archive::xml_oarchive& ar,
                            const unsigned int version )
    {
        serialize_members_xml( ar, version );
    }

    void watchChanges( ContentWindowPtr contentWindow );

    bool showWindowTitles_;
    ContentWindowPtrs contentWindows_;
    std::set<ContentWindowPtr> focusedWindows_;
};

BOOST_CLASS_VERSION( DisplayGroup, FIRST_DISPLAYGROUP_VERSION )
DECLARE_SERIALIZE_FOR_XML( DisplayGroup )

#endif
