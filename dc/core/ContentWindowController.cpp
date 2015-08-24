/*********************************************************************/
/* Copyright (c) 2014, EPFL/Blue Brain Project                       */
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

#include "ContentWindowController.h"

#include "ContentWindow.h"
#include "DisplayGroup.h"

#include <QTransform>

namespace
{
const qreal MIN_SIZE = 0.05;
const qreal MIN_VISIBLE_AREA_PX = 300.0;
const qreal INSIDE_MARGIN = 0.05;
const qreal LARGE_SIZE_SCALE = 0.75;
const qreal WINDOW_CONTROLS_MARGIN_PX = 175.0;
}

ContentWindowController::ContentWindowController()
    : contentWindow_( 0 )
    , displayGroup_( 0 )
{
}

ContentWindowController::ContentWindowController( ContentWindow& contentWindow,
                                                  const DisplayGroup& displayGroup )
    : contentWindow_( &contentWindow )
    , displayGroup_( &displayGroup )
{
}

void ContentWindowController::resize( const QSizeF size,
                                      const WindowPoint fixedPoint )
{
    QSizeF newSize( contentWindow_->getContent()->getDimensions( ));
    if( newSize.isEmpty( ))
        newSize = size;
    else
        newSize.scale( size, Qt::KeepAspectRatio );

    switch( fixedPoint )
    {
    case CENTER:
        resize_( contentWindow_->getCoordinates().center(), newSize );
        break;
    case TOP_LEFT:
    default:
        resize_( contentWindow_->getCoordinates().topLeft(), newSize );
    }
}

void ContentWindowController::resizeRelative( const QPointF& delta )
{
    QRectF coordinates( contentWindow_->getCoordinates( ));
    switch( contentWindow_->getBorder( ))
    {
    case ContentWindow::TOP:
        coordinates.adjust( 0, delta.y(), 0, 0 );
        break;
    case ContentWindow::RIGHT:
        coordinates.adjust( 0, 0, delta.x(), 0 );
        break;
    case ContentWindow::BOTTOM:
        coordinates.adjust( 0, 0, 0, delta.y( ));
        break;
    case ContentWindow::LEFT:
        coordinates.adjust( delta.x(), 0, 0, 0 );
        break;
    case ContentWindow::TOP_LEFT:
        resize_( coordinates.bottomRight(),
                 coordinates.size() - QSizeF( delta.x(), delta.y( )));
        return;
    case ContentWindow::BOTTOM_LEFT:
        resize_( coordinates.topRight(),
                 coordinates.size() - QSizeF( delta.x(), -delta.y( )));
        return;
    case ContentWindow::TOP_RIGHT:
        resize_( coordinates.bottomLeft(),
                 coordinates.size() - QSizeF( -delta.x(), delta.y( )));
        return;
    case ContentWindow::BOTTOM_RIGHT:
        resize_( coordinates.topLeft(),
                 coordinates.size() + QSizeF( delta.x(), delta.y( )));
        return;
    case ContentWindow::NOBORDER:
    default:
        return;
    }
    QSizeF newSize( coordinates.size( ));
    constrainSize_( newSize );
    coordinates.setSize( newSize );

    constrainPosition_( coordinates );
    contentWindow_->setCoordinates( coordinates );
}

void ContentWindowController::resize_( const QPointF& center, QSizeF size )
{
    constrainSize_( size );

    QRectF coordinates( contentWindow_->getCoordinates( ));
    QTransform transform;
    transform.translate( center.x(), center.y( ));
    transform.scale( size.width()/coordinates.width(),
                     size.height()/coordinates.height( ));
    transform.translate( -center.x(), -center.y( ));

    coordinates = transform.mapRect( coordinates );
    constrainPosition_( coordinates );
    contentWindow_->setCoordinates( coordinates );
}

void ContentWindowController::scale( const QPointF& center, const double factor)
{
    if( factor <= 0.0 )
        return;

    resize_( center, contentWindow_->getCoordinates().size() * factor );
}

void ContentWindowController::adjustSize( const SizeState state )
{
    switch( state )
    {
    case SIZE_1TO1:
        resize( contentWindow_->getContent()->getDimensions(), CENTER );
        break;

    case SIZE_LARGE:
    {
        const QSizeF wallSize = displayGroup_->getCoordinates().size();
        resize( LARGE_SIZE_SCALE * wallSize, CENTER );
    } break;

    case SIZE_FULLSCREEN:
    {
        QSizeF size = contentWindow_->getContent()->getDimensions();
        size.scale( displayGroup_->getCoordinates().size(),
                    Qt::KeepAspectRatio );
        constrainSize_( size );
        contentWindow_->setCoordinates( getCenteredCoordinates_( size ));
    } break;

    default:
        return;
    }
}

