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

#include "Markers.h"

#include <boost/date_time/posix_time/posix_time.hpp>

Markers::Markers()
{
}

const MarkersMap& Markers::getMarkers() const
{
    return markers_;
}

void Markers::addMarker(const int id, const QPointF position)
{
    if (markers_.count(id))
        return;

    markers_[id].setPosition(position);
    emit(updated(shared_from_this()));
}

void Markers::updateMarker(const int id, const QPointF position)
{
    if (!markers_.count(id))
        return;

    markers_[id].setPosition(position);
    emit(updated(shared_from_this()));
}

void Markers::removeMarker(const int id)
{
    if (!markers_.count(id))
        return;

    markers_.erase(id);
    emit(updated(shared_from_this()));
}

void Markers::clearOldMarkers()
{
    const size_t initialSize = markers_.size();

    const boost::posix_time::ptime now = boost::posix_time::microsec_clock::universal_time();

    MarkersMap::iterator it = markers_.begin();

    while(it != markers_.end())
    {
        if(!it->second.isActive(now))
            markers_.erase(it++);  // note the post increment; increments the iterator but returns original value for erase
        else
            ++it;
    }

    if (initialSize != markers_.size())
        emit(updated(shared_from_this()));
}
