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

#ifndef PDF_H
#define PDF_H

#include "WallContent.h"

#include "GLTexture2D.h"
#include "GLQuad.h"

#include <QtCore/QString>

namespace Poppler
{
    class Document;
    class Page;
}

class PDF : public WallContent
{
public:
    PDF( const QString& uri );
    ~PDF();

    bool isValid() const;

    QSize getSize() const;

    int getPage() const;
    void setPage( const int pageNumber );

    int getPageCount() const;

    QImage renderToImage( const QSize& imageSize,
                          const QRectF& region = UNIT_RECTF ) const;

private:
    Poppler::Document* pdfDoc_;
    Poppler::Page* pdfPage_;
    int pageNumber_;

    GLTexture2D texture_;
    GLTexture2D texturePreview_;
    GLQuad quad_;
    QRectF textureRect_;

    void openDocument( const QString& filename );
    void closeDocument();
    void closePage();
    bool isValid( const int pageNumber ) const;

    const QSize& getTextureSize() const;
    const QRectF& getTextureRegion() const;
    void updateTexture( const QSize& textureSize, const QRectF& pdfRegion );
    void render() override;
    void renderPreview() override;
    void preRenderUpdate( ContentWindowPtr window,
                          const QRect& wallArea ) override;
};

#endif // PDF_H
