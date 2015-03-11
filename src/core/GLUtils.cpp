/*********************************************************************/
/* Copyright (c) 2015, EPFL/Blue Brain Project                       */
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

#include "GLUtils.h"

#include "log.h"

#include <boost/bind.hpp>
#include <boost/function.hpp>

#include <QtCore/QtGlobal>
#if defined( Q_OS_WIN )
#include <GL/wglext.h>
#elif defined( Q_OS_LINUX )
#include <QtGui/QApplication>
#include <QtGui/QDesktopWidget>
#include <GL/glx.h>
#include <GL/glxext.h>
#endif

/** A function to set the swap interaval on an OpenGL window. */
typedef boost::function< int ( int ) > GLSwapIntervalFunction;

GLSwapIntervalFunction getSwapIntervalFunc( const QWidget* window )
{
#ifndef Q_OS_LINUX
    Q_UNUSED( window );
#endif

    bool condition = false;
    void* func = 0;

#if defined( Q_OS_WIN )
    const std::string ext(((PFNWGLGETEXTENSIONSSTRINGEXTPROC)
                           wglGetProcAddress( "wglGetExtensionsStringEXT" ))());
    condition = ext.find( "WGL_EXT_swap_control" ) != std::string::npos;
    func = (void*)wglGetProcAddress( "wglSwapIntervalEXT" );
#elif defined( Q_OS_LINUX )
    Display* display = glXGetCurrentDisplay();
    const int screen = qApp->desktop()->screenNumber( window );
    const std::string ext( glXQueryExtensionsString( display, screen ));
    condition = ext.find( "GLX_SGI_swap_control" ) != std::string::npos;
    func = (void*)glXGetProcAddress( (const GLubyte*)"glXSwapIntervalSGI" );
#endif

    if( condition && func != 0 )
    {
#if defined( Q_OS_WIN )
        return boost::bind( (PFNWGLSWAPINTERVALEXTPROC)func, _1 );
#elif defined( Q_OS_LINUX )
        return boost::bind( (PFNGLXSWAPINTERVALSGIPROC)func, _1 );
#endif
    }

    put_log( LOG_WARN, "disabling vsync not available" );
    return GLSwapIntervalFunction();
}

bool GLUtils::setEnableVSync( const QWidget* window, const bool enabled )
{
    GLSwapIntervalFunction setSwapInterval = getSwapIntervalFunc( window );
    if( setSwapInterval.empty( ))
        return false;

    setSwapInterval( enabled ? 1 : 0 );
    return true;
}
