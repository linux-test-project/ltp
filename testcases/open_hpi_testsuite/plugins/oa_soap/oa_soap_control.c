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
 *      Raghavendra M.S. <raghavendra.ms@hp.com>
 *      Shuah Khan <shuah.khan@hp.com>
 *
 *  This file handles all the control functionality related apis.
 *
 *      oa_soap_get_control_state       - Control ABI to return the control
 *                                        state and mode of the resource
 *
 *      oa_soap_set_control_state       - Control ABI to set the control
 *                                        state the resource
 *
 *      build_server_control_rdr        - Creates and adds the control rdr
 *                                        to the server resource
 *
 *      build_interconnect_control_rdr  - Creates and adds the control rdr
 *                                        to the interconnect resource
 */

#include "oa_soap_control.h"

/**
 * oa_soap_get_control_state
 *      @oh_handler: Handler data pointer
 *      @resource_id: Resource ID
 *      @rdr_num: Control rdr number
 *      @mode: Mode of the control
 *      @state: State of the control
 *
 * Purpose:
 *      Gets the current state and the mode of the control object
 *
 * Detailed Description:
 *      - Gets the current state and the default mode of control object
 *        of either server blade or interconnect
 *      - To determine the control state, power state of the resource is
 *        retrieved and is appropriately mapped to control state
 *      - Plug-in does not support changing the control mode
 *
 * Return values:
 *      SA_OK - Normal case.
 *      SA_ERR_HPI_CAPABILITY - Resource doesn't have SAHPI_CAPABILITY_CONTROL
 *      SA_ERR_HPI_INVALID_PARAMS - On wrong parameter
 *      SA_ERR_HPI_INVALID_RESOURCE - Resource does not exist
 *      SA_ERR_HPI_NOT_PRESENT - Control not present
 *      SA_ERR_INTERNAL_ERROR - Internal error encountered
 **/
SaErrorT oa_soap_get_control_state(void *oh_handler,
                                   SaHpiResourceIdT resource_id,
                                   SaHpiCtrlNumT rdr_num,
                                   SaHpiCtrlModeT *mode,
                                   SaHpiCtrlStateT *state)
{
        SaErrorT rv = SA_OK;
        struct oh_handler_state *handler = NULL;
        SaHpiRptEntryT *rpt = NULL;
        SaHpiRdrT *rdr = NULL;
        SaHpiCtrlTypeT type;
        SaHpiCtrlModeT ctrl_mode;
        SaHpiCtrlStateT ctrl_state;
        SaHpiCtrlRecDigitalT *digital = NULL;
        SaHpiCtrlRecT *ctrl = NULL;
        SaHpiPowerStateT power_state;

        if (oh_handler == NULL || mode == NULL || state == NULL) {
                err("Invalid parameter.");
                return (SA_ERR_HPI_INVALID_PARAMS);
        }

        handler = (struct oh_handler_state *) oh_handler;

        rpt = oh_get_resource_by_id (handler->rptcache, resource_id);
        if (rpt == NULL) {
                err("INVALID RESOURCE");
                return SA_ERR_HPI_INVALID_RESOURCE;
        }
        if (! (rpt->ResourceCapabilities & SAHPI_CAPABILITY_CONTROL)) {
                err("INVALID RESOURCE CAPABILITY");
                return SA_ERR_HPI_CAPABILITY;
        }

        rdr = oh_get_rdr_by_type(handler->rptcache, resource_id,
                                 SAHPI_CTRL_RDR, rdr_num);
        if (rdr == NULL) {
                err("INVALID RDR NUMBER");
               return (SA_ERR_HPI_NOT_PRESENT);
        }
        ctrl = &(rdr->RdrTypeUnion.CtrlRec);
        ctrl_mode = ctrl->DefaultMode.Mode;

        /* Set control mode of return parameter to Manual mode
         * Manual mode is the only mode supported by plug-in
         */
        *mode = ctrl_mode;
        type = ctrl->Type;
        ctrl_state.Type = type;

        digital = &(ctrl->TypeUnion.Digital);

        rv = oa_soap_get_power_state(handler, resource_id, &power_state);
        if (rv != SA_OK) {
                err("Failed to get the power state RDR");
                return rv;
        }

        /* Based on the power state, map into the appropriate control state */
        switch (power_state) {
                case SAHPI_POWER_ON:
                        digital->Default = SAHPI_CTRL_STATE_ON;
                        break;
                case SAHPI_POWER_OFF:
                        digital->Default = SAHPI_CTRL_STATE_OFF;
                        break;
                default:
                        err("Invalid power state");
                        return SA_ERR_HPI_INTERNAL_ERROR;
        }

        ctrl_state.StateUnion.Digital = digital->Default;
        /* Return the appropriately mapped control state */
        *state = ctrl_state;
        return rv;
}

