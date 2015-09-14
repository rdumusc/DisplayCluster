/*********************************************************************/
/* Copyright (c) 2014, EPFL/Blue Brain Project                       */
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

#include "FFMPEGMovie.h"

#include "FFMPEGFrame.h"
#include "FFMPEGVideoStream.h"
#include "log.h"

#define MIN_SEEK_DELTA_SEC  0.5
#define VIDEO_QUEUE_SIZE    4

#pragma clang diagnostic ignored "-Wdeprecated"
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

FFMPEGMovie::FFMPEGMovie( const QString& uri )
    : _avFormatContext( 0 )
    , _ptsPosition( 0.0 )
    , _streamPosition( 0.0 )
    , _isValid( false )
    , _isAtEOF( false )
    , _queue( VIDEO_QUEUE_SIZE )
    , _stopDecoding( true )
    , _stopConsuming( false )
    , _seek( false )
    , _seekPosition( 0.0 )
    , _targetTimestamp( 0.0 )
{
    FFMPEGMovie::initGlobalState();
    _isValid = _open( uri );
}

FFMPEGMovie::~FFMPEGMovie()
{
    stopDecoding();
    _videoStream.reset();
    _releaseAvFormatContext();
}

bool FFMPEGMovie::_open( const QString& uri )
{
    if( !_createAvFormatContext( uri ))
        return false;

    try
    {
        _videoStream.reset( new FFMPEGVideoStream( *_avFormatContext ));
    }
    catch( const std::runtime_error& e )
    {
        put_flog( LOG_FATAL, "Error opening file %s : '%s'",
                  uri.toLocal8Bit().constData(), e.what( ));
        return false;
    }

    return true;
}

bool FFMPEGMovie::_createAvFormatContext( const QString& uri )
{
    // Read movie header information into _avFormatContext and allocate it
    if( avformat_open_input( &_avFormatContext, uri.toLatin1(), 0, 0 ) != 0 )
    {
        put_flog( LOG_ERROR, "error reading movie headers: '%s'",
                  uri.toLocal8Bit().constData( ));
        return false;
    }

    // Read stream information into avFormatContext_->streams
    if( avformat_find_stream_info( _avFormatContext, NULL ) < 0 )
    {
        put_flog( LOG_ERROR, "error reading stream information: '%s'",
                  uri.toLocal8Bit().constData( ));
        return false;
    }

#if LOG_THRESHOLD <= LOG_VERBOSE
    // print detail information about the input or output format
    av_dump_format( avFormatContext_, 0, uri.toLatin1(), 0 );
#endif
    return true;
}

void FFMPEGMovie::_releaseAvFormatContext()
{
    if( _avFormatContext )
        avformat_close_input( &_avFormatContext );
}

void FFMPEGMovie::initGlobalState()
{
    static bool initialized = false;

    if( !initialized )
    {
        av_register_all();
        initialized = true;
    }
}

bool FFMPEGMovie::isValid() const
{
    return _isValid;
}

unsigned int FFMPEGMovie::getWidth() const
{
    return _videoStream->getWidth();
}

unsigned int FFMPEGMovie::getHeight() const
{
    return _videoStream->getHeight();
}

double FFMPEGMovie::getPosition() const
{
    return _ptsPosition;
}

bool FFMPEGMovie::isAtEOF() const
{
    return _isAtEOF && _queue.empty();
}

double FFMPEGMovie::getDuration() const
{
    return _videoStream->getDuration();
}

double FFMPEGMovie::getFrameDuration() const
{
    return _videoStream->getFrameDuration();
}

void FFMPEGMovie::startDecoding()
{
    stopDecoding();
    _stopDecoding = false;
    _stopConsuming = false;
    _consumeThread = std::thread( &FFMPEGMovie::_consume, this );
    _decodeThread = std::thread( &FFMPEGMovie::_decode, this );
}

void FFMPEGMovie::stopDecoding()
{
    _stopDecoding = true;
    _stopConsuming = true;
    _queue.clear();
    if( _isAtEOF )
        _seekRequested.notify_one();
    if( _decodeThread.joinable( ))
        _decodeThread.join();
    if( _consumeThread.joinable( ))
    {
        _queue.enqueue( std::make_shared<FFMPEGPicture>( 1, 1, PIX_FMT_RGBA ));
        _targetChanged.notify_one();
        _consumeThread.join();
    }
    _queue.clear();
}

bool FFMPEGMovie::isDecoding() const
{
    return !_stopDecoding;
}

std::future<PicturePtr> FFMPEGMovie::getFrame( const double posInSeconds )
{
    if( !isDecoding( ))
        return std::async( std::launch::async, &FFMPEGMovie::_grabSingleFrame,
                           this, posInSeconds );

    std::lock_guard<std::mutex> lock( _targetMutex );
    _promise = std::promise<PicturePtr>();
    _targetTimestamp = posInSeconds;
    _targetChanged.notify_one();
    return _promise.get_future();
}

