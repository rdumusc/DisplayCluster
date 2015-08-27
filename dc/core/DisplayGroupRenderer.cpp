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

#include "DisplayGroupRenderer.h"

#include "DisplayGroup.h"
#include "ContentWindow.h"
#include "ContentWindowController.h"
#include "RenderContext.h"
#include "Options.h"
#include "PixelStream.h"

#include <deflect/Frame.h>

#include <boost/foreach.hpp>
#include <boost/make_shared.hpp>

#include <QtDeclarative/QDeclarativeContext>
#include <QtDeclarative/QDeclarativeComponent>
#include <QtDeclarative/QDeclarativeItem>

namespace
{
const QUrl QML_DISPLAYGROUP_URL( "qrc:/qml/core/DisplayGroup.qml" );
const QString BACKGROUND_ITEM_OBJECT_NAME( "BackgroundItem" );
const int BACKGROUND_STACKING_ORDER = -1;
}

DisplayGroupRenderer::DisplayGroupRenderer( RenderContextPtr renderContext )
    : _renderContext( renderContext )
    , _displayGroup( new DisplayGroup(
                        renderContext->getScene().sceneRect().size().toSize( )))
    , _displayGroupItem( 0 )
    , _options( new Options )
{
    setDisplayGroup( _displayGroup );
    setRenderingOptions( _options );
}

void DisplayGroupRenderer::setRenderingOptions( OptionsPtr options )
{
    QDeclarativeEngine& engine = _renderContext->getQmlEngine();
    engine.rootContext()->setContextProperty( "options", options.get( ));

    _setBackground( options->getBackgroundContent( ));

    // Retain the new Options
    _options = options;
}

void DisplayGroupRenderer::setDisplayGroup( DisplayGroupPtr displayGroup )
{
    QDeclarativeEngine& engine = _renderContext->getQmlEngine();

    // Update the scene with the new information
    engine.rootContext()->setContextProperty( "displaygroup",
                                              displayGroup.get( ));

    if( !_displayGroupItem )
        _createDisplayGroupQmlItem();

    ContentWindowPtrs contentWindows = displayGroup->getContentWindows();

    // Update windows, creating new ones if needed
    QSet<QUuid> updatedWindows;
    int stackingOrder = BACKGROUND_STACKING_ORDER + 1;
    BOOST_FOREACH( ContentWindowPtr window, contentWindows )
    {
        const QUuid& id = window->getID();

        updatedWindows.insert( id );

        if( _windowItems.contains( id ))
            _windowItems[id]->update( window );
        else
            _createWindowQmlItem( window );

        _windowItems[id]->setStackingOrder( stackingOrder++ );
    }

    // Remove old windows
    QmlWindows::iterator it = _windowItems.begin();
    while( it != _windowItems.end( ))
    {
        if( updatedWindows.contains( it.key( )))
            ++it;
        else
        {
            emit windowRemoved( *it );
            it = _windowItems.erase( it );
        }
    }

    // Retain the new DisplayGroup
    _displayGroup = displayGroup;
}

void DisplayGroupRenderer::preRenderUpdate( WallToWallChannel& wallChannel )
{
    const QRect& visibleWallArea = _renderContext->getVisibleWallArea();
    foreach( QmlWindowPtr window, _windowItems )
    {
        window->preRenderUpdate( wallChannel, visibleWallArea );
    }
    if( _backgroundWindowItem )
        _backgroundWindowItem->preRenderUpdate( wallChannel, visibleWallArea );
}

void DisplayGroupRenderer::postRenderUpdate( WallToWallChannel& wallChannel )
{
    foreach( QmlWindowPtr window, _windowItems )
    {
        window->postRenderUpdate( wallChannel );
    }
    if( _backgroundWindowItem )
        _backgroundWindowItem->postRenderUpdate( wallChannel );
}

void DisplayGroupRenderer::_createDisplayGroupQmlItem()
{
    QDeclarativeEngine& engine = _renderContext->getQmlEngine();

    QDeclarativeComponent component( &engine, QML_DISPLAYGROUP_URL );
    _displayGroupItem = qobject_cast<QDeclarativeItem*>( component.create( ));
    _renderContext->getScene().addItem( _displayGroupItem );
}

void DisplayGroupRenderer::_createWindowQmlItem( ContentWindowPtr window )
{
    QDeclarativeEngine& engine = _renderContext->getQmlEngine();

    const QUuid& id = window->getID();
    _windowItems[id].reset( new QmlWindowRenderer( engine, *_displayGroupItem,
                                                   window ));
    emit windowAdded( _windowItems[id] );
}

bool DisplayGroupRenderer::_hasBackgroundChanged( const QString& newUri ) const
{
    ContentPtr prevContent = _options->getBackgroundContent();
    const QString& prevUri = prevContent ? prevContent->getURI() : QString();
    return newUri != prevUri;
}

void DisplayGroupRenderer::_setBackground( ContentPtr content )
{
    if( !content )
    {
        _backgroundWindowItem.reset();
        return;
    }

    if( !_hasBackgroundChanged( content->getURI( )))
        return;

    ContentWindowPtr window = boost::make_shared<ContentWindow>( content );
    window->setController(
               make_unique<ContentWindowController>( *window, *_displayGroup ));
    window->getController()->adjustSize( SIZE_FULLSCREEN );
    QDeclarativeEngine& engine = _renderContext->getQmlEngine();
    _backgroundWindowItem.reset( new QmlWindowRenderer( engine,
                                                        *_displayGroupItem,
                                                        window, true ));
    _backgroundWindowItem->setStackingOrder( BACKGROUND_STACKING_ORDER );
}