/**
 * oa_soap_set_control_state:
 *      @oh_handler: Handler data pointer
 *      @resource_id: Resource ID
 *      @rdr_num: Control rdr number
 *      @mode: Mode of the control
 *      @state: State of the control
 *
 * Purpose:
 *      Sets the current state of the control object. Mode setting is not
 *      allowed
 *
 * Detailed Description:
 *      - Validates the state parameter and sets the control state of resource
 *        to the specified value
 *      - the current state and the default mode of control object
 *        of either server blade or interconnect
 *      - To determine the control state, power state of the resource is
 *        retrieved and is appropriately mapped to control state
 *      - Plug-in does not support changing the control mode
 *
 * Return values:
 *      SA_OK - Normal case.
 *      SA_ERR_HPI_CAPABILITY - Resource doesn't have SAHPI_CAPABILITY_CONTROL
 *      SA_ERR_HPI_INVALID_PARAMS - On wrong parameter
 *      SA_ERR_HPI_INVALID_RESOURCE - Resource does not exist
 *      SA_ERR_HPI_NOT_PRESENT - Control not present
 *      SA_ERR_INTERNAL_ERROR - Internal error encountered
 *      SA_ERR_HPI_INVALID_DATA - Invalid Control Mode/State specified
 *      SA_ERR_HPI_UNSUPPORTED_PARAMS - Setting the control mode
 *                                      to AUTO mode is not supported
 **/
SaErrorT oa_soap_set_control_state(void *oh_handler,
                                   SaHpiResourceIdT resource_id,
                                   SaHpiCtrlNumT rdr_num,
                                   SaHpiCtrlModeT mode,
                                   SaHpiCtrlStateT *state)
{
        SaErrorT rv = SA_OK;
        struct oh_handler_state *handler = NULL;
        struct oa_soap_handler *oa_handler = NULL;
        SaHpiRptEntryT *rpt = NULL;
        SaHpiRdrT *rdr = NULL;
        SaHpiCtrlRecT *ctrl = NULL;
        SaHpiPowerStateT power_state;

        if (oh_handler == NULL || state == NULL) {
                err("Invalid parameter.");
                return (SA_ERR_HPI_INVALID_PARAMS);
        }

        handler = (struct oh_handler_state *) oh_handler;
        oa_handler = (struct oa_soap_handler *) handler->data;

        rpt = oh_get_resource_by_id (handler->rptcache, resource_id);
        if (rpt == NULL) {
                err("INVALID RESOURCE");
                return SA_ERR_HPI_INVALID_RESOURCE;
        }
        if (! (rpt->ResourceCapabilities & SAHPI_CAPABILITY_CONTROL)) {
                err("INVALID RESOURCE CAPABILITY");
                return SA_ERR_HPI_CAPABILITY;
        }

       rdr = oh_get_rdr_by_type (handler->rptcache, resource_id, SAHPI_CTRL_RDR,
                                 rdr_num);
        if (rdr == NULL) {
                err("INVALID RDR NUMBER");
                return (SA_ERR_HPI_NOT_PRESENT);
        }
        ctrl = &(rdr->RdrTypeUnion.CtrlRec);

       /* Validate the state specified in the parameter list */
        rv = oh_valid_ctrl_state_mode ((ctrl), mode, state);
        if (rv != SA_OK) {
                err("Control state specified is invalid");
                return (rv);
        }

        /* Auto mode is not supported */
        if (mode == SAHPI_CTRL_MODE_AUTO) {
                err( "AUTO CONTROL MODE is not supported");
                return SA_ERR_HPI_UNSUPPORTED_PARAMS;
        }

        /* If control mode is MANUAL and specified state is of digital type,
         * then the control state is updated with specified  state value
         */
        if (state->Type == SAHPI_CTRL_TYPE_DIGITAL) {
                ctrl->TypeUnion.Digital.Default = state->StateUnion.Digital;
                switch (state->StateUnion.Digital) {
                        case SAHPI_CTRL_STATE_OFF:
                                power_state = SAHPI_POWER_OFF;
                                rv = oa_soap_set_power_state(handler,
                                                             resource_id,
                                                             power_state);
                                if (rv != SA_OK) {
                                        err("Set power state failed");
                                        return (rv);
                                }
                                break;
                        case SAHPI_CTRL_STATE_ON:
                               power_state = SAHPI_POWER_ON;
                               rv = oa_soap_set_power_state(handler,
                                                            resource_id,
                                                            power_state);
                               if (rv != SA_OK) {
                                        err("Set power state failed");
                                        return (rv);
                               }
                               break;
                        default:
                                err("Invalid control state");
                                return (SA_ERR_HPI_INTERNAL_ERROR);
               }
        }

        return rv;
}

