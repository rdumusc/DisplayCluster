/*********************************************************************/
/* Copyright (c) 2014, EPFL/Blue Brain Project                       */
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

#define BOOST_TEST_MODULE PixelStreamWindowManagerTests
#include <boost/test/unit_test.hpp>
namespace ut = boost::unit_test;

#include <deflect/Frame.h>

#include "ContentWindow.h"
#include "DisplayGroup.h"
#include "Options.h"
#include "PixelStreamWindowManager.h"

#include "MinimalGlobalQtApp.h"
BOOST_GLOBAL_FIXTURE( MinimalGlobalQtApp )

namespace
{
const QString CONTENT_URI( "bla" );
const QSize wallSize( 1000, 1000 );
const QSize defaultPixelStreamWindowSize( 640, 480 );
const QSize testWindowSize( 500, 400 );
const QPointF testWindowPos( 400.0, 300.0 );
const QSize testFrameSize( 600, 500 );
const QSize testFrameSize2( 700, 600 );
}

deflect::FramePtr createTestFrame( const QSize& size )
{
    deflect::FramePtr frame( new deflect::Frame );
    frame->uri = CONTENT_URI;
    deflect::Segment segment;
    segment.parameters.width = size.width();
    segment.parameters.height = size.height();
    frame->segments.push_back( segment );
    return frame;
}

BOOST_AUTO_TEST_CASE( testNoStreamerWindowCreation )
{
    DisplayGroupPtr displayGroup( new DisplayGroup( wallSize ));
    PixelStreamWindowManager windowManager( *displayGroup );

    const QString uri = CONTENT_URI;
    const QPointF pos( testWindowPos );
    const QSize size( testWindowSize );

    windowManager.openPixelStreamWindow( uri, pos, size );
    ContentWindowPtr window = windowManager.getContentWindow( uri );
    BOOST_REQUIRE( window );

    BOOST_CHECK_EQUAL( window, windowManager.getContentWindow( uri ));

    const QRectF& coords = window->getCoordinates();
    BOOST_CHECK_EQUAL( coords.center(), pos );
    BOOST_CHECK_EQUAL( coords.size(), size );

    windowManager.closePixelStreamWindow( uri );
    BOOST_CHECK( !windowManager.getContentWindow( uri ));
}

BOOST_AUTO_TEST_CASE( testExplicitWindowCreation )
{
    DisplayGroupPtr displayGroup( new DisplayGroup( wallSize ));
    PixelStreamWindowManager windowManager( *displayGroup );

    const QString uri = CONTENT_URI;
    const QPointF pos( testWindowPos );
    const QSize size( testWindowSize );

    windowManager.openPixelStreamWindow( uri, pos, size );
    ContentWindowPtr window = windowManager.getContentWindow( uri );
    BOOST_REQUIRE( window );

    BOOST_CHECK_EQUAL( window, windowManager.getContentWindow( uri ));
    BOOST_CHECK_EQUAL( window,
                       displayGroup->getContentWindow( window->getID( )));

    windowManager.openPixelStreamWindow( uri );
    BOOST_CHECK_EQUAL( window, windowManager.getContentWindow( uri ));

    ContentPtr content = window->getContent();
    BOOST_REQUIRE( content );
    BOOST_CHECK( content->getURI() == uri );
    BOOST_CHECK_EQUAL( content->getType(), CONTENT_TYPE_PIXEL_STREAM );

    const QRectF& coords = window->getCoordinates();
    BOOST_CHECK_EQUAL( coords.center(), pos );
    BOOST_CHECK_EQUAL( coords.size(), size );

    // Check that the window is NOT resized to the first frame dimensions
    windowManager.updateStreamDimensions( createTestFrame( testFrameSize ));
    BOOST_CHECK_EQUAL( content->getDimensions(), testFrameSize );
    BOOST_CHECK_EQUAL( coords.center(), pos );
    BOOST_CHECK_EQUAL( coords.size(), size );

    windowManager.closePixelStreamWindow( uri );
    BOOST_CHECK( !windowManager.getContentWindow( uri ));
    BOOST_CHECK( !displayGroup->getContentWindow( window->getID( )));
}

BOOST_AUTO_TEST_CASE( testImplicitWindowCreation )
{
    DisplayGroupPtr displayGroup( new DisplayGroup( wallSize ));
    PixelStreamWindowManager windowManager( *displayGroup );

    const QString uri = CONTENT_URI;
    // window will be positioned centerred
    const QPointF pos( wallSize.width() * 0.5, wallSize.height() * 0.5 );
    const QSize size( defaultPixelStreamWindowSize );

    windowManager.openPixelStreamWindow( uri );
    ContentWindowPtr window = windowManager.getContentWindow( uri );
    BOOST_REQUIRE( window );
    BOOST_CHECK_EQUAL( window,
                       displayGroup->getContentWindow( window->getID( )));

    ContentPtr content = window->getContent();
    BOOST_REQUIRE( content );
    BOOST_CHECK( content->getURI() == uri );
    BOOST_CHECK_EQUAL( content->getType(), CONTENT_TYPE_PIXEL_STREAM );
    BOOST_CHECK_EQUAL( content->getDimensions(), UNDEFINED_SIZE );

    const QRectF& coords = window->getCoordinates();
    BOOST_CHECK_EQUAL( coords.center(), pos );
    BOOST_CHECK_EQUAL( coords.size(), size );

    // Check that the window is resized to the first frame dimensions
    windowManager.updateStreamDimensions( createTestFrame( testFrameSize ));
    BOOST_CHECK_EQUAL( content->getDimensions(), testFrameSize );
    BOOST_CHECK_EQUAL( coords.center(), pos );
    BOOST_CHECK_EQUAL( coords.size(), testFrameSize );

    // Check that the window is NOT resized to the next frame dimensions
    windowManager.updateStreamDimensions( createTestFrame( testFrameSize2 ));
    BOOST_CHECK_EQUAL( content->getDimensions(), testFrameSize2 );
    BOOST_CHECK_EQUAL( coords.center(), pos );
    BOOST_CHECK_EQUAL( coords.size(), testFrameSize );

    windowManager.closePixelStreamWindow( uri );
    BOOST_CHECK( !windowManager.getContentWindow( uri ));
    BOOST_CHECK( !displayGroup->getContentWindow( window->getID( )));
}
