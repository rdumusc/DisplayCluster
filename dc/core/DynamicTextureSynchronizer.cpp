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

#include "ContentWindow.h"
#include "Tile.h"
#include "ZoomHelper.h"

#include <QTextStream>

DynamicTextureSynchronizer::DynamicTextureSynchronizer( const QString& uri )
    : _reader( uri )
    , _lod( 0 )
{
    _updateTiles( _reader.getMaxLod( ));
}

void DynamicTextureSynchronizer::sync( WallToWallChannel& channel )
{
    Q_UNUSED( channel );
}

void DynamicTextureSynchronizer::updateTiles( const ContentWindow& window )
{
    const QRectF contentRect = ZoomHelper( window ).getContentRect();
    const auto lod = _reader.getLod( contentRect.size().toSize( ));
    _updateTiles( lod );
}

QString DynamicTextureSynchronizer::getSourceParams() const
{
    return QString();
}

bool DynamicTextureSynchronizer::allowsTextureCaching() const
{
    return true;
}

QList<QObject*> DynamicTextureSynchronizer::getTiles() const
{
    return _tiles;
}

QSize DynamicTextureSynchronizer::getTilesArea() const
{
    return _tilesArea.size();
}

QString DynamicTextureSynchronizer::getStatistics() const
{
    QString stats;
    QTextStream stream( &stats );
    stream << "LOD:  " << _lod << "/" << _reader.getMaxLod();
    stream << "  res: " << _tilesArea.width() << "x" << _tilesArea.height();
    return stats;
}

void DynamicTextureSynchronizer::_updateTiles( const uint lod )
{
    if( lod == _lod && !_tiles.empty( ))
        return;

    _lod = lod;
    _tiles.clear();
    _tilesArea = QRect();

    const QSize tilesCount = _reader.getTilesCount( _lod );
    const int startIndex = _reader.getFirstTileIndex( _lod );

    for( int y = 0; y < tilesCount.height(); ++y )
    {
        for( int x = 0; x < tilesCount.width(); ++x )
        {
            const int i = startIndex + y * tilesCount.width() + x;
            const QRect coord = _reader.getTileCoord( _lod, x, y );
            _tiles.push_back( new Tile( i, coord, this ));
            _tilesArea = _tilesArea.united( coord );
        }
    }
    emit tilesChanged();
    emit statisticsChanged();
}
