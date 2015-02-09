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

#ifndef WALLCONFIGURATION_H
#define WALLCONFIGURATION_H

#include "Configuration.h"

#include <QPoint>

/**
 * @brief The WallConfiguration class manages all the parameters needed
 * to setup a Wall process.
 */

class WallConfiguration : public Configuration
{
public:
    /**
     * Constructor.
     * @param filename \see Configuration
     * @param processIndex MPI index in the range [1;n] of the process
     * @throw std::runtime_error if the file could not be read
     */
    WallConfiguration(const QString &filename, const int processIndex);

    /** Get the name of the host on which this process is running. */
    const QString& getHost() const;

    /**
     * Get the display identifier in string format matching the current Linux
     * DISPLAY env_var.
     */
    const QString& getDisplay() const;

    /** Get the number of screens/GLWindows this process has to manage. */
    int getScreenCount() const;

    /**
     * Get the global index for the screen.
     * @param screenIndex index in the range [0,getScreenCount()-1]
     * @return index starting at {0,0} from the top-left
     */
    const QPoint& getGlobalScreenIndex( int screenIndex ) const;

    /** Get the coordinates of a screen in pixel units. */
    const QPoint& getWindowPos( int screenIndex ) const;

    /** Get the index of the process. */
    int getProcessIndex() const;

private:
    QString host_;
    QString display_;

    const int processIndex_;
    int screenCountForCurrentProcess_;
    std::vector<QPoint> screenPosition_;
    std::vector<QPoint> screenGlobalIndex_;

    void loadWallSettings(const int processIndex);
};

#endif // WALLCONFIGURATION_H
