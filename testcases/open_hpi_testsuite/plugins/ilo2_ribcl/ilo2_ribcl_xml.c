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

/*****************************
 * This file implements the iLO2 RIBCL plug-in XML functionality and provides
 * functions to parse XML responses from RIBCL. Uses libxml2
 * 
*****************************/

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <sys/types.h>
#include <libxml/parser.h>
#include <libxml/xmlmemory.h>
#include <ilo2_ribcl.h>
#include <ilo2_ribcl_xml.h>
#include <ilo2_ribcl_cmnds.h>
#include <ilo2_ribcl_sensor.h>

/* forward declarations */
static xmlNodePtr ir_xml_find_node( xmlNodePtr, char *);
static int ir_xml_checkresults_doc( xmlDocPtr, char *);
static int ir_xml_scan_response( xmlNodePtr, char *);
static char *ir_xml_convert_buffer( char*, int *);
static xmlDocPtr ir_xml_doparse( char *);
static int ir_xml_scan_fans( ilo2_ribcl_handler_t *, xmlNodePtr);
static int ir_xml_record_fandata( ilo2_ribcl_handler_t *, char *, char *,
		char *, char *, char *);
static void ir_xml_scan_temperature( ilo2_ribcl_handler_t *, xmlNodePtr);
static void ir_xml_scan_firmware_revision(ilo2_ribcl_handler_t *, xmlNodePtr);
static int ir_xml_scan_vrm( ilo2_ribcl_handler_t *, xmlNodePtr);
static int ir_xml_record_vrmdata( ilo2_ribcl_handler_t *, char *, char *);
static int ir_xml_scan_power( ilo2_ribcl_handler_t *, xmlNodePtr);
static int ir_xml_record_psdata( ilo2_ribcl_handler_t *, char *, char *);
static int ir_xml_scan_health_at_a_glance( ilo2_ribcl_handler_t *, xmlNodePtr);
static int ir_xml_stat_to_reading( char *);
static int ir_xml_scan_smbios_1( ilo2_ribcl_handler_t *, xmlNodePtr);
static int ir_xml_scan_smbios_4( ilo2_ribcl_handler_t *, xmlNodePtr);
static int ir_xml_scan_smbios_17( ilo2_ribcl_handler_t *, xmlNodePtr, int *);
static int ir_xml_record_memdata( ilo2_ribcl_handler_t *, int *, char *, char *,
		char *);
static xmlChar *ir_xml_smb_get_value( char *, xmlNodePtr);
static int ir_xml_insert_logininfo( char *, int, char *, char *, char *);
static int ir_xml_extract_index( char *, char *, int);
static int ir_xml_replacestr( char **, char *);

/* Error return values for ir_xml_extract_index */
#define IR_NO_PREFIX	-1
#define IR_NO_INDEX	-2

/* array containing all the RIBCL xml command templates */
char *ir_xml_cmd_templates[] = {
	[IR_CMD_GET_SERVER_DATA]	ILO2_RIBCL_GET_SERVER_DATA,
	[IR_CMD_GET_HOST_POWER_STATUS]	ILO2_RIBCL_GET_HOST_POWER_STATUS,
	[IR_CMD_SET_HOST_POWER_ON]	ILO2_RIBCL_SET_HOST_POWER_ON,
	[IR_CMD_SET_HOST_POWER_OFF]	ILO2_RIBCL_SET_HOST_POWER_OFF,
	[IR_CMD_RESET_SERVER]		ILO2_RIBCL_RESET_SERVER,
	[IR_CMD_COLD_BOOT_SERVER]	ILO2_RIBCL_COLD_BOOT_SERVER,
	[IR_CMD_GET_UID_STATUS]		ILO2_RIBCL_GET_UID_STATUS,
	[IR_CMD_UID_CONTROL_OFF]	ILO2_RIBCL_UID_CONTROL_OFF,
	[IR_CMD_UID_CONTROL_ON]		ILO2_RIBCL_UID_CONTROL_ON,
	[IR_CMD_GET_HOST_POWER_SAVER_STATUS]	ILO2_RIBCL_GET_HOST_POWER_SAVER_STATUS,
	[IR_CMD_SET_HOST_POWER_SAVER_1]	ILO2_RIBCL_SET_HOST_POWER_SAVER_1,	
	[IR_CMD_SET_HOST_POWER_SAVER_2]	ILO2_RIBCL_SET_HOST_POWER_SAVER_2,	
	[IR_CMD_SET_HOST_POWER_SAVER_3]	ILO2_RIBCL_SET_HOST_POWER_SAVER_3,	
	[IR_CMD_SET_HOST_POWER_SAVER_4]	ILO2_RIBCL_SET_HOST_POWER_SAVER_4,
	[IR_CMD_GET_SERVER_AUTO_PWR]	ILO2_RIBCL_GET_SERVER_AUTO_PWR,
	[IR_CMD_SERVER_AUTO_PWR_YES]	ILO2_RIBCL_SERVER_AUTO_PWR_YES,
	[IR_CMD_SERVER_AUTO_PWR_NO]	ILO2_RIBCL_SERVER_AUTO_PWR_NO,
	[IR_CMD_SERVER_AUTO_PWR_15]	ILO2_RIBCL_SERVER_AUTO_PWR_15,
	[IR_CMD_SERVER_AUTO_PWR_30]	ILO2_RIBCL_SERVER_AUTO_PWR_30,
	[IR_CMD_SERVER_AUTO_PWR_45]	ILO2_RIBCL_SERVER_AUTO_PWR_45,
	[IR_CMD_SERVER_AUTO_PWR_60]	ILO2_RIBCL_SERVER_AUTO_PWR_60,
	[IR_CMD_SERVER_AUTO_PWR_RANDOM]	ILO2_RIBCL_SERVER_AUTO_PWR_RANDOM
		};

/**
 * ir_xml_parse_status
 * @ribcl_outbuf: Ptr to a buffer containing the raw output from any RIBCL cmd.
 * @ilostr: String to identify a particular iLO2 in error messages.
 *
 * Parses the output of any RIBCL command and examines the STATUS attributes
 * of all RIBCL sections. This is the routine to call if the RIBCL command
 * you're using returns no other data that you want to collect and you are
 * only interested in the command's status. 
 *
 * Return value: RIBCL_SUCCESS if all STATUS attributes in the document
 *		 have a value of zero, otherwise the first non zero value
 *		 encountered or -1 if the document has no STATUS attributes. 
 **/
int ir_xml_parse_status( char *ribcl_outbuf, char *ilostr)
{
	xmlDocPtr doc;
	int status;

	doc = ir_xml_doparse( ribcl_outbuf);
	if( doc == NULL){
		return( -1);
	}

	status = ir_xml_checkresults_doc( doc, ilostr);
	if( status != RIBCL_SUCCESS){
		err("ir_xml_parse_status(): Unsuccessful RIBCL status.");
	}

	xmlFreeDoc( doc);
	return( status);
	
} /* end ir_xml_parse_status() */



/**
 * ir_xml_parse_emhealth
 * @ir_handler: Ptr to this instance's custom handler.
 * @ribcl_outbuf: Ptr to the raw RIBCL output from the GET_EMBEDDED_HEALTH cmd
 *
 * Parses the output of the RIBCL GET_EMBEDDED_HEALTH command, and enters
 * data about the discovered resources into the DiscoveryData structure
 * within our private handler. The following resources are detected:
 * 	Fans
 * 	Temperature Sensors
 * 	Voltage Regulator Modules
 * 	Power Supplies. 
 *
 * Return value: RIBCL_SUCCESS on success, -1 if error.
 **/
int ir_xml_parse_emhealth( ilo2_ribcl_handler_t *ir_handler, char *ribcl_outbuf)
{

	xmlDocPtr doc;
	xmlNodePtr eh_data_node;

	/* Convert the RIBCL command output into a single XML document,
	 * and the use the libxml2 parser to create an XML doc tree. */

	doc = ir_xml_doparse( ribcl_outbuf);
	if( doc == NULL){
		return( -1);
	}

	/* Check all RESULTS sections in the output for a status value
	 * of zero, which indicates success. */

	if( ir_xml_checkresults_doc( doc, ir_handler->ilo2_hostport)
							      != RIBCL_SUCCESS){
		err("ir_xml_parse_emhealth(): Unsuccessful RIBCL status.");
		xmlFreeDoc( doc);
		return( -1);
	}

	/* Locate the GET_EMBEDDED_HEALTH_DATA subtree. It will contain
	 * the useful results of the command. */

	eh_data_node = ir_xml_find_node( xmlDocGetRootElement(doc),
					    "GET_EMBEDDED_HEALTH_DATA");

	if( eh_data_node == NULL){
		err("ir_xml_parse_emhealth(): GET_EMBEDDED_HEALTH_DATA element not found."); 
		xmlFreeDoc( doc);
		return( -1);
	}

	/* Extract data for fans */
	if( ir_xml_scan_fans( ir_handler, eh_data_node) != RIBCL_SUCCESS){
		xmlFreeDoc( doc);
		return( -1);
	}
	
	/* extract data for voltage reg modules */
	if( ir_xml_scan_vrm( ir_handler, eh_data_node) != RIBCL_SUCCESS){
		xmlFreeDoc( doc);
		return( -1);
	}

	/* extract data for power supplies */
	if( ir_xml_scan_power( ir_handler, eh_data_node) != RIBCL_SUCCESS){
		xmlFreeDoc( doc);
		return( -1);
	}

	/* Extract data for temp sensors */
	ir_xml_scan_temperature( ir_handler, eh_data_node);


	xmlFreeDoc( doc);
	return( RIBCL_SUCCESS);

} /* end ir_xml_parse_emhealth() */



/**
 * ir_xml_parse_discoveryinfo
 * @ir_handler: Ptr to this instance's custom handler.
 * @ribcl_outbuf: Ptr to a buffer containing raw RIBCL output from the cmds 
 *
 * Parses the combined output of the RIBCL GET_EMBEDDED_HEALTH
 * and GET_HOST_DATA commands. When you have a single buffer containing output
 * from both commands, this routine is more efficient than calling
 * ir_xml_parse_emhealth() and ir_xml_parse_hostdata() separately. Here, the
 * xml parser is only invoked once.
 *
 * Data about the discovered resources are entered into the DiscoveryData
 * structure within our private handler. The following resources are detected:
 * 	Product name and serial number
 * 	Memory Modules
 * 	CPUs
 * 	Fans
 * 	Temperature Sensors
 * 	Voltage Regulator Modules
 * 	Power Supplies. 
 *
 * Return value: RIBCL_SUCCESS on success, -1 if error.
 **/
