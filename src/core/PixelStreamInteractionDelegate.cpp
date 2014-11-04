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
#include "globals.h"
#include "configuration/Configuration.h"
#include "gestures/DoubleTapGesture.h"
#include "gestures/PanGesture.h"
#include "gestures/PinchGesture.h"

#define WHEEL_EVENT_FACTOR 1440.0

PixelStreamInteractionDelegate::PixelStreamInteractionDelegate(ContentWindow& contentWindow)
    : ContentInteractionDelegate(contentWindow)
{
}

void PixelStreamInteractionDelegate::tap(QTapGesture *gesture)
{
    if ( gesture->state() == Qt::GestureFinished )
    {
        Event event = getGestureEvent(gesture);

        event.mouseLeft = true;
        event.type = Event::EVT_CLICK;

        contentWindow_.dispatchEvent(event);
    }
}

void PixelStreamInteractionDelegate::doubleTap(DoubleTapGesture *gesture)
{
    Event event = getGestureEvent(gesture);

    event.mouseLeft = true;
    event.type = Event::EVT_DOUBLECLICK;

    contentWindow_.dispatchEvent(event);
}

void PixelStreamInteractionDelegate::pan(PanGesture *gesture)
{
    Event event = getGestureEvent(gesture);

    event.mouseLeft = true;

    switch( gesture->state( ) )
    {
    case Qt::GestureStarted:
        event.type = Event::EVT_PRESS;
        break;
    case Qt::GestureUpdated:
        event.type = Event::EVT_MOVE;
        setPanGestureNormalizedDelta(gesture, event);
        break;
    case Qt::GestureFinished:
        event.type = Event::EVT_RELEASE;
        break;
    case Qt::NoGesture:
    case Qt::GestureCanceled:
    default:
        event.type = Event::EVT_NONE;
        break;
    }

    contentWindow_.dispatchEvent(event);
}

void PixelStreamInteractionDelegate::swipe(QSwipeGesture *gesture)
{
    Event event;

    if (gesture->horizontalDirection() == QSwipeGesture::Left)
    {
        event.type = Event::EVT_SWIPE_LEFT;
    }
    else if (gesture->horizontalDirection() == QSwipeGesture::Right)
    {
        event.type = Event::EVT_SWIPE_LEFT;
    }
    else if (gesture->verticalDirection() == QSwipeGesture::Up)
    {
        event.type = Event::EVT_SWIPE_UP;
    }
    else if (gesture->verticalDirection() == QSwipeGesture::Down)
    {
        event.type = Event::EVT_SWIPE_DOWN;
    }

    if (event.type != Event::EVT_NONE)
    {
        contentWindow_.dispatchEvent(event);
    }
}

void PixelStreamInteractionDelegate::pinch(PinchGesture *gesture)
{
    const qreal factor = (gesture->scaleFactor() - 1.) * 0.2f + 1.f;
    if( std::isnan( factor ) || std::isinf( factor ))
        return;

    Event event = getGestureEvent(gesture);
    event.dy = (factor - 1.f) * g_configuration->getTotalWidth();
    event.type = Event::EVT_WHEEL;

    contentWindow_.dispatchEvent(event);
}


void PixelStreamInteractionDelegate::mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    Event event = getMouseEvent(mouseEvent);
    event.type = Event::EVT_MOVE;

    setMouseMoveNormalizedDelta(mouseEvent, event);

    contentWindow_.dispatchEvent(event);
}

void PixelStreamInteractionDelegate::mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    Event event = getMouseEvent(mouseEvent);
    event.type = Event::EVT_PRESS;

    mousePressPos_ = mouseEvent->pos();

    contentWindow_.dispatchEvent(event);
}

void PixelStreamInteractionDelegate::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    Event event = getMouseEvent(mouseEvent);
    event.type = Event::EVT_DOUBLECLICK;

    contentWindow_.dispatchEvent(event);
}

void PixelStreamInteractionDelegate::mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    Event event = getMouseEvent(mouseEvent);
    event.type = Event::EVT_RELEASE;

    contentWindow_.dispatchEvent(event);

    // Also generate a click event if releasing the button in place
    if ( fabs(mousePressPos_.x() - mouseEvent->pos().x()) < std::numeric_limits< float >::epsilon() &&
         fabs(mousePressPos_.y() - mouseEvent->pos().y()) < std::numeric_limits< float >::epsilon() )
    {
        event.type = Event::EVT_CLICK;
        contentWindow_.dispatchEvent(event);
    }
}

