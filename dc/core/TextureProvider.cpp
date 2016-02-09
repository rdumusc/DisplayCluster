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

#include "log.h"
#include "DynamicTexture.h"

const QString TextureProvider::ID( "texture" );

TextureProvider::TextureProvider()
    : QQuickImageProvider( QQmlImageProviderBase::Image )
{}

TextureProvider::~TextureProvider() {}

QImage TextureProvider::requestImage( const QString& id, QSize* size,
                                      const QSize& /*requestedSize*/ )
{
    const QStringList params = id.split( "?" );

    QImage image;
    if( params.length() == 1 )
    {
        if( params[0].endsWith( ".pyr" ))
            image = DynamicTexture( params[0] ).getRootImage();
        else
            image = QImage( params[0] );
    }
    else if( params.length() == 2 )
    {
        const QString& pyramidFile = params[0];

        if( !_dynamicTextures.count( pyramidFile ))
            return QImage();

        bool ok = false;
        const int tileIndex = params[1].toInt( &ok );
        if( !ok )
            return QImage();

        image = _dynamicTextures[pyramidFile]->getTileImage( tileIndex );
    }

    if( image.isNull( ))
        return QImage();

    if( size )
        *size = image.size();

    return image;
}

DynamicTexturePtr TextureProvider::openDynamicTexture( const QString& uri )
{
    if( !_dynamicTextures.count( uri ))
        _dynamicTextures[ uri ] = std::make_shared< DynamicTexture >( uri );
    return _dynamicTextures[ uri ];
}

void TextureProvider::closeDynamicTexture( const QString& uri )
{
    _dynamicTextures.erase( uri );
}
