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

#ifndef TILE_H
#define TILE_H

#include "types.h"

#include <QQuickItem>
#include <memory> // std::enable_shared_from_this

class TextureNode;

/**
 * Qml item to render an image tile with texture double-buffering.
 */
class Tile : public QQuickItem, public std::enable_shared_from_this<Tile>
{
    Q_OBJECT
    Q_DISABLE_COPY( Tile )

    Q_PROPERTY( uint index READ getIndex CONSTANT )

public:
    Tile( const uint index, const QRect& rect );

    uint getIndex() const;

    void update( const QRect& rect );

    uint getBackGlTexture() const;
    QSize getBackGlTextureSize() const;

signals:
    /**
     * Notifies that the back texture is ready to be updated.
     * It is emitted after the texture has been created on the render thread,
     * or after a call to update().
     */
    void textureReady( TilePtr tile );

    /** Notifier for the DoubleBufferedImage to swap the texture/image. */
    void textureUpdated( TilePtr tile );

public slots:
    /** Swap the front and back texture. */
    void swapImage();

    /** Indicate that the back GL texture has been externally updated. */
    void markBackTextureUpdated();

protected:
    /** Called on the render thread to update the scene graph. */
    QSGNode* updatePaintNode( QSGNode* oldNode, UpdatePaintNodeData* ) override;

private:
    uint _index;

    bool _swap;
    bool _resize;
    QRect _nextCoord;
    bool _updateBackTexture;
    uint _backGlTexture;
};

#endif
