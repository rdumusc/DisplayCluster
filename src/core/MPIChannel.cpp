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

#include "MPIChannel.h"

#include "MessageHeader.h"

#include "log.h"

#define RANK0 0

MPIChannel::MPIChannel(int argc, char * argv[])
    : mpiRank_(-1)
    , mpiSize_(-1)
{
//    int required = MPI_THREAD_MULTIPLE;
//    int provided;
//    MPI_Init_thread(&argc, &argv, required, &provided);
//    if (provided < required)
//    {
//        put_flog(LOG_FATAL, "Error: MPI does not provide the required thread support. %d / %d\n", provided, required);
//        MPI_Abort(MPI_COMM_WORLD, 1);
//        exit(1);
//    }

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &mpiRank_);
    MPI_Comm_size(MPI_COMM_WORLD, &mpiSize_);
    MPI_Comm_split(MPI_COMM_WORLD, mpiRank_ != RANK0, mpiRank_, &mpiRenderComm_);
}

MPIChannel::~MPIChannel()
{
    MPI_Comm_free(&mpiRenderComm_);
    MPI_Finalize();
}

int MPIChannel::getRank() const
{
    return mpiRank_;
}

void MPIChannel::globalBarrier(const MPI_Comm mpiComm) const
{
    MPI_Barrier(mpiComm);
}

int MPIChannel::globalSum(const int localValue, const MPI_Comm mpiComm) const
{
    int globalValue = 0;
    MPI_Allreduce((void *)&localValue, (void *)&globalValue,
                  1, MPI_INT, MPI_SUM, mpiComm);
    return globalValue;
}

bool MPIChannel::messageAvailable()
{
    // check to see if we have a message (non-blocking)
    int flag;
    MPI_Status status;
    MPI_Iprobe(RANK0, 0, MPI_COMM_WORLD, &flag, &status);

    // check to see if all render processes have a message
    int allFlag;
    MPI_Allreduce(&flag, &allFlag, 1, MPI_INT, MPI_LAND, mpiRenderComm_);

    return (bool)allFlag;
}

void MPIChannel::send(const MessageHeader& messageHeader, const int dest)
{
    MPI_Send((void *)&messageHeader, sizeof(MessageHeader),
             MPI_BYTE, dest, 0, MPI_COMM_WORLD);
}

void MPIChannel::send(const std::string& serializedData, const int dest)
{
    MPI_Send((void *)serializedData.data(), (int)serializedData.size(),
             MPI_BYTE, dest, 0, MPI_COMM_WORLD);
}

void MPIChannel::broadcast(const std::string& serializedData, const MPI_Comm mpiComm)
{
    MPI_Bcast((void *)serializedData.data(), serializedData.size(),
              MPI_BYTE, RANK0, mpiComm);
}

MessageHeader MPIChannel::receiveHeader(const int src, const MPI_Comm mpiComm)
{
    MPI_Status status;
    MessageHeader mh;
    MPI_Recv((void *)&mh, sizeof(MessageHeader), MPI_BYTE, src, 0, mpiComm, &status);
    return mh;
}

void MPIChannel::receiveBroadcast(char* dataBuffer, const size_t messageSize, const MPI_Comm mpiComm)
{
    MPI_Bcast((void *)dataBuffer, messageSize, MPI_BYTE, RANK0, mpiComm);
}
