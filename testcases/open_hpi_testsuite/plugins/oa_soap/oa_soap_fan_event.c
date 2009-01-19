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
 * This file has the implementation of the thermal subsystem, fan zone and fan
 * event handling
 *
 *      process_fan_insertion_event()  - Processes the fan insertion event
 *
 *      process_fan_extraction_event() - Processes the fan extraction event
 *
 *	oa_soap_proc_therm_subsys_info() - Processes the thermal subsystem info
 *					  event and generates the sensor event
 *
 *	oa_soap_proc_fz_status() - Processes the fan zone status event
 *
 *	oa_soap_proc_fan_status() - Processes the fan status event
 *
 */

#include "oa_soap_fan_event.h"

/**
 * process_fan_insertion_event
 *      @oh_handler: Pointer to openhpi handler structure
 *      @event:      Pointer to the openhpi event structure
 *
 * Purpose:
 *      Adds the newly inserted fan information into RPT and RDR table
 *      Creates the hot swap event
 *
 * Detailed Description: NA
 *
 * Return values:
 *      SA_OK                     - success.
 *      SA_ERR_HPI_INVALID_PARAMS - on wrong parameters.
 *      SA_ERR_HPI_INTERNAL_ERROR - on failure
 **/
SaErrorT process_fan_insertion_event(struct oh_handler_state *oh_handler,
                                     SOAP_CON *con,
                                     struct eventInfo *oa_event)
{
        SaErrorT rv = SA_OK;

        if (oh_handler == NULL || con == NULL || oa_event == NULL) {
                err("Invalid parameters");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        rv = add_fan(oh_handler, con, &(oa_event->eventData.fanInfo));
        if (rv != SA_OK) {
                err("Add fan failed");
                return rv;
        }

        return SA_OK;
}

/**
 * process_fan_extraction_event
 *      @oh_handler: Pointer to openhpi handler structure
 *      @oa_event:   Pointer to oa event response structure
 *
 * Purpose:
 *      Deletes the extracted fan information from RPT
 *      Creates the hot swap event
 *
 * Detailed Description: NA
 *
 * Return values:
 *      SA_OK                     - success.
 *      SA_ERR_HPI_INVALID_PARAMS - on wrong parameters.
 *      SA_ERR_HPI_INTERNAL_ERROR - on failure
 **/
SaErrorT process_fan_extraction_event(struct oh_handler_state *oh_handler,
                                      struct eventInfo *oa_event)
{
        SaErrorT rv = SA_OK;

        if (oh_handler == NULL || oa_event == NULL) {
                err("Invalid parameters");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        rv = remove_fan(oh_handler, oa_event->eventData.fanInfo.bayNumber);
        if (rv != SA_OK) {
                err("Remove fan failed");
                return SA_ERR_HPI_INTERNAL_ERROR;
        }

        return SA_OK;
}

/**
 * oa_soap_proc_therm_subsys_info
 *      @oh_handler	: Pointer to openhpi handler structure
 *      @info		: Pointer to thermalSubsystemInfo structure
 *
 * Purpose:
 *	Process the thermal subsystem info event
 *
 * Detailed Description: NA
 *
 * Return values:
 *	NONE
 **/
void oa_soap_proc_therm_subsys_info(struct oh_handler_state *oh_handler,
				    struct thermalSubsystemInfo *info)
{
        SaErrorT rv = SA_OK;
        SaHpiResourceIdT resource_id;
        struct oa_soap_handler *oa_handler;

        if (oh_handler == NULL || info == NULL) {
                err("wrong parameters passed");
                return;
        }

        oa_handler = (struct oa_soap_handler *) oh_handler->data;
        resource_id = oa_handler->oa_soap_resources.thermal_subsystem_rid;

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
 * oa_soap_proc_fz_status
 *      @oh_handler	: Pointer to openhpi handler structure
 *      @fan_zone	: Pointer to fanZone structure
 *
 * Purpose:
 *	Process the fan zone status event
 *
 * Detailed Description: NA
 *
 * Return values:
 *	NONE
 **/
void oa_soap_proc_fz_status(struct oh_handler_state *oh_handler,
			    struct fanZone *fan_zone)
{
        SaErrorT rv = SA_OK;
        SaHpiResourceIdT resource_id;
        struct oa_soap_handler *oa_handler;

