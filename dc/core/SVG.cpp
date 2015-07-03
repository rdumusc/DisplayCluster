/*********************************************************************/
/* Copyright (c) 2011 - 2012, The University of Texas at Austin.     */
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

#include "SVG.h"

#include "log.h"
#include "ContentWindow.h"

namespace
{
const int MULTI_SAMPLE_ANTI_ALIASING_SAMPLES = 8;
const QSize PREVIEW_SIZE( 512, 512 );
}

SVG::SVG( const QString& uri )
{
    // flip the y texture coordinate since the textures are loaded upside down
    quad_.setTexCoords( QRectF( 0.0, 1.0, 1.0, -1.0 ));

    QFile file( uri );
    if( !file.open( QIODevice::ReadOnly ))
    {
        put_flog( LOG_WARN, "could not open file: '%s'",
                  uri.toLocal8Bit().constData( ));
        return;
    }
    if( !setImageData( file.readAll( )))
    {
        put_flog( LOG_WARN, "could not setImageData: '%s'",
                  uri.toLocal8Bit().constData( ));
        return;
    }
}

bool SVG::isValid() const
{
    return svgRenderer_.isValid();
}

QSize SVG::getSize() const
{
    return svgRenderer_.defaultSize();
}

void SVG::render()
{
    if( !textureData_.fbo )
        return;

    quad_.setTexture( textureData_.fbo->texture( ));
    quad_.enableAlphaBlending( true );
    quad_.render();
}

void SVG::renderPreview()
{
    if( !previewFbo_ )
        generatePreviewTexture();

    quad_.setTexture( previewFbo_->texture( ));
    quad_.enableAlphaBlending( false );
    quad_.render();
}

void SVG::preRenderUpdate( ContentWindowPtr window, const QRect& wallArea )
{
    if( window->isResizing( ))
        return;

    if( !QRectF( wallArea ).intersects( window->getCoordinates( )))
        return;

    const QSize& windowSize = window->getCoordinates().size().toSize();
    if( getTextureSize() != windowSize ||
        getTextureRegion() != window->getZoomRect( ))
    {
        updateTexture( windowSize, window->getZoomRect( ));
    }
}

QSize SVG::getTextureSize() const
{
    return textureData_.fbo ? textureData_.fbo->size() : QSize();
}

const QRectF& SVG::getTextureRegion() const
{
    return textureData_.region;
}

void SVG::updateTexture( const QSize& textureSize, const QRectF& svgRegion )
{
    if( !svgRenderer_.isValid( ))
        return;

    const bool recreateTextureFbo = !textureData_.fbo ||
                                    getTextureSize() != textureSize;

    if( recreateTextureFbo || svgRegion != textureData_.region )
    {
        if( recreateTextureFbo )
            textureData_.fbo.reset( new QGLFramebufferObject( textureSize ));

        renderToTexture( svgRegion, textureData_.fbo );
        textureData_.region = svgRegion;
    }

    assert( textureData_.fbo );
}

void SVG::generatePreviewTexture()
{
    previewFbo_.reset( new QGLFramebufferObject( PREVIEW_SIZE ));
    renderToTexture( UNIT_RECTF, previewFbo_ );
}

bool SVG::setImageData( const QByteArray& imageData )
{
    if( !svgRenderer_.load( imageData ) || !svgRenderer_.isValid( ))
        return false;

    svgExtents_ = svgRenderer_.viewBoxF();
    textureData_.region = QRectF();

    return true;
}

void SVG::renderToTexture( const QRectF& svgRegion, FBOPtr targetFbo )
{
    saveGLState();

    // Use a separate multisampled FBO for anti-aliased rendering
    FBOPtr renderFbo = createMultisampledFBO( targetFbo->size( ));
    QPainter painter( renderFbo.get( ));
    painter.setRenderHints( QPainter::Antialiasing |
                            QPainter::TextAntialiasing );
    svgRenderer_.setViewBox( getViewBox( svgRegion ));
    svgRenderer_.render( &painter );
    painter.end();

    // Blit to target texture FBO
    const QRect blitRect( 0, 0, renderFbo->width(), renderFbo->height( ));
    QGLFramebufferObject::blitFramebuffer( targetFbo.get(), blitRect,
                                           renderFbo.get(), blitRect );

    // Set texture parameters
    glBindTexture( GL_TEXTURE_2D, targetFbo->texture( ));
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );

    restoreGLState();
}

QRectF SVG::getViewBox( const QRectF& svgRegion )
{
    return QRectF( svgExtents_.x() + svgRegion.x() * svgExtents_.width(),
                   svgExtents_.y() + svgRegion.y() * svgExtents_.height(),
                   svgRegion.width() * svgExtents_.width(),
                   svgRegion.height() * svgExtents_.height( ));
}

FBOPtr SVG::createMultisampledFBO( const QSize& size )
{
    QGLFramebufferObjectFormat format;
    format.setAttachment( QGLFramebufferObject::CombinedDepthStencil );
    format.setSamples( MULTI_SAMPLE_ANTI_ALIASING_SAMPLES );

    FBOPtr fbo( new QGLFramebufferObject( size, format ));
    fbo->bind();
    glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );
    glClear( GL_COLOR_BUFFER_BIT );
    fbo->release();

    return fbo;
}

void SVG::saveGLState()
{
    glPushAttrib( GL_ALL_ATTRIB_BITS );
    glMatrixMode( GL_PROJECTION );
    glPushMatrix();
    glMatrixMode( GL_MODELVIEW );
    glPushMatrix();
}

void SVG::restoreGLState()
{
    glMatrixMode( GL_PROJECTION );
    glPopMatrix();
    glMatrixMode( GL_MODELVIEW );
    glPopMatrix();
    glPopAttrib();
}
