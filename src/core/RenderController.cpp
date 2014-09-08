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

#include "RenderController.h"

#include "RenderContext.h"
#include "GLWindow.h"

#include "DisplayGroupRenderer.h"
#include "MarkerRenderer.h"
#include "TestPattern.h"
#include "FpsRenderer.h"

#include "DisplayGroup.h"
#include "Options.h"

#include <boost/make_shared.hpp>
#include <boost/bind.hpp>

RenderController::RenderController(RenderContextPtr renderContext, FactoriesPtr factories)
    : renderContext_(renderContext)
    , displayGroupRenderer_(new DisplayGroupRenderer(factories))
    , markerRenderer_(new MarkerRenderer)
    , fpsRenderer_(new FpsRenderer(renderContext))
    , syncQuit_(false)
    , syncDisplayGroup_(boost::make_shared<DisplayGroup>())
    , syncOptions_(boost::make_shared<Options>())
{
    renderContext_->addRenderable(displayGroupRenderer_);
    renderContext_->addRenderable(markerRenderer_);
    renderContext_->addRenderable(fpsRenderer_);

    syncDisplayGroup_.setCallback(boost::bind(&DisplayGroupRenderer::setDisplayGroup,
                                               displayGroupRenderer_.get(), _1));

    syncMarkers_.setCallback(boost::bind(&MarkerRenderer::setMarkers,
                                          markerRenderer_.get(), _1));

    syncOptions_.setCallback(boost::bind(&RenderController::setRenderOptions,
                                         this, _1));
}

void RenderController::setupTestPattern(const int rank,
                                        const WallConfiguration& config)
{
    for (size_t i = 0; i < renderContext_->getGLWindowCount(); ++i)
    {
        RenderablePtr testPattern(new TestPattern(renderContext_, config, rank, i));
        testPattern->setVisible(false);
        testPatterns_.append(testPattern);
        GLWindowPtr glWindow = renderContext_->getGLWindow(i);
        glWindow->addRenderable(testPattern);
    }
}

DisplayGroupPtr RenderController::getDisplayGroup() const
{
    return syncDisplayGroup_.get();
}

void RenderController::synchronizeObjects(const SyncFunction& versionCheckFunc)
{
    syncQuit_.sync(versionCheckFunc);
    syncDisplayGroup_.sync(versionCheckFunc);
    syncMarkers_.sync(versionCheckFunc);
    syncOptions_.sync(versionCheckFunc);
}

bool RenderController::quitRendering() const
{
    return syncQuit_.get();
}

void RenderController::updateQuit()
{
    syncQuit_.update(true);
}

void RenderController::updateDisplayGroup(DisplayGroupPtr displayGroup)
{
    syncDisplayGroup_.update(displayGroup);
}

void RenderController::updateMarkers(MarkersPtr markers)
{
    syncMarkers_.update(markers);
}

void RenderController::updateOptions(OptionsPtr options)
{
    syncOptions_.update(options);
}

void RenderController::setRenderOptions(OptionsPtr options)
{
    renderContext_->setBackgroundColor(options->getBackgroundColor());

    markerRenderer_->setVisible(options->getShowTouchPoints());
    fpsRenderer_->setVisible(options->getShowStatistics());
    foreach (RenderablePtr testPattern, testPatterns_)
        testPattern->setVisible(options->getShowTestPattern());

    ContentWindowRenderer& winRenderer = displayGroupRenderer_->getWindowRenderer();
    winRenderer.setShowWindowBorders(options->getShowWindowBorders());
    winRenderer.setShowZoomContext(options->getShowZoomContext());
    winRenderer.setPixelStreamOptions(options->getShowStreamingSegments(),
                                      options->getShowStatistics());
}

