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

#include "ContentSynchronizer.h"

#include "Content.h"

#include "BasicSynchronizer.h"
#include "DynamicTextureSynchronizer.h"
#include "MovieSynchronizer.h"
#include "PixelStreamSynchronizer.h"
#include "VectorialSynchronizer.h"

#include "MovieProvider.h"
#include "PixelStreamProvider.h"

ContentSynchronizer::~ContentSynchronizer() {}

ContentSynchronizerPtr
ContentSynchronizer::create( ContentPtr content,
                             QQmlImageProviderBase& provider )
{
    switch( content->getType( ))
    {
    case CONTENT_TYPE_DYNAMIC_TEXTURE:
        return make_unique<DynamicTextureSynchronizer>( content->getURI( ));
    case CONTENT_TYPE_MOVIE:
        return make_unique<MovieSynchronizer>(
                   content->getURI(), dynamic_cast<MovieProvider&>( provider ));
    case CONTENT_TYPE_PIXEL_STREAM:
        return make_unique<PixelStreamSynchronizer>(
             content->getURI(), dynamic_cast<PixelStreamProvider&>( provider ));
    case CONTENT_TYPE_PDF:
    case CONTENT_TYPE_SVG:
        return make_unique<VectorialSynchronizer>();
    default:
        return make_unique<BasicSynchronizer>();
    }
}