int ir_xml_parse_discoveryinfo( ilo2_ribcl_handler_t *ir_handler,
				char *ribcl_outbuf)
{

	xmlDocPtr doc;
	xmlNodePtr h_node;
	xmlChar *typ;
	int mem_slotindex;
	int ret;

	/* Convert the RIBCL command output into a single XML document,
	 * and the use the libxml2 parser to create an XML doc tree. */

	doc = ir_xml_doparse( ribcl_outbuf);
	if( doc == NULL){
		return( -1);
	}

	/* Check all RESULTS sections in the output for a status value
	 * of zero, which indicates success. */

	if( ir_xml_checkresults_doc( doc, ir_handler->ilo2_hostport)
							     != RIBCL_SUCCESS){
		err("ir_xml_parse_discoveryinfo(): Unsuccessful RIBCL status.");
		xmlFreeDoc( doc);
		return( -1);
	}

	/* First, we parse the output from the GET_HOST_DATA
	 * RIBCL command */ 

	h_node = ir_xml_find_node( xmlDocGetRootElement(doc),
				    "GET_HOST_DATA");

	if( h_node == NULL){
		err("ir_xml_parse_discoveryinfo(): GET_HOST_DATA element not found."); 
		xmlFreeDoc( doc);
		return( -1);
	}

	/* Since we can have multiple processors and memory modules,
	 * each within their own SMBIOS_RECORD, we must iterate over
	 * all of them, rather that calling  ir_xml_find_node() */

	h_node = h_node->xmlChildrenNode;
	mem_slotindex = 1;

	while( h_node != NULL){

		if(!xmlStrcmp( h_node->name,
				(const xmlChar *)"SMBIOS_RECORD")){

			ret = RIBCL_SUCCESS;
			typ = xmlGetProp( h_node, (const xmlChar *)"TYPE");

			if( !xmlStrcmp( typ, (const xmlChar *)"1")){
				/* Scan type 1 node for product name */
				ret = ir_xml_scan_smbios_1( ir_handler,
								h_node);
			} else if( !xmlStrcmp( typ, (const xmlChar *)"4")){ 
				/* Scan type 4 node for processor info */
				ret = ir_xml_scan_smbios_4( ir_handler,
								h_node);
			} else if( !xmlStrcmp( typ, (const xmlChar *)"17")){
				/* Scan type 17 node for memory */
				ret = ir_xml_scan_smbios_17( ir_handler,
							     h_node,
							     &mem_slotindex);
			}

			if( ret != RIBCL_SUCCESS){
				xmlFreeDoc( doc);
				return( -1);
			}
			
		} /* end if name == "SMBIOS_RECORD" */

		h_node = h_node->next;

	} /* end while h_node != NULL */

	/* Now we parse the output from the GET_EMBEDDED_HEALTH_DATA
	 * RIBCL command */

	h_node = ir_xml_find_node( xmlDocGetRootElement(doc),
					    "GET_EMBEDDED_HEALTH_DATA");

	if( h_node == NULL){
		err("ir_xml_parse_discoveryinfo(): GET_EMBEDDED_HEALTH_DATA element not found."); 
		xmlFreeDoc( doc);
		return( -1);
	}

	/* Extract data for fans */
	if( ir_xml_scan_fans( ir_handler, h_node) != RIBCL_SUCCESS){
		xmlFreeDoc( doc);
		return( -1);
	}
	
	/* Extract data for voltage reg modules */
	if( ir_xml_scan_vrm( ir_handler, h_node) != RIBCL_SUCCESS){
		xmlFreeDoc( doc);
		return( -1);
	}

	/* Extract data for power supplies */
	if( ir_xml_scan_power( ir_handler, h_node) != RIBCL_SUCCESS){
		xmlFreeDoc( doc);
		return( -1);
	}

	/* Extract data for health_at_a_glance sensors */
	if( ir_xml_scan_health_at_a_glance( ir_handler, h_node)
							     != RIBCL_SUCCESS){
		xmlFreeDoc( doc);
		return( -1);
	}

	/* Extract data for temp sensors */
	ir_xml_scan_temperature( ir_handler, h_node);

	/* Extract firmware revision information */
	/* Now we parse the output from the GET_FW_VERSION RIBCL command */

	h_node = ir_xml_find_node( xmlDocGetRootElement(doc),
					    "GET_FW_VERSION");

	if( h_node == NULL){
		err("ir_xml_parse_discoveryinfo(): GET_FW_VERSION element not found."); 
		xmlFreeDoc( doc);
		return( -1);
	}
	ir_xml_scan_firmware_revision( ir_handler, h_node);

	xmlFreeDoc( doc);
	return( RIBCL_SUCCESS);

} /* end ir_xml_parse_discoveryinfo() */



/**
 * ir_xml_parse_hostdata
 * @ir_handler: Ptr to this instance's custom handler.
 * @ribcl_outbuf: Ptr to the raw RIBCL output from the GET_EMBEDDED_HEALTH cmd
 *
 * Parses the output of the RIBCL GET_EMBEDDED_HEALTH command, and enters
 * data about the discovered resources into the DiscoveryData structure
 * within our private handler. The following resources are detected:
 * 	Product name and serial number
 * 	Memory Modules
 * 	CPUs
 *
 * Return value: RIBCL_SUCCESS on success, -1 if error.
 **/
int ir_xml_parse_hostdata( ilo2_ribcl_handler_t *ir_handler, char *ribcl_outbuf)
{
	xmlDocPtr doc;
	xmlNodePtr hd_node;
	xmlChar *typ;
	int mem_slotindex;
	int ret;

	/* Convert the RIBCL command output into a single XML document,
	 * and the use the libxml2 parser to create an XML doc tree. */

	doc = ir_xml_doparse( ribcl_outbuf);
	if( doc == NULL){
		return( -1);
	}

	/* Check all RESULTS sections in the output for a status value
	 * of zero, which indicates success. */

	if( ir_xml_checkresults_doc( doc, ir_handler->ilo2_hostport)
							      != RIBCL_SUCCESS){
		err("ir_xml_parse_hostdata(): Unsuccessful RIBCL status.");
		xmlFreeDoc( doc);
		return( -1);
	}

	/* Locate the GET_HOST_DATA subtree. It will contain
	 * the useful results of the command. */

	hd_node = ir_xml_find_node( xmlDocGetRootElement(doc),
				    "GET_HOST_DATA");

	if( hd_node == NULL){
		err("ir_xml_parse_hostdata(): GET_HOST_DATA element not found."); 
		xmlFreeDoc( doc);
		return( -1);
	}

	/* Since we can have multiple processors and memory modules,
	 * each within their own SMBIOS_RECORD, we must iterate over
	 * all of them, rather that calling  ir_xml_find_node() */

	hd_node = hd_node->xmlChildrenNode;
	mem_slotindex = 1;

	while( hd_node != NULL){

		if(!xmlStrcmp( hd_node->name,
				(const xmlChar *)"SMBIOS_RECORD")){
	
			ret = RIBCL_SUCCESS;
			typ = xmlGetProp( hd_node, (const xmlChar *)"TYPE");

			if( !xmlStrcmp( typ, (const xmlChar *)"1")){
				/* Scan type 1 node for product name */
				ret = ir_xml_scan_smbios_1( ir_handler,
								hd_node);
			} else if( !xmlStrcmp( typ, (const xmlChar *)"4")){ 
				/* Scan type 4 node for processor info */
				ret = ir_xml_scan_smbios_4( ir_handler,
								hd_node);
			} else if( !xmlStrcmp( typ, (const xmlChar *)"17")){
				/* Scan type 17 node for memory */
				ret = ir_xml_scan_smbios_17( ir_handler,
							     hd_node,
							     &mem_slotindex);
			}

			if( ret != RIBCL_SUCCESS){
				xmlFreeDoc( doc);
				return( -1);
			}
			
		} /* end if name == "SMBIOS_RECORD" */

		hd_node = hd_node->next;

	} /* end while hd_node != NULL */

	xmlFreeDoc( doc);
	return( RIBCL_SUCCESS);

} /* end ir_xml_parse_hostdata() */



/**
 * ir_xml_parse_host_power_status
 * @ribcl_outbuf: Ptr to the raw RIBCL output from the GET_HOST_POWER_STATUS
 *		  cmd
 * @power_status: pointer to int to return the parsed power status.
 * @ilostr: String to identify a particular iLO2 in error messages.
 *
 * Parses the output of the RIBCL GET_HOST_POWER_STATUS command, and returns
 * current power status of the server.
 *
 * Return value: RIBCL_SUCCESS on success, -1 if error.
 **/
int ir_xml_parse_host_power_status(char *ribcl_outbuf, int *power_status,
			char *ilostr)
{
	xmlDocPtr doc;
	xmlNodePtr data_node;
	xmlChar *status = NULL;

	/* Convert the RIBCL command output into a single XML document,
	 * and the use the libxml2 parser to create an XML doc tree. */

	doc = ir_xml_doparse( ribcl_outbuf);
	if( doc == NULL){
		err("ir_xml_parse_host_power_status(): Null doc returned.");
		return( -1);
	}

	/* Check all RESULTS sections in the output for a status value
	 * of zero, which indicates success. */

	if( ir_xml_checkresults_doc( doc, ilostr) != RIBCL_SUCCESS){
		err("ir_xml_parse_host_power_status(): Unsuccessful RIBCL status.");
		xmlFreeDoc( doc);
		return( -1);
	}

	/* Locate the GET_HOST_POWER_STATUS subtree. It will contain
	 * the useful results of the command. */

	data_node = ir_xml_find_node( xmlDocGetRootElement(doc),
					    "GET_HOST_POWER");

	if( data_node == NULL){
		err("ir_xml_parse_host_power_status(): GET_HOST_POWER element not found."); 
		xmlFreeDoc( doc);
		return( -1);
	}

	status =  xmlGetProp( data_node, (const xmlChar *)"HOST_POWER");
	if(status == NULL) {
		err("ir_xml_parse_host_power_status(): HOST_POWER not found.");
		xmlFreeDoc( doc);
		return(-1);
	}
	if(xmlStrcmp(status, (const xmlChar *)"ON") == 0) {
		*power_status = ILO2_RIBCL_POWER_ON;
	}
	else if(xmlStrcmp(status, (const xmlChar *)"OFF") == 0) {
		*power_status = ILO2_RIBCL_POWER_OFF;
	}
	else {
		xmlFree( status);
		xmlFreeDoc( doc);
		err("ir_xml_parse_host_power_status(): Unkown power status."); 
		return(-1);
	}

	xmlFree( status);
	xmlFreeDoc( doc);
	return( RIBCL_SUCCESS);

} /* end ir_xml_parse_host_power_status() */

/**
 * ir_xml_parse_uid_status
 * @ribcl_outbuf: Ptr to the raw RIBCL output from the GET_UID_STATUS cmd
 * @uid_status: pointer to int to return the parsed uid status.
 * @ilostr: String to identify a particular iLO2 in error messages.
 *
 * Parses the output of the RIBCL GET_UID_STATUS command, and returns
 * current UID status.
 *
 * Return value: RIBCL_SUCCESS on success, -1 if error.
 **/
int ir_xml_parse_uid_status(char *ribcl_outbuf, int *uid_status,
			char *ilostr)
{
	xmlDocPtr doc;
	xmlNodePtr data_node;
	xmlChar *status = NULL;

	/* Convert the RIBCL command output into a single XML document,
	 * and the use the libxml2 parser to create an XML doc tree. */

	doc = ir_xml_doparse( ribcl_outbuf);
	if( doc == NULL){
		err("ir_xml_parse_uid_status(): Null doc returned.");
		return( -1);
	}

	/* Check all RESULTS sections in the output for a status value
	 * of zero, which indicates success. */

	if( ir_xml_checkresults_doc( doc, ilostr) != RIBCL_SUCCESS){
		err("ir_xml_parse_uid_status(): Unsuccessful RIBCL status.");
		xmlFreeDoc( doc);
		return( -1);
	}

	/* Locate the GET_UID_STATUS subtree. It will contain
	 * the useful results of the command. */

	data_node = ir_xml_find_node( xmlDocGetRootElement(doc),
					    "GET_UID_STATUS");

	if( data_node == NULL){
		err("ir_xml_parse_uid_status(): GET_UID_STATUS element not found."); 
		xmlFreeDoc( doc);
		return( -1);
	}

	status =  xmlGetProp( data_node, (const xmlChar *)"UID");
	if(status == NULL) {
		err("ir_xml_parse_uid_status(): UID not found.");
		xmlFreeDoc( doc);
		return(-1);
	}
	if(xmlStrcmp(status, (const xmlChar *)"ON") == 0) {
		*uid_status = ILO2_RIBCL_UID_ON;
	}
	else if(xmlStrcmp(status, (const xmlChar *)"OFF") == 0) {
		*uid_status = ILO2_RIBCL_UID_OFF;
	}
	else {
		xmlFree( status);
		xmlFreeDoc( doc);
		err("ir_xml_parse_uid_status(): Unknown UID status : %s",
			(char *)status);
		return(-1);
	}

	xmlFree( status);
	xmlFreeDoc( doc);
	return( RIBCL_SUCCESS);

} /* end ir_xml_parse_uid_status() */

/**
 * ir_xml_parse_power_saver_status
 * @ribcl_outbuf: Ptr to the raw RIBCL output from the 
 * GET_HOST_POWER_SAVER_STATUS cmd
 * @ps_status: pointer to int to return the parsed power saver status.
 * @ilostr: String to identify a particular iLO2 in error messages.
 *
 * Parses the output of the RIBCL GET_HOST_POWER_SAVER_STATUS command,
 * and returns current power saver status of the server.
 *
 * Return value: RIBCL_SUCCESS on success, -1 if error.
 **/
