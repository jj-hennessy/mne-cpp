//=============================================================================================================
/**
* @file     fiff_cov.cpp
* @author   Christoph Dinh <chdinh@nmr.mgh.harvard.edu>;
*           Matti Hämäläinen <msh@nmr.mgh.harvard.edu>
* @version  1.0
* @date     July, 2012
*
* @section  LICENSE
*
* Copyright (C) 2012, Christoph Dinh and Matti Hamalainen. All rights reserved.
*
* Redistribution and use in source and binary forms, with or without modification, are permitted provided that
* the following conditions are met:
*     * Redistributions of source code must retain the above copyright notice, this list of conditions and the
*       following disclaimer.
*     * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and
*       the following disclaimer in the documentation and/or other materials provided with the distribution.
*     * Neither the name of the Massachusetts General Hospital nor the names of its contributors may be used
*       to endorse or promote products derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
* WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
* PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL MASSACHUSETTS GENERAL HOSPITAL BE LIABLE FOR ANY DIRECT,
* INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
* PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
* HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
* NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
*
*
* @brief    Contains the implementation of the FiffCov Class.
*
*/

//*************************************************************************************************************
//=============================================================================================================
// INCLUDES
//=============================================================================================================

#include "fiff_cov.h"


//*************************************************************************************************************
//=============================================================================================================
// USED NAMESPACES
//=============================================================================================================

using namespace FIFFLIB;


//*************************************************************************************************************
//=============================================================================================================
// DEFINE MEMBER METHODS
//=============================================================================================================

FiffCov::FiffCov()
: kind(-1)
, data(NULL)
, eig(NULL)
, eigvec(NULL)
{

}


//*************************************************************************************************************

FiffCov::FiffCov(const FiffCov* p_pFiffCov)
: kind(p_pFiffCov->kind)
, diag(p_pFiffCov->diag)
, dim(p_pFiffCov->dim)
, names(p_pFiffCov->names)
, data(p_pFiffCov->data ? new MatrixXd(*p_pFiffCov->data) : NULL)
, bads(p_pFiffCov->bads)
, nfree(p_pFiffCov->nfree)
, eig(p_pFiffCov->eig ? new VectorXd(*p_pFiffCov->eig) : NULL)
, eigvec(p_pFiffCov->eigvec ? new MatrixXd(*p_pFiffCov->eigvec) : NULL)
{
    for(qint32 i = 0; i < p_pFiffCov->projs.size(); ++i)
    {
        projs.append(new FiffProj(p_pFiffCov->projs[i]));
    }

}


//*************************************************************************************************************

FiffCov::~FiffCov()
{
    if (data)
        delete data;
    for (qint32 i = 0; i < projs.size(); ++i)
        if(projs[i])
            delete projs[i];
    if (eig)
        delete eig;
    if (eigvec)
        delete eigvec;
}
