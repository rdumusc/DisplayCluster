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
    _updateTiles( helper.toTilesArea( visibleArea, tilesSurface ), lod );
}

void DynamicTextureSynchronizer::synchronize( WallToWallChannel& channel )
{
    Q_UNUSED( channel );
}

bool DynamicTextureSynchronizer::needRedraw() const
{
    return _reader->hasPendingTileLoads();
}

bool DynamicTextureSynchronizer::allowsTextureCaching() const
{
    return true;
}

Tiles& DynamicTextureSynchronizer::getTiles()
{
    return _tiles;
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

ImagePtr DynamicTextureSynchronizer::getTileImage( const uint tileIndex ) const
{
    return std::make_shared<QtImage>( _reader->getTileImage( tileIndex ));
}

void DynamicTextureSynchronizer::_updateTiles( const QRectF& visibleArea,
                                               const uint lod )
{
    if( visibleArea == _visibleArea && lod == _lod )
        return;

    _visibleArea = visibleArea;

    if( lod != _lod )
    {
        _tiles.reset( TileList( ));
        if( !_lodTilesMap.count( lod ))
            _lodTilesMap[ lod ] = _gatherAllTiles( lod );

        _lod = lod;
        _visibleSet.clear();
        _reader->cancelPendingTileLoads();

        emit statisticsChanged();
        emit tilesAreaChanged();
    }

    const TileList& tiles = _lodTilesMap[ lod ];

    const Indices visibleSet = _computeVisibleSet( visibleArea, tiles );

    const Indices addedTiles = set_difference( visibleSet, _visibleSet );
    const Indices removedTiles = set_difference( _visibleSet, visibleSet );


    // Remove tiles
    if( !removedTiles.empty( ))
    {
        _reader->cancelPendingTileLoads();
        for( auto i : removedTiles )
            _tiles.remove( tiles[i]->getIndex( ));

        // Retrigger tile loads which might have been canceled above
        const Indices current = set_difference( _visibleSet, removedTiles );
        for( auto i : current )
            _reader->triggerTileLoad( _tiles.get( tiles[i]->getIndex( )));
    }

    // Add tiles
    for( auto i : addedTiles )
    {
        auto tile = make_unique<Tile>( tiles[i]->getIndex(),
                                       tiles[i]->getCoord(), false );
        _reader->triggerTileLoad( tile.get( ));
        _tiles.add( std::move( tile ));
    }

    _visibleSet = visibleSet;
}

TileList DynamicTextureSynchronizer::_gatherAllTiles( const uint lod ) const
{
    TileList tiles;

    const QSize tilesCount = _reader->getTilesCount( lod );
    uint tileIndex = _reader->getFirstTileIndex( lod );
    for( int y = 0; y < tilesCount.height(); ++y )
    {
        for( int x = 0; x < tilesCount.width(); ++x )
        {
            const QRect& coord = _reader->getTileCoord( lod, x, y );
            tiles.push_back( make_unique<Tile>( tileIndex, coord, false ));
            ++tileIndex;
        }
    }

    return tiles;
}

Indices
DynamicTextureSynchronizer::_computeVisibleSet( const QRectF& visibleArea,
                                                const TileList& tiles ) const
{
    Indices visibleTiles;

    if( visibleArea.isEmpty( ))
        return visibleTiles;

    const QRect rectArea = visibleArea.toRect();

    int i = 0;
    for( const auto& tile : tiles )
    {
        if( tile->getCoord().intersects( rectArea ))
            visibleTiles.push_back( i );
        ++i;
    }

    return visibleTiles;
}
