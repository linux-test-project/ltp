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
 *      Raja Kumar Thatte <raja-kumar.thatte@hp.com>
 */

#ifndef _OA_SOAP_FUMI_H
#define _OA_SOAP_FUMI_H

/* Include files */
#include <SaHpi.h>
#include <oh_error.h>

SaErrorT oa_soap_set_fumi_source(void *oh_handler,
                                SaHpiResourceIdT resource_id,
                                SaHpiFumiNumT num,
                                SaHpiBankNumT banknum,
                                SaHpiTextBufferT *sourceuri);

SaErrorT oa_soap_validate_fumi_source(void *oh_handler,
                                SaHpiResourceIdT resource_id,
                                SaHpiFumiNumT num,
                                SaHpiBankNumT banknum);

SaErrorT oa_soap_get_fumi_source(void *oh_handler,
                                SaHpiResourceIdT resource_id,
                                SaHpiFumiNumT num,
                                SaHpiBankNumT banknum,
                                SaHpiFumiSourceInfoT *sourceinfo);

SaErrorT oa_soap_get_fumi_target(void *oh_handler,
                                SaHpiResourceIdT resource_id,
                                SaHpiFumiNumT num,
                                SaHpiBankNumT banknum,
                                SaHpiFumiBankInfoT *bankinfo);

SaErrorT oa_soap_start_fumi_backup(void *oh_handler,
                                SaHpiResourceIdT resource_id,
                                SaHpiFumiNumT num);

SaErrorT oa_soap_set_fumi_bank_order(void *oh_handler,
                                SaHpiResourceIdT resource_id,
                                SaHpiFumiNumT num,
                                SaHpiBankNumT banknum,
                                SaHpiUint32T position);

SaErrorT oa_soap_start_fumi_bank_copy(void *oh_handler,
                                SaHpiResourceIdT resource_id,
                                SaHpiFumiNumT num,
                                SaHpiBankNumT sourcebanknum,
                                SaHpiBankNumT targetbanknum);

SaErrorT oa_soap_start_fumi_install(void *oh_handler,
                                SaHpiResourceIdT resource_id,
                                SaHpiFumiNumT num,
                                SaHpiBankNumT banknum);

SaErrorT oa_soap_get_fumi_status(void *oh_handler,
                                SaHpiResourceIdT resource_id,
                                SaHpiFumiNumT num,
                                SaHpiBankNumT banknum,
                                SaHpiFumiUpgradeStatusT *status);

SaErrorT oa_soap_start_fumi_verify(void *oh_handler,
                                SaHpiResourceIdT resource_id,
                                SaHpiFumiNumT num,
                                SaHpiBankNumT banknum);

SaErrorT oa_soap_cancel_fumi_upgrade(void *oh_handler,
                                SaHpiResourceIdT resource_id,
                                SaHpiFumiNumT num,
                                SaHpiBankNumT banknum);

SaErrorT oa_soap_start_fumi_rollback(void *oh_handler,
                                SaHpiResourceIdT resource_id,
                                SaHpiFumiNumT num);

SaErrorT oa_soap_activate_fumi(void *oh_handler,
                                SaHpiResourceIdT resource_id,
                                SaHpiFumiNumT num);

#endif
