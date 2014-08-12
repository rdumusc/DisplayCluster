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

#include "configuration/Configuration.h"
#include "ContentWindowManager.h"
#include "DisplayGroupManager.h"
#include "log.h"

PixelStreamWindowManager::PixelStreamWindowManager( DisplayGroupManager& displayGroupManager )
    : QObject()
    , displayGroupManager_( displayGroupManager )
{
    connect(&displayGroupManager, SIGNAL(contentWindowManagerRemoved(ContentWindowManagerPtr, DisplayGroupInterface*)),
            this, SLOT(onContentWindowManagerRemoved(ContentWindowManagerPtr,DisplayGroupInterface*)));
}

PixelStreamWindowManager::~PixelStreamWindowManager()
{
}

ContentWindowManagerPtr
PixelStreamWindowManager::createContentWindow( const QString& uri,
                                               const QPointF& pos,
                                               const QSizeF& size )
{
    ContentWindowManagerPtr contentWindow = getContentWindow( uri );
    if(contentWindow)
        put_flog( LOG_WARN, "Already have a window for stream: '%s'", uri.toStdString().c_str( ));
    else
        contentWindow.reset( new ContentWindowManager );

    // constrain to wall size
    double width = size.width();
    double height = size.height();
    const double ar = width/height;

    height = std::min( height, 1. );
    width = ar * height;
    if( width > 1. )
    {
        height /= width;
        width = 1.;
    }

    contentWindow->setSize( width, height );
    contentWindow->centerPositionAround( pos, true );
    streamerWindows_[uri] = contentWindow;
    return contentWindow;
}

void PixelStreamWindowManager::removeContentWindow( const QString& uri )
{
    streamerWindows_.erase( uri );
}

ContentWindowManagerPtr
PixelStreamWindowManager::getContentWindow( const QString& uri ) const
{
    ContentWindowMap::const_iterator it = streamerWindows_.find( uri );
    return it != streamerWindows_.end() ? it->second : ContentWindowManagerPtr();
}

void PixelStreamWindowManager::updateDimension( QString uri, QSize size )
{
    ContentWindowManagerPtr contentWindow = getContentWindow( uri );
    if( !contentWindow )
        return;

    ContentPtr content = contentWindow->getContent();
    content->setDimensions( size.width(), size.height( ));
}

void PixelStreamWindowManager::hideWindow( const QString& uri )
{
    ContentWindowManagerPtr contentWindow = getContentWindow( uri );
    if( contentWindow )
    {
        double x, y;
        contentWindow->getSize( x, y );
        contentWindow->setPosition( 0, -2*y );
    }
}

void PixelStreamWindowManager::openPixelStreamWindow( QString uri, QSize size )
{
    put_flog( LOG_DEBUG, "adding pixel stream: %s", uri.toLocal8Bit().constData());

    ContentPtr content = ContentFactory::getPixelStreamContent( uri );
    content->setDimensions( size.width(), size.height( ));

    ContentWindowManagerPtr contentWindow = getContentWindow( uri );
    if( contentWindow )
        contentWindow->setContent( content );
    else
    {
        // external streamers have no window yet
        contentWindow = createContentWindow( uri, QPointF(), QSizeF( ));
        contentWindow->setContent( content );
        contentWindow->adjustSize( SIZE_1TO1 );
    }

    displayGroupManager_.addContentWindowManager( contentWindow );
}

void PixelStreamWindowManager::closePixelStreamWindow( const QString& uri )
{
    put_flog( LOG_DEBUG, "deleting pixel stream: %s", uri.toLocal8Bit().constData( ));

    ContentWindowManagerPtr contentWindow = getContentWindow( uri );
    if( contentWindow )
        displayGroupManager_.removeContentWindowManager( contentWindow );
}

void PixelStreamWindowManager::registerEventReceiver( QString uri, bool exclusive,
                                                      EventReceiver* receiver )
{
    bool success = false;

    ContentWindowManagerPtr contentWindow = getContentWindow( uri );
    if( contentWindow )
    {
        put_flog( LOG_DEBUG, "found window: '%s'", uri.toStdString().c_str( ));

        // If a receiver is already registered, don't register this one if exclusive was requested
        if( !exclusive || !contentWindow->hasEventReceivers( ))
        {
            success = contentWindow->registerEventReceiver( receiver );

            if( success )
                contentWindow->setWindowState( ContentWindowInterface::SELECTED );
        }
    }
    else
        put_flog( LOG_ERROR, "could not find window: '%s'", uri.toStdString().c_str( ));

    emit eventRegistrationReply( uri, success );
}

void PixelStreamWindowManager::onContentWindowManagerRemoved( ContentWindowManagerPtr contentWindowManager,
                                                              DisplayGroupInterface* )
{
    ContentPtr content = contentWindowManager->getContent();
    if( !content )
        return;

    const QString& uri = content->getURI();
    if( !getContentWindow( uri ))
        return;

    removeContentWindow( uri );
    emit pixelStreamWindowClosed( uri );
}
