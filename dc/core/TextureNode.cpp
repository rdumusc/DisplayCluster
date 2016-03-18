/*********************************************************************/
/* Copyright (c) 2016, EPFL/Blue Brain Project                       */
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

#include "TextureNode.h"

#include "Image.h"

#include <QQuickWindow>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <cstring>  // memcpy

TextureNode::TextureNode( const QSize& size, QQuickWindow* window )
    : _window( window )
    , _gl( _window->openglContext()->functions( ))
    , _frontTexture( window->createTextureFromId( 0 , QSize( 1 ,1 )))
    , _backTexture( _createTexture( size ))
    , _pixelBuffer( QOpenGLBuffer::PixelUnpackBuffer )
    , _readyToSwap( false )
{
    setTexture( _frontTexture.get( ));
    setFiltering( QSGTexture::Linear );
    setMipmapFiltering( QSGTexture::Linear );
    setFlag( QSGNode::UsePreprocess, true );

    _gl->initializeOpenGLFunctions();
    _pixelBuffer.create();
    _pixelBuffer.setUsagePattern( QOpenGLBuffer::StreamDraw );
}

void TextureNode::setMipmapFiltering( const QSGTexture::Filtering mipmapFiltering )
{
    auto mat = static_cast< QSGOpaqueTextureMaterial* >( material( ));
    auto opaqueMat = static_cast< QSGOpaqueTextureMaterial* >( opaqueMaterial( ));

    mat->setMipmapFiltering( mipmapFiltering );
    opaqueMat->setMipmapFiltering( mipmapFiltering );
}

void TextureNode::updateBackTexture( ImagePtr image )
{
    _image = image;
    if( _image )
    {
        setBackTextureSize( QSize( _image->getWidth(), _image->getHeight( )));
        _upload( *_image, _backTexture->textureId( ));
        _image.reset();
    }
}

uint TextureNode::getBackGlTexture() const
{
    return _backTexture->textureId();
}

void TextureNode::swap()
{
    std::swap( _frontTexture, _backTexture );

    setTexture( _frontTexture.get( ));
    markDirty( DirtyMaterial );
    _readyToSwap = false;
}

void TextureNode::setBackTextureSize( const QSize& size )
{
    if( _backTexture->textureSize() == size )
        return;

    _backTexture = _createTexture( size );
}

TextureNode::QSGTexturePtr
TextureNode::_createTexture( const QSize& size ) const
{
    uint textureID;
    glActiveTexture( GL_TEXTURE0 );
    glGenTextures( 1, &textureID );
    glBindTexture( GL_TEXTURE_2D, textureID );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA8, size.width(), size.height(),
                  0, GL_RGBA, GL_UNSIGNED_BYTE, 0 );
    glBindTexture( GL_TEXTURE_2D, 0 );

    return _createWrapper( textureID, size );
}

TextureNode::QSGTexturePtr
TextureNode::_createWrapper( const uint textureID, const QSize& size ) const
{
    const auto textureFlags = QQuickWindow::CreateTextureOptions(
                                  QQuickWindow::TextureHasAlphaChannel |
                                  QQuickWindow::TextureHasMipmaps |
                                  QQuickWindow::TextureOwnsGLTexture );
    return QSGTexturePtr( _window->createTextureFromId( textureID, size,
                                                        textureFlags ));
}

void TextureNode::_upload( const Image& image, const uint textureID )
{
    const size_t bufferSize = image.getSize();

    _pixelBuffer.bind();
    _pixelBuffer.allocate( bufferSize ); // make PBO big enough

    // copy pixels from CPU mem to GPU mem
    void* pboData = _pixelBuffer.map( QOpenGLBuffer::WriteOnly );
    std::memcpy( pboData, image.getData(), bufferSize );
    _pixelBuffer.unmap();

    // setup PBO and texture pixel storage
    GLint alignment = 1;
    if( (image.getWidth() % 4) == 0 )
        alignment = 4;
    else if( (image.getWidth() % 2) == 0 )
        alignment = 2;

    glPushClientAttrib( GL_CLIENT_PIXEL_STORE_BIT );
    _gl->glPixelStorei( GL_UNPACK_ALIGNMENT, alignment );
    _gl->glPixelStorei( GL_UNPACK_ROW_LENGTH, image.getWidth( ));

    // update texture with pixels from PBO
    _gl->glActiveTexture( GL_TEXTURE0 );
    _gl->glBindTexture( GL_TEXTURE_2D, textureID );
    _gl->glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, image.getWidth(),
                          image.getHeight(), image.getFormat(),
                          GL_UNSIGNED_BYTE, 0 );
    glPopClientAttrib();

    _pixelBuffer.release();

    _gl->glGenerateMipmap( GL_TEXTURE_2D );
    _gl->glBindTexture( GL_TEXTURE_2D, 0 );

    // Ensure the texture upload is complete before the render thread uses it
    _gl->glFinish();
    _readyToSwap = true;
    emit backTextureReady();
}
