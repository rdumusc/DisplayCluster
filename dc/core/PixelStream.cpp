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

#include "PixelStream.h"

#include "ContentWindow.h"
#include "WallToWallChannel.h"
#include "log.h"
#include "PixelStreamSegmentRenderer.h"
#include "FpsCounter.h"

#include <deflect/Frame.h>
#include <deflect/SegmentDecoder.h>
#include <deflect/SegmentParameters.h>

#include <boost/bind.hpp>
#include <boost/foreach.hpp>
#include <boost/make_shared.hpp>

// false-positive on qt signals for Q_PROPERTY notifiers
// cppcheck-suppress uninitMemberVar
PixelStream::PixelStream( const QString& uri )
    : uri_( uri )
    , width_( 0 )
    , height_ ( 0 )
    , buffersSwapped_( false )
{
}

PixelStream::~PixelStream()
{
    qDeleteAll( segmentsList_ );
}

void PixelStream::setNewFrame( deflect::FramePtr frame )
{
    backBuffer_ = frame->segments;
}

QString PixelStream::getStatistics() const
{
    return fpsCounter_.toString();
}

QList<QObject*> PixelStream::getSegments() const
{
    return segmentsList_;
}

void PixelStream::render()
{
    glPushMatrix();
    glScalef( 1.f/(float)width_, 1.f/(float)height_, 0.f );

    BOOST_FOREACH( PixelStreamSegmentRendererPtr renderer, segmentRenderers_ )
    {
        if( isVisible( renderer->getRect( )))
            renderer->render();
    }

    glPopMatrix();
}

void PixelStream::renderPreview()
{
    // PixelStreams don't support zooming and don't have previews
}

void PixelStream::preRenderUpdate( ContentWindowPtr, const QRect& wallArea )
{
    sceneRect_ = _qmlItem->getSceneRect();
    wallArea_ = wallArea;
}

void PixelStream::preRenderSync( WallToWallChannel& wallToWallChannel )
{
    if( isDecodingInProgress( wallToWallChannel ))
        return;

    // After swapping the buffers, wait until decoding has finished to update
    // the renderers.
    if( buffersSwapped_ )
    {
        adjustSegmentRendererCount( frontBuffer_.size( ));
        updateRenderers( frontBuffer_ );
        recomputeDimensions( frontBuffer_ );
        refreshSegmentsList( frontBuffer_ );
        buffersSwapped_ = false;
    }

    // The window may have moved, so always check if some segments have become
    // visible to upload them.
    updateVisibleTextures();

    if( !backBuffer_.empty( ))
    {
        swapBuffers();
        adjustFrameDecodersCount( frontBuffer_.size( ));
    }

    // The window may have moved, so always check if some segments have become
    // visible to decode them.
    decodeVisibleTextures();
}

bool PixelStream::isDecodingInProgress( WallToWallChannel& wallToWallChannel )
{
    // determine if threads are running on any processes for this PixelStream
    int localThreadsRunning = 0;

    std::vector<PixelStreamSegmentDecoderPtr>::const_iterator it;
    for( it = frameDecoders_.begin(); it != frameDecoders_.end(); ++it )
    {
        if( (*it)->isRunning( ))
            ++localThreadsRunning;
    }

    return wallToWallChannel.globalSum( localThreadsRunning ) > 0;
}

void PixelStream::updateRenderers( const deflect::Segments& segments )
{
    assert( segmentRenderers_.size() == segments.size( ));

    for( size_t i=0; i<segments.size(); i++ )
    {
        // The parameters always need to be up to date to determine visibility
        // when rendering.
        segmentRenderers_[i]->setParameters( segments[i].parameters );
        segmentRenderers_[i]->setTextureNeedsUpdate();
    }
}

void PixelStream::updateVisibleTextures()
{
    bool textureWasUpdated = false;
    for( size_t i=0; i<frontBuffer_.size(); ++i )
    {
        if( segmentRenderers_[i]->textureNeedsUpdate() &&
            !frontBuffer_[i].parameters.compressed &&
            isVisible( frontBuffer_[i] ))
        {
            const char* data = frontBuffer_[i].imageData.constData();
            const QImage textureWrapper( (const uchar*)data,
                                         frontBuffer_[i].parameters.width,
                                         frontBuffer_[i].parameters.height,
                                         QImage::Format_RGB32 );

            segmentRenderers_[i]->updateTexture( textureWrapper );

            textureWasUpdated = true;
        }
    }

    if( textureWasUpdated )
    {
        fpsCounter_.tick();
        emit statisticsChanged();
    }
}

