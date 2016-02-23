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

#include <QObject>
#include <QRect>
#include <QSet>

/**
 * Qml parameters for an image tile.
 */
class Tile : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY( Tile )

    Q_PROPERTY( uint index READ getIndex CONSTANT )
    Q_PROPERTY( QRect coord READ getCoord NOTIFY coordChanged )
    Q_PROPERTY( bool visible READ isVisible NOTIFY visibilityChanged )

public:
    // false-positive on qt signals for Q_PROPERTY notifiers
    // cppcheck-suppress uninitMemberVar
    Tile( const uint index, const QRect& rect, const bool visible )
        : _index( index )
        , _rect( rect )
        , _visible( visible )
    {}

    const QRect& getCoord() const
    {
        return _rect;
    }

    bool isVisible() const
    {
        return _visible;
    }

    void setVisible( const bool visible )
    {
        if( _visible == visible )
            return;

        _visible = visible;
        emit visibilityChanged();
    }

    uint getIndex() const
    {
        return _index;
    }

    void update( const QRect& rect )
    {
        if( _rect == rect )
            return;

        _rect = rect;
        emit coordChanged();
    }

public slots:
    void associateGlTexture( const uint id )
    {
        if( _glTextures.contains( id ))
            return;

        _glTextures.insert( id );
        if( _glTextures.size() == 1 )
            emit requestTextureUpdate();
    }

signals:
    void coordChanged();
    void visibilityChanged();


    void requestTextureUpdate();

    /** Notifier for the DoubleBufferedImage to swap the texture/image. */
    void swapImage();

private:
    uint _index;
    QRect _rect;
    bool _visible;
    QSet<uint> _glTextures;
};

#endif
