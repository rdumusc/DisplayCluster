/*********************************************************************/
/* Copyright (c) 2011 - 2012, The University of Texas at Austin.     */
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

#include "DisplayGroup.h"

#include "ContentWindow.h"
#include "ContentWindowController.h"

#include "log.h"
#include <boost/foreach.hpp>

IMPLEMENT_SERIALIZE_FOR_XML( DisplayGroup )

DisplayGroup::DisplayGroup()
{
}

DisplayGroup::DisplayGroup( const QSizeF& size )
    : showWindowTitles_( true )
{
    coordinates_.setSize( size );
}

DisplayGroup::~DisplayGroup()
{
}

void DisplayGroup::addContentWindow( ContentWindowPtr contentWindow )
{
    BOOST_FOREACH( ContentWindowPtr existingWindow, contentWindows_ )
    {
        if( contentWindow->getID() == existingWindow->getID( ))
        {
            put_flog( LOG_DEBUG, "A window with the same id already exists!" );
            return;
        }
    }

    contentWindows_.push_back( contentWindow );
    watchChanges( contentWindow );

    contentWindow->setController(
                make_unique<ContentWindowController>( *contentWindow, *this ));

    emit( contentWindowAdded( contentWindow ));
    sendDisplayGroup();
}

void DisplayGroup::removeContentWindow( const QUuid id )
{
    removeContentWindow( getContentWindow( id ));
}

void DisplayGroup::removeContentWindow( ContentWindowPtr contentWindow )
{
    ContentWindowPtrs::iterator it = find( contentWindows_.begin(),
                                           contentWindows_.end(),
                                           contentWindow );
    if( it == contentWindows_.end( ))
        return;

    removeFocusedWindow( *it );
    contentWindows_.erase( it );

    // disconnect any existing connections with the window
    disconnect( contentWindow.get(), 0, this, 0 );

    emit( contentWindowRemoved( contentWindow ));
    sendDisplayGroup();
}

void DisplayGroup::moveContentWindowToFront( const QUuid id )
{
    moveContentWindowToFront( getContentWindow( id ));
}

void DisplayGroup::moveContentWindowToFront( ContentWindowPtr contentWindow )
{
    if( contentWindow == contentWindows_.back( ))
        return;

    ContentWindowPtrs::iterator it = find( contentWindows_.begin(),
                                           contentWindows_.end(),
                                           contentWindow );
    if( it == contentWindows_.end( ))
        return;

    // move it to end of the list (last item rendered is on top)
    contentWindows_.erase( it );
    contentWindows_.push_back( contentWindow );

    emit( contentWindowMovedToFront( contentWindow ));
    sendDisplayGroup();
}

bool DisplayGroup::getShowWindowTitles() const
{
    return showWindowTitles_;
}

bool DisplayGroup::isEmpty() const
{
    return contentWindows_.empty();
}

ContentWindowPtr DisplayGroup::getActiveWindow() const
{
    if ( isEmpty( ))
        return ContentWindowPtr();

    return contentWindows_.back();
}

const ContentWindowPtrs& DisplayGroup::getContentWindows() const
{
    return contentWindows_;
}

ContentWindowPtr DisplayGroup::getContentWindow( const QUuid& id ) const
{
    BOOST_FOREACH( ContentWindowPtr window, contentWindows_ )
    {
        if( window->getID() == id )
            return window;
    }
    return ContentWindowPtr();
}

void DisplayGroup::setContentWindows( ContentWindowPtrs contentWindows )
{
    clear();

    BOOST_FOREACH( ContentWindowPtr window, contentWindows )
    {
        addContentWindow( window );
        if( window->isFocused( ))
            focusedWindows_.insert( window );
    }
}

DisplayGroup& DisplayGroup::operator=( const DisplayGroup& displayGroup )
{
    if( this == &displayGroup )
        return *this;

    setContentWindows( displayGroup.contentWindows_ );
    setShowWindowTitles( displayGroup.showWindowTitles_ );
    setCoordinates( displayGroup.coordinates_ );
    return *this;
}

bool DisplayGroup::hasFocusedWindows() const
{
    return !focusedWindows_.empty();
}

void DisplayGroup::focus( const QUuid& id )
{
    auto window = getContentWindow( id );
    if( ! window )
        return;

    window->setFocused( true );

    if( focusedWindows_.insert( window ).second && focusedWindows_.size() == 1 )
        emit hasFocusedWindowsChanged();

    sendDisplayGroup();
}

void DisplayGroup::unfocus( const QUuid& id )
{
    auto window = getContentWindow( id );
    if( !window )
        return;

    window->setFocused( false );
    removeFocusedWindow( window );
    sendDisplayGroup();
}

void DisplayGroup::clear()
{
    put_flog( LOG_INFO, "removing %i windows", contentWindows_.size( ));

    while( !contentWindows_.empty( ))
        removeContentWindow( contentWindows_[0] );
}

void DisplayGroup::setShowWindowTitles( const bool set )
{
    if( showWindowTitles_ == set )
        return;

    showWindowTitles_ = set;

    emit showWindowTitlesChanged( set );
    sendDisplayGroup();
}

void DisplayGroup::sendDisplayGroup()
{
    emit modified( shared_from_this( ));
}

void DisplayGroup::watchChanges( ContentWindowPtr contentWindow )
{
    connect( contentWindow.get(), SIGNAL( modified( )),
             this, SLOT( sendDisplayGroup( )));
    connect( contentWindow.get(), SIGNAL( contentModified( )),
             this, SLOT( sendDisplayGroup( )));
}

void DisplayGroup::removeFocusedWindow( ContentWindowPtr window )
{
    if( focusedWindows_.erase( window ) && focusedWindows_.empty( ))
        emit hasFocusedWindowsChanged();
}
