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

#include "MovieSynchronizer.h"

#include "ContentWindow.h"
#include "MovieContent.h"
#include "MovieProvider.h"
#include "MovieUpdater.h"

MovieSynchronizer::MovieSynchronizer( const QString& uri,
                                      MovieProvider& provider )
    : _provider( provider )
    , _uri( uri )
    , _timestamp( 0.0 )
{
    _updater = _provider.open( uri );
    connect( _updater.get(), SIGNAL( pictureUpdated( double )),
             this, SLOT( onPictureUpdated( double )));
}

MovieSynchronizer::~MovieSynchronizer()
{
    _provider.close( _uri );
}

void MovieSynchronizer::update( const ContentWindow& window,
                                const QRectF& visibleArea )
{
    Q_UNUSED( visibleArea );
    _updater->sync( static_cast< const MovieContent& >( *window.getContent( )));
}

QString MovieSynchronizer::getSourceParams() const
{
    return QString( "?%1" ).arg( _timestamp );
}

bool MovieSynchronizer::allowsTextureCaching() const
{
    return false;
}

QString MovieSynchronizer::getStatistics() const
{
    return _fpsCounter.toString();
}

void MovieSynchronizer::onPictureUpdated( const double timestamp )
{
    _timestamp = timestamp;
    emit sourceParamsChanged();

    _fpsCounter.tick();
    emit statisticsChanged();
}
