/*********************************************************************/
/* Copyright (c) 2013, EPFL/Blue Brain Project                       */
/*                     Daniel Nachbaur <daniel.nachbaur@epfl.ch>     */
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

#include "MultiTouchListener.h"

#include "log.h"

#include "gestures/DoubleTapGestureRecognizer.h"
#include "gestures/PanGestureRecognizer.h"
#include "gestures/PinchGestureRecognizer.h"

#include <QApplication>

MultiTouchListener::MultiTouchListener( QGraphicsView* graphicsView )
    : TUIO::TuioListener()
    , graphicsView_( graphicsView )
{
    assert( graphicsView_ );

    DoubleTapGestureRecognizer::install();
    PanGestureRecognizer::install();
    PinchGestureRecognizer::install();

    client_.addTuioListener( this );
    client_.connect();
}

MultiTouchListener::~MultiTouchListener()
{
    client_.removeTuioListener( this );
    client_.disconnect();

    DoubleTapGestureRecognizer::uninstall();
    PanGestureRecognizer::uninstall();
    PinchGestureRecognizer::uninstall();
}

void MultiTouchListener::addTuioObject( TUIO::TuioObject* )
{
}

void MultiTouchListener::updateTuioObject( TUIO::TuioObject* )
{
}

void MultiTouchListener::removeTuioObject( TUIO::TuioObject* )
{
}

void MultiTouchListener::addTuioCursor( TUIO::TuioCursor* tcur )
{
    handleEvent( tcur, QEvent::TouchBegin );
    emit touchPointAdded( tcur->getCursorID(), getScenePos( tcur ));
}

void MultiTouchListener::updateTuioCursor( TUIO::TuioCursor* tcur )
{
    handleEvent( tcur, QEvent::TouchUpdate );
    emit touchPointUpdated( tcur->getCursorID(), getScenePos( tcur ));
}

void MultiTouchListener::removeTuioCursor( TUIO::TuioCursor* tcur )
{
    handleEvent( tcur, QEvent::TouchEnd );
    emit touchPointRemoved( tcur->getCursorID( ));
}

void MultiTouchListener::refresh( TUIO::TuioTime )
{
}

QPointF MultiTouchListener::getScenePos( TUIO::TuioCursor* tcur ) const
{
    return QPointF( tcur->getX() * graphicsView_->scene()->width(),
                    tcur->getY() * graphicsView_->scene()->height( ));
}

void MultiTouchListener::fillBegin( QTouchEvent::TouchPoint& touchPoint ) const
{
    touchPoint.setStartPos( touchPoint.pos( ));
    touchPoint.setStartScenePos( touchPoint.scenePos( ));
    touchPoint.setStartScreenPos( touchPoint.screenPos( ));
    touchPoint.setStartNormalizedPos( touchPoint.normalizedPos( ));

    touchPoint.setLastPos( touchPoint.pos( ));
    touchPoint.setLastScenePos( touchPoint.scenePos( ));
    touchPoint.setLastScreenPos( touchPoint.screenPos( ));
    touchPoint.setLastNormalizedPos( touchPoint.normalizedPos( ));
}

void MultiTouchListener::fill( QTouchEvent::TouchPoint& touchPoint,
                               const QTouchEvent::TouchPoint& prevPoint) const
{
    touchPoint.setStartPos( prevPoint.startPos( ));
    touchPoint.setStartScenePos( prevPoint.startScenePos( ));
    touchPoint.setStartScreenPos( prevPoint.startScreenPos( ));
    touchPoint.setStartNormalizedPos( prevPoint.startNormalizedPos( ));

    touchPoint.setLastPos( prevPoint.pos( ));
    touchPoint.setLastScenePos( prevPoint.scenePos( ));
    touchPoint.setLastScreenPos( prevPoint.screenPos( ));
    touchPoint.setLastNormalizedPos( prevPoint.normalizedPos( ));
}

void MultiTouchListener::handleEvent( TUIO::TuioCursor* tcur,
                                      const QEvent::Type eventType )
{
    const QPointF scenePos = getScenePos( tcur );
    const QPoint viewPos = graphicsView_->mapFromScene( scenePos );
    const QPoint screenPos = graphicsView_->mapToGlobal( viewPos );
    const QPointF normalizedPos( tcur->getX(), tcur->getY( ));

    QTouchEvent::TouchPoint touchPoint( tcur->getCursorID( ));
    touchPoint.setPressure( 1.0 );
    // Note: According to Qt doc, touchPoint.pos() should be set to viewPos.
    // However it is set to scenePos here because QTapGesture and QPanGesture
    // use it, and gestures need to have scene positions for correct event
    // processing in this application.
    touchPoint.setPos( scenePos );
    touchPoint.setScenePos( scenePos );
    touchPoint.setScreenPos( screenPos ); // used for QGesture::hotspot & itemAt
    touchPoint.setNormalizedPos( normalizedPos );

    Qt::TouchPointStates touchPointStates = 0;
    if( tcur->getCursorID() == 0 )
        touchPointStates |= Qt::TouchPointPrimary;

    switch( eventType )
    {
    case QEvent::TouchBegin:
        touchPointStates = Qt::TouchPointPressed;
        fillBegin( touchPoint );
        break;

    case QEvent::TouchUpdate:
    case QEvent::TouchEnd:
    {
        if( eventType == QEvent::TouchUpdate )
            touchPointStates = tcur->isMoving() ? Qt::TouchPointMoved
                                                : Qt::TouchPointStationary;
        else
            touchPointStates = Qt::TouchPointReleased;

        const int id = tcur->getCursorID();
        const QTouchEvent::TouchPoint& prevPoint = touchPointMap_.value( id );
        fill( touchPoint, prevPoint );
        break;
    }

    default:
        put_flog( LOG_ERROR, "Got wrong touch event type %i", eventType );
        return;
    }

    touchPoint.setState( touchPointStates );
    touchPointMap_.insert( tcur->getCursorID(), touchPoint );

    QEvent* touchEvent = new QTouchEvent( eventType, QTouchEvent::TouchScreen,
                                          Qt::NoModifier, touchPointStates,
                                          touchPointMap_.values( ));
    QApplication::postEvent( graphicsView_->viewport(), touchEvent );

    if( eventType == QEvent::TouchEnd )
        touchPointMap_.remove( tcur->getCursorID( ));
}
