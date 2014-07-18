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

#ifndef WALLTOMASTERCHANNEL_H
#define WALLTOMASTERCHANNEL_H

#include "types.h"
#include "SerializeBuffer.h"

#include <QObject>

class WallToMasterChannel : public QObject
{
    Q_OBJECT

public:
    /** Constructor */
    WallToMasterChannel(MPIChannelPtr mpiChannel);

    /**
     * Ranks 1-N: Receive messages.
     * Will emit a signal if an object was reveived.
     * @see received(DisplayGroupManagerPtr)
     * @see received(OptionsPtr)
     */
    void receiveMessages();

    // TODO remove content dimension requests (DISCL-21)
    /** Rank1(-N): Set the factories to respond to Content Dimensions request */
    void setFactories(FactoriesPtr factories);

signals:
    /**
     * Rank 1-N: Emitted when a displayGroup was recieved
     * @see receiveMessages()
     * @param displayGroup The DisplayGroup that was received
     */
    void received(DisplayGroupManagerPtr displayGroup);

    /**
     * Rank 1-N: Emitted when new Options were recieved
     * @see receiveMessages()
     * @param options The options that were received
     */
    void received(OptionsPtr options);

    /**
     * Rank 1-N: Emitted when new Markers were recieved
     * @see receiveMessages()
     * @param markers The markers that were received
     */
    void received(MarkersPtr markers);

    /**
     * Rank 1-N: Emitted when a new PixelStream frame was recieved
     * @see receiveMessages()
     * @param frame The frame that was received
     */
    void received(PixelStreamFramePtr frame);

    /**
     * Rank 1-N: Emitted when the quit message was recieved
     * @see receiveMessages()
     */
    void receivedQuit();

private:
    MPIChannelPtr mpiChannel_;
    SerializeBuffer buffer_;

    template <typename T>
    T receiveBroadcast(const size_t messageSize);

    // TODO remove content dimension requests (DISCL-21)
    void sendContentsDimensionsReply();
    DisplayGroupManagerPtr displayGroup_;
    FactoriesPtr factories_;
};

#endif // WALLTOMASTERCHANNEL_H
