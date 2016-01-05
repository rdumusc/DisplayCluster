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
#include "ContentSynchronizer.h"
#include "ContentWindow.h"

#include <QQmlComponent>

#include <QGraphicsScene>

namespace
{
const QUrl QML_WINDOW_URL( "qrc:/qml/core/WallContentWindow.qml" );
}

QmlWindowRenderer::QmlWindowRenderer( QQmlEngine& engine,
                                      QQuickItem& parentItem,
                                      ContentWindowPtr contentWindow,
                                      const bool isBackground )
    : contentWindow_( contentWindow )
    , windowContext_( new QQmlContext( engine.rootContext( )))
    , windowItem_( 0 )
{
    windowContext_->setContextProperty( "contentwindow", contentWindow_.get( ));

    auto content = contentWindow_->getContent();
    auto provider = engine.imageProvider( content->getProviderId( ));
    _contentSynchronizer = ContentSynchronizer::create( content, *provider );
    windowContext_->setContextProperty( "contentsync",
                                        _contentSynchronizer.get( ));

    windowItem_ = createQmlItem( QML_WINDOW_URL );
    windowItem_->setParentItem( &parentItem );
    windowItem_->setProperty( "isBackground", isBackground );
}

QmlWindowRenderer::~QmlWindowRenderer()
{
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
                                         const QRect& /*visibleWallArea*/ )
{
    if( _contentSynchronizer )
        _contentSynchronizer->sync( wallChannel );
}

ContentWindowPtr QmlWindowRenderer::getContentWindow()
{
    return contentWindow_;
}

QQuickItem* QmlWindowRenderer::createQmlItem( const QUrl& url )
{
    QQmlComponent component( windowContext_->engine(), url );
    if( component.isError( ))
    {
        QList<QQmlError> errorList = component.errors();
        foreach( const QQmlError& error, errorList )
            qWarning() << error.url() << error.line() << error;
        return 0;
    }
    QObject* qmlObject = component.create( windowContext_.get( ));
    return qobject_cast<QQuickItem*>( qmlObject );
}