int ir_xml_parse_power_saver_status(char *ribcl_outbuf, int *ps_status,
			char *ilostr)
{
	xmlDocPtr doc;
	xmlNodePtr data_node;
	xmlChar *status = NULL;

	/* Convert the RIBCL command output into a single XML document,
	 * and the use the libxml2 parser to create an XML doc tree. */

	doc = ir_xml_doparse( ribcl_outbuf);
	if( doc == NULL){
		err("ir_xml_parse_power_saver_status(): Null doc returned.");
		return( -1);
	}

	/* Check all RESULTS sections in the output for a status value
	 * of zero, which indicates success. */

	if( ir_xml_checkresults_doc( doc, ilostr) != RIBCL_SUCCESS){
		err("ir_xml_parse_power_saver_status(): Unsuccessful RIBCL status.");
		xmlFreeDoc( doc);
		return( -1);
	}

	/* Locate the GET_HOST_POWER_SAVER subtree. It will contain
	 * the useful results of the command. */

	data_node = ir_xml_find_node( xmlDocGetRootElement(doc),
					    "GET_HOST_POWER_SAVER");

	if( data_node == NULL){
		err("ir_xml_parse_power_saver_status(): GET_HOST_POWER_SAVER element not found."); 
		xmlFreeDoc( doc);
		return( -1);
	}

	status =  xmlGetProp( data_node, (const xmlChar *)"HOST_POWER_SAVER");
	if(status == NULL) {
		err("ir_xml_parse_power_saver_status(): HOST_POWER_SAVER not found.");
		xmlFreeDoc( doc);
		return(-1);
	}
	if(xmlStrcmp(status, (const xmlChar *)"MIN") == 0) {
		*ps_status = ILO2_RIBCL_MANUAL_LOW_POWER_MODE;
	}
	else if(xmlStrcmp(status, (const xmlChar *)"OFF") == 0) {
		*ps_status = ILO2_RIBCL_MANUAL_OS_CONTROL_MODE;
	}
	else if(xmlStrcmp(status, (const xmlChar *)"AUTO") == 0) {
		*ps_status = ILO2_RIBCL_AUTO_POWER_SAVE_MODE;
	}
	else if(xmlStrcmp(status, (const xmlChar *)"MAX") == 0) {
		*ps_status = ILO2_RIBCL_MANUAL_HIGH_PERF_MODE;
	}
	else {
		xmlFree( status);
		xmlFreeDoc( doc);
		err("ir_xml_parse_power_saver_status(): Unkown Power Saver status."); 
		return(-1);
	}

	xmlFree( status);
	xmlFreeDoc( doc);
	return( RIBCL_SUCCESS);

} /* end ir_xml_parse_power_saver_status() */

/**
 * ir_xml_parse_auto_power_status
 * @ribcl_outbuf: Ptr to the raw RIBCL output from the 
 * GET_SERVER_AUTO_PWR cmd
 * @ps_status: pointer to int to return the parsed power saver status.
 * @ilostr: String to identify a particular iLO2 in error messages.
 *
 * Parses the output of the RIBCL GET_SERVER_AUTO_PWR command, and returns
 * current Auto Power status of the server.
 *
 * Return value: RIBCL_SUCCESS on success, -1 if error.
 **/
int ir_xml_parse_auto_power_status(char *ribcl_outbuf, int *ps_status,
			char *ilostr)
{
	xmlDocPtr doc;
	xmlNodePtr data_node;
	xmlChar *status = NULL;

	/* Convert the RIBCL command output into a single XML document,
	 * and the use the libxml2 parser to create an XML doc tree. */

	doc = ir_xml_doparse( ribcl_outbuf);
	if( doc == NULL){
		err("ir_xml_parse_auto_power_status(): Null doc returned.");
		return( -1);
	}

	/* Check all RESULTS sections in the output for a status value
	 * of zero, which indicates success. */

	if( ir_xml_checkresults_doc( doc, ilostr) != RIBCL_SUCCESS){
		err("ir_xml_parse_auto_power_status(): Unsuccessful RIBCL status.");
		xmlFreeDoc( doc);
		return( -1);
	}

	/* Locate the GET_SERVER_AUTO_PWR subtree. It will contain
	 * the useful results of the command. */

	data_node = ir_xml_find_node( xmlDocGetRootElement(doc),
					    "SERVER_AUTO_PWR");

	if( data_node == NULL){
		err("ir_xml_parse_auto_power_status(): SERVER_AUTO_PWR element not found."); 
		xmlFreeDoc( doc);
		return( -1);
	}

	status =  xmlGetProp( data_node, (const xmlChar *)"VALUE");
	if(status == NULL) {
		err("ir_xml_parse_auto_power_status(): VALUE not found.");
		xmlFreeDoc( doc);
		return(-1);
	}
	if(xmlStrcmp(status, (const xmlChar *)"No") == 0) {
		*ps_status = ILO2_RIBCL_AUTO_POWER_DISABLED;
	}
	else if(xmlStrcmp(status, (const xmlChar *)"Yes") == 0) {
		*ps_status = ILO2_RIBCL_AUTO_POWER_ENABLED;
	}
	else if(xmlStrcmp(status, (const xmlChar *)"15") == 0) {
		*ps_status = ILO2_RIBCL_AUTO_POWER_DELAY_15;
	}
	else if(xmlStrcmp(status, (const xmlChar *)"30") == 0) {
		*ps_status = ILO2_RIBCL_AUTO_POWER_DELAY_30;
	}
	else if(xmlStrcmp(status, (const xmlChar *)"45") == 0) {
		*ps_status = ILO2_RIBCL_AUTO_POWER_DELAY_45;
	}
	else if(xmlStrcmp(status, (const xmlChar *)"60") == 0) {
		*ps_status = ILO2_RIBCL_AUTO_POWER_DELAY_60;
	}
	else if(xmlStrcmp(status, (const xmlChar *)"RANDOM") == 0) {
		*ps_status = ILO2_RIBCL_AUTO_POWER_DELAY_RANDOM;
	}
	else {
		xmlFree( status);
		xmlFreeDoc( doc);
		err("ir_xml_parse_auto_power_status(): Unkown Power Saver status."); 
		return(-1);
	}

	xmlFree( status);
	xmlFreeDoc( doc);
	return( RIBCL_SUCCESS);
}

/**
 * ir_xml_parse_reset_server
 * @ribcl_outbuf: Ptr to the raw RIBCL output from the RESET_SERVER or
 * COLD_BOOT_SERVER command. It just looks for error conditions and nothing
 * specific about either command output.
 * cmd
 * @ilostr: String to identify a particular iLO2 in error messages.
 *
 * Parses the output of the RIBCL RESET_SERVER or COLD_BOOT_SERVER command.
 *
 * Return value: RIBCL_SUCCESS on success, -1 if error.
 **/
int ir_xml_parse_reset_server(char *ribcl_outbuf, char *ilostr)
{
	xmlDocPtr doc;

	/* Convert the RIBCL command output into a single XML document,
	 * and the use the libxml2 parser to create an XML doc tree. */

	doc = ir_xml_doparse( ribcl_outbuf);
	if( doc == NULL){
		return( -1);
	}

	/* Check all RESULTS sections in the output for a status value
	 * of zero, which indicates success. */

	if( ir_xml_checkresults_doc( doc, ilostr) != RIBCL_SUCCESS){
		err("ir_xml_parse_reset_server(): Unsuccessful RIBCL status.");
		xmlFreeDoc( doc);
		return( -1);
	}

	xmlFreeDoc( doc);
	return( RIBCL_SUCCESS);

} /* end ir_xml_parse_reset_server() */


/**
 * ir_xml_parse_set_host_power 
 * @ribcl_outbuf: Ptr to the raw RIBCL output from the SET_HOST_POWER
 * cmd
 * @ilostr: String to identify a particular iLO2 in error messages.
 *
 * Parses the output of the RIBCL SET_HOST_POWER command.
 *
 * Return value: RIBCL_SUCCESS on success, -1 if error.
 **/
int ir_xml_parse_set_host_power(char *ribcl_outbuf, char *ilostr)
{
	xmlDocPtr doc;

	/* Convert the RIBCL command output into a single XML document,
	 * and the use the libxml2 parser to create an XML doc tree. */

	doc = ir_xml_doparse( ribcl_outbuf);
	if( doc == NULL){
		return( -1);
	}

	/* Check all RESULTS sections in the output for a status value
	 * of zero, which indicates success. */

	if( ir_xml_checkresults_doc( doc, ilostr) != RIBCL_SUCCESS){
		err("ir_xml_parse_set_host_power(): Unsuccessful RIBCL status.");
		xmlFreeDoc( doc);
		return( -1);
	}

	xmlFreeDoc( doc);
	return( RIBCL_SUCCESS);

} /* end ir_xml_parse_set_host_power() */


/**
 * ir_xml_build_cmdbufs
 * @handler:  ptr to the ilo2_ribcl plugin handler.
 *
 * This routine is designed to be called at ilo2_ribcl plugin open time.
 * Iterate through the global RIBCL command template array
 * ir_xml_cmd_templates[]. For each command template string in that array,
 * allocate a buffer and fill it with a customized XML command string for
 * that host, containing the user_name and password that have been previously
 * read from the config file and saved in our private handler.
 *
 * Note that the command templates in ir_xml_cmd_templates[] have been 
 * created as *printf format strings with "%s" where the login and password
 * should be substituted to assign the LOGIN and PASSWORD attributes.
 *
 * These customized xml command strings are then stored in the ribcl_xml_cmd[]
 * array in the handler for this plugin instance.
 *
 * Note: you must free these allocated buffers at close time with a call to
 * ir_xml_free_cmdbufs().
 *
 * Return value: RIBCL_SUCCESS if success, -1 if failure.
 **/
int ir_xml_build_cmdbufs( ilo2_ribcl_handler_t *handler)
{

	int i;
	int cmdsize;
	int logpasslen;
	char *login;
	char *password;

	for( i =0; i < IR_NUM_COMMANDS; i++){
		handler->ribcl_xml_cmd[i] = NULL;
	}
	
	login = handler->user_name;
	password = handler->password;

	logpasslen = strlen( login) + strlen( password);

	for( i =0; i < IR_NUM_COMMANDS; i++){
		cmdsize = strlen( ir_xml_cmd_templates[i]) - 3 + logpasslen;
		handler->ribcl_xml_cmd[i] = malloc( cmdsize);
		if( handler->ribcl_xml_cmd[i] == NULL){
			err("ir_xml_build_cmdbufs(): malloc of command buffer %d failed.", i);
			while( --i >=0){
				free( handler->ribcl_xml_cmd[i]);
			}
			return( -1);
		}

		/* Note, since the OpenHPI makefiles set the GCC options
		 * -Werror and -Wformat-nonliteral, we can't use anything
		 * except a string literal  as the third "format" parameter
		 * to snprintf() without generating errors at compile time.
		 * So, we must use our own stripped down  version of snprintf()
		 * instead. 
		 */
		ir_xml_insert_logininfo( handler->ribcl_xml_cmd[i], cmdsize,
			 ir_xml_cmd_templates[i], login, password);
	}

	return( RIBCL_SUCCESS);

} /* end ir_xml_build_cmdbufs() */

/**
 * ir_xml_insert_logininfo
 * @dest - ptr to destination buffer for the customized RIBCL command string.
 * @dsize - size of the dest buffer, including the terminating null.
 * @format - A *printf format string containing at least two %s substitutions.
 * @login  - the login string for this host.
 * @passwd - the password string for this host.
 *
 * This is a stripped down version of the standard snprintf() routine.
 * It is designed to insert the login and password strings into a RIBCL
 * command template given by the 'format' parameter. The resulting customized
 * command string is written to the null terminated buffer 'dest'.
 *
 * Return value: The number of characters written to the dest buffer, not
 *		 including the terminating null (just like snprintf()) if
 *		 success, -1 if failure.
 **/
