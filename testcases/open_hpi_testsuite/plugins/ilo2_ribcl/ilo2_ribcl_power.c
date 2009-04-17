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
 * This source file contains Power HPI ABI routines iLO2 RIBCL plug-in
 * implements. Other source files provide support functionality for
 * these ABIs.
***************/

#include <ilo2_ribcl.h>
#include <ilo2_ribcl_ssl.h>
#include <ilo2_ribcl_xml.h>

/*****************************
	iLO2 RIBCL plug-in Power ABI Interface functions
*****************************/

/**
 * ilo2_ribcl_get_power_state:
 * @hnd: Handler data pointer.
 * @rid: Resource ID.
 * @act: Location to store resource's power state.
 *
 * Retrieves a resource's power state. If the resource has
 * SAHPI_CAPABILITY_POWER, then finds the current power state from
 * iLO2 by sending a GET_HOST_POWER_STATUS RIBCL command to it.
 *
 * Return values:
 * SA_OK - Normal case.
 * SA_ERR_HPI_CAPABILITY - Resource doesn't have SAHPI_CAPABILITY_POWER.
 * SA_ERR_HPI_INVALID_RESOURCE - Resource doesn't exist.
 * SA_ERR_HPI_INVALID_PARAMS - Pointer parameter(s) are NULL.
 **/
