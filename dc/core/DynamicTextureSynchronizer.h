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

#ifndef DYNAMICTEXTURESYNCHRONIZER_H
#define DYNAMICTEXTURESYNCHRONIZER_H

#include "ContentSynchronizer.h"

#include "DynamicTexture.h" // member

/**
 * A synchronizer which provides the list of Tiles for DynamicTextures.
 */
class DynamicTextureSynchronizer : public ContentSynchronizer
{
    Q_OBJECT
    Q_DISABLE_COPY( DynamicTextureSynchronizer )

public:
    /** Constructor */
    DynamicTextureSynchronizer( const QString& uri );

    /** @copydoc ContentSynchronizer::updateTiles */
    void update( const ContentWindow& window,
                 const QRectF& visibleArea ) override;

    /** @copydoc ContentSynchronizer::synchronize */
    void synchronize( WallToWallChannel& channel ) override;

    /** @copydoc ContentSynchronizer::needRedraw */
    bool needRedraw() const override;

    /** @copydoc ContentSynchronizer::allowsTextureCaching */
    bool allowsTextureCaching() const override;

    /** @copydoc ContentSynchronizer::getTilesArea */
    QSize getTilesArea() const override;

    /** @copydoc ContentSynchronizer::getStatistics */
    QString getStatistics() const override;

    /** @copydoc ContentSynchronizer::getTileImage */
    ImagePtr getTileImage( uint tileIndex, uint64_t timestamp ) const override;

    /** @copydoc ContentSynchronizer::onSwapReady */
    void onSwapReady( TilePtr tile ) override;

private:
    DynamicTexturePtr _reader;
    uint _lod;
    QRectF _visibleArea;

    Indices _visibleSet;
    typedef std::map<size_t, Tiles> LodTilesMap;
    LodTilesMap _lodTilesMap;

    void _updateTiles( const QRectF& visibleArea, uint lod );
    Tiles _gatherAllTiles( const uint lod ) const;
    Indices _computeVisibleSet( const QRectF& visibleArea,
                                const Tiles& tiles ) const;
};

#endif
