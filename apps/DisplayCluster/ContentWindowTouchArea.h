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

#ifndef CONTENT_WINDOW_TOUCH_AREA_H
#define CONTENT_WINDOW_TOUCH_AREA_H

#include "types.h"

#include "ContentWindowController.h"

#include <QtDeclarative/QDeclarativeItem>

class QGestureEvent;
class DoubleTapGesture;
class PanGesture;
class PinchGesture;
class QTapGesture;
class QSwipeGesture;
class QTapAndHoldGesture;

/**
 * Handle ContentWindow interactions in a QML View.
 */
class ContentWindowTouchArea : public QDeclarativeItem
{
    Q_OBJECT

public:
    /** Constructor. */
    ContentWindowTouchArea();

    /** Destructor. */
    virtual ~ContentWindowTouchArea();

    /** Init must be separate from the constructor for instanciation in QML. */
    void init( ContentWindowPtr contentWindow,
               const DisplayGroup& displayGroup );

    /** Get the window controller for exposing it in the Qml context. */
    ContentWindowController* getWindowController();

signals:
    /** Emitted whenever the user interacts with the touch area. */
    void activated();

protected:
    /** @name Re-implemented QGraphicsRectItem events */
    //@{
    bool sceneEvent( QEvent* event ) override;
    void mouseMoveEvent( QGraphicsSceneMouseEvent* event ) override;
    void mousePressEvent( QGraphicsSceneMouseEvent* event ) override;
    void mouseDoubleClickEvent( QGraphicsSceneMouseEvent* event ) override;
    void mouseReleaseEvent( QGraphicsSceneMouseEvent* event ) override;
    void wheelEvent( QGraphicsSceneWheelEvent* event ) override;
    void keyPressEvent( QKeyEvent* event ) override;
    void keyReleaseEvent( QKeyEvent* event ) override;
    //@}

private:
    void gestureEvent( QGestureEvent* event );
    void doubleTap( DoubleTapGesture* gesture );
    void pan( PanGesture* gesture );
    void pinch( PinchGesture* gesture );
    void tapAndHold( QTapAndHoldGesture* gesture );

    ContentWindowPtr contentWindow_;
    ContentWindowController* controller_;
};

#endif
