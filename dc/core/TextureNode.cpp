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

#include <QQuickWindow>

TextureNode::TextureNode( const QSize& size, QQuickWindow* window )
    : _size( size )
    , _frontTexture( _createTexture( window ))
    , _backTexture( _createTexture( window ))
{
    setTexture( _frontTexture.get( ));
    setFiltering( QSGTexture::Nearest );
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
}

TextureNode::QSGTexturePtr
TextureNode::_createTexture( QQuickWindow* window ) const
{
    uint textureID;
    glActiveTexture( GL_TEXTURE0 );
    glGenTextures( 1, &textureID );
    glBindTexture( GL_TEXTURE_2D, textureID );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, _size.width(), _size.height(),
                  0, GL_RGBA, GL_UNSIGNED_BYTE, 0 );
    glBindTexture( GL_TEXTURE_2D, 0 );

    const auto textureFlags = QQuickWindow::CreateTextureOptions(
                                  QQuickWindow::TextureHasAlphaChannel |
                                  QQuickWindow::TextureOwnsGLTexture );
    return QSGTexturePtr( window->createTextureFromId( textureID, _size,
                                                       textureFlags ));
}