static int ir_xml_insert_logininfo( char *dest, int dsize, char *format,
			 char *login, char *passwd)
{

	enum istates { COPY, INSERT_LOGIN, INSERT_PASSWD, COPY_POST };
	int dcount = 0;
	enum istates state;
	char ch;
	int login_entered = 0;
	
	state = COPY;

	while( dcount < dsize){

		switch( state){

		case COPY:
			if( (*format == '%') && ( (*(format +1)) == 's')){
				format += 2;
				if( login_entered){
					state = INSERT_PASSWD;
				} else {
					state = INSERT_LOGIN;
				}
			} else {
				/* Copy from format to dest */
				ch = *dest++ = *format++;
				if( ch == '\0'){
					/* We're done */
					return( dcount);
				}
				dcount++;
			}
			break;

		case INSERT_LOGIN:
			login_entered = 1;
			if( *login != '\0'){
				*dest++ = *login++;
				dcount++;
			} else {
				state = COPY;
			}
			break;

		case INSERT_PASSWD:
			if( *passwd != '\0'){
				*dest++ = *passwd++;
				dcount++;
			} else {
				state = COPY_POST;
			}
			break;

		case COPY_POST:
			/* Copy from format to dest */
			ch = *dest++ = *format++;
			if( ch == '\0'){
				/* We're done */
				return( dcount);
			}
			dcount++;
			break;

		default:
			err("ir_xml_insert_logininfo(): Illegal state.");
			return ( -1);
			break;

		} /* end switch state */

	} /* end while dcount < dsize */

	/* If we make it here, then we've run out of destination buffer space,
	 * so force a null termination to the destination buffer and return the
	 * number of non-null chars.
	 */

	*(dest -1) = '\0';
	return( dcount - 1);

} /* end ir_xml_insert_logininfo() */

/**
 * ir_xml_free_cmdbufs
 * @handler:  ptr to the ilo2_ribcl plugin private handler.
 *
 * This routine is desigend to be called at ilo2_ribcl plugin close time.
 * Iterate through the ribcl_xml_cmd[] array in the ilo2_ribcl plugin
 * handler passed as a parameter, and free all of the customized RIBCL
 * command strings. 
 *
 * Return value: None.
 **/
void ir_xml_free_cmdbufs( ilo2_ribcl_handler_t *handler)
{
	int i;

	for( i =0; i < IR_NUM_COMMANDS; i++){
		if( handler->ribcl_xml_cmd[i] != NULL){
			free( handler->ribcl_xml_cmd[i]);
			handler->ribcl_xml_cmd[i] = NULL;
		}
	}
	return;
}

/**
 * ir_xml_find_node
 * @s_node: Starting XML tree node for the search.
 * @s_name: Name of the node to search for.
 *
 * Search the XML doc subtree pointed to by s_node, searching for a node
 * with the name s_name.
 *
 * Return value: Ptr to a XML doc tree node on success, NULL on failure.
 **/
static xmlNodePtr ir_xml_find_node( xmlNodePtr s_node, char *s_name)
{
	xmlNodePtr c_node;

	while( s_node != NULL){
		if((!xmlStrcmp( s_node->name, (const xmlChar *)s_name))){
			return( s_node);
		} else {
			c_node = ir_xml_find_node( s_node->xmlChildrenNode,
						      s_name);
			if( c_node != NULL){
				return( c_node);
			}
		}
		
		s_node = s_node->next;
	}
	return( NULL);

} /* End ir_xml_find_node( ) */

/**
 * ir_xml_scan_fans
 * @ir_handler: Ptr to this instance's custom handler.
 * @eh_data_node: Points to GET_EMBEDDED_HEALTH subtree in the output xml
 *
 * Examines all the FAN subtrees of the FANS tree, and extracts the fan label,
 * zone, status, speed, and speed units for all fans in the system.
 *
 * The fan XML section should look something like:
 * <FANS>
 * 	<FAN>
 *		<LABEL VALUE = "Fan 1"/>
 *		<ZONE VALUE = "I/O Board"/>
 *		<STATUS VALUE = "Ok"/>
 *		<SPEED VALUE = "50" UNIT="Percentage"/>
 *	</FAN>
 * 	<FAN>
 *		<LABEL VALUE = "Fan 2"/>
 *		<ZONE VALUE = "I/O Board"/>
 *		<STATUS VALUE = "Ok"/>
 *		<SPEED VALUE = "50" UNIT="Percentage"/>
 *	</FAN>
 *	<FAN>
 *		<LABEL VALUE = "Fan 3"/>
 *		<ZONE VALUE = "CPU"/>
 *		<STATUS VALUE = "Ok"/>
 *		<SPEED VALUE = "40" UNIT="Percentage"/>
 *	</FAN>
 *	<FAN>
 *		<LABEL VALUE = "Fan 4"/>
 *		<ZONE VALUE = "CPU"/>
 *		<STATUS VALUE = "Ok"/>
 *		<SPEED VALUE = "40" UNIT="Percentage"/>
 *	</FAN>
 *	<FAN>
 *		<LABEL VALUE = "Fan 5"/>
 *		<ZONE VALUE = "CPU"/>
 *		<STATUS VALUE = "Ok"/>
 *		<SPEED VALUE = "40" UNIT="Percentage"/>
 *	</FAN>
 *	<FAN>
 *		<LABEL VALUE = "Fan 6"/>
 *		<ZONE VALUE = "CPU"/>
 *		<STATUS VALUE = "Ok"/>
 *		<SPEED VALUE = "40" UNIT="Percentage"/>
 *	</FAN>
 * </FANS>
 *
 * Return value: RIBCL_SUCCESS if success, -1 if failure.
 **/
static int ir_xml_scan_fans( ilo2_ribcl_handler_t *ir_handler,
		       xmlNodePtr eh_data_node)
{
	xmlNodePtr fan_node;
	xmlNodePtr n;
	xmlChar *lbl = NULL;
	xmlChar *zone = NULL;
	xmlChar *stat = NULL;
	xmlChar *speed = NULL;
	xmlChar *unit = NULL;
	int ret;

	fan_node = ir_xml_find_node( eh_data_node, "FANS");

	fan_node = fan_node->xmlChildrenNode;
	while( fan_node != NULL){
		if((!xmlStrcmp( fan_node->name, (const xmlChar *)"FAN"))){

			n =  ir_xml_find_node(fan_node, "LABEL");
			if( n != NULL){
				lbl = xmlGetProp( n, (const xmlChar *)"VALUE");
			}

			n = ir_xml_find_node(fan_node, "ZONE");
			if( n != NULL){
				zone = xmlGetProp( n, (const xmlChar *)"VALUE");
			}

			n = ir_xml_find_node(fan_node, "STATUS");
			if( n != NULL){
				stat = xmlGetProp( n, (const xmlChar *)"VALUE");
			}

			n = ir_xml_find_node(fan_node, "SPEED");
			if( n != NULL){
				speed = xmlGetProp( n, (const xmlChar *)"VALUE");
				unit =  xmlGetProp( n, (const xmlChar *)"UNIT");
			}

			ret = ir_xml_record_fandata( ir_handler, (char *)lbl,
				(char *)zone, (char *)stat, (char *)speed,
				(char *)unit);

			if( lbl){
				xmlFree( lbl);
			}
			if( zone){
				xmlFree( zone);
			}
			if( stat){
				xmlFree( stat);
			}
			if( speed){
				xmlFree( speed);
			}
			if( unit){
				xmlFree( unit);
			}

			if( ret != RIBCL_SUCCESS){
				return( -1);
			}

		} /* end if name == "FAN" */

		fan_node = fan_node->next;

	} /* end while fan_node != NULL */

	return( RIBCL_SUCCESS);

} /* end ir_xml_scan_fans() */

/**
 * ir_xml_record_fandata
 * @ir_handler:	ptr to the ilo2_ribcl plugin private handler.
 * @fanlabel: 	string label for the fan from iLO2.
 * @fanzone:	string location zone for the fan from iLO2.
 * @fanstat:	string status for the fan from iLO2.
 * @fanspeed:	string speed for the fan from iLO2.
 * @speedunits:	string units for the fan speed.
 *
 * This routine records the data for a fan reported by iLO2 into the
 * DiscoveryData structure within the plugin's private handler.
 *
 * Detailed description:
 *
 * 	- Extract an index for this fan from the fan label provided by iLO2.
 *	- Use this index to access the DiscoveryData.fandata[] array. 
 *	- Set the IR_DISCOVERED bit in fanflags.
 *	- if the fan speed differs from our previous reading, set the
 *	  IR_SPEED_UPDATED flag in fanflags.
 *	- Store updated values for the speed, label, zone, status, and
 *	  speedunit for this fan.  
 *
 * Return value: RIBCL_SUCCESS if success, -1 if failure. 
 **/
static int ir_xml_record_fandata( ilo2_ribcl_handler_t *ir_handler,
			char *fanlabel, char *fanzone, char *fanstat,
			char *fanspeed, char *speedunits)
{

	int fanindex = 0;
	int speedval;

	struct ir_fandata *fandat;

	/* Find the index of this fan. The parameter 'fanlabel' should be of the
	 * form 'Fan N', where N ranges from 1 to ILO2_RIBCL_DISCOVER_FAN_MAX.
	 */

	fanindex = ir_xml_extract_index("Fan", (char *)fanlabel, 1);
	if( fanindex == IR_NO_PREFIX){
		/* We didn't parse the fanlabel string prefix correctly */
		err("ir_xml_record_fandata: incorrect fan label string: %s",
			fanlabel);
		return( -1);
	}

	if( fanindex == IR_NO_INDEX){
		/* We didn't parse the fanlabel string index correctly */
		err("ir_xml_record_fandata: could not extract index from fan label string: %s",
			fanlabel);
		return( -1);
	}

	/* The index of this fan should be between 1 and
	 * ILO2_RIBCL_DISCOVER_FAN_MAX.
	 */
	
	if( (fanindex < 1) || (fanindex > ILO2_RIBCL_DISCOVER_FAN_MAX) ){
		err("ir_xml_record_fandata: Fan index out of range: %d.",
			fanindex);
		return( -1);
	}

	/* Now, if this information is different than what's already in
         * DiscoveryData, update DiscoveryData with the latest info */

	fandat = &(ir_handler->DiscoveryData.fandata[fanindex]);
	fandat->fanflags |= IR_DISCOVERED;

	speedval = atoi(fanspeed);
	if( fandat->speed != speedval){
		fandat->fanflags != IR_SPEED_UPDATED;
		fandat->speed = speedval;
	}

	if( ir_xml_replacestr( &(fandat->label), fanlabel) != RIBCL_SUCCESS){
		return( -1);
	}

	if( ir_xml_replacestr( &(fandat->zone), fanzone) != RIBCL_SUCCESS){
		return( -1);
	}
	
	if( ir_xml_replacestr( &(fandat->status), fanstat) != RIBCL_SUCCESS){
		return( -1);
	}

	if( ir_xml_replacestr( &(fandat->speedunit), speedunits) !=
							 RIBCL_SUCCESS){
		return( -1);
	}

	return( RIBCL_SUCCESS);

} /* end ir_xml_record_fandata() */ 

