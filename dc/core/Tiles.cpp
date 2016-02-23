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

#include "Tiles.h"

namespace
{
const int ROLE_TILE = Qt::UserRole;
}

Tiles::Tiles( QObject* parent_ )
    : QAbstractListModel( parent_ )
{}

QHash<int, QByteArray> Tiles::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[ ROLE_TILE ] = "tile";
    return roles;
}

QVariant Tiles::data( const QModelIndex& index_, const int role ) const
{
    if( index_.row() < 0 || index_.row() >= rowCount() || !index_.isValid( ))
        return QVariant();

    if( role == ROLE_TILE )
    {
        QVariant variant;
        variant.setValue( static_cast<QObject*>( _tiles[ index_.row() ].get( )));
        return variant;
    }
    return QVariant();
}

int Tiles::rowCount( const QModelIndex& parent_ ) const
{
    Q_UNUSED( parent_ );
    return _tiles.size();
}

void Tiles::add( TilePtr tile )
{
    if( contains( tile->getIndex( )))
        return;

    const int tileIndex = rowCount();

    beginInsertRows( QModelIndex(), tileIndex, tileIndex );

    _tiles.push_back( std::move( tile ));

    endInsertRows();
}

Tile* Tiles::get( const uint tileIndex )
{
    auto it = _findTile( tileIndex );
    return it  == _tiles.end() ? nullptr : it->get();
}

void Tiles::reset( TileList&& tiles )
{
    beginResetModel();

    _tiles = std::move( tiles );

    endResetModel();
}

bool Tiles::update( const uint tileIndex, const QRect& coordinates )
{
    auto it = _findTile( tileIndex );

    if( it  == _tiles.end( ))
        return false;

    (*it)->update( coordinates );

    // No need to emit data changed, Tiles does the notification itself
    //const int rowIndex = it - _tiles.begin();
    //emit dataChanged( index( rowIndex, 0 ), index( rowIndex, 0 ));

    return true;
}

void Tiles::remove( const uint tileIndex )
{
    auto it = _findTile( tileIndex );

    if( it  == _tiles.end( ))
        return;

    const int rowIndex = it - _tiles.begin();
    beginRemoveRows( QModelIndex(), rowIndex, rowIndex );

    _tiles.erase( it );

    endRemoveRows();
}

bool Tiles::contains( const uint tileIndex ) const
{
    return _findTile( tileIndex ) != _tiles.end();
}

TileList::const_iterator Tiles::_findTile( const uint tileIndex ) const
{
   return std::find_if( _tiles.begin(), _tiles.end(), [&tileIndex]( const TilePtr& tile )
             { return tile->getIndex() == tileIndex; });
}

TileList::iterator Tiles::_findTile( const uint tileIndex )
{
   return std::find_if( _tiles.begin(), _tiles.end(), [&tileIndex]( const TilePtr& tile )
             { return tile->getIndex() == tileIndex; });
}
