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

#include "MovieProvider.h"

#include "MovieUpdater.h"
#include "FFMPEGPicture.h"

const QString MovieProvider::ID( "movie" );

MovieProvider::MovieProvider()
    : QQuickImageProvider( QQmlImageProviderBase::Image )
{}

MovieProvider::~MovieProvider() {}

QImage MovieProvider::requestImage( const QString& id, QSize* size,
                                    const QSize& requestedSize )
{
    QStringList params = id.split( "?" );
    if( params.length() != 2 )
        return QImage();

    const QString& movieFile = params[0];

    bool ok = false;
    const double timestamp = params[1].toDouble( &ok );
    if( !ok )
        return QImage();
    Q_UNUSED( timestamp );

    if( !_movies.count( movieFile ))
        return QImage();

    PicturePtr picture = _movies[ movieFile ]->getPicture();
    if( !picture )
        return QImage();

    QImage image = picture->toQImage();

    if( !requestedSize.isEmpty( ))
        image = image.scaled( requestedSize );
    else
        image = image.copy();

    *size = image.size();
    return image;
}

MovieUpdaterSharedPtr MovieProvider::open( const QString& movieFile )
{
    if( !_movies.count( movieFile ))
        _movies[ movieFile ] = std::make_shared<MovieUpdater>( movieFile );

    return _movies[ movieFile ];
}

void MovieProvider::close( const QString& movieFile )
{
    _movies.erase( movieFile );
}

void MovieProvider::synchronize( WallToWallChannel& channel )
{
    for( auto& movie : _movies )
        movie.second->sync( channel );
}
