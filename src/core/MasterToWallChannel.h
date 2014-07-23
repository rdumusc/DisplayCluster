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

#ifndef MASTERTOWALLCHANNEL_H
#define MASTERTOWALLCHANNEL_H

#include "types.h"
#include "MPIHeader.h"
#include "SerializeBuffer.h"

#include <QObject>

/**
 * Communication channel between the master application and the wall processes.
 */
class MasterToWallChannel : public QObject
{
    Q_OBJECT

public:
    /** Constructor */
    MasterToWallChannel(MPIChannelPtr mpiChannel);

public slots:
    /**
     * Send the given DisplayGroup to the wall processes.
     * @param displayGroup The DisplayGroup to send
     */
    void send(DisplayGroupManagerPtr displayGroup);

    /**
     * Send the given Options to the wall processes.
     * @param options The options to send
     */
    void send(OptionsPtr options);

    /**
     * Send the given Markers to the wall processes.
     * @param markers The markers to send
     */
    void send(MarkersPtr markers);

    /**
     * Send pixel stream frame to the wall processes.
     * @param frame The frame to send
     */
    void send(PixelStreamFramePtr frame);

    /**
     * Send quit message to the wall processes, terminating the application.
     */
    void sendQuit();

private:
    MPIChannelPtr mpiChannel_;
    SerializeBuffer buffer_;

    template <typename T>
    void broadcast(const T& object, const MPIMessageType type);
};

#endif // MASTERTOWALLCHANNEL_H
