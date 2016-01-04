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

#include "PixelStreamProvider.h"

#include "PixelStreamUpdater.h"
#include "log.h"

#include <deflect/Frame.h>
#include <deflect/SegmentDecoder.h>

const QString PixelStreamProvider::ID( "pixelstream" );

PixelStreamProvider::PixelStreamProvider()
    : QQuickImageProvider( QQmlImageProviderBase::Image )
{}

QImage PixelStreamProvider::requestImage( const QString& id, QSize* size,
                                          const QSize& requestedSize )
{
    QStringList params = id.split( "?" );
    if( params.length() != 3 )
        return QImage();

    const QString& streamUri = params[0];

    bool ok = false;
    const uint frameIndex = params[1].toUInt( &ok );
    if( !ok )
        return QImage();
    Q_UNUSED( frameIndex );

    ok = false;
    const uint tileIndex = params[2].toUInt( &ok );
    if( !ok )
        return QImage();

    if( !_streams.count( streamUri ))
        return QImage();

    QImage image;
    try {
        image = _streams[streamUri]->getTileImage( tileIndex );
    }
    catch ( const std::out_of_range& ) {
        put_flog( LOG_DEBUG, "Invalid tile requested: %d", tileIndex );
        return QImage();
    }
    catch ( const std::runtime_error& e ) {
        put_flog( LOG_DEBUG, "%s", e.what( ));
        return QImage();
    }

    if( !requestedSize.isEmpty( ))
        image = image.scaled( requestedSize );

    *size = image.size();

    return image;
}

void PixelStreamProvider::setNewFrame( deflect::FramePtr frame )
{
    if( !_streams.count( frame->uri ))
        return;

    _streams[frame->uri]->updatePixelStream( frame );
}

PixelStreamUpdaterSharedPtr PixelStreamProvider::open( const QString& stream )
{
    if( !_streams.count( stream ))
    {
        _streams[ stream ] = std::make_shared<PixelStreamUpdater>();
        connect( _streams[ stream ].get(), &PixelStreamUpdater::requestFrame,
                 this, &PixelStreamProvider::requestFrame );
    }
    return _streams[ stream ];
}

void PixelStreamProvider::close( const QString& stream )
{
    _streams.erase( stream );
}

void PixelStreamProvider::update( WallToWallChannel& channel )
{
    for( auto& stream : _streams )
        stream.second->synchronizeFramesSwap( channel );
}
