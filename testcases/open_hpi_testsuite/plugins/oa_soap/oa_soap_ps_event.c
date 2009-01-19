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
 *
 * This file has the implementation of the power supply unit event handling
 *
 *      process_ps_insertion_event()  - Processes the power supply unit
 *                                      insertion event
 *
 *      process_ps_extraction_event() - Processes the power supply unit
 *                                      extraction event
 *
 *	oa_soap_proc_ps_subsys_info() - Processes the power subsystem info event
 *
 *	oa_soap_proc_ps_status()      - Processes the power supply status event
 */

#include "oa_soap_ps_event.h"

/**
 * process_ps_insertion_event
 *      @oh_handler: Pointer to openhpi handler structure
 *      @con:        Pointer to SOAP_CON structure
 *      @oa_event:   Pointer to oa event response structure
 *
 * Purpose:
 *      Adds the newly inserted power supply information into RPT and RDR table
 *      Creates the hot swap event
 *
 * Detailed Description: NA
 *
 * Return values:
 *      SA_OK                     - success.
 *      SA_ERR_HPI_INVALID_PARAMS - on wrong parameters.
 *      SA_ERR_HPI_INTERNAL_ERROR - on failure
 **/
SaErrorT process_ps_insertion_event(struct oh_handler_state *oh_handler,
                                    SOAP_CON *con,
                                    struct eventInfo *oa_event)
{
        struct getPowerSupplyInfo info;
        struct powerSupplyInfo response;
        SaErrorT rv = SA_OK;

        if (oh_handler == NULL || con == NULL || oa_event == NULL) {
                err("Invalid parameters");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        info.bayNumber = oa_event->eventData.powerSupplyStatus.bayNumber;
        rv = soap_getPowerSupplyInfo(con, &info, &response);
        if (rv != SOAP_OK) {
                err("Get power supply info failed");
                return SA_ERR_HPI_INTERNAL_ERROR;
        }

        /* If the power supply unit does not have the power cord plugged in,
         * then power supply unit will be in faulty condition. In this case,
         * all the information in the response structure is NULL. Consider the
         * faulty power supply unit as ABSENT
         */
        if (response.serialNumber == NULL) {
                err("Inserted power supply unit may be faulty");
                return SA_ERR_HPI_INTERNAL_ERROR;
        }

        rv = add_ps_unit(oh_handler, con, &response);
        if (rv != SA_OK) {
                err("Add power supply unit failed");
        }

        return SA_OK;
}

/**
 * process_ps_extraction_event
 *      @oh_handler: Pointer to openhpi handler structure
 *      @oa_event:   Pointer to oa event response structure
 *
 * Purpose:
 *      Gets the power extraction event.
 *      Removes the extracted power supply information from RPT
 *      Creates the hot swap event
 *
 * Detailed Description: NA
 *
 * Return values:
 *      SA_OK                     - success.
 *      SA_ERR_HPI_INVALID_PARAMS - on wrong parameters.
 *      SA_ERR_HPI_INTERNAL_ERROR - on failure
 **/
SaErrorT process_ps_extraction_event(struct oh_handler_state *oh_handler,
                                     struct eventInfo *oa_event)
{
        SaErrorT rv = SA_OK;
        SaHpiInt32T bay_number;
        struct oa_soap_handler *oa_handler;

        if (oh_handler == NULL || oa_event == NULL) {
                err("Invalid parameters");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        oa_handler = (struct oa_soap_handler *) oh_handler->data;
        bay_number = oa_event->eventData.powerSupplyStatus.bayNumber;

       /* If the power supply unit does not have the power cord
        * plugged in, then power supply unit will be in faulty
        * condition. In this case, all the information in the
        * response structure is NULL. Then the faulty power supply
        * unit is considered as ABSENT. Ignore the extraction event.
        */
        if (oa_handler->oa_soap_resources.ps_unit.presence[bay_number - 1] ==
            RES_ABSENT) {
                err("Extracted power supply unit may be in faulty condition");
                return SA_ERR_HPI_INTERNAL_ERROR;
        }

        rv = remove_ps_unit(oh_handler, bay_number);
        if (rv != SA_OK) {
                err("Remove power supply unit failed");
        }

        return SA_OK;
}

/**
 * oa_soap_proc_ps_subsys_info
 *      @oh_handler	: Pointer to openhpi handler structure
 *      @info		: Pointer to power subsystem info structure
 *
 * Purpose:
 *	Processes the power subsystem info event
 *
 * Detailed Description: NA
 *
 * Return values:
 *	NONE
 **/
void oa_soap_proc_ps_subsys_info(struct oh_handler_state *oh_handler,
				 struct powerSubsystemInfo *info)
{
        SaErrorT rv = SA_OK;
        struct oa_soap_handler *oa_handler;
	SaHpiResourceIdT resource_id;

