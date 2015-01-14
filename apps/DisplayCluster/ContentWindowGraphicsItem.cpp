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

#include "ContentWindowGraphicsItem.h"

#include "ContentWindow.h"
#include "Content.h"
#include "MovieContent.h"
#include "ZoomInteractionDelegate.h"

#include "gestures/DoubleTapGesture.h"
#include "gestures/DoubleTapGestureRecognizer.h"
#include "gestures/PanGesture.h"
#include "gestures/PanGestureRecognizer.h"
#include "gestures/PinchGesture.h"
#include "gestures/PinchGestureRecognizer.h"

#include <QtCore/QEvent>
#include <QtGui/QSwipeGesture>
#include <QtGui/QTapGesture>
#include <QtGui/QPainter>

#define STD_WHEEL_DELTA 120 // Common value for the delta of mouse wheel events

#define WHEEL_SCALE_SPEED   0.1

#define BUTTON_REL_WIDTH    0.23
#define BUTTON_REL_HEIGHT   0.45
#define WINDOW_INFO_FONT_SIZE_REL_TO_BUTTON_SIZE   0.15
#define TEXT_LABEL_FONT_SIZE_REL_TO_BUTTON_SIZE    0.25

qreal ContentWindowGraphicsItem::zCounter_ = 0;

ContentWindowGraphicsItem::ContentWindowGraphicsItem( ContentWindowPtr contentWindow,
                                                      const DisplayGroup& displayGroup )
    : contentWindow_( contentWindow )
    , controller_( *contentWindow, displayGroup )
{
    connect( contentWindow_.get(), SIGNAL( coordinatesAboutToChange( )),
             this, SLOT( prepareToChangeGeometry( )));

    setFlag( QGraphicsItem::ItemIsMovable, true );
    setFlag( QGraphicsItem::ItemIsFocusable, true );

    // new items at the front
    // we assume that interface items will be constructed in depth order so
    // this produces the correct result...
    setZToFront();

    grabGesture( DoubleTapGestureRecognizer::type( ));
    grabGesture( PanGestureRecognizer::type( ));
    grabGesture( PinchGestureRecognizer::type( ));
    grabGesture( Qt::SwipeGesture );
    grabGesture( Qt::TapAndHoldGesture );
    grabGesture( Qt::TapGesture );
}

ContentWindowGraphicsItem::~ContentWindowGraphicsItem()
{
}

ContentWindowPtr ContentWindowGraphicsItem::getContentWindow() const
{
    return contentWindow_;
}

void ContentWindowGraphicsItem::paint( QPainter* painter,
                                       const QStyleOptionGraphicsItem*,
                                       QWidget* )
{
    if( contentWindow_->isHidden( ))
        return;

    drawFrame_( painter );
    drawCloseButton_( painter );
    drawResizeIndicator_( painter );
    drawFullscreenButton_( painter );
    drawMovieControls_( painter );
    drawTextLabel_( painter );
    drawWindowInfo_( painter );
}

void ContentWindowGraphicsItem::setZToFront()
{
    setZValue( ++zCounter_ );
}

void ContentWindowGraphicsItem::prepareToChangeGeometry()
{
    prepareGeometryChange();
}

QRectF ContentWindowGraphicsItem::boundingRect() const
{
    return contentWindow_->getCoordinates();
}

bool ContentWindowGraphicsItem::sceneEvent( QEvent* event_ )
{
    if( contentWindow_->isHidden( ))
        return false;

    switch( event_->type( ))
    {
    case QEvent::Gesture:
        emit moveToFront( contentWindow_ );
        gestureEvent( static_cast< QGestureEvent* >( event_ ));
        return true;
    case QEvent::KeyPress:
        // Override default behaviour to process TAB key events
        keyPressEvent( static_cast< QKeyEvent* >( event_ ));
        return true;
    default:
        return QGraphicsObject::sceneEvent( event_ );
    }
}

void ContentWindowGraphicsItem::mouseMoveEvent( QGraphicsSceneMouseEvent* event_ )
{
    if( contentWindow_->isSelected( ))
    {
        contentWindow_->getInteractionDelegate().mouseMoveEvent( event_ );
        return;
    }

    if( event_->buttons().testFlag( Qt::LeftButton ))
    {
        if( contentWindow_->isResizing( ))
        {
            QRectF coordinates = boundingRect();
            coordinates.setBottomRight( event_->scenePos( ));

            const qreal targetAR = contentWindow_->getContent()->getAspectRatio();
            const qreal eventCoordAR = coordinates.width() / coordinates.height();

            if( eventCoordAR < targetAR )
                controller_.resize( QSizeF( coordinates.width(),
                                            coordinates.width() / targetAR ));
            else
                controller_.resize( QSizeF( coordinates.height() * targetAR,
                                            coordinates.height( )));
        }
        else if( contentWindow_->isMoving( ))
        {
            const QPointF delta = event_->scenePos() - event_->lastScenePos();
            const QPointF newPos = boundingRect().topLeft() + delta;
            controller_.moveTo( newPos );
        }
    }
}

