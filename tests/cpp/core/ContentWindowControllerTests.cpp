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
const QSizeF wallSize( 1000, 1000 );
const QSize CONTENT_SIZE( 800, 600 );
const qreal CONTENT_AR = qreal(CONTENT_SIZE.width()) /
                         qreal(CONTENT_SIZE.height());
}

BOOST_AUTO_TEST_CASE( testResizeAndMove )
{
    ContentPtr content( new DummyContent );
    content->setDimensions( CONTENT_SIZE );
    ContentWindow window( content );

    DisplayGroupPtr displayGroup( new DisplayGroup( wallSize ));
    ContentWindowController controller( window, *displayGroup );

    const QPointF targetPosition( 124.2, 457.3 );
    const QSizeF targetSize( 0.7 * QSizeF( content->getDimensions( )));

    controller.moveTo( targetPosition );
    controller.resize( targetSize );

    const QRectF& coords = window.getCoordinates();

    BOOST_CHECK_EQUAL( coords.topLeft(), targetPosition );
    BOOST_CHECK_EQUAL( coords.size(), targetSize );

    const QPointF targetCenterPosition( 568.2, 389.0 );
    controller.moveCenterTo( targetCenterPosition );

    BOOST_CHECK_EQUAL( coords.center(), targetCenterPosition );
    BOOST_CHECK_EQUAL( coords.size(), targetSize );

    const QPointF fixedCenter = coords.center();
    const QSizeF centeredSize( 0.5 * QSizeF( content->getDimensions( )));

    controller.resize( centeredSize, WindowPoint::CENTER );

    BOOST_CHECK_CLOSE( coords.center().x(), fixedCenter.x(), 0.00001 );
    BOOST_CHECK_CLOSE( coords.center().y(), fixedCenter.y(), 0.00001 );
    BOOST_CHECK_CLOSE( coords.width(), centeredSize.width(), 0.00001 );
    BOOST_CHECK_CLOSE( coords.height(), centeredSize.height(), 0.00001 );
}

BOOST_AUTO_TEST_CASE( testOneToOneSize )
{
    ContentPtr content( new DummyContent );
    content->setDimensions( CONTENT_SIZE );
    ContentWindow window( content );

    window.setCoordinates( QRectF( 610, 220, 30, 40 ));

    const QRectF& coords = window.getCoordinates();

    BOOST_REQUIRE_EQUAL( coords.topLeft(), QPointF( 610, 220 ));
    BOOST_REQUIRE_EQUAL( coords.center(), QPointF( 625, 240 ));

    DisplayGroupPtr displayGroup( new DisplayGroup( wallSize ));
    ContentWindowController controller( window, *displayGroup );

    controller.adjustSize( SIZE_1TO1 );

    // 1:1 size restored around existing window center
    BOOST_CHECK_EQUAL( coords.size(), CONTENT_SIZE );
    BOOST_CHECK_EQUAL( coords.center(), QPointF( 625, 240 ));
}

//BOOST_AUTO_TEST_CASE( testFullScreenSize )
//{
//    ContentPtr content( new DummyContent );
//    content->setDimensions( CONTENT_SIZE );
//    ContentWindow window( content );

//    DisplayGroupPtr displayGroup( new DisplayGroup( wallSize ));
//    ContentWindowController controller( window, *displayGroup );

//    BOOST_REQUIRE( !window.hasBackupCoordinates( ));

//    controller.toggleFullscreen();
//    const QRectF& coords = window.getCoordinates();

//    // full screen, center on wall
//    BOOST_CHECK( window.hasBackupCoordinates( ));
//    BOOST_CHECK_EQUAL( coords.x(), 0.0 );
//    BOOST_CHECK_EQUAL( coords.y(),  125 );
//    BOOST_CHECK_EQUAL( coords.width(), wallSize.width( ));
//    BOOST_CHECK_EQUAL( coords.height(), wallSize.width() / CONTENT_AR );
//}

//BOOST_AUTO_TEST_CASE( testFromFullscreenBackToNormalized )
//{
//    ContentPtr content( new DummyContent );
//    content->setDimensions( CONTENT_SIZE );
//    ContentWindow window( content );

//    DisplayGroupPtr displayGroup( new DisplayGroup( wallSize ));
//    ContentWindowController controller( window, *displayGroup );

//    BOOST_REQUIRE( !window.hasBackupCoordinates( ));

//    const QRectF target( QPointF( 100.0, 125.0 ),
//                         content->getDimensions() * 1.75 );
//    window.setCoordinates( target );

//    QRectF coords = window.getCoordinates();
//    BOOST_CHECK( !window.hasBackupCoordinates( ));
//    BOOST_CHECK_EQUAL( coords, target );

//    controller.toggleFullscreen();
//    BOOST_CHECK( window.hasBackupCoordinates( ));
//    controller.toggleFullscreen();

//    coords = window.getCoordinates();

//    // back to original position and size
//    BOOST_CHECK( !window.hasBackupCoordinates( ));
//    BOOST_CHECK_EQUAL( coords, target );
//}

BOOST_AUTO_TEST_CASE( testResizeRelative )
{
    ContentPtr content( new DummyContent );
    content->setDimensions( CONTENT_SIZE );
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

//BOOST_AUTO_TEST_CASE( testFocusMode )
//{
//    ContentPtr content( new DummyContent );
//    content->setDimensions( CONTENT_SIZE );
//    ContentWindow window( content );

//    DisplayGroupPtr displayGroup( new DisplayGroup( wallSize ));
//    ContentWindowController controller( window, *displayGroup );

//    controller.toggleFocusMode();
//    const QRectF& coords = window.getCoordinates();

//    // focus mode, vertically centered on wall and repect inner margin
//    BOOST_CHECK( window.isSelected( ));
//    BOOST_CHECK( window.hasBackupCoordinates( ));
//    BOOST_CHECK_EQUAL( coords.x(), 50 );
//    BOOST_CHECK_EQUAL( coords.y(), 162.5 );
//    BOOST_CHECK_EQUAL( coords.width(), 900 );
//    BOOST_CHECK_EQUAL( coords.height(), 675 );
//}

//BOOST_AUTO_TEST_CASE( testFromFocumsModeBackToNormalized )
//{
//    ContentPtr content( new DummyContent );
//    content->setDimensions( CONTENT_SIZE );
//    ContentWindow window( content );

//    DisplayGroupPtr displayGroup( new DisplayGroup( wallSize ));
//    ContentWindowController controller( window, *displayGroup );

//    BOOST_REQUIRE( !window.hasBackupCoordinates( ));
//    BOOST_REQUIRE( !window.isSelected( ));

//    // Place the content at a know location and size
//    const QRectF target( QPointF( 100.0, 125.0 ),
//                         content->getDimensions() * 0.5 );
//    window.setCoordinates( target );

//    QRectF coords = window.getCoordinates();
//    BOOST_CHECK( !window.hasBackupCoordinates( ));
//    BOOST_CHECK_EQUAL( coords, target );

//    controller.toggleFocusMode();
//    BOOST_CHECK( window.hasBackupCoordinates( ));
//    BOOST_CHECK( window.isSelected( ));
//    controller.toggleFocusMode();

//    coords = window.getCoordinates();

//    // back to original position and size
//    BOOST_CHECK( !window.hasBackupCoordinates( ));
//    BOOST_CHECK( !window.isSelected( ));
//    BOOST_CHECK_EQUAL( coords, target );
//}
