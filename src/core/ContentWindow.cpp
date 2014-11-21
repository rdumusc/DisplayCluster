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

#include "ContentWindow.h"

#include "ContentInteractionDelegate.h"
#include "EventReceiver.h"

#include "config.h"
#include "log.h"

#include "globals.h"
#include "configuration/Configuration.h"

#include "PixelStreamInteractionDelegate.h"
#include "ZoomInteractionDelegate.h"
#if ENABLE_PDF_SUPPORT
#  include "PDFInteractionDelegate.h"
#endif

#define DEFAULT_ASPECT_RATIO 16./9.
#define MAX_SIZE 2.0
#define MIN_SIZE 0.05

IMPLEMENT_SERIALIZE_FOR_XML( ContentWindow )

ContentWindow::ContentWindow()
    : uuid_( QUuid::createUuid( ))
    , centerX_( 0.5 )
    , centerY_( 0.5 )
    , zoom_( 1 )
    , windowState_( UNSELECTED )
    , sizeState_( SIZE_NORMALIZED )
    , controlState_( STATE_LOOP )
    , eventReceiversCount_( 0 )
{
}

ContentWindow::ContentWindow( ContentPtr content )
    : uuid_( QUuid::createUuid( ))
    , centerX_( 0.5 )
    , centerY_( 0.5 )
    , zoom_( 1 )
    , windowState_( UNSELECTED )
    , sizeState_( SIZE_NORMALIZED )
    , controlState_( STATE_LOOP )
    , eventReceiversCount_( 0 )
{
    setContent( content );
    adjustSize( SIZE_1TO1 );
}

ContentWindow::~ContentWindow()
{
}

const QUuid& ContentWindow::getID() const
{
    return uuid_;
}

const QRectF& ContentWindow::getCoordinates() const
{
    return coordinates_;
}

void ContentWindow::getPosition( double &x, double &y ) const
{
    x = coordinates_.x();
    y = coordinates_.y();
}

void ContentWindow::getSize( double &w, double &h ) const
{
    w = coordinates_.width();
    h = coordinates_.height();
}

void ContentWindow::getCenter( double &centerX, double &centerY ) const
{
    centerX = centerX_;
    centerY = centerY_;
}

double ContentWindow::getZoom() const
{
    return zoom_;
}

void ContentWindow::toggleWindowState()
{
    setWindowState( windowState_ == UNSELECTED ? SELECTED : UNSELECTED );
}

void ContentWindow::toggleFullscreen()
{
    adjustSize( sizeState_ == SIZE_FULLSCREEN ? SIZE_NORMALIZED : SIZE_FULLSCREEN );
}

ContentWindow::WindowState ContentWindow::getWindowState() const
{
    return windowState_;
}

bool ContentWindow::selected() const
{
    return windowState_ == SELECTED;
}

bool ContentWindow::registerEventReceiver( EventReceiver* receiver )
{
    const bool success = connect( this, SIGNAL( eventChanged( Event )),
                                  receiver, SLOT( processEvent( Event )));
    if ( success )
        ++eventReceiversCount_;

    return success;
}

bool ContentWindow::hasEventReceivers() const
{
    return eventReceiversCount_ > 0;
}

SizeState ContentWindow::getSizeState() const
{
    return sizeState_;
}

ControlState ContentWindow::getControlState() const
{
    return controlState_;
}

void ContentWindow::setControlState( const ControlState state )
{
    controlState_ = state;
}

void ContentWindow::adjustSize( const SizeState state )
{
    sizeState_ = state;

    switch( state )
    {
    case SIZE_FULLSCREEN:
    {
        coordinatesBackup_ = coordinates_;

        QSizeF size = getNormalized1To1Size();
        size.scale( 1.f, 1.f, Qt::KeepAspectRatio );
        setCoordinates( getCenteredCoordinates( size ));
    }
        break;

    case SIZE_1TO1:
    {
        QSizeF size = getNormalized1To1Size();
        clampSize( size );
        setCoordinates( getCenteredCoordinates( size ));
    }
        break;

    case SIZE_NORMALIZED:
        setCoordinates( coordinatesBackup_ );
        break;
    default:
        return;
    }
}

