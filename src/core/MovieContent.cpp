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

#include "MovieContent.h"

#include "Movie.h"
#include "FFMPEGMovie.h"
#include "ContentWindow.h"
#include "RenderContext.h"
#include "Factories.h"
#include "WallToWallChannel.h"

#include "serializationHelpers.h"
#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT_GUID(MovieContent, "MovieContent")

MovieContent::MovieContent(const QString& uri)
    : Content(uri)
{
}

CONTENT_TYPE MovieContent::getType()
{
    return CONTENT_TYPE_MOVIE;
}

bool MovieContent::readMetadata()
{
    QFileInfo file( getURI( ));
    if (!file.exists() || !file.isReadable())
        return false;

    const FFMPEGMovie movie(getURI());
    if (!movie.isValid())
        return false;

    size_ = QSize( movie.getWidth(), movie.getHeight());
    return true;
}

const QStringList& MovieContent::getSupportedExtensions()
{
    static QStringList extensions;

    if (extensions.empty())
    {
        extensions << "mov" << "avi" << "mp4" << "mkv" << "mpg" << "mpeg" << "flv" << "wmv";
    }

    return extensions;
}

void MovieContent::preRenderUpdate(Factories& factories, ContentWindowPtr window, WallToWallChannel &wallToWallChannel)
{
    // Stop decoding when the window is moving.
    // This is to avoid saccades when reaching a new GLWindow.
    // The decoding resumes when the movement is finished.
    if( blockAdvance_ )
        return;

    boost::shared_ptr< Movie > movie = factories.getMovieFactory().getObject(getURI());

    movie->setPause( window->getControlState() & STATE_PAUSED );
    movie->setLoop( window->getControlState() & STATE_LOOP );

    const RenderContext* renderContext = movie->getRenderContext();
    movie->setVisible(renderContext->isRegionVisible(window->getCoordinates()));

    movie->preRenderUpdate( wallToWallChannel );
}

void MovieContent::postRenderUpdate(Factories& factories, ContentWindowPtr, WallToWallChannel& wallToWallChannel)
{
    // Stop decoding when the window is moving to avoid saccades when reaching a new GLWindow
    // The decoding resumes when the movement is finished
    if( blockAdvance_ )
        return;

    boost::shared_ptr< Movie > movie = factories.getMovieFactory().getObject(getURI());
    movie->postRenderUpdate( wallToWallChannel );
}
