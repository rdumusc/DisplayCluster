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
#include "MessageHeader.h"
#include "log.h"

WallToWallChannel::WallToWallChannel(MPIChannelPtr mpiChannel)
    : mpiChannel_(mpiChannel)
{
}

int WallToWallChannel::globalSum(const int localValue) const
{
    return mpiChannel_->globalSum(localValue, mpiChannel_->mpiRenderComm_);
}

boost::posix_time::ptime WallToWallChannel::getTime() const
{
    return timestamp_;
}

void WallToWallChannel::synchronizeClock()
{
    if(mpiChannel_->getRank() == 1)
        sendClock();
    else
        receiveClock();
}

void WallToWallChannel::globalBarrier() const
{
    mpiChannel_->globalBarrier(mpiChannel_->mpiRenderComm_);
}

void WallToWallChannel::sendClock()
{
    if(mpiChannel_->getRank() != 1)
    {
        put_flog(LOG_WARN, "called by rank %i != 1", mpiChannel_->getRank());
        return;
    }

    timestamp_ = boost::posix_time::ptime(boost::posix_time::microsec_clock::universal_time());

    const std::string& serializedString = buffer_.serialize(timestamp_);

    MessageHeader mh;
    mh.size = serializedString.size();
    mh.type = MESSAGE_TYPE_FRAME_CLOCK;

    // the header is sent via a send, so that we can probe it on the render processes
    for(int i=2; i<mpiChannel_->mpiSize_; i++)
        mpiChannel_->send(mh, i);

    mpiChannel_->broadcast(serializedString, mpiChannel_->mpiRenderComm_);
}

void WallToWallChannel::receiveClock()
{
    if(mpiChannel_->getRank() == 1)
        return;

    MessageHeader messageHeader = mpiChannel_->receiveHeader(1, MPI_COMM_WORLD);
    if(messageHeader.type != MESSAGE_TYPE_FRAME_CLOCK)
    {
        put_flog(LOG_FATAL, "unexpected message type");
        return;
    }

    buffer_.setSize(messageHeader.size);
    mpiChannel_->receiveBroadcast(buffer_.data(), buffer_.size(), mpiChannel_->mpiRenderComm_);
    buffer_.deserialize(timestamp_);
}

