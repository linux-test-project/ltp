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
 **/

#include "oa_soap_dimi.h"

/**
 * oa_soap_get_dimi_info:
 *      @oh_handler  : Handler data pointer
 *      @session_id  : Session ID
 *      @resource_id : Resource ID
 *      @dimi_num    : Dimi Number
 *      @dimi_info   : Dimi Information Holder
 *
 * Purpose:
 *      This function gets information about the DIMI.
 *
 * Description:
 *      NA
 *
 * Returns:
 *      SA_ERR_HPI_UNSUPPORTED_API - current oa_soap implementation does not
 *                                   support this API
 **/
SaErrorT oa_soap_get_dimi_info(void *oh_handler,
                               SaHpiSessionIdT session_id,
                               SaHpiResourceIdT resource_id,
                               SaHpiDimiNumT dimi_num,
                               SaHpiDimiInfoT *dimi_info)
{
        err("oa_soap_get_dimi_info not implemented");
        return SA_ERR_HPI_UNSUPPORTED_API;
}

/**
 * oa_soap_get_dimi_test:
 *      @oh_handler   : Handler data pointer
 *      @session_id   : Session ID
 *      @resource_id  : Resource ID
 *      @dimi_num     : Dimi Number
 *      @dimi_testnum : Dimi Test Number
 *      @dimi_test    : Dimi Test Information Holder
 *
 * Purpose:
 *      This function gets information about a particular Test

 * Description:
 *      NA
 *
 * Returns:
 *      SA_ERR_HPI_UNSUPPORTED_API - current oa_soap implementation does not
 *                                   support this API
 **/
SaErrorT oa_soap_get_dimi_test(void *oh_handler,
                               SaHpiSessionIdT session_id,
                               SaHpiResourceIdT resource_id,
                               SaHpiDimiNumT dimi_num,
                               SaHpiDimiTestNumT dimi_testnum,
                               SaHpiDimiTestT *dimi_test)
{
        err("oa_soap_get_dimi_info not implemented");
        return SA_ERR_HPI_UNSUPPORTED_API;
}


/**
 * oa_soap_get_dimi_test_ready:
 *      @oh_handler      : Handler data pointer
 *      @session_id      : Session ID
 *      @resource_id     : Resource ID
 *      @dimi_num        : Dimi Number
 *      @dimi_testnum    : Dimi Test Number
 *      @dimi_test_ready : Dimi Test readiness Information Holder
 *
 * Purpose:
 *      This function provides the readiness of a DIMI to run a test

 * Description:
 *      NA
 *
 * Returns:
 *      SA_ERR_HPI_UNSUPPORTED_API - current oa_soap implementation does not
 *                                   support this API
 **/
SaErrorT oa_soap_get_dimi_test_ready(void *oh_handler,
                                     SaHpiSessionIdT session_id,
                                     SaHpiResourceIdT resource_id,
                                     SaHpiDimiNumT dimi_num,
                                     SaHpiDimiTestNumT dimi_testnum,
                                     SaHpiDimiReadyT *dimi_test_ready)
{
        err("oa_soap_get_dimi_info not implemented");
        return SA_ERR_HPI_UNSUPPORTED_API;
}

/**
 * oa_soap_start_dimi_test:
 *      @oh_handler   : Handler data pointer
 *      @session_id   : Session ID
 *      @resource_id  : Resource ID
 *      @dimi_num     : Dimi Number
 *      @dimi_testnum : Dimi Test Number
 *      @params       : Number of Parameters
 *      @param_list   : Parameters List
 *
 * Purpose:
 *      This function starts execution of a test on DIMI.

 * Description:
 *      NA
 *
 * Returns:
 *      SA_ERR_HPI_UNSUPPORTED_API - current oa_soap implementation does not
 *                                   support this API
 **/
SaErrorT oa_soap_start_dimi_test(void *oh_handler,
                                 SaHpiSessionIdT session_id,
                                 SaHpiResourceIdT resource_id,
                                 SaHpiDimiNumT dimi_num,
                                 SaHpiDimiTestNumT dimi_testnum,
                                 SaHpiUint8T params,
                                 SaHpiDimiTestVariableParamsT *param_list)
{
        err("oa_soap_get_dimi_info not implemented");
        return SA_ERR_HPI_UNSUPPORTED_API;
}

/**
 * oa_soap_cancel_dimi_test:
 *      @oh_handler   : Handler data pointer
 *      @session_id   : Session ID
 *      @resource_id  : Resource ID
 *      @dimi_num     : Dimi Number
 *      @dimi_testnum : Dimi Test Number
 *      @params       : Number of Parameters
 *      @param_list   : Parameters List
 *
 * Purpose:
 *      This function starts execution of a test on DIMI.

 * Description:
 *      NA
 *
 * Returns:
 *      SA_ERR_HPI_UNSUPPORTED_API - current oa_soap implementation does not
 *                                   support this API
 **/