void ContentWindowController::moveTo( const QPointF& position,
                                      const WindowPoint handle )
{
    QRectF coordinates( contentWindow_->getCoordinates( ));
    switch( handle )
    {
    case TOP_LEFT:
        coordinates.moveTopLeft( position );
        break;
    case CENTER:
        coordinates.moveCenter( position );
        break;
    default:
        return;
    }
    constrainPosition_( coordinates );

    contentWindow_->setCoordinates( coordinates );
}

QSizeF ContentWindowController::getMinSize() const
{
    const QSizeF& wallSize = displayGroup_->getCoordinates().size();
    return QSizeF( std::max( MIN_SIZE * wallSize.width(), MIN_VISIBLE_AREA_PX ),
                   std::max( MIN_SIZE * wallSize.height(),
                             MIN_VISIBLE_AREA_PX ));
}

QSizeF ContentWindowController::getMaxSize() const
{
    QSizeF maxSize = getMaxContentSize();
    maxSize *= ContentWindow::getMaxContentScale();
    maxSize.rwidth() *= contentWindow_->getZoomRect().size().width();
    maxSize.rheight() *= contentWindow_->getZoomRect().size().height();
    return maxSize;
}

QSizeF ContentWindowController::getMaxContentSize() const
{
    QSizeF maxContentSize = contentWindow_->getContent()->getMaxDimensions();
    if( maxContentSize.isEmpty() || maxContentSize == UNDEFINED_SIZE )
        maxContentSize = displayGroup_->getCoordinates().size();
    return maxContentSize;
}

QRectF ContentWindowController::getFocusedCoord() const
{
    const qreal margin = 2.0 * getInsideMargin_();
    const QSizeF margins( margin + WINDOW_CONTROLS_MARGIN_PX, margin );
    const QSizeF& wallSize = displayGroup_->getCoordinates().size();
    const QSizeF maxSize = wallSize.boundedTo( wallSize - margins );

    QSizeF size = contentWindow_->getContent()->getDimensions();
    size.scale( maxSize, Qt::KeepAspectRatio );
    constrainSize_( size );

    const qreal x = contentWindow_->getCoordinates().center().x();
    QRectF coord( QPointF(), size );
    coord.moveCenter( QPointF( x, wallSize.height() * 0.5 ));
    constrainFullyInside_( coord );
    return coord;
}

void ContentWindowController::constrainSize_( QSizeF& windowSize ) const
{
    const QSizeF& maxSize = getMaxSize();
    if( windowSize > maxSize )
    {
        windowSize.scale( maxSize, Qt::KeepAspectRatio );
        return;
    }

    const QSizeF& minSize = getMinSize();
    if( windowSize < minSize )
        windowSize = contentWindow_->getCoordinates().size();
}

void ContentWindowController::constrainPosition_( QRectF& window ) const
{
    const QRectF& group = displayGroup_->getCoordinates();

    const qreal minX = MIN_VISIBLE_AREA_PX - window.width();
    const qreal minY = MIN_VISIBLE_AREA_PX - window.height();

    const qreal maxX = group.width() - MIN_VISIBLE_AREA_PX;
    const qreal maxY = group.height() - MIN_VISIBLE_AREA_PX;

    const QPointF position( std::max( minX, std::min( window.x(), maxX )),
                            std::max( minY, std::min( window.y(), maxY )));

    window.moveTopLeft( position );
}

void ContentWindowController::constrainFullyInside_( QRectF& window ) const
{
    const QRectF& group = displayGroup_->getCoordinates();

    const qreal margin = getInsideMargin_();
    const qreal minX = margin + WINDOW_CONTROLS_MARGIN_PX;
    const qreal minY = margin;
    const qreal maxX = group.width() - window.width() - margin;
    const qreal maxY = group.height() - window.height() - margin;

    const QPointF position( std::max( minX, std::min( window.x(), maxX )),
                            std::max( minY, std::min( window.y(), maxY )));

    window.moveTopLeft( position );
}

QRectF
ContentWindowController::getCenteredCoordinates_( const QSizeF& size ) const
{
    const QRectF& group = displayGroup_->getCoordinates();

    // centered coordinates on the display group
    QRectF coord( QPointF(), size );
    coord.moveCenter( group.center( ));
    return coord;
}

qreal ContentWindowController::getInsideMargin_() const
{
    return displayGroup_->getCoordinates().height() * INSIDE_MARGIN;
}
