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

template <typename T>
class ScopedIncrementer
{
public:
    ScopedIncrementer( T& object, std::mutex& mutex )
        : _object( object )
        , _mutex( mutex )
    {
        std::lock_guard<std::mutex> lock( _mutex );
        ++_object;
    }
    ~ScopedIncrementer()
    {
        std::lock_guard<std::mutex> lock( _mutex );
        --_object;
    }

private:
    T& _object;
    std::mutex& _mutex;
};

PixelStreamUpdater::PixelStreamUpdater()
    : _decodeCount( 0 )
    , _frameIndex( 0 )
{
    _swapSyncFrame.setCallback( boost::bind(
                                    &PixelStreamUpdater::_onFrameSwapped,
                                    this, _1 ));
}

void PixelStreamUpdater::synchronizeFramesSwap( WallToWallChannel& channel )
{
    // Block calls to getTile() by holding the lock until the frame has
    // been swapped
    std::lock_guard<std::mutex> lock( _mutex );

    if( !channel.allReady( _decodeCount == 0 ))
        return;

    const SyncFunction& versionCheckFunc =
        boost::bind( &WallToWallChannel::checkVersion, &channel, _1 );

    _swapSyncFrame.sync( versionCheckFunc );
}

const QList<QObject*>& PixelStreamUpdater::getTiles() const
{
    return _tiles;
}

QImage PixelStreamUpdater::getTileImage( const uint index )
{
    // Ensure that this operation does not conflict with a frame swap
    ScopedIncrementer<int> increment( _decodeCount, _mutex );

    if( !_swapSyncFrame.get( ))
        throw std::runtime_error( "No frames yet" );

    auto& segment = _swapSyncFrame.get()->segments.at( index );

    if( segment.parameters.compressed )
        _decoders[index]->decode( segment );

    return QImage( (const uchar*)segment.imageData.constData(),
                   segment.parameters.width, segment.parameters.height,
                   QImage::Format_RGBX8888 );
}

void PixelStreamUpdater::updatePixelStream( deflect::FramePtr frame )
{
    _swapSyncFrame.update( frame );
}

void PixelStreamUpdater::_onFrameSwapped( deflect::FramePtr frame )
{
    std::sort( frame->segments.begin(), frame->segments.end(),
               []( const deflect::Segment& s1, const deflect::Segment& s2 )
        {
            return ( s1.parameters.y < s2.parameters.y ||
                     s1.parameters.x < s2.parameters.x );
        } );

    _adjustFrameDecodersCount( frame->segments.size( ));
    _refreshTiles( frame->segments );

    emit pictureUpdated( _frameIndex++ );
    emit requestFrame( frame->uri );
}

void PixelStreamUpdater::_adjustFrameDecodersCount( const size_t count )
{
    while( _decoders.size() < count )
        _decoders.push_back( make_unique<deflect::SegmentDecoder>( ));
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
        _tiles.push_back( new Tile( i, toRect( segments[i].parameters ), this));

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
