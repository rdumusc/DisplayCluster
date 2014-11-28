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
}

ContentWindowController::ContentWindowController( ContentWindow& contentWindow,
                                                  const DisplayGroup& displayGroup )
    : contentWindow_( contentWindow )
    , displayGroup_( displayGroup )
    , sizeState_( SIZE_NORMALIZED )
{
}

void ContentWindowController::adjustSize( const SizeState state )
{
    switch( state )
    {
    case SIZE_FULLSCREEN:
    {
        contentWindow_.coordinatesBackup_ = contentWindow_.coordinates_;

        QSizeF size = contentWindow_.getContent()->getDimensions();
        size.scale( displayGroup_.getCoordinates().size(),
                    Qt::KeepAspectRatio );
        contentWindow_.setCoordinates( getCenteredCoordinates( size ));
    }
        break;

    case SIZE_1TO1:
    {
        QSizeF size = contentWindow_.getContent()->getDimensions();
        clampSize( size );
        contentWindow_.setCoordinates( getCenteredCoordinates( size ));
    }
        break;

    case SIZE_NORMALIZED:
        contentWindow_.setCoordinates( contentWindow_.coordinatesBackup_ );
        break;
    default:
        return;
    }

    sizeState_ = state;
}

void ContentWindowController::constrainPosition( const QSizeF& minArea ) const
{
    const QRectF& window = contentWindow_.getCoordinates();
    const QRectF& group = displayGroup_.getCoordinates();

    const QSizeF offset = minArea.isEmpty() ? window.size() : minArea;

    const qreal minX = offset.width() - window.width();
    const qreal minY = offset.height() - window.height();

    const qreal maxX = group.width() - offset.width();
    const qreal maxY = group.height() - offset.height();

    const QPointF pos( std::max( minX, std::min( window.x(), maxX )),
                       std::max( minY, std::min( window.y(), maxY )));

    contentWindow_.setPosition( pos );
}

bool ContentWindowController::isValidSize( const QSizeF& size ) const
{
    const qreal max_size = MAX_SIZE * displayGroup_.getCoordinates().width();
    const qreal min_size = MIN_SIZE * displayGroup_.getCoordinates().height();

    return ( size.width() >= min_size && size.height() >= min_size &&
             size.width() <= max_size && size.height() <= max_size );
}

void ContentWindowController::toggleFullscreen()
{
    adjustSize( sizeState_ == SIZE_NORMALIZED ? SIZE_FULLSCREEN : SIZE_NORMALIZED );
}

SizeState ContentWindowController::getSizeState() const
{
    return sizeState_;
}

void ContentWindowController::clampSize( QSizeF& size ) const
{
    const qreal max_size = MAX_SIZE * displayGroup_.getCoordinates().width();
    const qreal min_size = MIN_SIZE * displayGroup_.getCoordinates().height();

    if ( size.width() > max_size || size.height() > max_size )
        size.scale( max_size, max_size, Qt::KeepAspectRatio );

    if ( size.width() < min_size || size.height() < min_size )
        size.scale( min_size, min_size, Qt::KeepAspectRatioByExpanding );
}

QRectF ContentWindowController::getCenteredCoordinates( const QSizeF& size ) const
{
    const qreal totalWidth = displayGroup_.getCoordinates().width();
    const qreal totalHeight = displayGroup_.getCoordinates().height();

    // center on the wall
    return QRectF((totalWidth - size.width()) * 0.5,
                  (totalHeight - size.height()) * 0.5,
                  size.width(), size.height( ));
}
