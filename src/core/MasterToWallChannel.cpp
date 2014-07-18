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

#include "MasterToWallChannel.h"

#include "MessageHeader.h"
#include "MPIChannel.h"

#include "DisplayGroupManager.h"
#include "ContentWindowManager.h"
#include "Options.h"
#include "Markers.h"
#include "PixelStreamFrame.h"

#include <mpi.h>

MasterToWallChannel::MasterToWallChannel(MPIChannelPtr mpiChannel)
    : mpiChannel_(mpiChannel)
{
}

template <typename T>
void MasterToWallChannel::broadcast(const T object, const MessageType type)
{
    const std::string& serializedString = buffer_.serialize(object);

    MessageHeader mh;
    mh.size = serializedString.size();
    mh.type = type;

    // Send header via a send so we can probe it on the render processes
    for(int i=1; i<mpiChannel_->mpiSize_; ++i)
        mpiChannel_->send(mh, i);

    mpiChannel_->broadcast(serializedString);
}

void MasterToWallChannel::send(DisplayGroupManagerPtr displayGroup)
{
    broadcast(displayGroup, MESSAGE_TYPE_CONTENTS);
}

void MasterToWallChannel::send(OptionsPtr options)
{
    broadcast(options, MESSAGE_TYPE_OPTIONS);
}

void MasterToWallChannel::send(MarkersPtr markers)
{
    broadcast(markers, MESSAGE_TYPE_MARKERS);
}

void MasterToWallChannel::send(PixelStreamFramePtr frame)
{
    assert(!frame->segments.empty() && "received an empty frame");
    broadcast(frame, MESSAGE_TYPE_PIXELSTREAM);
}

void MasterToWallChannel::sendQuit()
{
    MessageHeader mh;
    mh.type = MESSAGE_TYPE_QUIT;

    // Send header via a send so that we can probe it on the render processes
    for(int i=1; i<mpiChannel_->mpiSize_; ++i)
        mpiChannel_->send(mh, i);
}

void MasterToWallChannel::sendContentsDimensionsRequest(ContentWindowManagerPtrs contentWindows)
{
    MessageHeader mh;
    mh.type = MESSAGE_TYPE_CONTENTS_DIMENSIONS;

    // the header is sent via a send, so that we can probe it on the render processes
    for(int i=1; i<mpiChannel_->mpiSize_; i++)
        mpiChannel_->send(mh, i);

    // now, receive response from rank 1
    receiveContentsDimensionsReply(contentWindows);
}

void MasterToWallChannel::receiveContentsDimensionsReply(ContentWindowManagerPtrs contentWindows)
{
    MPI_Status status;
    MessageHeader mh = mpiChannel_->receiveHeader(1, MPI_COMM_WORLD);

    buffer_.setSize(mh.size);
    MPI_Recv((void *)buffer_.data(), buffer_.size(), MPI_BYTE, 1, 0, MPI_COMM_WORLD, &status);

    std::vector<std::pair<int, int> > dimensions;
    buffer_.deserialize(dimensions);

    // overwrite old dimensions
    for(size_t i=0; i<dimensions.size() && i<contentWindows.size(); ++i)
        contentWindows[i]->getContent()->setDimensions(dimensions[i].first, dimensions[i].second);
}
