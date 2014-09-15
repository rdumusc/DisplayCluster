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

#include "DisplayGroup.h"
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

IMPLEMENT_SERIALIZE_FOR_XML( ContentWindow )

ContentWindow::ContentWindow()
    : uuid_( QUuid::createUuid( ))
    , contentWidth_( 0 )
    , contentHeight_( 0 )
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
    , contentWidth_( 0 )
    , contentHeight_( 0 )
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

void ContentWindow::getContentDimensions( int &contentWidth, int &contentHeight )
{
    contentWidth = contentWidth_;
    contentHeight = contentHeight_;
}

void ContentWindow::getCoordinates( double &x, double &y, double &w, double &h )
{
    x = coordinates_.x();
    y = coordinates_.y();
    w = coordinates_.width();
    h = coordinates_.height();
}

QRectF ContentWindow::getCoordinates() const
{
    return coordinates_;
}

void ContentWindow::getPosition( double &x, double &y )
{
    x = coordinates_.x();
    y = coordinates_.y();
}

void ContentWindow::getSize( double &w, double &h )
{
    w = coordinates_.width();
    h = coordinates_.height();
}

void ContentWindow::getCenter( double &centerX, double &centerY )
{
    centerX = centerX_;
    centerY = centerY_;
}

double ContentWindow::getZoom()
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

ContentWindow::WindowState ContentWindow::getWindowState()
{
    return windowState_;
}

Event ContentWindow::getEvent() const
{
    return latestEvent_;
}

bool ContentWindow::registerEventReceiver( EventReceiver* receiver )
{
    const bool success = connect( this, SIGNAL( eventChanged( Event )),
                                  receiver, SLOT( processEvent( Event )));
    if ( success )
        ++eventReceiversCount_;

    return success;
}

SizeState ContentWindow::getSizeState() const
{
    return sizeState_;
}

void ContentWindow::fixAspectRatio()
{
    if( contentWidth_ == 0 && contentHeight_ == 0 )
        return;

    double aspect = (double)contentWidth_ / (double)contentHeight_;
    const double screenAspect = g_configuration->getAspectRatio();

    aspect /= screenAspect;

    double w = coordinates_.width();
    double h = coordinates_.height();

    if( aspect > coordinates_.width() / coordinates_.height( ))
    {
        h = coordinates_.width() / aspect;
    }
    else if( aspect <= coordinates_.width() / coordinates_.height( ))
    {
        w = coordinates_.height() * aspect;
    }

    // we don't want to call setSize unless necessary, since it will emit a signal
    if( w != coordinates_.width() || h != coordinates_.height( ))
    {
        coordinates_.setWidth( w );
        coordinates_.setHeight( h );

        setSize( coordinates_.width(), coordinates_.height( ));
    }
}

void ContentWindow::adjustSize( const SizeState state )
{
    sizeState_ = state;

    const double contentAR = contentHeight_ == 0 ? 16./9 :
                                 double(contentWidth_) / double(contentHeight_);
    const double wallAR = 1. / g_configuration->getAspectRatio();

    double height = contentHeight_ == 0
                            ? 1.
                            : double(contentHeight_) / double(g_configuration->getTotalHeight());
    double width = contentWidth_ == 0
                            ? wallAR * contentAR * height
                            : double(contentWidth_) / double(g_configuration->getTotalWidth());

    QRectF coordinates;

    switch( state )
    {
    case SIZE_FULLSCREEN:
        {
            coordinatesBackup_ = coordinates_;
            const double resize = std::min( 1. / height, 1. / width );
            width *= resize;
            height *= resize;

            // center on the wall
            coordinates.setRect( (1. - width) * .5 , (1. - height) * .5, width, height );
        } break;

    case SIZE_1TO1:
        height = std::min( height, 1. );
        width = wallAR * contentAR * height;
        if( width > 1. )
        {
            height /= width;
            width = 1.;
        }

        // center on the wall
        coordinates.setRect( (1. - width) * .5 , (1. - height) * .5, width, height );
        break;

    case SIZE_NORMALIZED:
        coordinates = coordinatesBackup_;
        break;
    default:
        return;
    }

    setCoordinates( coordinates );
}

