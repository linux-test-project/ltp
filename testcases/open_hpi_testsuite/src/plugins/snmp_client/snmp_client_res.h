/*      -*- linux-c -*-
 *
 * (C) Copyright IBM Corp. 2003
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  This
 * file and program are licensed under a BSD style license.  See
 * the Copying file included with the OpenHPI distribution for
 * full licensing terms.
 *
 * Authors:
 *	David Judkovics <djudkovi@us.ibm.com>
 *
 */

#ifndef _SNMP_CLIENT_RES_H_
#define _SNMP_CLIENT_RES_H_

#define MAX_LEN_OID 128


#define CHECK_END(a) ((a != SNMP_ENDOFMIBVIEW) &&  \
		  (a != SNMP_NOSUCHOBJECT) &&  \
		  (a != SNMP_NOSUCHINSTANCE))? 1:0 

int get_sahpi_table_entries(RPTable *temp_rptable, 
			    struct oh_handler_state *handle, 
			    const char *objid, 
			    int num_entries );

int get_sahpi_rdr_table( RPTable *temp_rptable,
			 struct oh_handler_state *handle, 
			 const char *objid, 
			 int num_entries );

int snmp_get_bulk( struct snmp_session *ss, 
			  const char *bulk_objid, 
			  struct snmp_pdu *bulk_pdu, 
			  struct snmp_pdu **bulk_response );



#define HPI_MIB 1,3,6,1,3,90
#define HPI_MIB_OID_LEN 6

#define SA_HPI_ENTRY_COUNT_OID_LEN				HPI_MIB_OID_LEN + 3
#define SA_HPI_ENTRY_OID_LEN 					HPI_MIB_OID_LEN + 3

#define NUM_RES_INDICES	3
#define NUM_RDR_INDICES 4
#define SA_HPI_ENTRY_TABLE_VARIABLE_OID_LENGTH			HPI_MIB_OID_LEN + 4
#define SA_HPI_ENTRY_TABLE_VARIABLE_FULL_OID_LENGTH	        SA_HPI_ENTRY_TABLE_VARIABLE_OID_LENGTH + NUM_RES_INDICES

/*
#define SA_HPI_DOMAIN_ID_OID_LEN				HPI_MIB_OID_LEN + 4
#define SA_HPI_ENTRY_ID_OID_LEN					HPI_MIB_OID_LEN + 4
#define SA_HPI_RESOURCE_ID_OID_LEN				HPI_MIB_OID_LEN + 4
#define SA_HPI_RESOURCE_ENTITY_PATH_OID_LEN			HPI_MIB_OID_LEN + 4
#define SA_HPI_RESOURCE_CAPABILITIES_OID_LEN			HPI_MIB_OID_LEN + 4
#define SA_HPI_RESOURCE_SEVERITY_OID_LEN			HPI_MIB_OID_LEN + 4
#define SA_HPI_RESOURCE_INFO_RESOURCE_REV_OID_LEN		HPI_MIB_OID_LEN + 4
#define SA_HPI_DOMAIN_RESOURCE_INFO_SPECIFIC_VER_OID_LEN	HPI_MIB_OID_LEN + 4
#define SA_HPI_DOMAIN_RESOURCE_INFO_DEVICE_SUPPORT_OID_LEN	HPI_MIB_OID_LEN + 4
#define SA_HPI_DOMAIN_RESOURCE_INFO_MANUFACTUER_ID_OID_LEN	HPI_MIB_OID_LEN + 4
#define SA_HPI_DOMAIN_RESOURCE_INFO_PRODUCT_ID_OID_LEN		HPI_MIB_OID_LEN + 4
#define SA_HPI_DOMAIN_RESOURCE_INFO_FIRMWARE_MAJOR_REV_OID_LEN	HPI_MIB_OID_LEN + 4
#define SA_HPI_DOMAIN_RESOURCE_INFO_FIRMWARE_MINOR_REV_OID_LEN	HPI_MIB_OID_LEN + 4
#define SA_HPI_DOMAIN_RESOURCE_INFO_AUX_FIRMWARE_REV_OID_LEN	HPI_MIB_OID_LEN + 4
#define SA_HPI_DOMAIN_RESOURCE_TAG_TEXT_TYPE_OID_LEN		HPI_MIB_OID_LEN + 4
#define SA_HPI_DOMAIN_RESOURCE_TAG_TEXT_LANGUAGE_OID_LEN	HPI_MIB_OID_LEN + 4
#define SA_HPI_DOMAIN_RESOURCE_TAG_OID_LEN			HPI_MIB_OID_LEN + 4
#define SA_HPI_DOMAIN_PARAM_CONTROL_OID_LEN			HPI_MIB_OID_LEN + 4
#define SA_HPI_DOMAIN_CLEAR_EVENTS_OID_LEN			HPI_MIB_OID_LEN + 4
#define SA_HPI_DOMAIN_EVENT_LOG_TIME_OID_LEN			HPI_MIB_OID_LEN + 4
#define SA_HPI_DOMAIN_EVENT_LOG_STATE_OID_LEN    		HPI_MIB_OID_LEN + 4
*/

