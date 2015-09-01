/*********************************************************************/
/* Copyright (c) 2011 - 2012, The University of Texas at Austin.     */
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

#include "Movie.h"

#include "log.h"

#include "ContentWindow.h"
#include "FFMPEGMovie.h"
#include "FFMPEGFrame.h"
#include "MovieContent.h"
#include "WallToWallChannel.h"

Movie::Movie( const QString& uri )
    : _ffmpegMovie( new FFMPEGMovie( uri ))
    , _paused( false )
    , _loop( true )
    , _isVisible( true )
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

Movie::~Movie() {}

void Movie::setVisible( const bool isVisible )
{
    _isVisible = isVisible;
}

void Movie::setPause( const bool pause )
{
    _paused = pause;
}

void Movie::setLoop( const bool loop )
{
    _loop = loop;
}

void Movie::render()
{
    if( !_texture.isValid( ))
        return;

    _quad.render();
}

void Movie::renderPreview()
{
    if( !_texture.isValid( ))
        return;

    _previewQuad.render();
}

void Movie::preRenderUpdate( ContentWindowPtr window, const QRect& wallArea )
{
    if( !_ffmpegMovie->isValid( ))
        return;

    if( !_texture.isValid( ))
    {
        _generateTexture();
        _quad.setTexture( _texture.getTextureId( ));
        _previewQuad.setTexture( _texture.getTextureId( ));
    }

    _quad.setTexCoords( window->getZoomRect( ));

    MovieContent& movie = static_cast<MovieContent&>( *window->getContent( ));
    setPause( movie.getControlState() & STATE_PAUSED );
    setLoop( movie.getControlState() & STATE_LOOP );

    setVisible( QRectF( wallArea ).intersects( _qmlItem->getSceneRect( )));
}

void Movie::preRenderSync( WallToWallChannel& wallToWallChannel )
{
    if( _paused || !_ffmpegMovie->isValid( ))
        return;

    _updateTimestamp( wallToWallChannel );
    _synchronizeTimestamp( wallToWallChannel );

    if( !_isVisible )
        return;

    if( _futurePicture.valid() && is_ready( _futurePicture ))
    {
        try
        {
            _texture.update( _futurePicture.get()->getData(), GL_RGBA );
        }
        catch( const std::exception& e )
        {
            put_flog( LOG_DEBUG, "Future was canceled: ", e.what( ));
        }
    }

    const double delay = fabs( _sharedTimestamp - _ffmpegMovie->getPosition( ));
    if( !_futurePicture.valid() && delay >= _ffmpegMovie->getFrameDuration( ))
        _futurePicture = _ffmpegMovie->getFrame( _sharedTimestamp );
}

bool Movie::_generateTexture()
{
    QImage image( _ffmpegMovie->getWidth(), _ffmpegMovie->getHeight(),
                  QImage::Format_RGB32 );
    image.fill( 0 );

    return _texture.init( image );
}

void Movie::_updateTimestamp( WallToWallChannel& wallToWallChannel )
{
    _timer.setCurrentTime( wallToWallChannel.getTime( ));
    _sharedTimestamp += ElapsedTimer::toSeconds( _timer.getElapsedTime( ));

    const double maxDelay = 2.0 * _ffmpegMovie->getFrameDuration();
    _sharedTimestamp = std::min( _sharedTimestamp, _sharedTimestamp + maxDelay );

    if( _loop && (_ffmpegMovie->isAtEOF( ) ||
                  _sharedTimestamp > _ffmpegMovie->getDuration( )))
        _sharedTimestamp = 0.0;
    else
        _ffmpegMovie->getDuration();
}

void Movie::_synchronizeTimestamp( WallToWallChannel& wallToWallChannel )
{
    // Elect a leader among processes which have decoded a frame
    const bool isCandidate = _ffmpegMovie->isValid() && _isVisible;
    const int leader = wallToWallChannel.electLeader( isCandidate );

    if( leader < 0 )
        return;

    if( leader == wallToWallChannel.getRank( ))
        wallToWallChannel.broadcast( ElapsedTimer::toTimeDuration(
                                         _sharedTimestamp ));
    else
        _sharedTimestamp = ElapsedTimer::toSeconds(
                         wallToWallChannel.receiveTimestampBroadcast( leader ));
}
