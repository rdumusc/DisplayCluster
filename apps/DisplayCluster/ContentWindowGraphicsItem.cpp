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
#include "ContentInteractionDelegate.h"

#include "globals.h"
#include "configuration/Configuration.h"

#include "gestures/DoubleTapGestureRecognizer.h"
#include "gestures/PanGestureRecognizer.h"
#include "gestures/PinchGestureRecognizer.h"

#include <QtGui/QPainter>
#include <QtGui/QGraphicsScene>
#include <QtGui/QGraphicsView>

#include <QEvent>
#include <QGestureEvent>

#define STD_WHEEL_DELTA 120 // Common value for the delta of mouse wheel events

qreal ContentWindowGraphicsItem::zCounter_ = 0;

ContentWindowGraphicsItem::ContentWindowGraphicsItem( ContentWindowPtr contentWindow )
    : contentWindow_( contentWindow )
    , resizing_( false )
    , moving_( false )
{
    assert( contentWindow_ );

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
    drawFrame_( painter );
    drawCloseButton_( painter );
    drawResizeIndicator_( painter );
    drawFullscreenButton_( painter );
    drawMovieControls_( painter );
    drawTextLabel_( painter );
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
    switch( event_->type( ))
    {
    case QEvent::Gesture:
        emit moveToFront( contentWindow_ );
        contentWindow_->getInteractionDelegate().gestureEvent( static_cast< QGestureEvent* >( event_ ));
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
    if( contentWindow_->selected( ))
    {
        contentWindow_->getInteractionDelegate().mouseMoveEvent( event_ );
        return;
    }

    if( event_->buttons().testFlag( Qt::LeftButton ))
    {
        if( resizing_ )
        {
            QRectF coordinates = boundingRect();
            coordinates.setBottomRight( event_->pos( ));

            float targetAR = contentWindow_->getContent()->getAspectRatio();
            targetAR /= g_configuration->getAspectRatio();

            const float eventCoordAR = coordinates.width() / coordinates.height();
            if( eventCoordAR < targetAR )
                contentWindow_->setSize( coordinates.width(),
                                         coordinates.width() / targetAR );
            else
                contentWindow_->setSize( coordinates.height() * targetAR,
                                         coordinates.height( ));
        }
        else
        {
            const QPointF delta = event_->pos() - event_->lastPos();

            const double new_x = boundingRect().x() + delta.x();
            const double new_y = boundingRect().y() + delta.y();

            contentWindow_->setPosition( new_x, new_y );
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

    if( hitCloseButton( event_->pos( )))
    {
        emit close( contentWindow_ );
        return;
    }

    emit moveToFront( contentWindow_ );

    if ( contentWindow_->selected( ))
    {
        contentWindow_->getInteractionDelegate().mousePressEvent( event_ );
        return;
    }

    contentWindow_->getContent()->blockAdvance( true );

    if( hitResizeButton( event_->pos( )))
        resizing_ = true;

    else if( hitFullscreenButton( event_->pos( )))
        contentWindow_->toggleFullscreen();

    else if( hitPauseButton( event_->pos( )))
        contentWindow_->setControlState( ControlState( contentWindow_->getControlState() ^ STATE_PAUSED ));

    else if( hitLoopButton( event_->pos( )))
        contentWindow_->setControlState( ControlState( contentWindow_->getControlState() ^ STATE_LOOP ));

    else
        moving_ = true;

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

    contentWindow_->toggleWindowState();

    QGraphicsItem::mouseDoubleClickEvent( event_ );
}

void ContentWindowGraphicsItem::mouseReleaseEvent( QGraphicsSceneMouseEvent* event_ )
{
    resizing_ = false;
    moving_ = false;

    contentWindow_->getContent()->blockAdvance( false );

    if( contentWindow_->selected( ))
        contentWindow_->getInteractionDelegate().mouseReleaseEvent( event_ );

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

    if ( contentWindow_->selected( ))
        contentWindow_->getInteractionDelegate().wheelEvent( event_ );
    else
        contentWindow_->scaleSize( 1. + (double)event_->delta() / ( 10. * STD_WHEEL_DELTA ));
}

void ContentWindowGraphicsItem::keyPressEvent( QKeyEvent* event_ )
{
    if( contentWindow_->selected( ))
        contentWindow_->getInteractionDelegate().keyPressEvent( event_ );
}

void ContentWindowGraphicsItem::keyReleaseEvent( QKeyEvent* event_ )
{
    if( contentWindow_->selected( ))
        contentWindow_->getInteractionDelegate().keyReleaseEvent( event_ );
}

void ContentWindowGraphicsItem::getButtonDimensions( float& width, float& height ) const
{
    const float sceneHeightFraction = 0.125f;
    const double screenAspect = g_configuration->getAspectRatio();

    width = sceneHeightFraction / screenAspect;
    height = sceneHeightFraction;

    // clamp to half rect dimensions
    if( width > 0.5 * boundingRect().width( ))
        width = 0.49 * boundingRect().width();

    if( height > 0.5 * boundingRect().height( ))
        height = 0.49 * boundingRect().height();
}

bool ContentWindowGraphicsItem::hitCloseButton( const QPointF& hitPos ) const
{
    float buttonWidth, buttonHeight;
    getButtonDimensions( buttonWidth, buttonHeight );
    const QRectF r = boundingRect();

    return ( fabs( ( r.x() + r.width( )) - hitPos.x( )) <= buttonWidth &&
             fabs( r.y() - hitPos.y( )) <= buttonHeight );
}

bool ContentWindowGraphicsItem::hitResizeButton(const QPointF& hitPos ) const
{
    float buttonWidth, buttonHeight;
    getButtonDimensions( buttonWidth, buttonHeight );
    const QRectF r = boundingRect();

    return ( fabs( ( r.x() + r.width( )) - hitPos.x( )) <= buttonWidth &&
             fabs( ( r.y() + r.height( )) - hitPos.y( )) <= buttonHeight );
}

bool ContentWindowGraphicsItem::hitFullscreenButton(const QPointF& hitPos) const
{
    float buttonWidth, buttonHeight;
    getButtonDimensions( buttonWidth, buttonHeight );
    const QRectF r = boundingRect();

    return ( fabs( r.x() - hitPos.x( )) <= buttonWidth &&
             fabs( ( r.y() + r.height( )) - hitPos.y( )) <= buttonHeight );
}

bool ContentWindowGraphicsItem::hitPauseButton(const QPointF& hitPos) const
{
    float buttonWidth, buttonHeight;
    getButtonDimensions( buttonWidth, buttonHeight );
    const QRectF r = boundingRect();

    return ( fabs( ( ( r.x() + r.width( )) / 2) - hitPos.x() - buttonWidth ) <= buttonWidth &&
             fabs( ( r.y() + r.height( )) - hitPos.y( )) <= buttonHeight );
}

bool ContentWindowGraphicsItem::hitLoopButton(const QPointF& hitPos) const
{
    float buttonWidth, buttonHeight;
    getButtonDimensions( buttonWidth, buttonHeight );
    const QRectF r = boundingRect();

    return ( fabs( ( ( r.x() + r.width( )) / 2) - hitPos.x( )) <= buttonWidth &&
             fabs( ( r.y() + r.height( )) - hitPos.y( )) <= buttonHeight );
}

void ContentWindowGraphicsItem::drawCloseButton_( QPainter* painter )
{
    float buttonWidth, buttonHeight;
    getButtonDimensions( buttonWidth, buttonHeight );
    const QRectF coordinates = boundingRect();

    QRectF closeRect(coordinates.x() + coordinates.width() - buttonWidth,
                     coordinates.y(), buttonWidth, buttonHeight);
    QPen pen;
    pen.setColor(QColor(255,0,0));
    painter->setPen(pen);
    painter->drawRect(closeRect);
    painter->drawLine(QPointF(coordinates.x() + coordinates.width() - buttonWidth, coordinates.y()),
                      QPointF(coordinates.x() + coordinates.width(), coordinates.y() + buttonHeight));
    painter->drawLine(QPointF(coordinates.x() + coordinates.width(), coordinates.y()),
                      QPointF(coordinates.x() + coordinates.width() - buttonWidth, coordinates.y() + buttonHeight));
}

void ContentWindowGraphicsItem::drawResizeIndicator_( QPainter* painter )
{
    float buttonWidth, buttonHeight;
    getButtonDimensions( buttonWidth, buttonHeight );
    const QRectF coordinates = boundingRect();

    QRectF resizeRect(coordinates.x() + coordinates.width() - buttonWidth,
                      coordinates.y() + coordinates.height() - buttonHeight,
                      buttonWidth, buttonHeight);
    QPen pen;
    pen.setColor(QColor(128,128,128));
    painter->setPen(pen);
    painter->drawRect(resizeRect);
    painter->drawLine(QPointF(coordinates.x() + coordinates.width(),
                              coordinates.y() + coordinates.height() - buttonHeight),
                      QPointF(coordinates.x() + coordinates.width() - buttonWidth,
                              coordinates.y() + coordinates.height()));
}

void ContentWindowGraphicsItem::drawFullscreenButton_( QPainter* painter )
{
    float buttonWidth, buttonHeight;
    getButtonDimensions( buttonWidth, buttonHeight );
    const QRectF coordinates = boundingRect();

    QRectF fullscreenRect(coordinates.x(),
                          coordinates.y() + coordinates.height() - buttonHeight,
                          buttonWidth, buttonHeight);
    QPen pen;
    pen.setColor(QColor(128,128,128));
    painter->setPen(pen);
    painter->drawRect(fullscreenRect);
}

void ContentWindowGraphicsItem::drawMovieControls_( QPainter* painter )
{
    float buttonWidth, buttonHeight;
    getButtonDimensions( buttonWidth, buttonHeight );
    const QRectF coordinates = boundingRect();

    QPen pen;

    if( contentWindow_->getContent()->getType() == CONTENT_TYPE_MOVIE )
    {
        // play/pause
        QRectF playPauseRect(coordinates.x() + coordinates.width()/2 - buttonWidth,
                             coordinates.y() + coordinates.height() - buttonHeight,
                             buttonWidth, buttonHeight);
        pen.setColor(QColor(contentWindow_->getControlState() & STATE_PAUSED ? 128 :200,0,0));
        painter->setPen(pen);
        painter->fillRect(playPauseRect, pen.color());

        // loop
        QRectF loopRect(coordinates.x() + coordinates.width()/2,
                        coordinates.y() + coordinates.height() - buttonHeight,
                        buttonWidth, buttonHeight);
        pen.setColor(QColor(0,contentWindow_->getControlState() & STATE_LOOP ? 200 :128,0));
        painter->setPen(pen);
        painter->fillRect(loopRect, pen.color());
    }
}

void ContentWindowGraphicsItem::drawTextLabel_( QPainter* painter )
{
    float buttonWidth, buttonHeight;
    getButtonDimensions( buttonWidth, buttonHeight );
    const QRectF coordinates = boundingRect();

    const float fontSize = 24.;

    QFont font;
    font.setPixelSize(fontSize);
    painter->setFont(font);

    // color the text black
    QPen pen;
    pen.setColor(QColor(0,0,0));
    painter->setPen(pen);

    // scale the text size down to the height of the graphics view
    // and, calculate the bounding rectangle for the text based on this scale
    // the dimensions of the view need to be corrected for the tiled display aspect ratio
    // recall the tiled display UI is only part of the graphics view since we show it at the correct aspect ratio
    // TODO refactor this for clarity!
    float viewWidth = (float)scene()->views()[0]->width();
    float viewHeight = (float)scene()->views()[0]->height();

    const float tiledDisplayAspect = g_configuration->getAspectRatio();

    if(viewWidth / viewHeight > tiledDisplayAspect)
        viewWidth = viewHeight * tiledDisplayAspect;

    else if(viewWidth / viewHeight <= tiledDisplayAspect)
        viewHeight = viewWidth / tiledDisplayAspect;

    float verticalTextScale = 1. / viewHeight;
    float horizontalTextScale = viewHeight / viewWidth * verticalTextScale;

    painter->scale(horizontalTextScale, verticalTextScale);

    QRectF textBoundingRect = QRectF(coordinates.x() / horizontalTextScale,
                                     coordinates.y() / verticalTextScale,
                                     coordinates.width() / horizontalTextScale,
                                     coordinates.height() / verticalTextScale);

    // get the label and render it
    QString label(contentWindow_->getContent()->getURI());
    QString labelSection = label.section("/", -1, -1).prepend(" ");
    painter->drawText(textBoundingRect, Qt::AlignLeft | Qt::AlignTop, labelSection);

    // draw window info at smaller scale
    verticalTextScale *= 0.5;
    horizontalTextScale *= 0.5;

    painter->scale(0.5, 0.5);

    textBoundingRect = QRectF((coordinates.x()+buttonWidth) / horizontalTextScale,
                               coordinates.y() / verticalTextScale,
                              (coordinates.width()-buttonWidth) / horizontalTextScale,
                               coordinates.height() / verticalTextScale);

    QString coordinatesLabel = QString(" (") + QString::number(coordinates.x(), 'f', 2) + QString(" ,") +
                                               QString::number(coordinates.y(), 'f', 2) + QString(", ") +
                                               QString::number(coordinates.width(), 'f', 2) + QString(", ") +
                                               QString::number(coordinates.height(), 'f', 2) + QString(")\n");
    double centerX, centerY;
    contentWindow_->getCenter( centerX, centerY );
    QString zoomCenterLabel = QString(" zoom = ") + QString::number(contentWindow_->getZoom(), 'f', 2) + QString(" @ (") +
                              QString::number(centerX, 'f', 2) + QString(", ") +
                              QString::number(centerY, 'f', 2) + QString(")");

    QString windowInfoLabel = coordinatesLabel + zoomCenterLabel;
    painter->drawText(textBoundingRect, Qt::AlignLeft | Qt::AlignBottom, windowInfoLabel);
}

void ContentWindowGraphicsItem::drawFrame_( QPainter* painter )
{
    QPen pen;
    if( contentWindow_->selected( ))
        pen.setColor( QColor( 255, 0, 0 ));
    else
        pen.setColor( QColor( 0, 0, 0 ));

    painter->setPen( pen );
    painter->setBrush( QBrush( QColor( 0, 0, 0, 128 )));
    painter->drawRect( boundingRect( ));
}
