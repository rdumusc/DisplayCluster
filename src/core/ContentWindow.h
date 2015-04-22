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

#include <deflect/Event.h>
#include "types.h"

#include "serializationHelpers.h"
#include "Coordinates.h"
#include "Content.h" // needed for serialization

#include <QObject>
#include <QUuid>
#include <QRectF>

#ifndef Q_MOC_RUN
// https://bugreports.qt.nokia.com/browse/QTBUG-22829: When Qt moc runs on CGAL
// files, do not process <boost/type_traits/has_operator.hpp>
#  include <boost/serialization/shared_ptr.hpp>
#endif

class ContentInteractionDelegate;

/**
 * A window for displaying Content on the Wall.
 *
 * Can be serialized and distributed to the Wall applications.
 */
class ContentWindow : public Coordinates
{
    Q_OBJECT
    Q_PROPERTY( QUuid id READ getID )
    Q_PROPERTY( WindowState state READ getState WRITE setState NOTIFY stateChanged )
    Q_PROPERTY( WindowBorder border READ getBorder WRITE setBorder NOTIFY borderChanged )
    Q_PROPERTY( QString label READ getLabel NOTIFY labelChanged )
    Q_PROPERTY( qreal controlsOpacity READ getControlsOpacity WRITE setControlsOpacity NOTIFY controlsOpacityChanged )
    Q_PROPERTY( Content* content READ getContentPtr CONSTANT )
    Q_PROPERTY( QRectF zoomRect READ getZoomRect CONSTANT )

public:
    /** The current active window border used for resizing */
    enum WindowBorder
    {
        TOP_LEFT,
        TOP,
        TOP_RIGHT,
        RIGHT,
        BOTTOM_RIGHT,
        BOTTOM,
        BOTTOM_LEFT,
        LEFT,
        NOBORDER
    };
    Q_ENUMS( WindowBorder )

    /** The possible states of a window. */
    enum WindowState
    {
        NONE,       // not selected, interaction modifies position/size
        SELECTED,   // selected, interaction goes to ContentInteractionDelegate
        MOVING,     // the window is being moved
        RESIZING,   // the window is being resized
        HIDDEN      // the window is hidden (invisible, not interacting)
    };
    Q_ENUMS( WindowState )

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

    /** Get the content from QML. */
    Content* getContentPtr() const;

    /** Get the content. */
    ContentPtr getContent() const;

    /** Set the content, replacing the existing one. @note Rank0 only. */
    void setContent( ContentPtr content );


    /** Set the coordinates in pixel units. */
    void setCoordinates( const QRectF& coordinates );

    /** Get the zoom rectangle in normalized coordinates, [0,0,1,1] default */
    const QRectF& getZoomRect() const;

    /** Set the zoom rectangle in normalized coordinates. */
    void setZoomRect( const QRectF& zoomRect );

    /** @return the current active resize border. */
    ContentWindow::WindowBorder getBorder() const;

    /** Get the current state. */
    ContentWindow::WindowState getState() const;

    /** Set the current active resize border. */
    void setBorder( const ContentWindow::WindowBorder border );

    /** Set the current state. */
    void setState( const ContentWindow::WindowState state );

    /** Toggle the state (selected / unselected). */
    Q_INVOKABLE void toggleSelectedState();

    /** Check if selected. */
    bool isSelected() const;

    /** Check if moving. */
    bool isMoving() const;

    /** Check if resizing. */
    bool isResizing() const;

    /** Check if hidden. */
    bool isHidden() const;


    /** Register an object to receive this window's Events. */
    bool registerEventReceiver( deflect::EventReceiver* receiver );

    /** Does this window already have registered Event receiver(s) */
    bool hasEventReceivers() const;

    /** Used by InteractionDelegate to emit notify( Event ). */
    void dispatchEvent( const deflect::Event event );

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

    /** Get the label for the window */
    QString getLabel() const;

    /** Get the opacity of the window control buttons. */
    qreal getControlsOpacity() const;

    /** Set the opacity of the window control buttons. */
    void setControlsOpacity( qreal value );

    /** Set the maximum factor for zoom and resize; value times content size */
    static void setMaxContentScale( qreal value );

    /** @return the maxium scale factor for zoom and resize */
    static qreal getMaxContentScale();

signals:
    /** Emitted when the Content signals that it has been modified. */
    void contentModified();

    /**
     * Emitted whenever this object is modified.
     * Used by DisplayGroup on Rank0 to distibute changes to the other ranks.
     */
    void modified();

    /** @internal Notify registered EventReceivers that an Event occured. */
    void notify( deflect::Event event );

    /** @name QProperty notifiers */
    //@{
    void borderChanged();
    void stateChanged();
    void labelChanged();
    void controlsOpacityChanged();
    //@}

private:
    friend class boost::serialization::access;

    /** No-argument constructor required for serialization. */
    ContentWindow();

    /** Serialize for sending to Wall applications. */
    template< class Archive >
    void serialize( Archive & ar, const unsigned int )
    {
        ar & uuid_;
        ar & content_;
        ar & coordinates_;
        ar & zoomRect_;
        ar & windowBorder_;
        ar & windowState_;
        ar & controlsOpacity_;
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
        QPointF zoomCenter = zoomRect_.center();
        qreal zoom = 1.0/zoomRect_.width();
        ar & boost::serialization::make_nvp( "centerX", zoomCenter.rx( ));
        ar & boost::serialization::make_nvp( "centerY", zoomCenter.ry( ));
        ar & boost::serialization::make_nvp( "zoom", zoom );
        QRectF zoomRect;
        zoomRect.setSize( QSizeF( 1.0/zoom, 1.0/zoom ));
        zoomRect.moveCenter( zoomCenter );
        setZoomRect( zoomRect );
        if( version < 1 )
        {
            int controlState = 0;
            ar & boost::serialization::make_nvp( "controlState", controlState );
        }
        ar & boost::serialization::make_nvp( "windowState", windowState_ );
    }

    /** Loading from xml. */
    void serialize_for_xml( boost::archive::xml_iarchive& ar,
                            const unsigned int version )
    {
        serialize_members_xml( ar, version );
        // The InteractionDelegate is not serialized and must be recreated
        createInteractionDelegate();
    }

    /** Saving to xml. */
    void serialize_for_xml( boost::archive::xml_oarchive& ar,
                            const unsigned int version )
    {
        serialize_members_xml( ar, version );
    }

    void createInteractionDelegate();
    void sendSizeChangedEvent();

    QUuid uuid_;
    ContentPtr content_;

    // coordinates in pixels, relative to the parent DisplayGroup
    QRectF coordinatesBackup_;

    // zooming
    QRectF zoomRect_;

    ContentWindow::WindowBorder windowBorder_;
    ContentWindow::WindowState windowState_;
    qreal controlsOpacity_;

    unsigned int eventReceiversCount_;

    boost::scoped_ptr< ContentInteractionDelegate > interactionDelegate_;

    static qreal maxContentScale_;
};

BOOST_CLASS_VERSION( ContentWindow, 2 )

DECLARE_SERIALIZE_FOR_XML( ContentWindow )

#endif
