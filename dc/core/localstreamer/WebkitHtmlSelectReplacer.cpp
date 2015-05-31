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

#include "WebkitHtmlSelectReplacer.h"

#include <QWebPage>
#include <QWebFrame>
#include <QFile>

// The SelectBoxIt scripts are developed and released by Greg Franko under MIT license
// Project page: http://gregfranko.com/jquery.selectBoxIt.js/
#define JQUERY_RESOURCE_FILE           ":/selectboxit/jquery.min.js"
#define JQUERY_UI_RESOURCE_FILE        ":/selectboxit/jquery-ui.min.js"
#define SELECTBOXIT_JS_RESOURCE_FILE   ":/selectboxit/jquery.selectBoxIt.min.js"
#define SELECTBOXIT_CSS_RESOURCE_FILE  ":/selectboxit/selectboxit.css"

WebkitHtmlSelectReplacer::WebkitHtmlSelectReplacer(QWebView& webView)
    : webView_(webView)
{
    connect(&webView_, SIGNAL(loadFinished(bool)), this, SLOT(pageLoaded(bool)));
}

void WebkitHtmlSelectReplacer::pageLoaded(bool success)
{
    if(!success)
        return;

    loadScripts();

    replaceAllSelectElements();
}

void WebkitHtmlSelectReplacer::loadScripts()
{
    if (!hasJQuery())
        loadJavascript(JQUERY_RESOURCE_FILE);

    if (!hasJQueryUi())
        loadJavascript(JQUERY_UI_RESOURCE_FILE);

    loadJavascript(SELECTBOXIT_JS_RESOURCE_FILE);
    loadCssUsingJQuery(SELECTBOXIT_CSS_RESOURCE_FILE);
}

void WebkitHtmlSelectReplacer::replaceAllSelectElements()
{
    QString js("var selectBox = $(\"select\").selectBoxIt();");

    webView_.page()->mainFrame()->evaluateJavaScript(js);
}

bool WebkitHtmlSelectReplacer::hasJQuery()
{
    QString js("var hasJquery = false;"
               "if( window.jQuery ) {"
               "  hasJquery = true;"
               "}");

    return webView_.page()->mainFrame()->evaluateJavaScript(js).toBool();
}

bool WebkitHtmlSelectReplacer::hasJQueryUi()
{
    QString js("var hasJqueryUi = false;"
               "if( window.jQuery.ui ) {"
               "  hasJqueryUi = true;"
               "}");

    return webView_.page()->mainFrame()->evaluateJavaScript(js).toBool();
}

void WebkitHtmlSelectReplacer::loadJavascript(const QString& jsFile)
{
    QFile file(jsFile);
    file.open(QIODevice::ReadOnly);
    QString jQuery = file.readAll();
    file.close();

    webView_.page()->mainFrame()->evaluateJavaScript(jQuery);
}

void WebkitHtmlSelectReplacer::loadCssUsingJQuery(const QString& cssFile)
{
    QFile file(cssFile);
    file.open(QIODevice::ReadOnly);
    QString cssStyle = file.readAll();
    file.close();

    cssStyle.remove(QRegExp("[\\n\\t\\r]"));

    QString js = QString("loadCSS = function(href) {"
                         "  var cssStyle = $(\"<style> %1 </style>\");"
                         "  $(\"head\").append(cssStyle);"
                         "};"
                         "loadCSS();"
                         ).arg(cssStyle);

    webView_.page()->mainFrame()->evaluateJavaScript(js);
}
