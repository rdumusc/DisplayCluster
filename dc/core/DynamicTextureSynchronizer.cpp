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

#include "DynamicTextureSynchronizer.h"

#include "Tile.h"
#include "QtImage.h"
#include "ZoomHelper.h"

#include <QTextStream>

DynamicTextureSynchronizer::DynamicTextureSynchronizer( const QString& uri )
    : _reader( new DynamicTexture( uri ))
    , _lod( 0 )
{
}

void DynamicTextureSynchronizer::update( const ContentWindow& window,
                                         const QRectF& visibleArea )
{
    const ZoomHelper helper( window );
    const uint lod = _reader->getLod( helper.getContentRect().size().toSize( ));
    const QSize tilesSurface = _reader->getTilesArea( lod );
    const QRectF visibleTilesArea = helper.toTilesArea( visibleArea,
                                                        tilesSurface );

    if( visibleTilesArea == _visibleTilesArea && lod == _lod )
        return;

    _visibleTilesArea = visibleTilesArea;

    if( lod != _lod )
    {
        _lod = lod;

        emit statisticsChanged();
        emit tilesAreaChanged();
    }

    _updateTiles();
}

void DynamicTextureSynchronizer::synchronize( WallToWallChannel& channel )
{
    Q_UNUSED( channel );
}

bool DynamicTextureSynchronizer::needRedraw() const
{
    return false;
}

bool DynamicTextureSynchronizer::allowsTextureCaching() const
{
    return true;
}

QSize DynamicTextureSynchronizer::getTilesArea() const
{
    return _reader->getTilesArea( _lod );
}

QString DynamicTextureSynchronizer::getStatistics() const
{
    QString stats;
    QTextStream stream( &stats );
    stream << "LOD:  " << _lod << "/" << _reader->getMaxLod();
    const QSize& area = getTilesArea();
    stream << "  res: " << area.width() << "x" << area.height();
    return stats;
}

ImagePtr DynamicTextureSynchronizer::getTileImage( const uint tileIndex,
                                                   const uint64_t timestamp ) const
{
    Q_UNUSED( timestamp );

    const QImage image = _reader->getTileImage( tileIndex );
    if( image.isNull( ))
        return ImagePtr();
    return std::make_shared<QtImage>( image );
}

void DynamicTextureSynchronizer::onSwapReady( TilePtr tile )
{
    tile->swapImage();
}

void DynamicTextureSynchronizer::_updateTiles()
{
    const Indices visibleSet = _reader->computeVisibleSet( _visibleTilesArea,
                                                           _lod );

    const Indices addedTiles = set_difference( visibleSet, _visibleSet );
    const Indices removedTiles = set_difference( _visibleSet, visibleSet );

    for( auto i : removedTiles )
        emit removeTile( i );

    for( auto i : addedTiles )
        emit addTile( std::make_shared<Tile>( i, _reader->getTileCoord( i )));

    _visibleSet = visibleSet;
}
