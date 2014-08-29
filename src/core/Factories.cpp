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

#include "Factories.h"

#include "Content.h"
#include "DisplayGroup.h"
#include "ContentWindow.h"
#include "PixelStreamFrame.h"

#include <boost/foreach.hpp>

Factories::Factories(const Factory<FactoryObject>::NewObjectFunc& func)
    : frameIndex_(0)
    , textureFactory_(func)
    , dynamicTextureFactory_(func)
#if ENABLE_PDF_SUPPORT
    , pdfFactory_(func)
#endif
    , svgFactory_(func)
    , movieFactory_(func)
    , pixelStreamFactory_(func)
{
}

void Factories::clearStaleFactoryObjects()
{
    textureFactory_.clearStaleObjects(frameIndex_);
    dynamicTextureFactory_.clearStaleObjects(frameIndex_);
#if ENABLE_PDF_SUPPORT
    pdfFactory_.clearStaleObjects(frameIndex_);
#endif
    svgFactory_.clearStaleObjects(frameIndex_);
    movieFactory_.clearStaleObjects(frameIndex_);
    pixelStreamFactory_.clearStaleObjects(frameIndex_);

    ++frameIndex_;
}

void Factories::clear()
{
    textureFactory_.clear();
    dynamicTextureFactory_.clear();
#if ENABLE_PDF_SUPPORT
    pdfFactory_.clear();
#endif
    svgFactory_.clear();
    movieFactory_.clear();
    pixelStreamFactory_.clear();
}

void Factories::preRenderUpdate(DisplayGroup& displayGroup, WallToWallChannel& wallChannel)
{
    ContentWindowPtrs contentWindows = displayGroup.getContentWindows();

    // note that if we have multiple ContentWindows for a single Content object,
    // we will call advance() multiple times per frame on that Content object...
    BOOST_FOREACH(ContentWindowPtr contentWindow, contentWindows)
    {
        contentWindow->getContent()->preRenderUpdate(*this, contentWindow, wallChannel);
    }
    ContentWindowPtr backgroundWindow = displayGroup.getBackgroundContentWindow();
    if (backgroundWindow)
        backgroundWindow->getContent()->preRenderUpdate(*this, backgroundWindow, wallChannel);
}

void Factories::postRenderUpdate(DisplayGroup& displayGroup, WallToWallChannel& wallChannel)
{
    ContentWindowPtrs contentWindows = displayGroup.getContentWindows();

    // note that if we have multiple ContentWindows for a single Content object,
    // we will call advance() multiple times per frame on that Content object...
    BOOST_FOREACH(ContentWindowPtr contentWindow, contentWindows)
    {
        contentWindow->getContent()->postRenderUpdate(*this, contentWindow, wallChannel);
    }
    ContentWindowPtr backgroundWindow = displayGroup.getBackgroundContentWindow();
    if (backgroundWindow)
        backgroundWindow->getContent()->postRenderUpdate(*this, backgroundWindow, wallChannel);

    clearStaleFactoryObjects();
}

FactoryObjectPtr Factories::getFactoryObject(ContentPtr content)
{
    FactoryObjectPtr object;
    switch (content->getType())
    {
    case CONTENT_TYPE_TEXTURE:
        object = textureFactory_.getObject(content->getURI());
        break;
    case CONTENT_TYPE_DYNAMIC_TEXTURE:
        object = dynamicTextureFactory_.getObject(content->getURI());
        break;
#if ENABLE_PDF_SUPPORT
    case CONTENT_TYPE_PDF:
        object = pdfFactory_.getObject(content->getURI());
        break;
#endif
    case CONTENT_TYPE_SVG:
        object = svgFactory_.getObject(content->getURI());
        break;
    case CONTENT_TYPE_MOVIE:
        object = movieFactory_.getObject(content->getURI());
        break;
    case CONTENT_TYPE_PIXEL_STREAM:
        object = pixelStreamFactory_.getObject(content->getURI());
        break;
    default:
        return FactoryObjectPtr();
        break;
    }
    object->setFrameIndex(frameIndex_);
    return object;
}

void Factories::updatePixelStream(PixelStreamFramePtr frame)
{
    typedef boost::shared_ptr<PixelStream> PixelStreamPtr;
    PixelStreamPtr pixelStream = pixelStreamFactory_.getObject(frame->uri);
    pixelStream->setNewFrame(frame);
    pixelStream->setFrameIndex(frameIndex_);
}

Factory<Texture> & Factories::getTextureFactory()
{
    return textureFactory_;
}

Factory<DynamicTexture> & Factories::getDynamicTextureFactory()
{
    return dynamicTextureFactory_;
}

#if ENABLE_PDF_SUPPORT
Factory<PDF> & Factories::getPDFFactory()
{
    return pdfFactory_;
}
#endif

Factory<SVG> & Factories::getSVGFactory()
{
    return svgFactory_;
}

Factory<Movie> & Factories::getMovieFactory()
{
    return movieFactory_;
}

Factory<PixelStream> & Factories::getPixelStreamFactory()
{
    return pixelStreamFactory_;
}
