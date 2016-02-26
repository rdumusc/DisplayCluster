/*********************************************************************/
/* Copyright (c) 2016, EPFL/Blue Brain Project                       */
/*                     Daniel.Nachbaur@epfl.ch                       */
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

#ifndef TEXTUREUPLOADER_H
#define TEXTUREUPLOADER_H

#include <QObject>

#include "types.h"

class QOffscreenSurface;
class QOpenGLContext;
class QOpenGLFunctions_2_1;

/**
 * A class responsible for uploading pixel data from CPU memory to GPU memory
 * using a PixelBufferObject. An object of this class needs to be moved to a
 * separate thread for optimal performance and for the init() and stop()
 * sequence to work.
 */
class TextureUploader : public QObject
{
    Q_OBJECT
public:
    /** Construction does not need any OpenGL context. */
    TextureUploader();

public slots:
    /** Performs the upload of pixels into the given tile's back texture. */
    void uploadTexture( ImagePtr image, TileWeakPtr tile );

signals:
    /**
     * Does the necessary OpenGL setup, needs to be called from the dedicated
     * upload thread.
     */
    void init( QOpenGLContext* shareContext );

    /**
     * Does the necessary OpenGL teardown, needs to be called from the dedicated
     * upload thread.
     */
    void stop();

private:
    void _onInit( QOpenGLContext* shareContext );
    void _onStop();

    QOpenGLContext* _glContext;
    QOffscreenSurface* _offscreenSurface;
    QOpenGLFunctions_2_1* _gl;

    uint _pbo;
    size_t _bufferSize;
};

#endif // TEXTUREUPLOADER_H