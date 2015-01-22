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
#include "Factories.h"
#include "RenderContext.h"

#include <boost/foreach.hpp>

#include <QtDeclarative/QDeclarativeContext>
#include <QtDeclarative/QDeclarativeComponent>

namespace
{
const QRectF UNIT_RECTF( 0.0, 0.0, 1.0, 1.0 );
const float BACKGROUND_Z_COORD = -1.f + std::numeric_limits<float>::epsilon();
}

DisplayGroupRenderer::DisplayGroupRenderer( RenderContextPtr renderContext,
                                            FactoriesPtr factories )
    : renderContext_( renderContext )
    , factories_( factories )
    , windowRenderer_( factories )
    , displayGroupItem_( 0 )
{
}

void DisplayGroupRenderer::render()
{
    if( !displayGroup_ )
        return;

    renderBackground( displayGroup_->getBackgroundContent( ));
    render( displayGroup_->getContentWindows( ));
}

ContentWindowRenderer& DisplayGroupRenderer::getWindowRenderer()
{
    return windowRenderer_;
}

void DisplayGroupRenderer::setDisplayGroup( DisplayGroupPtr displayGroup )
{
    QDeclarativeEngine& engine = renderContext_->getQmlEngine();

    // Update the scene with the new information
    engine.rootContext()->setContextProperty( "displaygroup", displayGroup.get( ));

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
            windowItems_[id].reset( new QmlWindowRenderer( engine,
                                                           *displayGroupItem_,
                                                           window ));
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

void DisplayGroupRenderer::renderBackground( ContentPtr content )
{
    if( !content )
        return;

    // Scale fullscreen, keep aspect ratio
    QSizeF size( content->getDimensions( ));
    size.scale( displayGroup_->getCoordinates().size(), Qt::KeepAspectRatio );
    QRectF coord( QPointF(), size );
    coord.moveCenter( displayGroup_->getCoordinates().center( ));

    glPushMatrix();
    glTranslatef( coord.x(), coord.y(), BACKGROUND_Z_COORD );
    glScalef( coord.width(), coord.height(), 1.f );

    factories_->getFactoryObject( content )->render( UNIT_RECTF );

    glPopMatrix();
}

void DisplayGroupRenderer::render( const ContentWindowPtrs& contentWindows )
{
    const unsigned int windowCount = contentWindows.size();
    unsigned int windowIndex = 0;
    for( ContentWindowPtrs::const_iterator it = contentWindows.begin();
         it != contentWindows.end(); ++it, ++windowIndex )
    {
        // It is currently not possible to cull windows that are invisible as
        // this conflics with the "garbage collection" mechanism for Contents.
        // In fact, "stale" objects are objects which have not been rendered for
        // more than one frame (implicitly: objects without a window) and those
        // are destroyed by Factory::clearStaleObjects(). It is currently the
        // only way to remove a Content.
        //if( !isRegionVisible( (*it)->getCoordinates( )))
        //    continue;

        // the visible depths are in the range (-1,1);
        // make the content window depths be in the range (-1,0)
        const float zCoordinate = -(float)( windowCount - windowIndex ) /
                                   (float)( windowCount + 1 );

        glPushMatrix();
        glTranslatef( 0.f, 0.f, zCoordinate );

        windowRenderer_.setContentWindow( *it );
        windowRenderer_.render();

        glPopMatrix();
    }
}

void DisplayGroupRenderer::createDisplayGroupQmlItem()
{
    QDeclarativeEngine& engine = renderContext_->getQmlEngine();

    QDeclarativeComponent component( &engine, QUrl( "qrc:/qml/core/DisplayGroup.qml" ));
    displayGroupItem_ = qobject_cast< QGraphicsObject* >( component.create( ));
    renderContext_->getScene().addItem( displayGroupItem_ );
}
