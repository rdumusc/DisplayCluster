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

#include "WallFromMasterChannel.h"

#include "MPIChannel.h"
#include "DisplayGroup.h"
#include "ContentWindow.h"
#include "Options.h"
#include "Markers.h"
#include "PixelStreamFrame.h"

#define RANK0 0

WallFromMasterChannel::WallFromMasterChannel(MPIChannelPtr mpiChannel)
    : mpiChannel_(mpiChannel)
    , processMessages_(true)
{
}

bool WallFromMasterChannel::isMessageAvailable()
{
    return mpiChannel_->isMessageAvailable(RANK0);
}

void WallFromMasterChannel::receiveMessage()
{
    MPIHeader mh = mpiChannel_->receiveHeader(RANK0);

    switch (mh.type)
    {
    case MPI_MESSAGE_TYPE_DISPLAYGROUP:
        emit received(receiveBroadcast<DisplayGroupPtr>(mh.size));
        break;
    case MPI_MESSAGE_TYPE_OPTIONS:
        emit received(receiveBroadcast<OptionsPtr>(mh.size));
        break;
    case MPI_MESSAGE_TYPE_MARKERS:
        emit received(receiveBroadcast<MarkersPtr>(mh.size));
        break;
    case MPI_MESSAGE_TYPE_PIXELSTREAM:
        emit received(receiveBroadcast<PixelStreamFramePtr>(mh.size));
        break;
    case MPI_MESSAGE_TYPE_QUIT:
        processMessages_ = false;
        emit receivedQuit();
        break;
    default:
        break;
    }
}

void WallFromMasterChannel::processMessages()
{
    while(processMessages_)
        receiveMessage();
}

template <typename T>
T WallFromMasterChannel::receiveBroadcast(const size_t messageSize)
{
    T object;

    buffer_.setSize(messageSize);
    mpiChannel_->receiveBroadcast(buffer_.data(), messageSize, RANK0);
    buffer_.deserialize(object);

    return object;
}
