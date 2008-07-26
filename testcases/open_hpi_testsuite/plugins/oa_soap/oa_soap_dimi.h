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
 *      Anand S <S.Anand@hp.com>
 */

#ifndef _OA_SOAP_DIMI_H
#define _OA_SOAP_DIMI_H

/* Include files */
#include <SaHpi.h>
#include <oh_error.h>

SaErrorT oa_soap_get_dimi_info(
                void *oh_handler,
                SaHpiSessionIdT session_id,
                SaHpiResourceIdT resource_id,
                SaHpiDimiNumT dimi_num,
                SaHpiDimiInfoT *dimi_info);

SaErrorT oa_soap_get_dimi_test(
                void *oh_handler,
                SaHpiSessionIdT session_id,
                SaHpiResourceIdT resource_id,
                SaHpiDimiNumT dimi_num,
                SaHpiDimiTestNumT dimi_testnum,
                SaHpiDimiTestT *dimi_test);

SaErrorT oa_soap_get_dimi_test_ready(
                void *oh_handler,
                SaHpiSessionIdT session_id,
                SaHpiResourceIdT resource_id,
                SaHpiDimiNumT dimi_num,
                SaHpiDimiTestNumT dimi_testnum,
                SaHpiDimiReadyT *dimi_test_ready);

SaErrorT oa_soap_start_dimi_test(
                void *oh_handler,
                SaHpiSessionIdT session_id,
                SaHpiResourceIdT resource_id,
                SaHpiDimiNumT dimi_num,
                SaHpiDimiTestNumT dimi_testnum,
                SaHpiUint8T params,
                SaHpiDimiTestVariableParamsT *param_list);

SaErrorT oa_soap_cancel_dimi_test(
                void *oh_handler,
                SaHpiSessionIdT session_id,
                SaHpiResourceIdT resource_id,
                SaHpiDimiNumT dimi_num,
                SaHpiDimiTestNumT dimi_testnum);

SaErrorT oa_soap_get_dimi_test_status(
                void *oh_handler,
                SaHpiSessionIdT session_id,
                SaHpiResourceIdT resource_id,
                SaHpiDimiNumT dimi_num,
                SaHpiDimiTestNumT dimi_testnum,
                SaHpiDimiTestPercentCompletedT* complete,
                SaHpiDimiTestRunStatusT *status);

SaErrorT oa_soap_get_dimi_test_result(
                void *oh_handler,
                SaHpiSessionIdT session_id,
                SaHpiResourceIdT resource_id,
                SaHpiDimiNumT dimi_num,
                SaHpiDimiTestNumT dimi_testnum,
                SaHpiDimiTestResultsT *test_result);

#endif
