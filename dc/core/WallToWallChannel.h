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

#ifndef WALLTOWALLCHANNEL_H
#define WALLTOWALLCHANNEL_H

#include "types.h"
#include "SerializeBuffer.h"

#include <QObject>
#include <boost/date_time/posix_time/posix_time.hpp>

/**
 * Communication channel between the Wall processes.
 */
class WallToWallChannel : public QObject
{
    Q_OBJECT

public:
    /** Constructor */
    WallToWallChannel(MPIChannelPtr mpiChannel);

    /** @return The rank of this process. */
    int getRank() const;

    /**
     * Get the sum of the given local values across all processes.
     * @param localValue The value to sum
     * @return the sum of the localValues
     */
    int globalSum(const int localValue) const;

    /** Check if all processes are ready to perform a common action. */
    bool allReady(const bool isReady) const;

    /** Get the current timestamp, synchronized accross processes. */
    boost::posix_time::ptime getTime() const;

    /** Synchronize clock time across all processes. */
    void synchronizeClock();

    /** Block execution until all programs have reached the barrier. */
    void globalBarrier() const;

    /** Check that all processes have the same version of an object. */
    bool checkVersion(const uint64_t version) const;

    /**
     * Elect a leader amongst wall processes.
     * @param isCandidate Is this process a candidate.
     * @return the rank of the leader, or -1 if no leader could be elected.
     */
    int electLeader(const bool isCandidate);

    /**
     * Broadcast a timestamp.
     * All other processes must recieve it with receiveTimestampBroadcast().
     */
    void broadcast(boost::posix_time::time_duration timestamp);

    /** Receive a timestamp broadcasted by broadcast(timestamp). */
    boost::posix_time::time_duration receiveTimestampBroadcast(const int src);

private:
    MPIChannelPtr mpiChannel_;
    boost::posix_time::ptime timestamp_;
    SerializeBuffer buffer_;

    void sendClock();
    void receiveClock();
};

#endif // WALLTOWALLCHANNEL_H
