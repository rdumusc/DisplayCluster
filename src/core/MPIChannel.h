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

#ifndef MPICHANNEL_H
#define MPICHANNEL_H

#include "types.h"

#include <mpi.h>

struct MessageHeader;

/**
 * Handle MPI communications between all DisplayCluster instances.
 */
class MPIChannel
{
public:
    /**
     * Constructor, initialize the MPI communication.
     * Only one instance of this class per program is allowed.
     * @param argc main program arguments count
     * @param argv main program arguments
     */
    MPIChannel(int argc, char* argv[]);

    /** Destructor, finalize the MPI communication. */
    ~MPIChannel();

    /** Get the rank of this process. */
    int getRank() const;

    /** Block execution until all programs have reached the barrier. */
    void globalBarrier(const MPI_Comm mpiComm = MPI_COMM_WORLD) const;

    /**
     * Get the sum of the given local values across all processes.
     * @param localValue The value to sum
     * @return the sum of the localValues
     */
    int globalSum(const int localValue, const MPI_Comm mpiComm) const;

    // TODO cleanup the send/receive code
    bool messageAvailable();

    void send(const MessageHeader& messageHeader, const int dest);
    void send(const std::string& serializedData, const int dest);
    void broadcast(const std::string& serializedData, const MPI_Comm mpiComm = MPI_COMM_WORLD);

    MessageHeader receiveHeader(const int src, const MPI_Comm mpiComm = MPI_COMM_WORLD);
    void receiveBroadcast(char* dataBuffer, const size_t messageSize, const MPI_Comm mpiComm = MPI_COMM_WORLD);

    int mpiRank_;
    int mpiSize_;
    MPI_Comm mpiRenderComm_;
private:
};

#endif // MPICHANNEL_H