void FFMPEGMovie::_decode()
{
    while( !_stopDecoding )
        _decodeOneFrame();
}

void FFMPEGMovie::_decodeOneFrame()
{
    {
        std::unique_lock<std::mutex> lock( _seekMutex );
        if( _seek )
        {
            _seekFileTo( _seekPosition );
            _seekPosition = 0.0;
            _seek = false;
            _seekFinished.notify_one();
            return;
        }
        if( _isAtEOF && isDecoding( ))
        {
            _seekRequested.wait( lock );
            return;
        }
    }
    _readVideoFrame();
}

void FFMPEGMovie::_consume()
{
    while( !_stopConsuming )
    {
        std::unique_lock<std::mutex> lock( _targetMutex );
        _targetChanged.wait( lock );
        if( _stopConsuming )
            return;

        double ptsDelta = _targetTimestamp - getPosition();

        // If we seek, we have to consume at least one frame.
        if( _seekTo( _targetTimestamp ))
            ptsDelta = getFrameDuration();

        PicturePtr frame;
        while( !_stopConsuming && ptsDelta >= getFrameDuration() && !isAtEOF( ))
        {
            frame = _queue.dequeue();
            _ptsPosition = _videoStream->getPositionInSec( frame->getTimestamp( ));
            ptsDelta = _targetTimestamp - getPosition();
        }

        if( !frame )
        {
            auto exception = std::runtime_error( "Frame unavailable error" );
            _promise.set_exception( std::make_exception_ptr( exception ));
        }
        else
            _promise.set_value( frame );
    }
}

bool FFMPEGMovie::_seekTo( double posInSeconds )
{
    if( _seek )
        return false;

    posInSeconds = std::max( 0.0, std::min( posInSeconds, getDuration( )));

    const double ptsDelta = posInSeconds - getPosition();
    const double streamDelta = fabs( posInSeconds - _streamPosition );

    // Don't seek forward if the delta is small. Always seek backwards.
    if( ptsDelta >= 0.0 && streamDelta < MIN_SEEK_DELTA_SEC )
        return false;

    _queue.clear();
    std::unique_lock<std::mutex> lock( _seekMutex );
    _seekPosition = posInSeconds;
    _seek = true;
    _seekRequested.notify_one();
    _seekFinished.wait( lock );
    return true;
}

bool FFMPEGMovie::_readVideoFrame()
{
    int avReadStatus = 0;

    AVPacket packet;
    av_init_packet( &packet );

    // keep reading frames until we decode a valid video frame
    while( (avReadStatus = av_read_frame( _avFormatContext, &packet )) >= 0 )
    {
        auto picture = _videoStream->decode( packet );
        if( picture )
        {
            _queue.enqueue( picture );
            _streamPosition = _videoStream->getPositionInSec( picture->getTimestamp( ));

            // free the packet that was allocated by av_read_frame
            av_free_packet( &packet );
            break;
        }
        // free the packet that was allocated by av_read_frame
        av_free_packet( &packet );
    }

    // False if file read error or EOF reached
    _isAtEOF = (avReadStatus < 0);
    return !_isAtEOF;
}

bool FFMPEGMovie::_seekFileTo( const double timePosInSeconds )
{
    const double frameDuration = _videoStream->getFrameDuration();
    const int64_t frameIndex = timePosInSeconds / frameDuration;

    if( !_videoStream->seekToNearestFullframe( frameIndex ))
        return false;

    // Read frames until we reach the correct timestamp
    int avReadStatus = 0;

    AVPacket packet;
    av_init_packet( &packet );

    const double target = std::max( 0.0, timePosInSeconds - frameDuration );
    const int64_t targetTimestamp = _videoStream->getTimestamp( target );

    while( (avReadStatus = av_read_frame( _avFormatContext, &packet )) >= 0 )
    {
        const int64_t timestamp = _videoStream->decodeTimestamp( packet );
        if( timestamp >= targetTimestamp )
        {
            auto picture = _videoStream->decode( packet );
            // This validity check is to prevent against rare decoding errors
            // and is not inherently part of the seeking process.
            if( picture )
            {
                _streamPosition = _videoStream->getPositionInSec( timestamp );
                _queue.clear();
                _queue.enqueue( picture );

                // free the packet that was allocated by av_read_frame
                av_free_packet( &packet );
                break;
            }
        }
        // free the packet that was allocated by av_read_frame
        av_free_packet( &packet );
    }

    _isAtEOF = (avReadStatus < 0);
    return !_isAtEOF;
}

PicturePtr FFMPEGMovie::_grabSingleFrame( const double posInSeconds )
{
    _seekFileTo( posInSeconds );
    if( _queue.empty( ))
        throw std::runtime_error( "Frame unavailable error" );
    return _queue.dequeue();
}