        if (oh_handler == NULL || info == NULL) {
                err("Invalid parameters");
                return;
        }

        oa_handler = (struct oa_soap_handler *) oh_handler->data;
        resource_id = oa_handler->oa_soap_resources.power_subsystem_rid;

	/* Process the operational status sensor */
	OA_SOAP_PROCESS_SENSOR_EVENT(OA_SOAP_SEN_OPER_STATUS,
				     info->operationalStatus, 0, 0);

	/* Process the predictive failure status sensor */
	OA_SOAP_PROCESS_SENSOR_EVENT(OA_SOAP_SEN_PRED_FAIL,
				     info->operationalStatus, 0, 0);

	/* Process the redundancy sensor */
	OA_SOAP_PROCESS_SENSOR_EVENT(OA_SOAP_SEN_REDUND,
				     info->redundancy, 0, 0);

	return;
}

/**
 * oa_soap_proc_ps_status
 *      @oh_handler	: Pointer to openhpi handler structure
 *      @status		: Pointer to power supply status structure
 *
 * Purpose:
 *	Processes the power supply status event
 *
 * Detailed Description: NA
 *
 * Return values:
 *	NONE
 **/
void oa_soap_proc_ps_status(struct oh_handler_state *oh_handler,
			    struct powerSupplyStatus *status)
{
        SaErrorT rv = SA_OK;
        struct oa_soap_handler *oa_handler;
	SaHpiResourceIdT resource_id;
	enum diagnosticStatus diag_ex_status[OA_SOAP_MAX_DIAG_EX];

        if (oh_handler == NULL || status == NULL) {
                err("Invalid parameters");
                return;
        }

        oa_handler = (struct oa_soap_handler *) oh_handler->data;
        resource_id = oa_handler->oa_soap_resources.ps_unit.
			resource_id[status->bayNumber - 1];

	/* Process the operational status sensor */
	OA_SOAP_PROCESS_SENSOR_EVENT(OA_SOAP_SEN_OPER_STATUS,
				     status->operationalStatus, 0, 0);

	/* Process the predictive failure status sensor */
	OA_SOAP_PROCESS_SENSOR_EVENT(OA_SOAP_SEN_PRED_FAIL,
				     status->operationalStatus, 0, 0);

	/* Process the redundancy sensor */
	OA_SOAP_PROCESS_SENSOR_EVENT(OA_SOAP_SEN_INT_DATA_ERR,
				    status->diagnosticChecks.
					internalDataError, 0, 0)

	/* Process the device location error sensor */
	OA_SOAP_PROCESS_SENSOR_EVENT(OA_SOAP_SEN_DEV_LOC_ERR,
				     status->diagnosticChecks.
					deviceLocationError, 0, 0)

	/* Process the device failure error sensor */
	OA_SOAP_PROCESS_SENSOR_EVENT(OA_SOAP_SEN_DEV_FAIL,
				     status->diagnosticChecks.deviceFailure,
				     0, 0)

	/* Process the device degraded error sensor */
	OA_SOAP_PROCESS_SENSOR_EVENT(OA_SOAP_SEN_DEV_DEGRAD,
				     status->diagnosticChecks.deviceDegraded,
				     0, 0)

	/* Process the AC failure sensor */
	OA_SOAP_PROCESS_SENSOR_EVENT(OA_SOAP_SEN_AC_FAIL,
				     status->diagnosticChecks.acFailure, 0, 0)

	oa_soap_parse_diag_ex(status->diagnosticChecksEx, diag_ex_status);

	/* Process device not supported sensor rdr */
	OA_SOAP_PROCESS_SENSOR_EVENT(OA_SOAP_SEN_DEV_NOT_SUPPORT,
				     diag_ex_status[DIAG_EX_DEV_NOT_SUPPORT],
				     0, 0)

	/* Process Device mix match sensor */
	OA_SOAP_PROCESS_SENSOR_EVENT(OA_SOAP_SEN_DEV_MIX_MATCH,
				     diag_ex_status[DIAG_EX_DEV_MIX_MATCH],
				     0, 0)

	return;
}
