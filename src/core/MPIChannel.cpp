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
#include "DisplayGroupManager.h"
#include "Options.h"
#include "Markers.h"
#include "PixelStreamFrame.h"

#include "log.h"

#include <boost/serialization/vector.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/date_time/posix_time/time_serialize.hpp>

// Will be removed when implementing DISCL-21
#include "ContentWindowManager.h"
#include "Content.h"
#include "Factories.h"

#define RANK0 0

template <typename T>
inline std::string serialize(T& object)
{
    std::ostringstream oss(std::ostringstream::binary);
    {
        boost::archive::binary_oarchive oa(oss);
        oa << object;
    }
    return oss.str();
}

template <typename T>
void deserialize(T& object, std::vector<char>& buffer)
{
    std::istringstream iss(std::istringstream::binary);
    iss.rdbuf()->pubsetbuf(buffer.data(), buffer.size());

    boost::archive::binary_iarchive ia(iss);
    ia >> object;
}

template <typename T>
void receiveBroadcast(T& object, const size_t messageSize, const MPI_Comm mpiComm = MPI_COMM_WORLD)
{
    std::vector<char> buffer(messageSize);
    MPI_Bcast((void *)buffer.data(), buffer.size(), MPI_BYTE, RANK0, mpiComm);

    deserialize(object, buffer);
}

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

void MPIChannel::globalBarrier() const
{
    MPI_Barrier(mpiRenderComm_);
}

int MPIChannel::globalSum(const int localValue) const
{
    int globalValue = 0;
    MPI_Allreduce((void *)&localValue, (void *)&globalValue,
                  1, MPI_INT, MPI_SUM, mpiRenderComm_);
    return globalValue;
}

boost::posix_time::ptime MPIChannel::getTime() const
{
    // rank 0 will return a timestamp calibrated to rank 1's clock
    if(mpiRank_ == 0)
        return boost::posix_time::microsec_clock::universal_time() + timestampOffset_;

    return timestamp_;
}

void MPIChannel::calibrateTimestampOffset()
{
    if(mpiSize_ < 2)
    {
        put_flog(LOG_DEBUG, "minimum 2 processes needed! (mpiSize == %i)", mpiSize_);
        return;
    }

    // synchronize all processes
    MPI_Barrier(mpiRenderComm_);

    // get current timestamp immediately after
    boost::posix_time::ptime timestamp(boost::posix_time::microsec_clock::universal_time());

    // rank 1: send timestamp to rank 0
    if(mpiRank_ == 1)
    {
        const std::string& serializedString = serialize(timestamp);

        // send the header and the message
        MessageHeader mh;
        mh.size = serializedString.size();

        send(mh, 0);
        send(serializedString, 0);
    }
    // rank 0: receive timestamp from rank 1
    else if(mpiRank_ == 0)
    {
        MessageHeader messageHeader;

        MPI_Status status;
        MPI_Recv((void *)&messageHeader, sizeof(MessageHeader), MPI_BYTE, 1, 0, MPI_COMM_WORLD, &status);

        std::vector<char> buffer(messageHeader.size);
        MPI_Recv((void *)buffer.data(), buffer.size(), MPI_BYTE, 1, 0, MPI_COMM_WORLD, &status);

        boost::posix_time::ptime rank1Timestamp;
        deserialize(rank1Timestamp, buffer);

        timestampOffset_ = rank1Timestamp - timestamp;

        put_flog(LOG_DEBUG, "timestamp offset = %s", (boost::posix_time::to_simple_string(timestampOffset_)).c_str());
    }
}

