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

#include <QTransform>

#define MIN_ZOOM 1.0

ZoomInteractionDelegate::ZoomInteractionDelegate( ContentWindow& contentWindow )
    : ContentInteractionDelegate( contentWindow )
{
}

ZoomInteractionDelegate::~ZoomInteractionDelegate()
{
}

void ZoomInteractionDelegate::pan( const QPointF position, const QPointF delta )
{
    Q_UNUSED( position );
    _moveZoomRect( delta );
}

void ZoomInteractionDelegate::pinch( const QPointF position,
                                     const qreal scaleFactor )
{
    const QPointF pos = position - _contentWindow.getCoordinates().topLeft();
    _scaleZoomRect( _getNormalizedPoint( pos ), 1.0 / scaleFactor );
}

void ZoomInteractionDelegate::_moveZoomRect( const QPointF& sceneDelta ) const
{
    QRectF zoomRect = _contentWindow.getZoomRect();
    const qreal zoom = zoomRect.width();
    const QPointF normalizedDelta = _getNormalizedPoint( sceneDelta ) * zoom;
    zoomRect.translate( -normalizedDelta );

    _constraintPosition( zoomRect );
    _contentWindow.setZoomRect( zoomRect );
}

void ZoomInteractionDelegate::_scaleZoomRect( const QPointF& center,
                                              const qreal zoomFactor ) const
{
    QRectF zoomRect = _contentWindow.getZoomRect();

    QTransform current;
    current.translate( zoomRect.x(), zoomRect.y( ));
    current.scale( zoomRect.width(), zoomRect.height( ));
    const QPointF point = current.map( center );

    QTransform transform;
    transform.translate( point.x(), point.y( ));
    transform.scale( zoomFactor, zoomFactor );
    transform.translate( -point.x(), -point.y( ));
    zoomRect = transform.mapRect( zoomRect );

    const QSizeF contentSize( _contentWindow.getContent()->getDimensions( ));
    const QSizeF windowSize( _contentWindow.getCoordinates().size( ));
    const qreal deltaW = contentSize.width() / windowSize.width();
    const qreal deltaH = contentSize.height() / windowSize.height();
    const qreal maxZoomW = 1.0 / ContentWindow::getMaxContentScale() / deltaW;
    const qreal maxZoomH = 1.0 / ContentWindow::getMaxContentScale() / deltaH;

    // constrain max zoom
    if( zoomRect.width() < maxZoomW || zoomRect.height() < maxZoomH )
        zoomRect = _contentWindow.getZoomRect();

    // constrain min zoom
    if( zoomRect.width() > MIN_ZOOM || zoomRect.height() > MIN_ZOOM )
        zoomRect = UNIT_RECTF;
    else
        _constraintPosition( zoomRect );
    _contentWindow.setZoomRect( zoomRect );
}

void ZoomInteractionDelegate::_constraintPosition( QRectF& zoomRect ) const
{
    if( zoomRect.left() < 0.0 )
        zoomRect.moveLeft( 0.0 );
    if( zoomRect.right() > 1.0 )
        zoomRect.moveRight( 1.0 );
    if( zoomRect.top() < 0.0 )
        zoomRect.moveTop( 0.0 );
    if( zoomRect.bottom() > 1.0 )
        zoomRect.moveBottom( 1.0 );
}

QPointF
ZoomInteractionDelegate::_getNormalizedPoint( const QPointF& point ) const
{
    const QRectF& window = _contentWindow.getCoordinates();
    return QPointF( point.x() / window.width(),
                    point.y() / window.height( ));
}
