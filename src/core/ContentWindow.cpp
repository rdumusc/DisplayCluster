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

#include "ContentWindow.h"

#include "DisplayGroup.h"
#include "ContentInteractionDelegate.h"
#include "EventReceiver.h"

#include "config.h"
#include "log.h"

#include "PixelStreamInteractionDelegate.h"
#include "ZoomInteractionDelegate.h"
#if ENABLE_PDF_SUPPORT
#  include "PDFInteractionDelegate.h"
#endif

IMPLEMENT_SERIALIZE_FOR_XML( ContentWindow )

ContentWindow::ContentWindow()
    : uuid_( QUuid::createUuid( ))
    , zoomCenter_( 0.5f, 0.5f )
    , zoom_( 1.0 )
    , windowState_( UNSELECTED )
    , controlState_( STATE_LOOP )
    , eventReceiversCount_( 0 )
{
}

ContentWindow::ContentWindow( ContentPtr content )
    : uuid_( QUuid::createUuid( ))
    , zoomCenter_( 0.5f, 0.5f )
    , zoom_( 1.0 )
    , windowState_( UNSELECTED )
    , controlState_( STATE_LOOP )
    , eventReceiversCount_( 0 )
{
    assert( content );
    setContent( content );
    coordinates_.setSize( content_->getDimensions( ));
    coordinatesBackup_ = coordinates_;
}

ContentWindow::~ContentWindow()
{
}

const QUuid& ContentWindow::getID() const
{
    return uuid_;
}

ContentPtr ContentWindow::getContent() const
{
    return content_;
}

void ContentWindow::setContent( ContentPtr content )
{
    assert( content );

    if( content_ )
        content_->disconnect( this, SIGNAL( contentModified( )));

    content_ = content;

    connect( content_.get(), SIGNAL( modified( )), SIGNAL( contentModified( )));

    createInteractionDelegate();
}

const QRectF& ContentWindow::getCoordinates() const
{
    return coordinates_;
}

void ContentWindow::setCoordinates( const QRectF& coordinates )
{
    emit coordinatesAboutToChange();

    coordinates_ = coordinates;

    emit modified();

    setEventToNewDimensions();
}

void ContentWindow::setPosition( const QPointF& position )
{
    emit coordinatesAboutToChange();

    coordinates_.moveTo( position );

    emit modified();
}

void ContentWindow::setSize( const QSizeF& size )
{
    emit coordinatesAboutToChange();

    coordinates_.setSize( size );

    emit modified();

    setEventToNewDimensions();
}

void ContentWindow::scaleSize( const double factor )
{
    if( factor < 0. )
        return;

    QRectF coordinates;
    coordinates.setX( coordinates_.x() - ( factor - 1. ) * coordinates_.width() / 2. );
    coordinates.setY( coordinates_.y() - ( factor - 1. ) * coordinates_.height() / 2. );
    coordinates.setWidth( coordinates_.width() * factor );
    coordinates.setHeight( coordinates_.height() * factor );

    setCoordinates( coordinates );
}

qreal ContentWindow::getZoom() const
{
    return zoom_;
}

void ContentWindow::setZoom( const qreal zoom )
{
    zoom_ = std::max( zoom, 1.0 );

    constrainZoomCenter();

    emit modified();
}

const QPointF& ContentWindow::getZoomCenter() const
{
    return zoomCenter_;
}

void ContentWindow::setZoomCenter( const QPointF& zoomCenter )
{
    zoomCenter_ = zoomCenter;

    constrainZoomCenter();

    emit modified();
}

ControlState ContentWindow::getControlState() const
{
    return controlState_;
}

void ContentWindow::setControlState( const ControlState state )
{
    controlState_ = state;
}

ContentWindow::WindowState ContentWindow::getWindowState() const
{
    return windowState_;
}

void ContentWindow::setWindowState( const ContentWindow::WindowState state )
{
    windowState_ = state;

    emit modified();
}

void ContentWindow::toggleWindowState()
{
    setWindowState( windowState_ == UNSELECTED ? SELECTED : UNSELECTED );
}

bool ContentWindow::selected() const
{
    return windowState_ == SELECTED;
}

bool ContentWindow::registerEventReceiver( EventReceiver* receiver )
{
    const bool success = connect( this, SIGNAL( eventChanged( Event )),
                                  receiver, SLOT( processEvent( Event )));
    if ( success )
        ++eventReceiversCount_;

    return success;
}

bool ContentWindow::hasEventReceivers() const
{
    return eventReceiversCount_ > 0;
}

void ContentWindow::dispatchEvent( const Event event_ )
{
    emit eventChanged( event_ );
}

ContentInteractionDelegate& ContentWindow::getInteractionDelegate() const
{
    return *interactionDelegate_;
}

void ContentWindow::createInteractionDelegate()
{
    assert( content_ );

    switch ( content_->getType( ))
    {
    case CONTENT_TYPE_PIXEL_STREAM:
        interactionDelegate_.reset( new PixelStreamInteractionDelegate( *this ));
        break;
#if ENABLE_PDF_SUPPORT
    case CONTENT_TYPE_PDF:
        interactionDelegate_.reset( new PDFInteractionDelegate( *this ));
        break;
#endif
    default:
        interactionDelegate_.reset( new ZoomInteractionDelegate( *this ));
        break;
    }
}

void ContentWindow::setEventToNewDimensions()
{
    Event state;
    state.type = Event::EVT_VIEW_SIZE_CHANGED;
    state.dx = coordinates_.width();
    state.dy = coordinates_.height();

    emit eventChanged( state );
}

void ContentWindow::constrainZoomCenter()
{
    // clamp center point such that view rectangle dimensions are constrained [0,1]
    const float tX = zoomCenter_.x() - 0.5f / zoom_;
    const float tY = zoomCenter_.y() - 0.5f / zoom_;
    const float tW = 1.f / zoom_;
    const float tH = 1.f / zoom_;

    if( !QRectF( 0.f, 0.f, 1.f, 1.f ).contains( QRectF( tX, tY, tW, tH )))
    {
        if( tX < 0.f )
            zoomCenter_.setX( 0.5f / zoom_ );
        else if( tX+tW > 1.f )
            zoomCenter_.setX( 1.f - tW + 0.5f / zoom_ );

        if( tY < 0.f )
            zoomCenter_.setY( 0.5f / zoom_ );
        else if( tY+tH > 1.f )
            zoomCenter_.setY( 1.f - tH + 0.5f / zoom_ );
    }
}