void MPIChannel::receiveMessages()
{
    if(mpiRank_ == RANK0)
    {
        put_flog(LOG_FATAL, "called on rank 0");
        return;
    }

    // check to see if we have a message (non-blocking)
    int flag;
    MPI_Status status;
    MPI_Iprobe(RANK0, 0, MPI_COMM_WORLD, &flag, &status);

    // check to see if all render processes have a message
    int allFlag;
    MPI_Allreduce(&flag, &allFlag, 1, MPI_INT, MPI_LAND, mpiRenderComm_);

//    static int counter = 0;
//    if(mpiRank_ == 1)
//    {
//        if (allFlag)
//            put_flog(LOG_FATAL, "Received message %d", counter++);
//        else
//            put_flog(LOG_FATAL, "No message. Last %d", counter);
//    }

    // continue receiving messages until we get to the last one which all render processes have
    // this will "drop frames" and keep all processes synchronized
    while(allFlag)
    {
        MessageHeader mh;
        MPI_Recv((void *)&mh, sizeof(MessageHeader), MPI_BYTE, RANK0, 0, MPI_COMM_WORLD, &status);

        if(mh.type == MESSAGE_TYPE_CONTENTS)
        {
            displayGroup_ = receiveDisplayGroup(mh);
            emit(received(displayGroup_));
        }
        else if(mh.type == MESSAGE_TYPE_OPTIONS)
        {
            emit(received(receiveOptions(mh)));
        }
        else if(mh.type == MESSAGE_TYPE_MARKERS)
        {
            emit(received(receiveMarkers(mh)));
        }
        else if(mh.type == MESSAGE_TYPE_CONTENTS_DIMENSIONS)
        {
            receiveContentsDimensionsRequest();
        }
        else if(mh.type == MESSAGE_TYPE_PIXELSTREAM)
        {
            receivePixelStreams(mh);
        }
        else if(mh.type == MESSAGE_TYPE_QUIT)
        {
            QApplication::instance()->quit();
            return;
        }

        // check to see if we have another message waiting for this process
        // and for all render processes
        MPI_Iprobe(RANK0, 0, MPI_COMM_WORLD, &flag, &status);
        MPI_Allreduce(&flag, &allFlag, 1, MPI_INT, MPI_LAND, mpiRenderComm_);

//    if(mpiRank_ == 1)
//    {
//        if (allFlag)
//            put_flog(LOG_FATAL, "Received message %d", counter++);
//        else
//            put_flog(LOG_FATAL, "No message. Last %d", counter);
//    }
    }
    // at this point, we've received the last message available for all render processes
}

void MPIChannel::send(DisplayGroupManagerPtr displayGroup)
{
    const std::string& serializedString = serialize(displayGroup);

    MessageHeader mh;
    mh.size = serializedString.size();
    mh.type = MESSAGE_TYPE_CONTENTS;

    // Send header via a send so we can probe it on the render processes
    for(int i=1; i<mpiSize_; ++i)
        send(mh, i);

    broadcast(serializedString);
}

void MPIChannel::send(OptionsPtr options)
{
    const std::string& serializedString = serialize(options);

    MessageHeader mh;
    mh.size = serializedString.size();
    mh.type = MESSAGE_TYPE_OPTIONS;

    // Send header via a send so we can probe it on the render processes
    for(int i=1; i<mpiSize_; ++i)
        send(mh, i);

    broadcast(serializedString);
}

void MPIChannel::send(MarkersPtr markers)
{
    const std::string& serializedString = serialize(markers);

    MessageHeader mh;
    mh.size = serializedString.size();
    mh.type = MESSAGE_TYPE_MARKERS;

    // Send header via a send so we can probe it on the render processes
    for(int i=1; i<mpiSize_; ++i)
        send(mh, i);

    broadcast(serializedString);
}

void MPIChannel::sendContentsDimensionsRequest(ContentWindowManagerPtrs contentWindows)
{
    if(mpiSize_ < 2)
    {
        put_flog(LOG_WARN, "cannot get contents dimension update for mpiSize == %i", mpiSize_);
        return;
    }

    if(mpiRank_ != RANK0)
    {
        put_flog(LOG_ERROR, "called on rank: %i != 0", mpiRank_);
        return;
    }

    // send the header and the message
    MessageHeader mh;
    mh.type = MESSAGE_TYPE_CONTENTS_DIMENSIONS;

    // the header is sent via a send, so that we can probe it on the render processes
    for(int i=1; i<mpiSize_; i++)
        send(mh, i);

    // now, receive response from rank 1
    MPI_Status status;
    MPI_Recv((void *)&mh, sizeof(MessageHeader), MPI_BYTE, 1, 0, MPI_COMM_WORLD, &status);

    // receive serialized data
    std::vector<char> buffer(mh.size);
    MPI_Recv((void *)buffer.data(), mh.size, MPI_BYTE, 1, 0, MPI_COMM_WORLD, &status);

    // read to a new vector
    std::vector<std::pair<int, int> > dimensions;
    deserialize(dimensions, buffer);

    // overwrite old dimensions
    for(size_t i=0; i<dimensions.size() && i<contentWindows.size(); ++i)
        contentWindows[i]->getContent()->setDimensions(dimensions[i].first, dimensions[i].second);
}

void MPIChannel::setFactories(FactoriesPtr factories)
{
    factories_ = factories;
}

void MPIChannel::synchronizeClock()
{
    if(getRank() == 1)
        sendFrameClockUpdate();
    else
        receiveFrameClockUpdate();
}

void MPIChannel::sendFrameClockUpdate()
{
    // this should only be called by the rank 1 process
    if(mpiRank_ != 1)
    {
        put_flog(LOG_WARN, "called by rank %i != 1", mpiRank_);
        return;
    }

    timestamp_ = boost::posix_time::ptime(boost::posix_time::microsec_clock::universal_time());

    const std::string& serializedString = serialize(timestamp_);

    // send the header and the message
    MessageHeader mh;
    mh.size = serializedString.size();
    mh.type = MESSAGE_TYPE_FRAME_CLOCK;

    // the header is sent via a send, so that we can probe it on the render processes
    for(int i=2; i<mpiSize_; i++)
        send(mh, i);

    broadcast(serializedString, mpiRenderComm_);
}

