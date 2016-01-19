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

#include "DisplayGroupView.h"

#include "ContentWindow.h"
#include "DisplayGroup.h"
#include "Options.h"
#include "qmlUtils.h"

#include <QQmlComponent>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQuickItem>

#define VIEW_MARGIN 0.05

namespace
{
const QString MASTER_WINDOW_OBJECT_NAME( "MasterContentWindow" );
const QUrl QML_CONTENTWINDOW_URL( "qrc:/qml/master/MasterContentWindow.qml" );
const QUrl QML_DISPLAYGROUP_URL( "qrc:/qml/master/MasterDisplayGroup.qml" );
const QUrl QML_BACKGROUND_URL( "qrc:/qml/master/DisplayGroupBackground.qml" );
}

DisplayGroupView::DisplayGroupView( OptionsPtr options )
    : displayGroupItem_( 0 )
{
    engine()->rootContext()->setContextProperty( "options", options.get( ));
    engine()->rootContext()->setContextProperty( "view", this );
    engine()->rootContext()->setContextProperty( "cppcontrolpanel",
                                                 &controlPanel_ );
}

DisplayGroupView::~DisplayGroupView() {}

void DisplayGroupView::setDataModel( DisplayGroupPtr displayGroup,
                                     const QSize& numberOfTiles,
                                     const int mullionWidth )
{
    if( displayGroup_ )
    {
        displayGroup_->disconnect( this );
        clearScene();
    }

    displayGroup_ = displayGroup;
    if( !displayGroup_ )
        return;

    rootContext()->setContextProperty( "displaygroup", displayGroup_.get( ));
    setSource( QML_BACKGROUND_URL );

    QQmlComponent component( engine(), QML_DISPLAYGROUP_URL );
    displayGroupItem_ = qobject_cast< QQuickItem* >( component.create( ));
    qmlCheckOrThrow( component );
    displayGroupItem_->setParentItem( rootObject( ));
    displayGroupItem_->setProperty( "numberOfTilesX", numberOfTiles.width( ));
    displayGroupItem_->setProperty( "numberOfTilesY", numberOfTiles.height( ));
    displayGroupItem_->setProperty( "mullionWidth", mullionWidth );

    ContentWindowPtrs contentWindows = displayGroup_->getContentWindows();
    for( ContentWindowPtr contentWindow : contentWindows )
        add( contentWindow );

    connect( displayGroup_.get(),
             SIGNAL( contentWindowAdded( ContentWindowPtr )),
             this, SLOT( add( ContentWindowPtr )));
    connect( displayGroup_.get(),
             SIGNAL( contentWindowRemoved( ContentWindowPtr )),
             this, SLOT( remove( ContentWindowPtr )));
    connect( displayGroup_.get(),
             SIGNAL( contentWindowMovedToFront( ContentWindowPtr )),
             this, SLOT( moveToFront( ContentWindowPtr )));

    setResizeMode( QQuickView::SizeRootObjectToView );
}

QmlControlPanel& DisplayGroupView::getControlPanel()
{
    return controlPanel_;
}

void DisplayGroupView::clearScene()
{
    foreach( QQuickItem* itemToRemove, uuidToWindowMap_ )
    {
        itemToRemove->setParentItem( 0 );
        delete itemToRemove;
    }

    uuidToWindowMap_.clear();
}

bool DisplayGroupView::event( QEvent *evt )
{
    switch( evt->type( ))
    {
    case QEvent::KeyPress:
    {
        QKeyEvent* k = static_cast< QKeyEvent* >( evt );

        // Override default behaviour to process TAB key events
        QQuickView::keyPressEvent( k );

        if( k->key() == Qt::Key_Backtab ||
            k->key() == Qt::Key_Tab ||
           ( k->key() == Qt::Key_Tab && ( k->modifiers() & Qt::ShiftModifier )))
        {
            evt->accept();
        }
        return true;
    }
    default:
        return QQuickView::event( evt );
    }
}

void DisplayGroupView::resizeEvent( QResizeEvent* resizeEvt )
{
//    const QSizeF& sceneSize = scene()->sceneRect().size();

//    QSizeF windowSize( width(), height( ));
//    windowSize.scale( sceneSize, Qt::KeepAspectRatioByExpanding );
//    windowSize = windowSize * ( 1.0 + VIEW_MARGIN );

//    // Center the scene in the view
//    setSceneRect( -0.5 * (windowSize.width() - sceneSize.width()),
//                  -0.5 * (windowSize.height() - sceneSize.height()),
//                  windowSize.width(), windowSize.height( ));
//    fitInView( sceneRect( ));

    QWindow::resizeEvent( resizeEvt );
}

void DisplayGroupView::add( ContentWindowPtr contentWindow )
{
    // New Context for the window, ownership retained by the windowItem
    QQmlContext* windowContext = new QQmlContext( rootContext( ));
    windowContext->setContextProperty( "contentwindow", contentWindow.get( ));

    QQmlComponent component( engine(), QML_CONTENTWINDOW_URL );
    QObject* windowItem = component.create( windowContext );
    windowContext->setParent( windowItem );

    // Store a reference to the window and add it to the scene
    const QUuid& id = contentWindow->getID();
    uuidToWindowMap_[ id ] = qobject_cast<QQuickItem*>( windowItem );
    uuidToWindowMap_[ id ]->setParentItem( displayGroupItem_ );
}

void DisplayGroupView::remove( ContentWindowPtr contentWindow )
{
    const QUuid& id = contentWindow->getID();
    if( !uuidToWindowMap_.contains( id ))
        return;

    QQuickItem* itemToRemove = uuidToWindowMap_[id];
    uuidToWindowMap_.remove( id );
    delete itemToRemove;
}

void DisplayGroupView::moveToFront( ContentWindowPtr contentWindow )
{
    const QUuid& id = contentWindow->getID();
    if( !uuidToWindowMap_.contains( id ))
        return;

    QQuickItem* itemToRaise = uuidToWindowMap_[id];

    QList<QQuickItem*> windows = displayGroupItem_->childItems();
    foreach( QQuickItem* item, windows )
    {
        if( item != itemToRaise && item->objectName() == MASTER_WINDOW_OBJECT_NAME )
            item->stackBefore( itemToRaise );
    }
}
