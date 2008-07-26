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
 *      Raja Kumar Thatte <raja-kumar.thatte@hp.com>
 */

#ifndef _OA_SOAP_HOTSWAP_H
#define _OA_SOAP_HOTSWAP_H

/* Include files */
#include "oa_soap_utils.h"
#include "oa_soap_power.h"

SaErrorT oa_soap_get_hotswap_state(void *oh_handler,
                                   SaHpiResourceIdT resource_id,
                                   SaHpiHsStateT *state);

SaErrorT oa_soap_set_hotswap_state(void *oh_handler,
                                   SaHpiResourceIdT resource_id,
                                   SaHpiHsStateT state);

SaErrorT oa_soap_get_indicator_state(void *oh_handler,
                                     SaHpiResourceIdT resource_id,
                                     SaHpiHsIndicatorStateT *state);

SaErrorT oa_soap_set_indicator_state(void *oh_handler,
                                     SaHpiResourceIdT resource_id,
                                     SaHpiHsIndicatorStateT state);

SaErrorT oa_soap_request_hotswap_action(void *oh_handler,
                                        SaHpiResourceIdT resource_id,
                                        SaHpiHsActionT action);

SaErrorT oa_soap_hotswap_policy_cancel(void *oh_handler,
                                       SaHpiResourceIdT resource_id,
                                       SaHpiTimeoutT timeout);

SaErrorT oa_soap_get_autoinsert_timeout(void *oh_handler,
                                        SaHpiResourceIdT resource_id,
                                        SaHpiTimeoutT timeout);

SaErrorT oa_soap_set_autoinsert_timeout(void *oh_handler,
                                        SaHpiTimeoutT timeout);

SaErrorT oa_soap_get_autoextract_timeout(void *oh_handler,
                                         SaHpiResourceIdT resource_id,
                                         SaHpiTimeoutT *timeout);

SaErrorT oa_soap_set_autoextract_timeout(void *oh_handler,
                                         SaHpiResourceIdT resource_id,
                                         SaHpiTimeoutT timeout);

#endif

