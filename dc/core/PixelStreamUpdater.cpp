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

#include "Tile.h"
#include "WallToWallChannel.h"

#include <deflect/Frame.h>
#include <boost/bind.hpp>
#include <QImage>
#include <QThreadStorage>

PixelStreamUpdater::PixelStreamUpdater()
    : _frameIndex( 0 )
    , _requestedFrameIndex( 0 )
{
    _swapSyncFrame.setCallback( boost::bind(
                                    &PixelStreamUpdater::_onFrameSwapped,
                                    this, _1 ));
}

void PixelStreamUpdater::synchronizeFramesSwap( WallToWallChannel& channel )
{
    const SyncFunction& versionCheckFunc =
        boost::bind( &WallToWallChannel::checkVersion, &channel, _1 );

    _swapSyncFrame.sync( versionCheckFunc );
}

const Tiles& PixelStreamUpdater::getTiles() const
{
    return _tiles;
}

QImage PixelStreamUpdater::getTileImage( const uint frameIndex,
                                         const uint tileIndex )
{
    // to determine which frames can be deleted from the _frames map
    _requestedFrameIndex = std::max( _requestedFrameIndex, frameIndex );

    // find the segments for the requested frame
    deflect::FramePtr frame;
    for( const auto& i : _frames )
    {
        if( i.first == frameIndex )
        {
            frame = i.second;
            break;
        }
    }

    if( !frame )
        throw std::runtime_error( "No frames yet" );

    const auto& segment = frame->segments.at( tileIndex );

    return QImage( (const uchar*)segment.imageData.constData(),
                   segment.parameters.width, segment.parameters.height,
                   QImage::Format_RGBX8888 );
}

void PixelStreamUpdater::updatePixelStream( deflect::FramePtr frame )
{
    _swapSyncFrame.update( frame );
}

void PixelStreamUpdater::updateTilesVisibility( const QRectF& visibleArea )
{
    Q_UNUSED( visibleArea );
}

void PixelStreamUpdater::_onFrameSwapped( deflect::FramePtr frame )
{
    std::sort( frame->segments.begin(), frame->segments.end(),
               []( const deflect::Segment& s1, const deflect::Segment& s2 )
        {
            return ( s1.parameters.y < s2.parameters.y ||
                     s1.parameters.x < s2.parameters.x );
        } );

    // decode everthing in a batch rather than doing it in the requestImage() for
    // better performance. TODO: only decode visible segments
    _decodeSegments( frame->segments );
    _refreshTiles( frame->segments );

    // cleanup old frames
    while( !_frames.empty( ))
    {
        if( _frames.front().first >= _requestedFrameIndex )
            break;
        _frames.pop_front();
    }

    _frames.push_back( std::make_pair( _frameIndex, frame ));

    emit pictureUpdated( _frameIndex++ );
    emit requestFrame( frame->uri );
}

void PixelStreamUpdater::_decodeSegments( deflect::Segments& segments )
{
#pragma omp parallel for
    for( size_t i = 0; i < segments.size(); ++i )
    {
        if( segments[i].parameters.compressed )
        {
            // turbojpeg handles need to be per thread
            static QThreadStorage< deflect::SegmentDecoder > decoder;
            decoder.localData().decode( segments[i] );
        }
    }
}

QRect toRect( const deflect::SegmentParameters& params )
{
    return QRect( params.x, params.y, params.width, params.height );
}

void PixelStreamUpdater::_refreshTiles( const deflect::Segments& segments )
{
    // Update existing segments
    const size_t maxIndex = std::min( (size_t)_tiles.size(), segments.size( ));
    for( size_t i = 0; i < maxIndex; ++i )
    {
        Tile* tile = qobject_cast<Tile*>( _tiles[i] );
        tile->update( toRect( segments[i].parameters ));
    }

    const bool sizeChange = segments.size() != (size_t)_tiles.size();

    // Insert new objects in the vector if it is smaller
    for( size_t i = _tiles.size(); i < segments.size(); ++i )
        _tiles.push_back( new Tile( i, toRect( segments[i].parameters ),
                                    this, true ));

    // Or remove objects if it is bigger
    const size_t removeCount = _tiles.size() - segments.size();
    for( size_t i = 0; i < removeCount; ++i )
    {
        delete _tiles.back();
        _tiles.pop_back();
    }

    if( sizeChange )
        emit tilesChanged();
}