void ContentWindow::setCoordinates( const QRectF& coordinates )
{
    if( !isValidSize( coordinates.size( )))
        return;

    emit coordinatesAboutToChange();

    coordinates_ = coordinates;

    emit modified();

    setEventToNewDimensions();
}

void ContentWindow::setPosition( const double x, const double y )
{
    emit coordinatesAboutToChange();

    coordinates_.moveTo( x, y );

    emit modified();
}

void ContentWindow::setSize( const double w, const double h )
{
    if( !isValidSize( QSizeF( w, h )))
        return;

    emit coordinatesAboutToChange();

    coordinates_.setWidth( w );
    coordinates_.setHeight( h );

    emit modified();

    setEventToNewDimensions();
}

void ContentWindow::scaleSize( const double factor )
{
    if( factor < 0. )
        return;

    QRectF coordinates;
    coordinates.setX( coordinates_.x() - ( factor - 1. ) * coordinates_.width() / 2. );
    coordinates.setY( coordinates_.y() - ( factor - 1. ) * coordinates_.height() / 2. );
    coordinates.setWidth( coordinates_.width() * factor );
    coordinates.setHeight( coordinates_.height() * factor );

    setCoordinates( coordinates );
}

void ContentWindow::setCenter( double centerX, double centerY )
{
    // clamp center point such that view rectangle dimensions are constrained [0,1]
    float tX = centerX - 0.5 / zoom_;
    float tY = centerY - 0.5 / zoom_;
    float tW = 1. / zoom_;
    float tH = 1. / zoom_;

    // handle centerX, clamping it if necessary
    if( tX >= 0. && tX+tW <= 1. )
    {
        centerX_ = centerX;
    }
    else if( tX < 0. )
    {
        centerX_ = 0.5 / zoom_;
    }
    else if( tX+tW > 1. )
    {
        centerX_ = 1. - tW + 0.5 / zoom_;
    }

    // handle centerY, clamping it if necessary
    if( tY >= 0. && tY+tH <= 1. )
    {
        centerY_ = centerY;
    }
    else if( tY < 0. )
    {
        centerY_ = 0.5 / zoom_;
    }
    else if( tY+tH > 1. )
    {
        centerY_ = 1. - tH + 0.5 / zoom_;
    }

    emit modified();
}

void ContentWindow::setZoom( const double zoom )
{
    zoom_ = std::max( zoom, 1.0 );

    float tX = centerX_ - 0.5 / zoom_;
    float tY = centerY_ - 0.5 / zoom_;
    float tW = 1. / zoom_;
    float tH = 1. / zoom_;

    // see if we need to adjust the center point since the rectangle view bounds are outside [0,1]
    if( !QRectF( 0., 0., 1., 1. ).contains( QRectF( tX, tY, tW, tH )))
    {
        // handle centerX, clamping it if necessary
        if( tX < 0. )
        {
            centerX_ = 0.5 / zoom_;
        }
        else if( tX+tW > 1. )
        {
            centerX_ = 1. - tW + 0.5 / zoom_;
        }

        // handle centerY, clamping it if necessary
        if( tY < 0. )
        {
            centerY_ = 0.5 / zoom_;
        }
        else if( tY+tH > 1. )
        {
            centerY_ = 1. - tH + 0.5 / zoom_;
        }

        setCenter( centerX_, centerY_ );
    }

    emit modified();
}

void ContentWindow::setWindowState(const ContentWindow::WindowState state )
{
    windowState_ = state;

    emit modified();
}

void ContentWindow::dispatchEvent( const Event event_ )
{
    emit eventChanged( event_ );
}

