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

#ifndef MOVIEUPDATER_H
#define MOVIEUPDATER_H

#include "DataSource.h"
#include "ElapsedTimer.h"
#include "FpsCounter.h"
#include "types.h"

#include <QMutex>

/**
 * Updates Movies synchronously across different processes.
 *
 * A single movie is designed to provide images to multiple windows on each
 * process.
 */
class MovieUpdater : public DataSource
{
public:
    explicit MovieUpdater( const QString& uri );
    ~MovieUpdater();

    /**
     * @copydoc DataSource::getTileImage
     * @threadsafe
     */
    ImagePtr getTileImage( uint tileIndex ) const final;

    /** @copydoc DataSource::getTileRect */
    QRect getTileRect( uint tileIndex ) const final;

    /** @copydoc DataSource::getTilesArea */
    QSize getTilesArea( uint lod ) const final;

    /** @copydoc DataSource::computeVisibleSet */
    Indices
    computeVisibleSet( const QRectF& visibleTilesArea, uint lod ) const final;

    /** @copydoc DataSource::getMaxLod */
    uint getMaxLod() const final;

    /** Update this datasource according to visibility and movie content. */
    void update( const MovieContent& movie, bool visible );

    /** Increment and synchronize movie timestamp across all wall processes. */
    bool synchronizeTimestamp( WallToWallChannel& channel );

    /**
     * Allow the updater to prepare the next frame, i.e. a previous
     * decode & swap of a movie frame is finished.
     */
    void requestNextFrame();

    /**
     * @return true if after synchronizeTimestamp() the tile needs update aka
     * decode the next movie frame.
     */
    bool updateTile() const;

    /** @return current / max fps, movie position in percentage */
    QString getStatistics() const;

private:
    MoviePtr _ffmpegMovie;
    FpsCounter _fpsCounter;

    bool _swapDone;
    bool _paused;
    bool _loop;
    bool _visible;
    bool _updateTile;

    ElapsedTimer _timer;
    double _elapsedTime;

    mutable QMutex _mutex;
    mutable double _sharedTimestamp;
    mutable double _currentPosition;
};

#endif
