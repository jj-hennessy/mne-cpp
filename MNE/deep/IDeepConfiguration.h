//=============================================================================================================
/**
* @file     IDeepConfiguration.h
* @author   Christoph Dinh <chdinh@nmr.mgh.harvard.edu>;
*           Matti Hamalainen <msh@nmr.mgh.harvard.edu>
* @version  1.0
* @date     February, 2017
*
* @section  LICENSE
*
* Copyright (C) 2017 Christoph Dinh and Matti Hamalainen. All rights reserved.
*
* Redistribution and use in source and binary forms, with or without modification, are permitted provided that
* the following conditions are met:
*     * Redistributions of source code must retain the above copyright notice, this list of conditions and the
*       following disclaimer.
*     * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and
*       the following disclaimer in the documentation and/or other materials provided with the distribution.
*     * Neither the name of MNE-CPP authors nor the names of its contributors may be used
*       to endorse or promote products derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
* WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
* PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
* INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
* PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
* HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
* NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
*
*
* @brief    Contains declaration of IDeepConfiguration interface class.
*
*/

#ifndef IDEEPCONFIGURATION_H
#define IDEEPCONFIGURATION_H

//*************************************************************************************************************
//=============================================================================================================
// INCLUDES
//=============================================================================================================

#include "deep_global.h"


//*************************************************************************************************************
//=============================================================================================================
// QT INCLUDES
//=============================================================================================================

#include <QObject>
#include <QSharedPointer>


//*************************************************************************************************************
//=============================================================================================================
// DEFINE NAMESPACE DEEPLIB
//=============================================================================================================

namespace DEEPLIB
{

//*************************************************************************************************************
//=============================================================================================================
// FORWARD DECLARATIONS
//=============================================================================================================



//=========================================================================================================
/**
* DECLARE CLASS IDeepConfiguration
*
* @brief The IDeepConfiguration class is the base interface class for all deep network configurations.
*/
class DEEPSHARED_EXPORT IDeepConfiguration : public QObject
{
    Q_OBJECT
public:
    typedef QSharedPointer<IDeepConfiguration> SPtr;               /**< Shared pointer type for IDeepConfiguration. */
    typedef QSharedPointer<const IDeepConfiguration> ConstSPtr;    /**< Const shared pointer type for IDeepConfiguration. */

    //=========================================================================================================
    /**
    * Destroys the network configuration.
    */
    virtual ~IDeepConfiguration() {}

    //=========================================================================================================
    /**
    * Clone the network configuration
    */
    virtual QSharedPointer<IDeepConfiguration> clone() const = 0;

    //=========================================================================================================
    /**
    * Initializes the network configuration.
    */
    virtual void init() = 0;

    //=========================================================================================================
    /**
    * Is called when network configuration unloaded.
    */
    virtual void unload() = 0;

    //=========================================================================================================
    /**
    * Returns the network configuration name.
    * Pure virtual method.
    *
    * @return the name of the configuration.
    */
    virtual QString getName() const = 0;

private:


};

//*************************************************************************************************************
//=============================================================================================================
// INLINE DEFINITIONS
//=============================================================================================================


} //Namespace

Q_DECLARE_INTERFACE(DEEPLIB::IDeepConfiguration, "deeplib/1.0")

#endif //IDEEPCONFIGURATION_H