        if (oh_handler == NULL || fan_zone == NULL) {
                err("wrong parameters passed");
                return;
        }

        oa_handler = (struct oa_soap_handler *) oh_handler->data;

        resource_id = oa_handler->oa_soap_resources.fan_zone.
			resource_id[fan_zone->zoneNumber - 1];

	/* Process the operational status sensor */
	OA_SOAP_PROCESS_SENSOR_EVENT(OA_SOAP_SEN_OPER_STATUS,
				     fan_zone->operationalStatus, 0, 0);

	/* Process the predictive failure status sensor */
	OA_SOAP_PROCESS_SENSOR_EVENT(OA_SOAP_SEN_PRED_FAIL,
				     fan_zone->operationalStatus, 0, 0);

	/* Process the redundancy sensor */
	OA_SOAP_PROCESS_SENSOR_EVENT(OA_SOAP_SEN_REDUND,
				     fan_zone->redundant,
				     0, 0);

        return;
}

/**
 * oa_soap_proc_fan_status
 *      @oh_handler	: Pointer to openhpi handler structure
 *      @info		: Pointer to fanInfo structure
 *
 * Purpose:
 *	Process the fan status event
 *
 * Detailed Description: NA
 *
 * Return values:
 *	NONE
 **/
void oa_soap_proc_fan_status(struct oh_handler_state *oh_handler,
			     struct fanInfo *info)
{
        SaErrorT rv = SA_OK;
        SaHpiResourceIdT resource_id;
        struct oa_soap_handler *oa_handler;
	SaHpiInt32T slot;
	enum diagnosticStatus diag_ex_status[OA_SOAP_MAX_DIAG_EX];

        if (oh_handler == NULL || info == NULL) {
                err("wrong parameters passed");
                return;
        }

        oa_handler = (struct oa_soap_handler *) oh_handler->data;
	slot = info->bayNumber;
        resource_id = oa_handler->oa_soap_resources.fan.resource_id[slot - 1];

	/* Process the operational status sensor */
	OA_SOAP_PROCESS_SENSOR_EVENT(OA_SOAP_SEN_OPER_STATUS,
				     info->operationalStatus, 0, 0);

	/* Process the predictive failure status sensor */
	OA_SOAP_PROCESS_SENSOR_EVENT(OA_SOAP_SEN_PRED_FAIL,
				     info->operationalStatus, 0, 0);

	/* Process the internal data error sensor */
	OA_SOAP_PROCESS_SENSOR_EVENT(OA_SOAP_SEN_INT_DATA_ERR,
				     info->diagnosticChecks.internalDataError,
				     0, 0);

	/* Process the device location error sensor */
	OA_SOAP_PROCESS_SENSOR_EVENT(OA_SOAP_SEN_DEV_LOC_ERR,
				     info->diagnosticChecks.deviceLocationError,
				     0, 0);

	/* Process the device failure error sensor */
	OA_SOAP_PROCESS_SENSOR_EVENT(OA_SOAP_SEN_DEV_FAIL,
				     info->diagnosticChecks.deviceFailure,
				     0, 0);

	/* Process the device degraded error sensor */
	OA_SOAP_PROCESS_SENSOR_EVENT(OA_SOAP_SEN_DEV_DEGRAD,
				     info->diagnosticChecks.deviceDegraded,
				     0, 0);

	oa_soap_parse_diag_ex(info->diagnosticChecksEx, diag_ex_status);

	/* Process device missing sensor */
	OA_SOAP_PROCESS_SENSOR_EVENT(OA_SOAP_SEN_DEV_MISS,
				     diag_ex_status[DIAG_EX_DEV_MISS], 0, 0)

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
