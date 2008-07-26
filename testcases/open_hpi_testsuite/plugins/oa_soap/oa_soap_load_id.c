/*
 * Copyright (C) 2007-2008, Hewlett-Packard Development Company, LLP
 *                     All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or
 * without modification, are permitted provided that the following
 * conditions are met:
 *
 * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in
 * the documentation and/or other materials provided with the distribution.
 *
 * Neither the name of the Hewlett-Packard Corporation, nor the names
 * of its contributors may be used to endorse or promote products
 * derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Author(s)
 *      Raghavendra P.G. <raghavendra.pg@hp.com>
 **/

#include "oa_soap_load_id.h"

/**
 * oa_soap_load_id_get
 *      @oh_handler:  Handler data pointer
 *      @resource_id: Resource ID
 *      @load_id:     Load Id
 *
 * Purpose:
 *      Retrieves the load id
 *
 * Detailed Description: NA
 *
 * Return values:
 *      SA_ERR_HPI_UNSUPPORTED_API - current oa_soap implementation does not
 *                                   support this API
 **/
SaErrorT oa_soap_load_id_get(void *oh_handler,
                             SaHpiResourceIdT resource_id,
                             SaHpiLoadIdT load_id)
{
        err("oa_soap_load_id_get not implemented");
        return SA_ERR_HPI_UNSUPPORTED_API;
}

/**
 * oa_soap_load_id_set
 *      @oh_handler:  Handler data pointer
 *      @resource_id: Resource ID
 *      @load_id:     Load Id
 *
 * Purpose:
 *      Sets Load Id
 *
 * Detailed Description: NA
 *
 * Return values:
 *      SA_ERR_HPI_UNSUPPORTED_API - current oa_soap implementation does not
 *                                   support this API
 **/
SaErrorT oa_soap_load_id_set(void *oh_handler,
                             SaHpiResourceIdT resource_id,
                             SaHpiLoadIdT *load_id)
{
        err("oa_soap_load_id_set not implemented");
        return SA_ERR_HPI_UNSUPPORTED_API;
}

void * oh_load_id_get (void *,
                       SaHpiResourceIdT,
                       SaHpiLoadIdT)
                __attribute__ ((weak, alias("oa_soap_load_id_get")));

void * oh_load_id_set (void *,
                       SaHpiResourceIdT,
                       SaHpiLoadIdT *)
                __attribute__ ((weak, alias("oa_soap_load_id_set")));
