/*
 * Copyright (C) 2008, Hewlett-Packard Development Company, LLP
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
 *      Raghavendra M.S. <raghavendra.ms@hp.com>
 *      Raghavendra P.G. <raghavendra.pg@hp.com>
 *
 * This file has the enclosure related events handling
 *
 * 	oa_soap_proc_enc_status()	- Processes the enclosure status event
 *
 */
#include "oa_soap_enclosure_event.h"

/**
 * oa_soap_proc_enc_status
 *      @oh_handler: Pointer to openhpi handler structure
 *      @con: Pointer to soap client con object
 *      @event: Pointer to the openhpi event structure
 *
 * Purpose:
 *     
 *
 * Detailed Description: NA
 *
 * Return values:
 *	NONE
 **/
void oa_soap_proc_enc_status(struct oh_handler_state *oh_handler,
			     struct enclosureStatus *status)
{
	SaErrorT rv = SA_OK;
	SaHpiResourceIdT resource_id;
	struct oa_soap_handler *oa_handler;
	enum diagnosticStatus diag_ex_status[OA_SOAP_MAX_DIAG_EX];

	if (oh_handler == NULL || status == NULL) {
		err("wrong parameters passed");
		return;
	}

	oa_handler = (struct oa_soap_handler *) oh_handler->data;
	resource_id = oa_handler->oa_soap_resources.enclosure_rid;

	/* Process the operational status sensor */
	OA_SOAP_PROCESS_SENSOR_EVENT(OA_SOAP_SEN_OPER_STATUS,
				     status->operationalStatus, 0, 0)

	/* Process the predictive failure status sensor */
	OA_SOAP_PROCESS_SENSOR_EVENT(OA_SOAP_SEN_PRED_FAIL,
				     status->operationalStatus, 0, 0)

	/* Process the internal data error sensor */
	OA_SOAP_PROCESS_SENSOR_EVENT(OA_SOAP_SEN_INT_DATA_ERR,
				     status->diagnosticChecks.internalDataError,
				     0, 0)

	/* Process the device failure sensor */
	OA_SOAP_PROCESS_SENSOR_EVENT(OA_SOAP_SEN_DEV_FAIL,
				     status->diagnosticChecks.deviceFailure,
				     0, 0)

	/* Process the device degraded sensor */
	OA_SOAP_PROCESS_SENSOR_EVENT(OA_SOAP_SEN_DEV_DEGRAD,
				     status->diagnosticChecks.deviceDegraded,
				     0, 0)

	/* Process the redundancy error sensor */
	OA_SOAP_PROCESS_SENSOR_EVENT(OA_SOAP_SEN_REDUND_ERR,
				     status->diagnosticChecks.redundancy, 0, 0)

	oa_soap_parse_diag_ex(status->diagnosticChecksEx, diag_ex_status);

	/* Build device not supported sensor rdr */
	OA_SOAP_PROCESS_SENSOR_EVENT(OA_SOAP_SEN_DEV_NOT_SUPPORT,
				     diag_ex_status[DIAG_EX_DEV_NOT_SUPPORT],
				     0, 0)

	return;
}
