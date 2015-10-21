/*********************************************************************/
/* Copyright (c) 2013-2015, EPFL/Blue Brain Project                  */
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

#ifndef PIXELSTREAMERLAUNCHER_H
#define PIXELSTREAMERLAUNCHER_H

#include <map>

#include <QObject>
#include <QPointF>
#include <QSize>

class QProcess;
class PixelStreamWindowManager;
class MasterConfiguration;

/**
 * Launch Pixel Streamers as separate processes.
 *
 * The processes connect to the Master application on localhost using the
 * deflect::Stream API. They can be terminated by the user by closing their
 * associated window.
 *
 * Due to an incompatibility between QProcess and MPI(*), we must start the
 * processes DETACHED.
 * In theory they might stay alive after the main application has exited.
 * In practice, this doesn't seem to happen; our processes exit when the
 * deflect::Stream is closed in any case.
 *
 * (*) MPI captures the SIGCHLD that QProcess relies on to detect that the
 * process has finished. Thus, the call to waitForFinished() blocks forever in
 * QProcess destructor...
 */
class PixelStreamerLauncher : public QObject
{
    Q_OBJECT

public:
    /**
     * Create a new PixelStreamerLauncher
     *
     * @param windowManager Manages the windows of the streamers
     * @param config The configuration for the default parameters
     */
    PixelStreamerLauncher( PixelStreamWindowManager& windowManager,
                           const MasterConfiguration& config );

public slots:
    /**
     * Open a WebBrowser.
     *
     * @param pos The position of the center of the browser window.
     *        If pos.isNull(), the window is centered on the DisplayWall.
     * @param size The initial size of the viewport of the webbrowser in pixels.
     * @param url The webpage to open.
     */
    void openWebBrowser( QPointF pos, QSize size, QString url );

    /**
     * Open the Dock using default parameters.
     *
     * A new dock instance is created if it was closed, otherwise the existing
     * Dock instance is moved to the given position.
     * @param pos The position of the center of the Dock
     */
    void openDock( QPointF pos );

    /** Hide the Dock. */
    void hideDock();

private slots:
    void dereferenceLocalStreamer( QString uri );

private:
    Q_DISABLE_COPY( PixelStreamerLauncher )

    typedef std::map< QString, QProcess* > Streamers;
    Streamers _processes;

    PixelStreamWindowManager& _windowManager;
    const MasterConfiguration& _config;

    bool _createDock( const QSize& size, const QString& rootDir );
    QString _getLocalStreamerBin() const;
};

#endif // PIXELSTREAMERLAUNCHER_H
