/*********************************************************************/
/* Copyright (c) 2016, EPFL/Blue Brain Project                       */
<<<<<<< HEAD
/*                     Raphael Dumusc <raphael.dumusc@epfl.ch>       */
=======
/*                     Daniel.Nachbaur@epfl.ch                       */
>>>>>>> 9f82d03... Asynchronous texture upload for movies
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

#include "TextureFactory.h"

#include "ContentSynchronizer.h"

#include <QQuickWindow>

TextureFactory::TextureFactory( ContentSynchronizerSharedPtr synchronizer )
    : _synchronizer( synchronizer )
    , _textureSize()
{}

QSGTexture* TextureFactory::createTexture( QQuickWindow* window ) const
{
    uint textureID;
    glActiveTexture( GL_TEXTURE0 );
    glGenTextures( 1, &textureID );
    glBindTexture( GL_TEXTURE_2D, textureID );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, _textureSize.width(),
                  _textureSize.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, 0 );
    glBindTexture( GL_TEXTURE_2D, 0 );

    emit textureCreated( textureID );

    const auto flags = QQuickWindow::CreateTextureOptions(
                            QQuickWindow::TextureHasAlphaChannel |
                            QQuickWindow::TextureOwnsGLTexture );
    return window->createTextureFromId( textureID, _textureSize, flags );
}

QSize TextureFactory::textureSize() const
{
    return _textureSize;
}

int TextureFactory::textureByteCount() const
{
    return _textureSize.width() * _textureSize.height() * 4;
}
