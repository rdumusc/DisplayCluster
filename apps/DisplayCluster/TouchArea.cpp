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

#include "TouchArea.h"

#include "ZoomInteractionDelegate.h"

#include "gestures/DoubleTapGesture.h"
#include "gestures/DoubleTapGestureRecognizer.h"
#include "gestures/PanGesture.h"
#include "gestures/PanGestureRecognizer.h"
#include "gestures/PinchGesture.h"
#include "gestures/PinchGestureRecognizer.h"

#include <QEvent>
#include <QTapGesture>
#include <QTapAndHoldGesture>
#include <QSwipeGesture>
#include <QGestureEvent>

#include <cmath>        /* std::isnan */

TouchArea::TouchArea( QDeclarativeItem* parentItem_ )
    : QDeclarativeItem( parentItem_ )
    , _blockTapGesture( false )
{
    grabGesture( Qt::TapGesture );
    grabGesture( DoubleTapGestureRecognizer::type( ));
    grabGesture( Qt::TapAndHoldGesture );
    grabGesture( PanGestureRecognizer::type( ));
    grabGesture( PinchGestureRecognizer::type( ));
    grabGesture( Qt::SwipeGesture );
}

TouchArea::~TouchArea() {}

bool TouchArea::sceneEvent( QEvent* event_ )
{
    switch( event_->type( ))
    {
    case QEvent::Gesture:
        return gestureEvent( static_cast< QGestureEvent* >( event_ ));
    default:
        return QGraphicsObject::sceneEvent( event_ );
    }
}

bool TouchArea::gestureEvent( QGestureEvent* event_ )
{
    QGesture* gesture = 0;

    if( ( gesture = event_->gesture( Qt::TapAndHoldGesture )))
    {
        event_->accept( Qt::TapAndHoldGesture );
        tapAndHold( static_cast< QTapAndHoldGesture* >( gesture ));

        // Qt does not allow canceling Tap gestures after a TapAndHold has been
        // accepted, so _blockTapGesture is here to prevent the Tap at the end
        // of the following sequence:
        // *Tap begin* ----- *TapAndHold begin* - *TapAndHold end* -- *Tap end*
        if( gesture->state() == Qt::GestureFinished )
            _blockTapGesture = true;

        return true;
    }
    if( ( gesture = event_->gesture( DoubleTapGestureRecognizer::type( ))))
    {
        event_->accept( DoubleTapGestureRecognizer::type( ));
        doubleTap( static_cast< DoubleTapGesture* >( gesture ));
        return true;
    }
    if( ( gesture = event_->gesture( PanGestureRecognizer::type( ))))
    {
        event_->accept( PanGestureRecognizer::type( ));
        pan( static_cast< PanGesture* >( gesture ));
        return true;
    }
    if( ( gesture = event_->gesture( PinchGestureRecognizer::type( ))))
    {
        event_->accept( PinchGestureRecognizer::type( ));
        pinch( static_cast< PinchGesture* >( gesture ));
        return true;
    }
    if( ( gesture = event_->gesture( Qt::SwipeGesture )))
    {
        event_->accept( Qt::SwipeGesture );
        swipe( static_cast< QSwipeGesture* >( gesture ));
        return true;
    }
    if( ( gesture = event_->gesture( Qt::TapGesture )))
    {
        if( gesture->state() == Qt::GestureStarted )
            _blockTapGesture = false;

        if( !_blockTapGesture )
        {
            event_->accept( Qt::TapGesture );
            tap( static_cast< QTapGesture* >( gesture ));
            return true;
        }
    }
    return false;
}

void TouchArea::tap( QTapGesture* gesture )
{
    if( gesture->state() == Qt::GestureStarted )
        emit touchBegin( gesture->position( ));

    if( gesture->state() == Qt::GestureCanceled ||
        gesture->state() == Qt::GestureFinished )
    {
        emit touchEnd( gesture->position( ));
    }

    if( gesture->state() == Qt::GestureFinished )
        emit tap( gesture->position( ));
}

void TouchArea::doubleTap( DoubleTapGesture* gesture )
{
    if( gesture->state() == Qt::GestureFinished )
        emit doubleTap( gesture->position( ));
}

void TouchArea::tapAndHold( QTapAndHoldGesture* gesture )
{
    if( gesture->state() == Qt::GestureFinished )
        emit tapAndHold( gesture->position( ));
}

void TouchArea::pan( PanGesture* panGesture )
{
    if( panGesture->state() == Qt::GestureStarted ||
        panGesture->state() == Qt::GestureUpdated )
    {
        emit pan( panGesture->position(), panGesture->delta( ));
    }
    else
        emit touchEnd( panGesture->position( ));
}

void TouchArea::pinch( PinchGesture* gesture )
{
    if( gesture->state() == Qt::GestureStarted ||
        gesture->state() == Qt::GestureUpdated )
    {
        const qreal factor = gesture->scaleFactor();

        if( std::isnan( factor ) || std::isinf( factor ))
            return;

        emit pinch( gesture->position(), factor );
    }
    else
        emit touchEnd( gesture->position( ));
}

void TouchArea::swipe( QSwipeGesture* gesture )
{
    if( gesture->state() == Qt::GestureFinished )
    {
        if( gesture->horizontalDirection() == QSwipeGesture::Left )
            emit swipeLeft();

        else if( gesture->horizontalDirection() == QSwipeGesture::Right )
            emit swipeRight();

        else if( gesture->verticalDirection() == QSwipeGesture::Up )
            emit swipeUp();

        else if( gesture->verticalDirection() == QSwipeGesture::Down )
            emit swipeDown();
    }
}
