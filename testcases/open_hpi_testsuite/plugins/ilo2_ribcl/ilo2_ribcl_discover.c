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

#include <ilo2_ribcl.h>
#include <ilo2_ribcl_discover.h>
#include <ilo2_ribcl_ssl.h>
#include <ilo2_ribcl_xml.h>
#include <ilo2_ribcl_sensor.h>
#include <ilo2_ribcl_idr.h>
#ifdef ILO2_RIBCL_SIMULATE_iLO2_RESPONSE
#include <sys/stat.h>   /* For test routine ilo2_ribcl_getfile() */
#include <fcntl.h>      /* For test routine ilo2_ribcl_getfile() */
#endif /* ILO2_RIBCL_SIMULATE_iLO2_RESPONSE */

/* Foreward decls: */

static SaErrorT ilo2_ribcl_do_discovery( ilo2_ribcl_handler_t *ir_handler);
static SaErrorT ilo2_ribcl_discover_chassis( struct oh_handler_state *,
	SaHpiEntityPathT *);
static SaErrorT ilo2_ribcl_discover_cpu( struct oh_handler_state *,
	SaHpiEntityPathT *);
static SaErrorT ilo2_ribcl_discover_memory( struct oh_handler_state *,
	SaHpiEntityPathT *);
static SaErrorT ilo2_ribcl_discover_powersupplies( struct oh_handler_state *,
	SaHpiEntityPathT *);
static SaErrorT ilo2_ribcl_discover_vrm( struct oh_handler_state *,
	SaHpiEntityPathT *);
static SaErrorT ilo2_ribcl_discover_fans( struct oh_handler_state *,
	SaHpiEntityPathT *);
static SaErrorT ilo2_ribcl_discovered_fru( struct oh_handler_state *,
	SaHpiEntityPathT *, enum ir_discoverstate *, int, char *,
	struct ilo2_ribcl_idr_info *);
static SaErrorT ilo2_ribcl_resource_set_failstatus( struct oh_handler_state *,
	 SaHpiEntityPathT *, SaHpiBoolT);
static SaErrorT ilo2_ribcl_undiscovered_fru( struct oh_handler_state *,
	SaHpiEntityPathT *, enum ir_discoverstate *, int, char *);
static void ilo2_ribcl_clear_discoveryflags( ilo2_ribcl_handler_t *);
static SaErrorT ilo2_ribcl_controls(struct oh_handler_state *, int,
	struct oh_event *, char *);
static SaErrorT ilo2_ribcl_add_severity_sensor( struct oh_handler_state *,
	struct oh_event *, int, SaHpiSensorTypeT, SaHpiEventStateT,
	struct ilo2_ribcl_sensinfo *, char *);
static void ilo2_ribcl_discover_chassis_sensors( struct oh_handler_state *, 
	struct oh_event *);
void ilo2_ribcl_add_resource_capability( struct oh_handler_state *, 
	struct oh_event *, SaHpiCapabilitiesT);

#ifdef ILO2_RIBCL_SIMULATE_iLO2_RESPONSE
static int ilo2_ribcl_getfile( char *, char *, int);
#endif /* ILO2_RIBCL_SIMULATE_iLO2_RESPONSE */



/**
 * ilo2_ribcl_discover_resources: Discovers resources for this instance 
 * of the iLO2 RIBCL plug-in 
 * @handler:  Handler data pointer.
 *
 * This function discovers resources for this instance of the iLO2 
 * RIBCL plugin.
 * Detailed description:
 *
 *	- Reset the discovery flags for each resource in DiscoveryData
 *	  via a call to ilo2_ribcl_clear_discoveryflags(). This allows us
 *	  to detect removed resources.
 *	- Call ilo2_ribcl_do_discovery(), which will open an SSL connection
 *	  to iLO2 using the hostname and port saved in the ilo2_ribcl_handler
 * 	  send the ILO2_RIBCL_GET_SERVER_DATA xml commands, and parse the
 *	  results into information stored in the DiscoveryData structure
 *	  within our private handler.
 *	- Finally, call a series of discovery routines that examine
 *	  DiscoveryData and generate rpt entries and events for
 *	  resources that have been added, removed, or failed.
 *
 * Return values:
 * Builds/updates internal RPT cache - normal operation.
 * Returns SA_OK for success
 * SA_ERR_HPI_OUT_OF_MEMORY - Cannot allocate space for internal memory
 * SA_ERR_HPI_INTERNAL_ERROR - SSL and XML errors
 * SA_ERR_HPI_INVALID_PARAMS - if plungin handler or private handler is null
 *
 **/
