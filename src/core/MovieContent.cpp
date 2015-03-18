/*********************************************************************/
/* Copyright (c) 2011 - 2012, The University of Texas at Austin.     */
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

#include "MovieContent.h"

#include "FFMPEGMovie.h"

#include <QtCore/QFileInfo>

#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT_GUID( MovieContent, "MovieContent" )

namespace
{
const QString ICON_PAUSE( "qrc:///img/pause.svg" );
const QString ICON_PLAY( "qrc:///img/play.svg" );
}

MovieContent::MovieContent( const QString& uri )
    : Content( uri )
    , controlState_( STATE_LOOP )
{
    createActions();
}

MovieContent::MovieContent()
    : controlState_( STATE_LOOP )
{
}

CONTENT_TYPE MovieContent::getType() const
{
    return CONTENT_TYPE_MOVIE;
}

bool MovieContent::readMetadata()
{
    QFileInfo file( getURI( ));
    if( !file.exists() || !file.isReadable( ))
        return false;

    const FFMPEGMovie movie( getURI( ));
    if( !movie.isValid( ))
        return false;

    size_ = QSize( movie.getWidth(), movie.getHeight( ));
    return true;
}

const QStringList& MovieContent::getSupportedExtensions()
{
    static QStringList extensions;

    if( extensions.empty( ))
    {
        extensions << "mov" << "avi" << "mp4" << "mkv" << "mpg" << "mpeg"
                   << "flv" << "wmv";
    }

    return extensions;
}

ControlState MovieContent::getControlState() const
{
    return controlState_;
}

void MovieContent::play()
{
    controlState_ = (ControlState)(controlState_ & ~STATE_PAUSED);
}

void MovieContent::pause()
{
    controlState_ = (ControlState)(controlState_ | STATE_PAUSED);
}

void MovieContent::createActions()
{
    ContentAction* playPauseAction = new ContentAction();
    playPauseAction->setCheckable( true );
    playPauseAction->setIcon( ICON_PAUSE );
    playPauseAction->setIconChecked( ICON_PLAY );
    connect( playPauseAction, SIGNAL( checked( )), this, SLOT( pause( )));
    connect( playPauseAction, SIGNAL( unchecked( )), this, SLOT( play( )));
    actions_.add( playPauseAction );
}