#ifdef _SNMP_CLIENT_C_
oid sa_hpi_entry_count[] = 				{ HPI_MIB, 1, 1, 0 };
oid sa_hpi_entry[] = 					{ HPI_MIB, 1, 4, 1 };
oid sa_hpi_domain_id[] = 				{ HPI_MIB, 1, 4, 1, 1 };
oid sa_hpi_entry_id[] = 				{ HPI_MIB, 1, 4, 1, 2 };
oid sa_hpi_resource_id[] = 				{ HPI_MIB, 1, 4, 1, 3 };
oid sa_hpi_resource_entity_path[] = 			{ HPI_MIB, 1, 4, 1, 4 };
oid sa_hpi_resource_capabilities[] = 			{ HPI_MIB, 1, 4, 1, 5 };
oid sa_hpi_resource_severity[] = 			{ HPI_MIB, 1, 4, 1, 6 };
oid sa_hpi_resource_info_resource_rev[] =  		{ HPI_MIB, 1, 4, 1, 7 };
oid sa_hpi_domain_resource_info_specific_ver[] =  	{ HPI_MIB, 1, 4, 1, 8 };
oid sa_hpi_domain_resource_info_device_support[] =  	{ HPI_MIB, 1, 4, 1, 9 };
oid sa_hpi_domain_resource_info_manufactuer_id[] =  	{ HPI_MIB, 1, 4, 1, 10 };
oid sa_hpi_domain_resource_info_product_id[] =  	{ HPI_MIB, 1, 4, 1, 11 };
oid sa_hpi_domain_resource_info_firmware_major_rev[] = 	{ HPI_MIB, 1 ,4, 1, 12 };
oid sa_hpi_domain_resource_info_firmware_minor_rev[] = 	{ HPI_MIB, 1, 4, 1, 13 };
oid sa_hpi_domain_resource_info_aux_firmware_rev[] = 	{ HPI_MIB, 1, 4, 1, 14 };
oid sa_hpi_domain_resource_tag_text_type[] =  		{ HPI_MIB, 1, 4, 1, 15 };
oid sa_hpi_domain_resource_tag_text_language[] =  	{ HPI_MIB, 1, 4, 1, 16 };
oid sa_hpi_domain_resource_tag[] = 			{ HPI_MIB, 1, 4, 1, 17 };
oid sa_hpi_domain_param_control[] =  			{ HPI_MIB, 1, 4, 1, 18 };
oid sa_hpi_domain_clear_events[] =  			{ HPI_MIB, 1, 4, 1, 19 };
oid sa_hpi_domain_event_log_time[] =  			{ HPI_MIB, 1, 4, 1, 20 };
oid sa_hpi_domain_event_log_state[] =     		{ HPI_MIB, 1, 4, 1, 21 };
#else
extern oid sa_hpi_entry_count[]; 			     
extern oid sa_hpi_entry[]; 				     
extern oid sa_hpi_domain_id[]; 			     
extern oid sa_hpi_entry_id[]; 	
extern oid sa_hpi_resource_id[]; 			     
extern oid sa_hpi_resource_entity_path[]; 		     
extern oid sa_hpi_resource_capabilities[]; 		     
extern oid sa_hpi_resource_severity[];		     
extern oid sa_hpi_resource_info_resource_rev[];  	     
extern oid sa_hpi_domain_resource_info_specific_ver[];      
extern oid sa_hpi_domain_resource_info_device_support[];    
extern oid sa_hpi_domain_resource_info_manufactuer_id[];    
extern oid sa_hpi_domain_resource_info_product_id[];
extern oid sa_hpi_domain_resource_info_firmware_major_rev[];
extern oid sa_hpi_domain_resource_info_firmware_minor_rev[];
extern oid sa_hpi_domain_resource_info_aux_firmware_rev[]; 
extern oid sa_hpi_domain_resource_tag_text_type[]; 	     
extern oid sa_hpi_domain_resource_tag_text_language[];      
extern oid sa_hpi_domain_resource_tag[]; 		     
extern oid sa_hpi_domain_param_control[];  		     
extern oid sa_hpi_domain_clear_events[]; 		     
extern oid sa_hpi_domain_event_log_time[];  		     
extern oid sa_hpi_domain_event_log_state[];     	     
#endif

