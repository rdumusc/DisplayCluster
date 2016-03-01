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

#include "PDFTiler.h"

#include "LodTools.h"

namespace
{
const uint tileSize = 512;
const qreal maxScaleFactor = 5;
}

PDFTiler::PDFTiler( PDF& pdf )
    : _pdf( pdf )
    , _lodTool( _pdf.getSize() * maxScaleFactor, tileSize )
{}

QImage PDFTiler::getTileImage( const uint tileId,
                               const uint64_t timestamp ) const
{
    Q_UNUSED( timestamp );

    const QRect tile = getTileRect( tileId );
    return _pdf.renderToImage( tile.size(), _getNormalizedTileRect( tileId ));
}

QRect PDFTiler::getTileRect( const uint tileId ) const
{
    return _lodTool.getTileCoord( tileId );
}

QSize PDFTiler::getTilesArea( const uint lod ) const
{
    return _lodTool.getTilesArea( lod );
}

Indices PDFTiler::computeVisibleSet( const QRectF& visibleTilesArea,
                                     const uint lod ) const
{
    return _lodTool.getVisibleTiles( visibleTilesArea, lod );
}

uint PDFTiler::getMaxLod() const
{
    return _lodTool.getMaxLod();
}

QRectF PDFTiler::_getNormalizedTileRect( const uint tileId ) const
{
    const QRectF tile( getTileRect( tileId ));
    const uint lod = _lodTool.getTileIndex( tileId ).lod;
    const QSize area = getTilesArea( lod );

    const auto t = QTransform::fromScale( 1.0 / area.width(),
                                          1.0 / area.height( ));
    return t.mapRect( tile );
}
