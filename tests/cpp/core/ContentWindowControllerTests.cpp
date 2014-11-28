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

#define BOOST_TEST_MODULE ContentWindowControllerTests
#include <boost/test/unit_test.hpp>
namespace ut = boost::unit_test;

#include "ContentWindow.h"
#include "DisplayGroup.h"
#include "ContentWindowController.h"

#include "MinimalGlobalQtApp.h"
BOOST_GLOBAL_FIXTURE( MinimalGlobalQtApp )

#include "DummyContent.h"

namespace
{
const int WIDTH = 512;
const int HEIGHT = 256;
const QSizeF wallSize( 1000, 1000 );
const QSizeF contentSize( WIDTH, HEIGHT );
const qreal CONTENT_AR = qreal(WIDTH)/qreal(HEIGHT);
}

BOOST_AUTO_TEST_CASE( testOneToOneSize )
{
    ContentPtr content( new DummyContent );
    content->setDimensions( QSize( WIDTH, HEIGHT ));
    ContentWindow window( content );

    DisplayGroupPtr displayGroup( new DisplayGroup( wallSize ));
    ContentWindowController controller( window, *displayGroup );

    controller.adjustSize( SIZE_1TO1 );

    const QRectF& coords = window.getCoordinates();
    const float posX = ( wallSize.width() - contentSize.width( )) * 0.5;
    const float posY = ( wallSize.height() - contentSize.height( )) * 0.5;

    // 1:1 size, centered on wall
    BOOST_CHECK_EQUAL( controller.getSizeState(), SIZE_1TO1 );
    BOOST_CHECK_EQUAL( coords.x(), posX );
    BOOST_CHECK_EQUAL( coords.y(), posY );
    BOOST_CHECK_EQUAL( coords.width(), WIDTH );
    BOOST_CHECK_EQUAL( coords.height(), HEIGHT );
}

BOOST_AUTO_TEST_CASE( testFullScreenSize )
{
    ContentPtr content( new DummyContent );
    content->setDimensions( QSize( WIDTH, HEIGHT ));
    ContentWindow window( content );

    DisplayGroupPtr displayGroup( new DisplayGroup( wallSize ));
    ContentWindowController controller( window, *displayGroup );

    BOOST_CHECK_EQUAL( controller.getSizeState(), SIZE_NORMALIZED );
    controller.toggleFullscreen();
    const QRectF& coords = window.getCoordinates();

    // full screen, center on wall
    BOOST_CHECK_EQUAL( controller.getSizeState(), SIZE_FULLSCREEN );
    BOOST_CHECK_EQUAL( coords.x(), 0.0 );
    BOOST_CHECK_EQUAL( coords.y(),  0.5 * ( wallSize.width() / CONTENT_AR ));
    BOOST_CHECK_EQUAL( coords.width(), wallSize.width( ));
    BOOST_CHECK_EQUAL( coords.height(), wallSize.width() / CONTENT_AR );
}

BOOST_AUTO_TEST_CASE( testFromFullscreenBackToNormalized )
{
    ContentPtr content( new DummyContent );
    content->setDimensions( QSize( WIDTH, HEIGHT ));
    ContentWindow window( content );

    DisplayGroupPtr displayGroup( new DisplayGroup( wallSize ));
    ContentWindowController controller( window, *displayGroup );

    const QRectF target( QPointF( 100.0, 125.0 ),
                         content->getDimensions() * 1.75 );
    window.setSize( target.size( ));
    window.setPosition( target.topLeft( ));

    QRectF coords = window.getCoordinates();
    BOOST_CHECK_EQUAL( coords.x(), target.x( ));
    BOOST_CHECK_EQUAL( coords.y(), target.y( ));
    BOOST_CHECK_EQUAL( coords.width(), target.width( ));
    BOOST_CHECK_EQUAL( coords.height(), target.height( ));

    controller.toggleFullscreen();
    controller.toggleFullscreen();

    coords = window.getCoordinates();

    // back to original position and size
    BOOST_CHECK_EQUAL( controller.getSizeState(), SIZE_NORMALIZED );
    BOOST_CHECK_EQUAL( coords.x(), target.x( ));
    BOOST_CHECK_EQUAL( coords.y(), target.y( ));
    BOOST_CHECK_EQUAL( coords.width(), target.width( ));
    BOOST_CHECK_EQUAL( coords.height(), target.height( ));
}
