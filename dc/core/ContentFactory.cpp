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

#include "ContentFactory.h"

#include "log.h"
#include "config.h"

#include "Content.h"
#include "TextureContent.h"
#include "DynamicTextureContent.h"
#include "SVGContent.h"
#include "MovieContent.h"
#if ENABLE_PDF_SUPPORT
#  include "PDFContent.h"
#  include "DisplayGroup.h"
#endif
#include "PixelStreamContent.h"

#include <QFile>
#include <QFileInfo>
#include <QImageReader>
#include <QTextStream>

#include <boost/make_shared.hpp>

#define ERROR_IMAGE_FILENAME ":/img/error.png"

namespace
{
const QSize maxTextureSize( 16384, 16384 );
}

CONTENT_TYPE getContentTypeForFile(const QString& uri)
{
    const QString extension = QFileInfo(uri).suffix().toLower();

    // SVGs must be processed first because they can also be read as an image
    if(SVGContent::getSupportedExtensions().contains(extension))
        return CONTENT_TYPE_SVG;

    if(MovieContent::getSupportedExtensions().contains(extension))
        return CONTENT_TYPE_MOVIE;

#if ENABLE_PDF_SUPPORT
    if(PDFContent::getSupportedExtensions().contains(extension))
        return CONTENT_TYPE_PDF;
#endif

    if(extension == "pyr")
        return CONTENT_TYPE_DYNAMIC_TEXTURE;

    // small images use Texture; large images use DynamicTexture
    const QImageReader imageReader(uri);
    if(imageReader.canRead())
    {
        const QSize size = imageReader.size();

        if(size.width() <= maxTextureSize.width() &&
           size.height() <= maxTextureSize.height())
            return CONTENT_TYPE_TEXTURE;

        return CONTENT_TYPE_DYNAMIC_TEXTURE;
    }

    return CONTENT_TYPE_ANY;
}

ContentPtr ContentFactory::getContent( const QString& uri )
{
    ContentPtr content;

    switch( getContentTypeForFile( uri ))
    {
    case CONTENT_TYPE_SVG:
        content = boost::make_shared<SVGContent>( uri );
        break;
    case CONTENT_TYPE_MOVIE:
        content = boost::make_shared<MovieContent>( uri );
        break;
#if ENABLE_PDF_SUPPORT
    case CONTENT_TYPE_PDF:
        content = boost::make_shared<PDFContent>( uri );
        break;
#endif
    case CONTENT_TYPE_DYNAMIC_TEXTURE:
        content = boost::make_shared<DynamicTextureContent>( uri );
        break;
    case CONTENT_TYPE_TEXTURE:
        content = boost::make_shared<TextureContent>( uri );
        break;
    case CONTENT_TYPE_ANY:
    default:
        break;
    }

    if( content && content->readMetadata( ))
        return content;

    return ContentPtr();
}

ContentPtr ContentFactory::getPixelStreamContent(const QString& uri)
{
    return ContentPtr(new PixelStreamContent(uri));
}

ContentPtr ContentFactory::getErrorContent()
{
    return ContentPtr(new TextureContent(ERROR_IMAGE_FILENAME));
}

const QStringList& ContentFactory::getSupportedExtensions()
{
    static QStringList extensions;

    if (extensions.empty())
    {
#if ENABLE_PDF_SUPPORT
        extensions.append(PDFContent::getSupportedExtensions());
#endif
        extensions.append(SVGContent::getSupportedExtensions());
        extensions.append(TextureContent::getSupportedExtensions());
        extensions.append(DynamicTextureContent::getSupportedExtensions());
        extensions.append(MovieContent::getSupportedExtensions());
        extensions.removeDuplicates();
    }

    return extensions;
}

const QStringList& ContentFactory::getSupportedFilesFilter()
{
    static QStringList filters;

    if (filters.empty())
    {
        const QStringList& extensions = getSupportedExtensions();
        foreach( const QString ext, extensions )
            filters.append( "*." + ext );
    }

    return filters;
}

QString ContentFactory::getSupportedFilesFilterAsString()
{
    const QStringList& extensions = getSupportedFilesFilter();

    QString s;
    QTextStream out(&s);

    out << "Content files (";
    foreach( const QString ext, extensions )
        out << ext << " ";
    out << ")";

    return s;
}
