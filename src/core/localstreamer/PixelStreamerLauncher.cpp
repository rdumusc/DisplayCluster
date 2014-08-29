/*********************************************************************/
/* Copyright (c) 2013, EPFL/Blue Brain Project                       */
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

#include "PixelStreamerLauncher.h"

#include "DockPixelStreamer.h"
#include "CommandLineOptions.h"

#include "log.h"
#include "PixelStreamWindowManager.h"
#include "configuration/MasterConfiguration.h"

#include <QCoreApplication>
#include <QProcess>

#define DOCK_WIDTH_RELATIVE_TO_WALL   0.175

#ifdef _WIN32
#define LOCALSTREAMER_BIN "localstreamer.exe"
#else
#define LOCALSTREAMER_BIN "localstreamer"
#endif

#define WEBBROWSER_DEFAULT_SIZE  QSize(1280, 1024)

PixelStreamerLauncher::PixelStreamerLauncher(PixelStreamWindowManager& windowManager,
                                             const MasterConfiguration& config)
    : windowManager_(windowManager)
    , config_(config)
{
    connect(&windowManager_, SIGNAL(pixelStreamWindowClosed(QString)),
            this, SLOT(dereferenceLocalStreamer(QString)), Qt::QueuedConnection);
}

void PixelStreamerLauncher::openWebBrowser(const QPointF pos, const QSize size, const QString url)
{
    static int webbrowserCounter = 0;
    const QString& uri = QString("WebBrowser_%1").arg(webbrowserCounter++);

    const QSize viewportSize = !size.isEmpty() ? size : WEBBROWSER_DEFAULT_SIZE;

    const QSizeF normalizedSize( (double)viewportSize.width() / config_.getTotalWidth(),
                                 (double)viewportSize.height() / config_.getTotalHeight());
    windowManager_.createContentWindow(uri, pos, normalizedSize);

    const QString program = QString("%1/%2").arg(QCoreApplication::applicationDirPath(), LOCALSTREAMER_BIN);

    CommandLineOptions options;
    options.setPixelStreamerType(PS_WEBKIT);
    options.setName(uri);
    options.setUrl(url);
    options.setWidth(viewportSize.width());
    options.setHeight(viewportSize.height());

    processes_[uri] = new QProcess(this);
    if ( !processes_[uri]->startDetached(program, options.getCommandLineArguments(), QDir::currentPath( )))
        put_flog(LOG_ERROR, "Browser process could not be started!");
}

void PixelStreamerLauncher::openDock(const QPointF pos)
{
    const unsigned int dockWidth = config_.getTotalWidth()*DOCK_WIDTH_RELATIVE_TO_WALL;
    const unsigned int dockHeight = dockWidth * DockPixelStreamer::getDefaultAspectRatio();

    openDock(pos, QSize(dockWidth, dockHeight), config_.getDockStartDir());
}

void PixelStreamerLauncher::openDock(const QPointF pos, const QSize size, const QString rootDir)
{
    const QString& uri = DockPixelStreamer::getUniqueURI();

    const QSize& dockSize = DockPixelStreamer::constrainSize(size);

    const QSizeF normalizedSize( (double)dockSize.width() / config_.getTotalWidth(),
                                 (double)dockSize.height() / config_.getTotalHeight());
    windowManager_.createContentWindow(uri, pos, normalizedSize);

    if( !processes_.count(uri) )
    {
        if( !createDock(dockSize, rootDir))
            put_flog(LOG_ERROR, "Dock process could not be started!");
    }
}

void PixelStreamerLauncher::hideDock()
{
    windowManager_.hideWindow(DockPixelStreamer::getUniqueURI());
}

void PixelStreamerLauncher::dereferenceLocalStreamer(const QString uri)
{
    processes_.erase(uri);
}

bool PixelStreamerLauncher::createDock(const QSize& size, const QString& rootDir)
{
    const QString& uri = DockPixelStreamer::getUniqueURI();

    assert( !processes_.count(uri) );

    QString program = QString("%1/%2").arg(QCoreApplication::applicationDirPath(), LOCALSTREAMER_BIN);

    CommandLineOptions options;
    options.setPixelStreamerType(PS_DOCK);
    options.setName(uri);
    options.setRootDir(rootDir);
    options.setWidth(size.width());
    options.setHeight(size.height());

    processes_[uri] = new QProcess(this);
    return processes_[uri]->startDetached(program, options.getCommandLineArguments(), QDir::currentPath());
}