#define SA_HPI_ENTRY_COUNT      			".1.3.6.1.3.90.1.1.0"
#define SA_HPI_ENTRY 					".1.3.6.1.3.90.1.4.1"
#define SA_HPI_DOMAIN_ID				".1.3.6.1.3.90.1.4.1.1"
#define SA_HPI_ENTRY_ID					".1.3.6.1.3.90.1.4.1.2"
#define SA_HPI_RESOURCE_ID				".1.3.6.1.3.90.1.4.1.3"
#define SA_HPI_RESOURCE_ENTITY_PATH			".1.3.6.1.3.90.1.4.1.4"
#define SA_HPI_RESOURCE_CAPABILITIES			".1.3.6.1.3.90.1.4.1.5"
#define SA_HPI_RESOURCE_SEVERITY			".1.3.6.1.3.90.1.4.1.6"
#define SA_HPI_RESOURCE_INFO_RESOURCE_REV		".1.3.6.1.3.90.1.4.1.7"
#define SA_HPI_DOMAIN_RESOURCE_INFO_SPECIFIC_VER	".1.3.6.1.3.90.1.4.1.8"
#define SA_HPI_DOMAIN_RESOURCE_INFO_DEVICE_SUPPORT	".1.3.6.1.3.90.1.4.1.9"
#define SA_HPI_DOMAIN_RESOURCE_INFO_MANUFACTUER_ID	".1.3.6.1.3.90.1.4.1.10"
#define SA_HPI_DOMAIN_RESOURCE_INFO_PRODUCT_ID		".1.3.6.1.3.90.1.4.1.11"
#define SA_HPI_DOMAIN_RESOURCE_INFO_FIRMWARE_MAJOR_REV	".1.3.6.1.3.90.1.4.1.12"
#define SA_HPI_DOMAIN_RESOURCE_INFO_FIRMWARE_MINOR_REV	".1.3.6.1.3.90.1.4.1.13"
#define SA_HPI_DOMAIN_RESOURCE_INFO_AUX_FIRMWARE_REV	".1.3.6.1.3.90.1.4.1.14"
#define SA_HPI_DOMAIN_RESOURCE_TAG_TEXT_TYPE		".1.3.6.1.3.90.1.4.1.15"
#define SA_HPI_DOMAIN_RESOURCE_TAG_TEXT_LANGUAGE	".1.3.6.1.3.90.1.4.1.16"
#define SA_HPI_DOMAIN_RESOURCE_TAG			".1.3.6.1.3.90.1.4.1.17"
#define SA_HPI_DOMAIN_PARAM_CONTROL			".1.3.6.1.3.90.1.4.1.18"
#define SA_HPI_DOMAIN_CLEAR_EVENTS			".1.3.6.1.3.90.1.4.1.19"
#define SA_HPI_DOMAIN_EVENT_LOG_TIME			".1.3.6.1.3.90.1.4.1.20"
#define SA_HPI_DOMAIN_EVENT_LOG_STATE    		".1.3.6.1.3.90.1.4.1.21"

#define SA_HPI_RDR_COUNT      		".1.3.6.1.3.90.3.1.0"
#define SA_HPI_RDR_ENTRY	      	".1.3.6.1.3.90.3.2.1"
#define SA_HPI_RDR_RECORD_ID		".1.3.6.1.3.90.3.2.1.1"
#define SA_HPI_RDR_TYPE			".1.3.6.1.3.90.3.2.1.2"
#define SA_HPI_RDR_ENTITY_PATH		".1.3.6.1.3.90.3.2.1.3"
#define SA_HPI_RDR			".1.3.6.1.3.90.3.2.1.4"
#define SA_HPI_RDR_ID			".1.3.6.1.3.90.3.2.1.5"
#define SA_HPI_RDR_RTP			".1.3.6.1.3.90.3.2.1.6"

#define SA_HPI_CTRL_COUNT		".1.3.6.1.3.90.3.3.0"
#define SA_HPI_CTRL_ENTRY		".1.3.6.1.3.90.3.4.1"

#define SA_HPI_SENSOR_COUNT		".1.3.6.1.3.90.3.5.0"
#define SA_HPI_SENSOR_ENTRY		".1.3.6.1.3.90.3.6.1"

#define SA_HPI_INVENTORY_COUNT		".1.3.6.1.3.90.3.7.0"
#define SA_HPI_INVENTORY_ENTRY		".1.3.6.1.3.90.3.8.1"

#define SA_HPI_WATCHDOG_COUNT		".1.3.6.1.3.90.3.9.0"
#define SA_HPI_WATCHDOG_ENTRY		".1.3.6.1.3.90.3.10.1"
 


#define NUM_REPITIONS	240

#define NUM_RDR_TYPES	5

#endif /* SNMP_CLIENT_RES_H */
