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

#include "ContentItem.h"
#include "ContentWindow.h"
#include "ContentWindowController.h"
#include "PixelStream.h"

#include <QtDeclarative/QDeclarativeComponent>

#include <QGraphicsScene>

namespace
{
const QString CONTENT_ITEM_OBJECT_NAME( "ContentItem" );
const QString ZOOM_CONTEXT_ITEM_OBJECT_NAME( "ZoomContextItem" );
const QUrl QML_WINDOW_URL( "qrc:/qml/core/WallContentWindow.qml" );
const QUrl QML_PIXELSTREAM_URL( "qrc:/qml/core/PixelStream.qml" );
}

QmlWindowRenderer::QmlWindowRenderer( QDeclarativeEngine& engine,
                                      QDeclarativeItem& parentItem,
                                      ContentWindowPtr contentWindow,
                                      const bool isBackground )
    : contentWindow_( contentWindow )
    , windowContext_( new QDeclarativeContext( engine.rootContext( )))
    , windowItem_( 0 )
    , wallContent_( WallContent::create( *(contentWindow->getContent( ))))
{
    windowContext_->setContextProperty( "contentwindow", contentWindow_.get( ));

    windowItem_ = createQmlItem( QML_WINDOW_URL );
    windowItem_->setParentItem( &parentItem );

    ContentItem* contentItem =
       windowItem_->findChild<ContentItem*>( CONTENT_ITEM_OBJECT_NAME );
    contentItem->setWallContent( wallContent_.get( ));
    wallContent_->setQmlItem( contentItem );

    ContentItem* zoomContextItem =
      contentItem->findChild<ContentItem*>( ZOOM_CONTEXT_ITEM_OBJECT_NAME );
    zoomContextItem->setWallContent( wallContent_.get( ));

    if( contentWindow_->getContent()->getType() == CONTENT_TYPE_PIXEL_STREAM )
        setupPixelStreamItem();

    windowItem_->setProperty( "isBackground", isBackground );
}

QmlWindowRenderer::~QmlWindowRenderer()
{
    QGraphicsScene* scene = windowItem_->scene();
    scene->removeItem( windowItem_ );
    delete windowItem_;
}

void QmlWindowRenderer::update( ContentWindowPtr contentWindow )
{
    // Could be optimized by checking for changes before updating the context
    windowContext_->setContextProperty( "contentwindow", contentWindow.get( ));
    contentWindow_ = contentWindow;
}

void QmlWindowRenderer::setStackingOrder( const int value )
{
    windowItem_->setProperty( "stackingOrder", value );
}

void QmlWindowRenderer::preRenderUpdate( WallToWallChannel& wallChannel,
                                         const QRect& visibleWallArea )
{
    wallContent_->preRenderUpdate( contentWindow_, visibleWallArea );
    wallContent_->preRenderSync( wallChannel );
}

void QmlWindowRenderer::postRenderUpdate( WallToWallChannel& wallChannel )
{
    wallContent_->postRenderSync( wallChannel );
}

WallContentPtr QmlWindowRenderer::getWallContent()
{
    return wallContent_;
}

ContentWindowPtr QmlWindowRenderer::getContentWindow()
{
    return contentWindow_;
}

void QmlWindowRenderer::setupPixelStreamItem()
{
    PixelStream* stream = static_cast<PixelStream*>( wallContent_.get( ));
    windowContext_->setContextProperty( "pixelstream", stream );
    QDeclarativeItem* pixelStreamItem = createQmlItem( QML_PIXELSTREAM_URL );
    pixelStreamItem->setParentItem( windowItem_ );
}

QDeclarativeItem* QmlWindowRenderer::createQmlItem( const QUrl& url )
{
    QDeclarativeComponent component( windowContext_->engine(), url );
    QObject* qmlObject = component.create( windowContext_.get( ));
    return qobject_cast<QDeclarativeItem*>( qmlObject );
}
