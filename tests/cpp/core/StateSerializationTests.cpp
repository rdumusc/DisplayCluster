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

#define BOOST_TEST_MODULE StateSerializationTests
#include <boost/test/unit_test.hpp>
namespace ut = boost::unit_test;

#include "State.h"
#include "StateSerializationHelper.h"

#include "types.h"
#include "Content.h"
#include "ContentFactory.h"
#include "TextureContent.h"
#include "DummyContent.h"
#include "ContentWindow.h"
#include "DisplayGroup.h"

#include <fstream>
#include <boost/make_shared.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_archive_exception.hpp>

#include <QtCore/QDir>
#include <QtGui/QImage>

// QCoreApplication is required by QtXml for loading legacy configuration files
#include "MinimalGlobalQtApp.h"
BOOST_GLOBAL_FIXTURE( MinimalGlobalQtApp )

namespace
{
const QSize wallSize( 3840*2+14, 1080*3+2*12 );
const QSize CONTENT_SIZE( 100, 100 );
const int DUMMY_PARAM_VALUE = 10;
const QString DUMMY_URI = "/dummuy/uri";
const QString INVALID_URI = "/invalid/uri";
const QString VALID_TEXTURE_URI = "wall.png";
const QString LEGACY_URI = "legacy.dcx";
const QString STATE_V0_URI = "state_v0.dcx";
const QString STATE_V0_PREVIEW_FILE = "state_v0.dcxpreview";
const QString STATE_V0_BROKEN_URI = "state_v0_broken.dcx";
const QString TEST_DIR = "tmp";
const QSize VALID_TEXTURE_SIZE( 256, 128 );
}

BOOST_AUTO_TEST_CASE( testWhenStateIsSerializedAndDeserializedThenContentPropertiesArePreserved )
{
    // Serialize
    std::stringstream ss;
    {
        DummyContent* dummyContent = new DummyContent( DUMMY_URI );
        ContentPtr content( dummyContent );
        dummyContent->dummyParam_ = DUMMY_PARAM_VALUE;

        content->setDimensions( CONTENT_SIZE );
        ContentWindowPtr window( new ContentWindow( content ));

        ContentWindowPtrs contentWindows;
        contentWindows.push_back( window );
        State state( contentWindows );
        boost::archive::xml_oarchive oa( ss );
        oa << BOOST_SERIALIZATION_NVP( state );
    }

    // Deserialize
    ContentWindowPtrs contentWindows;
    {
        State state;
        boost::archive::xml_iarchive ia( ss );
        ia >> BOOST_SERIALIZATION_NVP( state );
        contentWindows = state.getContentWindows();
    }

    BOOST_REQUIRE_EQUAL( contentWindows.size(), 1 );
    Content* content = contentWindows[0]->getContent().get();
    DummyContent* dummyContent = dynamic_cast< DummyContent* >( content );
    BOOST_REQUIRE( dummyContent );

    const QSize dimensions = dummyContent->getDimensions();

    BOOST_CHECK_EQUAL( dimensions, CONTENT_SIZE );
    BOOST_CHECK_EQUAL( dummyContent->dummyParam_, DUMMY_PARAM_VALUE );
    BOOST_CHECK_EQUAL( dummyContent->getType(), CONTENT_TYPE_ANY );
    BOOST_CHECK_EQUAL( dummyContent->getURI().toStdString(),
                       DUMMY_URI.toStdString( ));
}

BOOST_AUTO_TEST_CASE( testWhenOpeningInvalidLegacyStateThenReturnFalse )
{
    State state;
    BOOST_CHECK( !state.legacyLoadXML( INVALID_URI ));
}

BOOST_AUTO_TEST_CASE( testWhenOpeningValidLegacyStateThenContentIsLoaded )
{
    State state;
    BOOST_CHECK( state.legacyLoadXML( LEGACY_URI ));
    ContentWindowPtrs contentWindows = state.getContentWindows();

    BOOST_REQUIRE_EQUAL( contentWindows.size(), 1 );
}

BOOST_AUTO_TEST_CASE( testStateSerializationHelperReadingFromLegacyFile )
{
    DisplayGroupPtr displayGroup( new DisplayGroup( wallSize ));
    StateSerializationHelper helper( displayGroup );

    bool success = false;
    // cppcheck-suppress redundantAssignment
    BOOST_CHECK_NO_THROW( success = helper.load( LEGACY_URI ));
    BOOST_CHECK( success );

    BOOST_CHECK_EQUAL( displayGroup->getContentWindows().size(), 1 );
}

BOOST_AUTO_TEST_CASE( testWhenOpeningBrokenStateThenNoExceptionIsThrown )
{
    DisplayGroupPtr displayGroup( new DisplayGroup( wallSize ));
    StateSerializationHelper helper( displayGroup );

    bool success = false;
    // cppcheck-suppress redundantAssignment
    BOOST_CHECK_NO_THROW( success = helper.load( STATE_V0_BROKEN_URI ));
    BOOST_CHECK( !success );
}

void checkLegacyWindow( ContentWindowPtr window )
{
    BOOST_CHECK_EQUAL( window->getZoomRect().width(), 1.0/1.5 );
    BOOST_CHECK_EQUAL( window->getCoordinates(), QRectF( 0.25, 0.25, 0.5, 0.5 ));

    ContentPtr content = window->getContent();
    BOOST_CHECK_EQUAL( content->getDimensions(), VALID_TEXTURE_SIZE );
    BOOST_CHECK_EQUAL( content->getType(), CONTENT_TYPE_TEXTURE );
    BOOST_CHECK_EQUAL( content->getURI().toStdString(),
                       VALID_TEXTURE_URI.toStdString() );
}

