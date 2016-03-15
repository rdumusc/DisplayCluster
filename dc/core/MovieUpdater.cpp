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

#include "MovieUpdater.h"

#include "FFMPEGFrame.h"
#include "FFMPEGMovie.h"
#include "FFMPEGPicture.h"
#include "MovieContent.h"
#include "WallToWallChannel.h"

#include "log.h"

MovieUpdater::MovieUpdater( const QString& uri )
    : _ffmpegMovie( new FFMPEGMovie( uri ))
    , _swapDone( false )
    , _paused( false )
    , _loop( true )
    , _visible( true )
    , _updateTile( false )
    , _elapsedTime( 0.0 )
    , _sharedTimestamp( 0.0 )
    , _currentPosition( 0.0 )
{
    // Observed bug [DISCL-295]: opening a movie might fail on WallProcesses
    // despite correctly reading metadata on the MasterProcess.
    // bool FFMPEGMovie::openVideoStreamDecoder(): could not open codec
    // error: -11 Resource temporarily unavailable
    if( !_ffmpegMovie->isValid( ))
        put_flog( LOG_WARN, "Movie is invalid: %s",
                  uri.toLocal8Bit().constData( ));
}

MovieUpdater::~MovieUpdater() {}

void MovieUpdater::update( const MovieContent& movie, const bool visible )
{
    _paused = movie.getControlState() & STATE_PAUSED;
    _loop = movie.getControlState() & STATE_LOOP;
    _visible = visible;
}

QRect MovieUpdater::getTileRect( const uint tileIndex ) const
{
    Q_UNUSED( tileIndex );
    return QRect( 0, 0, _ffmpegMovie->getWidth(), _ffmpegMovie->getHeight( ));
}

QSize MovieUpdater::getTilesArea( const uint lod ) const
{
    Q_UNUSED( lod );
    return getTileRect( 0 ).size();
}

ImagePtr MovieUpdater::getTileImage( const uint tileIndex ) const
{
    Q_UNUSED( tileIndex );
    double timestamp;
    {
        QMutexLocker lock( &_mutex );
        timestamp = _sharedTimestamp;
    }

    ImagePtr image = _ffmpegMovie->getFrame( timestamp );
    if( _loop && !image )
        image = _ffmpegMovie->getFrame( 0.0 );

    {
        QMutexLocker lock( &_mutex );
        _currentPosition = _ffmpegMovie->getPosition();
        // stay inSync for start != 0.0 and loop conditions
        _sharedTimestamp = _currentPosition;
    }

    return image;
}

Indices MovieUpdater::computeVisibleSet( const QRectF& visibleTilesArea,
                                         const uint lod ) const
{
    Q_UNUSED( lod );

    Indices visibleSet;

    if( visibleTilesArea.isEmpty( ))
        return visibleSet;

    visibleSet.insert( 0 );
    return visibleSet;
}

uint MovieUpdater::getMaxLod() const
{
    return 0;
}

void MovieUpdater::requestNextFrame()
{
    _fpsCounter.tick();

    _updateTile = false;
    _swapDone = true;
}

bool MovieUpdater::updateTile() const
{
    return _updateTile;
}

QString MovieUpdater::getStatistics() const
{
    const QString fps =
            QString::number( 1.0 / _ffmpegMovie->getFrameDuration(), 'g', 3 );
    const QString progress =
            QString::number( _sharedTimestamp * 100.0 /
                             _ffmpegMovie->getDuration(), 'g', 3 );
    return QString( "%1 / %2 fps %3 %").arg(_fpsCounter.toString())
                                       .arg( fps )
                                       .arg( progress );
}

bool MovieUpdater::synchronizeTimestamp( WallToWallChannel& channel )
{
    _timer.setCurrentTime( channel.getTime( ));

    if( _paused )
    {
        // don't advance time if paused
        _timer.setCurrentTime( channel.getTime( ));
        return false;
    }

    const double frameDuration = _ffmpegMovie->getFrameDuration();

    // protect _sharedTimestamp & _currentPosition from getTileImage()
    QMutexLocker lock( &_mutex );

    // If a visible updater is out-of-sync, only update this tile which causes
    // a seek in the movie to _sharedTimestamp. The time stands still in this
    // case to avoid seeking of all processes if this seek takes longer than
    // frameDuration.
    const bool inSync =
            std::abs( _sharedTimestamp - _currentPosition ) <= frameDuration;
    if( !channel.allReady( !_visible || inSync ) && _swapDone )
    {
        _swapDone = false;
        _updateTile = !inSync;
        _timer.setCurrentTime( channel.getTime( ));
        return true;
    }

    // If everybody is in sync, to a proper increment and throttle to movie
    // frame duration and swap/decode speed accordingly.
    const double elapsedTime = ElapsedTimer::toSeconds( _timer.getElapsedTime( ));
    _elapsedTime += elapsedTime;
    if( !_swapDone || _elapsedTime < frameDuration )
        return false;

    // advance to the next frame, keep correct elapsedTime as vsync frequency
    // of this function might not match movie frequency.
    _sharedTimestamp = _currentPosition + frameDuration;
    _elapsedTime -= _ffmpegMovie->getFrameDuration();

    // Always exchange timestamp for processes where _currentPosition is not
    // advancing to allow seek if visible again.
    const int leader = channel.electLeader( _visible && inSync );
    if( leader >= 0 )
    {
        if( leader == channel.getRank( ))
            channel.broadcast( ElapsedTimer::toTimeDuration( _sharedTimestamp ));
        else
            _sharedTimestamp = ElapsedTimer::toSeconds(
                                       channel.receiveTimestampBroadcast( leader ));
    }

    _swapDone = false;
    _updateTile = _visible;
    return true;
}
