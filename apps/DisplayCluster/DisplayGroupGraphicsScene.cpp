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

#include "configuration/Configuration.h"

#include <QtGui/QKeyEvent>

DisplayGroupGraphicsScene::DisplayGroupGraphicsScene( const Configuration& config,
                                                      QObject* parent_ )
    : QGraphicsScene( parent_ )
{
    setSceneRect( QRectF( QPointF( 0.0, 0.0 ), config.getTotalSize( )));

    for( int i = 0; i < config.getTotalScreenCountX(); ++i )
        for( int j = 0; j < config.getTotalScreenCountY(); ++j )
            screens_.push_back( config.getScreenRect( QPoint( i, j )));

    addBackgroundRectangles();
}

void DisplayGroupGraphicsScene::clearAndRestoreBackground()
{
    clear();
    addBackgroundRectangles();
}

bool DisplayGroupGraphicsScene::event( QEvent *evt )
{
    switch( evt->type( ))
    {
    case QEvent::KeyPress:
    {
        QKeyEvent* k = static_cast< QKeyEvent* >( evt );

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

void DisplayGroupGraphicsScene::addBackgroundRectangles()
{
    addRect( sceneRect( )); // Background rectangle for the wall area

    const QPen pen( QColor( Qt::black ));       // screen border
    const QBrush brush( QColor( 0, 0, 0, 32 )); // screen fill color

    foreach( const QRectF& screen, screens_ )
        addRect( screen, pen, brush );
}