void checkWindow( ContentWindowPtr window )
{
    BOOST_CHECK_EQUAL( window->getZoomRect().width(), 1.0/1.5 );

    BOOST_CHECK_EQUAL( window->getCoordinates().x(), 0.25 * wallSize.width( ));
    BOOST_CHECK_EQUAL( window->getCoordinates().y(), 0.25 * wallSize.height( ));
    BOOST_CHECK_EQUAL( window->getCoordinates().size(), 0.5 * wallSize );

    ContentPtr content = window->getContent();
    BOOST_CHECK_EQUAL( content->getDimensions(), VALID_TEXTURE_SIZE );
    BOOST_CHECK_EQUAL( content->getType(), CONTENT_TYPE_TEXTURE );
    BOOST_CHECK_EQUAL( content->getURI().toStdString(),
                       VALID_TEXTURE_URI.toStdString() );
}

BOOST_AUTO_TEST_CASE( testWhenOpeningValidStateThenContentIsLoaded )
{
    std::ifstream ifs( STATE_V0_URI.toStdString( ));
    BOOST_REQUIRE( ifs.good( ));

    State state;
    boost::archive::xml_iarchive ia( ifs );
    BOOST_CHECK_NO_THROW( ia >> BOOST_SERIALIZATION_NVP( state ));
    ifs.close();

    ContentWindowPtrs contentWindows = state.getContentWindows();
    BOOST_CHECK_EQUAL( contentWindows.size(), 1 );

    checkLegacyWindow( contentWindows[0] );
}

BOOST_AUTO_TEST_CASE( testStateSerializationHelperReadingFromFile )
{
    DisplayGroupPtr displayGroup( new DisplayGroup( wallSize ));
    StateSerializationHelper helper( displayGroup );

    bool success;
    BOOST_REQUIRE_NO_THROW( success = helper.load( STATE_V0_URI ));
    BOOST_REQUIRE( success );
    BOOST_REQUIRE_EQUAL( displayGroup->getContentWindows().size(), 1 );

    checkWindow( displayGroup->getContentWindows()[0] );
}

DisplayGroupPtr createTestDisplayGroup()
{
    ContentPtr content = ContentFactory::getContent( VALID_TEXTURE_URI );
    BOOST_REQUIRE_EQUAL( content->getDimensions(), VALID_TEXTURE_SIZE );
    ContentWindowPtr contentWindow( new ContentWindow( content ));
    const QPointF position( 0.25 * wallSize.width(), 0.25 * wallSize.height( ));
    contentWindow->setCoordinates( QRectF( position, 0.5 * wallSize ));
    contentWindow->setZoomRect( QRectF( 0.1, 0.2, 1.0/1.5, 1.0/1.5 ));
    DisplayGroupPtr displayGroup( new DisplayGroup( wallSize ));
    displayGroup->addContentWindow( contentWindow );
    return displayGroup;
}

void cleanupTestDir()
{
    const QStringList files = QDir( TEST_DIR ).entryList( QDir::NoDotAndDotDot |
                                                          QDir::Files );
    foreach( QString file, files )
        QFile::remove( TEST_DIR + "/" + file );
}

float compareImages( const QString& file1, const QString& file2 )
{
    QImage image1, image2;
    BOOST_REQUIRE( image1.load( file1 ));
    BOOST_REQUIRE( image2.load( file2 ));

    BOOST_REQUIRE_EQUAL( image1.size(), image2.size( ));
    BOOST_REQUIRE_EQUAL( image1.byteCount(), image2.byteCount( ));

    // BOOST_CHECK_EQUAL_COLLECTION is too noisy so do a silent comparison
    unsigned int errors = 0;
    const uchar* it1 = image1.bits();
    const uchar* it2 = image2.bits();
    while( it1 < image1.bits() + image1.byteCount() &&
           it2 < image2.bits() + image2.byteCount() )
    {
        if( *it1 != *it2 )
            ++errors;
        ++it1;
        ++it2;
    }
    return (float)errors / (float)image1.byteCount();
}

BOOST_AUTO_TEST_CASE( testStateSerializationToFile )
{
    // 1) Setup
    QDir dir;
    if ( !dir.mkdir( TEST_DIR ))
        cleanupTestDir();
    // empty folders contain 2 elements: '.' and '..'
    BOOST_REQUIRE_EQUAL( QDir( TEST_DIR ).count(), 2 );

    // 2) Test saving
    DisplayGroupPtr displayGroup = createTestDisplayGroup();
    StateSerializationHelper helper( displayGroup );
    BOOST_CHECK( helper.save( TEST_DIR + "/test.dcx" ));

    const QStringList files = QDir( TEST_DIR ).entryList( QDir::NoDotAndDotDot |
                                                          QDir::Files );
    BOOST_CHECK_EQUAL( files.size(), 2 );
    BOOST_CHECK( files.contains( "test.dcx" ));
    BOOST_CHECK( files.contains( "test.dcxpreview" ));

    // 3) Check preview image.
    //    Observations have shown that a 2% error margin is imperceptible.
    const float previewError = compareImages( TEST_DIR + "/test.dcxpreview",
                                              STATE_V0_PREVIEW_FILE );
    BOOST_CHECK_LT( previewError, 0.02f );

    // 4) Test restoring
    DisplayGroupPtr loadedDisplayGroup = boost::make_shared<DisplayGroup>( QSize( ));
    StateSerializationHelper loader( loadedDisplayGroup );
    BOOST_CHECK( loader.load( TEST_DIR + "/test.dcx" ));

    BOOST_REQUIRE_EQUAL( loadedDisplayGroup->getContentWindows().size(),
                         displayGroup->getContentWindows().size( ));
    checkWindow( loadedDisplayGroup->getContentWindows()[0] );

    // 4) Cleanup
    cleanupTestDir();
    dir.rmdir( TEST_DIR );
}
