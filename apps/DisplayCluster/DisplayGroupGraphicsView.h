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

#ifndef DISPLAY_GROUP_GRAPHICS_VIEW_H
#define DISPLAY_GROUP_GRAPHICS_VIEW_H

#include "types.h"

#include <QtGui/QGraphicsView>
#include <QtGui/QGesture>
#include <QtGui/QGestureEvent>

class PanGesture;
class PinchGesture;

/**
 * An interactive graphical view of a DisplayGroup's ContentWindows.
 */
class DisplayGroupGraphicsView : public QGraphicsView
{
    Q_OBJECT

public:
    /** Constructor. */
    DisplayGroupGraphicsView( QWidget* parent = 0 );

    /** Destructor */
    virtual ~DisplayGroupGraphicsView();

    /** Set the DisplayGroup model that this view should present. */
    void setModel( DisplayGroupPtr displayGroup );

signals:
    /** Emitted when a user taps the background. */
    void backgroundTap( QPointF pos );

    /** Emitted when a user taps and holds the background. */
    void backgroundTapAndHold( QPointF pos );

protected:
    /** @name Re-implemented QGraphicsView events */
    //@{
    bool viewportEvent( QEvent* event ) override;
    void resizeEvent( QResizeEvent* event ) override;
    //@}

private slots:
    void addContentWindow( ContentWindowPtr contentWindow );
    void removeContentWindow( ContentWindowPtr contentWindow );
    void moveContentWindowToFront( ContentWindowPtr contentWindow );

private:
    void gestureEvent( QGestureEvent* event );
    void swipe( QSwipeGesture* gesture );
    void pan( PanGesture* gesture );
    void pinch( PinchGesture* gesture );
    void tap( QTapGesture* gesture );
    void tapAndHold( QTapAndHoldGesture* gesture );

    void grabGestures();
    QPointF getNormalizedPosition( const QGesture* gesture ) const;
    bool isOnBackground( const QPointF& position ) const;

    DisplayGroupPtr displayGroup_;
};

#endif
