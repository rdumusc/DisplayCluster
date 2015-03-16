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

#include "PixelStreamUpdater.h"

#include "log.h"

#include "QmlWindowRenderer.h"
#include "ContentWindow.h"
#include "PixelStream.h"

#include <deflect/PixelStreamFrame.h>

PixelStreamUpdater::PixelStreamUpdater()
{
}

void PixelStreamUpdater::synchronizeFramesSwap( const SyncFunction&
                                                versionCheckFunc )
{
    PixelStreamMap::const_iterator streamIt = pixelStreamMap_.begin();
    for( ; streamIt != pixelStreamMap_.end(); ++streamIt )
    {
        const QString& uri = streamIt.key();

        SwapSyncFrame& swapSyncFrame = swapSyncFrames_[uri];
        if( swapSyncFrame.sync( versionCheckFunc ))
        {
            streamIt.value()->setNewFrame( swapSyncFrame.get( ));
            emit requestFrame( uri );
        }
    }
}

void PixelStreamUpdater::updatePixelStream( deflect::PixelStreamFramePtr frame )
{
    swapSyncFrames_[frame->uri].update( frame );
}

void PixelStreamUpdater::onWindowAdded( QmlWindowPtr qmlWindow )
{
    ContentWindowPtr window = qmlWindow->getContentWindow();
    if( window->getContent()->getType() != CONTENT_TYPE_PIXEL_STREAM )
        return;

    WallContentPtr stream = qmlWindow->getWallContent();

    const QString& uri = window->getContent()->getURI();
    pixelStreamMap_[uri] = boost::static_pointer_cast<PixelStream>( stream );
}

void PixelStreamUpdater::onWindowRemoved( QmlWindowPtr qmlWindow )
{
    ContentWindowPtr window = qmlWindow->getContentWindow();
    if( window->getContent()->getType() != CONTENT_TYPE_PIXEL_STREAM )
        return;

    const QString& uri = window->getContent()->getURI();
    disconnect( pixelStreamMap_[uri].get( ));
    pixelStreamMap_.remove( uri );
    swapSyncFrames_.remove( uri );
}
