/*********************************************************************/
/* Copyright (c) 2014, EPFL/Blue Brain Project                       */
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

#ifndef CONTENTWINDOWRENDERER_H
#define CONTENTWINDOWRENDERER_H

#include "types.h"
#include "Renderable.h"
#include "GLQuad.h"

#include <QRectF>

/**
 * Render a ContentWindow and its Content using the associated FactoryObject.
 */
class ContentWindowRenderer : public Renderable
{
public:
    /**
     * Constructor.
     * @param factories Used to retrieve FactoryObjects for rendering Contents.
     */
    ContentWindowRenderer(FactoriesPtr factories);

    /**
     * Render the associated ContentWindow.
     * @see setContentWindow()
     */
    void render() override;

    /**
     * Set the ContentWindow to be rendered.
     * @see render()
     */
    void setContentWindow(ContentWindowPtr window);

    /** Display the window borders. */
    void setShowWindowBorders(const bool show);

    /** Display the zoom context. */
    void setShowZoomContext(const bool show);

    /** Set PixelStream rendering options. */
    void setPixelStreamOptions(const bool showSegmentBorders,
                               const bool showSegmentStatistics);

private:
    FactoriesPtr factories_;
    ContentWindowPtr window_;
    GLQuad quad_;

    bool showWindowBorders_;
    bool showZoomContext_;

    bool showSegmentBorders_;
    bool showSegmentStatistics_;

    void renderWindowBorder();
    void renderContent();
    void renderContextView(FactoryObjectPtr object, const QRectF& texCoord);
    QRectF getTexCoord() const;

    void drawQuad(const QRectF& coord);
    void drawQuadBorder(const QRectF& coord, const float width);
};

#endif // CONTENTWINDOWRENDERER_H
