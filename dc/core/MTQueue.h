/*********************************************************************/
/* Copyright (c) 2015, EPFL/Blue Brain Project                       */
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

#ifndef MTQUEUE_H
#define MTQUEUE_H

#include <queue>
#include <mutex>
#include <condition_variable>

template <class T>
class MTQueue
{
public:
    MTQueue( const size_t max_size )
        : _max_size( max_size )
    {}

    void enqueue( const T t )
    {
        std::unique_lock<std::mutex> lock( _mutex );
        while( _queue.size() >= _max_size )
            _full.wait( lock );
        _queue.push( t );
        _empty.notify_one();
    }

    T dequeue()
    {
        std::unique_lock<std::mutex> lock( _mutex );
        while( _queue.empty( ))
            _empty.wait( lock );
        T val = _queue.front();
        _queue.pop();
        _full.notify_one();
        return val;
    }

    void clear()
    {
        std::lock_guard<std::mutex> lock( _mutex );
        while( !_queue.empty( ))
            _queue.pop();
        _full.notify_one();
    }

    size_t size() const
    {
        std::lock_guard<std::mutex> lock( _mutex );
        return _queue.size();
    }

    bool empty() const
    {
        std::lock_guard<std::mutex> lock( _mutex );
        return _queue.empty();
    }

private:
    size_t _max_size;
    std::queue<T> _queue;
    mutable std::mutex _mutex;
    std::condition_variable _empty;
    std::condition_variable _full;
};

#endif