SaErrorT oa_soap_cancel_dimi_test(void *oh_handler,
                                  SaHpiSessionIdT session_id,
                                  SaHpiResourceIdT resource_id,
                                  SaHpiDimiNumT dimi_num,
                                  SaHpiDimiTestNumT dimi_testnum)
{
        err("oa_soap_get_dimi_info not implemented");
        return SA_ERR_HPI_UNSUPPORTED_API;
}

/**
 * oa_soap_get_dimi_test_status:
 *      @oh_handler   : Handler data pointer
 *      @session_id   : Session ID
 *      @resource_id  : Resource ID
 *      @dimi_num     : Dimi Number
 *      @dimi_testnum : Dimi Test Number
 *      @complete     : Percentage of Test run completed
 *      @status       : Test Run Status
 *
 * Purpose:
 *      This function returns the status of a particular DIMI test

 * Description:
 *      NA
 *
 * Returns:
 *      SA_ERR_HPI_UNSUPPORTED_API - current oa_soap implementation does not
 *                                   support this API
 **/
SaErrorT oa_soap_get_dimi_test_status(void *oh_handler,
                                      SaHpiSessionIdT session_id,
                                      SaHpiResourceIdT resource_id,
                                      SaHpiDimiNumT dimi_num,
                                      SaHpiDimiTestNumT dimi_testnum,
                                      SaHpiDimiTestPercentCompletedT *complete,
                                      SaHpiDimiTestRunStatusT *status)
{
        err("oa_soap_get_dimi_info not implemented");
        return SA_ERR_HPI_UNSUPPORTED_API;
}

/**
 * oa_soap_get_dimi_test_retult:
 *      @oh_handler   : Handler data pointer
 *      @session_id   : Session ID
 *      @resource_id  : Resource ID
 *      @dimi_num     : Dimi Number
 *      @dimi_testnum : Dimi Test Number
 *      @test_result  : Dimi Test Result Information Holder
 *
 * Purpose:
 *      This function retrieves the results from the last run of test in DIMI

 * Description:
 *      NA
 *
 * Returns:
 *      SA_ERR_HPI_UNSUPPORTED_API - current oa_soap implementation does not
 *                                   support this API
 **/
SaErrorT oa_soap_get_dimi_test_result(void *oh_handler,
                                      SaHpiSessionIdT session_id,
                                      SaHpiResourceIdT resource_id,
                                      SaHpiDimiNumT dimi_num,
                                      SaHpiDimiTestNumT dimi_testnum,
                                      SaHpiDimiTestResultsT *test_result)
{
        err("oa_soap_get_dimi_info not implemented");
        return SA_ERR_HPI_UNSUPPORTED_API;
}


/**
 *
 * Mapping of the ABIs with local function pointers
 *
 **/


void * oh_get_dimi_info (void*,
                SaHpiSessionIdT,
                SaHpiResourceIdT,
                SaHpiDimiNumT,
                SaHpiDimiInfoT*)
  __attribute__ ((weak, alias ("oa_soap_get_dimi_info")));

void * oh_get_dimi_test (void*,
                SaHpiSessionIdT,
                SaHpiResourceIdT,
                SaHpiDimiNumT,
                SaHpiDimiTestNumT,
                SaHpiDimiTestT*)
  __attribute__ ((weak, alias ("oa_soap_get_dimi_test")));

void * oh_get_dimi_test_ready (void*,
                SaHpiSessionIdT,
                SaHpiResourceIdT,
                SaHpiDimiNumT,
                SaHpiDimiTestNumT,
                SaHpiDimiReadyT*)
  __attribute__ ((weak, alias ("oa_soap_get_dimi_test_ready")));

void * oh_start_dimi_test (void*,
                SaHpiSessionIdT,
                SaHpiResourceIdT,
                SaHpiDimiNumT,
                SaHpiDimiTestNumT,
                SaHpiUint8T,
                SaHpiDimiTestVariableParamsT*)
  __attribute__ ((weak, alias ("oa_soap_start_dimi_test")));

void * oh_cancel_dimi_test (void*,
                SaHpiSessionIdT,
                SaHpiResourceIdT,
                SaHpiDimiNumT,
                SaHpiDimiTestNumT)
  __attribute__ ((weak, alias ("oa_soap_cancel_dimi_test")));

void * oh_get_dimi_test_status (void*,
                SaHpiSessionIdT,
                SaHpiResourceIdT,
                SaHpiDimiNumT,
                SaHpiDimiTestNumT,
                SaHpiDimiTestPercentCompletedT*,
                SaHpiDimiTestRunStatusT*)
  __attribute__ ((weak, alias ("oa_soap_get_dimi_test_status")));

void * oh_get_dimi_test_result (void*,
                SaHpiSessionIdT,
                SaHpiResourceIdT,
                SaHpiDimiNumT,
                SaHpiDimiTestNumT,
                SaHpiDimiTestResultsT*)
  __attribute__ ((weak, alias ("oa_soap_get_dimi_test_result")));
