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

#ifndef TILES_H
#define TILES_H

#include "types.h"
#include "Tile.h"

#include <QtCore/QAbstractListModel>

/**
 * Exposes Tiles through a model for using in dynamic QML views.
 */
class Tiles : public QAbstractListModel
{
    Q_OBJECT
    Q_DISABLE_COPY( Tiles )

public:
    Tiles( QObject* parent = 0 );

    QVariant data( const QModelIndex &index, int role ) const override;
    int rowCount( const QModelIndex& parent = QModelIndex( )) const override;

    /** Add a tile. */
    void add( TilePtr tile );

    /** Get a tile. */
    Tile* get( int tileIndex );

    /** Update a tile coordinates. */
    bool update( int tileIndex, const QRect& coordinates );

    /** Remove a Tile. */
    void remove( int tileIndex );

    /** Reset the model with a new list of tiles. */
    void reset( TileList&& tiles );

    /** Check for the existence of a tile. */
    bool contains( int tileIndex ) const;

    /** Get the internal list of tiles. */
    const TileList& getTileList() const;

private:
    QHash<int, QByteArray> roleNames() const final;

    TileList _tiles;

    TileList::const_iterator _findTile( int tileIndex ) const;
    TileList::iterator _findTile( int tileIndex );
};

#endif
