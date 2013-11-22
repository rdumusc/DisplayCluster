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

#ifndef GLOBALQTAPP_H
#define GLOBALQTAPP_H

#include <QApplication>

#include "globals.h"
#include "Options.h"
#include "configuration/MasterConfiguration.h"

#include "glxDisplay.h"

#define CONFIG_TEST_FILENAME  "configuration.xml"

// We need a global fixture because a bug in QApplication prevents
// deleting then recreating a QApplication in the same process.
// https://bugreports.qt-project.org/browse/QTBUG-7104
struct GlobalQtApp
{
    GlobalQtApp()
        : app( 0 )
    {
        if( !hasGLXDisplay( ))
          return;

        // need QApplication to instantiate WebkitPixelStreamer
        ut::master_test_suite_t& testSuite = ut::framework::master_test_suite();
        app = new QApplication( testSuite.argc, testSuite.argv );

        // To test wheel events the WebkitPixelStreamer needs access to the g_configuration element
        OptionsPtr options(new Options());
        g_configuration = new MasterConfiguration(CONFIG_TEST_FILENAME, options);
    }
    ~GlobalQtApp()
    {
        delete g_configuration;
        delete app;
    }

    QApplication* app;
};


#endif // GLOBALQTAPP_H