void ContentWindowGraphicsItem::mousePressEvent( QGraphicsSceneMouseEvent* event_ )
{
    // on Mac we've seen that mouse events can go to the wrong graphics item
    // this is due to the bug: https://bugreports.qt.nokia.com/browse/QTBUG-20493
    // here we ignore the event if it shouldn't have been sent to us, which ensures
    // it will go to the correct item...
    if( !boundingRect().contains( event_->pos( )))
    {
        event_->ignore();
        return;
    }

    if( getCloseRect().contains( event_->pos( )))
    {
        emit close( contentWindow_ );
        return;
    }

    emit moveToFront( contentWindow_ );

    if( contentWindow_->isSelected( ))
    {
        contentWindow_->getInteractionDelegate().mousePressEvent( event_ );
        return;
    }

    if( getResizeRect().contains( event_->pos( )))
        contentWindow_->setState( ContentWindow::RESIZING );

    else if( getFullscreenRect().contains( event_->pos( )))
        controller_.toggleFullscreen();

    else if( isMovie() && getPauseRect().contains( event_->pos( )))
    {
        ContentPtr content = contentWindow_->getContent();
        MovieContent* movie = static_cast< MovieContent* >( content.get( ));
        movie->setControlState( ControlState( movie->getControlState() ^ STATE_PAUSED ));
    }

    else if( isMovie() && getLoopRect().contains( event_->pos( )))
    {
        ContentPtr content = contentWindow_->getContent();
        MovieContent* movie = static_cast< MovieContent* >( content.get( ));
        movie->setControlState( ControlState( movie->getControlState() ^ STATE_LOOP ));
    }

    else
        contentWindow_->setState( ContentWindow::MOVING );

    QGraphicsItem::mousePressEvent( event_ );
}

void ContentWindowGraphicsItem::mouseDoubleClickEvent( QGraphicsSceneMouseEvent* event_ )
{
    // on Mac we've seen that mouse events can go to the wrong graphics item
    // this is due to the bug: https://bugreports.qt.nokia.com/browse/QTBUG-20493
    // here we ignore the event if it shouldn't have been sent to us, which ensures
    // it will go to the correct item...
    if( !boundingRect().contains( event_->pos( )))
    {
        event_->ignore();
        return;
    }

    contentWindow_->toggleSelectedState();

    QGraphicsItem::mouseDoubleClickEvent( event_ );
}

void ContentWindowGraphicsItem::mouseReleaseEvent( QGraphicsSceneMouseEvent* event_ )
{
    if( contentWindow_->isSelected( ))
    {
        contentWindow_->getInteractionDelegate().mouseReleaseEvent( event_ );
        return;
    }

    contentWindow_->setState( ContentWindow::NONE );

    QGraphicsItem::mouseReleaseEvent( event_ );
}

void ContentWindowGraphicsItem::wheelEvent( QGraphicsSceneWheelEvent* event_ )
{
    // on Mac we've seen that mouse events can go to the wrong graphics item
    // this is due to the bug: https://bugreports.qt.nokia.com/browse/QTBUG-20493
    // here we ignore the event if it shouldn't have been sent to us, which ensures
    // it will go to the correct item...
    if( !boundingRect().contains( event_->pos( )))
    {
        event_->ignore();
        return;
    }

    if ( contentWindow_->isSelected( ))
        contentWindow_->getInteractionDelegate().wheelEvent( event_ );
    else
        controller_.scale( event_->pos(), 1.0 + WHEEL_SCALE_SPEED *
                           (double)event_->delta() / STD_WHEEL_DELTA );
    update();
}

void ContentWindowGraphicsItem::keyPressEvent( QKeyEvent* event_ )
{
    if( contentWindow_->isSelected( ))
        contentWindow_->getInteractionDelegate().keyPressEvent( event_ );
}

void ContentWindowGraphicsItem::keyReleaseEvent( QKeyEvent* event_ )
{
    if( contentWindow_->isSelected( ))
        contentWindow_->getInteractionDelegate().keyReleaseEvent( event_ );
}

