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

#ifndef CONTENTSYNCHRONIZER_H
#define CONTENTSYNCHRONIZER_H

#include "types.h"
#include "ContentType.h"
#include "Tiles.h"

#include <QObject>
#include <QQmlImageProviderBase>

/**
 * Interface for synchronizing QML content rendering.
 *
 * An implementation should be provided for each ContentType which requires
 * a synchronization step before rendring.
 */
class ContentSynchronizer : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY( ContentSynchronizer )
    Q_PROPERTY( QString sourceParams READ getSourceParams
                NOTIFY sourceParamsChanged )
    Q_PROPERTY( bool allowsTextureCaching READ allowsTextureCaching CONSTANT )
    Q_PROPERTY( Tiles* tiles READ getTilesPtr CONSTANT )
    Q_PROPERTY( QSize tilesArea READ getTilesArea NOTIFY tilesAreaChanged )
    Q_PROPERTY( QString statistics READ getStatistics NOTIFY statisticsChanged )

public:
    /** Constructor */
    ContentSynchronizer() = default;

    /** Virtual destructor */
    virtual ~ContentSynchronizer();

    /** Update the Content. */
    virtual void update( const ContentWindow& window,
                         const QRectF& visibleArea ) = 0;

    /** Get the additional source parameters. */
    virtual QString getSourceParams() const = 0;

    /** @return true if the content allows texture caching for rendering. */
    virtual bool allowsTextureCaching() const = 0;


    /** Get the list of tiles that compose the content. */
    virtual Tiles& getTiles() = 0;

    /** Qml needs a pointer instead of a reference. */
    Tiles* getTilesPtr() { return &getTiles(); }

    /** The total area covered by the tiles (may depend on current LOD). */
    virtual QSize getTilesArea() const = 0;

    /** Get statistics about this Content. */
    virtual QString getStatistics() const = 0;

    /** @return a ContentSynchronizer for the given content. */
    static ContentSynchronizerPtr create( ContentPtr content,
                                          QQmlImageProviderBase& provider );

signals:
    /** Notifier for the sourceParams property. */
    void sourceParamsChanged();

    /** Notifier for the tiles area property. */
    void tilesAreaChanged();

    /** Notifier for the statistics property. */
    void statisticsChanged();
};

#endif // CONTENTSYNCHRONIZER_H
