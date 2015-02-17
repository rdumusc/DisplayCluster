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

#include "ContentWindowTouchArea.h"

#include "ContentWindow.h"
#include "Content.h"
#include "MovieContent.h"
#include "ZoomInteractionDelegate.h"

#include "gestures/DoubleTapGesture.h"
#include "gestures/DoubleTapGestureRecognizer.h"
#include "gestures/PanGesture.h"
#include "gestures/PanGestureRecognizer.h"
#include "gestures/PinchGesture.h"
#include "gestures/PinchGestureRecognizer.h"

#include <QtCore/QEvent>
#include <QtGui/QSwipeGesture>
#include <QtGui/QTapGesture>
#include <QtGui/QPainter>

#define STD_WHEEL_DELTA 120 // Common value for the delta of mouse wheel events

#define WHEEL_SCALE_SPEED   0.1

#define BUTTON_REL_WIDTH    0.23
#define BUTTON_REL_HEIGHT   0.45

ContentWindowTouchArea::ContentWindowTouchArea()
    : controller_( 0 )
{
    setFlag( QGraphicsItem::ItemIsMovable, true );
    setFlag( QGraphicsItem::ItemIsSelectable, true );
    setFlag( QGraphicsItem::ItemIsFocusable, true ); // to get key events
    setAcceptedMouseButtons( Qt::LeftButton );

    grabGesture( DoubleTapGestureRecognizer::type( ));
    grabGesture( PanGestureRecognizer::type( ));
    grabGesture( PinchGestureRecognizer::type( ));
    grabGesture( Qt::SwipeGesture );
    grabGesture( Qt::TapAndHoldGesture );
    grabGesture( Qt::TapGesture );
}

ContentWindowTouchArea::~ContentWindowTouchArea()
{
    delete controller_;
}

void ContentWindowTouchArea::init( ContentWindowPtr contentWindow,
                                   const DisplayGroup& displayGroup )
{
    contentWindow_ = contentWindow;
    controller_ = new ContentWindowController( *contentWindow, displayGroup );
}

ContentWindowController* ContentWindowTouchArea::getWindowController()
{
    return controller_;
}

bool ContentWindowTouchArea::sceneEvent( QEvent* event_ )
{
    if( contentWindow_->isHidden( ))
        return false;

    switch( event_->type( ))
    {
    case QEvent::Gesture:
        emit activated();
        gestureEvent( static_cast< QGestureEvent* >( event_ ));
        return true;
    case QEvent::KeyPress:
        // Override default behaviour to process TAB key events
        keyPressEvent( static_cast< QKeyEvent* >( event_ ));
        return true;
    default:
        return QGraphicsObject::sceneEvent( event_ );
    }
}

void ContentWindowTouchArea::mouseMoveEvent( QGraphicsSceneMouseEvent* event_ )
{
    if( contentWindow_->isSelected( ))
    {
        contentWindow_->getInteractionDelegate().mouseMoveEvent( event_ );
        return;
    }

    if( event_->buttons().testFlag( Qt::LeftButton ))
    {
        if( contentWindow_->isMoving( ))
        {
            const QPointF delta = event_->scenePos() - event_->lastScenePos();
            const QPointF newPos = parentItem()->pos() + delta;
            controller_->moveTo( newPos );
        }
    }
}

void ContentWindowTouchArea::mousePressEvent( QGraphicsSceneMouseEvent* event_ )
{
    emit activated();

    if( contentWindow_->isSelected( ))
    {
        contentWindow_->getInteractionDelegate().mousePressEvent( event_ );
        return;
    }

    contentWindow_->setState( ContentWindow::MOVING );
}

void ContentWindowTouchArea::mouseDoubleClickEvent( QGraphicsSceneMouseEvent* )
{
    contentWindow_->toggleSelectedState();
}

void ContentWindowTouchArea::mouseReleaseEvent( QGraphicsSceneMouseEvent* event_ )
{
    if( contentWindow_->isSelected( ))
    {
        contentWindow_->getInteractionDelegate().mouseReleaseEvent( event_ );
        return;
    }

    contentWindow_->setState( ContentWindow::NONE );
}

void ContentWindowTouchArea::wheelEvent( QGraphicsSceneWheelEvent* event_ )
{
    if ( contentWindow_->isSelected( ))
        contentWindow_->getInteractionDelegate().wheelEvent( event_ );
    else
        controller_->scale( event_->scenePos(), 1.0 + WHEEL_SCALE_SPEED *
                            (double)event_->delta() / STD_WHEEL_DELTA );
}

void ContentWindowTouchArea::keyPressEvent( QKeyEvent* event_ )
{
    if( contentWindow_->isSelected( ))
        contentWindow_->getInteractionDelegate().keyPressEvent( event_ );
}

void ContentWindowTouchArea::keyReleaseEvent( QKeyEvent* event_ )
{
    if( contentWindow_->isSelected( ))
        contentWindow_->getInteractionDelegate().keyReleaseEvent( event_ );
}

void ContentWindowTouchArea::gestureEvent( QGestureEvent* event_ )
{
    QGesture* gesture = 0;

    if( ( gesture = event_->gesture( Qt::TapAndHoldGesture )))
    {
        event_->accept( Qt::TapAndHoldGesture );
        tapAndHold( static_cast< QTapAndHoldGesture* >( gesture ));
        return;
    }

    if( contentWindow_->isSelected( ))
    {
        contentWindow_->getInteractionDelegate().gestureEvent( event_ );
        return;
    }

    if( ( gesture = event_->gesture( PanGestureRecognizer::type( ))))
    {
        event_->accept( PanGestureRecognizer::type( ));
        pan( static_cast< PanGesture* >( gesture ));
    }
    else if( ( gesture = event_->gesture( PinchGestureRecognizer::type( ))))
    {
        event_->accept( PinchGestureRecognizer::type( ));
        pinch( static_cast< PinchGesture* >( gesture ));
    }
    else if( ( gesture = event_->gesture( DoubleTapGestureRecognizer::type( ))))
    {
        event_->accept( DoubleTapGestureRecognizer::type( ));
        doubleTap( static_cast< DoubleTapGesture* >( gesture ));
    }
}

void ContentWindowTouchArea::doubleTap( DoubleTapGesture* gesture )
{
    if( gesture->state() == Qt::GestureFinished )
        controller_->toggleFullscreen();
}

void ContentWindowTouchArea::pan( PanGesture* gesture )
{
    if( gesture->state() == Qt::GestureStarted )
        contentWindow_->setState( ContentWindow::MOVING );

    else if( gesture->state() == Qt::GestureCanceled ||
             gesture->state() == Qt::GestureFinished )
        contentWindow_->setState( ContentWindow::NONE );

    const QPointF& windowPos = contentWindow_->getCoordinates().topLeft();
    controller_->moveTo( windowPos + gesture->delta( ));
}

void ContentWindowTouchArea::pinch( PinchGesture* gesture )
{
    const double factor =
             ZoomInteractionDelegate::adaptZoomFactor( gesture->scaleFactor( ));
    if( factor == 0.0 )
        return;

    if( gesture->state() == Qt::GestureStarted )
        contentWindow_->setState( ContentWindow::RESIZING );

    else if( gesture->state() == Qt::GestureCanceled ||
             gesture->state() == Qt::GestureFinished )
    {
        contentWindow_->setState( ContentWindow::NONE );
    }

    controller_->scale( gesture->position(), factor );
}

void ContentWindowTouchArea::tapAndHold( QTapAndHoldGesture* gesture )
{
    if( gesture->state() == Qt::GestureFinished )
        contentWindow_->toggleSelectedState();
}
