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

#include "PixelStreamWindowManager.h"

#include "ContentWindow.h"
#include "ContentWindowController.h"
#include "ContentFactory.h"
#include "DisplayGroup.h"
#include "localstreamer/DockPixelStreamer.h"
#include "log.h"

#include <deflect/Frame.h>

namespace
{
const QSize EMPTY_STREAM_SIZE( 640, 480 );
}

PixelStreamWindowManager::PixelStreamWindowManager( DisplayGroup& displayGroup )
    : QObject()
    , displayGroup_( displayGroup )
{
    connect( &displayGroup, SIGNAL( contentWindowRemoved( ContentWindowPtr )),
             this, SLOT( onContentWindowRemoved( ContentWindowPtr )));
}

ContentWindowPtr
PixelStreamWindowManager::getContentWindow( const QString& uri ) const
{
    ContentWindowMap::const_iterator it = streamerWindows_.find( uri );
    return it != streamerWindows_.end() ?
                     displayGroup_.getContentWindow( it->second ) :
                     ContentWindowPtr();
}

void PixelStreamWindowManager::hideWindow( const QString& uri )
{
    ContentWindowPtr contentWindow = getContentWindow( uri );
    if( contentWindow )
        contentWindow->setState( ContentWindow::HIDDEN );
}

void PixelStreamWindowManager::showWindow( const QString& uri )
{
    ContentWindowPtr contentWindow = getContentWindow( uri );
    if( contentWindow )
        contentWindow->setState( contentWindow->hasEventReceivers() ?
                                     ContentWindow::SELECTED :
                                     ContentWindow::NONE );
}

void PixelStreamWindowManager::openPixelStreamWindow( const QString uri,
                                                      QPointF pos,
                                                      QSize size )
{
    if( getContentWindow( uri ))
    {
        if( uri == DockPixelStreamer::getUniqueURI( ) && !pos.isNull( ))
        {
            ContentWindowPtr window = getContentWindow( uri );
            ContentWindowController controller( *window, displayGroup_ );
            controller.moveCenterTo( pos );
        }
        return;
    }

    put_flog( LOG_DEBUG, "opening pixel stream window: %s",
              uri.toLocal8Bit().constData( ));

    if( pos.isNull( ))
        pos = displayGroup_.getCoordinates().center();

    ContentPtr content = ContentFactory::getPixelStreamContent( uri );
    if( size.isValid( ))
        content->setDimensions( size );
    ContentWindowPtr contentWindow( new ContentWindow( content ));

    ContentWindowController controller( *contentWindow, displayGroup_ );
    controller.resize( size.isValid() ? size : EMPTY_STREAM_SIZE );
    controller.moveCenterTo( pos );

    streamerWindows_[ uri ] = contentWindow->getID();
    displayGroup_.addContentWindow( contentWindow );
}

void PixelStreamWindowManager::closePixelStreamWindow( const QString uri )
{
    put_flog( LOG_DEBUG, "deleting pixel stream: %s",
              uri.toLocal8Bit().constData( ));

    ContentWindowPtr contentWindow = getContentWindow( uri );
    if( contentWindow )
        displayGroup_.removeContentWindow( contentWindow );
}

void PixelStreamWindowManager::registerEventReceiver( const QString uri,
                                                      const bool exclusive,
                                                      deflect::EventReceiver* receiver )
{
    bool success = false;

    ContentWindowPtr contentWindow = getContentWindow( uri );
    if( !contentWindow )
    {
        put_flog( LOG_DEBUG, "No window found for stream: '%s', creating one.",
                  uri.toStdString().c_str( ));
        openPixelStreamWindow( uri );
        contentWindow = getContentWindow( uri );
    }

    // If a receiver is already registered, don't register this one if
    // "exclusive" was requested
    if( !exclusive || !contentWindow->hasEventReceivers( ))
    {
        success = contentWindow->registerEventReceiver( receiver );

        if( success )
            contentWindow->setState( ContentWindow::SELECTED );
    }

    emit eventRegistrationReply( uri, success );
}

void PixelStreamWindowManager::onContentWindowRemoved( ContentWindowPtr window )
{
    if( window->getContent()->getType() != CONTENT_TYPE_PIXEL_STREAM )
        return;

    const QString& uri = window->getContent()->getURI();
    streamerWindows_.erase( uri );
    emit pixelStreamWindowClosed( uri );
}

void PixelStreamWindowManager::updateStreamDimensions( deflect::FramePtr frame )
{
    const QSize size( frame->computeDimensions( ));

    ContentWindowPtr contentWindow = getContentWindow( frame->uri );
    if( !contentWindow )
        return;

    // External streamers don't have an initial size
    if( contentWindow->getContent()->getDimensions().isEmpty( ))
    {
        ContentWindowController controller( *contentWindow, displayGroup_ );
        controller.resize( size, CENTER );
    }
    contentWindow->getContent()->setDimensions( size );
}
