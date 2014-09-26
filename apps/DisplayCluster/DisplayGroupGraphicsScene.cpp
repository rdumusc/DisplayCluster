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

#include "DisplayGroupGraphicsScene.h"

#include "globals.h"
#include "configuration/Configuration.h"

#include <QtGui/QGraphicsRectItem>
#include <QtGui/QKeyEvent>
#include <QtGui/QGraphicsSceneMouseEvent>

DisplayGroupGraphicsScene::DisplayGroupGraphicsScene( QObject* parent_ )
    : QGraphicsScene( parent_ )
{
    setSceneRect(0., 0., 1., 1.);
    refreshTileRects();
}

void DisplayGroupGraphicsScene::refreshTileRects()
{
    // clear existing tile rects
    for( TileRectItems::iterator it = tileRects_.begin(); it != tileRects_.end(); ++it )
        delete *it;
    tileRects_.clear();

    // Add rectangle for the wall area
    tileRects_.push_back( addRect( 0., 0., 1., 1. ));

    // Add 1 rectangle for each monitor
    const int numTilesX = g_configuration->getTotalScreenCountX();
    const int numTilesY = g_configuration->getTotalScreenCountY();

    const QPen pen( QColor( 0, 0, 0 )); // border
    const QBrush brush( QColor( 0, 0, 0, 32 )); // fill color / opacity

    for( int i=0; i<numTilesX; ++i )
    {
        for( int j=0; j<numTilesY; ++j )
        {
            const QPoint tileIndex( i, j );
            const QRectF screen = g_configuration->getNormalizedScreenRect( tileIndex );

            tileRects_.push_back( addRect( screen, pen, brush ));
        }
    }
}

bool DisplayGroupGraphicsScene::event( QEvent *evt )
{
    switch( evt->type( ))
    {
    case QEvent::KeyPress:
    {
        QKeyEvent *k = static_cast< QKeyEvent* >( evt );

        // Override default behaviour to process TAB key events
        QGraphicsScene::keyPressEvent( k );

        if( k->key() == Qt::Key_Backtab ||
            k->key() == Qt::Key_Tab ||
           ( k->key() == Qt::Key_Tab && ( k->modifiers() & Qt::ShiftModifier )))
        {
            evt->accept();
        }
        return true;
    }
    default:
        return QGraphicsScene::event( evt );
    }
}

void DisplayGroupGraphicsScene::mouseMoveEvent( QGraphicsSceneMouseEvent* mouseEvent )
{
    QGraphicsScene::mouseMoveEvent( mouseEvent );
}

void DisplayGroupGraphicsScene::mousePressEvent( QGraphicsSceneMouseEvent* mouseEvent )
{
    QGraphicsScene::mousePressEvent( mouseEvent );
}

void DisplayGroupGraphicsScene::mouseReleaseEvent( QGraphicsSceneMouseEvent* mouseEvent )
{
    QGraphicsScene::mouseReleaseEvent( mouseEvent );
}
