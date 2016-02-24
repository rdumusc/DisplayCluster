/*********************************************************************/
/* Copyright (c) 2015, EPFL/Blue Brain Project                       */
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

#include "QmlWindowRenderer.h"

#include "ContentSynchronizer.h"
#include "ContentWindow.h"
#include "DataProvider.h"
#include "Tile.h"

#include <QQmlComponent>

namespace
{
const QUrl QML_WINDOW_URL( "qrc:/qml/core/WallContentWindow.qml" );
const QString TILES_PARENT_OBJECT_NAME( "TilesParent" );
}

QmlWindowRenderer::QmlWindowRenderer( QQmlEngine& engine,
                                      DataProvider& provider,
                                      QQuickItem& parentItem,
                                      ContentWindowPtr contentWindow,
                                      const bool isBackground )
    : _provider( provider )
    , _contentWindow( contentWindow )
    , _windowContext( new QQmlContext( engine.rootContext( )))
    , _windowItem( 0 )
    , _synchronizer( ContentSynchronizer::create( contentWindow->getContent( )))
{
    connect( _synchronizer.get(), &ContentSynchronizer::addTile,
             this, &QmlWindowRenderer::_addTile );
    connect( _synchronizer.get(), &ContentSynchronizer::removeTile,
             this, &QmlWindowRenderer::_removeTile );
    connect( _synchronizer.get(), &ContentSynchronizer::updateTile,
             this, &QmlWindowRenderer::_updateTile );

    connect( _synchronizer.get(), &ContentSynchronizer::requestUpdate,
             &_provider, &DataProvider::loadAsync );

    _windowContext->setContextProperty( "contentwindow", _contentWindow.get( ));
    _windowContext->setContextProperty( "contentsync", _synchronizer.get( ));

    _windowItem = _createQmlItem( QML_WINDOW_URL );
    _windowItem->setParentItem( &parentItem );
    _windowItem->setProperty( "isBackground", isBackground );
}

QmlWindowRenderer::~QmlWindowRenderer()
{
    for( auto& tile : _tiles )
        tile.second->setParentItem( nullptr );
    _tiles.clear();

    delete _windowItem;
}

void QmlWindowRenderer::update( ContentWindowPtr contentWindow,
                                const QRectF& visibleArea )
{
    // Could be optimized by checking for changes before updating the context
    _windowContext->setContextProperty( "contentwindow", contentWindow.get( ));
    _contentWindow = contentWindow;
    _synchronizer->update( *_contentWindow, visibleArea );
}

void QmlWindowRenderer::synchronize( WallToWallChannel& channel )
{
    _synchronizer->synchronize( channel );
}

bool QmlWindowRenderer::needRedraw() const
{
    return _synchronizer->needRedraw();
}

QQuickItem* QmlWindowRenderer::getQuickItem()
{
    return _windowItem;
}

ContentWindowPtr QmlWindowRenderer::getContentWindow()
{
    return _contentWindow;
}

void QmlWindowRenderer::_addTile( TilePtr tile )
{
    connect( tile.get(), &Tile::readyToSwap,
             _synchronizer.get(), &ContentSynchronizer::onSwapReady );

    connect( tile.get(), &Tile::textureInitialized,
             _synchronizer.get(), &ContentSynchronizer::onTextureInitialized,
             Qt::QueuedConnection );

    _tiles[tile->getIndex()] = tile;

    auto item = _windowItem->findChild<QQuickItem*>( TILES_PARENT_OBJECT_NAME );
    tile->setParentItem( item );
}

void QmlWindowRenderer::_removeTile( const uint tileIndex )
{
    if( !_tiles.count( tileIndex ))
        return;

    _tiles[tileIndex]->disconnect( _synchronizer.get( ));
    _tiles[tileIndex]->setParentItem( nullptr );
    _tiles.erase( tileIndex );
}

void QmlWindowRenderer::_updateTile( const uint tileIndex,
                                     const QRect& coordinates )
{
    if( _tiles.count( tileIndex ))
        _tiles[tileIndex]->update( coordinates );
}

QQuickItem* QmlWindowRenderer::_createQmlItem( const QUrl& url )
{
    QQmlComponent component( _windowContext->engine(), url );
    if( component.isError( ))
    {
        QList<QQmlError> errorList = component.errors();
        foreach( const QQmlError& error, errorList )
            qWarning() << error.url() << error.line() << error;
        return 0;
    }
    QObject* qmlObject = component.create( _windowContext.get( ));
    return qobject_cast<QQuickItem*>( qmlObject );
}
