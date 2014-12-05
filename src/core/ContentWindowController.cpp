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

namespace
{
const qreal MAX_SIZE = 2.0;
const qreal MIN_SIZE = 0.05;
const qreal MIN_VISIBLE_AREA_PX = 50.0;
}

ContentWindowController::ContentWindowController( ContentWindow& contentWindow,
                                                  const DisplayGroup& displayGroup )
    : contentWindow_( contentWindow )
    , displayGroup_( displayGroup )
    , sizeState_( SIZE_NORMALIZED )
    , minArea_( MIN_VISIBLE_AREA_PX, MIN_VISIBLE_AREA_PX )
{
}

void ContentWindowController::resize( const QSizeF& size,
                                      const FixedPoint fixedPoint )
{
    QSizeF windowSize = size;

    constrainSize( windowSize );

    QRectF coordinates( contentWindow_.getCoordinates( ));
    coordinates.setSize( windowSize );
    if( fixedPoint == CENTER )
        coordinates.moveCenter( contentWindow_.getCoordinates().center( ));
    constrainPosition( coordinates );
    contentWindow_.setCoordinates( coordinates );
}

void ContentWindowController::scale( const double factor )
{
    if( factor <= 0.0 )
        return;

    resize( contentWindow_.getCoordinates().size() * factor, CENTER );
}

void ContentWindowController::adjustSize( const SizeState state )
{
    switch( state )
    {
    case SIZE_FULLSCREEN:
    {
        contentWindow_.backupCoordinates();

        QSizeF size = contentWindow_.getContent()->getDimensions();
        size.scale( displayGroup_.getCoordinates().size(),
                    Qt::KeepAspectRatio );
        contentWindow_.setCoordinates( getCenteredCoordinates( size ));
    }
        break;

    case SIZE_1TO1:
        resize( contentWindow_.getContent()->getDimensions(), CENTER );
        break;

    case SIZE_NORMALIZED:
        contentWindow_.restoreCoordinates();
        break;
    default:
        return;
    }

    sizeState_ = state;
}

void ContentWindowController::toggleFullscreen()
{
    adjustSize( sizeState_ == SIZE_NORMALIZED ? SIZE_FULLSCREEN : SIZE_NORMALIZED );
}

void ContentWindowController::moveTo( const QPointF& position )
{
    QRectF coordinates( contentWindow_.getCoordinates( ));
    coordinates.moveTopLeft( position );

    constrainPosition( coordinates );

    contentWindow_.setCoordinates( coordinates );
}

void ContentWindowController::toggleWindowState()
{
    if ( contentWindow_.getState() == ContentWindow::UNSELECTED )
        contentWindow_.setState( ContentWindow::SELECTED );
    else if ( contentWindow_.getState() == ContentWindow::SELECTED )
        contentWindow_.setState( ContentWindow::UNSELECTED );
}

void ContentWindowController::constrainSize( QSizeF& windowSize ) const
{
    const QSizeF max_size = MAX_SIZE * displayGroup_.getCoordinates().size();
    const QSizeF min_size = MIN_SIZE * displayGroup_.getCoordinates().size();

    if ( windowSize.width() > max_size.width() ||
         windowSize.height() > max_size.height( ))
        windowSize.scale( max_size, Qt::KeepAspectRatio );

    if ( windowSize.width() < min_size.width() ||
         windowSize.height() < min_size.height( ))
        windowSize.scale( min_size, Qt::KeepAspectRatioByExpanding );
}

void ContentWindowController::constrainPosition( QRectF& window ) const
{
    const QRectF& group = displayGroup_.getCoordinates();

    const qreal minX = minArea_.width() - window.width();
    const qreal minY = minArea_.height() - window.height();

    const qreal maxX = group.width() - minArea_.width();
    const qreal maxY = group.height() - minArea_.height();

    const QPointF position( std::max( minX, std::min( window.x(), maxX )),
                            std::max( minY, std::min( window.y(), maxY )));

    window.moveTopLeft( position );
}

QRectF ContentWindowController::getCenteredCoordinates( const QSizeF& size ) const
{
    const qreal totalWidth = displayGroup_.getCoordinates().width();
    const qreal totalHeight = displayGroup_.getCoordinates().height();

    // centered coordinates on the display group
    return QRectF((totalWidth - size.width()) * 0.5,
                  (totalHeight - size.height()) * 0.5,
                  size.width(), size.height( ));
}
