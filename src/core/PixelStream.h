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

#ifndef PIXEL_STREAM_H
#define PIXEL_STREAM_H

#include "FactoryObject.h"
#include "FpsCounter.h"
#include "SwapSyncObject.h"
#include "types.h"

#include <deflect/PixelStreamSegment.h>

#include <QObject>
#include <QRectF>
#include <QString>
#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <vector>

class PixelStreamSegmentRenderer;
class WallToWallChannel;
typedef boost::shared_ptr<deflect::PixelStreamSegmentDecoder> PixelStreamSegmentDecoderPtr;
typedef boost::shared_ptr<PixelStreamSegmentRenderer> PixelStreamSegmentRendererPtr;

/**
 * Qml Wrapper for a PixelStream segment parameters.
 */
class Segment : public QObject
{
    Q_OBJECT
    Q_PROPERTY( QRect coord READ getCoord NOTIFY coordChanged )

public:
    // false-positive on qt signals for Q_PROPERY notifiers
    // cppcheck-suppress uninitMemberVar
    Segment( const deflect::PixelStreamSegmentParameters& params )
        : rect_( params.x, params.y, params.width, params.height )
    {}

    const QRect& getCoord() const
    {
        return rect_;
    }

    void update( const deflect::PixelStreamSegmentParameters& params )
    {
        if( rect_.x() == (int)params.x &&
            rect_.y() == (int)params.y &&
            rect_.width() == (int)params.width &&
            rect_.height() == (int)params.height
            )
            return;

        rect_.setRect( params.x, params.y, params.width, params.height );
        emit coordChanged();
    }

signals:
    void coordChanged();

private:
    QRect rect_;
};

/**
 * Decompress, upload and render the segments of a PixelStream.
 */
class PixelStream : public QObject, public FactoryObject
{
    Q_OBJECT
    Q_PROPERTY( QString statistics READ getStatistics NOTIFY statisticsChanged )
    Q_PROPERTY( QList<QObject*> segments READ getSegments NOTIFY segmentsChanged )

public:
    PixelStream(const QString& uri);
    ~PixelStream();

    void preRenderUpdate(const QRectF& windowRect, WallToWallChannel& wallToWallChannel);
    void render(const QRectF& texCoords) override;

    void setNewFrame(const deflect::PixelStreamFramePtr frame);

    QString getStatistics() const;
    QList<QObject*> getSegments() const;

signals:
    void requestFrame(const QString uri);

    void statisticsChanged();
    void segmentsChanged();

private:
    void sync(WallToWallChannel& wallToWallChannel);

    SwapSyncObject<deflect::PixelStreamFramePtr> syncPixelStreamFrame_;

    // pixel stream identifier
    QString uri_;

    // dimensions of entire pixel stream
    unsigned int width_;
    unsigned int height_;

    // The front buffer is decoded by the frameDecoders and then used to upload the frameRenderers
    deflect::PixelStreamSegments frontBuffer_;
    // The back buffer contains the next frame to process (last frame received)
    deflect::PixelStreamSegments backBuffer_;
    bool buffersSwapped_;

    // The list of decoded images for the next frame
    std::vector<PixelStreamSegmentDecoderPtr> frameDecoders_;

    // For each segment, object for image decoding, rendering and storing parameters
    std::vector<PixelStreamSegmentRendererPtr> segmentRenderers_;

    // The coordinates of the ContentWindow of this PixelStream
    QRectF contentWindowRect_;

    FpsCounter fpsCounter_;

    QList<QObject*> segmentsList_;

    void updateRenderers(const deflect::PixelStreamSegments& segments);
    void updateVisibleTextures(const QRectF& windowRect);
    void swapBuffers();
    void recomputeDimensions(const deflect::PixelStreamSegments& segments);
    void decodeVisibleTextures(const QRectF& windowRect);

    void adjustFrameDecodersCount(const size_t count);
    void adjustSegmentRendererCount(const size_t count);
    void refreshSegmentsList(const deflect::PixelStreamSegments& segments);

    bool isDecodingInProgress(WallToWallChannel& wallToWallChannel);

    QRectF getSceneCoordinates(const QRect& segment, const QRectF& windowRect) const;
    bool isVisible(const QRect& segment, const QRectF& windowRect);
    bool isVisible(const deflect::PixelStreamSegment& segment,
                   const QRectF& windowRect);
};


#endif
