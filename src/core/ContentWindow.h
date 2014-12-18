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

#ifndef CONTENT_WINDOW_H
#define CONTENT_WINDOW_H

#include "Event.h"
#include "types.h"

#include "serializationHelpers.h"
#include "Content.h" // needed for serialization

#include <QObject>
#include <QUuid>
#include <QRectF>

#ifndef Q_MOC_RUN
// https://bugreports.qt.nokia.com/browse/QTBUG-22829: When Qt moc runs on CGAL
// files, do not process <boost/type_traits/has_operator.hpp>
#  include <boost/serialization/shared_ptr.hpp>
#  include <boost/serialization/split_member.hpp>
#endif

class EventReceiver;
class ContentInteractionDelegate;

/**
 * A window for displaying Content on the Wall.
 *
 * Can be serialized and distributed to the Wall applications.
 */
class ContentWindow : public QObject
{
    Q_OBJECT

public:
    /** The possible states of a window. */
    enum WindowState
    {
        NONE,       // not selected, interaction modifies position/size
        SELECTED,   // selected, interaction goes to ContentInteractionDelegate
        MOVING,     // the window is being moved
        RESIZING,   // the window is being resized
        HIDDEN      // the window is hidden (invisible, not interacting)
    };

    /**
     * Create a new window.
     * @param content The Content to be displayed.
     * @note Rank0 only.
     */
    ContentWindow( ContentPtr content );

    /** Destructor. */
    ~ContentWindow();

    /** @return the unique identifier for this window. */
    const QUuid& getID() const;


    /** Get the content. */
    ContentPtr getContent() const;

    /** Set the content, replacing the existing one. @note Rank0 only. */
    void setContent( ContentPtr content );


    /** Get the coordiates in pixel units. */
    const QRectF& getCoordinates() const;

    /** Set the coordinates in pixel units. */
    void setCoordinates( const QRectF& coordinates );


    /** Get the zoom factor [1.0; 16.0]. */
    qreal getZoom() const;

    /** Set the zoom factor [1.0; 16.0]. */
    void setZoom( const qreal zoom );

    /** Get the zoom center in normalized coordinates. */
    const QPointF& getZoomCenter() const;

    /** Set the zoom center in normalized coordinates. */
    void setZoomCenter( const QPointF& zoomCenter );


    /** Get the current state. */
    ContentWindow::WindowState getState() const;

    /** Set the current state. */
    void setState( const ContentWindow::WindowState state );

    /** Toggle the state (selected / unselected). */
    void toggleSelectedState();

    /** Check if selected. */
    bool isSelected() const;

    /** Check if moving. */
    bool isMoving() const;

    /** Check if resizing. */
    bool isResizing() const;

    /** Check if hidden. */
    bool isHidden() const;


    /** Register an object to receive this window's Events. */
    bool registerEventReceiver( EventReceiver* receiver );

    /** Does this window already have registered Event receiver(s) */
    bool hasEventReceivers() const;

    /** Used by InteractionDelegate to emit notify( Event ). */
    void dispatchEvent( const Event event );

    /**
     * Get the interaction delegate.
     * @note Rank0 only.
     */
    ContentInteractionDelegate& getInteractionDelegate();


    /** Backup the current coordinates. */
    void backupCoordinates();

    /** Check if there are coordinates which can be restored. */
    bool hasBackupCoordinates() const;

    /** Restore and clear the backed-up coordinates. */
    void restoreCoordinates();

signals:
    /** Emitted when the Content signals that it has been modified. */
    void contentModified();

    /** Emitted just before the coordinates are going to change. */
    void coordinatesAboutToChange();

    /**
     * Emitted whenever this object is modified.
     * Used by DisplayGroup on Rank0 to distibute changes to the other ranks.
     */
    void modified();

    /** Notify registered EventReceivers that an Event occured. */
    void notify( Event event );

private:
    friend class boost::serialization::access;

    /** No-argument constructor required for serialization. */
    ContentWindow();

    /** Serialize for sending to Wall applications. */
    template< class Archive >
    void serialize( Archive & ar, const unsigned int )
    {
        ar & content_;
        ar & coordinates_;
        ar & zoom_;
        ar & zoomCenter_;
        ar & windowState_;
    }

    /** Serialize for saving to an xml file */
    template< class Archive >
    void serialize_members_xml( Archive & ar, const unsigned int version )
    {
        ar & boost::serialization::make_nvp( "content", content_ );
        if( version < 1 )
        {
            int contentWidth = 0, contentHeight = 0;
            ar & boost::serialization::make_nvp( "contentWidth", contentWidth );
            ar & boost::serialization::make_nvp( "contentHeight", contentHeight );
        }
        ar & boost::serialization::make_nvp( "coordinates", coordinates_ );
        ar & boost::serialization::make_nvp( "coordinatesBackup", coordinatesBackup_ );
        ar & boost::serialization::make_nvp( "centerX", zoomCenter_.rx() );
        ar & boost::serialization::make_nvp( "centerY", zoomCenter_.ry() );
        ar & boost::serialization::make_nvp( "zoom", zoom_ );
        if( version < 1 )
        {
            int controlState = 0;
            ar & boost::serialization::make_nvp( "controlState", controlState );
        }
        ar & boost::serialization::make_nvp( "windowState", windowState_ );
    }

    /** Saving to xml. */
    void serialize_for_xml( boost::archive::xml_iarchive& ar,
                            const unsigned int version )
    {
        serialize_members_xml( ar, version );
    }

    /** Loading from xml. */
    void serialize_for_xml( boost::archive::xml_oarchive& ar,
                            const unsigned int version )
    {
        serialize_members_xml( ar, version );
        // The InteractionDelegate is not serialized and must be recreated
        createInteractionDelegate();
    }

    void createInteractionDelegate();
    void sendSizeChangedEvent();
    void constrainZoomCenter();

    const QUuid uuid_;
    ContentPtr content_;

    // coordinates in pixels, relative to the parent DisplayGroup
    QRectF coordinates_;
    QRectF coordinatesBackup_;

    // panning and zooming
    QPointF zoomCenter_;
    qreal zoom_;

    ContentWindow::WindowState windowState_;

    unsigned int eventReceiversCount_;

    boost::scoped_ptr< ContentInteractionDelegate > interactionDelegate_;
};

BOOST_CLASS_VERSION( ContentWindow, 2 )

DECLARE_SERIALIZE_FOR_XML( ContentWindow )

#endif