/**
 * ir_xml_scan_temperature
 * @ir_handler: Ptr to this instance's custom handler.
 * @eh_data_node: Points to GET_EMBEDDED_HEALTH subtree in the output xml
 *
 * Examines all the TEMP subtrees of the TEMPERATURE tree, and extracts the
 * label, location, status, current reading, and temperature units for all
 * temperature sensors in the system.
 *
 * The temperature XML section should look something like:
 * <TEMPERATURE>
 * 	<TEMP>
 *		<LABEL VALUE = "Temp 1"/>
 *		<LOCATION VALUE = "Ambient"/>
 *		<STATUS VALUE = "Ok"/>
 *		<CURRENTREADING VALUE = "20" UNIT="Celsius"/>
 *		<CAUTION VALUE = "39" UNIT="Celsius"/>
 *		<CRITICAL VALUE = "44" UNIT="Celsius"/>
 *	</TEMP>
 *	<TEMP>
 *		<LABEL VALUE = "Temp 2"/>
 *		<LOCATION VALUE = "CPU 1"/>
 *		<STATUS VALUE = "Ok"/>
 *		<CURRENTREADING VALUE = "38" UNIT="Celsius"/>
 *		<CAUTION VALUE = "95" UNIT="Celsius"/>
 *		<CRITICAL VALUE = "100" UNIT="Celsius"/>
 *	</TEMP>
 *	<TEMP>
 *		<LABEL VALUE = "Temp 3"/>
 *		<LOCATION VALUE = "CPU 2"/>
 *		<STATUS VALUE = "n/a"/>
 *		<CURRENTREADING VALUE = "n/a" UNIT="n/a"/>
 *		<CAUTION VALUE = "95" UNIT="Celsius"/>
 *		<CRITICAL VALUE = "100" UNIT="Celsius"/>
 *	</TEMP>
 *	<TEMP>
 *		<LABEL VALUE = "Temp 4"/>
 *		<LOCATION VALUE = "Power Supply"/>
 *		<STATUS VALUE = "Ok"/>
 *		<CURRENTREADING VALUE = "38" UNIT="Celsius"/>
 *		<CAUTION VALUE = "60" UNIT="Celsius"/>
 *		<CRITICAL VALUE = "65" UNIT="Celsius"/>
 *	</TEMP>
 *	<TEMP>
 *		<LABEL VALUE = "Temp 5"/>
 *		<LOCATION VALUE = "CPU"/>
 *		<STATUS VALUE = "Ok"/>
 *		<CURRENTREADING VALUE = "47" UNIT="Celsius"/>
 *		<CAUTION VALUE = "65" UNIT="Celsius"/>
 *		<CRITICAL VALUE = "70" UNIT="Celsius"/>
 *	</TEMP>
 *	<TEMP>
 *		<LABEL VALUE = "Temp 6"/>
 *		<LOCATION VALUE = "I/O Board"/>
 *		<STATUS VALUE = "Ok"/>
 *		<CURRENTREADING VALUE = "44" UNIT="Celsius"/>
 *		<CAUTION VALUE = "66" UNIT="Celsius"/>
 *		<CRITICAL VALUE = "71" UNIT="Celsius"/>
 *	</TEMP>
 * </TEMPERATURE>
 *
 * Return value: None
 **/
static void ir_xml_scan_temperature( ilo2_ribcl_handler_t *ir_handler,
			      xmlNodePtr eh_data_node)
{
	xmlNodePtr t_node;
	xmlNodePtr n;
	xmlChar *lbl = NULL;
	xmlChar *loc = NULL;
	xmlChar *stat = NULL;
	xmlChar *cur_reading = NULL;
	xmlChar *unit = NULL;

	t_node = ir_xml_find_node( eh_data_node, "TEMPERATURE");

	t_node = t_node->xmlChildrenNode;
	while( t_node != NULL){
		if((!xmlStrcmp( t_node->name, (const xmlChar *)"TEMP"))){

			n =  ir_xml_find_node( t_node, "LABEL");
			if( n != NULL){
				lbl = xmlGetProp( n, (const xmlChar *)"VALUE");
			}

			n =  ir_xml_find_node( t_node, "LOCATION");
			if( n != NULL){
				loc = xmlGetProp( n, (const xmlChar *)"VALUE");
			}

			n =  ir_xml_find_node( t_node, "STATUS");
			if( n != NULL){
				stat = xmlGetProp( n, (const xmlChar *)"VALUE");
			}

			n =  ir_xml_find_node( t_node, "CURRENTREADING");
			if( n != NULL){
				cur_reading = xmlGetProp( n, (const xmlChar *)"VALUE");
				unit = xmlGetProp( n, (const xmlChar *)"UNIT");
			}

			/* Extract Caution value here, if needed */

			/* Call to process temperature data goes here, when
			 * we add support to this plugin for sensors. */

			if( lbl){
				xmlFree( lbl);
			}
			if( loc){
				xmlFree( loc);
			}
			if( stat){
				xmlFree( stat);
			}
			if( cur_reading){
				xmlFree( cur_reading);
			}
			if( unit){
				xmlFree( unit);
			}

		} /* end if name == "TEMP" */

		t_node = t_node->next;

	} /* end while t_node != NULL */

} /* end  ir_xml_scan_temperature() */

/**
 * ir_xml_firmaware_revision
 * @ir_handler: Ptr to this instance's custom handler.
 * @eh_data_node: Points to GET_FW_VERSION subtree in the output xml
 * This routine parses the firmware information and saves major and minor
 * revision numbers in the handler DiscoverData structure.
 * The response for GET_FW_VERSION RIBCL command should look something like:
 * <GET_FW_VERSION
 *  FIRMWARE_VERSION = "1.30"
 *  FIRMWARE_DATE    = "Jun 01 2007"
 *  MANAGEMENT_PROCESSOR    = "iLO2"
 *  /> </RIBCL>
 *
 * Return value: None
 **/
static void ir_xml_scan_firmware_revision( ilo2_ribcl_handler_t *ir_handler,
			      xmlNodePtr eh_data_node)
{
	xmlChar *fw_ver = NULL;
	char *result = NULL;
	SaHpiUint8T FirmwareMajorRev = 0;
        SaHpiUint8T FirmwareMinorRev = 0;

	fw_ver =  xmlGetProp(eh_data_node, (const xmlChar *)"FIRMWARE_VERSION");
	if(fw_ver == NULL) {
		err("ir_xml_scan_firmware_revision(): FIRMWARE_VERSION not found.");
		return;
	}

	/* Save a copy of the raw version string */
	ir_xml_replacestr( &(ir_handler->DiscoveryData.fwdata.version_string),
				 (char *)fw_ver);

	/* extract manjor and minor revision numbers and save them in the
	   handler discovery data structure */
	FirmwareMajorRev = atoi((char *) fw_ver);
	result = strchr((char *)fw_ver, '.');
	if(result != NULL) {
		FirmwareMinorRev = atoi(++result);
	}
	if(ir_handler->DiscoveryData.fwdata.FirmwareMajorRev != 
		FirmwareMajorRev) {
		ir_handler->DiscoveryData.fwdata.FirmwareMajorRev = 
			FirmwareMajorRev;
	}
	if(ir_handler->DiscoveryData.fwdata.FirmwareMinorRev != 
		FirmwareMinorRev) {
		ir_handler->DiscoveryData.fwdata.FirmwareMinorRev = 
			FirmwareMinorRev;
	}
	return;
}

/**
 * ir_xml_scan_vrm
 * @eh_data_node: Points to GET_EMBEDDED_HEALTH subtree in the output xml
 * @ir_handler: Ptr to this instance's custom handler.
 *
 * Examines all the MODULE subtrees of the VRM tree, and extracts the
 * label and status, for all voltage regulator modules in the system.
 *
 * The VRM XML section should look something like:
 * <VRM>
 * 	<MODULE>
 *		<LABEL VALUE = "VRM 1"/>
 *		<STATUS VALUE = "Ok"/>
 *	</MODULE>
 * 	<MODULE>
 *		<LABEL VALUE = "VRM 2"/>
 *		<STATUS VALUE = "Not Installed"/>
 *	</MODULE>
 * </VRM>
 *
 * Return value: RIBCL_SUCCESS if success, -1 if failure. 
 **/
static int ir_xml_scan_vrm( ilo2_ribcl_handler_t *ir_handler,
		 xmlNodePtr eh_data_node)
{
	xmlNodePtr v_node;
	xmlNodePtr n;
	int ret;
	xmlChar *lbl = NULL;
	xmlChar *stat = NULL;

	v_node = ir_xml_find_node( eh_data_node, "VRM");

	v_node = v_node->xmlChildrenNode;
	while( v_node != NULL){
		if((!xmlStrcmp( v_node->name, (const xmlChar *)"MODULE"))){

			ret = RIBCL_SUCCESS;
			n =  ir_xml_find_node( v_node, "LABEL");
			if( n != NULL){
				lbl = xmlGetProp( n, (const xmlChar *)"VALUE");
			}

			n =  ir_xml_find_node( v_node, "STATUS");
			if( n != NULL){
				stat = xmlGetProp( n, (const xmlChar *)"VALUE");
			}

			if( xmlStrcmp(stat, (xmlChar *)"Not Installed") != 0 ){
				ret = ir_xml_record_vrmdata( ir_handler,
						(char *)lbl, (char *)stat);
			}


			if( lbl){
				xmlFree( lbl);
			}
			if( stat){
				xmlFree( stat);
			}

			if( ret != RIBCL_SUCCESS){
				return( -1);
			}

		} /* end if name == "MODULE" */

		v_node = v_node->next;

	} /* end while v_node != NULL */

	return( RIBCL_SUCCESS);

} /* end ir_xml_scan_vrm() */



/**
 * ir_xml_record_vrmdata
 * @ir_handler:	ptr to the ilo2_ribcl plugin private handler.
 * @label:  string VRM label from iLO2.
 * @psstat: string VRM status from iLO2.
 *
 * This routine records the data for a voltage regulator module reported by
 * iLO2 into the DiscoveryData structure within the plugin's private handler.
 *
 * Detailed description:
 *
 * 	- Extract an index for this VRM from the VRM label provided by iLO2.
 *	- Use this index to access the DiscoveryData.vrmdata[] array. 
 *	- Set the IR_DISCOVERED bit in vrmflags.
 *	- Store updated values for the label and status for this VRM.  
 *
 * Return value: RIBCL_SUCCESS if success, -1 if failure. 
 **/
static int ir_xml_record_vrmdata( ilo2_ribcl_handler_t *ir_handler,
			char *vrmlabel, char *vrmstat)
{
	int vrmindex = 0;
	int ret;
	ir_vrmdata_t *vrmdat;

	/* The format of the VRM label is 'VRM n' where n is an integer
	 * value beginning with 1. We use this value as an index for
	 * the VRM. */

	ret = sscanf( (char *)vrmlabel, "VRM %d", &vrmindex);
	if( ret != 1){
		/* We didn't parse the VRM label correctly */
		err("ir_xml_record_vrmdata: incorrect VRM label string: %s",
			vrmlabel);
		return( -1);
	}

	/* The index for this VRM should be between 1 and
	 * ILO2_RIBCL_DISCOVER_VRM_MAX. */

	if( (vrmindex < 1) || (vrmindex > ILO2_RIBCL_DISCOVER_PSU_MAX) ){
		err("ir_xml_record_vrmdata: VRM index out of range: %d",
			vrmindex);
		return( -1);
	}

	/* Now, if this information is different than what's already in
	 DiscoveryData, update DiscoveryData with the latest info */

	vrmdat = &(ir_handler->DiscoveryData.vrmdata[vrmindex]);
	vrmdat->vrmflags |= IR_DISCOVERED;

	if( ir_xml_replacestr( &(vrmdat->label), vrmlabel) != RIBCL_SUCCESS){
		return( -1);
	}

	if( ir_xml_replacestr( &(vrmdat->status), vrmstat) != RIBCL_SUCCESS){
		return( -1);
	}

	return( RIBCL_SUCCESS);

} /* end ir_xml_record_vrmdata() */



/**
 * ir_xml_scan_power
 * @ir_handler: Ptr to this instance's custom handler.
 * @eh_data_node: Points to GET_EMBEDDED_HEALTH subtree in the output xml
 *
 * Examines all the SUPPLY subtrees of the  POWER_SUPPLIES tree, and extracts
 * the label and status, for all power supplies in the system.
 *
 * The XML data for POWER_SUPPLIES should look something like:
 * <POWER_SUPPLIES>
 *	<SUPPLY>
 *		<LABEL VALUE = "Power Supply 1"/>
 *		<STATUS VALUE = "Ok"/>
 *	</SUPPLY>
 *	<SUPPLY>
 *		<LABEL VALUE = "Power Supply 2"/>
 *		<STATUS VALUE = "Failed"/>
 *	</SUPPLY>
 * </POWER_SUPPLIES>
 *
 * Return value: RIBCL_SUCCESS if success, -1 if failure.
 **/
