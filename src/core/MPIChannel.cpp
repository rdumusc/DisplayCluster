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

#define MPI_CHECK(func) {                                   \
    const int err = (func);                                 \
    if( err != MPI_SUCCESS )                                \
        put_flog(LOG_ERROR, "Error detected! (%d)", err);   \
    }

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

bool MPIChannel::isValid(const int dest) const
{
    return dest != mpiRank_ && dest >= 0 && dest < mpiSize_;
}

void MPIChannel::send(const MPIHeader& header, const int dest)
{
    if (!isValid(dest))
        return;

    MPI_CHECK(MPI_Send((void *)&header, sizeof(MPIHeader), MPI_BYTE, dest, 0, mpiComm_));
}

void MPIChannel::send(const MPIMessageType type, const std::string& serializedData, const int dest)
{
    if (!isValid(dest))
        return;

    MPI_CHECK(MPI_Send((void*)serializedData.data(), serializedData.size(), MPI_BYTE, dest, type, mpiComm_));
}

void MPIChannel::sendAll(const MPIMessageType type)
{
    MPIHeader mh;
    mh.size = 0;
    mh.type = type;

    for(int i=0; i<mpiSize_; ++i)
        send(mh, i);
}

void MPIChannel::broadcast(const MPIMessageType type, const std::string& serializedData)
{
    MPIHeader mh;
    mh.size = serializedData.size();
    mh.type = type;

    for(int i=0; i<mpiSize_; ++i)
        send(mh, i);

    MPI_CHECK(MPI_Bcast((void *)serializedData.data(), serializedData.size(),
                         MPI_BYTE, mpiRank_, mpiComm_));
}

MPIHeader MPIChannel::receiveHeader(const int src)
{
    MPI_Status status;
    MPIHeader mh;
    MPI_CHECK(MPI_Recv((void *)&mh, sizeof(MPIHeader), MPI_BYTE, src, 0, mpiComm_, &status));
    return mh;
}

ProbeResult MPIChannel::probe(const int src, const int tag)
{
    MPI_Status status;
    MPI_CHECK(MPI_Probe(src, tag, mpiComm_, &status));

    int count;
    MPI_CHECK(MPI_Get_count( &status, MPI_BYTE, &count));

    return ProbeResult { status.MPI_SOURCE, count, MPIMessageType(status.MPI_TAG) };
}

void MPIChannel::receive(char* dataBuffer, const size_t messageSize, const int src, const int tag)
{
    MPI_Status status;
    MPI_CHECK(MPI_Recv((void *)dataBuffer, messageSize, MPI_BYTE, src, tag, mpiComm_, &status));
}

void MPIChannel::receiveBroadcast(char* dataBuffer, const size_t messageSize, const int src)
{
    MPI_CHECK(MPI_Bcast((void *)dataBuffer, messageSize, MPI_BYTE, src, mpiComm_));
}

std::vector<uint64_t> MPIChannel::gatherAll(const uint64_t value)
{
    std::vector<uint64_t> results(mpiSize_);
    MPI_CHECK(MPI_Allgather((void *)&value, 1, MPI_LONG_LONG_INT,
                            (void *)results.data(), 1, MPI_LONG_LONG_INT, mpiComm_));

    return results;
}