void ContentWindowGraphicsItem::gestureEvent( QGestureEvent* event_ )
{
    QGesture* gesture = 0;

    if( ( gesture = event_->gesture( Qt::TapAndHoldGesture )))
    {
        event_->accept( Qt::TapAndHoldGesture );
        tapAndHold( static_cast< QTapAndHoldGesture* >( gesture ));
        return;
    }

    if( contentWindow_->isSelected( ))
    {
        contentWindow_->getInteractionDelegate().gestureEvent( event_ );
        return;
    }

    if( ( gesture = event_->gesture( PanGestureRecognizer::type( ))))
    {
        event_->accept( PanGestureRecognizer::type( ));
        pan( static_cast< PanGesture* >( gesture ));
    }
    else if( ( gesture = event_->gesture( PinchGestureRecognizer::type( ))))
    {
        event_->accept( PinchGestureRecognizer::type( ));
        pinch( static_cast< PinchGesture* >( gesture ));
    }
    else if( ( gesture = event_->gesture( DoubleTapGestureRecognizer::type( ))))
    {
        event_->accept( DoubleTapGestureRecognizer::type( ));
        doubleTap( static_cast< DoubleTapGesture* >( gesture ));
    }
}

void ContentWindowGraphicsItem::doubleTap( DoubleTapGesture* gesture )
{
    if( gesture->state() == Qt::GestureFinished )
        controller_.toggleFullscreen();
}

void ContentWindowGraphicsItem::pan( PanGesture* gesture )
{
    if( gesture->state() == Qt::GestureStarted )
        contentWindow_->setState( ContentWindow::MOVING );

    else if( gesture->state() == Qt::GestureCanceled ||
             gesture->state() == Qt::GestureFinished )
        contentWindow_->setState( ContentWindow::NONE );

    const QPointF& windowPos = contentWindow_->getCoordinates().topLeft();
    controller_.moveTo( windowPos + gesture->delta( ));
}

void ContentWindowGraphicsItem::pinch( PinchGesture* gesture )
{
    const double factor =
             ZoomInteractionDelegate::adaptZoomFactor( gesture->scaleFactor( ));
    if( factor == 0.0 )
        return;

    if( gesture->state() == Qt::GestureStarted )
        contentWindow_->setState( ContentWindow::RESIZING );

    else if( gesture->state() == Qt::GestureCanceled ||
             gesture->state() == Qt::GestureFinished )
    {
        contentWindow_->setState( ContentWindow::NONE );
    }

    controller_.scale( gesture->position(), factor );
}

void ContentWindowGraphicsItem::tapAndHold( QTapAndHoldGesture* gesture )
{
    if( gesture->state() == Qt::GestureFinished )
        contentWindow_->toggleSelectedState();
}

bool ContentWindowGraphicsItem::isMovie() const
{
    return contentWindow_->getContent()->getType() == CONTENT_TYPE_MOVIE;
}

QSizeF ContentWindowGraphicsItem::getButtonDimensions() const
{
    const qreal size = std::min( BUTTON_REL_WIDTH * boundingRect().width(),
                                 BUTTON_REL_HEIGHT * boundingRect().height( ));
    return QSizeF( size, size );
}

QRectF ContentWindowGraphicsItem::getCloseRect() const
{
    const QSizeF button = getButtonDimensions();
    const QRectF coord = boundingRect();

    return QRectF( coord.x() + coord.width() - button.width(),
                   coord.y(), button.width(), button.height( ));
}

QRectF ContentWindowGraphicsItem::getResizeRect() const
{
    const QSizeF button = getButtonDimensions();
    const QRectF coord = boundingRect();

    return QRectF( coord.x() + coord.width() - button.width(),
                   coord.y() + coord.height() - button.height(),
                   button.width(), button.height( ));
}

QRectF ContentWindowGraphicsItem::getFullscreenRect() const
{
    const QSizeF button = getButtonDimensions();
    const QRectF coord = boundingRect();

    return QRectF( coord.x(), coord.y() + coord.height() - button.height(),
                   button.width(), button.height( ));
}

QRectF ContentWindowGraphicsItem::getPauseRect() const
{
    const QSizeF button = getButtonDimensions();
    const QRectF coord = boundingRect();

    return QRectF( coord.x() + 0.5 * coord.width() - button.width(),
                   coord.y() + coord.height() - button.height(),
                   button.width(), button.height() );
}

QRectF ContentWindowGraphicsItem::getLoopRect() const
{
    const QSizeF button = getButtonDimensions();
    const QRectF coord = boundingRect();

    return QRectF( coord.x() + 0.5 * coord.width(),
                   coord.y() + coord.height() - button.height(),
                   button.width(), button.height() );
}

void ContentWindowGraphicsItem::drawCloseButton_( QPainter* painter )
{
    const QRectF closeRect = getCloseRect();
    QPen pen;
    pen.setColor( QColor( Qt::red ));
    painter->setPen( pen );
    painter->drawRect( closeRect );
    painter->drawLine( closeRect.topLeft(), closeRect.bottomRight( ));
    painter->drawLine( closeRect.topRight(), closeRect.bottomLeft( ));
}

