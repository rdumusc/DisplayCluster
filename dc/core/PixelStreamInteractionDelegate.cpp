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

PixelStreamInteractionDelegate::PixelStreamInteractionDelegate( ContentWindow&
                                                                contentWindow )
    : ContentInteractionDelegate( contentWindow )
{
}

void PixelStreamInteractionDelegate::tap( const QPointF position )
{
    deflect::Event deflectEvent = _getNormEvent( position );
    deflectEvent.type = deflect::Event::EVT_CLICK;

    _contentWindow.dispatchEvent( deflectEvent );
}

void PixelStreamInteractionDelegate::doubleTap( const QPointF position )
{
    deflect::Event deflectEvent = _getNormEvent( position );
    deflectEvent.type = deflect::Event::EVT_DOUBLECLICK;

    _contentWindow.dispatchEvent( deflectEvent );
}

void PixelStreamInteractionDelegate::tapAndHold( const QPointF position )
{
    deflect::Event deflectEvent = _getNormEvent( position );
    deflectEvent.type = deflect::Event::EVT_TAP_AND_HOLD;

    _contentWindow.dispatchEvent( deflectEvent );
}

void PixelStreamInteractionDelegate::pan( const QPointF position,
                                          const QPointF delta )
{
    deflect::Event deflectEvent = _getNormEvent( position );

    if( delta.isNull( ))
        deflectEvent.type = deflect::Event::EVT_PRESS;
    else
        deflectEvent.type = deflect::Event::EVT_MOVE;

    deflectEvent.dx = delta.x() / _contentWindow.getCoordinates().width();
    deflectEvent.dy = delta.y() / _contentWindow.getCoordinates().height();

    _contentWindow.dispatchEvent( deflectEvent );
}

void PixelStreamInteractionDelegate::panFinished( const QPointF position )
{
    deflect::Event deflectEvent = _getNormEvent( position );
    deflectEvent.type = deflect::Event::EVT_RELEASE;

    _contentWindow.dispatchEvent( deflectEvent );
}

void PixelStreamInteractionDelegate::pinch( const QPointF position,
                                            const qreal scaleFactor )
{
    deflect::Event deflectEvent = _getNormEvent( position );
    deflectEvent.type = deflect::Event::EVT_WHEEL;
    deflectEvent.mouseLeft = false;
    deflectEvent.dy = scaleFactor - 1.0;

    _contentWindow.dispatchEvent( deflectEvent );
}

deflect::Event swipeEvent( deflect::Event::EventType type )
{
    deflect::Event event;
    event.type = type;
    return event;
}

void PixelStreamInteractionDelegate::swipeLeft()
{
    _contentWindow.dispatchEvent( swipeEvent( deflect::Event::EVT_SWIPE_LEFT ));
}

void PixelStreamInteractionDelegate::swipeRight()
{
    _contentWindow.dispatchEvent( swipeEvent( deflect::Event::EVT_SWIPE_RIGHT));
}

void PixelStreamInteractionDelegate::swipeUp()
{
    _contentWindow.dispatchEvent( swipeEvent( deflect::Event::EVT_SWIPE_UP ));
}

void PixelStreamInteractionDelegate::swipeDown()
{
    _contentWindow.dispatchEvent( swipeEvent( deflect::Event::EVT_SWIPE_DOWN ));
}

void PixelStreamInteractionDelegate::keyPress( const int key,
                                               const int modifiers,
                                               const QString text )
{
    deflect::Event deflectEvent;
    deflectEvent.type = deflect::Event::EVT_KEY_PRESS;
    deflectEvent.key = key;
    deflectEvent.modifiers = modifiers;
    strncpy( deflectEvent.text, text.toStdString().c_str(),
             sizeof( deflectEvent.text ));

    _contentWindow.dispatchEvent( deflectEvent );
}

void PixelStreamInteractionDelegate::keyRelease( const int key,
                                                 const int modifiers,
                                                 const QString text )
{
    deflect::Event deflectEvent;
    deflectEvent.type = deflect::Event::EVT_KEY_RELEASE;
    deflectEvent.key = key;
    deflectEvent.modifiers = modifiers;
    strncpy( deflectEvent.text, text.toStdString().c_str(),
             sizeof( deflectEvent.text ));

    _contentWindow.dispatchEvent( deflectEvent );
}

deflect::Event PixelStreamInteractionDelegate::_getNormEvent( const QPointF&
                                                              position ) const
{
    // For some QGestures, position() is a screen position (Qt global).
    // However, QTapGesture has a scene position because it uses touchPoint.pos,
    // which is intentionally set to scenePos in MultiTouchListener.
    // For custom gesture classes, we explicitly define the position to be
    // the scene position, so this method is correct for the following gestures:
    // QTapGesture, DoubleTapGesture, PanGesture, PinchGesture
    // Should also work for the same reason as QTapGesture, but untested:
    // QTapAndHoldGesture, QPanGesture

    const QRectF& win = _contentWindow.getCoordinates();

    deflect::Event deflectEvent;
    deflectEvent.mouseLeft = true;
    deflectEvent.mouseX = ( position.x() - win.x( )) / win.width();
    deflectEvent.mouseY = ( position.y() - win.y( )) / win.height();
    return deflectEvent;
}