SaErrorT ilo2_ribcl_get_power_state(void *hnd,
				 SaHpiResourceIdT rid,
				 SaHpiPowerStateT *state)
{
        struct oh_handler_state *handle;
	ilo2_ribcl_handler_t *ilo2_ribcl_handler = NULL;
	SaHpiRptEntryT *rpt;
	char *grs_cmd;
	char *response;	/* command response buffer */
	int ret;
	int power_status = -1;
	ilo2_ribcl_resource_info_t *res_info = NULL;

	if (!hnd || !state) {
		err("ilo2_ribcl_get_power_state(): Invalid parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}

	handle = (struct oh_handler_state *)hnd;
	ilo2_ribcl_handler = (ilo2_ribcl_handler_t *)handle->data;
	
	if (!ilo2_ribcl_handler) {
		err("ilo2_ribcl_get_power_state(): Invalid parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}

	/* Check if resource exists and has power capabilities */
	rpt = oh_get_resource_by_id(handle->rptcache, rid);
        if (!rpt) {
		return(SA_ERR_HPI_INVALID_RESOURCE);
	}

	if (!(rpt->ResourceCapabilities & SAHPI_CAPABILITY_POWER)) {
		return(SA_ERR_HPI_CAPABILITY);
	} 
	
	res_info =  (ilo2_ribcl_resource_info_t *)oh_get_resource_data(
		handle->rptcache, rpt->ResourceId);
	if (!res_info) {
		err("ilo2_ribcl_get_power_state(): no resource info.");
		return(SA_ERR_HPI_INVALID_RESOURCE);
	}

	/* Allocate a temporary response buffer. */

	response = malloc(ILO2_RIBCL_BUFFER_LEN);
	if( response == NULL){
		err("ilo2_ribcl_get_power_state: failed to allocate resp buffer.");
		return( SA_ERR_HPI_OUT_OF_MEMORY);
	}

	/* Retrieve our customized command buffer */
	grs_cmd = ilo2_ribcl_handler->ribcl_xml_cmd[IR_CMD_GET_HOST_POWER_STATUS];
	if( grs_cmd == NULL){
		err("ilo2_ribcl_get_power_state: null customized command.");
		free( response);
		return( SA_ERR_HPI_INTERNAL_ERROR);
	}

	/* Send the command to iLO2 and get the response. */
	ret = ilo2_ribcl_ssl_send_command( ilo2_ribcl_handler, grs_cmd,
				response, ILO2_RIBCL_BUFFER_LEN);

	if( ret != 0){
		err("ilo2_ribcl_get_power_state: command send failed.");
		free( response);
		return( SA_ERR_HPI_INTERNAL_ERROR);
	}

	/* Now, parse the response.
	 */

	ret = ir_xml_parse_host_power_status(response, &power_status,
			ilo2_ribcl_handler->ilo2_hostport);
	if( ret != RIBCL_SUCCESS){
		err("ilo2_ribcl_get_power_state: response parse failed.");
		free( response); 
		return( SA_ERR_HPI_INTERNAL_ERROR);
	}

	/* We're finished. Free up the temporary response buffer */
	free( response);

	if(power_status == ILO2_RIBCL_POWER_ON) {
		*state = SAHPI_POWER_ON;
		res_info->power_cur_state = *state;
	} else if (power_status == ILO2_RIBCL_POWER_OFF) {
		*state = SAHPI_POWER_OFF;
		res_info->power_cur_state = *state;
	} else {
		return( SA_ERR_HPI_INTERNAL_ERROR);
	}

	return(SA_OK);
}

/**
 * ilo2_ribcl_set_power_state:
 * @hnd: Handler data pointer.
 * @rid: Resource ID.
 * @state: Resources's power state to set.
 *
 * Sets a resource's power state.
 * Retrieves a resource's power state. If the resource has
 * SAHPI_CAPABILITY_POWER, sends SET_HOST_POWER command to turn power on
 * or off based on the requtested power state. 
 *	If state == SAHPI_POWER_OFF, sends SET_HOST_POWER_OFF command to
 *	turn power off.
 *	If state == SAHPI_POWER_ON sends SET_HOST_POWER_ON command.
 *	If state == SAHPI_POWER_CYCLE sends SET_HOST_POWER_OFF followed
 *	by SET_HOST_POWER_ON command.
 *
 * Return values:
 * SA_OK - Normal case.
 * SA_ERR_HPI_CAPABILITY - Resource doesn't have SAHPI_CAPABILITY_POWER.
 * SA_ERR_HPI_INVALID_CMD - Resource doesn't support SAHPI_RESET_ASSERT.
 * SA_ERR_HPI_INVALID_RESOURCE - Resource doesn't exist.
 * SA_ERR_HPI_INVALID_PARAMS - Pointer parameter(s) are NULL; @state invalid.
 **/
SaErrorT ilo2_ribcl_set_power_state(void *hnd,
				 SaHpiResourceIdT rid,
				 SaHpiPowerStateT state)
{
        struct oh_handler_state *handle;
	ilo2_ribcl_handler_t *ilo2_ribcl_handler = NULL;
	SaHpiRptEntryT *rpt;
	char *sps_cmd;
	char *response;	/* command response buffer */
	int ret;
	ilo2_ribcl_resource_info_t *res_info = NULL;

	if (!hnd || NULL == oh_lookup_powerstate(state)){
		err("ilo2_ribcl_set_power_state(): Invalid parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}

	handle = (struct oh_handler_state *)hnd;
	ilo2_ribcl_handler = (ilo2_ribcl_handler_t *)handle->data;
	
	if (!ilo2_ribcl_handler) {
		err("ilo2_ribcl_set_power_state(): Invalid parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}

	/* Check if resource exists and has power capabilities */
	rpt = oh_get_resource_by_id(handle->rptcache, rid);
        if (!rpt) {
		return(SA_ERR_HPI_INVALID_RESOURCE);
	}
	
        if (!(rpt->ResourceCapabilities & SAHPI_CAPABILITY_POWER)) {
		return(SA_ERR_HPI_CAPABILITY);
	}

	res_info =  (ilo2_ribcl_resource_info_t *)oh_get_resource_data(
		handle->rptcache, rpt->ResourceId);
	if (!res_info) {
		err("ilo2_ribcl_get_power_state(): no resource info.");
		return(SA_ERR_HPI_INVALID_RESOURCE);
	}

	/* Note that we don't check our cached power state, since the resource
	 * power state could have been changed via a local power button. */ 

	/* Allocate a temporary response buffer. */

	response = malloc(ILO2_RIBCL_BUFFER_LEN);
	if( response == NULL){
		err("ilo2_ribcl_set_power_state: failed to allocate resp buffer.");
		return( SA_ERR_HPI_OUT_OF_MEMORY);
	}

	/* Retrieve our customized command buffer */
	if(state == SAHPI_POWER_OFF) {
		sps_cmd = ilo2_ribcl_handler->ribcl_xml_cmd[IR_CMD_SET_HOST_POWER_OFF];
	} else if(state == SAHPI_POWER_ON) {
		sps_cmd = ilo2_ribcl_handler->ribcl_xml_cmd[IR_CMD_SET_HOST_POWER_ON];
	} else if(state == SAHPI_POWER_CYCLE) {
		/* first send power off command */
		sps_cmd = ilo2_ribcl_handler->ribcl_xml_cmd[IR_CMD_SET_HOST_POWER_OFF];
	} else {
		/* we should never get here as oh_lookup_powerstate() should
		   have returned a null string back. */
		err("ilo2_ribcl_set_power_state(): Invalid parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}

	if( sps_cmd == NULL){
		err("ilo2_ribcl_set_power_state: null customized command.");
		free( response);
		return( SA_ERR_HPI_INTERNAL_ERROR);
	}

	/* Send the command to iLO2 and get the response. */
	ret = ilo2_ribcl_ssl_send_command( ilo2_ribcl_handler, sps_cmd,
				response, ILO2_RIBCL_BUFFER_LEN);

	if( ret != 0){
		err("ilo2_ribcl_set_power_state: command send failed.");
		free( response);
		return( SA_ERR_HPI_INTERNAL_ERROR);
	}

	/* Now, parse the response. */

	ret = ir_xml_parse_set_host_power(response,
			ilo2_ribcl_handler->ilo2_hostport);

	if(ret == -1) {
		err("ilo2_ribcl_set_power_state: iLO2 returned error.");
		free( response);
		return( SA_ERR_HPI_INTERNAL_ERROR);
	}

	/* If the requested state is SAHPI_POWER_CYCLE, turn the power on */
	if(state == SAHPI_POWER_CYCLE) {
		SaHpiPowerStateT temp_state;
		int polls;

		/* First, wait for the power to go off. An orderly shutdown
		 * is being performed, so we might have to wait for
		 * an OS running on that resource to fully shut down.
		 * Unfortunately, the iLo2 will not queue the power commands.
		 * If we send a "power on" command before the system actually
		 * powers off, the "power on" command will be dropped, as the
		 * iLo2 will detect that the system is already (still) on. */
	
		temp_state = SAHPI_POWER_ON;
		for( polls=0; polls < ILO2_MAX_POWER_POLLS; polls++){

			dbg("Obtaining current power state from iLo2 at %s, try %d",
				ilo2_ribcl_handler->ilo2_hostport, polls);
			ilo2_ribcl_get_power_state(hnd, rid, &temp_state);
			if(temp_state == SAHPI_POWER_OFF){
				break;
			}

			/* iLo2 commands take around 10 seconds round trip,
			 * so sleep for a while before retrying. */
			sleep( ILO2_POWER_POLL_SLEEP_SECONDS);

		} /* end for polls */

		if( polls == ILO2_MAX_POWER_POLLS){
			err("Maximum tries exceeded ( %d) checking power off for system at address %s",
		 ILO2_MAX_POWER_POLLS, ilo2_ribcl_handler->ilo2_hostport);
			return( SA_ERR_HPI_INTERNAL_ERROR);
		}
	
		/* Power is off now. update res_info power status */
		res_info->power_cur_state = SAHPI_POWER_OFF;
	
		/* Now, try sending the power on command */

		sps_cmd = ilo2_ribcl_handler->ribcl_xml_cmd[IR_CMD_SET_HOST_POWER_ON];
		if( sps_cmd == NULL){
			err("ilo2_ribcl_set_power_state: null customized command.");
			free( response);
			return( SA_ERR_HPI_INTERNAL_ERROR);
		}

		/* Send the command to iLO2 and get the response. */
		ret = ilo2_ribcl_ssl_send_command( ilo2_ribcl_handler, sps_cmd,
					response, ILO2_RIBCL_BUFFER_LEN);

		if( ret != 0){
			err("ilo2_ribcl_set_power_state: command send failed.");
			free( response);
			return( SA_ERR_HPI_INTERNAL_ERROR);
		}

		/* Now, parse the response. */

		ret = ir_xml_parse_set_host_power(response,
				ilo2_ribcl_handler->ilo2_hostport);

		free( response);

		if(ret == -1) {
			err("ilo2_ribcl_set_power_state: iLO2 returned error.");
			return( SA_ERR_HPI_INTERNAL_ERROR);
		}
		/* Power is on, update res_info power status */
		res_info->power_cur_state = SAHPI_POWER_ON;
	} else {
		/* Save current value in res_info */
		res_info->power_cur_state = state;
	}
        return(SA_OK);
}

/*****************************
	OpenHPI plug-in to iLO2 RIBCL plug-in ABI function mapping
*****************************/
void * oh_get_power_state (void *, SaHpiResourceIdT, SaHpiResetActionT *)
                __attribute__ ((weak, alias("ilo2_ribcl_get_power_state")));

void * oh_set_power_state (void *, SaHpiResourceIdT, SaHpiResetActionT)
                __attribute__ ((weak, alias("ilo2_ribcl_set_power_state")));