static int ir_xml_scan_power( ilo2_ribcl_handler_t *ir_handler,
				xmlNodePtr eh_data_node)
{
	xmlNodePtr p_node;
	xmlNodePtr n;
	int ret;
	xmlChar *lbl = NULL;
	xmlChar *stat = NULL;

	p_node = ir_xml_find_node( eh_data_node, "POWER_SUPPLIES");

	p_node = p_node->xmlChildrenNode;
	while( p_node != NULL){
		if((!xmlStrcmp( p_node->name, (const xmlChar *)"SUPPLY"))){

			ret = RIBCL_SUCCESS;
			n =  ir_xml_find_node( p_node, "LABEL");
			if( n != NULL){
				lbl = xmlGetProp( n, (const xmlChar *)"VALUE");
			}

			n =  ir_xml_find_node( p_node, "STATUS");
			if( n != NULL){
				stat = xmlGetProp( n, (const xmlChar *)"VALUE");
			}

			/* XXX REVISIT - confirm that "Not Installed" is the
			 * correct string when the power supply is removed. */

			if( xmlStrcmp( stat, (xmlChar *)"Not Installed") != 0 ){	
				ret = ir_xml_record_psdata( ir_handler,
						(char *)lbl, (char *)stat);
			}

			if( lbl){
				xmlFree( lbl);
			}
			if( stat){
				xmlFree( stat);
			}

			if( ret != RIBCL_SUCCESS){
				return( -1);
			}

		} /* end if name == "SUPPLY" */

		p_node = p_node->next;

	} /* end while p_node != NULL */

	return( RIBCL_SUCCESS);

} /* end ir_xml_scan_power() */



/**
 * ir_xml_record_psdata 
 * @ir_handler:	ptr to the ilo2_ribcl plugin private handler.
 * @label:  string power supply label from iLO2.
 * @psstat: string power supply status from iLO2.
 *
 * This routine records the data for a power supply reported by
 * iLO2 into the DiscoveryData structure within the plugin's private handler.
 *
 * Detailed description:
 *
 * 	- Extract an index for this power supply from the power supply label
 *	  provided by iLO2.
 *	- Use this index to access the DiscoveryData.psudata[] array. 
 *	- Set the IR_DISCOVERED bit in psuflags.
 *	- Store updated values for the label,and status for this power supply.  
 *
 * Return value: RIBCL_SUCCESS if success, -1 if failure. 
 **/
static int ir_xml_record_psdata( ilo2_ribcl_handler_t *ir_handler,
			 char *pslabel, char *psstat)
{

	int psindex = 0;
	int ret;
	ir_psudata_t *psudat;

	/* The format of a the power supply label is 'Power Supply n' where
	 * n is an integer value beginning with 1. We use this value as
	 * an index for the power supply. */

	ret = sscanf( (char *)pslabel, "Power Supply %d", &psindex);
	if( ret != 1){
		/* We didn't parse the power supply label correctly */
		err("ir_xml_record_psdata: incorrect PSU label string: %s",
			pslabel);
		return( -1);
	}

	/* The index for this power supply should be between 1 and
	 * ILO2_RIBCL_DISCOVER_PSU_MAX. */
	
	if( (psindex < 1) || (psindex > ILO2_RIBCL_DISCOVER_PSU_MAX) ){
		err("ir_xml_record_psdata: PSU index out of range: %d.",
			psindex);
		return( -1);
	}

	/* Now, if this information is different than what's already in
	 * DiscoveryData, update DiscoveryData with the latest info */

	psudat = &(ir_handler->DiscoveryData.psudata[psindex]);
	psudat->psuflags |= IR_DISCOVERED;

	if( ir_xml_replacestr( &(psudat->label), pslabel) != RIBCL_SUCCESS){
		return( -1);
	}
	  
	if( ir_xml_replacestr( &(psudat->status), psstat) != RIBCL_SUCCESS){
		return( -1);
	}


	return( RIBCL_SUCCESS);

} /* end ir_xml_record_psdata() */



/**
 * ir_xml_scan_health_at_a_glance
 * @ir_handler: Ptr to this instance's custom handler.
 * @eh_data_node: Points to GET_EMBEDDED_HEALTH subtree in the output xml
 *
 * Examines the HEALTH_AT_A_GLANCE subtree within GET_EMBEDDED_HEALTH,
 * and extracts the status for fans, temperature, and power supply.
 * The status string values are then parsed into numeric values and
 * stored in the chassis_sensors[] element within the DiscoveryData element
 * located within the plugin's private handler.
 *
 * The XML data for HEALTH_AT_A_GLANCE should look something like:
 * <HEALTH_AT_A_GLANCE>
 *      <FANS STATUS= "Ok"/>
 *      <FANS REDUNDANCY= "Fully Redundant"/>
 *      <TEMPERATURE STATUS= "Ok"/>
 *      <VRM STATUS= "Ok"/>
 *      <POWER_SUPPLIES STATUS= "Ok"/>
 *      <POWER_SUPPLIES REDUNDANCY= "Not Redundant"/>
 *</HEALTH_AT_A_GLANCE>
 *
 * Return value: RIBCL_SUCCESS if success, -1 if failure.
 **/
static int ir_xml_scan_health_at_a_glance( ilo2_ribcl_handler_t *ir_handler,
		xmlNodePtr eh_data_node)
{
	xmlNodePtr p_node;
	int sens_reading;
	xmlChar *ts;
	xmlChar *fstat = NULL;
	xmlChar *pstat = NULL;
	xmlChar *tstat = NULL;
	ilo2_ribcl_DiscoveryData_t *ddata;
	I2R_SensorDataT *sens_dat;
	
	p_node = ir_xml_find_node( eh_data_node, "HEALTH_AT_A_GLANCE");

	if( p_node == NULL){
		return( RIBCL_SUCCESS);	/* Not supported */
	}

	/* Note, we assume the properties that we are looking for can occur
	 * in any order, and inbetween properties that we are not looking for.
	 * So, we use the temporary pointer 'ts' to capture them.
	 */
	p_node = p_node->xmlChildrenNode;
	while( p_node != NULL){

		if((!xmlStrcmp( p_node->name, (const xmlChar *)"FANS"))){
			ts = xmlGetProp( p_node, (const xmlChar *)"STATUS"); 
			if( ts != NULL){
				fstat = ts;
			}
		}

		if((!xmlStrcmp( p_node->name, (const xmlChar *)"TEMPERATURE"))){
			ts = xmlGetProp( p_node, (const xmlChar *)"STATUS"); 
			if( ts != NULL){
				tstat = ts;
			}
		}

		if((!xmlStrcmp( p_node->name,
					   (const xmlChar *)"POWER_SUPPLIES"))){
			ts = xmlGetProp( p_node, (const xmlChar *)"STATUS"); 
			if( ts != NULL){
				pstat = ts;
			}
		}

		p_node = p_node->next;
	}

	ddata = &(ir_handler->DiscoveryData);

	if( fstat){
		sens_dat = &(ddata->chassis_sensors[I2R_SEN_FANHEALTH]);
		sens_reading = ir_xml_stat_to_reading( (char *)fstat);

		if( sens_reading == -1){
			err("ir_xml_scan_health_at_a_glance: Unrecognized status value \"%s\" for fan health.",
				fstat);
		} else {
			sens_dat->reading.intval = sens_reading;
		}
		
		xmlFree( fstat);
	}

	if( tstat){
		sens_dat = &(ddata->chassis_sensors[I2R_SEN_TEMPHEALTH]);
		sens_reading = ir_xml_stat_to_reading( (char *)tstat);

		if( (sens_reading == -1) ||
		    (sens_reading == I2R_SEN_VAL_DEGRADED) ){
			err("ir_xml_scan_health_at_a_glance: Unrecognized status value \"%s\" for temperature health.",
				tstat);
		} else {
			sens_dat->reading.intval = sens_reading;
		}

		xmlFree( tstat);
	}

	if( pstat){
		sens_dat = &(ddata->chassis_sensors[I2R_SEN_POWERHEALTH]);
		sens_reading = ir_xml_stat_to_reading( (char *)pstat);

		if( sens_reading == -1){
			err("ir_xml_scan_health_at_a_glance: Unrecognized status value \"%s\" for power supply health.",
				pstat);
		} else {
			sens_dat->reading.intval = sens_reading;
		}

		xmlFree( pstat);
	}

	return( RIBCL_SUCCESS);

} /* end ir_xml_scan_health_at_a_glance() */



/**
 * ir_xml_stat_to_reading
 * @statstr: Ptr to a status string
 *
 * Performs a case insentive comparison of the input status string
 * with known possible values, and returns a numeric representation.
 * The value -1 is returned if there is no match for the string.
 *
 * Return values:
 * I2R_SEN_VAL_OK - status string was "Ok"
 * I2R_SEN_VAL_DEGRADED - status string was "Degraded"
 * I2R_SEN_VAL_FAILED - status string was "Failed"
 * -1 - The string was not recognized.
 **/
static int ir_xml_stat_to_reading( char *statstr)
{

	if( !strcasecmp( statstr, "Ok")){
		return( I2R_SEN_VAL_OK);
	} else if( !strcasecmp( statstr, "Degraded")){
		return( I2R_SEN_VAL_DEGRADED);
	} else if( !strcasecmp( statstr, "Failed")){
		return( I2R_SEN_VAL_FAILED);
	} else {
		return( -1);
	}

} /* end ir_xml_stat_to_reading() */



/**
 * ir_xml_scan_smbios_1
 * @ir_handler:	ptr to the ilo2_ribcl plugin private handler.
 * @b_node: porinter to an SMBIOS_RECORD node of type 1.
 *
 * This routine will parse a SMBIOS_RECORD type 1 node, which should contain
 * the system product name and serial number. If these values differ from the
 * product_name and serial_number recorded in the DiscoveryData structure
 * within the plugin's private handler, then update the DiscoveryData values.
 *
 * A type 1 SMBIOS_RECORD should look something like:
 * <SMBIOS_RECORD TYPE="1" B64_DATA="XXXX">
 * 	<FIELD NAME="Subject" VALUE="System Information"/>
 *	<FIELD NAME="Product Name" VALUE="ProLiant DL385 G2"/>
 *	<FIELD NAME="Serial Number" VALUE="USE718NAJL      "/>
 *	<FIELD NAME="UUID" VALUE="414109USE718NAJL"/>
 *</SMBIOS_RECORD>
 *
 * Return value: RIBCL_SUCCESS if success, -1 if failure.
 **/
static int ir_xml_scan_smbios_1( ilo2_ribcl_handler_t *ir_handler,
			xmlNodePtr b_node)
{

	xmlChar *prod = NULL;
	xmlChar *sernum = NULL;
	int ret;

	/* We need to search this node's children for FIELD nodes
	 * containing name and value pairs for the system product name and
	 * the serial number.  */

	b_node = b_node->xmlChildrenNode; 
	prod   = ir_xml_smb_get_value( "Product Name", b_node);
	sernum = ir_xml_smb_get_value( "Serial Number", b_node);

	/* If it's different than information already in the DiscoveryData of
	 * our handler, then replace it. */ 

	ret = ir_xml_replacestr( &(ir_handler->DiscoveryData.product_name),
		(char *)prod);

	if( ret == RIBCL_SUCCESS){
		ret = ir_xml_replacestr(
			&(ir_handler->DiscoveryData.serial_number),
			(char *)sernum);
	}

	if( prod){
		xmlFree( prod);
	}
	if( sernum){
		xmlFree( sernum);
	}

	return( ret);

} /* end ir_xml_scan_smbios_1() */



/**
 * ir_xml_scan_smbios_4
 * @ir_handler: Ptr to this instance's custom handler.
 * @b_node: porinter to an SMBIOS_RECORD node of type 4.
 *
 * This routine will parse a SMBIOS_RECORD type 4 node, which should contain
 * CPU information. We currently extract only the label information.
 *
 * A type 4 SMBIOS_RECORD for processor 1 should look something like:
 * <SMBIOS_RECORD TYPE="4" B64_DATA="XXXX">
 *	<FIELD NAME="Subject" VALUE="Processor Information"/>
 *	<FIELD NAME="Label" VALUE="Proc 1"/>
 *	<FIELD NAME="Speed" VALUE="1800 MHz"/>
 *	<FIELD NAME="Execution Technology" VALUE="2 of 2 cores; 2 threads"/>
 *	<FIELD NAME="Memory Technology" VALUE="64-bit extensions"/>
 *</SMBIOS_RECORD>
 *
 * A type 4 SMBIOS_RECORD for subsequent processors should look something like:
 * <SMBIOS_RECORD TYPE="4" B64_DATA="XXXX">
 *	<FIELD NAME="Subject" VALUE="Processor Information"/>
 *	<FIELD NAME="Label" VALUE="Proc 2"/>
 *	<FIELD NAME="Status" VALUE="unavailable"/>
 *</SMBIOS_RECORD>

 * Detailed description:
 *
 * 	- Extract an index for this CPU from the CPU label provided by iLO2.
 *	- Use this index to access the DiscoveryData.cpudata[] array. 
 *	- Set the IR_DISCOVERED bit in cpuflags.
 *	- Store updated values for the label for this CPU.  
 *
 * Return value: RIBCL_SUCCESS if success, -1 if failure.
 **/
