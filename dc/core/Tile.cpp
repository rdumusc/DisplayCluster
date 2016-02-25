/*********************************************************************/
/* Copyright (c) 2016, EPFL/Blue Brain Project                       */
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

#include "Tile.h"

#include "TextureNode.h"

// false-positive on qt signals for Q_PROPERTY notifiers
// cppcheck-suppress uninitMemberVar
Tile::Tile( const uint index, const QRect& rect )
    : _index( index )
    , _swap( false )
    , _backGlTexture( 0 )
{
    setFlag( ItemHasContents, true );
    setVisible( false );
    update( rect );
}

uint Tile::getIndex() const
{
    return _index;
}

void Tile::update( const QRect& rect )
{
    if( rect == getCoord( ))
        return;

    setVisible( false );
    setPosition( rect.topLeft( ));

    if( rect.size() != QSize( width(), height( )))
    {
        setImplicitSize( rect.width(), rect.height( ));
        setSize( rect.size( ));
        QQuickItem::update();
    }
}

QRect Tile::getCoord() const
{
    return QRect( x(), y(), width(), height( ));
}

uint Tile::getBackGlTexture() const
{
    return _backGlTexture;
}

void Tile::swapImage()
{
    _swap = true;
    if( !isVisible( ))
        setVisible( true );
    QQuickItem::update();
}

void Tile::markBackTextureUpdated()
{
    emit readyToSwap( shared_from_this( ));
}

QSGNode* Tile::updatePaintNode( QSGNode* oldNode,
                                QQuickItem::UpdatePaintNodeData* )
{
    TextureNode* node = static_cast<TextureNode*>( oldNode );

    if( !node )
    {
        node = new TextureNode( QSize( width(), height( )), window( ));
        _backGlTexture = node->getBackGlTexture();
        emit textureInitialized( shared_from_this( ));
    }

    node->resize( QSize( width(), height( )));

    if( _swap )
    {
        node->swap();
        _backGlTexture = node->getBackGlTexture();
        _swap = false;
    }

    return node;
}
