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

#ifndef TOUCHAREA_H
#define TOUCHAREA_H

#include <QtDeclarative/QDeclarativeItem>

class QGestureEvent;
class DoubleTapGesture;
class PanGesture;
class PinchGesture;
class QTapGesture;
class QSwipeGesture;
class QTapAndHoldGesture;

/**
 * A touch area for capturing touch events in QML.
 * Can be used in a similar way to Qt's MouseArea.
 */
class TouchArea : public QDeclarativeItem
{
    Q_OBJECT

public:
    explicit TouchArea( QDeclarativeItem* parentItem = 0 );
    virtual ~TouchArea();

signals:
    void touchBegin( QPointF position );
    void touchEnd( QPointF position );

    void tap( QPointF position );
    void doubleTap( QPointF position );
    void tapAndHold( QPointF position );

    void pan( QPointF position, QPointF delta );

    void pinch( QPointF position, qreal scaleFactor );

    void swipeLeft();
    void swipeRight();
    void swipeUp();
    void swipeDown();

protected:
    /** @name Re-implemented QGraphicsRectItem events */
    //@{
    bool sceneEvent( QEvent* event ) override;
    //@}

private:
    bool gestureEvent( QGestureEvent* event );
    void tapAndHold( QTapAndHoldGesture* gesture );
    void tap( QTapGesture* gesture );
    void doubleTap( DoubleTapGesture* gesture );
    void pan( PanGesture* gesture );
    void pinch( PinchGesture* gesture );
    void swipe( QSwipeGesture* gesture );

    bool _blockTapGesture;
};

#endif // TOUCHAREA_H
