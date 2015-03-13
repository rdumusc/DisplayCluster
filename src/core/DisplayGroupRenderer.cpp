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
#include "RenderContext.h"
#include "Options.h"

#include <boost/foreach.hpp>
#include <boost/make_shared.hpp>

#include <QtDeclarative/QDeclarativeContext>
#include <QtDeclarative/QDeclarativeComponent>
#include <QtDeclarative/QDeclarativeItem>

namespace
{
const float BACKGROUND_Z_COORD = -1.f + std::numeric_limits<float>::epsilon();
const QUrl QML_DISPLAYGROUP_URL( "qrc:/qml/core/DisplayGroup.qml" );
}

DisplayGroupRenderer::DisplayGroupRenderer( RenderContextPtr renderContext )
    : renderContext_( renderContext )
    , displayGroupItem_( 0 )
{
    setRenderingOptions( boost::make_shared<Options>( ));
}

void DisplayGroupRenderer::setRenderingOptions( OptionsPtr options )
{
    QDeclarativeEngine& engine = renderContext_->getQmlEngine();
    engine.rootContext()->setContextProperty( "options", options.get( ));
    options_ = options;
}

void DisplayGroupRenderer::setDisplayGroup( DisplayGroupPtr displayGroup )
{
    QDeclarativeEngine& engine = renderContext_->getQmlEngine();

    // Update the scene with the new information
    engine.rootContext()->setContextProperty( "displaygroup",
                                              displayGroup.get( ));

    if( !displayGroupItem_ )
        createDisplayGroupQmlItem();

    ContentWindowPtrs contentWindows = displayGroup->getContentWindows();

    // Update windows, creating new ones if needed
    QSet<QUuid> updatedWindows;
    BOOST_FOREACH( ContentWindowPtr window, contentWindows )
    {
        const QUuid& id = window->getID();

        updatedWindows.insert( id );

        if( windowItems_.contains( id ))
            windowItems_[id]->update( window );
        else
            createWindowQmlItem( window );
    }

    // Remove old windows
    QmlWindows::iterator it = windowItems_.begin();
    while( it != windowItems_.end( ))
    {
        if( updatedWindows.contains( it.key( )))
            ++it;
        else
            it = windowItems_.erase( it );
    }

    // Store the new DisplayGroup
    displayGroup_ = displayGroup;
}

void DisplayGroupRenderer::preRenderUpdate( WallToWallChannel& wallChannel )
{
    foreach( QmlWindowPtr window, windowItems_ )
    {
        const QRect& visibleWallArea = renderContext_->getVisibleWallArea();
        window->preRenderUpdate( wallChannel, visibleWallArea );
    }

    // TODO BACKGROUND CONTENT
}

void DisplayGroupRenderer::postRenderUpdate( WallToWallChannel& wallChannel )
{
    foreach( QmlWindowPtr window, windowItems_ )
    {
        window->postRenderUpdate( wallChannel );
    }

    // TODO BACKGROUND CONTENT
}

void DisplayGroupRenderer::createDisplayGroupQmlItem()
{
    QDeclarativeEngine& engine = renderContext_->getQmlEngine();

    QDeclarativeComponent component( &engine, QML_DISPLAYGROUP_URL );
    displayGroupItem_ = qobject_cast<QDeclarativeItem*>( component.create( ));
    renderContext_->getScene().addItem( displayGroupItem_ );
}

void DisplayGroupRenderer::createWindowQmlItem( ContentWindowPtr window )
{
    QDeclarativeEngine& engine = renderContext_->getQmlEngine();

    const QUuid& id = window->getID();
    windowItems_[id].reset( new QmlWindowRenderer( engine, *displayGroupItem_,
                                                   window ));
}
