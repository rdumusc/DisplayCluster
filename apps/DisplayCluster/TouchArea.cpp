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

#include "gestures/PanGesture.h"
#include "gestures/PanGestureRecognizer.h"

#include <QEvent>
#include <QTapGesture>
#include <QTapAndHoldGesture>
#include <QGestureEvent>

TouchArea::TouchArea( QDeclarativeItem* parentItem_ )
    : QDeclarativeItem( parentItem_ )
    , blockTapGesture_( false )
{
    grabGesture( Qt::TapGesture );
    grabGesture( Qt::TapAndHoldGesture );
    grabGesture( PanGestureRecognizer::type( ));
}

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

    if(( gesture = event_->gesture( Qt::TapAndHoldGesture )))
    {
        event_->accept( Qt::TapAndHoldGesture );
        if( gesture->state() == Qt::GestureFinished )
        {
            blockTapGesture_ = true;
            QTapAndHoldGesture* tapAndHoldGesture =
                    static_cast<QTapAndHoldGesture*>( gesture );
            emit tapAndHold( tapAndHoldGesture->position( ));
        }
        return true;
    }
    if(( gesture = event_->gesture( Qt::TapGesture )))
    {
        // Qt does not allow canceling Tap gestures after a TapAndHold has been
        // accepted, so blockTapGesture_ is here to prevent the Tap at the end
        // of the following sequence:
        // *Tap begin* ----- *TapAndHold begin* - *TapAndHold end* -- *Tap end*
        if( gesture->state() == Qt::GestureStarted )
            blockTapGesture_ = false;
        // Only block tap gesture, but let other gestures (like pan) continue
        // In some cases, they may have been in progress before the tapAndHold
        // happend
        if( !blockTapGesture_ )
        {
            event_->accept( Qt::TapGesture );
            if( gesture->state() == Qt::GestureFinished )
            {
                QTapGesture* tapGesture = static_cast<QTapGesture*>( gesture );
                emit tap( tapGesture->position( ));
            }
            return true;
        }
    }
    if(( gesture = event_->gesture( PanGestureRecognizer::type( ))))
    {
        event_->accept( PanGestureRecognizer::type( ));
        PanGesture* panGesture = static_cast<PanGesture*>( gesture );
        if( panGesture->state() == Qt::GestureCanceled ||
            panGesture->state() == Qt::GestureFinished )
        {
            emit panFinished();
            return true;
        }
        emit pan( panGesture->delta( ));
        return true;
    }
    return false;
}
