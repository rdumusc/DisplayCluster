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

BOOST_AUTO_TEST_CASE( testResizeAndMove )
{
    ContentPtr content( new DummyContent );
    content->setDimensions( QSize( WIDTH, HEIGHT ));
    ContentWindow window( content );

    DisplayGroupPtr displayGroup( new DisplayGroup( wallSize ));
    ContentWindowController controller( window, *displayGroup );

    const QPointF targetPosition( 124.2, 457.3 );
    const QSizeF targetSize( 0.7 * QSizeF( content->getDimensions( )));

    controller.moveTo( targetPosition );
    controller.resize( targetSize );

    const QRectF& coords = window.getCoordinates();

    BOOST_CHECK_EQUAL( coords.x(), targetPosition.x( ));
    BOOST_CHECK_EQUAL( coords.y(), targetPosition.y( ));
    BOOST_CHECK_EQUAL( coords.width(), targetSize.width( ));
    BOOST_CHECK_EQUAL( coords.height(), targetSize.height( ));

    const QPointF targetCenterPosition( 568.2, 389.0 );
    controller.moveCenterTo( targetCenterPosition );

    BOOST_CHECK_EQUAL( coords.center().x(), targetCenterPosition.x( ));
    BOOST_CHECK_EQUAL( coords.center().y(), targetCenterPosition.y( ));
    BOOST_CHECK_EQUAL( coords.width(), targetSize.width( ));
    BOOST_CHECK_EQUAL( coords.height(), targetSize.height( ));

    const QPointF fixedCenter = coords.center();
    const QSizeF centeredSize( 0.5 * QSizeF( content->getDimensions( )));

    controller.resize( centeredSize, WindowPoint::CENTER );

    BOOST_CHECK_EQUAL( coords.center().x(), fixedCenter.x( ));
    BOOST_CHECK_EQUAL( coords.center().y(), fixedCenter.y( ));
    BOOST_CHECK_EQUAL( coords.width(), centeredSize.width( ));
    BOOST_CHECK_EQUAL( coords.height(), centeredSize.height( ));
}

BOOST_AUTO_TEST_CASE( testOneToOneSize )
{
    ContentPtr content( new DummyContent );
    content->setDimensions( QSize( WIDTH, HEIGHT ));
    ContentWindow window( content );

    window.setCoordinates( QRectF( 610, 220, 30, 40 ));

    const QRectF& coords = window.getCoordinates();

    BOOST_REQUIRE_EQUAL( coords.x(), 610 );
    BOOST_REQUIRE_EQUAL( coords.y(), 220 );
    BOOST_REQUIRE_EQUAL( coords.center().x(), 625 );
    BOOST_REQUIRE_EQUAL( coords.center().y(), 240 );

    DisplayGroupPtr displayGroup( new DisplayGroup( wallSize ));
    ContentWindowController controller( window, *displayGroup );

    controller.adjustSize( SIZE_1TO1 );

    // 1:1 size restored around existing window center
    BOOST_CHECK_EQUAL( coords.center().x(), 625 );
    BOOST_CHECK_EQUAL( coords.center().y(), 240 );
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

    BOOST_REQUIRE( !window.hasBackupCoordinates( ));

    controller.toggleFullscreen();
    const QRectF& coords = window.getCoordinates();

    // full screen, center on wall
    BOOST_CHECK( window.hasBackupCoordinates( ));
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

    BOOST_REQUIRE( !window.hasBackupCoordinates( ));

    const QRectF target( QPointF( 100.0, 125.0 ),
                         content->getDimensions() * 1.75 );
    window.setCoordinates( target );

    QRectF coords = window.getCoordinates();
    BOOST_CHECK( !window.hasBackupCoordinates( ));
    BOOST_CHECK_EQUAL( coords.x(), target.x( ));
    BOOST_CHECK_EQUAL( coords.y(), target.y( ));
    BOOST_CHECK_EQUAL( coords.width(), target.width( ));
    BOOST_CHECK_EQUAL( coords.height(), target.height( ));

    controller.toggleFullscreen();
    BOOST_CHECK( window.hasBackupCoordinates( ));
    controller.toggleFullscreen();

    coords = window.getCoordinates();

    // back to original position and size
    BOOST_CHECK( !window.hasBackupCoordinates( ));
    BOOST_CHECK_EQUAL( coords.x(), target.x( ));
    BOOST_CHECK_EQUAL( coords.y(), target.y( ));
    BOOST_CHECK_EQUAL( coords.width(), target.width( ));
    BOOST_CHECK_EQUAL( coords.height(), target.height( ));
}

BOOST_AUTO_TEST_CASE( testResizeRelative )
{
    ContentPtr content( new DummyContent );
    content->setDimensions( QSize( WIDTH, HEIGHT ));
    ContentWindow window( content );

    DisplayGroupPtr displayGroup( new DisplayGroup( wallSize ));
    ContentWindowController controller( window, *displayGroup );

    const QRectF originalCoords = window.getCoordinates();

    controller.resizeRelative( QPointF( 5, 5 ));
    BOOST_CHECK( window.getCoordinates() == originalCoords );

    window.setBorder( ContentWindow::TOP );
    controller.resizeRelative( QPointF( 5, 5 ));
    BOOST_CHECK_EQUAL( window.getCoordinates().top() - 5, originalCoords.top());

    window.setBorder( ContentWindow::BOTTOM );
    controller.resizeRelative( QPointF( 2, 2 ));
    BOOST_CHECK_EQUAL( window.getCoordinates().bottom() - 2,
                       originalCoords.bottom( ));

    window.setBorder( ContentWindow::TOP_RIGHT );
    controller.resizeRelative( QPointF( 1, 2 ));
    BOOST_CHECK( window.getCoordinates().topRight() - QPointF( 1, 7 ) ==
                 originalCoords.topRight( ));
}
