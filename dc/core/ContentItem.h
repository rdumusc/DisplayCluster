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

#ifndef CONTENTITEM_H
#define CONTENTITEM_H

#include "types.h"

#include <QQuickPaintedItem>

/**
 * Render a Content in Qml.
 */
class ContentItem : public QQuickPaintedItem
{
    Q_OBJECT
    Q_PROPERTY( Role role READ getRole WRITE setRole NOTIFY roleChanged )

public:
    enum Role
    {
        ROLE_CONTENT,
        ROLE_PREVIEW
    };
    Q_ENUMS( Role )

    /** Constructor. */
    explicit ContentItem( QQuickItem* parentItem = 0  );

    /** Draw call, reimplemented from QQuickPaintedItem. */
    void paint( QPainter* painter ) override;

    /** Set the factory object to be rendered. */
    void setWallContent( WallContent* wallContent );

    /** Get the role of this item. */
    Role getRole() const;

    /** Get the scene coordinates of the item. */
    QRectF getSceneRect() const;

    /** Check if the item's size or position is being animated from QML. */
    bool isAnimating() const;

public slots:
    void setRole( Role arg );

signals:
    void roleChanged( Role arg );

private:
    WallContent* wallContent_;
    Role role_;
};

#endif // CONTENTITEM_H
