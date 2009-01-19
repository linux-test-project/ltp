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
 *      Raghavendra P.G. <raghavendra.pg@hp.com>
 *
 *  This file handles all the control functionality related apis.
 *
 *      oa_soap_get_control_state       - Control ABI to return the control
 *                                        state and mode of the resource
 *
 *      oa_soap_set_control_state       - Control ABI to set the control
 *                                        state the resource
 *
 *      oa_soap_build_control_rdr        - Creates and adds the control rdr
 *
 * 	oa_soap_set_lcd_butn_lck_cntrl()- Sets the LCD button lock control
 *
 *	oa_soap_get_lcd_butn_lck_cntrl()- Gets the LCD button lock control
 *
 *	oa_soap_set_pwr_cntrl()		- Sets the power control
 *
 *	oa_soap_get_pwr_cntrl()		- Gets the power control
 *
 *	oa_soap_set_uid_cntrl()		- Sets the UID control
 *
 *	oa_soap_get_uid_cntrl()		- Gets the UID control
 *
 */

#include "oa_soap_control.h"

/* Forward declaraction for static functions */
static SaErrorT oa_soap_set_lcd_butn_lck_cntrl(struct oh_handler_state
						*oh_handler,
					       SaHpiRptEntryT *rpt,
					       SaHpiCtrlStateDigitalT
						control_state);
static SaErrorT oa_soap_get_lcd_butn_lck_cntrl(struct oh_handler_state
						*oh_handler,
					       SaHpiRptEntryT *rpt,
					       SaHpiCtrlStateDigitalT
						*control_state);
static SaErrorT oa_soap_set_pwr_cntrl(struct oh_handler_state *oh_handler,
				      SaHpiResourceIdT resource_id,
                                      SaHpiCtrlStateDigitalT control_state);
static SaErrorT oa_soap_get_pwr_cntrl(struct oh_handler_state *oh_handler,
				      SaHpiResourceIdT resource_id,
                                      SaHpiCtrlStateDigitalT *control_state);
static SaErrorT oa_soap_set_uid_cntrl(struct oh_handler_state *oh_handler,
                               	      SaHpiRptEntryT *rpt,
			              SaHpiCtrlStateDigitalT control_state);
static SaErrorT oa_soap_get_uid_cntrl(struct oh_handler_state *oh_handler,
				      SaHpiRptEntryT *rpt,
				      SaHpiCtrlStateDigitalT *control_state);

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
 *      SA_ERR_HPI_INTERNAL_ERROR - Internal error encountered
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
	SaHpiCtrlStateDigitalT control_digital_state;

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

        switch (rdr_num){
                case OA_SOAP_PWR_CNTRL:
                        rv = oa_soap_get_pwr_cntrl(handler, resource_id, 
						   &control_digital_state);
                        if (rv != SA_OK) {
                                err("Failed to get the power state RDR");
                                return rv;
                        }
			break;
                case OA_SOAP_UID_CNTRL:
                        rv = oa_soap_get_uid_cntrl(handler, rpt,
                                                   &control_digital_state);
                        if (rv != SA_OK) {
                                err("Failed to get the uid status");
                                return rv;
                        }
			break;
		case OA_SOAP_LCD_BUTN_LCK_CNTRL:
			rv = oa_soap_get_lcd_butn_lck_cntrl(handler, rpt,
							&control_digital_state);
                        if (rv != SA_OK) {
                                err("Failed to get the LCD button lock status");
                                return rv;
                        }
			break;
                default:
                        err("Invalid control rdr num");
                        return SA_ERR_HPI_INTERNAL_ERROR;
        }
        ctrl_state.StateUnion.Digital = control_digital_state;
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
 *      SA_ERR_HPI_INTERNAL_ERROR - Internal error encountered
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

        rdr = oh_get_rdr_by_type (handler->rptcache, resource_id, 
				  SAHPI_CTRL_RDR, rdr_num);
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

	/* Return error for Non digital type control request */
	if (state->Type != SAHPI_CTRL_TYPE_DIGITAL) {
		err("Control type not supported");
		return SA_ERR_HPI_INTERNAL_ERROR;
	}
	
        /* If control mode is MANUAL and specified state is of digital type,
         * then the control state is updated with specified  state value
         */
        ctrl->TypeUnion.Digital.Default = state->StateUnion.Digital;
	switch (rdr_num) {
		case OA_SOAP_PWR_CNTRL:
			rv = oa_soap_set_pwr_cntrl(handler, resource_id,
						   state->StateUnion.Digital);
			if (rv != SA_OK) {
				err("Set power state failed");
				return rv;
			}
			break;
		case OA_SOAP_UID_CNTRL:
			rv = oa_soap_set_uid_cntrl(handler, rpt,
						   state->StateUnion.Digital);
			if (rv != SA_OK) {
				err("Set uid state failed");
				return rv;
			}
			break;
		case OA_SOAP_LCD_BUTN_LCK_CNTRL:
			rv = oa_soap_set_lcd_butn_lck_cntrl(handler, rpt,
						state->StateUnion.Digital);
                        if (rv != SA_OK) {
                                err("Failed to set the LCD button lock status");
                                return rv;
                        }
			break;
		default:
			err("Invalid control rdr num");
			return SA_ERR_HPI_INTERNAL_ERROR;
	}
        return SA_OK;
}

