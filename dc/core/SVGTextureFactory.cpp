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

#include "SVGTextureFactory.h"

#include "log.h"
#include "types.h" // for make_unique()

#include <QQuickWindow>
#include <QOpenGLPaintDevice>
#include <QOpenGLFramebufferObject>
#include <QPainter>

namespace
{
const int MULTI_SAMPLE_ANTI_ALIASING_SAMPLES = 8;
}

QRectF getViewBox( const QRectF& zoomRect, const QRectF& svgExtents )
{
    return QRectF( svgExtents.x() + zoomRect.x() * svgExtents.width(),
                   svgExtents.y() + zoomRect.y() * svgExtents.height(),
                   zoomRect.width() * svgExtents.width(),
                   zoomRect.height() * svgExtents.height( ));
}

SVGTextureFactory::SVGTextureFactory( const QString& uri,
                                      const QSize& textureSize_,
                                      const QRectF& zoomRect )
    : _textureSize( textureSize_ )
{
    QFile file( uri );
    if( !file.open( QIODevice::ReadOnly ))
    {
        put_flog( LOG_WARN, "could not open file: '%s'",
                  uri.toLocal8Bit().constData( ));
        return;
    }

    if( !_svgRenderer.load( file.readAll( )) || !_svgRenderer.isValid( ))
    {
        put_flog( LOG_WARN, "could not setImageData: '%s'",
                  uri.toLocal8Bit().constData( ));
        return;
    }

    if( _textureSize.isEmpty( ))
        _textureSize = _svgRenderer.defaultSize();

    _svgRenderer.setViewBox( getViewBox( zoomRect, _svgRenderer.viewBoxF( )));
}

void _saveGLState()
{
    glPushAttrib( GL_ALL_ATTRIB_BITS );
    glMatrixMode( GL_PROJECTION );
    glPushMatrix();
    glMatrixMode( GL_MODELVIEW );
    glPushMatrix();
}

void _restoreGLState()
{
    glMatrixMode( GL_PROJECTION );
    glPopMatrix();
    glMatrixMode( GL_MODELVIEW );
    glPopMatrix();
    glPopAttrib();
}

std::unique_ptr<QOpenGLFramebufferObject>
_createMultisampledFBO( const QSize& size )
{
    QOpenGLFramebufferObjectFormat format;
    format.setAttachment( QOpenGLFramebufferObject::CombinedDepthStencil );
    format.setSamples( MULTI_SAMPLE_ANTI_ALIASING_SAMPLES );

    auto fbo = make_unique<QOpenGLFramebufferObject>( size, format );
    fbo->bind();
    glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );
    glClear( GL_COLOR_BUFFER_BIT );
    fbo->release();

    return std::move( fbo );
}

QSGTexture* SVGTextureFactory::createTexture( QQuickWindow* window ) const
{
    // We have the window's OpenGL context bound here
    _saveGLState();

    // Use a separate multisampled FBO for anti-aliased rendering
    auto renderFbo = _createMultisampledFBO( textureSize( ));
    renderFbo->bind();
    // the paint device acts on the currently bound fbo
    QOpenGLPaintDevice device( renderFbo->size( ));
    device.setPaintFlipped( true );
    QPainter painter( &device );
    painter.setRenderHints( QPainter::Antialiasing |
                            QPainter::TextAntialiasing );
    _svgRenderer.render( &painter );
    painter.end();
    renderFbo->release();
    _restoreGLState();

    // Blit to target texture FBO
    const QRect blitRect( 0, 0, renderFbo->width(), renderFbo->height( ));
    QOpenGLFramebufferObject targetFbo( renderFbo->size( ));
    QOpenGLFramebufferObject::blitFramebuffer( &targetFbo, blitRect,
                                               renderFbo.get(), blitRect );

    const auto flags = QQuickWindow::CreateTextureOptions(
                            QQuickWindow::TextureHasAlphaChannel |
                            QQuickWindow::TextureOwnsGLTexture );
    return window->createTextureFromId( targetFbo.takeTexture(),
                                        targetFbo.size(), flags );
}

QSize SVGTextureFactory::textureSize() const
{
    return _textureSize;
}

int SVGTextureFactory::textureByteCount() const
{
    return _textureSize.width() * _textureSize.height() * 4;
}
