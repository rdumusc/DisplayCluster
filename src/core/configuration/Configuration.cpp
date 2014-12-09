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

#include "Configuration.h"

#include <QtXmlPatterns>

#include <stdexcept>

Configuration::Configuration(const QString &filename)
    : filename_(filename)
    , totalScreenCountX_(0)
    , totalScreenCountY_(0)
    , screenWidth_(0)
    , screenHeight_(0)
    , mullionWidth_(0)
    , mullionHeight_(0)
    , fullscreen_(false)
{
    load();
}

void Configuration::load()
{
    QXmlQuery query;
    if(!query.setFocus(QUrl(filename_)))
        throw std::runtime_error("Invalid configuration file: " + filename_.toStdString());

    QString queryResult;

    // get screen / mullion dimensions
    query.setQuery("string(/configuration/dimensions/@numTilesWidth)");
    if(query.evaluateTo(&queryResult))
        totalScreenCountX_ = queryResult.toInt();

    query.setQuery("string(/configuration/dimensions/@numTilesHeight)");
    if(query.evaluateTo(&queryResult))
        totalScreenCountY_ = queryResult.toInt();

    query.setQuery("string(/configuration/dimensions/@screenWidth)");
    if(query.evaluateTo(&queryResult))
        screenWidth_ = queryResult.toInt();

    query.setQuery("string(/configuration/dimensions/@screenHeight)");
    if(query.evaluateTo(&queryResult))
        screenHeight_ = queryResult.toInt();

    query.setQuery("string(/configuration/dimensions/@mullionWidth)");
    if(query.evaluateTo(&queryResult))
        mullionWidth_ = queryResult.toInt();

    query.setQuery("string(/configuration/dimensions/@mullionHeight)");
    if(query.evaluateTo(&queryResult))
        mullionHeight_ = queryResult.toInt();

    // check for fullscreen mode flag
    query.setQuery("string(/configuration/dimensions/@fullscreen)");
    if(query.evaluateTo(&queryResult))
        fullscreen_ = queryResult.toInt() != 0;
}

int Configuration::getTotalScreenCountX() const
{
    return totalScreenCountX_;
}

int Configuration::getTotalScreenCountY() const
{
    return totalScreenCountY_;
}

int Configuration::getScreenWidth() const
{
    return screenWidth_;
}

int Configuration::getScreenHeight() const
{
    return screenHeight_;
}

int Configuration::getMullionWidth() const
{
    return mullionWidth_;
}

int Configuration::getMullionHeight() const
{
    return mullionHeight_;
}

int Configuration::getTotalWidth() const
{
    return totalScreenCountX_ * screenWidth_ +
           (totalScreenCountX_ - 1) * getMullionWidth();
}

int Configuration::getTotalHeight() const
{
    return totalScreenCountY_ * screenHeight_ +
            (totalScreenCountY_ - 1) * getMullionHeight();
}

double Configuration::getAspectRatio() const
{
    return double(getTotalWidth()) / getTotalHeight();
}

QRectF Configuration::getNormalizedScreenRect(const QPoint& tileIndex) const
{
    assert(tileIndex.x() < totalScreenCountX_);
    assert(tileIndex.y() < totalScreenCountY_);

    const int xPos = tileIndex.x() * (screenWidth_ + mullionWidth_);
    const int yPos = tileIndex.y() * (screenHeight_ + mullionHeight_);

    // normalize to 0->1
    const float totalWidth = (float)getTotalWidth();
    const float totalHeight = (float)getTotalHeight();

    const float screenLeft = (float)xPos / totalWidth;
    const float screenTop = (float)yPos / totalHeight;
    const float screenWidth = (float)screenWidth_ / totalWidth;
    const float screenHeight = (float)screenHeight_ / totalHeight;

    return QRectF(screenLeft, screenTop, screenWidth, screenHeight);
}

bool Configuration::getFullscreen() const
{
    return fullscreen_;
}
