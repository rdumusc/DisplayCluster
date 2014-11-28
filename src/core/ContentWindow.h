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
#  include <boost/shared_ptr.hpp>
#  include <boost/weak_ptr.hpp>
#  include <boost/date_time/posix_time/posix_time.hpp>
#  include <boost/serialization/shared_ptr.hpp>
#  include <boost/date_time/posix_time/time_serialize.hpp>
#endif

class EventReceiver;

enum ControlState
{
    STATE_PAUSED = 1 << 0,
    STATE_LOOP   = 1 << 1
};

class ContentInteractionDelegate;

/**
 * A window for placing a Content on the Wall.
 *
 * Can be serialized and distributed to the Wall applications.
 */
class ContentWindow : public QObject
{
    Q_OBJECT

public:
    /** The possible states of the window. */
    enum WindowState
    {
        UNSELECTED, // not selected, interaction modifies position/size
        SELECTED    // selected, interaction goes to ContentInteractionDelegate
    };

    /** No-argument constructor required for serialization. */
    ContentWindow();

    /**
     * Create a new window.
     * @param content The Content for this window.
     * @note Rank0 only.
     */
    ContentWindow( ContentPtr content );

    /** Destructor. */
    virtual ~ContentWindow();

    /** @return the unique identifier for this window. */
    const QUuid& getID() const;


    /** Get the content. */
    ContentPtr getContent() const;

    /** Set the content, replacing the existing one. @note Rank0 only. */
    void setContent( ContentPtr content );


    /** Get the window coordiates. */
    const QRectF& getCoordinates() const;

    /** Set the window coordinates. */
    void setCoordinates( const QRectF& coordinates );

    /** Set the window position. */
    void setPosition( const QPointF& position );

    /** Set the window dimensions. */
    void setSize( const QSizeF& size );

    /** Scale the window by the given factor around its center. */
    void scaleSize( const double factor );


    /** Get the zoom factor [1;inf]. */
    qreal getZoom() const;

    /** Get the zoom factor [1;inf]. */
    void setZoom( const qreal zoom );

    /** Get the zoom center in normalized coordinates. */
    const QPointF& getZoomCenter() const;

    /** Set the zoom center in normalized coordinates. */
    void setZoomCenter( const QPointF& zoomCenter );


    /** Get the control state. */
    ControlState getControlState() const;

    /** Set the control state. */
    void setControlState( const ControlState state );


    /** Get the window state. */
    ContentWindow::WindowState getWindowState() const;

    /** Set the window state. */
    void setWindowState( const ContentWindow::WindowState state );

    /** Toggle the window state. */
    void toggleWindowState();

    /** Is the window selected. */
    bool selected() const;


    /** Register an object to receive this window's Events. */
    bool registerEventReceiver( EventReceiver* receiver );

    /** Does this window already have registered Event receiver(s) */
    bool hasEventReceivers() const;

    /** Used by InteractionDelegate to emit eventChanged(). */
    void dispatchEvent( const Event event );

    /**
     * Get the interaction delegate.
     * @see createInteractionDelegate()
     * @note Rank0 only.
     */
    ContentInteractionDelegate& getInteractionDelegate() const;

    /**
     * Create a delegate to handle user interaction through dc::Events.
     * The type of delegate created depends on the ContentType.
     * @note Rank0 only.
     */
    void createInteractionDelegate();

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
    void eventChanged( Event event );

private:
    friend class boost::serialization::access;

    /** Serialize for sending to Wall applications. */
    template< class Archive >
    void serialize( Archive & ar, const unsigned int )
    {
        ar & content_;
        ar & coordinates_;
        ar & zoom_;
        ar & zoomCenter_;
        ar & controlState_;
        ar & windowState_;
    }

    /** Serialize for saving to an xml file */
    template< class Archive >
    void serialize_for_xml( Archive & ar, const unsigned int )
    {
        int contentWidth = 0, contentHeight = 0; // For reading legacy archives
        ar & boost::serialization::make_nvp( "content", content_ );
        ar & boost::serialization::make_nvp( "contentWidth", contentWidth );
        ar & boost::serialization::make_nvp( "contentHeight", contentHeight );
        ar & boost::serialization::make_nvp( "coordinates", coordinates_ );
        ar & boost::serialization::make_nvp( "coordinatesBackup", coordinatesBackup_ );
        ar & boost::serialization::make_nvp( "centerX", zoomCenter_.rx() );
        ar & boost::serialization::make_nvp( "centerY", zoomCenter_.ry() );
        ar & boost::serialization::make_nvp( "zoom", zoom_ );
        ar & boost::serialization::make_nvp( "controlState", controlState_ );
        ar & boost::serialization::make_nvp( "windowState", windowState_ );
    }

    const QUuid uuid_;
    ContentPtr content_;

    friend class ContentWindowController;

    // coordinates in pixels, relative to the parent DisplayGroup
    QRectF coordinates_;
    QRectF coordinatesBackup_;

    // panning and zooming
    QPointF zoomCenter_;
    qreal zoom_;

    ContentWindow::WindowState windowState_;
    ControlState controlState_;
    unsigned int eventReceiversCount_;

    boost::scoped_ptr< ContentInteractionDelegate > interactionDelegate_;

    void setEventToNewDimensions();
    void constrainZoomCenter();
};

DECLARE_SERIALIZE_FOR_XML( ContentWindow )

#endif
