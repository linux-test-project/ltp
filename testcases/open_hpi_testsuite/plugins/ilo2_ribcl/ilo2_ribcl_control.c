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
 *     Shuah Khan <shuah.khan@hp.com>
 *     Richard White <richard.white@hp.com>
 */

/***************
 * This source file contains Control HPI ABI routines iLO2 RIBCL plug-in
 * implements. Other source files provide support functionality for
 * these ABIs.
***************/

#include <ilo2_ribcl.h>
#include <ilo2_ribcl_ssl.h>
#include <ilo2_ribcl_xml.h>

/************************************
	Forward declarations for static functions in this file
************************************/
static SaErrorT ilo2_ribcl_get_uid_status(ilo2_ribcl_handler_t *,
	SaHpiCtrlStateDigitalT *);
static SaErrorT ilo2_ribcl_set_uid_status(ilo2_ribcl_handler_t *,
	SaHpiCtrlStateDigitalT );
static SaErrorT ilo2_ribcl_get_power_saver_status(ilo2_ribcl_handler_t *,
	SaHpiCtrlStateDiscreteT *);
static SaErrorT ilo2_ribcl_set_power_saver_status(ilo2_ribcl_handler_t *,
	SaHpiCtrlStateDiscreteT );
static SaErrorT ilo2_ribcl_get_auto_power_status(ilo2_ribcl_handler_t *,
	SaHpiCtrlStateDiscreteT *);
static SaErrorT ilo2_ribcl_set_auto_power_status(ilo2_ribcl_handler_t *,
	SaHpiCtrlStateDiscreteT);

/*****************************
	iLO2 RIBCL plug-in Control ABI Interface functions
*****************************/
/**
 * ilo2_ribcl_get_control_state:
 * @hnd: Handler data pointer.
 * @rid: Resource ID.
 * @cid: Control ID.
 * @mode: Location to store control's operational mode.
 * @state: Location to store control's state.
 *
 * Retrieves a control's operational mode and/or state. Both @mode and @state
 * may be NULL (e.g. check for presence).
 *
 * Return values:
 * SA_OK - Normal case.
 * SA_ERR_HPI_CAPABILITY - Resource doesn't have SAHPI_CAPABILITY_CONTROL.
 * SA_ERR_HPI_INVALID_CMD - Control is write-only.
 * SA_ERR_HPI_INVALID_DATA - @state contain invalid text line number.
 * SA_ERR_HPI_INVALID_RESOURCE - Resource doesn't exist.
 * SA_ERR_HPI_NOT_PRESENT - Control doesn't exist.
 **/