void ContentWindow::setEventToNewDimensions()
{
    Event state;
    state.type = Event::EVT_VIEW_SIZE_CHANGED;
    state.dx = coordinates_.width() * g_configuration->getTotalWidth();
    state.dy = coordinates_.height() * g_configuration->getTotalHeight();

    emit eventChanged( state );
}

bool ContentWindow::isValidSize( const QSizeF& size ) const
{
    return ( size.width() >= MIN_SIZE && size.height() >= MIN_SIZE &&
             size.width() <= MAX_SIZE && size.height() <= MAX_SIZE );
}

QSizeF ContentWindow::getNormalized1To1Size() const
{
    const QSize contentSize = content_->getDimensions();

    const double contentAR = ( contentSize.height() == 0 )
            ? DEFAULT_ASPECT_RATIO
            : double( contentSize.width( )) / double( contentSize.height( ));
    const double wallAR = 1. / g_configuration->getAspectRatio();

    double height = ( contentSize.height() == 0 )
            ? 1.
            : double( contentSize.height( )) / double( g_configuration->getTotalHeight( ));
    double width = ( contentSize.width() == 0 )
            ? wallAR * contentAR * height
            : double( contentSize.width( )) / double( g_configuration->getTotalWidth( ));

    return QSizeF( width, height );
}

void ContentWindow::clampSize( QSizeF& size ) const
{
    if ( size.width() > MAX_SIZE || size.height() > MAX_SIZE )
        size.scale( MAX_SIZE, MAX_SIZE, Qt::KeepAspectRatio );

    if ( size.width() < MIN_SIZE || size.height() < MIN_SIZE )
        size.scale( MIN_SIZE, MIN_SIZE, Qt::KeepAspectRatioByExpanding );
}

QRectF ContentWindow::getCenteredCoordinates( const QSizeF& size ) const
{
    // center on the wall
    return QRectF((1.0f - size.width()) * 0.5f, (1.0f - size.height()) * 0.5f,
                  size.width(), size.height( ));
}

void ContentWindow::setContent( ContentPtr content )
{
    if( content_ )
        content_->disconnect( this, SIGNAL( contentModified( )));

    content_ = content;

    if( content_ )
        connect( content.get(), SIGNAL( modified( )),
                 this, SIGNAL( contentModified( )));

    createInteractionDelegate();
}

ContentPtr ContentWindow::getContent() const
{
    return content_;
}

void ContentWindow::createInteractionDelegate()
{
    if( !content_ )
    {
        interactionDelegate_.reset();
        return;
    }

    switch ( content_->getType( ))
    {
    case CONTENT_TYPE_PIXEL_STREAM:
        interactionDelegate_.reset( new PixelStreamInteractionDelegate( *this ));
        break;
#if ENABLE_PDF_SUPPORT
    case CONTENT_TYPE_PDF:
        interactionDelegate_.reset( new PDFInteractionDelegate( *this ));
        break;
#endif
    default:
        interactionDelegate_.reset( new ZoomInteractionDelegate( *this ));
        break;
    }
}

ContentInteractionDelegate& ContentWindow::getInteractionDelegate() const
{
    return *interactionDelegate_;
}

QPointF ContentWindow::getWindowCenterPosition() const
{
    return QPointF( coordinates_.x() + 0.5 * coordinates_.width(),
                    coordinates_.y() + 0.5 * coordinates_.height( ));
}

void ContentWindow::centerPositionAround( const QPointF& position,
                                          const bool constrainToWindowBorders )
{
    if( position.isNull( ))
        return;

    double newX = position.x() - 0.5 * coordinates_.width();
    double newY = position.y() - 0.5 * coordinates_.height();

    if ( constrainToWindowBorders )
    {
        if( newX + coordinates_.width() > 1.0 )
            newX = 1.0 - coordinates_.width();
        if( newY + coordinates_.height() > 1.0 )
            newY = 1.0 - coordinates_.height();

        newX = std::max( 0.0, newX );
        newY = std::max( 0.0, newY );
    }

    setPosition( newX, newY );
}
