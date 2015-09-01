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

#include "FFMPEGVideoFrameConverter.h"

#include "FFMPEGFrame.h"
#include "log.h"

#pragma clang diagnostic ignored "-Wdeprecated"
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

FFMPEGVideoFrameConverter::FFMPEGVideoFrameConverter( const AVCodecContext&
                                                      videoCodecContext,
                                                      PixelFormat targetFormat )
    : swsContext_( 0 )
{
    // create sws scaler context
    swsContext_ = sws_getContext( videoCodecContext.width,
                                  videoCodecContext.height,
                                  videoCodecContext.pix_fmt,
                                  videoCodecContext.width,
                                  videoCodecContext.height,
                                  targetFormat, SWS_FAST_BILINEAR,
                                  NULL, NULL, NULL );
    if( !swsContext_ )
    {
        put_flog( LOG_ERROR, "Error allocating SwsContext" );
        return;
    }
}

FFMPEGVideoFrameConverter::~FFMPEGVideoFrameConverter()
{
    sws_freeContext( swsContext_ );
}

bool FFMPEGVideoFrameConverter::convert( const FFMPEGFrame& srcFrame,
                                         FFMPEGPicture& dstFrame )
{
    const AVFrame& avFrame = srcFrame.getAVFrame();

    dstFrame.getAVFrame().pkt_dts = avFrame.pkt_dts;

    const int output_height = sws_scale( swsContext_, avFrame.data,
                                         avFrame.linesize, 0,
                                         avFrame.height,
                                         dstFrame.getAVFrame().data,
                                         dstFrame.getAVFrame().linesize );
    return output_height == avFrame.height;
}
