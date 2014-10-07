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

#ifndef CONTENT_WINDOW_GRAPHICS_ITEM_H
#define CONTENT_WINDOW_GRAPHICS_ITEM_H

#include "types.h"

#include <QtGui/QGraphicsObject>

/**
 * Represent a ContentWindow in a QListView.
 */
class ContentWindowGraphicsItem : public QGraphicsObject
{
    Q_OBJECT

public:
    /** Constructor. */
    explicit ContentWindowGraphicsItem( ContentWindowPtr contentWindow );

    /** Destructor. */
    virtual ~ContentWindowGraphicsItem();

    /** Get the associated ContentWindow. */
    ContentWindowPtr getContentWindow() const;

public slots:
    /** QGraphicsRectItem paint event */
    void paint( QPainter* painter, const QStyleOptionGraphicsItem* option,
                QWidget* widget = 0 ) override;

    /** Move this item to the front. */
    void setZToFront();

private slots:
    /** Used to trigger prepareGeometryChange() when window dimensions change */
    void prepareToChangeGeometry();

signals:
    /** Emitted when a user clicks the window to bring it to the front. */
    void moveToFront( ContentWindowPtr contentWindow );

    /** Emitted when the user clicks the close button. */
    void close( ContentWindowPtr contentWindow );

protected:
    /** Get normalized window dimensions. */
    QRectF boundingRect() const override;

    /** @name Re-implemented QGraphicsRectItem events */
    //@{
    bool sceneEvent( QEvent* event ) override;
    void mouseMoveEvent( QGraphicsSceneMouseEvent* event ) override;
    void mousePressEvent( QGraphicsSceneMouseEvent* event ) override;
    void mouseDoubleClickEvent( QGraphicsSceneMouseEvent* event ) override;
    void mouseReleaseEvent( QGraphicsSceneMouseEvent* event ) override;
    void wheelEvent( QGraphicsSceneWheelEvent* event ) override;
    void keyPressEvent( QKeyEvent* event ) override;
    void keyReleaseEvent( QKeyEvent* event ) override;
    //@}

private:
    void getButtonDimensions( float &width, float &height ) const;

    bool hitCloseButton( const QPointF& hitPos ) const;
    bool hitResizeButton( const QPointF& hitPos ) const;
    bool hitFullscreenButton( const QPointF& hitPos ) const;
    bool hitPauseButton( const QPointF& hitPos ) const;
    bool hitLoopButton( const QPointF& hitPos ) const;

    void drawFrame_( QPainter* painter );
    void drawCloseButton_( QPainter* painter );
    void drawResizeIndicator_( QPainter* painter );
    void drawFullscreenButton_( QPainter* painter );
    void drawMovieControls_( QPainter* painter );
    void drawTextLabel_( QPainter* painter );

    ContentWindowPtr contentWindow_;
    bool resizing_;
    bool moving_;

    // counter used to determine stacking order in the UI
    static qreal zCounter_;
};

#endif