static int ir_xml_scan_smbios_4( ilo2_ribcl_handler_t *ir_handler,
			xmlNodePtr b_node)
{

	xmlChar *cpu = NULL;
	xmlChar *cpu_speed = NULL;
	int ret;
	int procnum = 0;

	/* We need to search this node's children for FIELD nodes
	 * containing name and value pairs for the CPUs */ 

	b_node = b_node->xmlChildrenNode;
	cpu = ir_xml_smb_get_value( "Label", b_node);

	/* Can also get the speed, execution technology, and memory
	 * technology for the CPU. However, this data is only provided
	 * for processor 1. */

	cpu_speed = ir_xml_smb_get_value( "Speed", b_node);

	/* Find the index of this processor. The label returned in string
	 * 'cpu' above should be of the form 'Proc N'.
	 */

	ret = sscanf( (char *)cpu, "Proc %d", &procnum);
	if( ret != 1){
		/* We didn't parse the cpu string correctly */
		err("ir_xml_scan_smbios_4: incorrect CPU string: %s", cpu);
		return( -1);
	}

	/* The index of this processor should be between 1 and 
	 * ILO2_RIBCL_DISCOVER_CPU_MAX.
	 */
	
	if( (procnum < 1) || (procnum > ILO2_RIBCL_DISCOVER_CPU_MAX) ){
		err("ir_xml_scan_smbios_4: Proc index out of range: %d.",
			procnum);
		return( -1);
	}

	ir_handler->DiscoveryData.cpudata[ procnum].cpuflags |= IR_DISCOVERED;

	/* Now, if this information is different than what's already in
	 * DiscoveryData, update DiscoveryData with the latest info */

	ret = ir_xml_replacestr(
			&(ir_handler->DiscoveryData.cpudata[ procnum].label),
			(char *)cpu);

	/* Since we only have cpu speed reported for processor one, we make
	 * the assumption that all processors are of the same rated speed.
	 * Probably a safe assumption, since we are talking about Symetric
	 * Multi-Processing here after all. */

	if( (ret == RIBCL_SUCCESS) && ( cpu_speed != NULL)){
		ret = ir_xml_replacestr( 
				&(ir_handler->DiscoveryData.system_cpu_speed),
				(char *)cpu_speed);
	}

	if( cpu){	
		xmlFree(cpu);
	}

	if( cpu_speed){
		 xmlFree(cpu_speed);
	}

	return( ret);

} /* end ir_xml_scan_smbios_4() */



/**
 * ir_xml_scan_smbios_17
 * @ir_handler: Ptr to this instance's custom handler.
 * @b_node: porinter to an SMBIOS_RECORD node of type 17.
 * @mem_slotindex: pointer to an integer value for the current slot index.
 *
 * This routine will parse a SMBIOS_RECORD type 17 node, which should contain
 * memory DIMM information. We currently extract the label, size and speed.
 * Then, a call is made to ir_xml_record_memdata() to enter this data into
 * the DiscoveryData structure within the plugin's private handler.
 *
 * A type 17 SMBIOS_RECORD should look something like:
 * <SMBIOS_RECORD TYPE="17" B64_DATA="XXXX">
 * 	<FIELD NAME="Subject" VALUE="Memory Device"/>
 *	<FIELD NAME="Label" VALUE="DIMM 1A"/>
 *	<FIELD NAME="Size" VALUE="1024 MB"/>
 *	<FIELD NAME="Speed" VALUE="667 MHz"/>
 * </SMBIOS_RECORD>
 *
 * Return value: RIBCL_SUCCESS if success, -1 if failure.
 **/
static int ir_xml_scan_smbios_17( ilo2_ribcl_handler_t *ir_handler,
			xmlNodePtr b_node,
			int *mem_slotindex)
{

	xmlChar *mem_label = NULL;
	xmlChar *mem_size = NULL;
	xmlChar *mem_speed = NULL;
	int ret = RIBCL_SUCCESS;

	b_node = b_node->xmlChildrenNode;
	mem_label  = ir_xml_smb_get_value( "Label", b_node);
	mem_size   = ir_xml_smb_get_value( "Size", b_node);
	mem_speed  = ir_xml_smb_get_value( "Speed", b_node);
	
	if( xmlStrcmp( mem_size, (xmlChar *)"not installed") != 0 ){

		/* Record data for a populated DIMM slot */
		ret = ir_xml_record_memdata( ir_handler, mem_slotindex,
			(char *)mem_label,
			(char *)mem_size, (char *) mem_speed);


	}

	/* Increment the slot index for both populated and unpopulated slots */
	(*mem_slotindex)++;

	if( mem_label){
		xmlFree( mem_label);
	}
	if( mem_size){
		xmlFree( mem_size);
	}
	if( mem_speed){
		xmlFree( mem_speed);
	}

	return( ret);

} /* end ir_xml_scan_smbios_17() */



/**
 * ir_xml_record_memdata
 * @ir_handler:	ptr to the ilo2_ribcl plugin private handler.
 * @memcount:   ptr to an int that contains a mem slot count.
 * @memlabel:	string memory DIMM label from iLO2.
 * @memsize:	string memory DIMM size from iLO2.
 * @memspeed:	string memory DIMM speed from iLO2.
 *
 * This routine records the data for a memory DIMM reported by
 * iLO2 into the DiscoveryData structure within the plugin's private handler.
 *
 * Detailed description:
 *
 * 	- Extract an index for this DIMM from the DIMM label provided by iLO2.
 *	- Use this index to access the DiscoveryData.memdata[] array. 
 *	- Set the IR_DISCOVERED bit in memflags.
 *	- Store updated values for the label, size, and speed for this DIMM.  
 *
 * Return value: RIBCL_SUCCESS if success, -1 if failure. 
 **/
static int ir_xml_record_memdata( ilo2_ribcl_handler_t *ir_handler,
			int *memcount,
			char *memlabel, char *memsize, char *memspeed)
{
	int dimmindex = 0;
	int procindex = 0;
	int ret;
	ir_memdata_t *dimmdat;

	/* The format of a the memory label is 'DIMM nX' where n is an
	 * integer and X is a capital letter. The integer value is
	 * unique and begins at one, so we use this value as an index
	 * for the DIMM. */

	ret = sscanf( (char *)memlabel, "DIMM %d", &dimmindex);
	if( ret != 1){

		/* Try for an alternate format. We will also accept a label
		 * of the format 'PROC k DIMM nX' where k and n are integers
		 * and X is a capital letter. Since n may not be unique across
		 * all DIMMS, we will use the memcount parameter as the index
		 * for the DIMM. */

		ret = sscanf( (char*)memlabel, "PROC %d DIMM %d", &procindex,
			      &dimmindex);

		/* In this case, we use the memory slot count passed to us via
		 * parameter memcount as the memory index. */
		dimmindex = *memcount;  

		if( ret != 2){

			/* We didn't parse the DIMM label correctly */
			err("ir_xml_record_memdata: incorrect DIMM label string: %s",
			memlabel);
			return( -1);
		}
	}

	/* The index for this DIMM should be between 1 and
	 * ILO2_RIBCL_DISCOVER_MEM_MAX. */
	
	if( (dimmindex < 1) || (dimmindex > ILO2_RIBCL_DISCOVER_MEM_MAX) ){
		err("ir_xml_record_memdata: DIMM index out of range: %d.",
			dimmindex);
		return( -1);
	}

	/* Now, if this information is different than what's already in
	 * DiscoveryData, update DiscoveryData with the latest info */

	dimmdat = &(ir_handler->DiscoveryData.memdata[dimmindex]);
	dimmdat->memflags |= IR_DISCOVERED;

	if( ir_xml_replacestr( &(dimmdat->label), memlabel) != RIBCL_SUCCESS){
		return( -1);
	}
	  
	if( ir_xml_replacestr( &(dimmdat->memsize), memsize) != RIBCL_SUCCESS){
		return( -1);
	}

	if( ir_xml_replacestr( &(dimmdat->speed), memspeed) != RIBCL_SUCCESS){
		return( -1);
	}

	return( RIBCL_SUCCESS);

} /* end ir_xml_record_memdata() */



/**
 * ir_xml_smb_get_value
 * @name - name property string to match
 * @fld_node - doc subtree node containing one or more FIELD nodes
 *
 * This routine will scan all node siblings starting with fld_node,
 * Looking for FIELD nodes. If we find an atrirbute NAME on the FIELD
 * node with a value that matches the 'name' parameter, we return the
 * string value of the VALUE attribute for that node. Since this paragraph
 * doesn't make much sense, here's an example:
 *
 * <SMBIOS_RECORD>
 *	<SOMEOTHER_TAG/>
 *	<FIELD NAME="Moe" VALUE="Stooge 1"/>
 *	<FIELD NAME="Larry" VALUE="Stooge 2"/>
 *	<FIELD NAME="Curly" VALUE="Stooge 3"/>
 *	<FIELD NAME="Shemp" VALUE="Stooge 4"/>
 *	<YETANOTHER_TAG/>
 * </SMBIOS_RECORD> 
 *
 * Using the above XML with a name parameter of "Curly" would return the
 * string "Stooge 3". 
 *
 * Return value:
 * String value of the VALUE attribute if the NAME attrubute is matched,
 * NULL if no matches found.
 **/
static xmlChar *ir_xml_smb_get_value( char *name, xmlNodePtr fld_node)
{
	xmlChar *smbnam = NULL;
	
	while( fld_node != NULL){

		if((!xmlStrcmp( fld_node->name, (const xmlChar *)"FIELD"))){

			smbnam = xmlGetProp( fld_node, (const xmlChar *)"NAME");
			if( smbnam){
				if(!xmlStrcmp( smbnam, (xmlChar *)name)){
					xmlFree( smbnam);
					return( xmlGetProp( fld_node,
						 (const xmlChar *)"VALUE"));
				}
				xmlFree( smbnam);
			} /* end if smbnam */

		} /* end if name == "FIELD" */

		fld_node = fld_node->next;

	} /* end while fld_node != NULL */

	return( NULL);

} /* end ir_xml_smb_get_value() */



/**
 * ir_xml_checkresults_doc
 * @doc: Ptr to XML tree results obtained from a RIBCL command output.
 * @ilostr: String to identify a particular iLO2 in error messages.
 *
 * Scans the XML tree for all occurences of a RIBCL node, and for each,
 * examines the STATUS attribute of the RESULTS.
 *
 * Return value: RIBCL_SUCCESS if all STATUS attributes in the document
 *		 have a value of zero, otherwise the first non zero value
 *		 encountered or -1 if the document has no STATUS attributes. 
 **/
int ir_xml_checkresults_doc( xmlDocPtr doc, char *ilostr){

	xmlNodePtr cur_node;
	int ribcl_status;
	int successful;

	cur_node = xmlDocGetRootElement(doc);

	if( cur_node == NULL){
		err("ir_xml_checkresults_doc(): XML document has no root.");
		return( -1);
	}

	successful = 0;
	
	/* Get the list of childern nodes from the root */
	cur_node = cur_node->xmlChildrenNode;

	while( cur_node != NULL){
		if((!xmlStrcmp( cur_node->name, (const xmlChar *)"RIBCL"))){
			ribcl_status = ir_xml_scan_response( cur_node, ilostr);
			if( ribcl_status != RIBCL_SUCCESS){
				return( ribcl_status);
			} else {
				successful = 1;
			}
		}
		cur_node = cur_node->next;
	}

	if( successful){
		return( RIBCL_SUCCESS);
	} else {
		return( -1);
	}

} /* end ir_xml_checkresults_doc() */



/**
 * ir_xml_scan_response
 * @RIBCLnode
 * ilostring
 *
 * Examines the RIBCL node for a RESPONSE section, and then returns the value
 * of the STATUS propery.
 *
 * If the STATUS property in the RESPONSE section is not equal to the value
 * zero (RIBCL_SUCCESS), then log any text in the MESSAGE property as an
 * error message. The string passed in parameter 'ilostring' will be
 * incorporated into the error message so each iLO2's messages can be
 * identified if multiple instances are in use. 
 *
 * Return value: Integer value of the STATUS property on success, -1 on failure.
 **/
