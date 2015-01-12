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
#include <deflect/EventReceiver.h>

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
    , zoomRect_( 0.0, 0.0, 1.0, 1.0 )
    , windowState_( NONE )
    , eventReceiversCount_( 0 )
{
}

ContentWindow::ContentWindow( ContentPtr content )
    : uuid_( QUuid::createUuid( ))
    , zoomRect_( 0.0, 0.0, 1.0, 1.0 )
    , windowState_( NONE )
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

    sendSizeChangedEvent();
}

const QRectF& ContentWindow::getZoomRect() const
{
    return zoomRect_;
}

void ContentWindow::setZoomRect( const QRectF& zoomRect )
{
    zoomRect_ = zoomRect;
    emit modified();
}

ContentWindow::WindowState ContentWindow::getState() const
{
    return windowState_;
}

void ContentWindow::setState( const ContentWindow::WindowState state )
{
    windowState_ = state;

    emit modified();
}

void ContentWindow::toggleSelectedState()
{
    if ( windowState_ == ContentWindow::NONE )
        windowState_ = ContentWindow::SELECTED;
    else if ( windowState_ == ContentWindow::SELECTED )
        windowState_ = ContentWindow::NONE;
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

ContentInteractionDelegate& ContentWindow::getInteractionDelegate()
{
    return *interactionDelegate_;
}

void ContentWindow::backupCoordinates()
{
    coordinatesBackup_ = coordinates_;
}

bool ContentWindow::hasBackupCoordinates() const
{
    return coordinatesBackup_.isValid();
}

void ContentWindow::restoreCoordinates()
{
    if( !hasBackupCoordinates( ))
        return;

    setCoordinates( coordinatesBackup_ );
    coordinatesBackup_ = QRectF();
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
