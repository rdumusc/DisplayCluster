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

#include "MasterConfiguration.h"

#include "log.h"

#include <QDomElement>
#include <QtXmlPatterns>
#include <stdexcept>

#define DEFAULT_WEBSERVICE_PORT 8888
#define TRIM_REGEX "[\\n\\t\\r]"
#define DEFAULT_URL "http://www.google.com";

MasterConfiguration::MasterConfiguration(const QString &filename)
    : Configuration(filename)
    , backgroundColor_(Qt::black)
{
    loadMasterSettings();
}

void MasterConfiguration::loadMasterSettings()
{
    QXmlQuery query;
    if(!query.setFocus(QUrl(filename_)))
        throw std::runtime_error("Invalid configuration file: " + filename_.toStdString());

    loadDockStartDirectory(query);
    loadWebBrowserStartURL(query);
    loadBackgroundProperties(query);
}

void MasterConfiguration::loadDockStartDirectory(QXmlQuery& query)
{
    QString queryResult;

    query.setQuery("string(/configuration/dock/@directory)");
    if (query.evaluateTo(&queryResult))
        dockStartDir_ = queryResult.remove(QRegExp(TRIM_REGEX));
    if (dockStartDir_.isEmpty())
        dockStartDir_ = QDir::homePath();

    // WebService server port
    query.setQuery("string(/configuration/webservice/@port)");
    if (query.evaluateTo(&queryResult))
    {
        if (queryResult.isEmpty())
            dcWebServicePort_ = DEFAULT_WEBSERVICE_PORT;
        else
            dcWebServicePort_ = queryResult.toInt();
    }
}

void MasterConfiguration::loadWebBrowserStartURL(QXmlQuery& query)
{
    QString queryResult;

    query.setQuery("string(/configuration/webbrowser/@defaultURL)");
    if (query.evaluateTo(&queryResult))
        webBrowserDefaultURL_ = queryResult.remove(QRegExp(TRIM_REGEX));
    if (webBrowserDefaultURL_.isEmpty())
        webBrowserDefaultURL_ = DEFAULT_URL;
}

void MasterConfiguration::loadBackgroundProperties(QXmlQuery& query)
{
    QString queryResult;

    query.setQuery("string(/configuration/background/@uri)");
    if(query.evaluateTo(&queryResult))
        backgroundUri_ = queryResult.remove(QRegExp("[\\n\\t\\r]"));

    query.setQuery("string(/configuration/background/@color)");
    if (query.evaluateTo(&queryResult))
    {
        queryResult.remove(QRegExp("[\\n\\t\\r]"));

        const QColor newColor( queryResult );
        if( newColor.isValid( ))
            backgroundColor_ = newColor;
    }
}

const QString& MasterConfiguration::getDockStartDir() const
{
    return dockStartDir_;
}

int MasterConfiguration::getWebServicePort() const
{
    return dcWebServicePort_;
}

const QString& MasterConfiguration::getWebBrowserDefaultURL() const
{
    return webBrowserDefaultURL_;
}

const QString& MasterConfiguration::getBackgroundUri() const
{
    return backgroundUri_;
}

const QColor& MasterConfiguration::getBackgroundColor() const
{
    return backgroundColor_;
}

void MasterConfiguration::setBackgroundColor(const QColor& color)
{
    backgroundColor_ = color;
}

void MasterConfiguration::setBackgroundUri(const QString& uri)
{
    backgroundUri_ = uri;
}

bool MasterConfiguration::save() const
{
    return save(filename_);
}

bool MasterConfiguration::save(const QString& filename) const
{
    QDomDocument doc("XmlDoc");
    QFile infile(filename_);
    if (!infile.open(QIODevice::ReadOnly))
    {
        put_flog(LOG_ERROR, "could not open configuration xml file for saving");
        return false;
    }
    doc.setContent(&infile);
    infile.close();

    QDomElement root = doc.documentElement();

    QDomElement background = root.firstChildElement("background");
    if (background.isNull())
    {
        background = doc.createElement("background");
        root.appendChild(background);
    }
    background.setAttribute("uri", backgroundUri_);
    background.setAttribute("color", backgroundColor_.name());

    QFile outfile(filename);
    if (!outfile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        put_flog(LOG_ERROR, "could not open configuration xml file for saving");
        return false;
    }
    QTextStream out(&outfile);
    out << doc.toString(4);
    outfile.close();
    return true;
}