/**
 * oa_soap_build_control_rdr:
 *      @oh_handler: Handler data pointer
 *      @rdr: Pointer to the rdr structure
 *      @resource_id: Resource ID
 *      @control_num: Control rdr number
 *
 * Purpose:
 *      Builds the control rdr.
 *
 * Detailed Description:
 *      - Retrieves and populates the control rdr contents from global controls 
 *	  array based in control num. 
 *
 * Return values:
 *      SA_OK - Normal case.
 *      SA_ERR_HPI_INVALID_PARAMS - On wrong parameter
 *      SA_ERR_HPI_INTERNAL_ERROR - Internal error encountered
 **/
SaErrorT oa_soap_build_control_rdr(struct oh_handler_state *oh_handler,
				   SaHpiRdrT *rdr,
				   SaHpiResourceIdT resource_id,
				   SaHpiCtrlNumT control_num)
{
        SaHpiRptEntryT *rpt = NULL;

        if (oh_handler == NULL || rdr == NULL) {
                err("Invalid parameters");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        rpt = oh_get_resource_by_id(oh_handler->rptcache, resource_id);
        if (!rpt) {
                err("Could not find blade resource rpt");
                return(SA_ERR_HPI_INTERNAL_ERROR);
        }

        /* Set the control rdr with default values */
        rdr->Entity = rpt->ResourceEntity;
        rdr->RdrType = SAHPI_CTRL_RDR;
	rdr->RdrTypeUnion.CtrlRec = oa_soap_cntrl_arr[control_num].control;

	oh_init_textbuffer(&(rdr->IdString));
	oh_append_textbuffer(&(rdr->IdString),
			oa_soap_cntrl_arr[control_num].comment);
        return SA_OK;
}

/**
 * oa_soap_get_uid_cntrl
 *      @oh_handler: Handler data pointer
 *      @rpt: Pointer to the rpt structure
 *      @control_state: Pointer to digital control state
 *
 * Purpose:
 *      Retrieves the UID status of the resource 
 *
 * Detailed Description:
 *      - Retrieves the UID status of different resources based on entity type.
 *
 * Return values:
 *      SA_OK - Normal case.
 *      SA_ERR_HPI_INVALID_PARAMS - On wrong parameter
 *      SA_ERR_HPI_INTERNAL_ERROR - Internal error encountered
 **/
static SaErrorT oa_soap_get_uid_cntrl(struct oh_handler_state *oh_handler,
				      SaHpiRptEntryT *rpt,
				      SaHpiCtrlStateDigitalT *control_state)
{
        SaErrorT rv = SA_OK;
        SaHpiInt32T bay_number;
        struct oa_soap_handler *oa_handler = NULL;
        struct enclosureStatus enclosure_status_response; 
        struct getOaStatus oa_status_request; 
        struct oaStatus oa_status_response; 
        struct getBladeStatus server_status_request;
        struct bladeStatus server_status_response;
        struct getInterconnectTrayStatus interconnect_status_request; 
        struct interconnectTrayStatus interconnect_status_response; 
	enum uidStatus  uid_status;

        if (oh_handler == NULL || rpt == NULL || control_state == NULL) {
                err("Invalid parameters");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        oa_handler = (struct oa_soap_handler *) oh_handler->data;

        /* Check whether the re-discovery is started by trying to lock the
	 * oa_handler mutex
	 */
        rv = lock_oa_soap_handler(oa_handler);
        if (rv != SA_OK) {
                err("OA SOAP handler is locked");
                return rv;
        }
        
        bay_number = rpt->ResourceEntity.Entry[0].EntityLocation;
        switch (rpt->ResourceEntity.Entry[0].EntityType) {
                case (SAHPI_ENT_SYSTEM_CHASSIS):
			/* Make a soap call to get the enclosure UID status */
                        rv = soap_getEnclosureStatus(oa_handler->active_con, 
                                                    &enclosure_status_response);
                        if (rv != SOAP_OK) {
                                err("Get enclosure status failed");
                                return SA_ERR_HPI_INTERNAL_ERROR;
                        }
                        uid_status = enclosure_status_response.uid;
                        break;
                case (SAHPI_ENT_SYS_MGMNT_MODULE):
			/* Make a soap call to get the OA UID status */
                        oa_status_request.bayNumber = bay_number;
                        rv = soap_getOaStatus(oa_handler->active_con, 
                                              &oa_status_request,
                                              &oa_status_response);
                        if (rv != SOAP_OK) {
                                err("Get OA status failed");
                                return SA_ERR_HPI_INTERNAL_ERROR;
                        }
                        uid_status = oa_status_response.uid;
                        break;
                case (SAHPI_ENT_SYSTEM_BLADE):
                case (SAHPI_ENT_IO_BLADE):
                case (SAHPI_ENT_DISK_BLADE):
			/* Make a soap call to get the Blade UID status */
                        server_status_request.bayNumber = bay_number;
                        rv = soap_getBladeStatus(oa_handler->active_con, 
                                                 &server_status_request,
                                                 &server_status_response);
                        if (rv != SOAP_OK) {
                                err("Get Blade status failed");
                                return SA_ERR_HPI_INTERNAL_ERROR;
                        }
                        uid_status = server_status_response.uid;
                        break;
                case (SAHPI_ENT_SWITCH_BLADE):
			/* Make a soap call to get the Interconnect 
			 * UID status 
			 */
                        interconnect_status_request.bayNumber = bay_number;
                        rv = soap_getInterconnectTrayStatus(
						 oa_handler->active_con, 
                                                 &interconnect_status_request,
                                                 &interconnect_status_response);
                        if (rv != SOAP_OK) {
                                err("Get Interconnect status failed");
                                return SA_ERR_HPI_INTERNAL_ERROR;
                        }
                        uid_status = interconnect_status_response.uid;
                        break;
                default:
                        err("Invalid Resource Type");
                        return (SA_ERR_HPI_INTERNAL_ERROR);
        }

	switch (uid_status) {
		case UID_ON:
			*control_state = SAHPI_CTRL_STATE_ON;
			break;
		case UID_OFF:
		case UID_NO_OP:
			*control_state = SAHPI_CTRL_STATE_OFF;
			break;
		default:
			err("Invalid uid status");
			return SA_ERR_HPI_INTERNAL_ERROR;
	}
	return SA_OK;
}

/**
 * oa_soap_set_uid_cntrl
 *      @oh_handler: Handler data pointer
 *      @rpt: Pointer to the rpt structure
 *      @control_state: Digital control state
 *
 * Purpose:
 *      Sets the UID status of the resource 
 *
 * Detailed Description:
 *      - Sets the UID status of different resources based on entity type.
 *
 * Return values:
 *      SA_OK - Normal case.
 *      SA_ERR_HPI_INVALID_PARAMS - On wrong parameter
 *      SA_ERR_HPI_INTERNAL_ERROR - Internal error encountered
 **/
static SaErrorT oa_soap_set_uid_cntrl(struct oh_handler_state *oh_handler,
                               	      SaHpiRptEntryT *rpt,
			              SaHpiCtrlStateDigitalT control_state)
{
        SaErrorT rv = SA_OK;
        SaHpiInt32T bay_number;
        struct oa_soap_handler *oa_handler = NULL;
	struct setEnclosureUid enclosure_uid;
	struct setOaUid	oa_uid;
	struct setBladeUid blade_uid;
	struct setInterconnectTrayUid interconnect_uid;
	enum uidStatus  uid_status;

        if (oh_handler == NULL || rpt == NULL) {
                err("Invalid parameters");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        oa_handler = (struct oa_soap_handler *) oh_handler->data;

        /* Check whether the re-discovery is started by trying to lock the
	 * oa_handler mutex
	 */
        rv = lock_oa_soap_handler(oa_handler);
        if (rv != SA_OK) {
                err("OA SOAP handler is locked");
                return rv;
        }

	/* Return error if the control state is PULSE_ON or PULSE_OFF */
	if ((control_state == SAHPI_CTRL_STATE_PULSE_ON) ||
	    (control_state == SAHPI_CTRL_STATE_PULSE_OFF)) {
		err("Setting the control state to PULSE ON/OFF not supported");
		return SA_ERR_HPI_INVALID_DATA;
	}

	/* Map the control state to uid status */
	if (control_state == SAHPI_CTRL_STATE_ON) {
		uid_status = UID_CMD_ON;
	} else {
		uid_status = UID_CMD_OFF;
	}
        
	/* Get the bay_number of the resource */
        bay_number = rpt->ResourceEntity.Entry[0].EntityLocation;
        switch (rpt->ResourceEntity.Entry[0].EntityType) {
                case (SAHPI_ENT_SYSTEM_CHASSIS):
			/* Make a soap call to set Enclosure UID */
			enclosure_uid.uid = uid_status;
                        rv = soap_setEnclosureUid(oa_handler->active_con, 
                                                 &enclosure_uid);
                        if (rv != SOAP_OK) {
                                err("Set enclosure UID failed");
                                return SA_ERR_HPI_INTERNAL_ERROR;
                        }
                        break;
                case (SAHPI_ENT_SYS_MGMNT_MODULE):
			/* Make a soap call to set OA UID */
			oa_uid.uid = uid_status;
			oa_uid.bayNumber = bay_number;
			rv = soap_setOaUid(oa_handler->active_con,
					   &oa_uid);
                        if (rv != SOAP_OK) {
                                err("Set OA UID failed");
                                return SA_ERR_HPI_INTERNAL_ERROR;
                        }
                        break;
                case (SAHPI_ENT_SYSTEM_BLADE):
                case (SAHPI_ENT_IO_BLADE):
                case (SAHPI_ENT_DISK_BLADE):
			/* Make a soap call to set Blade UID */
			blade_uid.uid = uid_status;
			blade_uid.bayNumber = bay_number;
			rv = soap_setBladeUid(oa_handler->active_con,
					   &blade_uid);
                        if (rv != SOAP_OK) {
                                err("Set Blade UID failed");
                                return SA_ERR_HPI_INTERNAL_ERROR;
                        }
                        break;
                case (SAHPI_ENT_SWITCH_BLADE):
			/* Make a soap call to set Interconnect UID */
			interconnect_uid.uid = uid_status;
			interconnect_uid.bayNumber = bay_number;
			rv = soap_setInterconnectTrayUid(oa_handler->active_con,
					   &interconnect_uid);
                        if (rv != SOAP_OK) {
                                err("Set Interconnect UID failed");
                                return SA_ERR_HPI_INTERNAL_ERROR;
                        }
                        break;
                default:
                        err("Invalid Resource Type");
                        return (SA_ERR_HPI_INTERNAL_ERROR);
        }
        return SA_OK;
}

/**
 * oa_soap_get_pwr_cntrl:
 *      @oh_handler: Handler data pointer
 *      @resource_id: Resource id
 *      @control state: Pointer to digital control state
 *
 * Purpose:
 *      Gets the control state of power controls on resource
 *
 * Detailed Description:
 *      - Gets the power state of resource and maps it to control state
 *
 * Return values:
 *      SA_OK - Normal case.
 *      SA_ERR_HPI_INVALID_PARAMS - On wrong parameter
 *      SA_ERR_HPI_INTERNAL_ERROR - Internal error encountered
 **/
static SaErrorT oa_soap_get_pwr_cntrl(struct oh_handler_state *oh_handler,
				      SaHpiResourceIdT resource_id,
                                      SaHpiCtrlStateDigitalT *control_state)
{
        SaErrorT rv = SA_OK;
        SaHpiPowerStateT power_state;

	if (oh_handler == NULL || control_state == NULL) {
		err("Invalid parameters");
		return SA_ERR_HPI_INVALID_PARAMS;
	}
	
	rv = oa_soap_get_power_state(oh_handler, resource_id,
				     &power_state);
	if (rv != SA_OK) {
		err("Failed to get the power state RDR");
		return rv;
	}

	switch (power_state) {
		case SAHPI_POWER_ON:
			*control_state = SAHPI_CTRL_STATE_ON;
			break;
		case SAHPI_POWER_OFF:
			*control_state = SAHPI_CTRL_STATE_OFF;
			break;
		default:
			err("Invalid power state");
			return SA_ERR_HPI_INTERNAL_ERROR;
	}

	return SA_OK;
}

/**
 * oa_soap_set_pwr_cntrl:
 *      @oh_handler: Handler data pointer
 *      @resource_id: Resource id
 *      @control state: Digital control state
 *
 * Purpose:
 *      Sets the control state of power controls on resource
 *
 * Detailed Description:
 *      - Sets the power state of resource after mapping the control state
 *	  appropriate power state
 *
 * Return values:
 *      SA_OK - Normal case.
 *      SA_ERR_HPI_INVALID_PARAMS - On wrong parameter
 *      SA_ERR_HPI_INTERNAL_ERROR - Internal error encountered
 **/
static SaErrorT oa_soap_set_pwr_cntrl(struct oh_handler_state *oh_handler,
				      SaHpiResourceIdT resource_id,
                                      SaHpiCtrlStateDigitalT control_state)
{
        SaErrorT rv = SA_OK;
        SaHpiPowerStateT power_state;

	if (oh_handler == NULL) {
		err("Invalid parameters");
		return SA_ERR_HPI_INVALID_PARAMS;
	}
	
	/* Return error if the control state is PULSE_ON or PULSE_OFF */
	if ((control_state == SAHPI_CTRL_STATE_PULSE_ON) ||
	    (control_state == SAHPI_CTRL_STATE_PULSE_OFF)) {
		err("Setting the control state to PULSE ON/OFF not supported");
		return SA_ERR_HPI_INVALID_DATA;
	}

	if (control_state == SAHPI_CTRL_STATE_ON) {
		power_state = SAHPI_POWER_ON;
	} else {
		power_state = SAHPI_POWER_OFF;
	}

	rv = oa_soap_set_power_state(oh_handler, resource_id,
				     power_state);
	if (rv != SA_OK) {
		err("Failed to set the power state of resource");
		return rv;
	}
	return SA_OK;
}

/**
 * oa_soap_get_lcd_butn_lck_cntrl_state
 *      @oh_handler: Handler data pointer
 *      @rpt: Pointer to the rpt structure
 *      @control_state: Pointer to digital control state
 *
 * Purpose:
 *      Retrieves the LCD button lock state
 *
 * Detailed Description:
 * 	NA
 *
 * Return values:
 *      SA_OK - Normal case.
 *      SA_ERR_HPI_INVALID_PARAMS - On wrong parameter
 *      SA_ERR_HPI_INTERNAL_ERROR - Internal error encountered
 **/
static SaErrorT oa_soap_get_lcd_butn_lck_cntrl(struct oh_handler_state
						*oh_handler,
					       SaHpiRptEntryT *rpt,
					       SaHpiCtrlStateDigitalT
						*control_state)
{
        SaErrorT rv = SA_OK;
        struct oa_soap_handler *oa_handler = NULL;
        struct lcdStatus status;

        if (oh_handler == NULL || rpt == NULL || control_state == NULL) {
                err("Invalid parameters");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        oa_handler = (struct oa_soap_handler *) oh_handler->data;

        if (rpt->ResourceEntity.Entry[0].EntityType !=
	    SAHPI_ENT_DISPLAY_PANEL) {
                err("LCD button lock is supported only on LCD");
                err("Requested on wrong resource type %d",
		    rpt->ResourceEntity.Entry[0].EntityType);
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        /* Check whether the re-discovery is started by trying to lock the
	 * oa_handler mutex
	 */
        rv = lock_oa_soap_handler(oa_handler);
        if (rv != SA_OK) {
                err("OA SOAP handler is locked");
                return rv;
        }
        
	/* Make a soap call to set Enclosure UID */
	rv = soap_getLcdStatus(oa_handler->active_con, &status);
	if (rv != SOAP_OK) {
		err("Get LCD status SOAP call failed");
		return SA_ERR_HPI_INTERNAL_ERROR;
	}

	switch (status.buttonLock) {
		case HPOA_TRUE:
			*control_state = SAHPI_CTRL_STATE_ON;
			break;
		case HPOA_FALSE:
			*control_state = SAHPI_CTRL_STATE_OFF;
			break;
		default:
			err("Invalid LCD button lock state");
			return SA_ERR_HPI_INTERNAL_ERROR;
	}

	return SA_OK;
}

/**
 * oa_soap_set_lcd_butn_lck_cntrl_state
 *      @oh_handler: Handler data pointer
 *      @rpt: Pointer to the rpt structure
 *      @control_state: Digital control state
 *
 * Purpose:
 *      Sets the LCD button lock state
 *
 * Detailed Description:
 *	- The LCD button lock control is supported only for LCD resource
 *	- PULSE_ON and PULSE_OFF control states are not supported. If requested
 *	  by the user, then error message is returned
 *	- Sets the LCD button lock control to requested state
 *
 * Return values:
 *      SA_OK - Normal case.
 *      SA_ERR_HPI_INVALID_PARAMS - On wrong parameter
 *      SA_ERR_HPI_INTERNAL_ERROR - Internal error encountered
 **/
static SaErrorT oa_soap_set_lcd_butn_lck_cntrl(struct oh_handler_state
						*oh_handler,
					       SaHpiRptEntryT *rpt,
					       SaHpiCtrlStateDigitalT
						control_state)
{
        SaErrorT rv = SA_OK;
        struct oa_soap_handler *oa_handler = NULL;
	enum hpoa_boolean lcd_button_lock;

        if (oh_handler == NULL || rpt == NULL) {
                err("Invalid parameters");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        oa_handler = (struct oa_soap_handler *) oh_handler->data;

        if (rpt->ResourceEntity.Entry[0].EntityType !=
	    SAHPI_ENT_DISPLAY_PANEL) {
                err("LCD button lock is supported only on LCD");
                err("Requested on wrong resource type %d",
		    rpt->ResourceEntity.Entry[0].EntityType);
                return SA_ERR_HPI_INVALID_PARAMS;
        }

	/* Return error if the control state is PULSE_ON or PULSE_OFF */
	if ((control_state == SAHPI_CTRL_STATE_PULSE_ON) ||
	    (control_state == SAHPI_CTRL_STATE_PULSE_OFF)) {
		err("Setting the control state to PULSE ON/OFF not supported");
		return SA_ERR_HPI_INVALID_DATA;
	}

	/* Map the control state to LCD button lock state */
	if (control_state == SAHPI_CTRL_STATE_ON) {
		lcd_button_lock = HPOA_TRUE;
	} else {
		lcd_button_lock = HPOA_FALSE;
	}
        
        /* Check whether the re-discovery is started by trying to lock the
	 * oa_handler mutex
	 */
        rv = lock_oa_soap_handler(oa_handler);
        if (rv != SA_OK) {
                err("OA SOAP handler is locked");
                return rv;
        }

	rv = soap_setLcdButtonLock(oa_handler->active_con, lcd_button_lock);
	if (rv != SOAP_OK) {
		err("Set LCD button lock SOAP call failed");
		return SA_ERR_HPI_INTERNAL_ERROR;
	}

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
