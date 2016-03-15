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

#include "MovieSynchronizer.h"

#include "ContentWindow.h"
#include "MovieContent.h"
#include "MovieUpdater.h"
#include "Tile.h"

MovieSynchronizer::MovieSynchronizer( const QString& uri )
    : TiledSynchronizer( TileSwapPolicy::SwapTilesAlwaysSynchronously )
    , _updater( new MovieUpdater( uri ))
    , _tilesDirty( true )
    , _updateExistingTiles( true )
{
}

void MovieSynchronizer::update( const ContentWindow& window,
                                const QRectF& visibleArea )
{
    _updater->update( static_cast< const MovieContent& >( *window.getContent()),
                      !visibleArea.isEmpty( ));

    if( _visibleTilesArea == visibleArea )
        return;

    _visibleTilesArea = visibleArea;
    _tilesDirty = true;
}

void MovieSynchronizer::synchronize( WallToWallChannel& channel )
{
    if( TiledSynchronizer::swapTiles( channel ))
    {
        _fpsCounter.tick();
        emit statisticsChanged();

        _tilesDirty = true;
        _updateExistingTiles = true;
    }

    _updateExistingTiles = _updater->synchronizeTimestamp( channel ) &&
                           _updateExistingTiles;

    if( _tilesDirty )
    {
        TiledSynchronizer::updateTiles( *_updater, _updateExistingTiles );
        _tilesDirty = false;
        _updateExistingTiles = false;
    }
}

QSize MovieSynchronizer::getTilesArea() const
{
    return _updater->getTilesArea( 0 );
}

QString MovieSynchronizer::getStatistics() const
{
    return _fpsCounter.toString();
}

ImagePtr MovieSynchronizer::getTileImage( const uint tileIndex ) const
{
    return _updater->getTileImage( tileIndex );
}