static int ir_xml_scan_response( xmlNodePtr RIBCLnode, char *ilostring)
{
	xmlNodePtr resp_node;
	xmlChar *statprop;
	xmlChar *errmes;
	int ret_stat = RIBCL_SUCCESS;
	
	/* Parameter RIBCLnode should point to a RIBCL node in the RIBCL xml
	 * output document. */

	resp_node = RIBCLnode->xmlChildrenNode;
	while( resp_node != NULL){

		if((!xmlStrcmp( resp_node->name, (const xmlChar *)"RESPONSE"))){
			statprop = xmlGetProp( resp_node, (const xmlChar *)"STATUS");
			if( statprop != NULL){
				ret_stat = (int)( strtol( (char *)statprop,
						 NULL, 0));
				xmlFree( statprop);
			}

			/* Log the error message from iLO2 */
			if( ret_stat != RIBCL_SUCCESS){
				errmes = xmlGetProp( resp_node,
						(const xmlChar *)"MESSAGE");
				if( errmes){
					/* this condition indicates the 
					   requested setting is not supported
					   on the platform. For example
					   SET_HOST_POWER_SAVER 
					   HOST_POWER_SAVER="4" is not a
					   supported value on a DL385 G2.
					   Return RIBCL_UNSUPPORTED to the
					   calling routine. */

					if(xmlStrcmp(errmes,
						(const xmlChar *)"The value specified is invalid.") == 0) {
						ret_stat = RIBCL_UNSUPPORTED;
					}
					err("Error from iLO2 at %s : %s.",
						ilostring, (char *)errmes);
					xmlFree( errmes);
				}
			}

			return( ret_stat);
		}
		
		resp_node = resp_node->next;
		
	}
	return( -1);
} /* end ir_xml_scan_response() */



/**
 * ir_xml_convert_buffer
 * @oldbuffer: Ptr to memory buffer containing the raw RIBCL output.
 * @ret_size: Ptr to an integer used to return the size of the new buffer.
 *
 * Converts a buffer containing the raw output from a RIBCL command
 * that may contain multiple XML documents into a new buffer containing
 * that output massaged into a single XML document. This is done by adding
 * an outer enclosing tag called RIBCL_RESPONSE_OUTPUT.
 *
 * The libxml2 parser also doesn't like multiple occurences of the XML
 * header <?xml version=X.X ?>, so we filter those out as we encounter
 * them during the buffer copy. 
 *
 * The DL380 G6 also has a <DRIVES> section that causes errors with the
 * libxml2 parser. Since we don't use this information, we filter this out
 * during the buffer copy as well.
 *
 * Return value: Ptr to the new buffer on success, or NULL if failure.
 **/

static char *ir_xml_convert_buffer( char* oldbuffer, int *ret_size) 
{
	int newsize;

	static char declmatch[] = "<?xml version=";
	static char drives_start[] ="<DRIVES>";
	static char drives_end[] ="</DRIVES>";
	static char prefix[] = "<RIBCL_RESPONSE_OUTPUT>";
	static char suffix[] = "</RIBCL_RESPONSE_OUTPUT>";
	int prefix_len = (int)strlen( prefix);
	int suffix_len = (int)strlen( suffix);
	int matchlen = (int)strlen( declmatch);
	int drives_startlen = (int)(strlen(drives_start));
	int drives_endlen = (int)(strlen(drives_end));
	char *newbuffer;
	char *retbuffer;

	*ret_size = 0;

	/* Allocate a buffer large enough to hold the original buffer
	 * contents, along with our added enclosing tag */

	newsize = (int)strlen( oldbuffer) + prefix_len + suffix_len + 1;
	if( (newbuffer = malloc( newsize)) == NULL){
		return( NULL);
	}
	retbuffer = newbuffer;

	/* Add the prefix to the new buffer */
	strcpy( newbuffer, prefix);
	newbuffer += prefix_len;

	while( *oldbuffer != '\0'){

		if( *oldbuffer == '<'){

			/* Filter out embedded XML headers */

			if( strncmp( oldbuffer, declmatch, matchlen) == 0){
				while( *oldbuffer != '>'){

					/* Unclosed XML decl - Error */
					if( *oldbuffer == '\0'){
						free( newbuffer);
						return( NULL);
					}
					oldbuffer++;
				}
				oldbuffer++; /* Skip trailing '>' */
				continue;
			}
			/* Filter out <DRIVES>...</DRIVES> section */
			else if( strncmp( oldbuffer, drives_start,
							drives_startlen) == 0){

				while( strncmp( oldbuffer, drives_end,
								drives_endlen)){
					/* Unclosed XML decl - Error */
					if( *oldbuffer == '\0'){
						free( newbuffer);
						return( NULL);
					}
					oldbuffer++;
				}
				oldbuffer += drives_endlen;
				continue;
			}

		} /* end if *oldbuffer == '<' */

		*newbuffer++ = *oldbuffer++;
	}

	/* Add the suffix to the new buffer */
	strcpy( newbuffer, suffix);
	newbuffer += suffix_len;

	*newbuffer = '\0';
	*ret_size = (int)strlen( retbuffer);

#ifdef IR_XML_DEBUG
	printf("Size of converted buffer is %d\n", *ret_size);
#endif /* IR_XML_DEBUG */

	return( retbuffer);
} /* end ir_xml_convert_buffer() */



/**
 * ir_xml_doparse
 * @raw_ribcl_outbuf : ptr to a buffer containing RIBCL command output
 *
 * Fix up the raw RIBCL output and then use the libxml2 parser to
 * translate it into a xmlDoc tree.
 *
 * Return value: pointer to an XML doc tree on success, NULL on failure.
 **/
static xmlDocPtr ir_xml_doparse( char *raw_ribcl_outbuf)
{
	xmlDocPtr doc;
	char *ribcl_xml_buf;
	int xml_buf_size;

	ribcl_xml_buf = ir_xml_convert_buffer( raw_ribcl_outbuf,
					      &xml_buf_size);

	if(  ribcl_xml_buf == NULL){
		err("ir_xml_doparse(): Error converting XML output buffer.");
		return( NULL);
	}

	doc = xmlParseMemory( ribcl_xml_buf, xml_buf_size);

	if( doc == NULL){
		err("ir_xml_doparse(): XML parsing failed.");
	}
	
	free( ribcl_xml_buf);
	return( doc);

} /* end ir_xml_doparse() */



/**
 * ir_xml_extract_index
 * @prefix: String prefix for match 
 * @sourcestr: Source string containing resource label
 * @inexact: Flag to specify inexact matching between prefix and index.
 *
 * This routine is used to parse an index number from a resource label
 * given by iLO2 in a RIBCL command response. The label consists of an
 * alpha prefix string followed by one or more numeric characters. An example
 * of a label would be "Fan 4". The index value of a label must be greater
 * than zero. 
 *
 * Parameter 'prefix' gives a string prefix which must match the beginning
 * of the label passed with parameter 'sourcestr'. If sourcestr does not 
 * begin with prefix, IR_NO_PREFIX is returned.
 *
 * If parameter 'inexact' is non-zero, there can be an arbitrary sequence
 * fo non-numeric characters between the prefix and the numeric value. As
 * an example, if prefix is "Fan", the lable string "Fan Unit 4" would result
 * in a return value of 4 from this function when inexact is non-zero, and
 * a return of IR_NO_INDEX when inexact is zero.
 *
 * Return value:
 * The resource index extracted from the label if success.
 * IR_NO_PREFIX if the prefix does not match the beginning of sourcestr.
 * IR_NO_INDEX if there was a problem extracting a numeric index.
 **/
static int ir_xml_extract_index( char *prefix, char *sourcestr, int inexact)
{

	size_t p_len;
	int retval;

	/* First, check that the prefix matches the beginning of the
	 * source string */

	p_len = strlen( prefix);
	if( strncmp( prefix, sourcestr, p_len) ){
		/* prefix didn't match */
		return( IR_NO_PREFIX);
	}

	sourcestr = &sourcestr[p_len]; /* skip past the prefix */

	/* If the inexact parameter is non zero, allow any number of
	 * non-numeric characters between the prefix and the digits
	 * of the index */ 

	if( inexact){
		while( *sourcestr != '\0'){
			if( isdigit( *sourcestr)){
				break;
			}
			sourcestr++;
		}
	}

	/* If we are at the end of the string now, return an error */
	if( *sourcestr == '\0'){
		return( IR_NO_INDEX);
	}

	/* Try to extract the index. We use strtol() here because it checks
	 * for errors, while atoi() does not. */
	errno = 0;
	retval = (int)strtol( sourcestr, (char **)NULL, 10);
	if( errno){
		return( IR_NO_INDEX);
	}

	/* If strtol() couldn't find a number to convert, it returns zero.
	 * Fortunately for us, zero is an illegal index value, so we can 
 	 * detect it and return an error */

	if( retval == 0){
		retval = IR_NO_INDEX;
	}

	return( retval);

} /*end ir_xml_extract_index() */



/**
 * ir_xml_replacestr
 * @ostring: pointer to the char pointer of oldstring.
 * @nstring: pointer to the new string.
 *
 * This routine will update the oldstring with the new string if they
 * differ, using this algorithm:
 *
 * If the new string is null{
 * 	Don't modify the old string
 * } Else {
 *	If the old string in non-null and the new string is different than
 *	the old string {
 *		Replace the old string with the new string
 *	}
 * }
 *
 * Return value:
 * RIBCL_SUCCESS if success
 * -1 if failure
 **/
static int ir_xml_replacestr( char **ostring, char *nstring)
{
	size_t len;

	/* If the new string is NULL, return success, leaving the 
	 * old string unchanged. */

	if( nstring == NULL){
		return( RIBCL_SUCCESS);
	}

	/* If the old string is non null, and the new string is
	 * different than the old string, free the old string
	 * and replace it with the new string. */

	if( (*ostring != NULL) && (strcmp( *ostring, nstring)) ){
		free( *ostring);
		*ostring = NULL;	
	}

	if( *ostring == NULL){
		len = strlen( nstring);
		*ostring = malloc( len+1);
		if( *ostring == NULL){
			return( -1);
		}
		strncpy( *ostring, nstring, len+1);
	}

	return( RIBCL_SUCCESS);
	
} /* end ir_xml_replacestr() */



#ifdef ILO2_RIBCL_DEBUG_SENSORS
static char *ilo2_ribcl_sval2string[] = {
		"I2R_SEN_VAL_OK",
		"I2R_SEN_VAL_DEGRADED",
		"I2R_SEN_VAL_FAILED" };

static char *ilo2_ribcl_sstate2string[] = {
		"I2R_INITIAL",
		"I2R_OK",
		"I2R_DEGRADED_FROM_OK",
		"I2R_DEGRADED_FROM_FAIL",
		"I2R_FAILED" };

static char *ilo2_ribcl_snum2string[] = {
		"INVALID NUMBER",
		"I2R_SEN_FANHEALTH",
		"I2R_SEN_TEMPHEALTH",
		"I2R_SEN_POWERHEALTH" };
		
void dump_chassis_sensors( ilo2_ribcl_handler_t *ir_handler)
{
	int iter;
	int val;
	ilo2_ribcl_DiscoveryData_t *ddata;
	char *s1;
	char *s2;
	char *s3;

	ddata = &(ir_handler->DiscoveryData);

	for( iter = 1; iter < I2R_NUM_CHASSIS_SENSORS; iter++){
		s1 = ilo2_ribcl_snum2string[ iter];

		val = ddata->chassis_sensors[ iter].state;
		if( val == 0xFFFF){
			s2 = "I2R_NO_EXIST";
		} else {
			s2 = ilo2_ribcl_sstate2string[ val];
		}

		val = ddata->chassis_sensors[ iter].reading.intval;
		if( val == -1){
			s3 = "I2R_SEN_VAL_UNINITIALIZED";
		} else {
			s3 = ilo2_ribcl_sval2string[val];
		}

		err("Sensor %s state %s value %s.", s1, s2, s3); 
	}

} /* end dump_chassis_sensors() */ 
#endif /* ILO2_RIBCL_DEBUG_SENSORS */
