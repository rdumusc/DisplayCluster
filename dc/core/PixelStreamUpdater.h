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

#ifndef PIXELSTREAMUPDATER_H
#define PIXELSTREAMUPDATER_H

#include "types.h"

#include "SwapSyncObject.h"

#include <QtCore/QObject>
#include <QtCore/QMap>

/**
 * Synchronize the update of PixelStreams and send new frame requests.
 */
class PixelStreamUpdater : public QObject
{
    Q_OBJECT

public:
    /** Constructor. */
    PixelStreamUpdater();

    /** Synchronize the update of the PixelStreams. */
    void synchronizeFramesSwap( const SyncFunction& versionCheckFunc );

public slots:
    /** Update the appropriate PixelStream with the given frame. */
    void updatePixelStream( deflect::FramePtr frame );

    /** Connect the new window to receive PixelStream frame updates. */
    void onWindowAdded( QmlWindowPtr qmlWindow );

    /** Disconnect the window from PixelStream frame updates. */
    void onWindowRemoved( QmlWindowPtr qmlWindow );

signals:
    /** Emitted to request a new frame after a successful swap. */
    void requestFrame( QString uri );

private:
    Q_DISABLE_COPY( PixelStreamUpdater )

    typedef QMap<QString,PixelStreamPtr> PixelStreamMap;
    PixelStreamMap _pixelStreamMap;

    typedef SwapSyncObject<deflect::FramePtr> SwapSyncFrame;
    typedef QMap<QString,SwapSyncFrame> SwapSyncFramesMap;
    SwapSyncFramesMap _swapSyncFrames;
};

#endif // PIXELSTREAMUPDATER_H
