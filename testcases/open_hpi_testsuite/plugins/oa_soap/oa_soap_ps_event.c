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

