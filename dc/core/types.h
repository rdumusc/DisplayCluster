/*********************************************************************/
/* Copyright (c) 2013, EPFL/Blue Brain Project                       */
/*                     Daniel Nachbaur <daniel.nachbaur@epfl.ch>     */
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

#ifndef TYPES_H
#define TYPES_H

#include <deflect/types.h>
#include <boost/shared_ptr.hpp>
#include <QtCore/QRectF>
#include <future>
#include <iostream>
#include <set>
#include <vector>

class Configuration;
class Content;
class ContentWindow;
class ContentWindowController;
class DisplayGroup;
class DisplayGroupAdapter;
class DisplayGroupRenderer;
class DynamicTexture;
class FFMPEGFrame;
class FFMPEGPicture;
class FFMPEGVideoStream;
class FFMPEGVideoFrameConverter;
class GLWindow;
class MarkerRenderer;
class Markers;
class MasterConfiguration;
class MPIChannel;
class Options;
class PDF;
class PixelStream;
class PixelStreamWindowManager;
class QmlWindowRenderer;
class Renderable;
class RenderContext;
class SVG;
class TestPattern;
class WallContent;
class WallWindow;
class WallConfiguration;
class WallToWallChannel;

typedef boost::shared_ptr< Content > ContentPtr;
typedef boost::shared_ptr< ContentWindow > ContentWindowPtr;
typedef std::unique_ptr<ContentWindowController> ContentWindowControllerPtr;
typedef boost::shared_ptr< DisplayGroupAdapter > DisplayGroupAdapterPtr;
typedef boost::shared_ptr< DisplayGroup > DisplayGroupPtr;
typedef boost::shared_ptr< DisplayGroupRenderer > DisplayGroupRendererPtr;
typedef boost::shared_ptr< DynamicTexture > DynamicTexturePtr;
typedef std::shared_ptr<FFMPEGPicture> PicturePtr;
typedef boost::shared_ptr< MarkerRenderer > MarkerRendererPtr;
typedef boost::shared_ptr< Markers > MarkersPtr;
typedef boost::shared_ptr< MPIChannel > MPIChannelPtr;
typedef boost::shared_ptr< Options > OptionsPtr;
typedef boost::shared_ptr< PixelStream > PixelStreamPtr;
typedef boost::shared_ptr< PDF > PDFPtr;
typedef boost::shared_ptr< QmlWindowRenderer > QmlWindowPtr;
typedef boost::shared_ptr< Renderable > RenderablePtr;
typedef boost::shared_ptr< RenderContext > RenderContextPtr;
typedef boost::shared_ptr< SVG > SVGPtr;
typedef boost::shared_ptr< TestPattern > TestPatternPtr;
typedef boost::shared_ptr< WallContent > WallContentPtr;
typedef boost::shared_ptr< WallWindow > WallWindowPtr;

typedef std::set< ContentWindowPtr > ContentWindowSet;
typedef std::vector< ContentWindowPtr > ContentWindowPtrs;
typedef std::vector< WallWindowPtr > WallWindowPtrs;

static const QRectF UNIT_RECTF( 0.0, 0.0, 1.0, 1.0 );
static const QSize UNDEFINED_SIZE( -1, -1 );

inline bool operator < ( const QSizeF& a, const QSizeF& b )
{
    return (a.width() < b.width() || a.height() < b.height());
}

inline bool operator > ( const QSizeF& a, const QSizeF& b )
{
    return (a.width() > b.width() || a.height() > b.height());
}

inline std::ostream& operator << ( std::ostream& str, const QSizeF& s )
{
    str << s.width() << 'x' << s.height();
    return str;
}

inline std::ostream& operator << ( std::ostream& str, const QPointF& p )
{
    str << p.x() << ',' << p.y();
    return str;
}

inline std::ostream& operator << ( std::ostream& str, const QRectF& r )
{
    str << r.x() << ',' << r.y() << ' ' << r.width() << 'x' << r.height();
    return str;
}

// missing make_unique() implementation in C++11 standard
// source: http://herbsutter.com/gotw/_102/
template<typename T, typename ...Args>
std::unique_ptr<T> make_unique( Args&& ...args )
{
    return std::unique_ptr<T>( new T( std::forward<Args>(args)... ) );
}

template<typename R>
bool is_ready( std::future<R> const& f )
{
    return f.wait_for( std::chrono::seconds( 0 )) == std::future_status::ready;
}

#endif
