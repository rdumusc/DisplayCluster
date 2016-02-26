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
#include "log.h"

// false-positive on qt signals for Q_PROPERTY notifiers
// cppcheck-suppress uninitMemberVar
Tile::Tile( const uint index, const QRect& rect )
    : _index( index )
    , _swap( false )
    , _resize( false )
    , _nextCoord( rect )
    , _backGlTexture( 0 )
{
    setFlag( ItemHasContents, true );
    setVisible( false );
    setPosition( rect.topLeft( ));
    setSize( rect.size( ));
    update( rect );
}

uint Tile::getIndex() const
{
    return _index;
}

QRect Tile::getCoord() const
{
    return QRect( x(), y(), width(), height( ));
}

void Tile::update( const QRect& rect )
{
    _updateBackTexture = true;

    if( rect != getCoord( ))
    {
        _resize = true;
        _nextCoord = rect;
    }

    QQuickItem::update();
}

uint Tile::getBackGlTexture() const
{
    return _backGlTexture;
}

QSize Tile::getBackGlTextureSize() const
{
    return _nextCoord.size();
}

void Tile::swapImage()
{
    _swap = true;
    if( !isVisible( ))
        setVisible( true );

    setPosition( _nextCoord.topLeft( ));
    setSize( _nextCoord.size( ));

    QQuickItem::update();
}

void Tile::markBackTextureUpdated()
{
    emit readyToSwap( shared_from_this( ));
}

QSGNode* Tile::updatePaintNode( QSGNode* oldNode,
                                QQuickItem::UpdatePaintNodeData* )
{
    //if( !isVisible( ))
    //    qDebug() << "updating without visible: " << getIndex();

    TextureNode* node = static_cast<TextureNode*>( oldNode );
    if( !node )
        node = new TextureNode( QSize( width(), height( )), window( ));

    if( _swap )
    {
        node->swap();
        _swap = false;
    }

    if( _resize )
    {
        node->resize( _nextCoord.size( ));
        _resize = false;
    }

    _backGlTexture = node->getBackGlTexture();

    if( _updateBackTexture )
    {
        _updateBackTexture = false;
        emit textureInitialized( shared_from_this( ));
    }

    return node;
}
