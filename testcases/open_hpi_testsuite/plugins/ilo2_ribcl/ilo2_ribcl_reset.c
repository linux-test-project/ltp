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
 * This source file contains Reset HPI ABI routines iLO2 RIBCL plug-in
 * implements. Other source files provide support functionality for
 * these ABIs.
***************/

#include <ilo2_ribcl.h>
#include <ilo2_ribcl_ssl.h>
#include <ilo2_ribcl_xml.h>


/*****************************
	iLO2 RIBCL plug-in Reset ABI Interface functions
*****************************/

/**
 * ilo2_ribcl_get_reset_state:
 * @hnd: Handler data pointer.
 * @rid: Resource ID.
 * @act: Location to store resource's reset action state.
 *
 * Retrieves a resource's reset action state. If the resource has
 * SAHPI_CAPABILITY_RESET, then returns SAHPI_RESET_DEASSERT as
 * ProLiant Rack Mount Server doesn't support hold in reset.
 * ProLiant Rack Mount Server doesn't support pulsed reset. Devices
 * that supprt pulsed reset can be held in reset for a specified 
 * perid of time using SAHPI_RESET_ASSERT followed by a
 * SAHPI_RESET_DEASSERT.
 *
 * Return values:
 * SA_OK - Normal case.
 * SA_ERR_HPI_CAPABILITY - Resource doesn't have SAHPI_CAPABILITY_RESET.
 * SA_ERR_HPI_INVALID_RESOURCE - Resource doesn't exist.
 * SA_ERR_HPI_INVALID_PARAMS - Pointer parameter(s) are NULL.
 **/
