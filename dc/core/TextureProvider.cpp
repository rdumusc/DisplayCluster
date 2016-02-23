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

#include "TextureProvider.h"

#include "Content.h"
#include "ContentFactory.h"
#include "ContentSynchronizer.h"
#include "PixelStreamSynchronizer.h"
#include "SVGTextureFactory.h"
#include "TextureFactory.h"
#include "TextureUploader.h"

#include "log.h"

#include <deflect/Frame.h>

const QString TextureProvider::ID( "texture" );

TextureProvider::TextureProvider( TextureUploader* uploader )
    : QQuickImageProvider( QQmlImageProviderBase::Texture )
    , _uploader( uploader )
{}

TextureProvider::~TextureProvider() {}

QQuickTextureFactory*
TextureProvider::requestTexture( const QString& id, QSize* size,
                                 const QSize& requestedSize )
{
    QStringList params = id.split( "?" );
    if( params.size() != 2 )
        return nullptr;

    const QString& uri = params[0];

    bool ok = false;
    const uint tileIndex = params[1].toUInt( &ok );
    if( !ok )
    {
        put_flog( LOG_WARN, "No tile index specified for texture id: %s",
                  id.toLocal8Bit().constData( ));
        return nullptr;
    }

    QQuickTextureFactory* factory = nullptr;
    if( ContentFactory::getContentTypeForFile( uri ) == CONTENT_TYPE_SVG )
        factory = new SVGTextureFactory( uri, requestedSize, UNIT_RECTF );
    else
    {
        Tile* tile = _synchronizers[uri]->getTiles().get( tileIndex );
        if( !tile )
            return nullptr;
        factory = new TextureFactory( *tile );
    }

    if( size )
        *size = factory->textureSize();
    return factory;
}

ContentSynchronizerSharedPtr TextureProvider::open( ContentPtr content )
{
    const QString& uri = content->getURI();

    if( !_synchronizers.count( uri ))
    {
        _synchronizers[uri] = ContentSynchronizer::create( content );

        if( content->getType() == CONTENT_TYPE_PIXEL_STREAM )
        {
            auto synchronizer = dynamic_cast<PixelStreamSynchronizer*>(
                                    _synchronizers[uri].get( ));
            connect( synchronizer, &PixelStreamSynchronizer::requestFrame,
                     this, &TextureProvider::requestFrame );
        }
    }
    return _synchronizers[uri];
}

void TextureProvider::close( const QString& uri )
{
    _synchronizers.erase( uri );
}

void TextureProvider::setNewFrame( deflect::FramePtr frame )
{
    if( !_synchronizers.count( frame->uri ))
        return;

    auto synchronizer = dynamic_cast<PixelStreamSynchronizer*>(
                            _synchronizers[frame->uri].get( ));
    if( !synchronizer )
        return;

    synchronizer->updatePixelStream( frame );
}