/**
 * build_server_control_rdr
 *      @oh_handler: Handler data pointer
 *      @rdr_num: Control rdr number
 *      @rdr: RDR pointer
 *
 * Purpose:
 *      Creates and adds the control rdr to server resource
 *
 * Detailed Description:
 *      - Models the power state of server blade as a control object of
 *        digital type
 *      - Plug-in does not support changing the control mode, mode is set
 *        as Manual mode and as read only
 *
 * Return values:
 *      SA_OK - Normal case
 *      SA_ERR_HPI_INVALID_PARAMS - On wrong parameter
 *      SA_ERR_HPI_INTERNAL_ERROR - oa_soap plugin has encountered an OA error
 **/
SaErrorT build_server_control_rdr(struct oh_handler_state *oh_handler,
                                  SaHpiInt32T rdr_num,
                                  SaHpiRdrT *rdr)
{
        char *server_ctrl_str = SERVER_CONTROL_STRING;
        struct oa_soap_handler *oa_handler = NULL;
        SaHpiResourceIdT resource_id;
        SaHpiRptEntryT *rpt = NULL;

        if (oh_handler == NULL ||  rdr == NULL) {
                err("Invalid parameters");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        oa_handler = (struct oa_soap_handler *) oh_handler->data;
        resource_id =
           oa_handler->oa_soap_resources.server.resource_id[rdr_num - 1];
        rpt = oh_get_resource_by_id(oh_handler->rptcache, resource_id);
        if (!rpt) {
                err("Could not find blade resource rpt");
                return(SA_ERR_HPI_INTERNAL_ERROR);
        }

        /* Set the control rdr with default values */
        rdr->Entity = rpt->ResourceEntity;
        rdr->RdrType = SAHPI_CTRL_RDR;
        rdr->RdrTypeUnion.CtrlRec.Num = OA_SOAP_RES_CNTRL_NUM;
        rdr->RdrTypeUnion.CtrlRec.OutputType = SAHPI_CTRL_POWER_STATE;
        rdr->RdrTypeUnion.CtrlRec.Type = SAHPI_CTRL_TYPE_DIGITAL;
        rdr->RdrTypeUnion.CtrlRec.TypeUnion.Digital.Default =
                SAHPI_CTRL_STATE_ON;
        rdr->RdrTypeUnion.CtrlRec.DefaultMode.Mode = SAHPI_CTRL_MODE_MANUAL;
        rdr->RdrTypeUnion.CtrlRec.DefaultMode.ReadOnly = SAHPI_TRUE;
        rdr->RdrTypeUnion.CtrlRec.WriteOnly = SAHPI_FALSE;
        rdr->IdString.DataType = SAHPI_TL_TYPE_TEXT;
        rdr->IdString.Language = SAHPI_LANG_ENGLISH;
        rdr->IdString.DataLength = strlen (server_ctrl_str) + 1;
        snprintf((char *) rdr->IdString.Data,
                 rdr->IdString.DataLength, "%s", server_ctrl_str);

        return SA_OK;
}

/**
 * build_interconnect_control_rdr
 *      @oh_handler: Handler data pointer
 *      @rdr_num: Control rdr number
 *      @rdr: Control rdr pointer
 *
 * Purpose:
 *      Creates and adds control rdr to interconnect resource
 *
 * Detailed Description:
 *      - Models the power state of interconnect blade as a control object of
 *        digital type
 *      - Plug-in does not support changing the control mode, mode is set
 *        as Manual mode and as read only
 *
 * Return values:
 *      SA_OK - Normal case
 *      SA_ERR_HPI_INVALID_PARAMS - On wrong parameter
 *      SA_ERR_HPI_INTERNAL_ERROR - oa_soap plugin has encountered an OA error
 **/
SaErrorT build_interconnect_control_rdr(struct oh_handler_state *oh_handler,
                                        SaHpiInt32T rdr_num,
                                        SaHpiRdrT *rdr)
{
        char *interconnect_ctrl_str = INTERCONNECT_CONTROL_STRING;
        struct oa_soap_handler *oa_handler = NULL;
        SaHpiResourceIdT resource_id;
        SaHpiRptEntryT *rpt = NULL;

        if (oh_handler == NULL || rdr == NULL) {
                err("Invalid parameters");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        oa_handler = (struct oa_soap_handler *) oh_handler->data;
        resource_id =
           oa_handler->oa_soap_resources.interconnect.resource_id[rdr_num - 1];
        rpt = oh_get_resource_by_id(oh_handler->rptcache, resource_id);
        if (!rpt) {
                err("Could not find blade resource rpt");
                return(SA_ERR_HPI_INTERNAL_ERROR);
        }

        /* Set the control rdr with default values */
        rdr->Entity = rpt->ResourceEntity;
        rdr->RdrType = SAHPI_CTRL_RDR;
        rdr->RdrTypeUnion.CtrlRec.Num = OA_SOAP_RES_CNTRL_NUM;
        rdr->RdrTypeUnion.CtrlRec.OutputType = SAHPI_CTRL_POWER_STATE;
        rdr->RdrTypeUnion.CtrlRec.Type = SAHPI_CTRL_TYPE_DIGITAL;
        rdr->RdrTypeUnion.CtrlRec.TypeUnion.Digital.Default =
                SAHPI_CTRL_STATE_ON;
        rdr->RdrTypeUnion.CtrlRec.DefaultMode.Mode = SAHPI_CTRL_MODE_MANUAL;
        rdr->RdrTypeUnion.CtrlRec.DefaultMode.ReadOnly = SAHPI_TRUE;
        rdr->RdrTypeUnion.CtrlRec.WriteOnly = SAHPI_FALSE;
        rdr->IdString.DataType = SAHPI_TL_TYPE_TEXT;
        rdr->IdString.Language = SAHPI_LANG_ENGLISH;
        rdr->IdString.DataLength = strlen(interconnect_ctrl_str) + 1;
        snprintf((char *) rdr->IdString.Data,
                 rdr->IdString.DataLength, "%s", interconnect_ctrl_str);

        return SA_OK;
}

void *oh_get_control_state (void *,
                            SaHpiResourceIdT,
                            SaHpiCtrlNumT,
                            SaHpiCtrlModeT *,
                            SaHpiCtrlStateT *)
               __attribute__ ((weak, alias ("oa_soap_get_control_state")));

void *oh_set_control_state (void *,
                            SaHpiResourceIdT,
                            SaHpiCtrlNumT,
                            SaHpiCtrlModeT,
                            SaHpiCtrlStateT *)
               __attribute__ ((weak, alias ("oa_soap_set_control_state")));
