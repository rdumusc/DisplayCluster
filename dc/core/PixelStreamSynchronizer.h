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

#ifndef PIXELSTREAMSYNCHRONIZER_H
#define PIXELSTREAMSYNCHRONIZER_H

#include "ContentSynchronizer.h"
#include "FpsCounter.h"

#include <QObject>

/**
 * Synchronizes a PixelStream between different QML windows.
 *
 * The PixelStreamSynchronizer serves as an interface between the
 * PixelStreamProvider and the QML rendering, to inform it when new frames are
 * ready and swap them synchronously.
 */
class PixelStreamSynchronizer : public ContentSynchronizer
{
    Q_OBJECT
    Q_DISABLE_COPY( PixelStreamSynchronizer )

public:
    /**
     * Construct a synchronizer for a stream, opening it in the provider.
     * @param uri The uri of the movie to open.
     * @param provider The PixelStreamProvider where the stream will be opened.
     */
    PixelStreamSynchronizer( const QString& uri,
                             PixelStreamProvider& provider );

    /** Destruct the synchronizer and close the stream in the provider. */
    ~PixelStreamSynchronizer();

    /** @copydoc ContentSynchronizer::sync */
    void sync( WallToWallChannel& channel ) override;

    /** @copydoc ContentSynchronizer::updateTiles */
    void updateTiles( const ContentWindow& window ) override;

    /** @copydoc ContentSynchronizer::getSourceParams */
    QString getSourceParams() const override;

    /** @copydoc ContentSynchronizer::allowsTextureCaching */
    bool allowsTextureCaching() const override;

    /** @copydoc ContentSynchronizer::getTiles */
    QList<QObject*> getTiles() const override;

    /** @copydoc ContentSynchronizer::getTilesArea */
    QSize getTilesArea() const override;

    /** @copydoc ContentSynchronizer::getStatistics */
    QString getStatistics() const override;

private:
    QString _uri;
    PixelStreamProvider& _provider;
    PixelStreamUpdaterSharedPtr _updater;
    uint _frameIndex;
    FpsCounter _fpsCounter;

    void onPictureUpdated( uint frameIndex );
};

#endif
