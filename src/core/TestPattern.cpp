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

#include "TestPattern.h"

#include "RenderContext.h"
#include "configuration/WallConfiguration.h"

#include <QFont>
#include <QtOpenGL/qgl.h>

#define FONT_SIZE   24
#define LINE_WIDTH  10
#define TEXT_POS_X  50

TestPattern::TestPattern( const WallConfiguration& configuration,
                          const int tileIndex )
    : wallSize_( configuration.getTotalSize( ))
{
    setVisible( false );
    const QPoint globalScreenIndex = configuration.getGlobalScreenIndex( tileIndex );
    const QString fullsceenMode = configuration.getFullscreen() ? "True" : "False";

    labels_.push_back(QString("Rank: %1").arg(configuration.getProcessIndex()));
    labels_.push_back(QString("Host: %1").arg(configuration.getHost()));
    labels_.push_back(QString("Display: %1").arg(configuration.getDisplay()));
    labels_.push_back(QString("Tile coordinates: (%1,%2)").arg(globalScreenIndex.x()).arg(globalScreenIndex.y()));
    labels_.push_back(QString("Resolution: %1 x %2").arg(configuration.getScreenWidth()).arg(configuration.getScreenHeight()));
    labels_.push_back(QString("Fullscreen mode: %1").arg(fullsceenMode));
}

void TestPattern::draw( QPainter* painter, const QRectF& rect )
{
    renderCrossPattern( painter );
    renderLabels( painter, rect );
}

void TestPattern::renderCrossPattern( QPainter* painter )
{
    const qreal height = wallSize_.height();
    const qreal width = wallSize_.width();

    QPen pen;
    pen.setWidth( LINE_WIDTH );

    for( qreal y = -1.0 * height; y <= 2.0 * height; y += 0.1 * height )
    {
        const qreal hue = (y + height) / (3.0 * height);
        pen.setColor( QColor::fromHsvF( hue, 1.0, 1.0 ));
        painter->setPen( pen );
        painter->drawLine( QPointF( 0.0, y ), QPointF( width, y + height ));
        painter->drawLine( QPointF( 0.0, y ), QPointF( width, y - height ));
    }
}

void TestPattern::renderLabels( QPainter* painter, const QRectF& rect )
{
    const QPoint offset = rect.topLeft( ).toPoint();

    QFont textFont;
    textFont.setPixelSize( FONT_SIZE );
    painter->setFont( textFont );
    painter->setPen( QColor( Qt::white ));

    unsigned int pos = 0;
    foreach( QString label, labels_ )
        painter->drawText( QPoint( TEXT_POS_X, ++pos * FONT_SIZE ) + offset,
                           label );
}
