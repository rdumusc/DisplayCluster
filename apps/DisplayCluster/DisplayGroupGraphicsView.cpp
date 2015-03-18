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

#include "DisplayGroupGraphicsView.h"

#include "DisplayGroup.h"
#include "DisplayGroupGraphicsScene.h"
#include "ContentWindowTouchArea.h"
#include "ContentWindow.h"

#include "gestures/PanGesture.h"
#include "gestures/PanGestureRecognizer.h"
#include "gestures/PinchGesture.h"
#include "gestures/PinchGestureRecognizer.h"

#include <boost/foreach.hpp>
#include <QtDeclarative/QDeclarativeComponent>
#include <QtDeclarative/QDeclarativeContext>

#define VIEW_MARGIN 0.05

namespace
{
const QString TOUCH_AREA_OBJECT_NAME( "ContentWindowTouchArea" );
const QUrl QML_CONTENTWINDOW_URL( "qrc:/qml/master/MasterContentWindow.qml" );
const QUrl QML_DISPLAYGROUP_URL( "qrc:/qml/master/MasterDisplayGroup.qml" );
}

DisplayGroupGraphicsView::DisplayGroupGraphicsView( const Configuration& config,
                                                    QWidget* parent_ )
    : QGraphicsView( parent_ )
    , displayGroupItem_( 0 )
{
    setScene( new DisplayGroupGraphicsScene( config, this ));
    setAlignment( Qt::AlignLeft | Qt::AlignTop );

    setInteractive( true );
    setDragMode( QGraphicsView::RubberBandDrag );
    setAcceptDrops( true );
}

DisplayGroupGraphicsView::~DisplayGroupGraphicsView()
{
}

void DisplayGroupGraphicsView::notifyBackgroundTap( QPointF globalPos )
{
    emit backgroundTap( getScenePos( globalPos ));
}

void DisplayGroupGraphicsView::notifyBackgroundTapAndHold( QPointF globalPos )
{
    emit backgroundTapAndHold( getScenePos( globalPos ));
}

void DisplayGroupGraphicsView::setDataModel( DisplayGroupPtr displayGroup )
{
    if( displayGroup_ )
    {
        displayGroup_->disconnect( this );
        clearScene();
    }

    displayGroup_ = displayGroup;
    if( !displayGroup_ )
        return;

    engine_.rootContext()->setContextProperty( "displaygroup",
                                               displayGroup_.get( ));
    engine_.rootContext()->setContextProperty( "dggv", this );

    QDeclarativeComponent component( &engine_, QML_DISPLAYGROUP_URL );
    displayGroupItem_ = qobject_cast< QGraphicsObject* >( component.create( ));
    scene()->addItem( displayGroupItem_ );

    ContentWindowPtrs contentWindows = displayGroup_->getContentWindows();
    BOOST_FOREACH( ContentWindowPtr contentWindow, contentWindows )
    {
        add( contentWindow );
    }

    connect( displayGroup_.get(),
             SIGNAL( contentWindowAdded( ContentWindowPtr )),
             this, SLOT( add( ContentWindowPtr )));
    connect( displayGroup_.get(),
             SIGNAL( contentWindowRemoved( ContentWindowPtr )),
             this, SLOT( remove( ContentWindowPtr )));
    connect( displayGroup_.get(),
             SIGNAL( contentWindowMovedToFront( ContentWindowPtr )),
             this, SLOT( moveToFront( ContentWindowPtr )));
}

void DisplayGroupGraphicsView::clearScene()
{
    foreach( QGraphicsItem* itemToRemove, uuidToWindowMap_ )
        scene()->removeItem( itemToRemove );

    uuidToWindowMap_.clear();
}

