/*********************************************************************/
/* Copyright (c) 2013, EPFL/Blue Brain Project                       */
/*                     Raphael Dumusc <raphael.dumusc@epfl.ch>       */
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

#include "PixelStreamInteractionDelegate.h"

#include "ContentWindow.h"

#include "gestures/DoubleTapGesture.h"
#include "gestures/PanGesture.h"
#include "gestures/PinchGesture.h"

#define WHEEL_EVENT_FACTOR 1440.0

PixelStreamInteractionDelegate::PixelStreamInteractionDelegate( ContentWindow& contentWindow )
    : ContentInteractionDelegate( contentWindow )
{
}

void PixelStreamInteractionDelegate::tap( QTapGesture* gesture )
{
    if ( gesture->state() != Qt::GestureFinished )
        return;

    deflect::Event event = getGestureEvent( gesture );
    event.type = deflect::Event::EVT_CLICK;

    contentWindow_.dispatchEvent( event );
}

void PixelStreamInteractionDelegate::doubleTap( DoubleTapGesture* gesture )
{
    deflect::Event event = getGestureEvent( gesture );
    event.type = deflect::Event::EVT_DOUBLECLICK;

    contentWindow_.dispatchEvent( event );
}

void PixelStreamInteractionDelegate::pan( PanGesture* gesture )
{
    deflect::Event event = getGestureEvent( gesture );

    switch( gesture->state( ))
    {
    case Qt::GestureStarted:
        event.type = deflect::Event::EVT_PRESS;
        break;
    case Qt::GestureUpdated:
        event.type = deflect::Event::EVT_MOVE;
        event.dx = gesture->delta().x() / contentWindow_.getCoordinates().width();
        event.dy = gesture->delta().y() / contentWindow_.getCoordinates().height();
        break;
    case Qt::GestureFinished:
        event.type = deflect::Event::EVT_RELEASE;
        break;
    case Qt::NoGesture:
    case Qt::GestureCanceled:
    default:
        event.type = deflect::Event::EVT_NONE;
        break;
    }

    contentWindow_.dispatchEvent( event );
}

void PixelStreamInteractionDelegate::swipe( QSwipeGesture* gesture )
{
    deflect::Event event;

    if( gesture->horizontalDirection() == QSwipeGesture::Left )
        event.type = deflect::Event::EVT_SWIPE_LEFT;

    else if( gesture->horizontalDirection() == QSwipeGesture::Right )
        event.type = deflect::Event::EVT_SWIPE_LEFT;

    else if( gesture->verticalDirection() == QSwipeGesture::Up )
        event.type = deflect::Event::EVT_SWIPE_UP;

    else if( gesture->verticalDirection() == QSwipeGesture::Down )
        event.type = deflect::Event::EVT_SWIPE_DOWN;

    if( event.type != deflect::Event::EVT_NONE )
        contentWindow_.dispatchEvent( event );
}

void PixelStreamInteractionDelegate::pinch( PinchGesture* gesture )
{
    const qreal factor = ( gesture->scaleFactor() - 1.0 ) * 0.2;
    if( std::isnan( factor ) || std::isinf( factor ))
        return;

    deflect::Event event = getGestureEvent( gesture );
    event.type = deflect::Event::EVT_WHEEL;
    event.mouseLeft = false;
    event.dy = factor;

    contentWindow_.dispatchEvent( event );
}

void PixelStreamInteractionDelegate::mouseMoveEvent( QGraphicsSceneMouseEvent* mouseEvent )
{
    deflect::Event event = getMouseEvent( mouseEvent );
    event.type = deflect::Event::EVT_MOVE;

    const QPointF delta = mouseEvent->pos() - mouseEvent->lastPos();
    event.dx = delta.x() / contentWindow_.getCoordinates().width();
    event.dy = delta.y() / contentWindow_.getCoordinates().height();

    contentWindow_.dispatchEvent( event );
}

void PixelStreamInteractionDelegate::mousePressEvent( QGraphicsSceneMouseEvent* mouseEvent )
{
    deflect::Event event = getMouseEvent( mouseEvent );
    event.type = deflect::Event::EVT_PRESS;

    mousePressPos_ = mouseEvent->pos();

    contentWindow_.dispatchEvent( event );
}

