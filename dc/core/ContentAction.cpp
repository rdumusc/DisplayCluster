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

#include "ContentAction.h"

// false-positive on qt signals for Q_PROPERTY notifiers
// cppcheck-suppress uninitMemberVar
ContentAction::ContentAction( const QUuid& actionId )
    : uuid_( actionId )
    , checkable_( false )
    , checked_( false )
    , enabled_( true )
{
}

const QString& ContentAction::getIcon() const
{
    return icon_;
}

const QString& ContentAction::getIconChecked() const
{
    return iconChecked_;
}

bool ContentAction::isCheckable() const
{
    return checkable_;
}

bool ContentAction::isChecked() const
{
    return checked_;
}

bool ContentAction::isEnabled() const
{
    return enabled_;
}

void ContentAction::setIcon( const QString icon )
{
    if( icon == icon_ )
        return;

    icon_ = icon;
    emit iconChanged();
}

void ContentAction::setIconChecked( const QString icon )
{
    if( icon == iconChecked_ )
        return;

    iconChecked_ = icon;
    emit iconCheckedChanged();
}

void ContentAction::setCheckable( const bool value )
{
    if( checkable_ == value )
        return;

    checkable_ = value;
    emit checkableChanged();
}

void ContentAction::setEnabled( const bool value )

{
    if( enabled_ == value )
        return;

    enabled_ = value;
    emit enabledChanged();
}

void ContentAction::setChecked( const bool value )
{
    if( !checkable_ || checked_ == value )
        return;

    checked_ = value;
    emit checkedChanged();

    if( checked_ )
        emit checked();
    else
        emit unchecked();
}

void ContentAction::trigger()
{
    if( !enabled_ )
        return;

    if( checkable_ )
        setChecked( !checked_ );

    emit triggered( uuid_, checked_ );
}
