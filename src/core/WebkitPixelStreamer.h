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

#ifndef WEBKITPIXELSTREAMER_H
#define WEBKITPIXELSTREAMER_H

#include "LocalPixelStreamer.h"
#include <QString>
#include <QImage>
#include <QMutex>

class QWebView;
class QTimer;
class QRect;
class QWebHitTestResult;
class QWebElement;

class WebkitAuthenticationHelper;

class WebkitPixelStreamer : public LocalPixelStreamer
{
    Q_OBJECT

public:
    WebkitPixelStreamer(QString uri);
    ~WebkitPixelStreamer();

    virtual QSize size() const;

    void setUrl(QString url);

    QWebView* getView() const;

public slots:
    virtual void updateInteractionState(InteractionState interactionState);

private slots:
    void update();

private:

    QWebView* webView_;
    WebkitAuthenticationHelper* authenticationHelper_;
    QTimer* timer_;
    QMutex mutex_;
    int frameIndex_;

    QImage image_;

    bool interactionModeActive_;

    void processClickEvent(const InteractionState &interactionState);
    void processPressEvent(const InteractionState &interactionState);
    void processMoveEvent(const InteractionState &interactionState);
    void processReleaseEvent(const InteractionState &interactionState);
    void processWheelEvent(const InteractionState &interactionState);
    void processKeyPress(const InteractionState &interactionState);
    void processKeyRelease(const InteractionState &interactionState);
    void processViewSizeChange(const InteractionState &interactionState);

    QWebHitTestResult performHitTest(const InteractionState &interactionState) const;
    QPoint getPointerPosition(const InteractionState &interactionState) const;
    PixelStreamSegmentParameters createSegmentHeader() const;
    bool isWebGLElement(const QWebElement &element) const;
};

#endif // WEBKITPIXELSTREAMER_H
