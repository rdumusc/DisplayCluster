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

#ifndef PIXEL_STREAM_SEGMENT_RENDERER_H
#define PIXEL_STREAM_SEGMENT_RENDERER_H

#include "types.h"
#include "GLTexture2D.h"
#include "GLQuad.h"

#include <boost/noncopyable.hpp>

/**
 * Render a single PixelStream Segment
 *
 * This class is a texture renderer specialized for PixelStreamSegments
 */
class PixelStreamSegmentRenderer : public boost::noncopyable
{
public:
    /** Constructor. */
    PixelStreamSegmentRenderer();

    /** Get the position and dimensions of this segment */
    const QRect& getRect() const;

    /**
     * Update the texture.
     *
     * This call is blocking (texture upload to GPU).
     * @param image The new texture to upload, in (GL_)RGBA format.
     */
    void updateTexture(const QImage &image);

    /** Has the texture been marked as oudated with setTextureOutdated() */
    bool textureNeedsUpdate() const;

    /** Mark the texture as being outdated */
    void setTextureNeedsUpdate();

    /** Set the position and size paramters. (0,0) == top-left of the stream. */
    void setParameters( const deflect::PixelStreamSegmentParameters& param );

    /**
     * Render the current texture.
     *
     * Assume that the GL matrices have been set to the normalized dimensions of the stream.
     * @return true on successful render; false if no texture available.
     */
    bool render();

private:
    GLTexture2D texture_;
    GLQuad quad_;
    QRect rect_;

    bool textureNeedsUpdate_;

    void drawUnitTexturedQuad();
};

#endif