void DisplayGroupGraphicsView::resizeEvent( QResizeEvent* resizeEvt )
{
    const QSizeF& sceneSize = scene()->sceneRect().size();

    QSizeF windowSize( width(), height( ));
    windowSize.scale( sceneSize, Qt::KeepAspectRatioByExpanding );
    windowSize = windowSize * ( 1.0 + VIEW_MARGIN );

    // Center the scene in the view
    setSceneRect( -0.5 * (windowSize.width() - sceneSize.width()),
                  -0.5 * (windowSize.height() - sceneSize.height()),
                  windowSize.width(), windowSize.height( ));
    fitInView( sceneRect( ));

    QGraphicsView::resizeEvent( resizeEvt );
}

QPointF DisplayGroupGraphicsView::getScenePos( const QPointF& pos_ ) const
{
    // QGesture::hotSpot() gives the position (in pixels) in "global screen
    // coordinates", i.e. on the display where the Rank0 Qt MainWindow lives.

    // Some gestures also have a position attribute but it is inconsistent.
    // For most gestures it is the same as the hotSpot, but not for QTapGesture.
    //
    // Examples taken from qstandardgestures.cpp:
    // QTapGesture.position() == touchPoint.pos()  (== viewPos)
    // QTapGesture.hotSpot() == touchPoint.screenPos()
    // QPinchGesture.centerPoint() == middle_of_2(touchPoint.screenPos())
    // QPinchGesture.hotSpot() == touchPoint.screenPos()
    // QTapAndHoldGesture.position() == touchPoint.startScreenPos()
    // QTapAndHoldGesture.hotSpot() == touchPoint.startScreenPos()

    // Note that the necessary rounding here is likely to cause imprecisions if
    // the view is small...
    return mapToScene( mapFromGlobal( pos_.toPoint( )));
}

void DisplayGroupGraphicsView::add( ContentWindowPtr contentWindow )
{
    // New Context for the window, ownership retained by the windowItem
    QDeclarativeContext* rootContext = engine_.rootContext();
    QDeclarativeContext* windowContext = new QDeclarativeContext( rootContext );
    ContentWindowController* controller =
            new ContentWindowController( *contentWindow, *displayGroup_,
                                         windowContext );
    windowContext->setContextProperty( "controller", controller );
    windowContext->setContextProperty( "contentwindow", contentWindow.get( ));

    QDeclarativeComponent component( &engine_, QML_CONTENTWINDOW_URL );
    QObject* windowItem = component.create( windowContext );
    windowContext->setParent( windowItem );

    ContentWindowTouchArea* touchArea =
       windowItem->findChild<ContentWindowTouchArea*>( TOUCH_AREA_OBJECT_NAME );
    touchArea->init( contentWindow, controller );

    // Store a reference to the window and add it to the scene
    const QUuid& id = contentWindow->getID();
    uuidToWindowMap_[ id ] = qobject_cast<QGraphicsItem*>( windowItem );
    uuidToWindowMap_[ id ]->setParentItem( displayGroupItem_ );
}

void DisplayGroupGraphicsView::remove( ContentWindowPtr contentWindow )
{
    const QUuid& id = contentWindow->getID();
    if( !uuidToWindowMap_.contains( id ))
        return;

    QGraphicsItem* itemToRemove = uuidToWindowMap_[id];
    uuidToWindowMap_.remove( contentWindow->getID( ));

    scene()->removeItem( itemToRemove );
}

void DisplayGroupGraphicsView::moveToFront( ContentWindowPtr contentWindow )
{
    const QUuid& id = contentWindow->getID();
    if( !uuidToWindowMap_.contains( id ))
        return;

    QGraphicsItem* itemToRaise = uuidToWindowMap_[id];

    QList<QGraphicsItem*> windows = displayGroupItem_->childItems();
    foreach( QGraphicsItem* item, windows )
    {
        QGraphicsObject* obj = item->toGraphicsObject();
        if( !obj )
            continue;

        if( obj->findChild<ContentWindowTouchArea*>( TOUCH_AREA_OBJECT_NAME ))
            item->stackBefore( itemToRaise );
    }
}
