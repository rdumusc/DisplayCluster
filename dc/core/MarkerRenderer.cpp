/*********************************************************************/
/* Copyright (c) 2014, EPFL/Blue Brain Project                       */
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

#include "MarkerRenderer.h"

#include "Marker.h"

#include "log.h"

#define MARKER_IMAGE_FILENAME ":/img/marker.png"

#define MARKER_SIZE_PIXELS 30

MarkerRenderer::MarkerRenderer()
    : markers_( new Markers )
{
}

void MarkerRenderer::render()
{
    const MarkersMap& map = markers_->getMarkers();
    for( MarkersMap::const_iterator it = map.begin(); it != map.end(); ++it )
        render( it->second );
}

void MarkerRenderer::setMarkers( MarkersPtr markers )
{
    markers_ = markers;
}

void MarkerRenderer::render( const Marker& marker )
{
    if ( !texture_.isValid() && !generateTexture( ))
        return;

    const QPointF pos = marker.getPosition();

    glPushAttrib( GL_ENABLE_BIT | GL_TEXTURE_BIT );

    glDisable( GL_DEPTH_TEST );
    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

    glPushMatrix();

    glTranslatef( pos.x(), pos.y(), 0.f );
    glScalef( MARKER_SIZE_PIXELS, MARKER_SIZE_PIXELS, 1.f );
    glTranslatef( -0.5f, -0.5f, 0.f ); // Center unit quad

    quad_.setTexture( texture_.getTextureId( ));
    quad_.render();

    glPopMatrix();

    glPopAttrib();
}

bool MarkerRenderer::generateTexture()
{
    const QImage image( MARKER_IMAGE_FILENAME );

    if( image.isNull( ))
    {
        put_flog( LOG_ERROR, "error loading marker texture '%s'",
                  MARKER_IMAGE_FILENAME );
        return false;
    }

    return texture_.init( image, GL_BGRA );
}
