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
#include <vector>
#include <boost/shared_ptr.hpp>
#include <QtCore/QRectF>

class Configuration;
class Content;
class ContentWindow;
class DisplayGroup;
class DisplayGroupAdapter;
class DisplayGroupRenderer;
class DynamicTexture;
class Factories;
class FactoryObject;
class GLWindow;
class MarkerRenderer;
class Markers;
class MasterConfiguration;
class MPIChannel;
class Options;
class PDF;
class PixelStreamWindowManager;
class Renderable;
class RenderContext;
class SVG;
class TestPattern;
class WallWindow;
class WallConfiguration;

typedef boost::shared_ptr< Content > ContentPtr;
typedef boost::shared_ptr< ContentWindow > ContentWindowPtr;
typedef boost::shared_ptr< DisplayGroupAdapter > DisplayGroupAdapterPtr;
typedef boost::shared_ptr< DisplayGroup > DisplayGroupPtr;
typedef boost::shared_ptr< DisplayGroupRenderer > DisplayGroupRendererPtr;
typedef boost::shared_ptr< DynamicTexture > DynamicTexturePtr;
typedef boost::shared_ptr< Factories > FactoriesPtr;
typedef boost::shared_ptr< FactoryObject > FactoryObjectPtr;
typedef boost::shared_ptr< WallWindow > WallWindowPtr;
typedef boost::shared_ptr< MarkerRenderer > MarkerRendererPtr;
typedef boost::shared_ptr< Markers > MarkersPtr;
typedef boost::shared_ptr< MPIChannel > MPIChannelPtr;
typedef boost::shared_ptr< Options > OptionsPtr;
typedef boost::shared_ptr< PDF > PDFPtr;
typedef boost::shared_ptr< Renderable > RenderablePtr;
typedef boost::shared_ptr< RenderContext > RenderContextPtr;
typedef boost::shared_ptr< SVG > SVGPtr;
typedef boost::shared_ptr< TestPattern > TestPatternPtr;

typedef std::vector< ContentWindowPtr > ContentWindowPtrs;
typedef std::vector< WallWindowPtr > WallWindowPtrs;

static const QRectF UNIT_RECTF( 0.0, 0.0, 1.0, 1.0 );

#endif
