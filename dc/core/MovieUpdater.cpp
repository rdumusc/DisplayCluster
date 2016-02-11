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
#include "FFMPEGPicture.h"
#include "FFMPEGMovie.h"
#include "MovieContent.h"
#include "WallToWallChannel.h"
#include "log.h"

MovieUpdater::MovieUpdater( const QString& uri )
    : _ffmpegMovie( new FFMPEGMovie( uri ))
    , _paused( false )
    , _loop( true )
    , _visible( true )
    , _sharedTimestamp( 0.0 )
{
    // Observed bug [DISCL-295]: opening a movie might fail on WallProcesses
    // despite correctly reading metadata on the MasterProcess.
    // bool FFMPEGMovie::openVideoStreamDecoder(): could not open codec
    // error: -11 Resource temporarily unavailable
    if( !_ffmpegMovie->isValid( ))
        put_flog( LOG_WARN, "Movie is invalid: %s",
                  uri.toLocal8Bit().constData( ));
    else
        _ffmpegMovie->startDecoding();
}

MovieUpdater::~MovieUpdater() {}

void MovieUpdater::setVisible( const bool visible )
{
    _visible = visible;
}

void MovieUpdater::update( const MovieContent& movie )
{
    _paused = movie.getControlState() & STATE_PAUSED;
    _loop = movie.getControlState() & STATE_LOOP;
}

void MovieUpdater::sync( WallToWallChannel& channel )
{
    if( !_ffmpegMovie->isValid( ))
        return;

    _timer.setCurrentTime( channel.getTime( ));
    if( !_paused )
    {
        _updateTimestamp( channel );
        _synchronizeTimestamp( channel );
    }

    if( !_visible )
        return;

    if( _futurePicture.valid() && is_ready( _futurePicture ))
    {
        try
        {
            _currentPicture = _futurePicture.get();
            emit pictureUpdated( _currentPicture->getTimestamp( ));
        }
        catch( const std::exception& e )
        {
            put_flog( LOG_DEBUG, "Future was canceled: ", e.what( ));
        }
    }

    if( _loop && (_ffmpegMovie->isAtEOF( ) ||
                  _sharedTimestamp > _ffmpegMovie->getDuration( )))
        _sharedTimestamp = 0.0;

    const bool needsFrame = _getDelay() >= _ffmpegMovie->getFrameDuration();
    if( !_futurePicture.valid() && needsFrame )
        _futurePicture = _ffmpegMovie->getFrame( _sharedTimestamp );
}

PicturePtr MovieUpdater::getPicture()
{
    return _currentPicture;
}

double MovieUpdater::_getDelay() const
{
    return fabs( _sharedTimestamp - _ffmpegMovie->getPosition( ));
}

void MovieUpdater::_updateTimestamp( WallToWallChannel& channel )
{
    // Don't increment the timestamp until all the processes have caught up
    const bool isInSync = _getDelay() <= _ffmpegMovie->getFrameDuration();
    if( !channel.allReady( !_visible || isInSync ))
        return;

    _sharedTimestamp += ElapsedTimer::toSeconds( _timer.getElapsedTime( ));
}

void MovieUpdater::_synchronizeTimestamp( WallToWallChannel& channel )
{
    // Elect a leader among processes which have decoded a frame
    const bool isCandidate = _ffmpegMovie->isValid() && _visible;
    const int leader = channel.electLeader( isCandidate );

    if( leader < 0 )
        return;

    if( leader == channel.getRank( ))
        channel.broadcast( ElapsedTimer::toTimeDuration( _sharedTimestamp ));
    else
        _sharedTimestamp = ElapsedTimer::toSeconds(
                                   channel.receiveTimestampBroadcast( leader ));
}
