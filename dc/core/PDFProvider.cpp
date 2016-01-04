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

#include "PDFProvider.h"

#include "PDF.h"
#include "ImageProviderStringifier.h"

namespace
{
const QSize PREVIEW_SIZE( 512, 512 );
}

const QString PDFProvider::ID( "pdf" );

PDFProvider::PDFProvider()
    : QQuickImageProvider( QQmlImageProviderBase::Image,
                           ForceAsynchronousImageLoading )
{}

PDFProvider::~PDFProvider() {}

QImage PDFProvider::requestImage( const QString& id, QSize* size,
                                  const QSize& requestedSize )
{
    QStringList list = id.split("#");
    if( list.size() != 2 )
        return QImage();

    PDF pdf( list.at( 0 ));
    if( !pdf.isValid( ))
        return QImage();

    bool ok = false;
    const int pageNumber = list.at( 1 ).toInt( &ok );
    if( !ok || !pdf.isValid( pageNumber ))
        return QImage();
    pdf.setPage( pageNumber );

    QRectF zoomRect( UNIT_RECTF );
    if( list.size() > 2 )
        zoomRect = destringify( list.at( 2 ));

    const QSize targetSize( requestedSize.isEmpty() ? pdf.getSize() :
                                                      requestedSize );
    const QImage image = pdf.renderToImage( targetSize, zoomRect );
    if( size )
        *size = image.size();

    return image;
}
