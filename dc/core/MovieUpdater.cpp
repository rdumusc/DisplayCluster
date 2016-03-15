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
    , _paused( false )
    , _loop( true )
    , _visible( true )
    , _sharedTimestamp( 0.0 )
    , _lastTimestamp( 0.0 )
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
    ImagePtr image = _ffmpegMovie->getFrame( _sharedTimestamp );
    if( _loop && !image )
        image = _ffmpegMovie->getFrame( 0.0 );
    _lastTimestamp = _ffmpegMovie->getPosition();
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

bool MovieUpdater::synchronizeTimestamp( WallToWallChannel& channel )
{
    _timer.setCurrentTime( channel.getTime( ));

    if( !_ffmpegMovie->isValid() || _paused )
        return false;

    _sharedTimestamp = _lastTimestamp;
    _sharedTimestamp += ElapsedTimer::toSeconds( _timer.getElapsedTime( ));

    // Elect a leader among processes which have decoded a frame
    const int leader = channel.electLeader( _visible );
    if( leader < 0 )
        return true;

    if( leader == channel.getRank( ))
        channel.broadcast( ElapsedTimer::toTimeDuration( _sharedTimestamp ));
    else
        _sharedTimestamp = ElapsedTimer::toSeconds(
                                   channel.receiveTimestampBroadcast( leader ));
    return true;
}
