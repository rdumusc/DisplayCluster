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

#include "ContentInteractionDelegate.h"
#include "ContentWindowController.h"
#include <deflect/EventReceiver.h>

#include "config.h"
#include "log.h"

#include "PixelStreamInteractionDelegate.h"
#include "ZoomInteractionDelegate.h"
#if ENABLE_PDF_SUPPORT
#  include "PDFInteractionDelegate.h"
#endif

IMPLEMENT_SERIALIZE_FOR_XML( ContentWindow )

qreal ContentWindow::maxContentScale_ = 2.0;

ContentWindow::ContentWindow()
    : uuid_( QUuid::createUuid( ))
    , zoomRect_( UNIT_RECTF )
    , windowBorder_( NOBORDER )
    , focused_( false )
    , windowState_( NONE )
    , controlsVisible_( false )
    , eventReceiversCount_( 0 )
{
}

ContentWindow::ContentWindow( ContentPtr content )
    : uuid_( QUuid::createUuid( ))
    , zoomRect_( UNIT_RECTF )
    , windowBorder_( NOBORDER )
    , focused_( false )
    , windowState_( NONE )
    , controlsVisible_( false )
    , eventReceiversCount_( 0 )
{
    assert( content );
    setContent( content );
    coordinates_.setSize( content_->getDimensions( ));
}

ContentWindow::~ContentWindow()
{
}

const QUuid& ContentWindow::getID() const
{
    return uuid_;
}

Content* ContentWindow::getContentPtr() const
{
    return content_.get();
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

ContentWindowController* ContentWindow::getController()
{
    return controller_.get();
}

void ContentWindow::setController( ContentWindowControllerPtr controller )
{
    controller_.reset( controller.release( ));
}

void ContentWindow::setCoordinates( const QRectF& coordinates )
{
    if( coordinates == coordinates_ )
        return;

    setX( coordinates.x( ));
    setY( coordinates.y( ));
    setWidth( coordinates.width( ));
    setHeight( coordinates.height( ));

    emit modified();

    sendSizeChangedEvent();
}

const QRectF& ContentWindow::getZoomRect() const
{
    return zoomRect_;
}

void ContentWindow::setZoomRect( const QRectF& zoomRect )
{
    if( zoomRect_ == zoomRect )
        return;

    zoomRect_ = zoomRect;
    emit modified();
}

ContentWindow::WindowBorder ContentWindow::getBorder() const
{
    return windowBorder_;
}

ContentWindow::WindowState ContentWindow::getState() const
{
    return windowState_;
}

void ContentWindow::setBorder( const ContentWindow::WindowBorder border )
{
    if( windowBorder_ == border )
        return;
    windowBorder_ = border;
    emit borderChanged();
    emit modified();
}

bool ContentWindow::isFocused() const
{
    return focused_;
}

void ContentWindow::setFocused( const bool value )
{
    if( focused_ == value )
        return;

    focused_ = value;

    emit focusedChanged();

    // Only emit modified once, in setState() or here
    if( !setState( focused_ ? SELECTED : NONE ))
        emit modified();
}

bool ContentWindow::setState( const ContentWindow::WindowState state )
{
    if( windowState_ == state )
        return false;

    if( content_->getType() == CONTENT_TYPE_PIXEL_STREAM &&
        state == SELECTED && !hasEventReceivers( ))
    {
        return false;
    }

    windowState_ = state;

    emit stateChanged();
    emit modified();
    return true;
}

void ContentWindow::toggleSelectedState()
{
    if ( windowState_ == ContentWindow::NONE )
        setState( ContentWindow::SELECTED );
    else if ( windowState_ == ContentWindow::SELECTED )
        setState( ContentWindow::NONE );
}

bool ContentWindow::isSelected() const
{
    return windowState_ == SELECTED;
}

bool ContentWindow::isMoving() const
{
    return windowState_ == MOVING;
}

bool ContentWindow::isResizing() const
{
    return windowState_ == RESIZING;
}

bool ContentWindow::isHidden() const
{
    return windowState_ == HIDDEN;
}

bool ContentWindow::registerEventReceiver( deflect::EventReceiver* receiver )
{
    const bool success = connect( this, SIGNAL( notify( deflect::Event )),
                                  receiver, SLOT( processEvent( deflect::Event )));
    if ( success )
        ++eventReceiversCount_;

    return success;
}

bool ContentWindow::hasEventReceivers() const
{
    return eventReceiversCount_ > 0;
}

void ContentWindow::dispatchEvent( const deflect::Event event_ )
{
    emit notify( event_ );
}

ContentInteractionDelegate* ContentWindow::getInteractionDelegate()
{
    return interactionDelegate_.get();
}

QString ContentWindow::getLabel() const
{
    return content_->getURI().section( "/", -1, -1 );
}

bool ContentWindow::getControlsVisible() const
{
    return controlsVisible_;
}

void ContentWindow::setControlsVisible( const bool value )
{
    if( value == controlsVisible_ )
        return;

    controlsVisible_ = value;
    emit controlsVisibleChanged();
    emit modified();
}

void ContentWindow::setMaxContentScale( const qreal value )
{
    if( value > 0 )
        maxContentScale_ = value;
}

qreal ContentWindow::getMaxContentScale()
{
    return maxContentScale_;
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

void ContentWindow::sendSizeChangedEvent()
{
    deflect::Event state;
    state.type = deflect::Event::EVT_VIEW_SIZE_CHANGED;
    state.dx = coordinates_.width();
    state.dy = coordinates_.height();

    emit notify( state );
}
