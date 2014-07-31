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

#include "config.h"
#include "log.h"

#include "MPIChannel.h"
#include "WallApplication.h"
#include "MasterApplication.h"

#include <stdexcept>

#include <boost/scoped_ptr.hpp>
#include <QThreadPool>

#if ENABLE_TUIO_TOUCH_LISTENER
#include <X11/Xlib.h>
#endif

int main(int argc, char * argv[])
{
    MPIChannelPtr worldChannel(new MPIChannel(argc, argv));
    if (!worldChannel->isThreadSafe())
    {
        put_flog(LOG_FATAL, "MPI implementation must support at least "
                 "MPI_THREAD_SERIALIZED. (MPI_THREAD_MULTIPLE is recommended "
                 "for better performances)");
        return EXIT_FAILURE;
    }

    const int rank = worldChannel->getRank();
    MPIChannelPtr wallChannel(new MPIChannel(*worldChannel, rank != 0, rank));

#if ENABLE_TUIO_TOUCH_LISTENER
    // we need X multithreading support if we're running the
    // TouchListener thread and creating X events
    if (rank == 0)
        XInitThreads();
#endif

    boost::scoped_ptr<QApplication> app;
    try
    {
        if (rank == 0)
            app.reset(new MasterApplication(argc, argv, worldChannel));
        else
            app.reset(new WallApplication(argc, argv, worldChannel, wallChannel));
    }
    catch (const std::runtime_error& e)
    {
        put_flog(LOG_FATAL, "Could not initialize application. %s", e.what());
        return EXIT_FAILURE;
    }

    app->exec(); // enter Qt event loop

    put_flog(LOG_INFO, "quitting");
    QThreadPool::globalInstance()->waitForDone();

    return EXIT_SUCCESS;
}
