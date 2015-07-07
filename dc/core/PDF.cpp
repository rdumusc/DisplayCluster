/*********************************************************************/
/* Copyright (c) 2013, EPFL/Blue Brain Project                       */
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

#include "PDF.h"

// detect Qt version
#if QT_VERSION >= 0x050000
#define POPPLER_QT5
#include <poppler-qt5.h>
#elif QT_VERSION >= 0x040000
#define POPPLER_QT4
#include <poppler-qt4.h>
#else
#error PopplerPixelStreamer needs Qt4 or Qt5
#endif

#include "ContentWindow.h"
#include "PDFContent.h"
#include "log.h"

namespace
{
const int INVALID_PAGE_NUMBER = -1;
const qreal PDF_RES = 72.0;
const QSize PREVIEW_SIZE( 512, 512 );
}

PDF::PDF( const QString& uri )
    : pdfDoc_( 0 )
    , pdfPage_( 0 )
    , pageNumber_( INVALID_PAGE_NUMBER )
{
    openDocument( uri );
}

PDF::~PDF()
{
    closeDocument();
}

bool PDF::isValid() const
{
    return ( pdfDoc_ != 0 );
}

QSize PDF::getSize() const
{
    return pdfPage_ ? pdfPage_->pageSize() : QSize();
}

int PDF::getPage() const
{
    return pageNumber_;
}

void PDF::setPage( const int pageNumber )
{
    if( pageNumber == pageNumber_ || !isValid( pageNumber ))
        return;

    Poppler::Page* page = pdfDoc_->page( pageNumber );
    if( !page )
    {
        put_flog( LOG_WARN, "Could not open page: %d in PDF document: '%s'",
                  pageNumber, filename_.toLocal8Bit().constData( ));
        return;
    }

    closePage();
    pdfPage_ = page;
    pageNumber_ = pageNumber;
}

int PDF::getPageCount() const
{
    return pdfDoc_->numPages();
}

QImage PDF::renderToImage( const QSize& imageSize, const QRectF& region ) const
{
    const QSize pageSize( pdfPage_->pageSize( ));

    const qreal zoomX = 1.0 / region.width();
    const qreal zoomY = 1.0 / region.height();

    const QPoint topLeft( region.x() * imageSize.width(),
                          region.y() * imageSize.height( ));

    const qreal resX = PDF_RES * imageSize.width() / pageSize.width();
    const qreal resY = PDF_RES * imageSize.height() / pageSize.height();

    return pdfPage_->renderToImage( resX * zoomX, resY * zoomY,
                                    topLeft.x() * zoomX, topLeft.y() * zoomY,
                                    imageSize.width(), imageSize.height( ));
}

const QSize& PDF::getTextureSize() const
{
    return texture_.getSize();
}

const QRectF& PDF::getTextureRegion() const
{
    return textureRect_;
}

void PDF::updateTexture( const QSize& textureSize, const QRectF& pdfRegion )
{
    const QImage image = renderToImage( textureSize, pdfRegion );

    if( image.isNull( ))
    {
        put_flog( LOG_ERROR, "Could not render page in PDF document: '%s'",
                  filename_.toLocal8Bit().constData( ));
        return;
    }

    texture_.update( image, GL_BGRA );
    textureRect_ = pdfRegion;
}

void PDF::render()
{
    if( !texture_.isValid( ))
        return;

    quad_.setTexture( texture_.getTextureId( ));
    quad_.render();
}

void PDF::renderPreview()
{
    if( !texturePreview_.isValid( ))
    {
        const QImage image = renderToImage( PREVIEW_SIZE );
        if( image.isNull( ))
        {
            put_flog( LOG_ERROR, "Could not render document preview for: '%s'",
                      filename_.toLocal8Bit().constData( ));
        }
        texturePreview_.update( image, GL_BGRA );
    }

    quad_.setTexture( texturePreview_.getTextureId( ));
    quad_.render();
}

void PDF::openDocument( const QString& filename )
{
    closeDocument();

    pdfDoc_ = Poppler::Document::load( filename );
    if ( !pdfDoc_ || pdfDoc_->isLocked( ))
    {
        put_flog( LOG_DEBUG, "Could not open document: '%s'",
                  filename_.toLocal8Bit().constData( ));
        closeDocument();
        return;
    }

    filename_ = filename;
    pdfDoc_->setRenderHint( Poppler::Document::TextAntialiasing );

    setPage( 0 );
}

void PDF::closeDocument()
{
    if( pdfDoc_ )
    {
        closePage();
        delete pdfDoc_;
        pdfDoc_ = 0;
        filename_.clear();
    }
}

void PDF::closePage()
{
    if( pdfPage_ )
    {
        delete pdfPage_;
        pdfPage_ = 0;
        pageNumber_ = INVALID_PAGE_NUMBER;
        textureRect_ = QRectF();
    }
}

bool PDF::isValid( const int pageNumber ) const
{
    return pageNumber >=0 && pageNumber < pdfDoc_->numPages();
}

void PDF::preRenderUpdate( ContentWindowPtr window, const QRect& /*wallArea*/ )
{
    if( window->isResizing( ))
        return;

    PDFContent& content = static_cast<PDFContent&>( *window->getContent( ));

    const bool pageHasChanged = (pageNumber_ != content.getPage( ));
    setPage( content.getPage( ));

    const QSize& windowSize = window->getCoordinates().size().toSize();
    if( pageHasChanged || texture_.getSize() != windowSize ||
        textureRect_ != window->getZoomRect( ) )
    {
        updateTexture( windowSize, window->getZoomRect( ));
    }
}

