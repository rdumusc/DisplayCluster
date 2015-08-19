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

#ifndef CONTENTWINDOWCONTROLLER_H
#define CONTENTWINDOWCONTROLLER_H

#include "types.h"

#include <QtCore/QObject>
#include <QtCore/QRectF>

#include <boost/serialization/access.hpp>

/** Common window size states. */
enum SizeState
{
    SIZE_1TO1,
    SIZE_LARGE,
    SIZE_FULLSCREEN
};

/** Window point, used for affine transforms of the window. */
enum WindowPoint
{
    TOP_LEFT,
    CENTER
};

/**
 * Controller for moving and resizing windows.
 */
class ContentWindowController : public QObject
{
    Q_OBJECT

public:
    ContentWindowController( ContentWindow& contentWindow,
                             const DisplayGroup& displayGroup );

    /** Resize the window. */
    Q_INVOKABLE void resize( QSizeF size, WindowPoint fixedPoint = TOP_LEFT );

    /** Resize the window relative to the current active window border */
    Q_INVOKABLE void resizeRelative( const QPointF& delta );

    /** Scale the window by the given factor (around the given center). */
    Q_INVOKABLE void scale( const QPointF& center, double factor );

    /** Adjust the window coordinates to match the desired state. */
    void adjustSize( const SizeState state );

    /** Adjust the window coordinates to match the Content dimensions. */
    Q_INVOKABLE void adjustSizeOneToOne() { adjustSize( SIZE_1TO1 ); }

    /** Move the window to the desired position. */
    Q_INVOKABLE void moveTo( const QPointF& position,
                             WindowPoint handle = TOP_LEFT );

    /** Move the center of the window to the desired position. */
    inline void moveCenterTo( const QPointF& position )
    {
        moveTo( position, CENTER );
    }

    /** @return the minimum size of the window, 5% of wall size or 300px. */
    QSizeF getMinSize() const;

    /** @return the maximum size of the window, considering max size of content,
     *          wall size and current content zoom. */
    QSizeF getMaxSize() const;

    /** @return the maximum size of the content, considering wall size. */
    QSizeF getMaxContentSize() const;

    /** @return the focused coordinates for the window. */
    Q_INVOKABLE QRectF getFocusedCoord() const;

private:
    /**
     * Resize the window around a given center point.
     * @param center the center of scaling
     * @param size the new desired size
     */
    void resize_( const QPointF& center, QSizeF size );

    void constrainSize_( QSizeF& windowSize ) const;
    void constrainPosition_( QRectF& window ) const;
    void constrainFullyInside_( QRectF& window ) const;
    QRectF getCenteredCoordinates_( const QSizeF& size ) const;
    qreal getInsideMargin_() const;

    friend class boost::serialization::access;

    /** Serialize for sending to Wall applications. */
    template< class Archive >
    void serialize( Archive & ar, const unsigned int )
    {
        ar & contentWindow_;
        ar & displayGroup_;
    }

    /** No-argument constructor required for serialization. */
    ContentWindowController();

    // Storing pointers because references cannot be serialized with boost
    ContentWindow* contentWindow_;
    const DisplayGroup* displayGroup_;
};

#endif // CONTENTWINDOWCONTROLLER_H
