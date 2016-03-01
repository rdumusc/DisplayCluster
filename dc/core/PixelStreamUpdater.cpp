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

#include "PixelStreamUpdater.h"

#include "WallToWallChannel.h"

#include "log.h"

#include <deflect/Frame.h>
#include <deflect/SegmentDecoder.h>
#include <boost/bind.hpp>
#include <QImage>
#include <QThreadStorage>

#include "FpsCounter.h"
#include <QDebug>

PixelStreamUpdater::PixelStreamUpdater()
    : _frameIndex( 0 )
    , _readyToSwap( true )
{
    _swapSyncFrame.setCallback( boost::bind(
                                    &PixelStreamUpdater::_onFrameSwapped,
                                    this, _1 ));
}

bool PixelStreamUpdater::synchronizeFramesSwap( WallToWallChannel& channel )
{
    static FpsCounter counter;
    counter.tick();
    qDebug() << counter.toString();

    if( !_readyToSwap )
    {
        qDebug() << "not ready to swap";
        return false;
    }

    const SyncFunction& versionCheckFunc =
        boost::bind( &WallToWallChannel::checkVersion, &channel, _1 );

    return _swapSyncFrame.sync( versionCheckFunc );
}

QRect toRect( const deflect::SegmentParameters& params )
{
    return QRect( params.x, params.y, params.width, params.height );
}

QRect PixelStreamUpdater::getTileRect( const uint tileIndex ) const
{
    return toRect( _currentFrame->segments.at( tileIndex ).parameters );
}

QSize PixelStreamUpdater::getTilesArea( const uint lod ) const
{
    Q_UNUSED( lod );
    if( !_currentFrame )
        return QSize();
    return _currentFrame->computeDimensions();
}

QImage PixelStreamUpdater::getTileImage( const uint tileIndex,
                                         const uint64_t timestamp ) const
{
    if( !_currentFrame )
        throw std::runtime_error( "No frames yet" );

    const QReadLocker lock( &_mutex );
    if( timestamp != _frameIndex )
    {
        put_flog( LOG_DEBUG, "incorrect timestamp: %d, current frameIndex: %d",
                  timestamp, _frameIndex );
        return QImage();
    }

    auto& segment = _currentFrame->segments.at( tileIndex );
    if( segment.parameters.compressed )
    {
        // turbojpeg handles need to be per thread, and this function may be
        // called from multiple threads
        static QThreadStorage< deflect::SegmentDecoder > decoder;
        decoder.localData().decode( segment );
    }

    return QImage( (const uchar*)segment.imageData.constData(),
                   segment.parameters.width, segment.parameters.height,
                   QImage::Format_RGBX8888 );
}

Indices PixelStreamUpdater::computeVisibleSet( const QRectF& visibleTilesArea,
                                               const uint lod ) const
{
    Q_UNUSED( lod );

    Indices visibleSet;

    if( !_currentFrame || visibleTilesArea.isEmpty( ))
        return visibleSet;

    for( size_t i = 0; i < _currentFrame->segments.size(); ++i )
    {
        if( visibleTilesArea.intersects( toRect( _currentFrame->segments[i].parameters )))
            visibleSet.insert( i );
    }
    return visibleSet;
}

void PixelStreamUpdater::getNextFrame()
{
    //emit requestFrame( _currentFrame->uri );
    _readyToSwap = true;
}

void PixelStreamUpdater::updatePixelStream( deflect::FramePtr frame )
{
    _swapSyncFrame.update( frame );
}

void PixelStreamUpdater::_onFrameSwapped( deflect::FramePtr frame )
{
    _readyToSwap = false;

    std::sort( frame->segments.begin(), frame->segments.end(),
               []( const deflect::Segment& s1, const deflect::Segment& s2 )
        {
            return ( s1.parameters.y == s2.parameters.y ?
                         s1.parameters.x < s2.parameters.x :
                         s1.parameters.y < s2.parameters.y );
        } );

    {
        const QWriteLocker lock( &_mutex );
        ++_frameIndex;
        _currentFrame = frame;
    }

    emit pictureUpdated( _frameIndex );
    emit requestFrame( _currentFrame->uri );
}
