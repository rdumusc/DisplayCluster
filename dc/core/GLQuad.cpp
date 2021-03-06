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

#include "GLQuad.h"

#include "types.h"

#include <QtOpenGL/qgl.h>

GLQuad::GLQuad()
    : texCoords_( UNIT_RECTF )
    , renderMode_( GL_QUADS )
    , textureId_( 0 )
    , alphaBlending_( false )
{
}

void GLQuad::setTexCoords( const QRectF& texCoords )
{
    texCoords_ = texCoords;
}

void GLQuad::setTexture( const GLuint textureId )
{
    textureId_ = textureId;
}

void GLQuad::setRenderMode( const GLenum mode )
{
    if( mode == GL_QUADS || mode == GL_LINE_LOOP )
        renderMode_ = mode;
}

void GLQuad::enableAlphaBlending( const bool value )
{
    alphaBlending_ = value;
}

void GLQuad::render()
{
    glPushAttrib( GL_ENABLE_BIT | GL_TEXTURE_BIT );

    if( textureId_ )
    {
        glEnable( GL_TEXTURE_2D );
        glBindTexture( GL_TEXTURE_2D, textureId_ );
    }
    else
        glDisable( GL_TEXTURE_2D );

    if( alphaBlending_ )
    {
        glEnable( GL_BLEND );
        glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    }

    glBegin( renderMode_ );

    glTexCoord2f( texCoords_.x(), texCoords_.y( ));
    glVertex2f( 0.f, 0.f );

    glTexCoord2f( texCoords_.x() + texCoords_.width(), texCoords_.y( ));
    glVertex2f( 1.f, 0.f );

    glTexCoord2f( texCoords_.x() + texCoords_.width(),
                  texCoords_.y() + texCoords_.height( ));
    glVertex2f( 1.f, 1.f );

    glTexCoord2f( texCoords_.x(), texCoords_.y() + texCoords_.height( ));
    glVertex2f( 0.f, 1.f );

    glEnd();

    glPopAttrib();
}
