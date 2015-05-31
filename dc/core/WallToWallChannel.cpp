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

#include "WallToWallChannel.h"

#include "MPIChannel.h"
#include "log.h"

#include <boost/date_time/posix_time/time_serialize.hpp>

#define RANK0 0

WallToWallChannel::WallToWallChannel(MPIChannelPtr mpiChannel)
    : mpiChannel_(mpiChannel)
{
}

int WallToWallChannel::getRank() const
{
    return mpiChannel_->getRank();
}

int WallToWallChannel::globalSum(const int localValue) const
{
    return mpiChannel_->globalSum(localValue);
}

bool WallToWallChannel::allReady(const bool isReady) const
{
    return mpiChannel_->globalSum(isReady ? 1 : 0) == mpiChannel_->getSize();
}

boost::posix_time::ptime WallToWallChannel::getTime() const
{
    return timestamp_;
}

void WallToWallChannel::synchronizeClock()
{
    if(mpiChannel_->getRank() == RANK0)
        sendClock();
    else
        receiveClock();
}

void WallToWallChannel::globalBarrier() const
{
    mpiChannel_->globalBarrier();
}

bool WallToWallChannel::checkVersion(const uint64_t version) const
{
    std::vector<uint64_t> versions = mpiChannel_->gatherAll(version);

    for (std::vector<uint64_t>::const_iterator it = versions.begin(); it != versions.end(); ++it)
    {
        if (*it != version)
            return false;
    }
    return true;
}

int WallToWallChannel::electLeader(const bool isCandidate)
{
    const int status = isCandidate ? (1 << getRank()) : 0;
    int globalStatus = globalSum(status);

    if(globalStatus <= 0)
        return -1;

    int leader = 0;
    while (globalStatus > 1)
    {
        globalStatus = globalStatus >> 1;
        ++leader;
    }
    return leader;
}

void WallToWallChannel::broadcast(boost::posix_time::time_duration timestamp)
{
    const std::string& serializedString = buffer_.serialize(timestamp);

    mpiChannel_->broadcast(MPI_MESSAGE_TYPE_TIMESTAMP, serializedString);
}

boost::posix_time::time_duration WallToWallChannel::receiveTimestampBroadcast(const int src)
{
    MPIHeader header = mpiChannel_->receiveHeader(src);
    assert(header.type == MPI_MESSAGE_TYPE_TIMESTAMP);

    buffer_.setSize(header.size);
    mpiChannel_->receiveBroadcast(buffer_.data(), buffer_.size(), src);

    boost::posix_time::time_duration timestamp;
    buffer_.deserialize(timestamp);
    return timestamp;
}

void WallToWallChannel::sendClock()
{
    assert(mpiChannel_->getRank() == RANK0);

    timestamp_ = boost::posix_time::ptime(boost::posix_time::microsec_clock::universal_time());

    const std::string& serializedString = buffer_.serialize(timestamp_);

    mpiChannel_->broadcast(MPI_MESSAGE_TYPE_FRAME_CLOCK, serializedString);
}

void WallToWallChannel::receiveClock()
{
    assert(mpiChannel_->getRank() != RANK0);

    MPIHeader header = mpiChannel_->receiveHeader(RANK0);
    assert(header.type == MPI_MESSAGE_TYPE_FRAME_CLOCK);

    buffer_.setSize(header.size);
    mpiChannel_->receiveBroadcast(buffer_.data(), buffer_.size(), RANK0);
    buffer_.deserialize(timestamp_);
}

