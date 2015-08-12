/*********************************************************************/
/* Copyright (c) 2015, EPFL/Blue Brain Project                       */
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

#include "TouchMouseArea.h"

#include <QEvent>
#include <QGraphicsSceneMouseEvent>

#define WHEEL_EVENT_FACTOR 1440.0

TouchMouseArea::TouchMouseArea()
{
    setFlag( QGraphicsItem::ItemIsSelectable, true );
    setFlag( QGraphicsItem::ItemIsFocusable, true ); // to get key events
    setAcceptedMouseButtons( Qt::LeftButton );
}

bool TouchMouseArea::sceneEvent( QEvent* event_ )
{
    switch( event_->type( ))
    {
    case QEvent::KeyPress:
        // Override default behaviour to process TAB key events
        keyPressEvent( static_cast< QKeyEvent* >( event_ ));
        return true;
    default:
        return TouchArea::sceneEvent( event_ );
    }
}

void TouchMouseArea::mousePressEvent( QGraphicsSceneMouseEvent* event_ )
{
    _mousePressPos = event_->scenePos();
    emit touchBegin();
}

void TouchMouseArea::mouseMoveEvent( QGraphicsSceneMouseEvent* event_ )
{
    const QPointF delta = event_->scenePos() - event_->lastScenePos();
    emit pan( event_->scenePos(), delta );
}

void TouchMouseArea::mouseReleaseEvent( QGraphicsSceneMouseEvent* event_ )
{
    emit panFinished( event_->scenePos( ));

    // Also generate a tap event if releasing the button in place
    const QPointF delta = _mousePressPos - event_->scenePos();
    const double epsilon = std::numeric_limits< double >::epsilon();
    if( fabs( delta.x( )) < epsilon && fabs( delta.y( )) < epsilon )
        emit tap( event_->scenePos( ));
}

void TouchMouseArea::mouseDoubleClickEvent( QGraphicsSceneMouseEvent* event_ )
{
    emit doubleTap( event_->scenePos( ));
}

void TouchMouseArea::wheelEvent( QGraphicsSceneWheelEvent* event_ )
{
    // change zoom based on wheel delta.
    // deltas are counted in 1/8 degrees, so scale based on 180 degrees =>
    // delta = 180*8 = 1440
    const qreal scaleFactor = 1.0 + event_->delta() / WHEEL_EVENT_FACTOR;
    emit pinch( event_->scenePos(), scaleFactor );
}

void TouchMouseArea::keyPressEvent( QKeyEvent* keyEvent )
{
    emit keyPress( keyEvent->key(), keyEvent->modifiers(), keyEvent->text( ));
}

void TouchMouseArea::keyReleaseEvent( QKeyEvent* keyEvent )
{
    emit keyPress( keyEvent->key(), keyEvent->modifiers(), keyEvent->text( ));
}
