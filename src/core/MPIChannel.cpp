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

#include "MPIContext.h"
#include "MessageHeader.h"

#include "log.h"

#define RANK0 0

MPIChannel::MPIChannel(int argc, char * argv[])
    : mpiContext_(new MPIContext(argc, argv))
    , mpiComm_(MPI_COMM_WORLD)
    , mpiRank_(-1)
    , mpiSize_(-1)
{
    MPI_Comm_rank(MPI_COMM_WORLD, &mpiRank_);
    MPI_Comm_size(MPI_COMM_WORLD, &mpiSize_);
}

MPIChannel::MPIChannel(const MPIChannel& parent, const int color, const int key)
    : mpiContext_(parent.mpiContext_)
    , mpiComm_(MPI_COMM_WORLD)
    , mpiRank_(-1)
    , mpiSize_(-1)
{
    MPI_Comm_split(parent.mpiComm_, color, key, &mpiComm_);
    MPI_Comm_rank(mpiComm_, &mpiRank_);
    MPI_Comm_size(mpiComm_, &mpiSize_);
}

MPIChannel::~MPIChannel()
{
    if (mpiComm_ != MPI_COMM_WORLD)
        MPI_Comm_free(&mpiComm_);
}

int MPIChannel::getRank() const
{
    return mpiRank_;
}

int MPIChannel::getSize() const
{
    return mpiSize_;
}

bool MPIChannel::isThreadSafe() const
{
    return mpiContext_->hasMultithreadSupport();
}

void MPIChannel::globalBarrier() const
{
    MPI_Barrier(mpiComm_);
}

int MPIChannel::globalSum(const int localValue) const
{
    int globalValue = 0;
    MPI_Allreduce((void *)&localValue, (void *)&globalValue,
                  1, MPI_INT, MPI_SUM, mpiComm_);
    return globalValue;
}

bool MPIChannel::isMessageAvailable(const int src)
{
    int flag;
    MPI_Status status;
    MPI_Iprobe(src, 0, mpiComm_, &flag, &status);

    return (bool)flag;
}

void MPIChannel::send(const MPIHeader& header, const int dest)
{
    int err = MPI_Send((void *)&header, sizeof(MPIHeader), MPI_BYTE, dest, 0, mpiComm_);

    if (err != MPI_SUCCESS)
        put_flog(LOG_ERROR, "Error detected! (%d)", err);
}

void MPIChannel::send(const MPIMessageType type, const std::string& serializedData, const int dest)
{
    MPIHeader mh;
    mh.size = serializedData.size();
    mh.type = type;

    send(mh, dest);
}

void MPIChannel::sendAll(const MPIMessageType type)
{
    MPIHeader mh;
    mh.size = 0;
    mh.type = type;

    for(int i=1; i<mpiSize_; ++i)
        send(mh, i);
}

void MPIChannel::broadcast(const MPIMessageType type, const std::string& serializedData)
{
    MPIHeader mh;
    mh.size = serializedData.size();
    mh.type = type;

    for(int i=1; i<mpiSize_; ++i)
        send(mh, i);

    int err = MPI_Bcast((void *)serializedData.data(), serializedData.size(),
                         MPI_BYTE, RANK0, mpiComm_);

    if (err != MPI_SUCCESS)
        put_flog(LOG_ERROR, "Error detected! (%d)", err);
}

MPIHeader MPIChannel::receiveHeader(const int src)
{
    MPI_Status status;
    MPIHeader mh;
    MPI_Recv((void *)&mh, sizeof(MPIHeader), MPI_BYTE, src, 0, mpiComm_, &status);
    return mh;

    if (status.MPI_ERROR != MPI_SUCCESS)
        put_flog(LOG_ERROR, "Error detected! (%d)", status.MPI_ERROR);
}

void MPIChannel::receive(char* dataBuffer, const size_t messageSize, const int src)
{
    MPI_Status status;
    MPI_Recv((void *)dataBuffer, messageSize, MPI_BYTE, src, 0, mpiComm_, &status);

    if (status.MPI_ERROR != MPI_SUCCESS)
        put_flog(LOG_ERROR, "Error detected! (%d)", status.MPI_ERROR);
}

void MPIChannel::receiveBroadcast(char* dataBuffer, const size_t messageSize)
{
    int err = MPI_Bcast((void *)dataBuffer, messageSize, MPI_BYTE, RANK0, mpiComm_);

    if (err != MPI_SUCCESS)
        put_flog(LOG_ERROR, "Error detected! (%d)", err);
}

std::vector<uint64_t> MPIChannel::gatherAll(const uint64_t value)
{
    std::vector<uint64_t> results(mpiSize_);
    int err = MPI_Allgather((void *)&value, 1, MPI_LONG_LONG_INT,
                            (void *)results.data(), 1, MPI_LONG_LONG_INT, mpiComm_);

    if (err != MPI_SUCCESS)
        put_flog(LOG_ERROR, "Error detected! (%d)", err);

    return results;
}
