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

#ifndef FACTORY_OBJECT_H
#define FACTORY_OBJECT_H

#include "types.h"

/**
 * An interface for objects that store Content data on Wall processes.
 *
 * An implementation must exist for every valid ContentType.
 */
class FactoryObject
{
public:
    /** Constructor */
    FactoryObject();

    /** Destructor */
    virtual ~FactoryObject();

    /** Render the FactoryObject */
    virtual void render() = 0;

    /** Render the preview ( whole object at low resolution.) */
    virtual void renderPreview();

    /** Update internal state before rendering. */
    virtual void preRenderUpdate( ContentWindowPtr window,
                                  const QRect& visibleWallArea ) = 0;

    /** Optional synchronize step before rendering. */
    virtual void preRenderSync( WallToWallChannel& wallToWallChannel )
    {
        Q_UNUSED( wallToWallChannel )
    }

    /** Optional synchronize step after rendering. */
    virtual void postRenderSync( WallToWallChannel& wallToWallChannel )
    {
        Q_UNUSED( wallToWallChannel )
    }

    /** Create an object corresponding to the given content. */
    static FactoryObjectPtr create( const Content& content );
};

#endif