void PixelStreamInteractionDelegate::mouseDoubleClickEvent( QGraphicsSceneMouseEvent* mouseEvent )
{
    deflect::Event event = getMouseEvent( mouseEvent );
    event.type = deflect::Event::EVT_DOUBLECLICK;

    contentWindow_.dispatchEvent( event );
}

void PixelStreamInteractionDelegate::mouseReleaseEvent( QGraphicsSceneMouseEvent* mouseEvent )
{
    deflect::Event event = getMouseEvent( mouseEvent );
    event.type = deflect::Event::EVT_RELEASE;

    contentWindow_.dispatchEvent( event );

    // Also generate a click event if releasing the button in place
    const QPointF delta = mousePressPos_ - mouseEvent->pos();
    const double epsilon = std::numeric_limits< double >::epsilon();
    if( fabs( delta.x( )) < epsilon && fabs( delta.y( )) < epsilon )
    {
        event.type = deflect::Event::EVT_CLICK;
        contentWindow_.dispatchEvent( event );
    }
}

void PixelStreamInteractionDelegate::wheelEvent( QGraphicsSceneWheelEvent* evt )
{
    deflect::Event event = getMouseEvent( evt );
    event.type = deflect::Event::EVT_WHEEL;

    if( evt->orientation() == Qt::Vertical )
    {
        event.dx = 0.0;
        event.dy = (double)evt->delta() / WHEEL_EVENT_FACTOR;
    }
    else
    {
        event.dx = (double)evt->delta() / WHEEL_EVENT_FACTOR;
        event.dy = 0.0;
    }

    contentWindow_.dispatchEvent( event );
}

void PixelStreamInteractionDelegate::keyPressEvent( QKeyEvent* keyEvent )
{
    deflect::Event event;
    event.type = deflect::Event::EVT_KEY_PRESS;
    event.key = keyEvent->key();
    event.modifiers = keyEvent->modifiers();
    strncpy( event.text, keyEvent->text().toStdString().c_str(),
             sizeof( event.text ));

    contentWindow_.dispatchEvent( event );
}

void PixelStreamInteractionDelegate::keyReleaseEvent( QKeyEvent* keyEvent )
{
    deflect::Event event;
    event.type = deflect::Event::EVT_KEY_RELEASE;
    event.key = keyEvent->key();
    event.modifiers = keyEvent->modifiers();
    strncpy( event.text, keyEvent->text().toStdString().c_str(),
             sizeof( event.text ));

    contentWindow_.dispatchEvent( event );
}

template <typename T>
deflect::Event PixelStreamInteractionDelegate::getMouseEvent( const T* qtEvent )
{
    const QRectF& window = contentWindow_.getCoordinates();

    deflect::Event event;
    event.mouseX = ( qtEvent->scenePos().x() - window.x( )) / window.width();
    event.mouseY = ( qtEvent->scenePos().y() - window.y( )) / window.height();

    event.mouseLeft = qtEvent->buttons().testFlag( Qt::LeftButton );
    event.mouseMiddle = qtEvent->buttons().testFlag( Qt::MidButton );
    event.mouseRight = qtEvent->buttons().testFlag( Qt::RightButton );

    return event;
}

template <typename T>
deflect::Event PixelStreamInteractionDelegate::getGestureEvent( const T* gesture )
{
    // For some QGestures, position() is a screen position (Qt global).
    // However, QTapGesture has a scene position because it uses touchPoint.pos,
    // which is intentionally set to scenePos in MultiTouchListener.
    // For custom gesture classes, we explicitly define the position to be
    // the scene position, so this method is correct for the following gestures:
    // QTapGesture, DoubleTapGesture, PanGesture, PinchGesture
    // Should also work for the same reason as QTapGesture, but untested:
    // QTapAndHoldGesture, QPanGesture

    const QRectF& win = contentWindow_.getCoordinates();

    deflect::Event event;
    event.mouseLeft = true;
    event.mouseX = ( gesture->position().x() - win.x( )) / win.width();
    event.mouseY = ( gesture->position().y() - win.y( )) / win.height();
    return event;
}