void ContentWindowGraphicsItem::drawResizeIndicator_( QPainter* painter )
{
    const QRectF resizeRect = getResizeRect();
    QPen pen;
    pen.setColor( Qt::gray );
    painter->setPen( pen );
    painter->drawRect( resizeRect );
    painter->drawLine( resizeRect.bottomLeft(), resizeRect.topRight( ));
}

void ContentWindowGraphicsItem::drawFullscreenButton_( QPainter* painter )
{
    const QRectF fullscreenRect = getFullscreenRect();
    QPen pen;
    pen.setColor( QColor( Qt::gray ));
    painter->setPen( pen );
    painter->drawRect( fullscreenRect );
}

void ContentWindowGraphicsItem::drawMovieControls_( QPainter* painter )
{
    if( contentWindow_->getContent()->getType() != CONTENT_TYPE_MOVIE )
        return;

    ContentPtr content = contentWindow_->getContent();
    MovieContent* movie = static_cast< MovieContent* >( content.get( ));
    const ControlState controlState = movie->getControlState();

    QPen pen;
    const QRectF pauseRect = getPauseRect();
    pen.setColor( QColor( controlState & STATE_PAUSED ? 128 : 200, 0, 0 ));
    painter->setPen( pen );
    painter->fillRect( pauseRect, pen.color( ));

    const QRectF loopRect = getLoopRect();
    pen.setColor( QColor( 0, controlState & STATE_LOOP ? 200 : 128, 0 ));
    painter->setPen( pen );
    painter->fillRect( loopRect, pen.color( ));
}

void ContentWindowGraphicsItem::drawTextLabel_( QPainter* painter )
{
    const QSizeF button = getButtonDimensions();

    const QString label( contentWindow_->getContent()->getURI( ));
    const QString labelSection = label.section( "/", -1, -1 ).prepend( " " );

    QFont font;
    font.setPixelSize( TEXT_LABEL_FONT_SIZE_REL_TO_BUTTON_SIZE *
                       button.height( ));
    painter->setFont( font );

    QPen pen;
    pen.setColor( QColor( Qt::black ));
    painter->setPen( pen );

    QRectF textBoundingRect = boundingRect();
    textBoundingRect.setWidth( textBoundingRect.width() - button.width() );

    painter->drawText( textBoundingRect, Qt::AlignLeft | Qt::AlignTop,
                       labelSection );
}

void ContentWindowGraphicsItem::drawWindowInfo_( QPainter* painter )
{
    const QRectF coord = boundingRect();

    const QString coordinatesLabel = QString(" (") +
                                     QString::number( coord.x(), 'f', 2 ) +
                                     QString(" ,") +
                                     QString::number( coord.y(), 'f', 2 ) +
                                     QString(", ") +
                                     QString::number( coord.width(), 'f', 2 ) +
                                     QString(", ") +
                                     QString::number( coord.height(), 'f', 2 ) +
                                     QString(")\n");

    const qreal zoom = contentWindow_->getZoomRect().width();
    const QPointF& zoomCenter = contentWindow_->getZoomRect().center();
    const QString zoomCenterLabel = QString(" zoom = ") +
                                    QString::number( zoom, 'f', 2 ) +
                                    QString(" @ (") +
                                    QString::number( zoomCenter.x(), 'f', 2 ) +
                                    QString(", ") +
                                    QString::number( zoomCenter.y(), 'f', 2 ) +
                                    QString(")");

    const QString windowInfoLabel = coordinatesLabel + zoomCenterLabel;

    const QSizeF button = getButtonDimensions();
    const QRectF textBoundingRect( coord.x() + button.width(),
                                   coord.y(),
                                   coord.width() - 2.0 * button.width(),
                                   coord.height( ));
    QPen pen;
    pen.setColor( QColor( Qt::black ));
    painter->setPen( pen );

    QFont font;
    font.setPixelSize( WINDOW_INFO_FONT_SIZE_REL_TO_BUTTON_SIZE *
                       button.height( ));
    painter->setFont( font );
    painter->drawText( textBoundingRect, Qt::AlignLeft | Qt::AlignBottom,
                       windowInfoLabel );
}

void ContentWindowGraphicsItem::drawFrame_( QPainter* painter )
{
    QPen pen;
    if( contentWindow_->isSelected( ))
        pen.setColor( QColor( Qt::red ));
    else
        pen.setColor( QColor( Qt::black ));

    painter->setPen( pen );
    painter->setBrush( QBrush( QColor( 0, 0, 0, 128 )));
    painter->drawRect( boundingRect( ));
}