SaErrorT ilo2_ribcl_discover_resources(void *handler)
{
	struct oh_handler_state *oh_handler =
		(struct oh_handler_state *) handler;
        ilo2_ribcl_handler_t *ilo2_ribcl_handler;
	SaErrorT  ret;
	SaHpiEntityPathT ep_root;
	
	if (!handler) {
		err("ilo2_ribcl_discover_resources(): Invalid handler parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}

        ilo2_ribcl_handler = (ilo2_ribcl_handler_t *) oh_handler->data;
	if(! ilo2_ribcl_handler) {
		err("ilo2_ribcl_discover_resources(): Invalid private handler parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}

	if( ilo2_ribcl_handler->entity_root == NULL){
		err("ilo2_ribcl_discover_resources(): entity_root is NULL.");
		return(SA_ERR_HPI_INTERNAL_ERROR);
	}

	/* If our plugin becomes multhithreaded, lock here? */

	ret = oh_encode_entitypath(ilo2_ribcl_handler->entity_root, &ep_root);
	if( ret != SA_OK){
		err("ilo2_ribcl_discover_resources(): Cannot convert entity path.");
		return(SA_ERR_HPI_INTERNAL_ERROR);
	}

	ilo2_ribcl_clear_discoveryflags( ilo2_ribcl_handler);

	/* Send the RIBCL commands to iLO2 for discovery. The results
	 * will have been written to DiscoveryData in the private handler
	 * when ilo2_ribcl_do_discovery() returns.
	 */

	ret = ilo2_ribcl_do_discovery( ilo2_ribcl_handler);
	if( ret != SA_OK){
		err("ilo2_ribcl_discover_resources(): discovery failed.");
		return( ret);
	}

	/* Now, we examine DiscoveryData and create resource events.
	   Creating events is done in other functions called from this
	   routine
	 */

	/* set first_discovery_done. */ 
	if( !ilo2_ribcl_handler->first_discovery_done){

		/* We only need to create a resource for the chassis once */
		ret = ilo2_ribcl_discover_chassis( oh_handler,
			&ep_root);
		if( ret != SA_OK){
			err("ilo2_ribcl_discover_resources(): chassis discovery failed.");
			return( ret);
		}
		ilo2_ribcl_handler->first_discovery_done = 1;

	} else {
		/* Even though the chassis itself is not an FRU, its
		 * firmware can be updated, which can change its inventory
		 * data. So, check for inventory updates. */

		ilo2_ribcl_update_chassis_idr( oh_handler, &ep_root);
	}

	ret = ilo2_ribcl_discover_cpu( oh_handler, &ep_root);
	if( ret != SA_OK){
		err("ilo2_ribcl_discover_resources(): cpu discovery failed.");
		return( ret);
	}

	ret = ilo2_ribcl_discover_memory( oh_handler, &ep_root);
	if( ret != SA_OK){
		err("ilo2_ribcl_discover_resources(): memory discovery failed.");
		return( ret);
	}

	ret = ilo2_ribcl_discover_powersupplies( oh_handler, &ep_root);
	if( ret != SA_OK){
		err("ilo2_ribcl_discover_resources(): power supply discovery failed.");
		return( ret);
	}

	ret = ilo2_ribcl_discover_vrm( oh_handler, &ep_root);
	if( ret != SA_OK){
		err("ilo2_ribcl_discover_resources(): VRM discovery failed.");
		return( ret);
	}

	ret = ilo2_ribcl_discover_fans( oh_handler, &ep_root);
	if( ret != SA_OK){
		err("ilo2_ribcl_discover_resources(): fan discovery failed.");
		return( ret);
	}

	/* Since we do not have a polling thread for sensors (yet), we process
	 * any changed sensor values as part of a discovery operation, since
	 * the openhpid should be regularly calling us.
	 */
	ilo2_ribcl_process_sensors( oh_handler);

	return(SA_OK);
}



/**
 * ilo2_ribcl_do_discovery
 * @ir_handler: The private handler for this plugin instance.
 *
 *	This routine sends the ILO2_RIBCL_GET_SERVER_DATA xml commands 
 *	to an iLO2 via a SSL connection, receive the resulting output
 *	from iLO2, parses that output, and adds information about discovered
 *	resources into the private handler's DiscoveryData structure.
 *
 * Detailed description:
 *
 *	- Allocate a temporary buffer to receive the response from iLO2.
 *	- Retrieve the xml code for the ILO2_RIBCL_GET_SERVER_DATA commands
 *	  that have already been customized with the login and password of
 *	  the target iLO2.
 *	- Call ilo2_ribcl_ssl_send_command() to make a SSL connection with
 *	  the iLO2 addressed in the configuration file, send the xml commands,
 *	  and retrieve the response in our temporary buffer.
 *	- Call ir_xml_parse_discoveryinfo() to parse the iLO2 response and
 *	  enter data about the resources into the DiscoveryData structure
 *	  located in out private handler.
 *	- Free the temporary response buffer.
 *
 * Return values:
 *	SA_OK for success.
 *	SA_ERR_HPI_OUT_OF_MEMORY if allocation fails
 *	SA_ERR_HPI_INTERNAL_ERROR if our customized command is null or
 *		if response parsing fails.
 *
 **/
static SaErrorT ilo2_ribcl_do_discovery( ilo2_ribcl_handler_t *ir_handler)
{

	char *discover_cmd;
	char *d_response;	/* command response buffer */
	int ret;

	/* Allocate a temporary response buffer. Since the discover
	 * command should be infrequently run, we dynamically allocate
	 * instead of keeping a buffer around all the time.
	 */

	d_response = malloc(ILO2_RIBCL_DISCOVER_RESP_MAX);
	if( d_response == NULL){
		err("ilo2_ribcl_do_discovery(): failed to allocate resp buffer.");
		return( SA_ERR_HPI_OUT_OF_MEMORY);
	}

	/* Retrieve our customized command buffer */
	discover_cmd = ir_handler->ribcl_xml_cmd[IR_CMD_GET_SERVER_DATA];
	if( discover_cmd == NULL){
		err("ilo2_ribcl_do_discovery(): null customized command.");
		free( d_response);
		return( SA_ERR_HPI_INTERNAL_ERROR);
	}

#ifdef ILO2_RIBCL_SIMULATE_iLO2_RESPONSE

	if( ir_handler->discovery_responsefile){

		/* If we have specified a discovery_responsefile in our
		 * config file, read the iLO2 response from this file
		 * rather that communicating with an actual iLO2. This is
		 * used for automated testing of this plugin.
		 */
		err("ilo2_ribcl_do_discovery(): Using contents of %s instead of iLO2 communication.",
			ir_handler->discovery_responsefile);
 
		ret = ilo2_ribcl_getfile( ir_handler->discovery_responsefile,
			d_response, ILO2_RIBCL_DISCOVER_RESP_MAX);
	} else {

		/* Send the command to iLO2 and get the response. */
		ret = ilo2_ribcl_ssl_send_command( ir_handler, discover_cmd,
				d_response, ILO2_RIBCL_DISCOVER_RESP_MAX);
	}

#else /* ILO2_RIBCL_SIMULATE_iLO2_RESPONSE not defined. This the the
	 normal use, non-testing case. */

	/* Send the command to iLO2 and get the response. */
	ret = ilo2_ribcl_ssl_send_command( ir_handler, discover_cmd,
				d_response, ILO2_RIBCL_DISCOVER_RESP_MAX);

#endif /* ILO2_RIBCL_SIMULATE_iLO2_RESPONSE */

	if( ret != 0){
		err("ilo2_ribcl_do_discovery(): command send failed.");
		free( d_response);
		return( SA_ERR_HPI_INTERNAL_ERROR);
	}

	/* Now, parse the response. The information we extract will be
	 * written into the DiscoveryData element in our private handler
	 */

	ret = ir_xml_parse_discoveryinfo( ir_handler, d_response);
	if( ret != RIBCL_SUCCESS){
		err("ilo2_ribcl_do_discovery(): response parse failed.");
		free( d_response); 
		return( SA_ERR_HPI_INTERNAL_ERROR);
	}

	/* We're finished. Free up the temporary response buffer */
	free( d_response);
	return( SA_OK);
	
} /* end ilo2_ribcl_do_discovery() */



/**
 * ilo2_ribcl_clear_discoveryflags
 * @ir_handler: Pointer to the plugin private handler. 
 *
 * This routine clears the IR_DISCOVERED bit in the flags element
 * for all discoverable resources that can be removed. 
 *
 * Return values: None
 **/
void ilo2_ribcl_clear_discoveryflags( ilo2_ribcl_handler_t *ir_handler){

	ilo2_ribcl_DiscoveryData_t *ddata;
	int idex;

	ddata = & (ir_handler->DiscoveryData);

	/* Clear the CPU flags */
	for( idex = 1; idex <= ILO2_RIBCL_DISCOVER_CPU_MAX; idex++){
		ddata->cpudata[idex].cpuflags &= ~IR_DISCOVERED;
	}

	/* Clear the memory flags */
	for( idex = 1; idex <= ILO2_RIBCL_DISCOVER_MEM_MAX; idex++){
		ddata->memdata[idex].memflags &= ~IR_DISCOVERED;
	}

	/* Clear the fan flags */
	for( idex = 1; idex <= ILO2_RIBCL_DISCOVER_FAN_MAX; idex++){
		ddata->fandata[idex].fanflags &= ~IR_DISCOVERED;
	}

	/* Clear the power supply flags */
	for( idex = 1; idex <= ILO2_RIBCL_DISCOVER_PSU_MAX; idex++){
		ddata->psudata[idex].psuflags &= ~IR_DISCOVERED;
	}

	/* Clear the VRM flags */
	for( idex = 1; idex <= ILO2_RIBCL_DISCOVER_PSU_MAX; idex++){
		ddata->vrmdata[idex].vrmflags &= ~IR_DISCOVERED;
	}

} /* end ilo2_ribcl_clear_discoveryflags() */



/**
 * ilo2_ribcl_discover_chassis
 * @oh_handler:  Handler data pointer.
 *
 * This function creates the rpt entry for our root resource, which
 * is the rack-mount server itself. After adding the resource to the handler's
 * rpt cache, we send a SAHPI_RESE_RESOURCE_ADDED event.
 *
 * Detailed description:
 *
 *	- First, we build a resource tag from the product name, the
 *	  serial number, and the EntityLocation.
 *	- Then we allocate a new event, with the resource entry specifying
 *	  ResourceCapabilities of SAHPI_CAPABILITY_RESOURCE and 
 *	  SAHPI_CAPABILITY_RESET, no HotSwapCapabilities and a
 *	  ResourceSeverity of SAHPI_CRITICAL.
 *	- Call oh_add_resource() to add the resource to the handler's rpt
 *	  cache.
 *	- Call oh_evt_queue_push() to send a SAHPI_RESE_RESOURCE_ADDED
 *	  resource event. 
 *
 * Return values:
 *	SA_OK if success
 *	SA_ERR_HPI_OUT_OF_MEMORY if allocation fails
 *
 **/
static SaErrorT ilo2_ribcl_discover_chassis(
				struct oh_handler_state *oh_handler, 
				SaHpiEntityPathT *ep_root)
{
	struct oh_event *ev = NULL;
	ilo2_ribcl_handler_t *ir_handler = NULL;
	ilo2_ribcl_resource_info_t *res_info = NULL;
	char *tmptag = NULL;
	size_t tagsize;
	SaErrorT  ret;

        ir_handler = (ilo2_ribcl_handler_t *) oh_handler->data;

	if( (ir_handler->DiscoveryData.product_name) &&
	    (ir_handler->DiscoveryData.serial_number) ){

		/* We build the tag from the product name, serial number, and
	 	 * EntityLocation. Add 6 additional bytes for " SN: " notation
		 */

		tagsize = strlen( ir_handler->DiscoveryData.product_name) +
			strlen( ir_handler->DiscoveryData.serial_number) +
			OH_MAX_LOCATION_DIGITS + 8;
		tmptag = malloc( tagsize);
		if( tmptag == NULL){
			err("ilo2_ribcl_discover_chassis(): tag message allocation failed.");
			return( SA_ERR_HPI_OUT_OF_MEMORY);
		}
		snprintf( tmptag, tagsize, "%s SN:%s (%2d)",
			ir_handler->DiscoveryData.product_name,
			ir_handler->DiscoveryData.serial_number,   
			ep_root->Entry[0].EntityLocation);
	
	} else {
		/* If we didn't get a product name and a serial number,
		 * use the default of "HP Rackmount Server".
		 */
		tagsize = OH_MAX_LOCATION_DIGITS + 19 + 3;
		/* String plus " ()" */ 
		
		tmptag = malloc( tagsize);
		if( tmptag == NULL){
			err("ilo2_ribcl_discover_chassis(): tag message allocation failed.");
			return( SA_ERR_HPI_OUT_OF_MEMORY);
		}

		snprintf( tmptag, tagsize, "%s (%2d)",
		"HP Rackmount Server",
		ep_root->Entry[0].EntityLocation);
	}

	/* Create a resource event for the box itself */
	ev = oh_new_event();
	if( ev == NULL){
		err("ilo2_ribcl_discover_chassis(): event allocation failed.");
		free( tmptag);
		return( SA_ERR_HPI_OUT_OF_MEMORY);
	}

	/* Fill in the resource part of the event */

	/* Plugin doesn't set EntryID */
	ev->resource.ResourceEntity = *ep_root; 
	ev->resource.ResourceId =
		oh_uid_from_entity_path(&(ev->resource.ResourceEntity));
	ev->resource.ResourceInfo.ManufacturerId = HP_MANUFACTURING_ID;
	ev->resource.ResourceInfo.FirmwareMajorRev = 
		ir_handler->DiscoveryData.fwdata.FirmwareMajorRev;
	ev->resource.ResourceInfo.FirmwareMinorRev = 
		ir_handler->DiscoveryData.fwdata.FirmwareMinorRev;
	ev->resource.ResourceCapabilities = 
		(SAHPI_CAPABILITY_RESOURCE | SAHPI_CAPABILITY_RESET 
			| SAHPI_CAPABILITY_POWER | SAHPI_CAPABILITY_RDR); 
	ev->resource.HotSwapCapabilities = ILO2_RIBCL_MANAGED_HOTSWAP_CAP_FALSE;
	ev->resource.ResourceSeverity = SAHPI_CRITICAL;
	ev->resource.ResourceFailed = SAHPI_FALSE;

	oh_init_textbuffer(&(ev->resource.ResourceTag));
	oh_append_textbuffer( &(ev->resource.ResourceTag), tmptag);

	/* Allocate and populate iLO2 RIBCL private data area to be added
	   to the resource rpt cache */
	res_info = (ilo2_ribcl_resource_info_t *)
		g_malloc0(sizeof(ilo2_ribcl_resource_info_t));
	if(res_info == NULL) {
		err("ilo2_ribcl_discover_chassis(): out of memory");
		oh_event_free( ev, 0);
		free( tmptag);
		return( SA_ERR_HPI_OUT_OF_MEMORY);
	}
	res_info->rid = ev->resource.ResourceId;
	/* Safe to mark it active. This state is not applicable to 
	   non-FRU resources and will not be used as 
	   SAHPI_CAPABILITY_FRU is not set for this resource. */
	res_info->fru_cur_state = SAHPI_HS_STATE_ACTIVE;
	res_info->disc_data_idx = ILO2_RIBCL_CHASSIS_INDEX;
	res_info->power_cur_state = ILO2_RIBCL_POWER_STATUS_UNKNOWN;
	
	/* Add the resource to this instance's rpt cache */
	ret = oh_add_resource( oh_handler->rptcache, &(ev->resource),
			res_info, 0);
	if( ret != SA_OK){
		err("ilo2_ribcl_discover_chassis(): cannot add resource to rptcache.");
		oh_event_free( ev, 0);
		free( tmptag);
		return( ret);
	}

	/*
	 * Add control RDRs. ProLiant Rack Mounts have allow Unit ID (UID) 
	 * Light on the server to be turned off or on. There are power 
	 * saver, and Server Auto Power controls avalable as well. Add one
	 * RDR each for UID, Power Saver conrol, and Auto Power Control.
	*/
	if(ilo2_ribcl_controls(oh_handler, ILO2_RIBCL_CTL_UID, ev,
		"Unit Identification Light (UID) Values: On(1)/Off(0)") == SA_OK) {
		/* UID control RDR has beeen added successfully. Set RDR
		   and CONTROL capability flags. Please note that failing
		   to add control rdr is not treated as a critical error
		   that requires to fail plug-in initilization */

		ilo2_ribcl_add_resource_capability( oh_handler, ev,
			     (SAHPI_CAPABILITY_RDR | SAHPI_CAPABILITY_CONTROL));
		
	} else {
		err("iLO2 RIBCL: Failed to setup UID Control RDR. Plug-in will run without this control feature");
	}

	if(ilo2_ribcl_controls(oh_handler, ILO2_RIBCL_CTL_POWER_SAVER, ev,
		"Power Regulator Control Power Modes: Disabled(1)/Low(2)/DynamicSavings(3)/High(4)") == SA_OK) {
		/* Power Saver control RDR has beeen added successfully.
		   Set RDR and CONTROL capability flags. Please note that
		   failing to add control rdr is not treated as a critical
		   error that requires to fail plug-in initilization */

		ilo2_ribcl_add_resource_capability( oh_handler, ev,
			     (SAHPI_CAPABILITY_RDR | SAHPI_CAPABILITY_CONTROL));
		
	} else {
		err("iLO2 RIBCL: Failed to setup Power Saver Control RDR. Plug-in will run without this control feature");
	}

	if(ilo2_ribcl_controls(oh_handler, ILO2_RIBCL_CTL_AUTO_POWER, ev,
		"Auto Power Control Delay:Min.(1)/Disabled(2)/random (3)/15 sec (15)/30 sec (30)/45 sec(45)/60 sec(60)") == SA_OK) {
		/* Auto Power control RDR has beeen added successfully.
		   Set RDR and CONTROL capability flags. Please note that
		   failing to add control rdr is not treated as a critical
		   error that requires to fail plug-in initilization */

		ilo2_ribcl_add_resource_capability( oh_handler, ev,
			     (SAHPI_CAPABILITY_RDR | SAHPI_CAPABILITY_CONTROL));
		
	} else {
		err("iLO2 RIBCL: Failed to setup Auto Power Control RDR. Plug-in will run without this control feature");
	}

	/* Add sensor RDRs. We use three general system health sensors. */
	ilo2_ribcl_discover_chassis_sensors( oh_handler, ev); 

	/* Add an inventory RDR */
	ilo2_ribcl_discover_chassis_idr( oh_handler, ev, tmptag);
	free( tmptag);

	/* Now, fill out the rest of the event structure and push it
	 * onto the event queue. */

	ev->hid = oh_handler->hid;
	ev->event.EventType = SAHPI_ET_RESOURCE;
	ev->event.Severity = ev->resource.ResourceSeverity;
	ev->event.Source = ev->resource.ResourceId;
	if ( oh_gettimeofday(&(ev->event.Timestamp)) != SA_OK){
		ev->event.Timestamp = SAHPI_TIME_UNSPECIFIED;
	}
	ev->event.EventDataUnion.ResourceEvent.ResourceEventType = SAHPI_RESE_RESOURCE_ADDED; 

	oh_evt_queue_push(oh_handler->eventq, ev);
	return( SA_OK);

} /* end ilo2_ribcl_discover_chassis() */

/**
 * ilo2_ribcl_discover_cpu
 * @oh_handler:  Handler data pointer.
 * @ep_root: pointer to the root path for this instance.
 *
 * This function creates the rpt entry for our CPU resources. After adding
 * the CPU resource to the handler's rpt cache, we send a
 * SAHPI_RESE_RESOURCE_ADDED event.
 *
 * Detailed description:
 *
 *	For all CPUs found in DiscoveryData
 *		- Use the CPU label from iLO2 as the resource tag.
 *		- Build an EntityPath for this resource with an EntityType of
 *		  SAHPI_ENT_PROCESSOR and an EntityLocation set to the index
 *		  of this processor given by iLO2 in its label.  
 *		- If the IR_DISCOVERED flag is set for this dimm, call
 *		  ilo2_ribcl_discovered_fru(), otherwise call
 *		  ilo2_ribcl_undiscovered_fru().
 *	End for all CPUs
 *
 * Return values:
 *	SA_OK if success
 *	SA_ERR_HPI_OUT_OF_MEMORY if allocation fails
 **/
static SaErrorT ilo2_ribcl_discover_cpu( struct oh_handler_state *oh_handler,
				SaHpiEntityPathT *ep_root)
{
	ilo2_ribcl_handler_t *ir_handler;
	ir_cpudata_t	*cpudata;
	SaHpiEntityPathT cpu_ep;
	int idex;
	SaErrorT ret;

        ir_handler = (ilo2_ribcl_handler_t *) oh_handler->data;

	for( idex = 1; idex <= ILO2_RIBCL_DISCOVER_CPU_MAX; idex++){

		cpudata = &(ir_handler->DiscoveryData.cpudata[idex]);

		if( cpudata->label == NULL){
			continue;
		}

		/* 
		 * Build the entity path for this CPU. Please note that
		 * cpu_ep.Entry[1].EntityLocation = 0 is necessary to
		 * indicate path termination to oh_ConCat_Ep and other
		 * OpenHPI utilities
		*/

		cpu_ep.Entry[0].EntityType = SAHPI_ENT_PROCESSOR;
                cpu_ep.Entry[0].EntityLocation = idex;
                cpu_ep.Entry[1].EntityType = SAHPI_ENT_ROOT;
                cpu_ep.Entry[1].EntityLocation = 0;
                oh_concat_ep(&cpu_ep, ep_root);

		if( cpudata->cpuflags & IR_DISCOVERED ){

			/* Build the cpu IDR from DiscoveryData. 
			 * Use the temporary ilo2_ribcl_idr_info structure in
			 * our private handler to collect the IDR information.
			 * We use this buffer in the handler because it's too
			 * large to put on the stack as a local valiable, and
			 * we don't want to be allocating/deallocating it
			 * frequently. */

			ilo2_ribcl_build_cpu_idr( ir_handler, 
						     &(ir_handler->tmp_idr));

			ret = ilo2_ribcl_discovered_fru( oh_handler, &cpu_ep,
					&(cpudata->dstate), 0, cpudata->label,
					&(ir_handler->tmp_idr));

			/* Update the IDR data if it has changed */
			ilo2_ribcl_update_fru_idr( oh_handler, &cpu_ep,
							&(ir_handler->tmp_idr));

		} else { /* We didn't find it on the last iLO2 poll */

			ret = ilo2_ribcl_undiscovered_fru( oh_handler, &cpu_ep,
				&(cpudata->dstate), 0, cpudata->label);

		}

		/* If there's an error, abort the rest of the recovery */
		if( ret != SA_OK){
			return( ret);
		}

	} /* end for idex */

	return( SA_OK);

} /* end ilo2_ribcl_discover_cpu() */



/**
 * ilo2_ribcl_discover_memory
 * @oh_handler:  Handler data pointer.
 * @ep_root: pointer to the root path for this instance.
 *
 * This function handles the insertion and deletion of memory DIMM resources
 * detected during discovery.
 *
 * Detailed description:
 *
 *	For all Memory DIMMS found in DiscoveryData
 *		- Use the DIMM label from iLO2 as the resource tag.
 *		- Build an EntityPath for this resource with an EntityType of
 *		  SAHPI_ENT_MEMORY_DEVICE and an EntityLocation set to the index
 *		  of this DIMM given by iLO2 in its label. 
 *		- If the IR_DISCOVERED flag is set for this dimm, call
 *		  ilo2_ribcl_discovered_fru(), otherwise call
 *		  ilo2_ribcl_undiscovered_fru(). 
 *	End for all DIMMS
 *
 * Return values:
 *	SA_OK if success
 *	SA_ERR_HPI_OUT_OF_MEMORY if allocation fails
 **/
static SaErrorT ilo2_ribcl_discover_memory( struct oh_handler_state *oh_handler,
				SaHpiEntityPathT *ep_root)
{
	ilo2_ribcl_handler_t *ir_handler;
	ir_memdata_t *memdata;	
	SaHpiEntityPathT mem_ep;
	int idex;
	SaErrorT  ret;

        ir_handler = (ilo2_ribcl_handler_t *) oh_handler->data;

	for( idex = 1; idex <= ILO2_RIBCL_DISCOVER_MEM_MAX; idex++){

		memdata = &(ir_handler->DiscoveryData.memdata[idex]);

		if( memdata->label == NULL){
			/* If we have ever obtained data on this resource,
			 * the memory label will be non-NULL. */
			continue;
		}

		/* Build the entity path for this memory DIMM */

		mem_ep.Entry[0].EntityType = SAHPI_ENT_MEMORY_DEVICE;
		mem_ep.Entry[0].EntityLocation = idex;
		mem_ep.Entry[1].EntityType = SAHPI_ENT_ROOT;
		mem_ep.Entry[1].EntityLocation = 0;
		oh_concat_ep(&mem_ep, ep_root);

		/* iLO2 response for Memory components doesn't contain 
		   RIBCL status. */

		if( memdata->memflags & IR_DISCOVERED ){

			/* Build the memory IDR from DiscoveryData. 
			 * Use the temporary ilo2_ribcl_idr_info structure in
			 * our private handler to collect the IDR information.
			 * We use this buffer in the handler because it's too
			 * large to put on the stack as a local valiable, and
			 * we don't want to be allocating/deallocating it
			 * frequently. */

			ilo2_ribcl_build_memory_idr( memdata, 
						     &(ir_handler->tmp_idr));

			ret = ilo2_ribcl_discovered_fru( oh_handler, &mem_ep,
				&(memdata->dstate), 0, memdata->label,
				&(ir_handler->tmp_idr));

			/* Update the IDR data if it has changed */
			ilo2_ribcl_update_fru_idr( oh_handler, &mem_ep,
							&(ir_handler->tmp_idr));

		} else { /* We didn't find it on the last iLO2 poll */
			
			ret = ilo2_ribcl_undiscovered_fru( oh_handler, &mem_ep,
				&(memdata->dstate), 0, memdata->label);
	
		}

		/* If there's an error, abort the rest of the recovery */
		if( ret != SA_OK){
			return( ret);
		}

	} /* end for idex */

	return( SA_OK);

} /* end ilo2_ribcl_discover_memory() */



/**
 * ilo2_ribcl_discover_fans
 * @oh_handler:  Handler data pointer.
 * @ep_root: pointer to the root path for this instance.
 *
 * This function handles the insertion and deletion of fan resources
 * detected during discovery.
 *
 * Detailed description:
 *
 *	For all fans found in DiscoveryData
 *		- Use the fan label and fan zone from iLO2 as the resource tag.
 *		- Build an EntityPath for this resource with an EntityType of
 *		  SAHPI_ENT_COOLING_DEVICE and an EntityLocation set to the
 *		  index of this fan given by iLO2 in its label. 
 *		- If the IR_DISCOVERED flag is set for this fan, call
 *		  ilo2_ribcl_discovered_fru(), otherwise call
 *		  ilo2_ribcl_undiscovered_fru(). 
 *	End for all fans
 *
 * Return values:
 *	SA_OK if success
 *	SA_ERR_HPI_OUT_OF_MEMORY if allocation fails
 **/
static SaErrorT ilo2_ribcl_discover_fans( struct oh_handler_state *oh_handler,
				SaHpiEntityPathT *ep_root)
{
	ilo2_ribcl_handler_t *ir_handler;
	char *fanstatus;
	ir_fandata_t *fandata;
	char *fantag;
	size_t tagsize;
	SaHpiEntityPathT fan_ep;
	int idex;
	int failed;
	SaErrorT  ret;

        ir_handler = (ilo2_ribcl_handler_t *) oh_handler->data;

	for( idex = 1; idex <= ILO2_RIBCL_DISCOVER_FAN_MAX; idex++){

		fandata = &(ir_handler->DiscoveryData.fandata[idex]);

		if( fandata->label == NULL){
			/* If we have ever obtained data on this resource,
			 * the fan label will be non-NULL. */
			continue;
		}

		/* Build the entity path for this fan */

		fan_ep.Entry[0].EntityType = SAHPI_ENT_COOLING_DEVICE;
		fan_ep.Entry[0].EntityLocation = idex;
		fan_ep.Entry[1].EntityType = SAHPI_ENT_ROOT;
		fan_ep.Entry[1].EntityLocation = 0;
		oh_concat_ep(&fan_ep, ep_root);

		/* Check if the discovery reported a failed fan */
		failed = SAHPI_FALSE;
		fanstatus = fandata->status;
		if( (fanstatus != NULL) && !strcmp( fanstatus, "Failed")){
			failed = SAHPI_TRUE;
		}

		/* include the fan location in the text tag */
		tagsize =  strlen( fandata->label) +
				strlen( fandata->zone) + 11;
		fantag = malloc( tagsize);
		if( fantag == NULL){
			err("ilo2_ribcl_discover_fans(): malloc of %zd failed",
				tagsize);
			return(  SA_ERR_HPI_OUT_OF_MEMORY);
		}
		snprintf( fantag, tagsize, "%s Location %s", fandata->label,
			fandata->zone);

		if( fandata->fanflags & IR_DISCOVERED ){

			ret = ilo2_ribcl_discovered_fru( oh_handler, &fan_ep,
				&(fandata->dstate), failed, fantag, NULL); 

		} else { /* We didn't find it on the last iLO2 poll */
			
			ret = ilo2_ribcl_undiscovered_fru( oh_handler, &fan_ep,
				&(fandata->dstate), failed, fantag);
	
		}

		free( fantag);

		/* If there's an error, abort the rest of the recovery */
		if( ret != SA_OK){
			return( ret);
		}

	} /* end for idex */

	return( SA_OK);

} /* end ilo2_ribcl_discover_fans() */



/**
 * ilo2_ribcl_discover_powersupplies
 * @oh_handler:  Handler data pointer.
 * @ep_root: pointer to the root path for this instance.
 *
 * This function handles the insertion and deletion of power supply resources
 * detected during discovery.
 *
 * Detailed description:
 *
 *	For all power supplies found in DiscoveryData
 *		- Use the power supply label from iLO2 as the resource tag.
 *		- Build an EntityPath for this resource with an EntityType of
 *		  SAHPI_ENT_POWER_SUPPLY and an EntityLocation set to the
 *		  index of this power supply given by iLO2 in its label. 
 *		- If the IR_DISCOVERED flag is set for this power supply, call
 *		  ilo2_ribcl_discovered_fru(), otherwise call
 *		  ilo2_ribcl_undiscovered_fru(). 
 *	End for all power supplies
 *
 * Return values:
 *	SA_OK if success
 *	SA_ERR_HPI_OUT_OF_MEMORY if allocation fails
 **/
static SaErrorT ilo2_ribcl_discover_powersupplies( struct oh_handler_state *oh_handler,
				SaHpiEntityPathT *ep_root)
{
	ilo2_ribcl_handler_t *ir_handler;
	char *psustatus;
	ir_psudata_t *psudata;	
	SaHpiEntityPathT psu_ep;
	int idex;
	int failed;
	SaErrorT  ret;

        ir_handler = (ilo2_ribcl_handler_t *) oh_handler->data;

	for( idex = 1; idex <= ILO2_RIBCL_DISCOVER_PSU_MAX; idex++){

		psudata = &(ir_handler->DiscoveryData.psudata[idex]);

		if( psudata->label == NULL){
			/* If we have ever obtained data on this resource,
			 * the power supply label will be non-NULL. */
			continue;
		}

		/* Build the entity path for this psu */

		psu_ep.Entry[0].EntityType = SAHPI_ENT_POWER_SUPPLY;
		psu_ep.Entry[0].EntityLocation = idex;
		psu_ep.Entry[1].EntityType = SAHPI_ENT_ROOT;
		psu_ep.Entry[1].EntityLocation = 0;
		oh_concat_ep(&psu_ep, ep_root);

		/* Check if the discovery reported a failed power supply */
		failed = SAHPI_FALSE;
		psustatus = psudata->status;
		if( (psustatus != NULL) && !strcmp( psustatus, "Failed")){
			failed = SAHPI_TRUE;
		}

		if( psudata->psuflags & IR_DISCOVERED ){

			ret = ilo2_ribcl_discovered_fru( oh_handler, &psu_ep,
				&(psudata->dstate), failed, psudata->label,
				NULL); 

		} else { /* We didn't find it on the last iLO2 poll */
			
			ret = ilo2_ribcl_undiscovered_fru( oh_handler, &psu_ep,
				&(psudata->dstate), failed, psudata->label);
	
		}

		/* If there's an error, abort the rest of the recovery */
		if( ret != SA_OK){
			return( ret);
		}

	} /* end for idex */

	return( SA_OK);

} /* end ilo2_ribcl_discover_powersupplies() */



/**
 * ilo2_ribcl_discover_vrm
 * @oh_handler:  Handler data pointer.
 * @ep_root: pointer to the root path for this instance.
 *
 * This function handles the insertion and deletion of voltage regulator module
 *  resources detected during discovery.
 *
 * Detailed description:
 *
 *	For all VRMs found in DiscoveryData
 *		- Use the VRM label from iLO2 as the resource tag.
 *		- Build an EntityPath for this resource with an EntityType of
 *		  SAHPI_ENT_POWER_MODULE and an EntityLocation set to the
 *		  index of this VRM given by iLO2 in its label. 
 *		- If the IR_DISCOVERED flag is set for this VRM, call
 *		  ilo2_ribcl_discovered_fru(), otherwise call
 *		  ilo2_ribcl_undiscovered_fru(). 
 *	End for all VRMs
 *
 * Return values:
 *	SA_OK if success
 *	SA_ERR_HPI_OUT_OF_MEMORY if allocation fails
 **/
static SaErrorT ilo2_ribcl_discover_vrm( struct oh_handler_state *oh_handler,
				SaHpiEntityPathT *ep_root)
{
	ilo2_ribcl_handler_t *ir_handler;
	char *vrmstatus;
	ir_vrmdata_t *vrmdata;	
	SaHpiEntityPathT vrm_ep;
	int idex;
	int failed;
	SaErrorT  ret;

        ir_handler = (ilo2_ribcl_handler_t *) oh_handler->data;

	for( idex = 1; idex <= ILO2_RIBCL_DISCOVER_VRM_MAX; idex++){

		vrmdata = &(ir_handler->DiscoveryData.vrmdata[idex]);

		if( vrmdata->label == NULL){
			/* If we have ever obtained data on this resource,
			 * the power supply label will be non-NULL. */
			continue;
		}

		/* Build the entity path for this vrm */

		vrm_ep.Entry[0].EntityType = SAHPI_ENT_POWER_MODULE;
		vrm_ep.Entry[0].EntityLocation = idex;
		vrm_ep.Entry[1].EntityType = SAHPI_ENT_ROOT;
		vrm_ep.Entry[1].EntityLocation = 0;
		oh_concat_ep(&vrm_ep, ep_root);

		/* Check if the discovery reported a failed vrm */
		failed = SAHPI_FALSE;
		vrmstatus = vrmdata->status;
		if( (vrmstatus != NULL) && !strcmp( vrmstatus, "Failed")){
			failed = SAHPI_TRUE;
		}

		if( vrmdata->vrmflags & IR_DISCOVERED ){

			ret = ilo2_ribcl_discovered_fru( oh_handler, &vrm_ep,
				&(vrmdata->dstate), failed, vrmdata->label,
				NULL); 

		} else { /* We didn't find it on the last iLO2 poll */
			
			ret = ilo2_ribcl_undiscovered_fru( oh_handler, &vrm_ep,
				&(vrmdata->dstate), failed, vrmdata->label);
	
		}

		/* If there's an error, abort the rest of the recovery */
		if( ret != SA_OK){
			return( ret);
		}

	} /* end for idex */

	return( SA_OK);

} /* end ilo2_ribcl_discover_vrm()*/


/**
 * ilo2_ribcl_discovered_fru
 * @oh_handler:  Handler data pointer.
 * @resource_ep: pointer to the entity path for this resource.
 * @d_state: 	 The current discovery state of the resource.
 * @isfailed:	 Indicates if the resource is currently detected as failed.
 * @tag:	 Characer string used for resource tag if rpt entry is created.
 * @idr_info:	 Pointer to IDR information if this resource should have an IDR,
 *		 Null otherwise.
 *
 * This function is called for removable resources whose presence have
 * been detected during a discovery operation. The action taken depends
 * upon the current resource state passed as parameter d_state, and whether
 * the resource is currently detected to be in a failed condition. The value of
 * d_state can be modified by this routine.
 *
 * Detailed description:
 *
 * These are the actions taken for each value of d_state:
 *
 * BLANK: This is the initial state for a resource. In this state, a resource
 *	has never been detected before.
 * 	- Allocate a new resource event.
 *	- Set ResourceCapabilities to SAHPI_CAPABILITY_RESOURCE,
 *	  HotSwapCapabilities to 0, ResourceSeverity to SAHPI_CRITICAL,
 *	  and use the tag parameter for the ResourceTag.
 *	- Add the resource to the handler's rpt cache with a call to
 *	  oh_add_resource().
 *	- If this resource has an Inventory Data Repository, the idr_info
 *	  parameter will be non-null. In this case, call ilo2_ribcl_add_idr()
 *	  to add the IDR to this resource.  
 *	- Call oh_evt_queue_push() to send a SAHPI_RESE_RESOURCE_ADDED
 *	  resource event. 
 *	- Set d_state to OK
 *	- Fall through to the OK state.
 *
 * OK:	This is the state for a resource that has been previously discovered
 *	and had not been detected as failing during the previous discovery.
 *	In this state, we check for failure.
 *	If the isfailed parameter is non-zero
 *		- Look up the existing rpt entry from the resource_ep
 *		  entity path parameter.
 *		- Set ResourceFailed to SAHPI_TRUE in that rpt entry.
 *		- Call oh_evt_queue_push() to send a SAHPI_RESE_RESOURCE_FAILURE
 *		  resource event. 
 *		- Set d_state to FAILED.
 *	End isfailed parameter is non-zero
 *	 
 * FAILED: This is the state for a resource that has been previously discovered
 *	and has been detected as failing during the previous discovery.
 *	In this state, we check if the resource is no longer failing. 
 *	If the isfailed parameter is zero
 *		- Look up the existing rpt entry from the resource_ep
 *		  entity path parameter.
 *		- Set ResourceFailed to SAHPI_FALSE in that rpt entry.
 *		- Call oh_evt_queue_push() to send a
 *		  SAHPI_RESE_RESOURCE_RESTORED resource event. 
 *		- Set d_state to OK.
 *	End isfailed parameter is non-zero
 *	 
 * REMOVED: This is the state for a resource that has been previously
 *	discovered and has been detected as missing during the previous
 *	discovery operation. We now need to re-add this device.
 *
 * Return values:
 *	SA_OK if success
 *	SA_ERR_HPI_OUT_OF_MEMORY if allocation fails
 *	SA_ERR_HPI_NOT_PRESENT if a rpt entry for a resource isn't found.
 *	SA_ERR_HPI_INTERNAL_ERROR id d_state is unknown.
 **/
static SaErrorT ilo2_ribcl_discovered_fru( struct oh_handler_state *oh_handler,
			SaHpiEntityPathT *resource_ep,
 			enum ir_discoverstate *d_state,
			int isfailed,
			char *tag,
			struct ilo2_ribcl_idr_info *idr_info )
{
	struct oh_event *ev;
	SaHpiRptEntryT *rpt;
	SaErrorT ret;
	SaHpiBoolT resource_wasfailed;
	ilo2_ribcl_resource_info_t *res_info = NULL;

	switch( *d_state){

	case BLANK:	/* Do our initial rpt creation and addition */

		/* Create a resource event */
		ev = oh_new_event();
		if( ev == NULL){
			err("ilo2_ribcl_discovered_fru(): event allocation failed.");
			return( SA_ERR_HPI_OUT_OF_MEMORY);
		}

		/* Fill in the resource part of the event */

		ev->resource.ResourceEntity = *resource_ep;

		/* Plugin doesn't set EntryID */
		ev->resource.ResourceId = oh_uid_from_entity_path(
					&(ev->resource.ResourceEntity));
		ev->resource.ResourceInfo.ManufacturerId = HP_MANUFACTURING_ID;
		ev->resource.ResourceCapabilities = 
			(SAHPI_CAPABILITY_RESOURCE | SAHPI_CAPABILITY_FRU); 

		ev->resource.HotSwapCapabilities = 0;
		ev->resource.ResourceSeverity = SAHPI_CRITICAL;
		ev->resource.ResourceFailed = isfailed;
		oh_init_textbuffer(&(ev->resource.ResourceTag));
		oh_append_textbuffer( &(ev->resource.ResourceTag), tag);

		/* Allocate and populate iLO2 RIBCL private data area to 
		   be added to the resource rpt cache */
		res_info = (ilo2_ribcl_resource_info_t *)
			g_malloc0(sizeof(ilo2_ribcl_resource_info_t));
		if(res_info == NULL) {
			err("ilo2_ribcl_discovered_fru(): out of memory");
			oh_event_free( ev, 0);
			return( SA_ERR_HPI_OUT_OF_MEMORY);
		}
		res_info->rid = ev->resource.ResourceId;
		res_info->fru_cur_state = SAHPI_HS_STATE_ACTIVE;
		res_info->disc_data_idx = resource_ep->Entry[0].EntityLocation;

		/* Add the resource to this instance's rpt cache */
		ret = oh_add_resource( oh_handler->rptcache, &(ev->resource),
					res_info, 0);
		if( ret != SA_OK){
			err("ilo2_ribcl_discovered_fru(): cannot add resource to rptcache.");
			oh_event_free( ev, 0);
			return( ret);
		}

		/* If this this resource has an associated Inventory Data
		 * Repository, the IDR data will be passed in via the 
		 * idr_info parameter. */

		if( idr_info != NULL){
			ret = ilo2_ribcl_add_idr( oh_handler, ev,
				SAHPI_DEFAULT_INVENTORY_ID, idr_info, tag);
			if( ret != SA_OK){
				err("ilo2_ribcl_discovered_fru: could not add IDR to resource id %d.",
					 ev->resource.ResourceId);
			}
		}

		/* Now, fill out the rest of the event structure and push it
	 	* onto the event queue. */

		ev->hid = oh_handler->hid;
		ev->event.EventType = SAHPI_ET_HOTSWAP;
		ev->event.Severity = ev->resource.ResourceSeverity;
		ev->event.Source = ev->resource.ResourceId;
		if ( oh_gettimeofday(&(ev->event.Timestamp)) != SA_OK){
			ev->event.Timestamp = SAHPI_TIME_UNSPECIFIED;
		}
		ev->event.EventDataUnion.HotSwapEvent.HotSwapState =
			SAHPI_HS_STATE_ACTIVE;
		ev->event.EventDataUnion.HotSwapEvent.PreviousHotSwapState =
			SAHPI_HS_STATE_NOT_PRESENT;
		ev->event.EventDataUnion.HotSwapEvent.CauseOfStateChange =
			SAHPI_HS_CAUSE_UNKNOWN;

		oh_evt_queue_push(oh_handler->eventq, ev);
		*d_state = OK;

		/* Fall through to OK state */

	case OK: /* Check to see if we have failed. If so, update the rpt
		  * entry for this resource, and send a 
		  * SAHPI_RESE_RESOURCE_FAILURE event. */

		if( isfailed){
			ret = ilo2_ribcl_resource_set_failstatus( oh_handler,
				resource_ep, SAHPI_TRUE);

			/* If we were out of memory, stay in state OK
			 * and try again next time.
			 */
			if( ret != SA_ERR_HPI_OUT_OF_MEMORY){
				*d_state = FAILED;
			}

			if( ret != SA_OK){
				return( ret);
			}
		}
		break;
	
	case FAILED: /* Check to see if we are no longer failed. If so,
		      * update the rpt entry for this resource, and send a
		      * SAHPI_RESE_RESOURCE_RESTORED event */

		if( !isfailed){
			ret = ilo2_ribcl_resource_set_failstatus( oh_handler,
				resource_ep, SAHPI_FALSE);

			/* If we were out of memory, stay in state FAILED
			 * and try again next time.
			 */
			if( ret != SA_ERR_HPI_OUT_OF_MEMORY){
				*d_state = OK;
			}

			if( ret != SA_OK){
				return( ret);
			}
		}
		break;

	case REMOVED: /* We have been rediscovered after being removed */

		/* - get our rpt entry from our entity path with a call to
		 *   oh_get_resource_by_ep().
		 * - send a hotswap event putting us into SAHPI_HS_STATE_ACTIVE
		 * - check for failure. If previously failed, and not failed
		 *   now, send a SAHPI_RESE_RESOURCE_RESTORED resource event.
		 *   if failed now, send a SAHPI_RESE_RESOURCE_FAILURE resource
		 *   event.
 		 * - set d_state based on failure status */

		rpt = oh_get_resource_by_ep( oh_handler->rptcache,
			resource_ep);
		if( rpt == NULL){
			/* This should never happen */
			err("ilo2_ribcl_discovered_fru(): Null rpt entry for removed resource");
			*d_state = OK;
			return( SA_ERR_HPI_NOT_PRESENT);
		}
		res_info =  (ilo2_ribcl_resource_info_t *)oh_get_resource_data(
			oh_handler->rptcache, rpt->ResourceId);
		if (!res_info) {
			/* This should never happen */
			err("ilo2_ribcl_discovered_fru(): No resource information for a removed resource.");
			return( SA_ERR_HPI_NOT_PRESENT);
		}

		resource_wasfailed = rpt->ResourceFailed;

		ev = oh_new_event();
		if( ev == NULL){
			err("ilo2_ribcl_discovered_fru(): event allocation failed.");
			return( SA_ERR_HPI_OUT_OF_MEMORY);
		}

		/* Copy the rpt information from our handler's rptcache */
		ev->resource = *rpt;

		/* If this this resource has an associated Inventory Data
		 * Repository, the IDR data will be passed in via the 
		 * idr_info parameter. When we sent the
		 * SAHPI_HS_STATE_NOT_PRESENT event previously for this
		 * resource, the daemon removed the rpt entry and all of
		 * its rdrs from the domain table. So, we need to add the
		 * rdrs back via this event's rdrs list. */

		if( idr_info != NULL){
			ret = ilo2_ribcl_add_idr( oh_handler, ev,
				SAHPI_DEFAULT_INVENTORY_ID, idr_info, tag);
			if( ret != SA_OK){
				err("ilo2_ribcl_discovered_fru: could not add IDR to resource id %d.",
					 ev->resource.ResourceId);
			}
		}

		ev->hid = oh_handler->hid;
		ev->event.EventType = SAHPI_ET_HOTSWAP;
		ev->event.Severity = ev->resource.ResourceSeverity;
		ev->event.Source = ev->resource.ResourceId;
		if ( oh_gettimeofday(&(ev->event.Timestamp)) != SA_OK){
			ev->event.Timestamp = SAHPI_TIME_UNSPECIFIED;
		}
		ev->event.EventDataUnion.HotSwapEvent.HotSwapState =
			SAHPI_HS_STATE_ACTIVE;
		ev->event.EventDataUnion.HotSwapEvent.PreviousHotSwapState =
			SAHPI_HS_STATE_NOT_PRESENT;
		ev->event.EventDataUnion.HotSwapEvent.CauseOfStateChange =
			SAHPI_HS_CAUSE_UNKNOWN;

		/* update resource private data with the new state */
		res_info->fru_cur_state = ev->event.EventDataUnion.HotSwapEvent.HotSwapState;

		oh_evt_queue_push(oh_handler->eventq, ev);

		ret = SA_OK;
		*d_state = OK;

		/* Now handle the resource failure condition. */

		if( isfailed){

			ret = ilo2_ribcl_resource_set_failstatus(
				 oh_handler, resource_ep, SAHPI_TRUE);

			/* If we were out of memory, stay in state OK 
			 * and try again next time.
			 */
			if( ret != SA_ERR_HPI_OUT_OF_MEMORY){
				*d_state = FAILED;
			}

			if( ret != SA_OK){
				return( ret);
			}

		} else { /* If the resource was failed before removal,
			  * and now is no longer failed, send a 
			  * SAHPI_RESE_RESOURCE_RESTORED resource event */

			if( resource_wasfailed){
				ret = ilo2_ribcl_resource_set_failstatus(
					 oh_handler, resource_ep, SAHPI_FALSE);
				/* If we were out of memory, stay in state
				 * FAILED and try again next time.
				 */
				if( ret != SA_ERR_HPI_OUT_OF_MEMORY){
					*d_state = FAILED;
				}

				if( ret != SA_OK){
					return( ret);
				}
			}

		}

		break;

	default: 
		err("ilo2_ribcl_discovered_fru(): invalid d_state");
		return( SA_ERR_HPI_INTERNAL_ERROR);  
		break;
	
	} /* end switch d_state */

	return( SA_OK);

} /* end ilo2_ribcl_discovered_fru() */



/**
 * ilo2_ribcl_resource_set_failstatus
 * @oh_handler:  Handler data pointer.
 * @resource_ep: pointer to the entity path for this resource.
 * @resource_failed: SAHPI_TRUE if resource has failed, SAHPI_FALSE otherwise.
 *
 * Detailed description:
 *	- Look up the existing rpt entry from the resource_ep entity
 *	  path parameter.
 *	- Set ResourceFailed to the value of parameter resource_failed
 *	  in that rpt entry.
 *	- Call oh_evt_queue_push() to send a SAHPI_RESE_RESOURCE_FAILURE
 *	  resource event if the resource has failed, or a
 *	  SAHPI_RESE_RESOURCE_RESTORED resource event if the device is no
 *	  longer failed. 
 *
 * Return values:
 *	SA_OK if success
 *	SA_ERR_HPI_OUT_OF_MEMORY if allocation fails.
 *	SA_ERR_HPI_NOT_PRESENT if the rpt entry for resource_ep is not found.
 **/
static SaErrorT ilo2_ribcl_resource_set_failstatus(
	struct oh_handler_state *oh_handler, SaHpiEntityPathT *resource_ep,
	SaHpiBoolT resource_failed )
{
	struct oh_event *ev;
	SaHpiRptEntryT *rpt;

	rpt = oh_get_resource_by_ep( oh_handler->rptcache,
		resource_ep);
	if( rpt == NULL){
		/* This should never happen */
		err("ilo2_ribcl_resource_set_failstatus(): Null rpt entry for failed resource");
		return( SA_ERR_HPI_NOT_PRESENT);
	}

	rpt->ResourceFailed = resource_failed;

	/* Generate a RESOURCE_FAILURE event */

	ev = oh_new_event();
	if( ev == NULL){
		err("ilo2_ribcl_resource_set_failstatus(): event allocation failed.");
		return( SA_ERR_HPI_OUT_OF_MEMORY);
	}

	ev->resource = *rpt;
	ev->hid = oh_handler->hid;
	ev->event.EventType = SAHPI_ET_RESOURCE;
	ev->event.Severity = ev->resource.ResourceSeverity;
	ev->event.Source = ev->resource.ResourceId;
	if ( oh_gettimeofday(&(ev->event.Timestamp)) != SA_OK){
		ev->event.Timestamp = SAHPI_TIME_UNSPECIFIED;
	}
	if( resource_failed == SAHPI_FALSE){
		ev->event.EventDataUnion.ResourceEvent.ResourceEventType
					= SAHPI_RESE_RESOURCE_RESTORED;
	} else {
		ev->event.EventDataUnion.ResourceEvent.ResourceEventType
					= SAHPI_RESE_RESOURCE_FAILURE;
	}
				
	oh_evt_queue_push(oh_handler->eventq, ev);

	return( SA_OK);

} /* end ilo2_ribcl_resource_fail() */



/**
 * ilo2_ribcl_undiscovered_fru
 * @oh_handler:  Handler data pointer.
 * @resource_ep: pointer to the entity path for this resource.
 * @d_state: 	 The current discovery state of the resource.
 * @isfailed:	 Indicates if the resource is currently detected as failed.
 * @tag:	 Characer string used for resource tag if rpt entry is created.
 *
 * This function is called for removable resources whose presence have
 * not been detected during a discovery operation. The action taken depends
 * upon the current resource state passed as parameter d_state, and whether
 * the resource is currently detected to be in a failed condition. The value of
 * d_state can be modified by this routine.
 *
 * Detailed description:
 *
 * These are the actions taken for each value of d_state:
 *
 * BLANK: This is the initial state for a resource. In this state, a resource
 *	has never been detected before. For this state, we do nothing.

 * OK:	This is the state for a resource that has been previously discovered
 *	and had not been detected as failing during the previous discovery.
 *	Since this resource is now missing, we should indicate it has been 
 *	removed.  
 *
 * FAILED: This is the state for a resource that has been previously discovered
 *	and has been detected as failing during the previous discovery.
 *	Since this resource is now missing, we should indicate it has been
 *      removed.
 *
 * REMOVED: This is the state for a resource that has been previously
 *	discovered and has been detected as missing during the previous
 *	discovery operation. Since it's still missing now, we do nothing.
 *
 * Return values:
 *	SA_OK if success
 *	SA_ERR_HPI_INTERNAL_ERROR id d_state is unknown.
 **/
static SaErrorT ilo2_ribcl_undiscovered_fru( struct oh_handler_state *oh_handler,
	SaHpiEntityPathT *resource_ep, enum ir_discoverstate *d_state,
	int isfailed, char *tag )
{
	struct oh_event *ev;
	SaHpiRptEntryT *rpt;
	ilo2_ribcl_resource_info_t *res_info = NULL;

	switch( *d_state){

	case BLANK:
	case REMOVED:	/* nothing to do for these two states */
		return( SA_OK);
		break;

	case OK:
	case FAILED:	/* remove this resource */

		/* Use the resource_ep to locate the rpt entry. Then
		 * send a hotswap event putting this resource into a
		 * SAHPI_HS_STATE_NOT_PRESENT state */

		rpt = oh_get_resource_by_ep( oh_handler->rptcache,
			resource_ep);
		if( rpt == NULL){
			/* This should never happen */
			err("ilo2_ribcl_undiscovered_fru(): Null rpt entry for removed resource");
			*d_state = OK;
			return( SA_ERR_HPI_NOT_PRESENT);
		}

		ev = oh_new_event();
		if( ev == NULL){
			err("ilo2_ribcl_undiscovered_fru(): event allocation failed.");
			return( SA_ERR_HPI_OUT_OF_MEMORY);
		}

		ev->resource = *rpt;
		ev->hid = oh_handler->hid;
		ev->event.EventType = SAHPI_ET_HOTSWAP;
		ev->event.Severity = ev->resource.ResourceSeverity;
		ev->event.Source = ev->resource.ResourceId;
		if ( oh_gettimeofday(&(ev->event.Timestamp)) != SA_OK){
			ev->event.Timestamp = SAHPI_TIME_UNSPECIFIED;
		}
		ev->event.EventDataUnion.HotSwapEvent.HotSwapState =
			SAHPI_HS_STATE_NOT_PRESENT;
		ev->event.EventDataUnion.HotSwapEvent.PreviousHotSwapState =
			SAHPI_HS_STATE_ACTIVE;
		ev->event.EventDataUnion.HotSwapEvent.CauseOfStateChange =
			SAHPI_HS_CAUSE_UNKNOWN;

		res_info =  (ilo2_ribcl_resource_info_t *)oh_get_resource_data(
			oh_handler->rptcache, rpt->ResourceId);
		if (!res_info) {
			/* This should never happen */
			err("ilo2_ribcl_discovered_fru(): No resource information for a removed resource.");
			return( SA_ERR_HPI_NOT_PRESENT);
		}
		res_info->fru_cur_state = 
			ev->event.EventDataUnion.HotSwapEvent.HotSwapState; 

		oh_evt_queue_push(oh_handler->eventq, ev);
		*d_state = REMOVED;

		break;
	
	default:
		err("ilo2_ribcl_undiscovered_fru(): invalid d_state");
		return( SA_ERR_HPI_INTERNAL_ERROR);  
		break;

	} /* end switch d_state */

	return( SA_OK);

} /* end ilo2_ribcl_undiscovered_fru() */

/**
 * ilo2_ribcl_free_discoverydata
 * @ir_handler: The private handler for this plugin instance.
 *
 * This function, intended to be called at plugin close time, will free any
 * dynamically allocated data in the handler DiscoveryData structure.
 *
 * Return values: None.
 **/
void ilo2_ribcl_free_discoverydata( ilo2_ribcl_handler_t *ir_handle)
{
	int idex;
	ilo2_ribcl_DiscoveryData_t *ddata;

	ddata = &(ir_handle->DiscoveryData);

	if( ddata->product_name != NULL){
		free( ddata->product_name);
	}

	if( ddata->serial_number != NULL){
		free( ddata->serial_number);
	}

	if( ddata->fwdata.version_string != NULL){
		free(  ddata->fwdata.version_string);
	}

	if( ddata->serial_number != NULL){
		free( ddata->system_cpu_speed);
	}

	/* Free the CPU data */
	for( idex = 1; idex <= ILO2_RIBCL_DISCOVER_CPU_MAX; idex++){
		if( ddata->cpudata[idex].label){
			free( ddata->cpudata[idex].label);
		}
	}

	/* Free the memory data */
	for( idex = 1; idex <= ILO2_RIBCL_DISCOVER_MEM_MAX; idex++){
		if( ddata->memdata[idex].label){
			free( ddata->memdata[idex].label);
		}
		if( ddata->memdata[idex].memsize){
			free( ddata->memdata[idex].memsize);
		}
		if( ddata->memdata[idex].speed){
			free( ddata->memdata[idex].speed);
		}
	}

	/* Free the fan data */
	for( idex = 1; idex <= ILO2_RIBCL_DISCOVER_FAN_MAX; idex++){
		if( ddata->fandata[idex].label){
			free( ddata->fandata[idex].label);
		}
		if( ddata->fandata[idex].zone){
			free( ddata->fandata[idex].zone);
		}
		if( ddata->fandata[idex].status){
			free( ddata->fandata[idex].status);
		}
		if( ddata->fandata[idex].speedunit){
			free( ddata->fandata[idex].speedunit);
		}
	}

	/* Free the power supply data */
	for( idex = 1; idex <= ILO2_RIBCL_DISCOVER_PSU_MAX; idex++){
		if( ddata->psudata[idex].label){
			free( ddata->psudata[idex].label);
		}
		if( ddata->psudata[idex].status){
			free( ddata->psudata[idex].status);
		}
	}

	/* Free the VRM data */
	for( idex = 1; idex <= ILO2_RIBCL_DISCOVER_PSU_MAX; idex++){
		if( ddata->vrmdata[idex].label){
			free( ddata->vrmdata[idex].label);
		}
		if( ddata->vrmdata[idex].status){
			free( ddata->vrmdata[idex].status);
		}
	}

	/* Temperature data not yet implemented */
	
}

/**
 * ilo2_ribcl_controls
 * @oh_handler:  Handler data pointer.
 * @ctl_type: iLO2 RIBCL control type.
 * @event: Pointer to event structure. 
 * @desc
 *
 * This function is called from chassis discovery routine to add control
 * RDR for a given control. This routine validates control type.
 *
 * Return values:
 *	SA_OK if success
 *	SA_ERR_HPI_INVALID_PARAMS
 *	SA_ERR_HPI_INTERNAL_ERROR
 *	SA_ERR_HPI_OUT_OF_MEMORY
 **/
static SaErrorT ilo2_ribcl_controls(struct oh_handler_state *oh_handler,
	int ctl_type, struct oh_event *event, char *desc)
{
	SaErrorT err;
	SaHpiRdrT *rdrptr;
	ilo2_ribcl_cinfo_t cinfo, *cinfo_ptr = NULL; 

	if(oh_handler == NULL) {
		err("ilo2_ribcl_controls(): Null handler");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}

	if(event == NULL) {
		err("ilo2_ribcl_controls(): Null event");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}

	if(desc == NULL) {
		err("ilo2_ribcl_controls(): Null Control Description String");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}

	if((ctl_type != ILO2_RIBCL_CTL_UID) &&
		(ctl_type != ILO2_RIBCL_CTL_POWER_SAVER) &&
		(ctl_type != ILO2_RIBCL_CTL_AUTO_POWER)) {
		err("ilo2_ribcl_controls(): Invalid iLO2 RIBCL control type");
		return(SA_ERR_HPI_INTERNAL_ERROR);
	}

	rdrptr = (SaHpiRdrT *)g_malloc0(sizeof(SaHpiRdrT));
	if (rdrptr == NULL) {
		err("ilo2_ribcl_controls(): Out of memory.");
		return(SA_ERR_HPI_OUT_OF_MEMORY);
	}
	rdrptr->RdrType = SAHPI_CTRL_RDR;
	rdrptr->Entity = event->resource.ResourceEntity;

	switch(ctl_type) {
		case ILO2_RIBCL_CTL_UID:
		{
			rdrptr->RdrTypeUnion.CtrlRec.Num = ILO2_RIBCL_CONTROL_1;
			rdrptr->RdrTypeUnion.CtrlRec.OutputType = SAHPI_CTRL_LED;
			rdrptr->RdrTypeUnion.CtrlRec.Type = SAHPI_CTRL_TYPE_DIGITAL;
			rdrptr->RdrTypeUnion.CtrlRec.TypeUnion.Digital.Default = SAHPI_CTRL_STATE_OFF;
			rdrptr->RdrTypeUnion.CtrlRec.DefaultMode.Mode = SAHPI_CTRL_MODE_MANUAL;
			rdrptr->RdrTypeUnion.CtrlRec.DefaultMode.ReadOnly = SAHPI_FALSE;
			rdrptr->RdrTypeUnion.CtrlRec.WriteOnly = SAHPI_FALSE;
			rdrptr->RdrTypeUnion.CtrlRec.Oem = 0;
			cinfo.ctl_type = ctl_type;
			cinfo.cur_mode = rdrptr->RdrTypeUnion.CtrlRec.DefaultMode.Mode;
			cinfo.cur_state.Digital = rdrptr->RdrTypeUnion.CtrlRec.TypeUnion.Digital.Default;
		}
		break;
		case ILO2_RIBCL_CTL_POWER_SAVER:
		{
		/*
		   The following outlines the Power Regulator feature:
		   The values are
			1 = OS Control Mode (Disabled Mode for iLO)
			2 = HP Static Low Power Mode
			3 = HP Dynamic Power Savings Mode
			4 = HP Static High Performance Mode
			Note: Value 4 is availble only for iLO 2 firmware
			version 1.20 and later.
		*/
			rdrptr->RdrTypeUnion.CtrlRec.Num = ILO2_RIBCL_CONTROL_2;
			rdrptr->RdrTypeUnion.CtrlRec.OutputType = SAHPI_CTRL_GENERIC;
			rdrptr->RdrTypeUnion.CtrlRec.Type = SAHPI_CTRL_TYPE_DISCRETE;
			rdrptr->RdrTypeUnion.CtrlRec.TypeUnion.Discrete.Default = ILO2_RIBCL_MANUAL_OS_CONTROL_MODE;
			rdrptr->RdrTypeUnion.CtrlRec.DefaultMode.Mode = SAHPI_CTRL_MODE_MANUAL;
			rdrptr->RdrTypeUnion.CtrlRec.DefaultMode.ReadOnly = SAHPI_FALSE;
			rdrptr->RdrTypeUnion.CtrlRec.WriteOnly = SAHPI_FALSE;
			rdrptr->RdrTypeUnion.CtrlRec.Oem = 0;
			cinfo.ctl_type = ctl_type;
			cinfo.cur_mode = rdrptr->RdrTypeUnion.CtrlRec.DefaultMode.Mode;
			cinfo.cur_state.Discrete = rdrptr->RdrTypeUnion.CtrlRec.TypeUnion.Discrete.Default;
		}
		break;
		case ILO2_RIBCL_CTL_AUTO_POWER:
		{
		/*
		   The following outlines the Auto Power feature:
		   The Auto Power Control allows user to change the
		   automatic power on and power on delay settings of the
		   server. The values are
			Yes = Enable automatic power on with a minimum delay.
			No = Disable automatic power on.
			15 = Enable automatic power on with 15 seconds delay.
			30 = Enable automatic power on with 30 seconds delay.
			45 = Enable automatic power on with 45 seconds delay.
			60 = Enable automatic power on with 60 seconds delay.
			Random = Enable automatic power on with random delay
				 up to 60 seconds.
		*/
			rdrptr->RdrTypeUnion.CtrlRec.Num = ILO2_RIBCL_CONTROL_3;
			rdrptr->RdrTypeUnion.CtrlRec.OutputType = SAHPI_CTRL_GENERIC;
			rdrptr->RdrTypeUnion.CtrlRec.Type = SAHPI_CTRL_TYPE_DISCRETE;
			rdrptr->RdrTypeUnion.CtrlRec.TypeUnion.Discrete.Default = ILO2_RIBCL_AUTO_POWER_DISABLED;
			rdrptr->RdrTypeUnion.CtrlRec.DefaultMode.Mode = SAHPI_CTRL_MODE_MANUAL;
			rdrptr->RdrTypeUnion.CtrlRec.DefaultMode.ReadOnly = SAHPI_FALSE;
			rdrptr->RdrTypeUnion.CtrlRec.WriteOnly = SAHPI_FALSE;
			rdrptr->RdrTypeUnion.CtrlRec.Oem = 0;
			cinfo.ctl_type = ctl_type;
			cinfo.cur_mode = rdrptr->RdrTypeUnion.CtrlRec.DefaultMode.Mode;
			cinfo.cur_state.Discrete = rdrptr->RdrTypeUnion.CtrlRec.TypeUnion.Discrete.Default;
		}
		break;
		default:
		{
			err("ilo2_ribcl_controls(): Invalid iLO2 RIBCL control type");
			g_free(rdrptr);
			return(SA_ERR_HPI_INTERNAL_ERROR);
		}
	}
	oh_init_textbuffer(&(rdrptr->IdString));
	oh_append_textbuffer(&(rdrptr->IdString), desc);

	/*
	   Allocate memory to save internal control type in private RDR area
	   This saved value will be used by the control API to determine the
	   the type of the control and send appropriate command down to RIBCL
	 */
	cinfo_ptr = g_memdup(&cinfo, sizeof(cinfo));
	if(cinfo_ptr == NULL) {
		err("ilo2_ribcl_controls(): Out of memory.");
		g_free(rdrptr);
		return(SA_ERR_HPI_OUT_OF_MEMORY);
	}
	
	err = oh_add_rdr(oh_handler->rptcache, event->resource.ResourceId,
		rdrptr, cinfo_ptr, 0);
	if (err) {
		err("Could not add RDR. Error=%s.", oh_lookup_error(err));
		g_free(rdrptr);
		g_free(cinfo_ptr);
		return(SA_ERR_HPI_INTERNAL_ERROR);
	} else {
		event->rdrs = g_slist_append(event->rdrs, rdrptr);
	}

	return(SA_OK);
}



/**
 * ilo2_ribcl_add_severity_sensor:
 * @oh_handler:  Handler data pointer.
 * @event: Pointer to event structure for sensor parent resource event.
 * @sens_num: Sensor number for new sensor.
 * @sens_type: HPI type of new sensor. 
 * @supported_states: Mask of all the EV states this sensor can support.
 * @sens_info: Private sensor info associated with RDR.
 * @description: Character string description of this sensor.
 *
 * This routine creates a new sensor of category SAHPI_EC_SEVERITY, using
 * the data given by the parameters. The new sensor RDR is added to
 * the parent resource given in the oh_event structure paramenter. 
 *
 * The following fields in the SensorRec of the RDR will be set to fixed
 * values:
 *	EnableCtrl = SAHPI_TRUE;
 *	EventCtrl  = SAHPI_SEC_PER_EVENT;
 *	DataFormat.IsSupported = SAHPI_TRUE 
 * 	DataFormat.ReadingType = SAHPI_SENSOR_READING_TYPE_UINT64
 *	DataFormat.BaseUnits   = SAHPI_SU_UNSPECIFIED
 *	DataFormat.ModifierUse = SAHPI_SMUU_NONE
 *	DataFormat.Percentage  = SAHPI_FALSE
 *	ThresholdDefn.IsAccessible = SAHPI_FALSE
 *
 * Return values:
 * SA_ERR_HPI_OUT_OF_MEMORY - memory allocation failed.
 * SA_ERR_HPI_INTERNAL_ERROR - could not add sensor RDR
 **/
static SaErrorT ilo2_ribcl_add_severity_sensor(
			struct oh_handler_state *oh_handler,
			struct oh_event *event,
			int sens_num,
			SaHpiSensorTypeT sens_type,
			SaHpiEventStateT supported_states,
			struct ilo2_ribcl_sensinfo *sens_info,
			char *description)
{

	SaErrorT ret = SA_OK;
	SaHpiRdrT *rdr;
	SaHpiSensorRecT *sensor_rec;
	struct ilo2_ribcl_sensinfo *si;

	rdr = (SaHpiRdrT *)g_malloc0(sizeof(SaHpiRdrT));
	if( rdr == NULL){
		err("ilo2_ribcl_add_severity_sensor: Memory allocation failed.");
		return(SA_ERR_HPI_OUT_OF_MEMORY);
	}

	/* Fill in generic RDR stuff */
	rdr->RdrType = SAHPI_SENSOR_RDR;
	rdr->Entity  = event->resource.ResourceEntity;
	rdr->IsFru   = SAHPI_FALSE;

	/* Fill in sensor specific info */
	sensor_rec = &(rdr->RdrTypeUnion.SensorRec);
	sensor_rec->Num = sens_num;
	sensor_rec->Type = sens_type;
	sensor_rec->Category = SAHPI_EC_SEVERITY;
	sensor_rec->EnableCtrl = SAHPI_TRUE;
	sensor_rec->EventCtrl  = SAHPI_SEC_PER_EVENT;
	sensor_rec->Events = supported_states;

	sensor_rec->DataFormat.IsSupported = SAHPI_TRUE;
	sensor_rec->DataFormat.ReadingType = SAHPI_SENSOR_READING_TYPE_UINT64;
	sensor_rec->DataFormat.BaseUnits   = SAHPI_SU_UNSPECIFIED;
	sensor_rec->DataFormat.ModifierUse = SAHPI_SMUU_NONE; 
	sensor_rec->DataFormat.Percentage  = SAHPI_FALSE; 
	/* Range and AccuracyFactor have been cleared by g_malloc0() */

	sensor_rec->ThresholdDefn.IsAccessible = SAHPI_FALSE; 

	oh_init_textbuffer(&(rdr->IdString));
	oh_append_textbuffer(&(rdr->IdString), description);


	/* Copy the private sensor data initial values into a new allocation
 	 * to be associated with this RDR. */
	si = g_memdup(sens_info, sizeof(struct ilo2_ribcl_sensinfo));
	if( si == NULL){
		g_free( rdr);
		err("ilo2_ribcl_add_severity_sensor: Memory allocation failed.");
		return(SA_ERR_HPI_OUT_OF_MEMORY);
	}

	ret = oh_add_rdr(oh_handler->rptcache, event->resource.ResourceId,
		rdr, si, 0); 
	if( ret != SA_OK){
		err("ilo2_ribcl_add_severity_sensor: could not add RDR. Error = %s.",
			oh_lookup_error(ret));
		g_free( si);
		g_free( rdr);
		return( SA_ERR_HPI_INTERNAL_ERROR);
	} else {
		event->rdrs = g_slist_append(event->rdrs, rdr);
	}

	return( SA_OK);

} /* end ilo2_ribcl_add_severity_sensor() */



/**
 * ilo2_ribcl_discover_chassis_sensors:
 * @oh_handler:  Handler data pointer.
 * @event: Pointer to event structure for chassis resource event.
 *
 * This routine will create RDRs on the chassis rpt entry for the following
 * three sensors, if they have been detected during a discovery operation.
 * These sensors correspond to the system's general health, and are created
 * from information given in the HEALTH_AT_AT_GLANCE stanza returned by the
 * GET_EMBEDDED_HEALTH RIBCL command.
 *
 * Sensor 1: System Fan Health 
 *	This sensor is of type SAHPI_FAN.
 * 	This sensor is of class SAHPI_EC_SEVERITY, and supports the severity
 *	states SAHPI_ES_OK, SAHPI_ES_MAJOR_FROM_LESS, 
 *	SAHPI_ES_MAJOR_FROM_CRITICAL, and SAHPI_ES_CRITICAL.
 *	Its reading values (int64) are:
 *		I2R_SEN_VAL_OK (0)		- RIBCL reports "Ok"
 *		I2R_SEN_VAL_DEGRADED (1)	- RIBCL reports "Degraded"
 *		I2R_SEN_VAL_FAILED (2)		- RIBCL reports "Failed"
 *
 * Sensor 2: System Temperature Health.
 *	This sensor is of type SAHPI_TEMPERATURE.
 * 	This sensor is of class SAHPI_EC_SEVERITY, and supports the severity
 *	states SAHPI_ES_OK, and  SAHPI_ES_CRITICAL.
 *	Its reading values (int64) are:
 *		I2R_SEN_VAL_OK (0)		- RIBCL reports "Ok"
 *		I2R_SEN_VAL_FAILED (2)		- RIBCL reports "Failed"
 *
 * Sensor 3: System Power Supply Health
 *	This sensor is of type SAHPI_POWER_SUPPLY.
 * 	This sensor is of class SAHPI_EC_SEVERITY, and supports the severity
 *	states SAHPI_ES_OK, SAHPI_ES_MAJOR_FROM_LESS, 
 *	SAHPI_ES_MAJOR_FROM_CRITICAL, and SAHPI_ES_CRITICAL.
 *	Its reading values (int64) are:
 *		I2R_SEN_VAL_OK (0)		- RIBCL reports "Ok"
 *		I2R_SEN_VAL_DEGRADED (1)	- RIBCL reports "Degraded"
 *		I2R_SEN_VAL_FAILED (2)		- RIBCL reports "Failed"
 *
 * Return values:
 * None
 **/
static void ilo2_ribcl_discover_chassis_sensors(
			struct oh_handler_state *oh_handler, 
			struct oh_event *event)
{

	SaErrorT ret = SA_OK;
	ilo2_ribcl_handler_t *ir_handler = NULL;
	struct ilo2_ribcl_sensinfo si_initial;
	I2R_SensorDataT *sensordat;

	ir_handler = (ilo2_ribcl_handler_t *) oh_handler->data;

	/* Look for the system fan health intication from the RIBCL
	 * HEALTH_AT_A_GLANCE stanza from GET_EMBEDDED_HEALTH */

	sensordat = 
		&(ir_handler->DiscoveryData.chassis_sensors[I2R_SEN_FANHEALTH]);
	if( sensordat->reading.intval != I2R_SEN_VAL_UNINITIALIZED){

		si_initial.sens_num = I2R_SEN_FANHEALTH;
		si_initial.sens_ev_state = SAHPI_ES_OK;
		si_initial.sens_enabled = SAHPI_TRUE;
		si_initial.sens_ev_enabled = SAHPI_TRUE;
		si_initial.sens_assertmask = I2R_SEVERITY_THREESTATE_EV;
		si_initial.sens_deassertmask = I2R_SEVERITY_THREESTATE_EV;
		si_initial.sens_value = I2R_SEN_VAL_UNINITIALIZED;

		ret =  ilo2_ribcl_add_severity_sensor( oh_handler, event,
			I2R_SEN_FANHEALTH, SAHPI_FAN,
			I2R_SEVERITY_THREESTATE_EV, &si_initial,
			I2R_SEN_FANHEALTH_DESCRIPTION);

		if( ret == SA_OK){
			sensordat->state = I2R_INITIAL; 
			sensordat->rid = event->resource.ResourceId;
			ilo2_ribcl_add_resource_capability( oh_handler,
			      event,
			      (SAHPI_CAPABILITY_RDR | SAHPI_CAPABILITY_SENSOR));

		} else {
			err("ilo2_ribcl_discover_chassis_sensors: Failed to set up fan health sensor."); 
		}
	}
	
	/* Look for the system temperature health intication from the RIBCL
	 * HEALTH_AT_A_GLANCE stanza from GET_EMBEDDED_HEALTH */

	sensordat = 
	       &(ir_handler->DiscoveryData.chassis_sensors[I2R_SEN_TEMPHEALTH]);
	if( sensordat->reading.intval != I2R_SEN_VAL_UNINITIALIZED){

		si_initial.sens_num = I2R_SEN_TEMPHEALTH;
		si_initial.sens_ev_state = SAHPI_ES_OK;
		si_initial.sens_enabled = SAHPI_TRUE;
		si_initial.sens_ev_enabled = SAHPI_TRUE;
		si_initial.sens_assertmask = I2R_SEVERITY_TWOSTATE_EV;
		si_initial.sens_deassertmask = I2R_SEVERITY_TWOSTATE_EV;
		si_initial.sens_value = I2R_SEN_VAL_UNINITIALIZED;

		ret =  ilo2_ribcl_add_severity_sensor( oh_handler, event,
			I2R_SEN_TEMPHEALTH, SAHPI_TEMPERATURE,
			I2R_SEVERITY_TWOSTATE_EV, &si_initial,
			I2R_SEN_TEMPHEALTH_DESCRIPTION);

		if( ret == SA_OK){
			sensordat->state = I2R_INITIAL; 
			sensordat->rid = event->resource.ResourceId;
			ilo2_ribcl_add_resource_capability( oh_handler,
			      event,
			      (SAHPI_CAPABILITY_RDR | SAHPI_CAPABILITY_SENSOR));
		} else {
			err("ilo2_ribcl_discover_chassis_sensors: Failed to set up temperature health sensor."); 
		}
	}

	/* Look for the system power supply health intication from the RIBCL
	 * HEALTH_AT_A_GLANCE stanza from GET_EMBEDDED_HEALTH */

	sensordat = 
	      &(ir_handler->DiscoveryData.chassis_sensors[I2R_SEN_POWERHEALTH]);
	if( sensordat->reading.intval != I2R_SEN_VAL_UNINITIALIZED){

		si_initial.sens_num = I2R_SEN_POWERHEALTH;
		si_initial.sens_ev_state = SAHPI_ES_OK;
		si_initial.sens_enabled = SAHPI_TRUE;
		si_initial.sens_ev_enabled = SAHPI_TRUE;
		si_initial.sens_assertmask = I2R_SEVERITY_THREESTATE_EV;
		si_initial.sens_deassertmask = I2R_SEVERITY_THREESTATE_EV;
		si_initial.sens_value = I2R_SEN_VAL_UNINITIALIZED;

		ret =  ilo2_ribcl_add_severity_sensor( oh_handler, event,
			I2R_SEN_POWERHEALTH, SAHPI_POWER_SUPPLY,
			I2R_SEVERITY_THREESTATE_EV, &si_initial,
			I2R_SEN_POWERHEALTH_DESCRIPTION);

		if( ret == SA_OK){
			sensordat->state = I2R_INITIAL; 
			sensordat->rid = event->resource.ResourceId;
			ilo2_ribcl_add_resource_capability( oh_handler,
			      event,
			      (SAHPI_CAPABILITY_RDR | SAHPI_CAPABILITY_SENSOR));
		} else {
			err("ilo2_ribcl_discover_chassis_sensors: Failed to set up power supply health sensor."); 
		}
	}

} /* end ilo2_ribcl_discover_chassis_sensors() */



/**
 * ilo2_ribcl_add_resource_capability:
 * @oh_handler:  Handler data pointer.
 * @event: 	 Pointer to event structure for the resource event.
 * @capability:	 Capabilitiy (or capabilities) to add to this resource.
 *
 * Add the new resource capability (or capabilities) given in the 'capability'
 * parameter to the resource in the event structure, which is presumably a
 * resource event. Also, look up the rpt entry already in our handler's
 * rptcache, and if it exists, add the resource capability there also.
 *
 * Return values:
 * None
 **/
void ilo2_ribcl_add_resource_capability( struct oh_handler_state *oh_handler, 
			struct oh_event *event,
			SaHpiCapabilitiesT capability)
{
	SaHpiRptEntryT *rpt;

	/* Set the capabilities of the resource in this event structure */
	event->resource.ResourceCapabilities |= capability;

	/* Now, just in case we've already performed a oh_add_resource()
	 * call to place this resource into our handler's rptcache, look
	 * it up using the entity path, and add the capability there, too. */

	rpt = oh_get_resource_by_ep( oh_handler->rptcache,
				     &(event->resource.ResourceEntity)); 

	if( rpt != NULL){
		rpt->ResourceCapabilities |= capability;
	}
	
} /* end ilo2_ribcl_add_resource_capability() */




#ifdef ILO2_RIBCL_SIMULATE_iLO2_RESPONSE
/**
 * ilo2_ribcl_getfile
 * @fname: The file name.
 * @buffer: Ptr for the destination buffer.
 * @bufsize: Size of the destination buffer.
 *
 * This function, intended for testing, will read the contents of file
 * 'fname' into the buffer pointed to by 'buffer'.
 *
 * Return values: 0 if Success, 1 otherwise.
 **/
static int ilo2_ribcl_getfile( char *fname, char *buffer, int bufsize)
{

	int fd;
	struct stat stbuf;
	int i;
	int rcount;

	if( (fd = open( fname, O_RDONLY)) == -1){
		err("ilo2_ribcl_getfile(): Open failed for file %s", fname);
		return( 1);
	}

	if( fstat( fd, &stbuf) != 0){
		err("ilo2_ribcl_getfile: Stat failed for file %s", fname);
		close(fd);
		return( 1);
	}

	if( (stbuf.st_size + 1) > bufsize){
		err("ilo2_ribcl_getfile(): File exceeds buffer by %ld bytes.",
			(stbuf.st_size + 1) - bufsize);
		close(fd);
		return( 1);
	}

	i = 0;
	while( (rcount = read( fd, &buffer[i], 1)) != 0){
		if( rcount == -1){
			err("ilo2_ribcl_getfile(): Read error at byte %d", i);
			close(fd);
			return( 1);
		}
		i++;

		/* If someone is writing to the file after we did the above
		 * fstat, we could overflow the buffer.
		 */
		if( i >= (bufsize -1)){
			break;
		}
	}
	buffer[i] = 0; /* Null terminate */

	close(fd);
	return( 0); /* Success */

} /* end ilo2_ribcl_getfile() */

#endif /* ILO2_RIBCL_SIMULATE_iLO2_RESPONSE */



/*****************************
	OpenHPI plug-in to iLO2 RIBCL plug-in ABI function mapping
*****************************/

void * oh_discover_resources (void *)
                __attribute__ ((weak, alias("ilo2_ribcl_discover_resources")));
