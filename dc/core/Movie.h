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

#ifndef MOVIE_H
#define MOVIE_H

#include "WallContent.h"

#include "GLTexture2D.h"
#include "GLQuad.h"
#include "ElapsedTimer.h"

#include <future>

class FFMPEGMovie;

class Movie : public WallContent
{
public:
    Movie( const QString& uri );
    ~Movie();

    void setVisible( bool isVisible );

    void setPause( bool pause );
    void setLoop( bool loop );

private:
    std::unique_ptr<FFMPEGMovie> _ffmpegMovie;

    GLTexture2D _texture;
    GLQuad _quad;
    GLQuad _previewQuad;

    bool _paused;
    bool _loop;
    bool _isVisible;

    ElapsedTimer _timer;
    double _sharedTimestamp;
    std::future<PicturePtr> _futurePicture;

    void render() override;
    void renderPreview() override;
    void preRenderUpdate( ContentWindowPtr window,
                          const QRect& wallArea ) override;
    void preRenderSync( WallToWallChannel& wallToWallChannel ) override;

    bool _generateTexture();

    double _getDelay() const;
    void _updateTimestamp( WallToWallChannel& wallToWallChannel );
    void _synchronizeTimestamp( WallToWallChannel& wallToWallChannel );
    void _rewind();
};

#endif