void PixelStream::swapBuffers()
{
    assert( !backBuffer_.empty( ));

    frontBuffer_ = backBuffer_;
    backBuffer_.clear();

    buffersSwapped_ = true;
}

void PixelStream::recomputeDimensions( const deflect::Segments &segments )
{
    width_ = 0;
    height_ = 0;

    for( size_t i=0; i<segments.size(); ++i )
    {
        const deflect::SegmentParameters& params = segments[i].parameters;
        width_ = std::max( width_, params.width+params.x );
        height_ = std::max( height_, params.height+params.y );
    }
}

void PixelStream::decodeVisibleTextures()
{
    assert( frameDecoders_.size() == frontBuffer_.size( ));

    std::vector<PixelStreamSegmentDecoderPtr>::iterator frameDecoder_it = frameDecoders_.begin();
    deflect::Segments::iterator segment_it = frontBuffer_.begin();
    for( ; segment_it != frontBuffer_.end(); ++segment_it, ++frameDecoder_it )
    {
        if( segment_it->parameters.compressed && isVisible( *segment_it ))
            (*frameDecoder_it)->startDecoding( *segment_it );
    }
}

void PixelStream::adjustFrameDecodersCount( const size_t count )
{
    // We need to insert NEW objects in the vector if it is smaller
    for( size_t i=frameDecoders_.size(); i<count; ++i )
        frameDecoders_.push_back( boost::make_shared<deflect::SegmentDecoder>( ));
    // Or resize it if it is bigger
    frameDecoders_.resize( count );
}

void PixelStream::adjustSegmentRendererCount( const size_t count )
{
    // Recreate the renderers if the number of segments has changed
    if( segmentRenderers_.size() == count )
        return;

    segmentRenderers_.clear();
    for( size_t i=0; i<count; ++i )
        segmentRenderers_.push_back( boost::make_shared<PixelStreamSegmentRenderer>( ));
}

void PixelStream::refreshSegmentsList( const deflect::Segments& segments )
{
    // Update existing segments
    const size_t maxIndex = std::min( (size_t)segmentsList_.size(),
                                      segments.size( ));
    for( size_t i = 0; i < maxIndex; ++i )
    {
        Segment* segment = qobject_cast<Segment*>( segmentsList_[i] );
        segment->update( segments[i].parameters );
    }

    const bool sizeChange = segments.size() != (size_t)segmentsList_.size();

    // Insert new objects in the vector if it is smaller
    for( size_t i = segmentsList_.size(); i < segments.size(); ++i )
        segmentsList_.push_back( new Segment( segments[i].parameters ));

    // Or remove objects it if it is bigger
    const size_t removeCount = segmentsList_.size() - segments.size();
    for( size_t i = 0; i < removeCount; ++i )
    {
        delete segmentsList_.back();
        segmentsList_.pop_back();
    }

    if( sizeChange )
        emit segmentsChanged();
}

QRectF PixelStream::getSceneCoordinates( const QRect& segment ) const
{
    const qreal normX = (qreal)segment.x() / (qreal)width_;
    const qreal normY = (qreal)segment.y() / (qreal)height_;
    const qreal normWidth = (qreal)segment.width() / (qreal)width_;
    const qreal normHeight = (qreal)segment.height() / (qreal)height_;

    // coordinates of segment in global tiled display space
    return QRectF( sceneRect_.x() + normX * sceneRect_.width(),
                   sceneRect_.y() + normY * sceneRect_.height(),
                   normWidth * sceneRect_.width(),
                   normHeight * sceneRect_.height( ));
}

bool PixelStream::isVisible( const QRect& segment ) const
{
    return wallArea_.intersects( getSceneCoordinates( segment ));
}

bool PixelStream::isVisible( const deflect::Segment& segment ) const
{
    const deflect::SegmentParameters& param = segment.parameters;
    return isVisible( QRect( param.x, param.y, param.width, param.height ));
}