void PixelStreamInteractionDelegate::wheelEvent(QGraphicsSceneWheelEvent* evt)
{
    Event event = getMouseEvent(evt);

    event.type = Event::EVT_WHEEL;

    if (evt->orientation() == Qt::Vertical)
    {
        event.dx = 0.;
        event.dy = (double)evt->delta() / WHEEL_EVENT_FACTOR * g_configuration->getTotalHeight();
    }
    else
    {
        event.dx = (double)evt->delta() / WHEEL_EVENT_FACTOR * g_configuration->getTotalWidth();
        event.dy = 0.;
    }

    contentWindow_.dispatchEvent(event);
}

void PixelStreamInteractionDelegate::keyPressEvent( QKeyEvent* keyEvent )
{
    Event event;
    event.type = Event::EVT_KEY_PRESS;
    event.key = keyEvent->key();
    event.modifiers = keyEvent->modifiers();
    strncpy( event.text, keyEvent->text().toStdString().c_str(),
             sizeof( event.text ));

    contentWindow_.dispatchEvent( event );
}

void PixelStreamInteractionDelegate::keyReleaseEvent( QKeyEvent* keyEvent )
{
    Event event;
    event.type = Event::EVT_KEY_RELEASE;
    event.key = keyEvent->key();
    event.modifiers = keyEvent->modifiers();
    strncpy( event.text, keyEvent->text().toStdString().c_str(),
             sizeof( event.text ));

    contentWindow_.dispatchEvent( event );
}

template <typename T>
Event PixelStreamInteractionDelegate::getMouseEvent(const T* qtEvent)
{
    // Bounding rectangle
    const QRectF& coord = contentWindow_.getCoordinates();

    QPointF eventPos = qtEvent->pos();

    Event event;

    // Normalize mouse coordinates
    event.mouseX = ( eventPos.x() - coord.x( )) / coord.width();
    event.mouseY = ( eventPos.y() - coord.y( )) / coord.height();

    event.mouseLeft = qtEvent->buttons().testFlag( Qt::LeftButton );
    event.mouseMiddle = qtEvent->buttons().testFlag( Qt::MidButton );
    event.mouseRight = qtEvent->buttons().testFlag( Qt::RightButton );

    return event;
}

void PixelStreamInteractionDelegate::setMouseMoveNormalizedDelta(const QGraphicsSceneMouseEvent* mouseEvent, Event& event)
{
    // Bounding rectangle
    double w, h;
    contentWindow_.getSize(w, h);

    event.dx = (mouseEvent->pos().x() - mouseEvent->lastPos().x()) / w;
    event.dy = (mouseEvent->pos().y() - mouseEvent->lastPos().y()) / h;
}


/** Returns an new Event with normalized mouse coordinates */

template<typename T>
Event PixelStreamInteractionDelegate::getGestureEvent(const T *gesture)
{
    const QRectF& win = contentWindow_.getCoordinates();

    // For (almost) all QGestures, position() is the normalized position
    // (on the wall / QGraphicsView)
    Event event;
    event.mouseX = ( gesture->position().x() - win.x( )) / win.width();
    event.mouseY = ( gesture->position().y() - win.y( )) / win.height();

    // The returned event holds the normalized position inside the window
    return event;
}

Event PixelStreamInteractionDelegate::getGestureEvent(const QTapGesture *gesture)
{
    const QRectF& win = contentWindow_.getCoordinates();

    // Overall wall dimensions (in pixels)
    const double tWidth = g_configuration->getTotalWidth();
    const double tHeight = g_configuration->getTotalHeight();

    // For QTapGestures, position() is the position on the WALL
    // (not QGraphicsView!) in pixels
    Event event;
    event.mouseX = ( gesture->position().x() / tWidth - win.x( )) / win.width();
    event.mouseY = ( gesture->position().y() / tHeight - win.y( )) / win.height();

    // The returned event holds the normalized position inside the window
    return event;
}

Event PixelStreamInteractionDelegate::getGestureEvent(const PinchGesture *gesture)
{
    const QRectF& win = contentWindow_.getCoordinates();

    // For PinchGesture, normalizedCenterPoint() is the normalized position
    // (on the wall / QGraphicsView)
    Event event;
    event.mouseX = ( gesture->normalizedCenterPoint().x() - win.x( )) / win.width();
    event.mouseY = ( gesture->normalizedCenterPoint().y() - win.y( )) / win.height();

    // The returned event holds the normalized position inside the window
    return event;
}

void PixelStreamInteractionDelegate::setPanGestureNormalizedDelta(const PanGesture* gesture, Event& event)
{
    // Bounding rectangle
    double w, h;
    contentWindow_.getSize(w, h);

    // Touchpad dimensions
    const double tWidth = g_configuration->getTotalWidth();
    const double tHeight = g_configuration->getTotalHeight();

    event.dx = (gesture->delta().x() / tWidth) / w;
    event.dy = (gesture->delta().y() / tHeight) / h;
}

