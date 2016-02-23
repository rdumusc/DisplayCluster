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

#ifndef MOVIEUPDATER_H
#define MOVIEUPDATER_H

#include "ElapsedTimer.h"
#include "SwapSyncObject.h"
#include "types.h"

#include <QObject>

/**
 * Updates Movies synchronously accross different processes.
 *
 * A single movie is designed to provide images to multiple windows on each
 * process.
 */
class MovieUpdater : public QObject
{
    Q_OBJECT

public:
    explicit MovieUpdater( const QString& uri );
    ~MovieUpdater();

    bool isVisible() const;
    void setVisible( bool visible );

    bool isPaused() const;

    void update( const MovieContent& movie );
    void sync( WallToWallChannel& channel );

    ImagePtr getImage() const;

//    TextureFactory* createTextureFactory();

signals:
    void textureUploaded();
    void uploadTexture( ImagePtr image, uint textureID );

public slots:
//    void onTextureUploaded( ImagePtr image, uint textureID );

private:
    MoviePtr _ffmpegMovie;

    bool _paused;
    bool _loop;
    bool _visible;

    ElapsedTimer _timer;
    double _sharedTimestamp;
    std::future<PicturePtr> _futurePicture;
//    std::deque< uint > _textures;

    typedef SwapSyncObject< int64_t > SyncSwapImage;
    SyncSwapImage _syncSwapImage;

    PicturePtr _image;

//    uint _popTextureID();
    double _getDelay() const;
    void _updateTimestamp( WallToWallChannel& channel );
    void _synchronizeTimestamp( WallToWallChannel& channel );
    void _rewind();
};

#endif // MOVIEUPDATER_H