SaErrorT ilo2_ribcl_get_reset_state(void *hnd,
				 SaHpiResourceIdT rid,
				 SaHpiResetActionT *act)
{
        struct oh_handler_state *handle;
	ilo2_ribcl_handler_t *ilo2_ribcl_handler = NULL;
	SaHpiRptEntryT *rpt;

	if (!hnd || !act) {
		err("ilo2_ribcl_get_reset_state(): Invalid parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}

	handle = (struct oh_handler_state *)hnd;
	ilo2_ribcl_handler = (ilo2_ribcl_handler_t *)handle->data;
	
	if (!ilo2_ribcl_handler) {
		err("ilo2_ribcl_get_reset_state(): Invalid parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}

	/* Check if resource exists and has reset capabilities */
	rpt = oh_get_resource_by_id(handle->rptcache, rid);
        if (!rpt) {
		return(SA_ERR_HPI_INVALID_RESOURCE);
	} 
	
        if (!(rpt->ResourceCapabilities & SAHPI_CAPABILITY_RESET)) {
		return(SA_ERR_HPI_CAPABILITY);
	}

	/* ProLiant Rack Mount Server doesn't support holding in reset
	   Return SAHPI_RESET_DEASSERT */

	*act = SAHPI_RESET_DEASSERT;

	return(SA_OK);
}

/**
 * ilo2_ribcl_set_reset_state:
 * @hnd: Handler data pointer.
 * @rid: Resource ID.
 * @act: Reset action state to set.
 *
 * Sets a resource's reset action state.
 * Retrieves a resource's reset action state. If the resource has
 * SAHPI_CAPABILITY_RESET then sends RESET_SERVER RIBCL command to iLO2
 * to do a warm reset the system and a COLD_BOOT_SERVER commad to do a
 * cold reset. Please note that this command doesn't bring the system and
 * (OS running on it down gracefully.
 *
 * Return values:
 * SA_OK - Normal case.
 * SA_ERR_HPI_CAPABILITY - Resource doesn't have SAHPI_CAPABILITY_RESET.
 * SA_ERR_HPI_INVALID_CMD - Resource doesn't support SAHPI_RESET_ASSERT.
 * SA_ERR_HPI_INVALID_CMD - Resource doesn't support SAHPI_RESET_DEASSERT.
 * SA_ERR_HPI_INVALID_RESOURCE - Resource doesn't exist.
 * SA_ERR_HPI_INVALID_PARAMS - Pointer parameter(s) are NULL; @act invalid.
 **/
SaErrorT ilo2_ribcl_set_reset_state(void *hnd,
				 SaHpiResourceIdT rid,
				 SaHpiResetActionT act)
{
        struct oh_handler_state *handle;
	ilo2_ribcl_handler_t *ilo2_ribcl_handler = NULL;
	SaHpiRptEntryT *rpt;
	char *srs_cmd;
	char *response;	/* command response buffer */
	int ret;

	if (!hnd || NULL == oh_lookup_resetaction(act)){
		err("ilo2_ribcl_set_reset_state(): Invalid parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}

	/* 
	   ProLiant Rack Mount Server doesn't support pulsed reset. Devices
	   that supprt pulsed reset can be held in reset for a specified 
	   perid of time using SAHPI_RESET_ASSERT followed by a
	   SAHPI_RESET_DEASSERT.
	   Both cold and warm reset actions initiare an ungraceful shutdown
	   and will bring the server down even without notifying the OS.
	*/
	if (act == SAHPI_RESET_ASSERT || act == SAHPI_RESET_DEASSERT) {
		return(SA_ERR_HPI_INVALID_CMD);
	}	

	if (act != SAHPI_COLD_RESET && act != SAHPI_WARM_RESET) {
		return(SA_ERR_HPI_INVALID_CMD);
	}

	handle = (struct oh_handler_state *)hnd;
	ilo2_ribcl_handler = (ilo2_ribcl_handler_t *)handle->data;
	
	if (!ilo2_ribcl_handler) {
		err("ilo2_ribcl_set_reset_state(): Invalid parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}

	/* Check if resource exists and has reset capabilities */
	rpt = oh_get_resource_by_id(handle->rptcache, rid);
        if (!rpt) {
		return(SA_ERR_HPI_INVALID_RESOURCE);
	}
	
        if (!(rpt->ResourceCapabilities & SAHPI_CAPABILITY_RESET)) {
		return(SA_ERR_HPI_CAPABILITY);
	}

	/* Allocate a temporary response buffer. */

	response = malloc(ILO2_RIBCL_BUFFER_LEN);
	if( response == NULL){
		err("ilo2_ribcl_set_reset_state: failed to allocate resp buffer.");
		return( SA_ERR_HPI_OUT_OF_MEMORY);
	}

	/* Retrieve our customized command buffer */
	if(act == SAHPI_COLD_RESET) {
		srs_cmd = ilo2_ribcl_handler->ribcl_xml_cmd[
			IR_CMD_COLD_BOOT_SERVER];
	} else {
		srs_cmd = ilo2_ribcl_handler->ribcl_xml_cmd
			[IR_CMD_RESET_SERVER];
	}

	if( srs_cmd == NULL){
		err("ilo2_ribcl_set_reset_state: null customized command.");
		free( response);
		return( SA_ERR_HPI_INTERNAL_ERROR);
	}

	/* Send the command to iLO2 and get the response. */
	ret = ilo2_ribcl_ssl_send_command( ilo2_ribcl_handler, srs_cmd,
				response, ILO2_RIBCL_BUFFER_LEN);

	if( ret != 0){
		err("ilo2_ribcl_set_reset_state: command send failed.");
		free( response);
		return( SA_ERR_HPI_INTERNAL_ERROR);
	}

	/* Now, parse the response.
	 */

	ret = ir_xml_parse_reset_server(response,
			ilo2_ribcl_handler->ilo2_hostport);

	free( response);

	if(ret == -1) {
		err("ilo2_ribcl_set_reset_state: iLO2 returned error.");
		return( SA_ERR_HPI_INTERNAL_ERROR);
	}

        return(SA_OK);
}

/*****************************
	OpenHPI plug-in to iLO2 RIBCL plug-in ABI function mapping
*****************************/
void * oh_get_reset_state (void *, SaHpiResourceIdT, SaHpiResetActionT *)
                __attribute__ ((weak, alias("ilo2_ribcl_get_reset_state")));

void * oh_set_reset_state (void *, SaHpiResourceIdT, SaHpiResetActionT)
                __attribute__ ((weak, alias("ilo2_ribcl_set_reset_state")));

