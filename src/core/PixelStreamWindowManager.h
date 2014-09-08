/*********************************************************************/
/* Copyright (c) 2014, EPFL/Blue Brain Project                       */
/*                     Daniel Nachbaur <daniel.nachbaur@epfl.ch>     */
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

#ifndef PIXEL_STREAM_WINDOW_MANAGER_H
#define PIXEL_STREAM_WINDOW_MANAGER_H

#include <QObject>
#include <QPointF>
#include <QSize>
#include <map>

#include "types.h"

class EventReceiver;

/**
 * Handles window creation, association and updates for pixel streamers, both
 * local and external. The association is one streamer to one window.
 */
class PixelStreamWindowManager : public QObject
{
    Q_OBJECT

public:
    /**
     * Create a window manager that handles windows for streamers.
     *
     * @param displayGroup the content windows of streamers will be added
     *                     to and removed from this DisplayGroup.
     */
    PixelStreamWindowManager( DisplayGroup& displayGroup );

    ~PixelStreamWindowManager();

    /**
     * Create a content window which will be associated to the pixel stream once
     * the stream is ready.
     *
     * @param uri the URI of the streamer
     * @param pos the desired normalized position of the window
     * @param size the desired normalized size of the window
     * @return the window of the streamer. Is never NULL.
     */
    ContentWindowPtr createContentWindow( const QString& uri,
                                          const QPointF& pos,
                                          const QSizeF& size );

    /**
     * Remove the associated content window from the given stream. This should
     * usually happen automatically once the stream is closed or the content
     * window is closed from the DisplayGroup.
     *
     * @param uri the URI of the streamer
     */
    void removeContentWindow( const QString& uri );

    /**
     * @param uri the URI of streamer
     * @return the associated window of the given streamer. Can be NULL.
     * @todo This should not be public! See DISCL-230
     */
    ContentWindowPtr getContentWindow( const QString& uri ) const;

    /**
     * Update the dimension of the content (and maybe the window) when the size
     * of the streaming source changed.
     *
     * @param uri the URI of the streamer
     * @param size the size in pixels of the streaming source
     */
    void updateDimension( QString uri, QSize size );

    /**
     * Hide the associated content window of the stream.
     *
     * @param uri the URI of the streamer
     */
    void hideWindow( const QString& uri );

public slots:
    /**
     * Is called once a new streamer was created. This will associate the
     * streamer to its content window. If there is no window yet, one will be
     * created using the size of the streamer source.
     *
     * @param uri the URI of the streamer
     * @param size the size in pixels of the streaming source
     */
    void openPixelStreamWindow( QString uri, QSize size );

    /**
     * Is called when streamer is closed. This will remove the associated
     * content window.
     *
     * @param uri the URI of the streamer
     */
    void closePixelStreamWindow( const QString& uri );

    /**
     * Is called when the streamer wants to enable event handling. This will
     * register the given EventReceiver in the content window.
     *
     * @param uri the URI of the streamer
     * @param exclusive true if only one source of the streamer should
     *                  send/handle events
     * @param receiver the event receiver instance
     */
    void registerEventReceiver( QString uri, bool exclusive,
                                EventReceiver* receiver );

signals:
    /**
     * Is emitted when the associated content window of the streamer is closed.
     *
     * @param uri the URI of the streamer
     */
    void pixelStreamWindowClosed( QString uri );

    /**
     * Is emitted after registerEventReceiver() was executed.
     *
     * @param uri the URI of the streamer
     * @param success true if event registration was successful, false otherwise
     */
    void eventRegistrationReply( QString uri, bool success );

private slots:
    /**
     * This will close the streamer that is associated with the given window.
     *
     * @param contentWindow the content window that might be associated
     *                      to a streamer
     */
    void onContentWindowRemoved( ContentWindowPtr contentWindow );

private:
    DisplayGroup& displayGroup_;

    typedef std::map<QString, ContentWindowPtr> ContentWindowMap;
    ContentWindowMap streamerWindows_;
};

#endif
