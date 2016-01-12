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

#ifndef WALLWINDOW_H
#define WALLWINDOW_H

#include "types.h"

#include "WallScene.h"

#include <QQuickView>
#include <QOpenGLContext>

class WallWindow : public QQuickView
{
    Q_OBJECT

public:
    /**
     * Create a wall window.
     * @param the area of the wall that this window shows
     */
    WallWindow( const QRect& wallArea );

    /** Get the scene that this window renders. */
    WallScene& getScene();

    /** Create the scene, must happen after the setSource() or setContent(). */
    void createScene( const QPoint& pos );

    /** Set the test pattern. */
    void setTestPattern( TestPatternPtr testPattern );

    /** Get the test pattern */
    TestPatternPtr getTestPattern();

    /** Block all the update() and repaint() calls. */
    void setBlockDrawCalls( bool enable );

    /** Update and synchronize scene objects before rendering a frame. */
    void preRenderUpdate( WallToWallChannel& wallChannel );

    /** Set new render options. */
    void setRenderOptions( OptionsPtr options );

    /** Set new display group. */
    void setDisplayGroup( DisplayGroupPtr displayGroup );

    /** Set new touchpoint's markers. */
    void setMarkers( MarkersPtr markers );

    /** @return the pixel stream provider. */
    PixelStreamProvider& getPixelStreamProvider();

//    /** Check if the window is exposed in the window system. */
//    bool isExposed() const;

private:
//    /** Reimplemented from QGraphicsView to draw the test pattern */
//    void drawForeground( QPainter* painter, const QRectF& rect ) override;

//    /** Reimplemented from QGraphicsView to block unsolicited draw calls. */
//    void paintEvent( QPaintEvent* event ) override;

    const QRect _wallArea;
    std::unique_ptr<WallScene> _scene;
    TestPatternPtr _testPattern;
    bool _blockUpdates;
    bool _isExposed;
};

#endif // WALLWINDOW_H
