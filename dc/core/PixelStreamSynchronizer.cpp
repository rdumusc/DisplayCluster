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

#include "PixelStreamSynchronizer.h"

#include "ContentWindow.h"
#include "PixelStreamUpdater.h"
#include "QtImage.h"
#include "Tile.h"
#include "WallToWallChannel.h"
#include "ZoomHelper.h"

PixelStreamSynchronizer::PixelStreamSynchronizer()
    : _frameIndex( 0 )
    , _tilesDirty( true )
    , _updateExistingTiles( false )
{}

void
PixelStreamSynchronizer::setDataSource( PixelStreamUpdaterSharedPtr updater )
{
    if( _updater )
        _updater->disconnect( this );

    _updater = updater;

    connect( _updater.get(), &PixelStreamUpdater::pictureUpdated,
             this, &PixelStreamSynchronizer::_onPictureUpdated );
}

void PixelStreamSynchronizer::update( const ContentWindow& window,
                                      const QRectF& visibleArea )
{
    // Tiles area corresponds to Content dimensions for PixelStreams
    const QSize tilesSurface = window.getContent()->getDimensions();

    ZoomHelper helper( window );
    const QRectF visibleTilesArea = helper.toTilesArea( visibleArea,
                                                        tilesSurface );

    if( _visibleTilesArea == visibleTilesArea )
        return;

    _visibleTilesArea = visibleTilesArea;
    _tilesDirty = true;
}

void PixelStreamSynchronizer::synchronize( WallToWallChannel& channel )
{
    if( !_updater )
        return;

    const bool swap = !_syncSet.empty() &&
                      set_difference( _syncSet, _tilesReadySet ).empty();

    if( channel.allReady( swap ))
    {
        for( auto& tile : _tilesReadyToSwap )
            tile->swapImage();
        _tilesReadyToSwap.clear();
        _tilesReadySet.clear();
        _syncSet.clear();

        _fpsCounter.tick();
        emit statisticsChanged();

        _updater->getNextFrame();
    }

    if( _tilesDirty )
    {
        _updateTiles();
        _tilesDirty = false;
        _updateExistingTiles = false;
    }
}

bool PixelStreamSynchronizer::needRedraw() const
{
    return false;
}

bool PixelStreamSynchronizer::allowsTextureCaching() const
{
    return false;
}

QSize PixelStreamSynchronizer::getTilesArea() const
{
    return QSize();
}

QString PixelStreamSynchronizer::getStatistics() const
{
    return _fpsCounter.toString();
}

ImagePtr PixelStreamSynchronizer::getTileImage( const uint tileIndex,
                                                const uint64_t timestamp ) const
{
    if( !_updater )
        return ImagePtr();

    const QImage image = _updater->getTileImage( tileIndex, timestamp );
    if( image.isNull( ))
        return ImagePtr();

    return std::make_shared<QtImage>( image );
}

void PixelStreamSynchronizer::onSwapReady( TilePtr tile )
{
    if( _syncSet.find( tile->getIndex( )) != _syncSet.end( ))
    {
        _tilesReadyToSwap.insert( tile );
        _tilesReadySet.insert( tile->getIndex( ));
    }
    else
        tile->swapImage();
}

void PixelStreamSynchronizer::_onPictureUpdated( const uint64_t frameIndex )
{
    _frameIndex = frameIndex;
    _tilesDirty = true;
    _updateExistingTiles = true;
}

void PixelStreamSynchronizer::_updateTiles()
{
    const Indices visibleSet = _updater->computeVisibleSet( _visibleTilesArea );

    const Indices addedTiles = set_difference( visibleSet, _visibleSet );
    const Indices removedTiles = set_difference( _visibleSet, visibleSet );

    for( auto i : removedTiles )
        emit removeTile( i );

    for( auto i : addedTiles )
        emit addTile( std::make_shared<Tile>( i, _updater->getTileRect( i )));

    if( _updateExistingTiles )
    {
        const Indices currentTiles = set_difference( _visibleSet, removedTiles);
        for( auto i : currentTiles )
            emit updateTile( i, _updater->getTileRect( i ));
        _syncSet = visibleSet;
    }
    else
        _syncSet = set_difference( _syncSet, removedTiles );

    _visibleSet = visibleSet;
}
