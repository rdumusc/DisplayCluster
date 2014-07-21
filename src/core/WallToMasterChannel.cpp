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

#include "WallToMasterChannel.h"

#include "MPIChannel.h"
#include "DisplayGroupManager.h"
#include "Options.h"
#include "Markers.h"
#include "PixelStreamFrame.h"

// Will be removed when implementing DISCL-21
#include "ContentWindowManager.h"
#include "Content.h"
#include "Factories.h"
#include "log.h"

WallToMasterChannel::WallToMasterChannel(MPIChannelPtr mpiChannel)
    : mpiChannel_(mpiChannel)
    , processMessages_(true)
{
}

bool WallToMasterChannel::isMessageAvailable()
{
    return mpiChannel_->isMessageAvailable(0);
}

void WallToMasterChannel::receiveMessage()
{
    MPIHeader mh = mpiChannel_->receiveHeader(0);

    switch (mh.type)
    {
    case MPI_MESSAGE_TYPE_DISPLAYGROUP:
        displayGroup_ = receiveBroadcast<DisplayGroupManagerPtr>(mh.size);
        emit received(displayGroup_);
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
    case MPI_MESSAGE_TYPE_CONTENTS_DIMENSIONS:
        sendContentsDimensionsReply();
        break;
    default:
        break;
    }
}

void WallToMasterChannel::setFactories(FactoriesPtr factories)
{
    factories_ = factories;
}

void WallToMasterChannel::processMessages()
{
    while(processMessages_)
        receiveMessage();
}

template <typename T>
T WallToMasterChannel::receiveBroadcast(const size_t messageSize)
{
    T object;

    buffer_.setSize(messageSize);
    mpiChannel_->receiveBroadcast(buffer_.data(), messageSize);
    buffer_.deserialize(object);

    return object;
}

void WallToMasterChannel::sendContentsDimensionsReply()
{
    if(mpiChannel_->getRank() != 1)
        return;

    if (!displayGroup_)
    {
        put_flog(LOG_ERROR, "Cannot send content dimensions before a DisplayGroup was received!");
        return;
    }
    if (!factories_)
    {
        put_flog(LOG_FATAL, "Cannot send content dimensions before setFactories was called!");
        return;
    }

    ContentWindowManagerPtrs contentWindows = displayGroup_->getContentWindowManagers();

    std::vector<std::pair<int, int> > dimensions;

    for(size_t i=0; i<contentWindows.size(); ++i)
    {
        int w,h;
        FactoryObjectPtr object = factories_->getFactoryObject(contentWindows[i]->getContent());
        object->getDimensions(w, h);

        dimensions.push_back(std::pair<int,int>(w,h));
    }

    const std::string& serializedString = buffer_.serialize(dimensions);

    mpiChannel_->send(MPI_MESSAGE_TYPE_CONTENTS_DIMENSIONS, serializedString, 0);
}
