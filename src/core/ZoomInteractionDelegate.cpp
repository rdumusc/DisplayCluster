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

#include "ZoomInteractionDelegate.h"
#include "ContentWindow.h"
#include "gestures/PanGesture.h"
#include "gestures/PinchGesture.h"

#define ZOOM_PAN_GAIN_FACTOR  2.0

ZoomInteractionDelegate::ZoomInteractionDelegate( ContentWindow& contentWindow )
    : ContentInteractionDelegate( contentWindow )
{
}

void ZoomInteractionDelegate::pan( PanGesture* gesture )
{
    const QPointF delta = computeZoomPanDelta( gesture->delta( ));
    contentWindow_.setZoomCenter( contentWindow_.getZoomCenter() - delta );
}

void ZoomInteractionDelegate::pinch( PinchGesture* gesture )
{
    const double factor = adaptZoomFactor( gesture->scaleFactor( ));
    if( factor == 0.0 )
        return;

    contentWindow_.setZoom( contentWindow_.getZoom() * factor );
}

void ZoomInteractionDelegate::mouseMoveEvent( QGraphicsSceneMouseEvent* event )
{
    const QPointF mouseDelta = event->scenePos() - event->lastScenePos();

    if( event->buttons().testFlag( Qt::RightButton ))
    {
        const double zoomDelta = 1.0 - getNormalizedDelta( mouseDelta ).y();
        contentWindow_.setZoom( contentWindow_.getZoom() * zoomDelta );
    }
    else if( event->buttons().testFlag( Qt::LeftButton ))
    {
        const QPointF delta = computeZoomPanDelta( mouseDelta );
        contentWindow_.setZoomCenter( contentWindow_.getZoomCenter() + delta );
    }
}

void ZoomInteractionDelegate::wheelEvent( QGraphicsSceneWheelEvent* event )
{
    // change zoom based on wheel delta.
    // deltas are counted in 1/8 degrees, so scale based on 180 degrees =>
    // delta = 180*8 = 1440
    const double zoomDelta = (double)event->delta() / 1440.0;
    contentWindow_.setZoom( contentWindow_.getZoom() * ( 1.0 + zoomDelta ));
}

QPointF
ZoomInteractionDelegate::computeZoomPanDelta( const QPointF& sceneDelta ) const
{
    const QPointF normalizedDelta = getNormalizedDelta( sceneDelta );
    const qreal zoom = contentWindow_.getZoom();
    return QPointF( normalizedDelta * ( ZOOM_PAN_GAIN_FACTOR / zoom ));
}

QPointF
ZoomInteractionDelegate::getNormalizedDelta( const QPointF& sceneDelta ) const
{
    const QRectF& window = contentWindow_.getCoordinates();
    return QPointF ( sceneDelta.x() / window.width(),
                     sceneDelta.y() / window.height( ));
}

double ZoomInteractionDelegate::adaptZoomFactor( const double
                                                 pinchGestureScaleFactor )
{
    const double factor = ( pinchGestureScaleFactor - 1.0 ) * 0.2 + 1.0;
    if( std::isnan( factor ) || std::isinf( factor ))
        return 0.0;
    return factor;
}
