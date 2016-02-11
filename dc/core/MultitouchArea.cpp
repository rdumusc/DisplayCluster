/*********************************************************************/
/* Copyright (c) 2016, EPFL/Blue Brain Project                       */
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

#include "MultitouchArea.h"

#include <QQuickWindow>
#include <cmath>

namespace
{
const qreal wheelFactor = 0.3;
const int invalidPointId = -1;
const qreal defaultPanThreshold = 20;
const qreal pinchThreshold = 10;
const qreal doubleTapThreshold = 40;
const uint tapAndHoldTimeout = 1000;
const uint doubleTapTimeout = 750;
}

MultitouchArea::MultitouchArea( QQuickItem* parent_ )
    : QQuickItem( parent_ )
    , _referenceItem( nullptr )
    , _panThreshold( defaultPanThreshold )
    , _panning( false )
    , _pinchDetectionStarted( false )
    , _pinching( false )
    , _lastPinchDist( 0.0 )
    , _tapCounter( 0 )
{
    setAcceptedMouseButtons( Qt::LeftButton );

    _tapAndHoldTimer.setInterval( tapAndHoldTimeout );
    connect( &_tapAndHoldTimer, &QTimer::timeout,
             this, [this]() { emit tapAndHold( _tapStartPos ); } );

    _doubleTapTimer.setInterval( doubleTapTimeout );
    connect( &_doubleTapTimer, &QTimer::timeout,
             this, [this]() { _tapCounter = 0; } );
}

QQuickItem* MultitouchArea::getReferenceItem() const
{
    return _referenceItem;
}

qreal MultitouchArea::getPanThreshold() const
{
    return _panThreshold;
}

void MultitouchArea::setReferenceItem( QQuickItem* arg )
{
    if( _referenceItem == arg )
        return;

    _referenceItem = arg;
    emit referenceItemChanged( arg );
}

void MultitouchArea::setPanThreshold( const qreal arg )
{
    if( _panThreshold == arg )
        return;

    _panThreshold = arg;
    emit panThresholdChanged( arg );
}

QPointF MultitouchArea::_getScenePos( QMouseEvent* mouse )
{
    return mapToItem( _referenceItem, mouse->localPos( ));
}

QPointF MultitouchArea::_getScenePos( const QTouchEvent::TouchPoint& point )
{
    return mapToItem( _referenceItem, point.pos( ));
}

QPointF MultitouchArea::_getScenePos( const QPointF& itemPos )
{
    return mapToItem( _referenceItem, itemPos );
}

void MultitouchArea::mousePressEvent( QMouseEvent* mouse )
{
    const auto pos = _getScenePos( mouse );

     _mousePrevPos = pos;
     emit tapStarted( pos );
}

void MultitouchArea::mouseMoveEvent( QMouseEvent* mouse )
{
    const auto pos = _getScenePos( mouse );

    const QPointF delta = pos - _mousePrevPos;
    _mousePrevPos = pos;

    _startPanGesture();
    emit pan( pos, delta );
}

void MultitouchArea::mouseReleaseEvent( QMouseEvent* mouse )
{
    if( !_panning )
         emit tapEnded( _getScenePos( mouse ));
    _cancelPanGesture();
}

void MultitouchArea::mouseDoubleClickEvent( QMouseEvent* mouse )
{
    emit doubleTap( _getScenePos( mouse ));
}

void MultitouchArea::wheelEvent( QWheelEvent* wheel )
{
    emit pinch( wheel->posF(), wheel->angleDelta().y() * wheelFactor );
}

void MultitouchArea::touchEvent( QTouchEvent* touch )
{
    const auto& points = touch->touchPoints();
    switch( points.size( ))
    {
    case 1:
        _handleSinglePoint( points.at( 0 ));
        break;
    case 2:
        _handleTwoPoints( points.at( 0 ), points.at( 1 ));
        break;
    default:
        break;
    }
}

void MultitouchArea::_handleSinglePoint( const QTouchEvent::TouchPoint& point )
{
    const QPointF pos = _getScenePos( point );

    switch( point.state( ))
    {
    case Qt::TouchPointPressed:
        _tapStartPos = pos;
        emit tapStarted( pos );

        if( _tapCounter == 0 )
            _startDoubleTapGesture();
        if( ++_tapCounter == 2 )
        {
            if( (pos - _tapStartPos).manhattanLength() < doubleTapThreshold )
                emit doubleTap( pos );
            _tapCounter = 0;
        }

        _startTapAndHoldGesture();
        break;

    case Qt::TouchPointStationary:
        break;

    case Qt::TouchPointMoved:
        if( !_panning && (pos - _tapStartPos).manhattanLength() > _panThreshold )
        {
            _startPanGesture();
            _cancelTapAndHoldGesture();
        }
        if( _panning )
            emit pan( pos, pos - _getScenePos( point.lastPos( )));
        break;

    case Qt::TouchPointReleased:
        if( !_panning )
             emit tapEnded( pos );
        _cancelPanGesture();
        _cancelTapAndHoldGesture();
        break;
    }
}

void MultitouchArea::_startPanGesture()
{
    if( _panning )
        return;

    _panning = true;
    emit panStarted();
}

void MultitouchArea::_cancelPanGesture()
{
    if( !_panning )
        return;

    _panning = false;
    emit panEnded();
}

void MultitouchArea::_startTapAndHoldGesture()
{
    _tapAndHoldTimer.start();
}

void MultitouchArea::_cancelTapAndHoldGesture()
{
    _tapAndHoldTimer.stop();
}

void MultitouchArea::_startDoubleTapGesture()
{
    _doubleTapTimer.start();
}

void MultitouchArea::_cancelDoubleTapGesture()
{
    _doubleTapTimer.stop();
    _tapCounter = 0;
}

qreal _getDist( const QPointF& p0, const QPointF& p1 )
{
    const QPointF dist = p1 - p0;
    return std::sqrt( QPointF::dotProduct( dist, dist ));
}

qreal MultitouchArea::_getPinchDistance( const QTouchEvent::TouchPoint& p0,
                                         const QTouchEvent::TouchPoint& p1 )
{
    return _getDist(_getScenePos( p0 ), _getScenePos( p1 )) - _initialPinchDist;
}

void MultitouchArea::_handleTwoPoints( const QTouchEvent::TouchPoint& p0,
                                       const QTouchEvent::TouchPoint& p1 )
{
    if( p0.state() == Qt::TouchPointReleased ||
            p1.state() == Qt::TouchPointReleased )
    {
        _pinchDetectionStarted = false;
        _pinching = false;
        return;
    }

    if( !_pinchDetectionStarted )
    {
        _pinchDetectionStarted = true;
        _initialPinchDist = _getDist( _getScenePos( p0 ), _getScenePos( p1 ));
        _cancelPanGesture();
        _cancelTapAndHoldGesture();
        _cancelDoubleTapGesture();
    }

    if( !_pinching && std::abs( _getPinchDistance( p0, p1 )) > pinchThreshold )
    {
        _pinching = true;
        _lastPinchDist = _getPinchDistance( p0, p1 );
    }

    if( _pinching )
    {
        const auto pinchDist = _getPinchDistance( p0, p1 );
        const auto pinchDelta = pinchDist - _lastPinchDist;
        _lastPinchDist = pinchDist;
        const auto pinchCenter = ( _getScenePos( p0 ) + _getScenePos( p1 )) / 2;
        emit pinch( pinchCenter, pinchDelta );
    }
}
