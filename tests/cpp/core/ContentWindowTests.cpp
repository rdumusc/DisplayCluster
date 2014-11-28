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

#define BOOST_TEST_MODULE ContentWindowTests
#include <boost/test/unit_test.hpp>
namespace ut = boost::unit_test;

#include "ContentWindow.h"
#include "DisplayGroup.h"

#include "MinimalGlobalQtApp.h"
BOOST_GLOBAL_FIXTURE( MinimalGlobalQtApp )

#include "DummyContent.h"

namespace
{
const QSize wallSize( 1000, 1000 );
const int WIDTH = 512;
const int HEIGHT = 512;
}

BOOST_AUTO_TEST_CASE( testInitialSize )
{
    ContentPtr content( new DummyContent );
    content->setDimensions( QSize( WIDTH, HEIGHT ));
    ContentWindow window( content );

    const QRectF& coords = window.getCoordinates();

    // default 1:1 size, left-corner at the origin
//    BOOST_CHECK_EQUAL( window.getSizeState(), SIZE_1TO1 );
    BOOST_CHECK_EQUAL( coords.x(), 0.0 );
    BOOST_CHECK_EQUAL( coords.y(), 0.0 );
    BOOST_CHECK_EQUAL( coords.width(), WIDTH );
    BOOST_CHECK_EQUAL( coords.height(), HEIGHT );
}

BOOST_AUTO_TEST_CASE( testOneToOneSize )
{
    ContentPtr content( new DummyContent );
    content->setDimensions( QSize( WIDTH, HEIGHT ));
    ContentWindow window( content );

    DisplayGroupPtr displayGroup( new DisplayGroup( wallSize ));
//    window.adjustSize( SIZE_1TO1 );

    const QRectF& coords = window.getCoordinates();

    const float posX = ( wallSize.width() - float( WIDTH )) * 0.5;
    const float posY = ( wallSize.height() - float( HEIGHT )) * 0.5;

    // 1:1 size, centered on wall
//    BOOST_CHECK_EQUAL( window.getSizeState(), SIZE_1TO1 );
    BOOST_CHECK_EQUAL( coords.x(), posX );
    BOOST_CHECK_EQUAL( coords.y(), posY );
    BOOST_CHECK_EQUAL( coords.width(), WIDTH );
    BOOST_CHECK_EQUAL( coords.height(), HEIGHT );
}

BOOST_AUTO_TEST_CASE( testToggleSizeWithoutDisplayGroupDoesNothing )
{
    ContentPtr content( new DummyContent );
    content->setDimensions( QSize( WIDTH, HEIGHT ));
    ContentWindow window( content );

//    window.toggleFullscreen();

    const QRectF& coords = window.getCoordinates();

    // full screen, center on wall
//    BOOST_CHECK_EQUAL( window.getSizeState(), SIZE_1TO1 );
    BOOST_CHECK_EQUAL( coords.x(), 0.0 );
    BOOST_CHECK_EQUAL( coords.y(), 0.0 );
    BOOST_CHECK_EQUAL( coords.width(), WIDTH );
    BOOST_CHECK_EQUAL( coords.height(), HEIGHT );
}

BOOST_AUTO_TEST_CASE( testFullScreenSize )
{
    ContentPtr content( new DummyContent );
    content->setDimensions( QSize( WIDTH, HEIGHT ));
    ContentWindow window( content );

    DisplayGroupPtr displayGroup( new DisplayGroup( wallSize ));
//    window.toggleFullscreen();

    const QRectF& coords = window.getCoordinates();

    const float posX = ( wallSize.width() - float( WIDTH )) * 0.5;
    const float posY = ( wallSize.height() - float( HEIGHT )) * 0.5;

    // full screen, center on wall
//    BOOST_CHECK_EQUAL( window.getSizeState(), SIZE_FULLSCREEN );
    BOOST_CHECK_EQUAL( coords.x(), posX );
    BOOST_CHECK_EQUAL( coords.y(), posY );
    BOOST_CHECK_EQUAL( coords.width(), WIDTH );
    BOOST_CHECK_EQUAL( coords.height(), HEIGHT );
}

BOOST_AUTO_TEST_CASE( testFromFullscreenBackToNormalized )
{
    ContentPtr content( new DummyContent );
    content->setDimensions( QSize( WIDTH, HEIGHT ));
    ContentWindow window( content );

    const QRectF target( 900.0, 700.0, 200.0, 1000.0 );
    window.setSize( target.size( ));
    window.setPosition( target.topLeft( ));

    QRectF coords = window.getCoordinates();
    BOOST_CHECK_EQUAL( coords.x(), target.x( ));
    BOOST_CHECK_EQUAL( coords.y(), target.y( ));
    BOOST_CHECK_EQUAL( coords.width(), target.width( ));
    BOOST_CHECK_EQUAL( coords.height(), target.height( ));

//    window.toggleFullscreen();
//    window.toggleFullscreen();

    coords = window.getCoordinates();

    // back to original position and size
    BOOST_CHECK_EQUAL( coords.x(), target.x( ));
    BOOST_CHECK_EQUAL( coords.y(), target.y( ));
    BOOST_CHECK_EQUAL( coords.width(), target.width( ));
    BOOST_CHECK_EQUAL( coords.height(), target.height( ));
}

BOOST_AUTO_TEST_CASE( testValidID )
{
    ContentPtr content( new DummyContent );
    content->setDimensions( QSize( WIDTH, HEIGHT ));
    ContentWindow window( content );

    BOOST_CHECK( window.getID() != QUuid());
}

BOOST_AUTO_TEST_CASE( testUniqueID )
{
    ContentPtr content( new DummyContent );
    content->setDimensions( QSize( WIDTH, HEIGHT ));

    ContentWindow window1( content );
    BOOST_CHECK( window1.getID() != QUuid());

    ContentWindow window2( content );
    BOOST_CHECK( window2.getID() != QUuid());

    BOOST_CHECK( window1.getID() != window2.getID());
}

BOOST_AUTO_TEST_CASE( testSetContent )
{
    ContentPtr content( new DummyContent );
    content->setDimensions( QSize( WIDTH, HEIGHT ));

    ContentWindow window;
    BOOST_CHECK( !window.getContent( ));

    window.setContent( content );
    BOOST_CHECK_EQUAL( window.getContent(), content );

    window.setContent( ContentPtr( ));
    BOOST_CHECK( !window.getContent( ));
}