void MPIChannel::receiveFrameClockUpdate()
{
    // we shouldn't receive the broadcast if we're rank 1
    if(mpiRank_ == 1)
        return;

    // receive the message header
    MessageHeader messageHeader;
    MPI_Status status;
    MPI_Recv((void *)&messageHeader, sizeof(MessageHeader), MPI_BYTE, 1, 0, MPI_COMM_WORLD, &status);

    if(messageHeader.type != MESSAGE_TYPE_FRAME_CLOCK)
    {
        put_flog(LOG_FATAL, "unexpected message type");
        return;
    }

    receiveBroadcast(timestamp_, messageHeader.size, mpiRenderComm_);
}

void MPIChannel::sendQuit()
{
    MessageHeader mh;
    mh.type = MESSAGE_TYPE_QUIT;

    // Send header via a send so that we can probe it on the render processes
    for(int i=1; i<mpiSize_; ++i)
        send(mh, i);
}

DisplayGroupManagerPtr MPIChannel::receiveDisplayGroup(const MessageHeader& messageHeader)
{
    if(mpiRank_ < 1)
    {
        put_flog(LOG_WARN, "called on rank %i < 1", mpiRank_);
        return DisplayGroupManagerPtr();
    }

    DisplayGroupManagerPtr displayGroup;
    receiveBroadcast(displayGroup, messageHeader.size);

    return displayGroup;
}

OptionsPtr MPIChannel::receiveOptions(const MessageHeader& messageHeader)
{
    if(mpiRank_ < 1)
    {
        put_flog(LOG_WARN, "called on rank %i < 1", mpiRank_);
        return OptionsPtr();
    }

    OptionsPtr options;
    receiveBroadcast(options, messageHeader.size);

    return options;
}

MarkersPtr MPIChannel::receiveMarkers(const MessageHeader& messageHeader)
{
    if(mpiRank_ < 1)
    {
        put_flog(LOG_WARN, "called on rank %i < 1", mpiRank_);
        return MarkersPtr();
    }

    MarkersPtr markers;
    receiveBroadcast(markers, messageHeader.size);

    return markers;
}

void MPIChannel::receiveContentsDimensionsRequest()
{
    if(mpiRank_ != 1)
        return;

    ContentWindowManagerPtrs contentWindows;
    if (displayGroup_)
        contentWindows = displayGroup_->getContentWindowManagers();
    else
        put_flog(LOG_ERROR, "Cannot send content dimensions before a DisplayGroup was received!");

    if (!factories_)
    {
        put_flog(LOG_FATAL, "Cannot send content dimensions before setFactories was called!");
        return;
    }

    std::vector<std::pair<int, int> > dimensions;

    for(size_t i=0; i<contentWindows.size(); ++i)
    {
        int w,h;
        FactoryObjectPtr object = factories_->getFactoryObject(contentWindows[i]->getContent());
        object->getDimensions(w, h);

        dimensions.push_back(std::pair<int,int>(w,h));
    }

    const std::string& serializedString = serialize(dimensions);

    // send the header and the message
    MessageHeader mh;
    mh.size = serializedString.size();
    mh.type = MESSAGE_TYPE_CONTENTS_DIMENSIONS;

    send(mh, RANK0);
    send(serializedString, RANK0);
}

void MPIChannel::send(PixelStreamFramePtr frame)
{
    if(mpiRank_ != RANK0)
    {
        put_flog(LOG_WARN, "called on rank %i != 0", mpiRank_);
        return;
    }

    assert(!frame->segments.empty() && "sendPixelStreamSegments() received an empty vector");

    const std::string& serializedString = serialize(frame->segments);

    // send the header and the message
    MessageHeader mh;
    mh.size = serializedString.size();
    mh.type = MESSAGE_TYPE_PIXELSTREAM;

    // add the truncated URI to the header
    strncpy(mh.uri, frame->uri.toLocal8Bit().constData(), MESSAGE_HEADER_URI_LENGTH-1);

    // the header is sent via a send, so that we can probe it on the render processes
    for(int i=1; i<mpiSize_; ++i)
        send(mh, i);

    broadcast(serializedString);
}

void MPIChannel::receivePixelStreams(const MessageHeader& messageHeader)
{
    if(mpiRank_ < 1)
    {
        put_flog(LOG_WARN, "called on rank %i < 1", mpiRank_);
        return;
    }

    PixelStreamFramePtr frame(new PixelStreamFrame);
    frame->uri = QString(messageHeader.uri);
    receiveBroadcast(frame->segments, messageHeader.size);

    emit received(frame);
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