void ContentWindow::setContentDimensions( int contentWidth, int contentHeight )
{
    contentWidth_ = contentWidth;
    contentHeight_ = contentHeight;

    emit( contentDimensionsChanged( contentWidth_, contentHeight_ ));
}

void ContentWindow::setCoordinates( const QRectF coordinates )
{
    // don't allow negative width or height
    if( coordinates.isValid( ))
        return;

    coordinates_ = coordinates;
    fixAspectRatio();

    emit( coordinatesChanged( coordinates_ ));

    setEventToNewDimensions();
}

void ContentWindow::setPosition( const double x, const double y )
{
    coordinates_.moveTo( x, y );

    emit( positionChanged( coordinates_.x(), coordinates_.y( )));
}

void ContentWindow::setSize( const double w, const double h )
{
    // don't allow negative width or height
    if( w > 0. && h > 0. )
    {
        coordinates_.setWidth( w );
        coordinates_.setHeight( h );
    }

    fixAspectRatio();

    emit( sizeChanged( coordinates_.width(), coordinates_.height( )));

    setEventToNewDimensions();
}

void ContentWindow::scaleSize( const double factor )
{
    if( factor < 0. )
        return;

    coordinates_.setX( coordinates_.x() - ( factor - 1. ) * coordinates_.width() / 2. );
    coordinates_.setY( coordinates_.y() - ( factor - 1. ) * coordinates_.height() / 2. );
    coordinates_.setWidth( coordinates_.width() * factor );
    coordinates_.setHeight( coordinates_.height() * factor );

    setCoordinates( coordinates_ );
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

    emit( centerChanged( centerX_, centerY_ ));
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

    emit( zoomChanged( zoom_ ));
}

void ContentWindow::setWindowState(const ContentWindow::WindowState state )
{
    windowState_ = state;

    emit( windowStateChanged( windowState_ ));
}

void ContentWindow::setEvent( const dc::Event event_ )
{
    latestEvent_ = event_;

    emit( eventChanged( event_ ));
}

void ContentWindow::moveToFront()
{
    DisplayGroupPtr displayGroup = getDisplayGroup();
    if ( displayGroup )
        displayGroup->moveContentWindowToFront( shared_from_this( ));
    else
        put_flog( LOG_DEBUG, "The DisplayGroupMangerPtr is invalid" );
}

void ContentWindow::close()
{
    getDisplayGroup()->removeContentWindow( shared_from_this( ));
    emit( closed( ));
}

void ContentWindow::setEventToNewDimensions()
{
    Event state;
    state.type = Event::EVT_VIEW_SIZE_CHANGED;
    state.dx = coordinates_.width() * g_configuration->getTotalWidth();
    state.dy = coordinates_.height() * g_configuration->getTotalHeight();
    setEvent( state );
}

void ContentWindow::setContent( ContentPtr content )
{
    if( content_ )
    {
        content_->disconnect( this, SLOT( setContentDimensions( int, int )));
        content_->disconnect( this, SIGNAL( contentModified( )));
    }

    content_ = content;

    if( content_ )
    {
        content_->getDimensions( contentWidth_, contentHeight_ );

        connect( content.get(), SIGNAL( dimensionsChanged( int, int )),
                 this, SLOT( setContentDimensions( int, int )));

        connect( content.get(), SIGNAL( modified( )),
                 this, SIGNAL( contentModified( )));
    }
    else
        contentWidth_ = contentHeight_ = 0;

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

DisplayGroupPtr ContentWindow::getDisplayGroup() const
{
    return displayGroup_.lock();
}

void ContentWindow::setDisplayGroup( DisplayGroupPtr displayGroup )
{
    displayGroup_ = displayGroup;
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
