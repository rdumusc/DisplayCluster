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

#include "QuadLineNode.h"
#include "TextureNode.h"
#include "log.h"

namespace
{
const qreal borderWidth = 10.0;
const QColor borderColor( "lightgreen" );
}

// false-positive on qt signals for Q_PROPERTY notifiers
// cppcheck-suppress uninitMemberVar
Tile::Tile( const uint id, const QRect& rect )
    : _tileId( id )
    , _policy( AdjustToTexture )
    , _swapRequested( false )
    , _nextCoord( rect )
    , _showBorder( false )
    , _border( nullptr )
{
    setFlag( ItemHasContents, true );
    setVisible( true );

    connect( this, &QQuickItem::parentChanged, this, &Tile::_onParentChanged );
}

uint Tile::getId() const
{
    return _tileId;
}

bool Tile::getShowBorder() const
{
    return _showBorder;
}

void Tile::setShowBorder( const bool set )
{
    if( _showBorder == set )
        return;

    _showBorder = set;
    emit showBorderChanged();
    QQuickItem::update();
}

void Tile::update( const QRect& rect )
{
    _nextCoord = rect;
    QQuickItem::update();
}

void Tile::setSizePolicy( const Tile::SizePolicy policy )
{
    _policy = policy;
}

void Tile::swapImage()
{
    _swapRequested = true;

    if( !isVisible( ))
        setVisible( true );

    if( _policy == AdjustToTexture )
    {
        setPosition( _nextCoord.topLeft( ));
        setSize( _nextCoord.size( ));
    }

    QQuickItem::update();
}

void Tile::updateBackTexture( ImagePtr image )
{
    if( !image )
    {
        put_flog( LOG_DEBUG, "Invalid image" );
        return;
    }

    _image = image;
    QQuickItem::update();
}

QSGNode* Tile::updatePaintNode( QSGNode* oldNode,
                                QQuickItem::UpdatePaintNodeData* )
{
    TextureNode* node = static_cast<TextureNode*>( oldNode );
    if( !node )
    {
        node = new TextureNode( _nextCoord.size(), window( ));

        connect( node, &TextureNode::backTextureReady, this, [this]() {
            emit textureUpdated( shared_from_this( ));
        });

        emit textureReady( shared_from_this( ));
    }

//    put_flog( LOG_DEBUG, "Swap: %i, Image: %i", _swapRequested, _image.get( ));
//    put_flog( LOG_DEBUG, "node->isReadyToSwap(): %i", node->isReadyToSwap( ));

    if( _swapRequested && node->isReadyToSwap( ))
    {
        node->swap();
        _swapRequested = false;
        emit textureReady( shared_from_this( ));
    }

    if( _image )
    {
        node->updateBackTexture( _image );
        _image.reset();
    }

    node->setRect( boundingRect( ));

    _updateBorderNode( node );

    return node;
}

void Tile::_updateBorderNode( TextureNode* parentNode )
{
    if( _showBorder )
    {
        if( !_border )
        {
            _border = new QuadLineNode( parentNode->rect(), borderWidth );
            _border->setColor( borderColor );
            parentNode->appendChildNode( _border );
        }
        else
            _border->setRect( parentNode->rect( ));
    }
    else if( _border )
    {
        parentNode->removeChildNode( _border );
        delete _border;
        _border = nullptr;
    }
}

void Tile::_onParentChanged( QQuickItem* newParent )
{
    if( !newParent )
    {
        disconnect( _widthConn );
        disconnect( _heightConn );
        return;
    }

    if( _policy != FillParent )
        return;

    setWidth( newParent->width( ));
    setHeight( newParent->height( ));

    _widthConn = connect( newParent, &QQuickItem::widthChanged, [this]() {
        setWidth( parentItem()->width( ));
    } );
    _heightConn = connect( newParent, &QQuickItem::heightChanged, [this]() {
        setHeight( parentItem()->height( ));
    } );
}
