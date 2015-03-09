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

#include "PixelStreamSegmentRenderer.h"

#include <deflect/PixelStreamSegmentParameters.h>

PixelStreamSegmentRenderer::PixelStreamSegmentRenderer()
    : textureNeedsUpdate_( true )
{
}

const QRect& PixelStreamSegmentRenderer::getRect() const
{
    return rect_;
}

void PixelStreamSegmentRenderer::updateTexture(const QImage& image)
{
    texture_.update(image, GL_RGBA);
    textureNeedsUpdate_ = false;
}

bool PixelStreamSegmentRenderer::textureNeedsUpdate() const
{
    return textureNeedsUpdate_;
}

void PixelStreamSegmentRenderer::setTextureNeedsUpdate()
{
    textureNeedsUpdate_ = true;
}

void PixelStreamSegmentRenderer::setParameters( const deflect::PixelStreamSegmentParameters& param )
{
    rect_.setX( param.x );
    rect_.setY( param.y );
    rect_.setWidth( param.width );
    rect_.setHeight( param.height );
}

bool PixelStreamSegmentRenderer::render()
{
    if(!texture_.isValid())
        return false;

    // OpenGL transformation
    glPushMatrix();

    glTranslatef(rect_.x(), rect_.y(), 0.);
    // The following draw calls assume normalized coordinates, so we must
    // pre-multiply by this segment's dimensions
    glScalef(rect_.width(), rect_.height(), 0.);

    drawUnitTexturedQuad();

    glPopMatrix();

    return true;
}

void PixelStreamSegmentRenderer::drawUnitTexturedQuad()
{
    glPushAttrib(GL_ENABLE_BIT | GL_TEXTURE_BIT);

    texture_.bind();
    quad_.setEnableTexture(true);
    quad_.setRenderMode(GL_QUADS);
    quad_.render();

    glPopAttrib();
}