SaErrorT ilo2_ribcl_get_control_state(void *hnd, SaHpiResourceIdT rid,
	SaHpiCtrlNumT cid, SaHpiCtrlModeT *mode, SaHpiCtrlStateT *state)
{
        struct oh_handler_state *handle = NULL;
	ilo2_ribcl_handler_t *ilo2_ribcl_handler = NULL;
	SaHpiRptEntryT *rpt = NULL;
        SaHpiRdrT *rdr = NULL;
	ilo2_ribcl_cinfo_t *cinfo_ptr = NULL;
	SaHpiCtrlStateDiscreteT status;

	if (!hnd || !state) {
		err("ilo2_ribcl_get_control_state(): Invalid parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}

	handle = (struct oh_handler_state *)hnd;
	ilo2_ribcl_handler = (ilo2_ribcl_handler_t *)handle->data;
	
	if (!ilo2_ribcl_handler) {
		err("ilo2_ribcl_get_control_state(): Invalid parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}

	/* Check if resource exists and has control capabilities */
	rpt = oh_get_resource_by_id(handle->rptcache, rid);
        if (!rpt) {
		err("ilo2_ribcl_get_control_state(): Invalid resource.");
		return(SA_ERR_HPI_INVALID_RESOURCE);
	}

	if (!(rpt->ResourceCapabilities & SAHPI_CAPABILITY_CONTROL)) {
		err("ilo2_ribcl_get_control_state(): Resource doesn't have control capability.");
		return(SA_ERR_HPI_CAPABILITY);
	} 
	
	/* Find control and its mapping data - see if it accessable */
	rdr = oh_get_rdr_by_type(handle->rptcache, rid, SAHPI_CTRL_RDR, cid);
	if (rdr == NULL) {
		err("ilo2_ribcl_get_control_state(): Control RDR is not present.");
		return(SA_ERR_HPI_NOT_PRESENT);
	}

	/* Fetch RDR data to determine the control type */
	cinfo_ptr = (ilo2_ribcl_cinfo_t *) oh_get_rdr_data(handle->rptcache,
		rid, rdr->RecordId);
	if(cinfo_ptr == NULL) {
		err("ilo2_ribcl_get_control_state(): No control data. Control=%s", rdr->IdString.Data);
		return(SA_ERR_HPI_INTERNAL_ERROR);
	}
	/* check if this control has WriteOnly flag set. If so return
	   SA_ERR_HPI_INVALID_CMD. Please see the OpenHPI Spec for details */
	if (rdr->RdrTypeUnion.CtrlRec.WriteOnly) {
		err("ilo2_ribcl_get_control_state(): WriteOnly Control=%s", rdr->IdString.Data);
		return(SA_ERR_HPI_INVALID_CMD);
	}

	/* if mode and state are null return SA_OK. Spec allows these out
	   parms to be null and allows this call to return without setting
	   mode and/or state respectively */
	if(!mode && !state) {
		return(SA_OK);
	}
	
	switch(cinfo_ptr->ctl_type) {
		case ILO2_RIBCL_CTL_UID:
		{
			if(mode) {
				*mode = cinfo_ptr->cur_mode;
			}
			if(state) {

				SaErrorT ret = SA_OK;

				/* send GET_UID_STATUS to iLO to get the
				   current status of the UID */
				if((ret = ilo2_ribcl_get_uid_status(
					ilo2_ribcl_handler,
					&status)) != SA_OK) {
					err("ilo2_ribcl_get_control_state(): Get Uid Status failed for Control=%s", rdr->IdString.Data);
					return(ret);
				}
				state->Type = rdr->RdrTypeUnion.CtrlRec.Type;
				state->StateUnion.Digital = status;

				/* update private data current state */
				cinfo_ptr->cur_state.Digital = status;
			}
		}
		break;
		case ILO2_RIBCL_CTL_POWER_SAVER:
		{
			if(mode) {
				*mode = cinfo_ptr->cur_mode;
			}
			if(state) {

				SaErrorT ret = SA_OK;

				/* send GET_HOST_POWER_SAVER_STATUS to iLO to
				   get the current Power Regulator setting */
				if((ret = ilo2_ribcl_get_power_saver_status(
					ilo2_ribcl_handler,
					&status)) != SA_OK) {
					err("ilo2_ribcl_get_control_state(): Get Uid Status failed for Control=%s", rdr->IdString.Data);
					return(ret);
				}
				state->Type = rdr->RdrTypeUnion.CtrlRec.Type;
				state->StateUnion.Discrete = status;

				/* update private data current state */
				cinfo_ptr->cur_state.Discrete = status;
			}
		}
		break;
		case ILO2_RIBCL_CTL_AUTO_POWER:
		{
			if(mode) {
				*mode = cinfo_ptr->cur_mode;
			}
			if(state) {

				SaErrorT ret = SA_OK;

				/* send GET_SERVER_AUTO_PWR to iLO to
				   get the current Auto Power Status */
				if((ret = ilo2_ribcl_get_auto_power_status(
					ilo2_ribcl_handler,
					&status)) != SA_OK) {
					err("ilo2_ribcl_get_control_state(): Get Uid Status failed for Control=%s", rdr->IdString.Data);
					return(ret);
				}
				state->Type = rdr->RdrTypeUnion.CtrlRec.Type;
				state->StateUnion.Discrete = status;

				/* update private data current state */
				cinfo_ptr->cur_state.Discrete = status;
			}
		}
		break;
		default:
		{
			err("ilo2_ribcl_get_control_state(): Invalid internal control type for Control=%s", rdr->IdString.Data);
			return(SA_ERR_HPI_INTERNAL_ERROR);
		}
	}

	return(SA_OK);
}

/**
 * ilo2_ribcl_set_control_state:
 * @hnd: Handler data pointer.
 * @rid: Resource ID.
 * @cid: Control ID.
 * @mode: Control's operational mode to set.
 * @state: Pointer to control's state to set.
 *
 * Sets a control's operational mode and/or state. 
 *
 * Return values:
 * SA_OK - Normal case.
 * SA_ERR_HPI_CAPABILITY - Resource doesn't have SAHPI_CAPABILITY_CONTROL.
 * SA_ERR_HPI_INVALID_REQUEST - @state contains bad text control data.
 * SA_ERR_HPI_INVALID_RESOURCE - Resource doesn't exist.
 * SA_ERR_HPI_NOT_PRESENT - Control doesn't exist.
 * SA_ERR_HPI_READ_ONLY - Change mode of a read-only mode control.
 *                        Note this is only returned if the specified mode
 *                        is different than the control's default mode.
 **/
SaErrorT ilo2_ribcl_set_control_state(void *hnd, SaHpiResourceIdT rid,
	SaHpiCtrlNumT cid, SaHpiCtrlModeT mode, SaHpiCtrlStateT *state)
{
        struct oh_handler_state *handle = NULL;
	ilo2_ribcl_handler_t *ilo2_ribcl_handler = NULL;
	SaHpiRptEntryT *rpt = NULL;
        SaHpiRdrT *rdr = NULL;
	ilo2_ribcl_cinfo_t *cinfo_ptr = NULL;
	SaErrorT ret;

	if (!hnd || !state) {
		err("ilo2_ribcl_get_control_state(): Invalid parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}

	handle = (struct oh_handler_state *)hnd;
	ilo2_ribcl_handler = (ilo2_ribcl_handler_t *)handle->data;
	
	if (!ilo2_ribcl_handler) {
		err("ilo2_ribcl_get_control_state(): Invalid parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}

	/* Check if resource exists and has control capabilities */
	rpt = oh_get_resource_by_id(handle->rptcache, rid);
        if (!rpt) {
		err("ilo2_ribcl_get_control_state(): Invalid resource.");
		return(SA_ERR_HPI_INVALID_RESOURCE);
	}

	if (!(rpt->ResourceCapabilities & SAHPI_CAPABILITY_CONTROL)) {
		err("ilo2_ribcl_get_control_state(): Resource doesn't have control capability.");
		return(SA_ERR_HPI_CAPABILITY);
	} 
	
	/* Find control and its mapping data - see if it accessable */
	rdr = oh_get_rdr_by_type(handle->rptcache, rid, SAHPI_CTRL_RDR, cid);
	if (rdr == NULL) {
		err("ilo2_ribcl_get_control_state(): Control RDR is not present.");
		return(SA_ERR_HPI_NOT_PRESENT);
	}

	/* Fetch RDR data to determine the control type */
	cinfo_ptr = (ilo2_ribcl_cinfo_t *) oh_get_rdr_data(handle->rptcache,
		rid, rdr->RecordId);
	if(cinfo_ptr == NULL) {
		err("ilo2_ribcl_get_control_state(): No control data. Control=%s", rdr->IdString.Data);
		return(SA_ERR_HPI_INTERNAL_ERROR);
	}
	/* validate static control state and mode data */
	 ret = oh_valid_ctrl_state_mode(&(rdr->RdrTypeUnion.CtrlRec),
		mode, state);
	if(ret) {
		return(ret);
	}

	/* send appropriate RIBCL command to iLO2 to set the new control
	   state */
	if(mode != SAHPI_CTRL_MODE_AUTO && state) {
		switch(cinfo_ptr->ctl_type) {
			case ILO2_RIBCL_CTL_UID:
			{
				/* Requested state is same as the current.
				   return SA_OK */
				if(cinfo_ptr->cur_state.Digital == 
					state->StateUnion.Digital) {
					return(SA_OK);
				}
				if((ret = ilo2_ribcl_set_uid_status(
					ilo2_ribcl_handler,
					state->StateUnion.Digital)) != SA_OK) {
					return(ret);
				}
				/* control has been changed to the requested
				   status value, now update the private status
				   info. */
				cinfo_ptr->cur_state.Digital = state->StateUnion.Digital;
			}
			break;
			case ILO2_RIBCL_CTL_POWER_SAVER:
			{
				/* Requested state is same as the current.
				   return SA_OK */
				if(cinfo_ptr->cur_state.Discrete == 
					state->StateUnion.Discrete) {
					return(SA_OK);
				}
				if((ret = ilo2_ribcl_set_power_saver_status(
					ilo2_ribcl_handler,
					state->StateUnion.Discrete)) != SA_OK) {
					return(ret);
				}
				/* control has been changed to the requested
				   status value, now update the private status
				   info. */
				cinfo_ptr->cur_state.Discrete = state->StateUnion.Discrete;
			}
			break;
			case ILO2_RIBCL_CTL_AUTO_POWER:
			{
				/* Requested state is same as the current.
				   return SA_OK */
				if(cinfo_ptr->cur_state.Discrete == 
					state->StateUnion.Discrete) {
					return(SA_OK);
				}
				if((ret = ilo2_ribcl_set_auto_power_status(
					ilo2_ribcl_handler,
					state->StateUnion.Discrete)) != SA_OK) {
					return(ret);
				}
				/* control has been changed to the requested
				   status value, now update the private status
				   info. */
				cinfo_ptr->cur_state.Discrete = state->StateUnion.Discrete;
			}
			break;
			default:
			{
				err("ilo2_ribcl_set_control_state(): Invalid internal control type for Control=%s", rdr->IdString.Data);
				return(SA_ERR_HPI_INTERNAL_ERROR);
			}
		}
	}

	return(SA_OK);
}

/**
 * ilo2_ribcl_get_uid_status:
 * @hnd: Private Handler data pointer.
 * @status: Location to store control's state.
 *
 * This routine send GET_UID_STATUS RIBCL command to iLO and gets back
 * the current UID status. This status then gets mapped to HPI status
 * before returning. 
 * Return values:
 *	SA_OK
 *	SA_ERR_HPI_OUT_OF_MEMORY
 *	SA_ERR_HPI_INTERNAL_ERROR
**/
static SaErrorT ilo2_ribcl_get_uid_status(ilo2_ribcl_handler_t *hnd,
	SaHpiCtrlStateDigitalT *status)
{
	char *uid_cmd;
	char *response;	/* command response buffer */
	int ret;
	int uid_status = -1;

	/* Allocate a temporary response buffer. */
	response = malloc(ILO2_RIBCL_BUFFER_LEN);
	if( response == NULL){
		err("ilo2_ribcl_get_uid_status: Out of memory.");
		return(SA_ERR_HPI_OUT_OF_MEMORY);
	}

	/* Retrieve our customized command buffer */
	uid_cmd = hnd->ribcl_xml_cmd[IR_CMD_GET_UID_STATUS];
	if( uid_cmd == NULL){
		err("ilo2_ribcl_get_uid_status: null customized command.");
		free( response);
		return( SA_ERR_HPI_INTERNAL_ERROR);
	}

	/* Send the command to iLO2 and get the response. */
	ret = ilo2_ribcl_ssl_send_command( hnd, uid_cmd,
				response, ILO2_RIBCL_BUFFER_LEN);

	if( ret != 0){
		err("ilo2_ribcl_get_uid_status: command send failed.");
		free( response);
		return( SA_ERR_HPI_INTERNAL_ERROR);
	}

	/* Now, parse the response. */
	ret = ir_xml_parse_uid_status(response, &uid_status,
			hnd->ilo2_hostport);
	if( ret != RIBCL_SUCCESS){
		err("ilo2_ribcl_get_uid_status: response parse failed.");
		free( response); 
		return( SA_ERR_HPI_INTERNAL_ERROR);
	}

	/* We're finished. Free up the temporary response buffer */
	free( response);

	if(uid_status == ILO2_RIBCL_UID_ON) {
		*status = SAHPI_CTRL_STATE_ON;
	} else if (uid_status == ILO2_RIBCL_UID_OFF) {
		*status = SAHPI_CTRL_STATE_OFF;
	} else {
		return( SA_ERR_HPI_INTERNAL_ERROR);
	}

	return(SA_OK);
}

/**
 * ilo2_ribcl_set_uid_status:
 * @hnd: Private Handler data pointer.
 * @status: Location to store control's state.
 *
 * This routine send UID_CONTROL RIBCL command to iLO to change the 
 * current UID status. This routine maps the passed in HPI status to
 * RIBCL status before sending the command.
 * Return values:
 *	SA_OK
 *	SA_ERR_HPI_OUT_OF_MEMORY
 *	SA_ERR_HPI_INTERNAL_ERROR
 *	SA_ERR_HPI_INVALID_PARAMS
**/
static SaErrorT ilo2_ribcl_set_uid_status(ilo2_ribcl_handler_t *hnd,
	SaHpiCtrlStateDigitalT status)
{
	char *uid_cmd;
	char *response;	/* command response buffer */
	int ret;

	if((status != SAHPI_CTRL_STATE_OFF) &&
		(status != SAHPI_CTRL_STATE_ON)) {
		return(SA_ERR_HPI_INVALID_PARAMS);
	}

	/* Allocate a temporary response buffer. */
	response = malloc(ILO2_RIBCL_BUFFER_LEN);
	if( response == NULL){
		err("ilo2_ribcl_set_uid_status: Out of memory.");
		return(SA_ERR_HPI_OUT_OF_MEMORY);
	}

	/* Retrieve our customized command buffer */
	if(status == SAHPI_CTRL_STATE_OFF) {
		uid_cmd = hnd->ribcl_xml_cmd[IR_CMD_UID_CONTROL_OFF];
	} else {
		uid_cmd = hnd->ribcl_xml_cmd[IR_CMD_UID_CONTROL_ON];
	}

	if( uid_cmd == NULL){
		err("ilo2_ribcl_set_uid_status: null customized command.");
		free( response);
		return( SA_ERR_HPI_INTERNAL_ERROR);
	}

	/* Send the command to iLO2 and get the response. */
	ret = ilo2_ribcl_ssl_send_command( hnd, uid_cmd,
				response, ILO2_RIBCL_BUFFER_LEN);

	if( ret != 0){
		err("ilo2_ribcl_set_uid_status: command send failed.");
		free( response);
		return( SA_ERR_HPI_INTERNAL_ERROR);
	}

	/* Now, parse the response. */
	ret = ir_xml_parse_status(response, hnd->ilo2_hostport);
	if( ret != RIBCL_SUCCESS){
		err("ilo2_ribcl_set_uid_status: response parse failed.");
		free( response); 
		return( SA_ERR_HPI_INTERNAL_ERROR);
	}

	/* We're finished. Free up the temporary response buffer */
	free( response);
	return(SA_OK);
}

/**
 * ilo2_ribcl_get_power_saver_status:
 * @hnd: Private Handler data pointer.
 * @status: Location to store control's state.
 *
 * This routine send GET_HOST_POWER_SAVER_STATUS RIBCL command to iLO
 * and gets back the current Power Regulator status. This status then
 * gets mapped to HPI status before returning. 
 * Return values:
 *	SA_OK
 *	SA_ERR_HPI_OUT_OF_MEMORY
 *	SA_ERR_HPI_INTERNAL_ERROR
**/
static SaErrorT ilo2_ribcl_get_power_saver_status(ilo2_ribcl_handler_t *hnd,
	SaHpiCtrlStateDiscreteT *status)
{
	char *ps_cmd;
	char *response;	/* command response buffer */
	int ret;
	int ps_status = -1;

	/* Allocate a temporary response buffer. */
	response = malloc(ILO2_RIBCL_BUFFER_LEN);
	if( response == NULL){
		err("ilo2_ribcl_get_power_saver_status: Out of memory.");
		return(SA_ERR_HPI_OUT_OF_MEMORY);
	}

	/* Retrieve our customized command buffer */
	ps_cmd = hnd->ribcl_xml_cmd[IR_CMD_GET_HOST_POWER_SAVER_STATUS];
	if( ps_cmd == NULL){
		err("ilo2_ribcl_get_power_saver_status: null customized command.");
		free( response);
		return( SA_ERR_HPI_INTERNAL_ERROR);
	}

	/* Send the command to iLO2 and get the response. */
	ret = ilo2_ribcl_ssl_send_command( hnd, ps_cmd,
				response, ILO2_RIBCL_BUFFER_LEN);

	if( ret != 0){
		err("ilo2_ribcl_get_power_saver_status: command send failed.");
		free( response);
		return( SA_ERR_HPI_INTERNAL_ERROR);
	}

	/* Now, parse the response. */
	ret = ir_xml_parse_power_saver_status(response, &ps_status,
			hnd->ilo2_hostport);
	if( ret != RIBCL_SUCCESS) {
		err("ilo2_ribcl_get_power_saver_status: response parse failed.");
		free( response); 
		return( SA_ERR_HPI_INTERNAL_ERROR);
	}

	/* We're finished. Free up the temporary response buffer */
	free( response);

	if (ps_status >= ILO2_RIBCL_MANUAL_OS_CONTROL_MODE && 
		ps_status <= ILO2_RIBCL_MANUAL_HIGH_PERF_MODE) {
		*status = ps_status;
	} else {
		return( SA_ERR_HPI_INTERNAL_ERROR);
	}

	return(SA_OK);
}

/**
 * ilo2_ribcl_set_power_saver_status:
 * @hnd: Private Handler data pointer.
 * @status: Location to store control's state.
 *
 * This routine send SET_HOST_POWER_SAVER RIBCL command to iLO to change the 
 * current Power Saver status. This routine maps the passed in HPI status to
 * RIBCL status before sending the command.
 * Return values:
 *	SA_OK
 *	SA_ERR_HPI_OUT_OF_MEMORY
 *	SA_ERR_HPI_INTERNAL_ERROR
 *	SA_ERR_HPI_INVALID_PARAMS
**/
static SaErrorT ilo2_ribcl_set_power_saver_status(ilo2_ribcl_handler_t *hnd,
	SaHpiCtrlStateDiscreteT status)
{
	char *ps_cmd;
	char *response;	/* command response buffer */
	int ret;

	if((status < ILO2_RIBCL_MANUAL_OS_CONTROL_MODE) ||
		(status > ILO2_RIBCL_MANUAL_HIGH_PERF_MODE)) {
		return(SA_ERR_HPI_INVALID_PARAMS);
	}

	/* Allocate a temporary response buffer. */
	response = malloc(ILO2_RIBCL_BUFFER_LEN);
	if( response == NULL){
		err("ilo2_ribcl_set_power_saver_status: Out of memory.");
		return(SA_ERR_HPI_OUT_OF_MEMORY);
	}

	/* Retrieve our customized command buffer */
	if(status == ILO2_RIBCL_MANUAL_OS_CONTROL_MODE) {
		ps_cmd = hnd->ribcl_xml_cmd[IR_CMD_SET_HOST_POWER_SAVER_1];
	} else if(status == ILO2_RIBCL_MANUAL_LOW_POWER_MODE) {
		ps_cmd = hnd->ribcl_xml_cmd[IR_CMD_SET_HOST_POWER_SAVER_2];
	} else if(status == ILO2_RIBCL_AUTO_POWER_SAVE_MODE) {
		ps_cmd = hnd->ribcl_xml_cmd[IR_CMD_SET_HOST_POWER_SAVER_3];
	} else if(status == ILO2_RIBCL_MANUAL_HIGH_PERF_MODE) {
		ps_cmd = hnd->ribcl_xml_cmd[IR_CMD_SET_HOST_POWER_SAVER_4];
	} else {
		free( response);
		return(SA_ERR_HPI_INVALID_PARAMS);
	}

	if( ps_cmd == NULL){
		err("ilo2_ribcl_set_power_saver_status: null customized command.");
		free( response);
		return( SA_ERR_HPI_INTERNAL_ERROR);
	}

	/* Send the command to iLO2 and get the response. */
	ret = ilo2_ribcl_ssl_send_command( hnd, ps_cmd,
				response, ILO2_RIBCL_BUFFER_LEN);

	if( ret != 0){
		err("ilo2_ribcl_set_power_saver_status: command send failed.");
		free( response);
		return( SA_ERR_HPI_INTERNAL_ERROR);
	}

	/* Now, parse the response. */
	ret = ir_xml_parse_status(response, hnd->ilo2_hostport);
	if( ret != RIBCL_SUCCESS){
		err("ilo2_ribcl_set_power_saver_status: response parse failed.");
		free( response);
		/* DL365 G1 doesn't support Power Saver Feature and DL385 G2
		   supports just the ILO2_RIBCL_MANUAL_LOW_POWER_MODE */  
		if(ret == RIBCL_UNSUPPORTED) {
			return(SA_ERR_HPI_UNSUPPORTED_API);
		}
		return( SA_ERR_HPI_INTERNAL_ERROR);
	}

	/* We're finished. Free up the temporary response buffer */
	free( response);
	return(SA_OK);
}

/**
 * ilo2_ribcl_get_auto_power_status:
 * @hnd: Private Handler data pointer.
 * @status: Location to store control's state.
 *
 * This routine send GET_SERVER_AUTO_PWR RIBCL command to iLO
 * and gets back the current Auto Power status. This status then
 * gets mapped to HPI status before returning. 
 * Return values:
 *	SA_OK
 *	SA_ERR_HPI_OUT_OF_MEMORY
 *	SA_ERR_HPI_INTERNAL_ERROR
**/
static SaErrorT ilo2_ribcl_get_auto_power_status(ilo2_ribcl_handler_t *hnd,
	SaHpiCtrlStateDiscreteT *status)
{
	char *ps_cmd;
	char *response;	/* command response buffer */
	int ret;
	int ps_status = -1;

	/* Allocate a temporary response buffer. */
	response = malloc(ILO2_RIBCL_BUFFER_LEN);
	if( response == NULL){
		err("ilo2_ribcl_get_auto_power_status: Out of memory.");
		return(SA_ERR_HPI_OUT_OF_MEMORY);
	}

	/* Retrieve our customized command buffer */
	ps_cmd = hnd->ribcl_xml_cmd[IR_CMD_GET_SERVER_AUTO_PWR];
	if( ps_cmd == NULL){
		err("ilo2_ribcl_get_auto_power_status: null customized command.");
		free( response);
		return( SA_ERR_HPI_INTERNAL_ERROR);
	}

	/* Send the command to iLO2 and get the response. */
	ret = ilo2_ribcl_ssl_send_command( hnd, ps_cmd,
				response, ILO2_RIBCL_BUFFER_LEN);

	if( ret != 0){
		err("ilo2_ribcl_get_auto_power_status: command send failed.");
		free( response);
		return( SA_ERR_HPI_INTERNAL_ERROR);
	}

	/* Now, parse the response. */
	ret = ir_xml_parse_auto_power_status(response, &ps_status,
			hnd->ilo2_hostport);
	if( ret != RIBCL_SUCCESS) {
		err("ilo2_ribcl_get_auto_power_status: response parse failed.");
		free( response); 
		return( SA_ERR_HPI_INTERNAL_ERROR);
	}

	/* We're finished. Free up the temporary response buffer */
	free( response);

	if (ps_status >= ILO2_RIBCL_AUTO_POWER_ENABLED && 
		ps_status <= ILO2_RIBCL_AUTO_POWER_DELAY_60) {
		*status = ps_status;
	} else {
		return( SA_ERR_HPI_INTERNAL_ERROR);
	}

	return(SA_OK);
}

/**
 * ilo2_ribcl_set_auto_power_status:
 * @hnd: Private Handler data pointer.
 * @status: Location to store control's state.
 *
 * This routine send SERVER_AUTO_PWR RIBCL command to iLO to change the 
 * current Auto Power setting/status. This routine maps the passed in
 * HPI status to RIBCL status before sending the command.
 * Return values:
 *	SA_OK
 *	SA_ERR_HPI_OUT_OF_MEMORY
 *	SA_ERR_HPI_INTERNAL_ERROR
 *	SA_ERR_HPI_INVALID_PARAMS
**/
static SaErrorT ilo2_ribcl_set_auto_power_status(ilo2_ribcl_handler_t *hnd,
	SaHpiCtrlStateDiscreteT status)
{
	char *ps_cmd;
	char *response;	/* command response buffer */
	int ret;

	if((status != ILO2_RIBCL_AUTO_POWER_ENABLED) &&
		(status != ILO2_RIBCL_AUTO_POWER_DISABLED) &&
		(status != ILO2_RIBCL_AUTO_POWER_DELAY_RANDOM) &&
		(status != ILO2_RIBCL_AUTO_POWER_DELAY_15) &&
		(status != ILO2_RIBCL_AUTO_POWER_DELAY_30) &&
		(status != ILO2_RIBCL_AUTO_POWER_DELAY_45) &&
		(status != ILO2_RIBCL_AUTO_POWER_DELAY_60)) {
		return(SA_ERR_HPI_INVALID_PARAMS);
	}

	/* Allocate a temporary response buffer. */
	response = malloc(ILO2_RIBCL_BUFFER_LEN);
	if( response == NULL){
		err("ilo2_ribcl_set_auto_power_status: Out of memory.");
		return(SA_ERR_HPI_OUT_OF_MEMORY);
	}

	/* Retrieve our customized command buffer */
	if(status == ILO2_RIBCL_AUTO_POWER_ENABLED) {
		ps_cmd = hnd->ribcl_xml_cmd[IR_CMD_SERVER_AUTO_PWR_YES];
	} else if(status == ILO2_RIBCL_AUTO_POWER_DISABLED) {
		ps_cmd = hnd->ribcl_xml_cmd[IR_CMD_SERVER_AUTO_PWR_NO];
	} else if(status == ILO2_RIBCL_AUTO_POWER_DELAY_RANDOM) {
		ps_cmd = hnd->ribcl_xml_cmd[IR_CMD_SERVER_AUTO_PWR_RANDOM];
	} else if(status == ILO2_RIBCL_AUTO_POWER_DELAY_15) {
		ps_cmd = hnd->ribcl_xml_cmd[IR_CMD_SERVER_AUTO_PWR_15];
	} else if(status == ILO2_RIBCL_AUTO_POWER_DELAY_30) {
		ps_cmd = hnd->ribcl_xml_cmd[IR_CMD_SERVER_AUTO_PWR_30];
	} else if(status == ILO2_RIBCL_AUTO_POWER_DELAY_45) {
		ps_cmd = hnd->ribcl_xml_cmd[IR_CMD_SERVER_AUTO_PWR_45];
	} else if(status == ILO2_RIBCL_AUTO_POWER_DELAY_60) {
		ps_cmd = hnd->ribcl_xml_cmd[IR_CMD_SERVER_AUTO_PWR_60];
	} else {
		free( response);
		return(SA_ERR_HPI_INVALID_PARAMS);
	}

	if( ps_cmd == NULL){
		err("ilo2_ribcl_set_auto_power_status: null customized command.");
		free( response);
		return( SA_ERR_HPI_INTERNAL_ERROR);
	}

	/* Send the command to iLO2 and get the response. */
	ret = ilo2_ribcl_ssl_send_command( hnd, ps_cmd,
				response, ILO2_RIBCL_BUFFER_LEN);

	if( ret != 0){
		err("ilo2_ribcl_set_auto_power_status: command send failed.");
		free( response);
		return( SA_ERR_HPI_INTERNAL_ERROR);
	}

	/* Now, parse the response. */
	ret = ir_xml_parse_status(response, hnd->ilo2_hostport);
	if( ret != RIBCL_SUCCESS){
		err("ilo2_ribcl_set_auto_power_status: response parse failed.");
		free( response);
		return( SA_ERR_HPI_INTERNAL_ERROR);
	}

	/* We're finished. Free up the temporary response buffer */
	free( response);
	return(SA_OK);
}

/*****************************
	OpenHPI plug-in to iLO2 RIBCL plug-in ABI function mapping
*****************************/
void * oh_get_control_state (void *, SaHpiResourceIdT, SaHpiCtrlNumT,
		SaHpiCtrlModeT *, SaHpiCtrlStateT *)
	__attribute__ ((weak, alias("ilo2_ribcl_get_control_state")));

void * oh_set_control_state (void *, SaHpiResourceIdT,SaHpiCtrlNumT,
		SaHpiCtrlModeT, SaHpiCtrlStateT *)
	__attribute__ ((weak, alias("ilo2_ribcl_set_control_state")));
