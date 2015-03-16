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

#include "Movie.h"

#include "FFMPEGMovie.h"
#include "ContentWindow.h"
#include "MovieContent.h"
#include "WallToWallChannel.h"

Movie::Movie( const QString& uri )
    : ffmpegMovie_( new FFMPEGMovie( uri ))
    , uri_( uri )
    , paused_( false )
    , suspended_( false )
    , isVisible_( true )
    , skippedLastFrame_( false )
{}

Movie::~Movie()
{
    delete ffmpegMovie_;
}

void Movie::setVisible( const bool isVisible )
{
    isVisible_ = isVisible;
}

void Movie::setPause( const bool pause )
{
    paused_ = pause;
}

void Movie::setLoop( const bool loop )
{
    ffmpegMovie_->setLoop( loop );
}

void Movie::render()
{
    if( !texture_.isValid( ))
        return;

    quad_.render();
}

void Movie::renderPreview()
{
    if( !texture_.isValid( ))
        return;

    previewQuad_.render();
}

void Movie::preRenderUpdate( ContentWindowPtr window, const QRect& wallArea )
{
    if( !texture_.isValid( ))
    {
        generateTexture();
        quad_.setTexture( texture_.getTextureId( ));
        previewQuad_.setTexture( texture_.getTextureId( ));
    }

    quad_.setTexCoords( window->getZoomRect( ));

    // Stop decoding when the window is moving.
    // This is to avoid saccades when reaching a new GLWindow.
    // The decoding resumes when the movement is finished.
    suspended_ = window->isMoving() || window->isResizing();

    MovieContent& movie = static_cast<MovieContent&>( *window->getContent( ));

    setPause( movie.getControlState() & STATE_PAUSED );
    setLoop( movie.getControlState() & STATE_LOOP );

    setVisible( QRectF( wallArea ).intersects( window->getCoordinates( )));
}

void Movie::preRenderSync( WallToWallChannel& wallToWallChannel )
{
    if( paused_ || suspended_ )
        return;

    elapsedTimer_.setCurrentTime( wallToWallChannel.getTime( ));
    timestamp_ += elapsedTimer_.getElapsedTime();

    skippedLastFrame_ = !isVisible_;
    ffmpegMovie_->update( timestamp_, skippedLastFrame_ );

    // Get the current timestamp back from the FFMPEG movie.
    timestamp_ = ffmpegMovie_->getTimestamp();

    if( ffmpegMovie_->isNewFrameAvailable( ))
        texture_.update( ffmpegMovie_->getData( ), GL_RGBA );
}

void Movie::postRenderSync( WallToWallChannel& wallToWallChannel )
{
    if( suspended_ )
        return;

    synchronizeTimestamp( wallToWallChannel );
}

bool Movie::generateTexture()
{
    QImage image( ffmpegMovie_->getWidth(), ffmpegMovie_->getHeight(),
                  QImage::Format_RGB32 );
    image.fill( 0 );

    return texture_.init( image );
}

void Movie::synchronizeTimestamp( WallToWallChannel& wallToWallChannel )
{
    // Elect a leader among processes which have decoded a frame
    const int leader = wallToWallChannel.electLeader( !skippedLastFrame_ );

    if( leader < 0 )
        return;

    if( leader == wallToWallChannel.getRank( ))
        wallToWallChannel.broadcast( timestamp_ );
    else
        timestamp_ = wallToWallChannel.receiveTimestampBroadcast( leader );
}

