/*********************************************************************/
/* Copyright (c) 2011 - 2012, The University of Texas at Austin.     */
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

#ifndef CONTENT_H
#define CONTENT_H

#include "types.h"
#include "ContentType.h"

#include <boost/serialization/access.hpp>
#include <boost/serialization/assume_abstract.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <QObject>
#include <QSize>

class WallToWallChannel;

/**
 * An abstract Content displayed in a ContentWindow.
 *
 * This class does not actually hold any content data because it
 * is meant to be sent through MPI to Rank>0 processes.
 * The content data is held by FactoryObjects on Rank>0 processes.
 * A Content object references a FactoryObject of the same ContentType based on its URI.
 * It is possible for multiple Content objects to reference the same FactoryObject.
 */
class Content : public QObject
{
    Q_OBJECT

public:
    /** Constructor **/
    Content( const QString& uri );

    /** Get the content URI **/
    const QString& getURI() const;

    /** Get the content type **/
    virtual CONTENT_TYPE getType() = 0;

    /**
     * Read content metadata from the data source.
     * Used on Rank0 for file-based content types to refresh data from source URI.
     * @return true if the informations could be read.
    **/
    virtual bool readMetadata() = 0;

    /** Get the dimensions. */
    QSize getDimensions() const;

    /** Set the dimensions. */
    void setDimensions( const QSize& dimensions );

    /** Get the aspect ratio. */
    float getAspectRatio() const;

    /** Used to indicate that the window is being moved. TODO: move to ContentWindow. */
    void blockAdvance( bool block ) { blockAdvance_ = block; }

    /** Re-implement this method to update or synchronize before rendering. */
    virtual void preRenderUpdate( Factories&, ContentWindowPtr, WallToWallChannel& ) { }

    /** Re-implement this method to update or synchronize after rendering. */
    virtual void postRenderUpdate( Factories&, ContentWindowPtr, WallToWallChannel& ) { }

signals:
    /** Emitted by any Content subclass when its state has been modified */
    void modified();

protected:
    friend class boost::serialization::access;

    // Default constructor required for boost::serialization
    Content() {}

    template< class Archive >
    void serialize( Archive & ar, const unsigned int )
    {
        ar & boost::serialization::make_nvp( "uri", uri_ );
        ar & boost::serialization::make_nvp( "width", size_.rwidth( ));
        ar & boost::serialization::make_nvp( "height", size_.rheight( ));
        ar & boost::serialization::make_nvp( "block_advance", blockAdvance_ );
    }

    QString uri_;
    QSize size_;
    bool blockAdvance_;
};

BOOST_SERIALIZATION_ASSUME_ABSTRACT( Content )

#endif
