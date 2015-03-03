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

#include "WallWindow.h"

#include "TestPattern.h"

WallWindow::WallWindow( QGraphicsScene* scene_, const QRect& sceneRect_,
                        const QPoint& windowPos )
    : QGraphicsView( scene_ )
{
    QRect windowGeometry( sceneRect_ );
    if( !windowPos.isNull( ))
        windowGeometry.moveTopLeft( windowPos );

    setGeometry( windowGeometry );
    setSceneRect( sceneRect_ );

    setCacheMode( QGraphicsView::CacheNone );
    setViewportUpdateMode( QGraphicsView::FullViewportUpdate );
    setAttribute( Qt::WA_TranslucentBackground );
    setStyleSheet( "border: 0px" );

    setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
    setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
}

void WallWindow::setTestPattern( TestPatternPtr testPattern )
{
    testPattern_ = testPattern;
}

TestPatternPtr WallWindow::getTestPattern()
{
    return testPattern_;
}

void WallWindow::drawForeground( QPainter* painter, const QRectF& rect_ )
{
    if( testPattern_ && testPattern_->isVisible( ))
    {
        testPattern_->draw( painter, rect_ );
        return;
    }

    if( fpsRenderer_.isVisible( ))
        fpsRenderer_.draw( painter, rect_ );

    QGraphicsView::drawForeground( painter, rect_ );
}

void WallWindow::setShowFps( const bool value )
{
    fpsRenderer_.setVisible( value );
}
