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

#ifndef PIXELSTREAMPROVIDER_H
#define PIXELSTREAMPROVIDER_H

#include "types.h"

#include <QQuickImageProvider>

/**
 * Provides PixelStream frames to QML.
 */
class PixelStreamProvider : public QObject, public QQuickImageProvider
{
    Q_OBJECT
    Q_DISABLE_COPY( PixelStreamProvider )

public:
    /** Constructor */
    PixelStreamProvider();

    static const QString ID;

    /** @copydoc QQuickImageProvider::requestImage */
    QImage requestImage( const QString& id, QSize* size,
                         const QSize& requestedSize ) final;

    /** Open a stream, shared with the other windows for this process. */
    PixelStreamUpdaterSharedPtr open( const QString& stream );

    /** Close a stream by removing it from the internal list. */
    void close( const QString& stream );

    /** Update the streams, using the channel to synchronize processes. */
    void update( WallToWallChannel& channel );

public slots:
    /** Add a new frame. */
    void setNewFrame( deflect::FramePtr frame );

signals:
    /** Emitted to request a new frame after a successful swap. */
    void requestFrame( QString uri );

private:
    typedef std::map<QString, PixelStreamUpdaterSharedPtr> PixelStreamMap;
    PixelStreamMap _streams;
};

#endif
