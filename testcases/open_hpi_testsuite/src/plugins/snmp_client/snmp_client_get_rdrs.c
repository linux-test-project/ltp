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
 

#include <stdio.h>
#include <time.h>

#include <SaHpi.h>
#include <openhpi.h>
#include <epath_utils.h>
#include <rpt_utils.h>
#include <uid_utils.h>
#include <snmp_util.h>

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>

#include <snmp_client.h>
#include <snmp_client_res.h>

#include <netinet/in.h>

static int get_ctrl_capabilities(struct snmp_session *ss, 
				 SaHpiRdrT *rdr_cache, 
				 int *rdr_type_count,
				 int num_rdrs);

static int get_sensor_capabilities(struct snmp_session *ss, 
				   SaHpiRdrT *rdr_cache, 
				   int *rdr_type_count,
				   int num_rdrs);

static int get_inventory_capabilities(struct snmp_session *ss, 
				      SaHpiRdrT *rdr_cache, 
				      int *rdr_type_count,
				      int num_rdrs);

static int get_watchdog_capabilities(struct snmp_session *ss, 
				     SaHpiRdrT *rdr_cache, 
				     int *rdr_type_count,
				     int num_rdrs);

static void set_interpreted(SaHpiSensorInterpretedT *d, 
			    SaHpiSensorInterpretedT *s);

static void set_event(SaHpiSensorEvtStatusT *d, 
		      SaHpiSensorEvtStatusT *e);




int get_sahpi_rdr_table( RPTable *temp_rptable, struct oh_handler_state *handle,  
		         const char *objid,  
			 int num_entries )
{
	struct snmp_pdu *pdu = NULL;
	struct snmp_pdu *response = NULL;
	int status = SA_OK;
	int snmp_status;
	struct variable_list *vars;
	char *ep;

	int i;
	static int c = 0;
	int id = 0;


	SaHpiRdrT *rdr_cache = NULL;

	struct rdr_data *remote_rdr_data = NULL;
	
	int rdr_type_count[NUM_RDR_TYPES];
 
	struct snmp_client_hnd 
		*custom_handle = (struct snmp_client_hnd *)handle->data;
		    
	/* memory for temporary rdr_cache */
	rdr_cache = g_malloc0(num_entries * sizeof(*rdr_cache));
	if (!rdr_cache) 
		status = SA_ERR_HPI_ERROR;
	/* memory for remote rdr data */
	remote_rdr_data = g_malloc0(num_entries * sizeof(*remote_rdr_data));
	if (!remote_rdr_data) 
		status = SA_ERR_HPI_ERROR;

	snmp_status = snmp_get_bulk(custom_handle->ss, objid, pdu, &response);

	/* Process the response */
	if (status == SA_OK && snmp_status == STAT_SUCCESS && response->errstat == SNMP_ERR_NOERROR) {
	/* SUCCESS: Print the result variables */  

		/* Get the data from the response */
		vars = response->variables;
		
		/* SA_HPI_RDR_RECORD_ID */
		for (i = 0; i < num_entries; i++) {
			if (vars->type == ASN_UNSIGNED)	{
				/* save remote rdr info */
				/* INDEX   { saHpiDomainID, saHpiResourceID, saHpiRdrRecordId, saHpiRdrType } */
				remote_rdr_data[i].index.remote_domain = 
					vars->name[vars->name_length - 4];
				remote_rdr_data[i].index.remote_resource_id = 
					vars->name[vars->name_length - 3];
				remote_rdr_data[i].index.remote_record_id = 
					vars->name[vars->name_length - 2];

				rdr_cache[i].RecordId = (SaHpiEntryIdT)*vars->val.integer;
				
			} else
				printf("SA_HPI_DOMAIN_ID:something terrible has happened\n");
			vars = vars->next_variable;
		}

		/* SA_HPI_RDR_TYPE */
		for (i = 0; i < num_entries; i++) {
			if (vars->type == ASN_INTEGER)
				rdr_cache[i].RdrType = (SaHpiRdrTypeT)*vars->val.integer;
			else
				printf("SA_HPI_RDR_TYPE:something terrible has happened\n");
			vars = vars->next_variable;
		
		}
		/* SA_HPI_RDR_ENTITY_PATH */
		for (i = 0; i < num_entries; i++) {
			if (vars->type == ASN_OCTET_STR)  {
				ep = g_malloc0(vars->val_len + 1);
				memcpy(ep, vars->val.string, vars->val_len);
				if(string2entitypath(ep, &rdr_cache[i].Entity))
					dbg("something terrible happened with SA_HPI_RDR_ENTITY_PATH");
				free(ep);
			}
			else
				printf("SA_HPI_RDR_ENTITY_PATH:something terrible has happened\n");
			vars = vars->next_variable;		
		}


		/* count the number of different type rdrs, see SaHpiRdrTypeT */
		memset(rdr_type_count, 0, NUM_RDR_TYPES * sizeof(int));
		for (i=0; i < num_entries; i++) {
			switch (rdr_cache[i].RdrType) {
			case SAHPI_NO_RECORD:
				rdr_type_count[SAHPI_NO_RECORD]++;
				break;
			case SAHPI_CTRL_RDR:
				rdr_type_count[SAHPI_CTRL_RDR]++;
				break;
			case SAHPI_SENSOR_RDR:
				rdr_type_count[SAHPI_SENSOR_RDR]++;
				break;
			case SAHPI_INVENTORY_RDR:
				rdr_type_count[SAHPI_INVENTORY_RDR]++;
				break;
			case SAHPI_WATCHDOG_RDR:
				rdr_type_count[SAHPI_WATCHDOG_RDR]++;
				break;
			}
		}

		get_ctrl_capabilities(custom_handle->ss, rdr_cache, rdr_type_count, num_entries);
		
		get_sensor_capabilities(custom_handle->ss, rdr_cache, rdr_type_count, num_entries);
		
		get_inventory_capabilities(custom_handle->ss, rdr_cache, rdr_type_count, num_entries);
		
		get_watchdog_capabilities(custom_handle->ss, rdr_cache, rdr_type_count, num_entries);

		/* the RDR's to the plugins rptcache */
		for ( i = 0; i < num_entries; i++) {
			id = oh_uid_lookup(&rdr_cache[i].Entity);
			if( id < 0 ) { 
				dbg("error looking up uid in get_sahpi_rdr_table");  
				status = SA_ERR_HPI_ERROR;
				break;
			}
			if ( oh_add_rdr(temp_rptable, id, &rdr_cache[i], &remote_rdr_data[i], 1) )
				dbg("oh_add_resource failed for rdr %d, in get_sahpi_rdr_table", i);
		}

		/* print the RDR objects for fun */
		for(vars = response->variables; vars; vars = vars->next_variable) {
			c++;
			printf("\n**** oid count %d ******\n", c);
			
			if(CHECK_END(vars->type)) {          
				print_variable(vars->name, vars->name_length, vars);  	
			}  else 
				fprintf(stderr,"No idea.\n");
		} 

		status = SA_OK;

	/* FAILURE: print what went wrong! */
	} else { 
		if (snmp_status == STAT_SUCCESS) {
			fprintf(stderr, "Error in packet, WHilst getting rdr's\nReason: %s\n", snmp_errstring(response->errstat));
			if (remote_rdr_data) 
				g_free(remote_rdr_data);
		} else
			snmp_sess_perror("snmpget", custom_handle->ss );
	}

	/* free the temporary rdr_cache */
	if(rdr_cache)		
		g_free(rdr_cache);

	/* Clean up: free the response. */
	if (response)
		snmp_free_pdu(response);

	return(status);

}


int get_ctrl_capabilities(struct snmp_session *ss, 
			  SaHpiRdrT *rdr_cache, 
			  int *rdr_type_count, 
			  int num_rdrs)
{
	struct snmp_pdu *get_cap_pdu = NULL;
	struct snmp_pdu *get_cap_response = NULL;
	int status = SA_OK;
	int i = 0;
	struct variable_list *vars;
	struct snmp_value get_value;

	SaHpiRdrTypeUnionT *sahpi_ctr_cap = NULL;
	
	/* get SAHPI_CTRL_RDR data */
	snmp_get(ss, SA_HPI_CTRL_COUNT, &get_value);
	if (get_value.integer == rdr_type_count[SAHPI_CTRL_RDR]) {
		
		/* Get the data from the response  check return status code 
		   maybe awhile loop of get bulks  until all data is had
		   then fill in table */
		status = snmp_get_bulk(ss, SA_HPI_CTRL_ENTRY, get_cap_pdu, &get_cap_response);

		/* begin filling in table */
		sahpi_ctr_cap = g_malloc0(get_value.integer * sizeof(*sahpi_ctr_cap));
		vars = get_cap_response->variables;

		/* SA_HPI_CTRL_NUM */
		for (i = 0; i < get_value.integer; i++) {
			if (vars->type == ASN_UNSIGNED)
				sahpi_ctr_cap[i].CtrlRec.Num = (SaHpiCtrlNumT)*vars->val.integer;
			else
				printf("SA_HPI_CTRL_NUM:something terrible has happened\n");
			vars = vars->next_variable;
		
		}
		/* SA_HPI_CTRL_OUTPUT_TYPE */
		for (i = 0; i < get_value.integer; i++) {
			if (vars->type == ASN_INTEGER)
				sahpi_ctr_cap[i].CtrlRec.OutputType = 
					(SaHpiCtrlOutputTypeT)*vars->val.integer - 1;
			else
				printf("SA_HPI_CTRL_OUTPUT_TYPE:something terrible has happened\n");
			vars = vars->next_variable;
		
		}
		/* SA_HPI_CTRL_IGNORE */
		for (i = 0; i < get_value.integer; i++) {
			if (vars->type == ASN_INTEGER)
				sahpi_ctr_cap[i].CtrlRec.Ignore = 
					(*vars->val.integer == 1) ? SAHPI_TRUE : SAHPI_FALSE;
			else
				printf("SA_HPI_CTRL_IGNORE:something terrible has happened\n");
			vars = vars->next_variable;
		
		}
		/* SA_HPI_CTRL_TYPE */
		for (i = 0; i < get_value.integer; i++) {
			if (vars->type == ASN_INTEGER)
				sahpi_ctr_cap[i].CtrlRec.Type = (SaHpiCtrlTypeT)*vars->val.integer - 1;
			else
				printf("SA_HPI_CTRL_TYPE:something terrible has happened\n");
			vars = vars->next_variable;
		
		}				    
		/* SA_HPI_CTRL_STATE */
		for (i = 0; i < get_value.integer; i++) { 				    
			vars = vars->next_variable;
		}
		/* SA_HPI_CTRL_ATTRIBUTES */
		for (i = 0; i < get_value.integer; i++) { 
			if (vars->type == ASN_OCTET_STR) {

				/* local variables used by case statements */
				int *data;
				char *repeat;
				int *stream_data;
				char *text_info;
				char *oem_data;

				switch (sahpi_ctr_cap[i].CtrlRec.Type) {
				case SAHPI_CTRL_TYPE_DIGITAL:
					memcpy(&sahpi_ctr_cap[i].CtrlRec.TypeUnion.Digital.Default, 
					       vars->val.string,
					       sizeof(sahpi_ctr_cap[i].CtrlRec.TypeUnion.Digital.Default));
					sahpi_ctr_cap[i].CtrlRec.TypeUnion.Digital.Default = 
						ntohs(sahpi_ctr_cap[i].CtrlRec.TypeUnion.Digital.Default);
					break;
				case SAHPI_CTRL_TYPE_DISCRETE:
					memcpy(&sahpi_ctr_cap[i].CtrlRec.TypeUnion.Discrete.Default, 
					       vars->val.string,
					       sizeof(sahpi_ctr_cap[i].CtrlRec.TypeUnion.Discrete.Default));
					sahpi_ctr_cap[i].CtrlRec.TypeUnion.Discrete.Default =
						ntohl(sahpi_ctr_cap[i].CtrlRec.TypeUnion.Discrete.Default);
					break;
				case SAHPI_CTRL_TYPE_ANALOG:
					data = (int *)vars->val.string;
					sahpi_ctr_cap[i].CtrlRec.TypeUnion.Analog.Min = *data;		
					sahpi_ctr_cap[i].CtrlRec.TypeUnion.Analog.Min =
						ntohl(sahpi_ctr_cap[i].CtrlRec.TypeUnion.Analog.Min);
					data++;
					sahpi_ctr_cap[i].CtrlRec.TypeUnion.Analog.Max = *data;
					sahpi_ctr_cap[i].CtrlRec.TypeUnion.Analog.Max =
						ntohl(sahpi_ctr_cap[i].CtrlRec.TypeUnion.Analog.Max);
					data++;
					sahpi_ctr_cap[i].CtrlRec.TypeUnion.Analog.Default = *data;
					sahpi_ctr_cap[i].CtrlRec.TypeUnion.Analog.Default = 
						ntohl(sahpi_ctr_cap[i].CtrlRec.TypeUnion.Analog.Default);
					break;
				case SAHPI_CTRL_TYPE_STREAM:
					/* set repeat */
					repeat = vars->val.string;
					memcpy(&sahpi_ctr_cap[i].CtrlRec.TypeUnion.Stream.Default.Repeat, 
					       repeat, 
					       sizeof(sahpi_ctr_cap[i].CtrlRec.TypeUnion.Stream.Default.Repeat));
					repeat++;
					stream_data = (int *)repeat;
					/* set .StreamLength */
					memcpy(&sahpi_ctr_cap[i].CtrlRec.TypeUnion.Stream.Default.StreamLength,
					       stream_data, 
					       sizeof(sahpi_ctr_cap[i].CtrlRec.TypeUnion.Stream.Default.StreamLength));
					sahpi_ctr_cap[i].CtrlRec.TypeUnion.Stream.Default.StreamLength = 
						ntohs(sahpi_ctr_cap[i].CtrlRec.TypeUnion.Stream.Default.StreamLength);
					stream_data++;
					/* set the .Stream data */
					memcpy(&sahpi_ctr_cap[i].CtrlRec.TypeUnion.Stream.Default.Stream, 
					       stream_data, 
					       SAHPI_CTRL_MAX_STREAM_LENGTH); 
                                              break;
				case SAHPI_CTRL_TYPE_TEXT:
					text_info = vars->val.string;
					sahpi_ctr_cap[i].CtrlRec.TypeUnion.Text.MaxChars = *text_info;
					text_info++;
					sahpi_ctr_cap[i].CtrlRec.TypeUnion.Text.MaxLines = *text_info;
					text_info++;
					sahpi_ctr_cap[i].CtrlRec.TypeUnion.Text.Language = *text_info;
					text_info++;
					sahpi_ctr_cap[i].CtrlRec.TypeUnion.Text.DataType = *text_info;
					text_info++;
				        sahpi_ctr_cap[i].CtrlRec.TypeUnion.Text.Default.Line = *text_info,
					text_info++;
					sahpi_ctr_cap[i].CtrlRec.TypeUnion.Text.Default.Text.DataLength =
						*text_info;
					sahpi_ctr_cap[i].CtrlRec.TypeUnion.Text.Default.Text.DataType =
						sahpi_ctr_cap[i].CtrlRec.TypeUnion.Text.DataType;
					sahpi_ctr_cap[i].CtrlRec.TypeUnion.Text.Default.Text.Language =
						sahpi_ctr_cap[i].CtrlRec.TypeUnion.Text.Language;
					text_info++;
					memcpy(&sahpi_ctr_cap[i].CtrlRec.TypeUnion.Text.Default.Text.Data,
					       text_info, 
					       sahpi_ctr_cap[i].CtrlRec.TypeUnion.Text.Default.Text.DataLength);
					break;
				case SAHPI_CTRL_TYPE_OEM:
					/* get Mid */
					oem_data = vars->val.string;
					memcpy(&sahpi_ctr_cap[i].CtrlRec.TypeUnion.Oem.MId, 
					vars->val.string, 
					sizeof(sahpi_ctr_cap[i].CtrlRec.TypeUnion.Oem.MId));
					oem_data = oem_data + 
						sizeof(sahpi_ctr_cap[i].CtrlRec.TypeUnion.Oem.MId);
					/* get ConfigData */
					memcpy(&sahpi_ctr_cap[i].CtrlRec.TypeUnion.Oem.ConfigData, 
					       oem_data, 
					       SAHPI_CTRL_OEM_CONFIG_LENGTH);
					oem_data = oem_data + SAHPI_CTRL_OEM_CONFIG_LENGTH;
					/* get BodyLength */
					sahpi_ctr_cap[i].CtrlRec.TypeUnion.Oem.Default.BodyLength = 
						*oem_data;
					oem_data++;
					/* set MId */
					sahpi_ctr_cap[i].CtrlRec.TypeUnion.Oem.Default.MId = 
						sahpi_ctr_cap[i].CtrlRec.TypeUnion.Oem.MId;
					/* get Body/Text data */
					memcpy(&sahpi_ctr_cap[i].CtrlRec.TypeUnion.Oem.Default.Body,
					       oem_data,
					       sahpi_ctr_cap[i].CtrlRec.TypeUnion.Oem.Default.BodyLength);					     
					break;
				default:
					dbg("ERROR: unknown CTRL TYPE");
					break;
				}
			} else
				printf("SA_HPI_CTRL_STATE:something terrible has happened\n");

			vars = vars->next_variable;		
		}
		/* SA_HPI_CTRL_OEM */
		for (i = 0; i < get_value.integer; i++) {
			if (vars->type == ASN_UNSIGNED)
				sahpi_ctr_cap[i].CtrlRec.Oem = (SaHpiUint32T)*vars->val.integer;
			else
				printf("SA_HPI_CTRL_OEM:something terrible has happened\n");
			vars = vars->next_variable;
		
		}
		/* SA_HPI_CTRL_TEXT_TYPE */
		for (i = 0; i < get_value.integer; i++) {
			vars = vars->next_variable;
		
		}
		/* SA_HPI_CTRL_TEXT_LANGUAGE */
		for (i = 0; i < get_value.integer; i++) {
			vars = vars->next_variable;
		
		}
		/* SA_HPI_CTRL_TEXT */
		for (i = 0; i < get_value.integer; i++) {
			vars = vars->next_variable;
		
		}
		/* SA_HPI_CTRL_RDR */
		for (i = 0; i < get_value.integer; i++) {
			vars = vars->next_variable;
		
		}

		/* CTRL CAPABILITIES ADD TO rdr_cache */
		int ii = 0;
		for( i = 0; i < num_rdrs; i++) {
			if (rdr_cache[i].RdrType == SAHPI_CTRL_RDR ) {
				rdr_cache[i].RdrTypeUnion.CtrlRec = sahpi_ctr_cap[ii].CtrlRec;
				ii++;
				if (ii > get_value.integer) {
					dbg("Number of RDRs of type CTRL exceeds discovered CTRL caps");
					status = SA_ERR_HPI_ERROR;
					break;
				}
			}
		}
	}
	else {
		dbg("Sever Error: SA_HPI_CTRL_COUNT value doesn't match Resource Capability Settings");
	}

	/* free the temporary rdr ctrl cap cache */
	if (sahpi_ctr_cap) 
		g_free(sahpi_ctr_cap);

	/* free the snmp response */
	if (get_cap_response) 
		snmp_free_pdu(get_cap_response);
		
	return(status);
}

int get_sensor_capabilities(struct snmp_session *ss, 
			  SaHpiRdrT *rdr_cache, 
			  int *rdr_type_count, 
			  int num_rdrs)
{
	struct snmp_pdu *get_cap_pdu = NULL;
	struct snmp_pdu *get_cap_response = NULL;
	int status = SA_OK;
	int i = 0;
	struct variable_list *vars;
	struct snmp_value get_value;

	SaHpiRdrTypeUnionT *sahpi_sensor_cap = NULL;
	
	/* get SAHPI_SENSOR_RDR data */
	snmp_get(ss, SA_HPI_SENSOR_COUNT, &get_value);
	if (get_value.integer == rdr_type_count[SAHPI_SENSOR_RDR]) {
		status = snmp_get_bulk(ss, SA_HPI_SENSOR_ENTRY, get_cap_pdu, &get_cap_response);

		/* begin filling in table */
		sahpi_sensor_cap = g_malloc0(get_value.integer * sizeof(*sahpi_sensor_cap));
		vars = get_cap_response->variables;

		/* SA_HPI_SENSOR_INDEX */
		for (i = 0; i < get_value.integer; i++) {
			if (vars->type == ASN_UNSIGNED)
				sahpi_sensor_cap[i].SensorRec.Num = 
					(SaHpiSensorNumT)*vars->val.integer;
			else
				printf("SA_HPI_SENSOR_INDEX:something terrible has happened\n");
			vars = vars->next_variable;
		
		}
		/* SA_HPI_SENSOR_TYPE */
		for (i = 0; i < get_value.integer; i++) {
			if (vars->type == ASN_INTEGER) 
				sahpi_sensor_cap[i].SensorRec.Type = 
					(SaHpiSensorTypeT)*vars->val.integer;

			else
				printf("SA_HPI_SENSOR_TYPE:something terrible has happened\n");
			vars = vars->next_variable;
		
		}
		/* SA_HPI_SENSOR_CATEGORY */
		for (i = 0; i < get_value.integer; i++) {
			if (vars->type == ASN_INTEGER)
				sahpi_sensor_cap[i].SensorRec.Category = 
					(SaHpiEventCategoryT)*vars->val.integer;
			else
				printf("SA_HPI_SENSOR_CATEGORY:something terrible has happened\n");
			vars = vars->next_variable;
		
		}
		/* SA_HPI_SENSOR_EVENTS_CATEGORY_CONTROL */
		for (i = 0; i < get_value.integer; i++) {
			if (vars->type == ASN_INTEGER)
				sahpi_sensor_cap[i].SensorRec.EventCtrl = 
					(SaHpiSensorEventCtrlT)*vars->val.integer - 1;
			else
				printf("SA_HPI_SENSOR_EVENTS_CATEGORY_CONTROL:something terrible has happened\n");
			vars = vars->next_variable;
		
		}
		/* SA_HPI_SENSOR_EVENT_STATE */
		for (i = 0; i < get_value.integer; i++) {
			if (vars->type == ASN_UNSIGNED)	
				sahpi_sensor_cap[i].SensorRec.Events = 
					(SaHpiEventStateT)*vars->val.integer;
			else
				printf("SA_HPI_SENSOR_EVENT_STATE:something terrible has happened\n");
			vars = vars->next_variable;
		
		}
		/* SA_HPI_SENSOR_STATUS */
		for (i = 0; i < get_value.integer; i++) {
			/* SaHpiSensorEvtEnablesT is ONLY used in "saHpiSensorEventEnablesGet" */
			/* and "saHpiSensorEventEnablesSet" function calls */
			vars = vars->next_variable;
		
		}
		/* SA_HPI_SENSOR_ASSERT_EVENTS*/
		for (i = 0; i < get_value.integer; i++) {
			vars = vars->next_variable;
		}
		/* SA_HPI_SENSOR_DEASSERT_EVENTS */
		for (i = 0; i < get_value.integer; i++) {
			vars = vars->next_variable;
		}
		/* SA_HPI_SENSOR_IGNORE */
		for (i = 0; i < get_value.integer; i++) {
			if (vars->type == ASN_INTEGER) 
				sahpi_sensor_cap[i].SensorRec.Ignore = 
					(*vars->val.integer == 1) ? SAHPI_TRUE : SAHPI_FALSE;
			else
				printf("SA_HPI_SENSOR_IGNORE:something terrible has happened\n");
			vars = vars->next_variable;
		
		}
		/* SA_HPI_SENSOR_READING_FORMATS */
		for (i = 0; i < get_value.integer; i++) {
			if (vars->type == ASN_UNSIGNED)
				sahpi_sensor_cap[i].SensorRec.DataFormat.ReadingFormats = 
					(SaHpiSensorReadingFormatsT)*vars->val.integer;
			else
				printf("SA_HPI_SENSOR_READING_FORMATS:something terrible has happened\n");
			vars = vars->next_variable;
		
		}
		/* SA_HPI_SENSOR_IS_NUMERIC */
		for (i = 0; i < get_value.integer; i++) {
			if (vars->type == ASN_INTEGER)
				sahpi_sensor_cap[i].SensorRec.DataFormat.IsNumeric = 
					(*vars->val.integer == 1) ? SAHPI_TRUE : SAHPI_FALSE;
			else
				printf("SA_HPI_SENSOR_IS_NUMERIC:something terrible has happened\n");
			vars = vars->next_variable;
		
		}
		/* SA_HPI_SENSOR_SIGN_FORMAT */
		for (i = 0; i < get_value.integer; i++) {
			if (vars->type == ASN_INTEGER)
				sahpi_sensor_cap[i].SensorRec.DataFormat.SignFormat = 
					(SaHpiSensorSignFormatT)*vars->val.integer;
			else
				printf("SA_HPI_SENSOR_SIGN_FORMAT:something terrible has happened\n");
			vars = vars->next_variable;
		
		}
		/* SA_HPI_SENSOR_BASE_UNITS */
		for (i = 0; i < get_value.integer; i++) {
			if (vars->type == ASN_INTEGER)
				sahpi_sensor_cap[i].SensorRec.DataFormat.BaseUnits = 
					(SaHpiSensorUnitsT)*vars->val.integer;
			else
				printf("SA_HPI_SENSOR_BASE_UNITS:something terrible has happened\n");
			vars = vars->next_variable;
		
		}
		/* SA_HPI_SENSOR_MODIFIER_UNITS */
		for (i = 0; i < get_value.integer; i++) {
			if (vars->type == ASN_INTEGER)
				sahpi_sensor_cap[i].SensorRec.DataFormat.ModifierUnits = 
					(SaHpiSensorUnitsT)*vars->val.integer;
			else
				printf("SA_HPI_SENSOR_MODIFIER_UNITS:something terrible has happened\n");
			vars = vars->next_variable;
		
		}
		/* SA_HPI_SENSOR_MODIFIER_USE */
		for (i = 0; i < get_value.integer; i++) {
			if (vars->type == ASN_INTEGER)
				sahpi_sensor_cap[i].SensorRec.DataFormat.ModifierUse = 
					(SaHpiSensorModUnitUseT)*vars->val.integer;
			else
				printf("SA_HPI_SENSOR_MODIFIER_USE:something terrible has happened\n");
			vars = vars->next_variable;
		
		}
		/* SA_HPI_SENSOR_FACTORS_STATIC */
		for (i = 0; i < get_value.integer; i++) {
			if (vars->type == ASN_INTEGER)
				sahpi_sensor_cap[i].SensorRec.DataFormat.FactorsStatic = 
					(*vars->val.integer == 1) ? SAHPI_TRUE : SAHPI_FALSE;
			else
				printf("SA_HPI_SENSOR_FACTORS_STATIC:something terrible has happened\n");
			vars = vars->next_variable;
		
		}
		/* SA_HPI_SENSOR_FACTORS */
		char *factors;
		for (i = 0; i < get_value.integer; i++) {
			if (vars->type == ASN_OCTET_STR) {
				if(sahpi_sensor_cap[i].SensorRec.DataFormat.FactorsStatic == SAHPI_TRUE) {
					factors = vars->val.string;
					/* set M_Factor */
					memcpy(&sahpi_sensor_cap[i].SensorRec.DataFormat.Factors.M_Factor,
					       factors,
					       sizeof(SaHpiInt16T));
					sahpi_sensor_cap[i].SensorRec.DataFormat.Factors.M_Factor =
						ntohs(sahpi_sensor_cap[i].SensorRec.DataFormat.Factors.M_Factor);
					factors = factors + sizeof(sahpi_sensor_cap[i].SensorRec.DataFormat.Factors.M_Factor);
					/* set B_Factor */
					memcpy(&sahpi_sensor_cap[i].SensorRec.DataFormat.Factors.B_Factor,
					       factors,
					       sizeof(sahpi_sensor_cap[i].SensorRec.DataFormat.Factors.B_Factor));
					sahpi_sensor_cap[i].SensorRec.DataFormat.Factors.B_Factor =
						ntohs(sahpi_sensor_cap[i].SensorRec.DataFormat.Factors.B_Factor);
					factors = factors + sizeof(sahpi_sensor_cap[i].SensorRec.DataFormat.Factors.B_Factor);
					/* set Accuracy_Factor */
					memcpy(&sahpi_sensor_cap[i].SensorRec.DataFormat.Factors.AccuracyFactor,
					       factors,
					       sizeof(sahpi_sensor_cap[i].SensorRec.DataFormat.Factors.AccuracyFactor));
					sahpi_sensor_cap[i].SensorRec.DataFormat.Factors.AccuracyFactor = 
						ntohs(sahpi_sensor_cap[i].SensorRec.DataFormat.Factors.AccuracyFactor);
					factors++;
					/*set Tolerance */
					sahpi_sensor_cap[i].SensorRec.DataFormat.Factors.ToleranceFactor =
						*factors++;
					/* set Accuracy Exp*/
					sahpi_sensor_cap[i].SensorRec.DataFormat.Factors.ExpA = 
						*factors++;
					/* set Result Exp */
					sahpi_sensor_cap[i].SensorRec.DataFormat.Factors.ExpR = 
						*factors++;
					/* set B Exp */
					sahpi_sensor_cap[i].SensorRec.DataFormat.Factors.ExpB =
						*factors++;
				}     
			} else
				printf("SA_HPI_SENSOR_FACTORS:something terrible has happened\n");
			vars = vars->next_variable;
		}
		/* SA_HPI_SENSOR_FACTORS_LINEARIZATION */
		for (i = 0; i < get_value.integer; i++) {
			if (vars->type == ASN_INTEGER)
				sahpi_sensor_cap[i].SensorRec.DataFormat.Factors.Linearization = 
					(SaHpiSensorLinearizationT)*vars->val.integer;
			else
				printf("SA_HPI_SENSOR_FACTORS_LINEARIZATION:something terrible has happened\n");
			vars = vars->next_variable;
		
		}
		/* SA_HPI_SENSOR_PERCENTAGE */
		for (i = 0; i < get_value.integer; i++) {
			if (vars->type == ASN_INTEGER)
				sahpi_sensor_cap[i].SensorRec.DataFormat.Percentage = 
					(*vars->val.integer == 1) ? SAHPI_TRUE : SAHPI_FALSE;
			else
				printf("SA_HPI_SENSOR_PERCENTAGE:something terrible has happened\n");
			vars = vars->next_variable;
		
		}
		/* SA_HPI_SENSOR_RANGE_FLAGS */
		for (i = 0; i < get_value.integer; i++) {
			if (vars->type == ASN_UNSIGNED)
				sahpi_sensor_cap[i].SensorRec.DataFormat.Range.Flags = 
					(SaHpiSensorRangeFlagsT)*vars->val.integer;
			else
				printf("SA_HPI_SENSOR_RANGE_FLAGS:something terrible has happened\n");
			vars = vars->next_variable;
		
		}
		/* SA_HPI_SENSOR_RANGE_READINGS_VALUE_PRESENT */
		char *range;
		for (i = 0; i < get_value.integer; i++) {
			if (vars->type == ASN_OCTET_STR) {
				range = vars->val.string;
				sahpi_sensor_cap[i].SensorRec.DataFormat.Range.Max.ValuesPresent = 
					(SaHpiSensorReadingFormatsT)*range++;
				sahpi_sensor_cap[i].SensorRec.DataFormat.Range.Min.ValuesPresent = 
					(SaHpiSensorReadingFormatsT)*range++;
				sahpi_sensor_cap[i].SensorRec.DataFormat.Range.Nominal.ValuesPresent = 
					(SaHpiSensorReadingFormatsT)*range++;
				sahpi_sensor_cap[i].SensorRec.DataFormat.Range.NormalMax.ValuesPresent = 
					(SaHpiSensorReadingFormatsT)*range++;
				sahpi_sensor_cap[i].SensorRec.DataFormat.Range.NormalMin.ValuesPresent = 
					(SaHpiSensorReadingFormatsT)*range++;
			} else
				printf("SA_HPI_SENSOR_RANGE_FLAGS:something terrible has happened\n");
			vars = vars->next_variable;
		}
		/* SA_HPI_SENSOR_RANGE_READING_RAW */
		SaHpiUint32T *raw;
		for (i = 0; i < get_value.integer; i++) {
			if (vars->type == ASN_OCTET_STR) {
				raw = (SaHpiUint32T *)vars->val.string;
				/* MAX */
				sahpi_sensor_cap[i].SensorRec.DataFormat.Range.Max.Raw = 
					(SaHpiUint32T)*raw++;
				sahpi_sensor_cap[i].SensorRec.DataFormat.Range.Max.Raw =
					ntohl(sahpi_sensor_cap[i].SensorRec.DataFormat.Range.Max.Raw);
				/* MIN */
				sahpi_sensor_cap[i].SensorRec.DataFormat.Range.Min.Raw = 
					(SaHpiUint32T)*raw++;
				sahpi_sensor_cap[i].SensorRec.DataFormat.Range.Min.Raw =
					ntohl(sahpi_sensor_cap[i].SensorRec.DataFormat.Range.Min.Raw);

				/* NOMINAL */
				sahpi_sensor_cap[i].SensorRec.DataFormat.Range.Nominal.Raw = 
					(SaHpiUint32T)*raw++;
				sahpi_sensor_cap[i].SensorRec.DataFormat.Range.Nominal.Raw =
					ntohl(sahpi_sensor_cap[i].SensorRec.DataFormat.Range.Nominal.Raw);

				/* NORMAL_MAX */
				sahpi_sensor_cap[i].SensorRec.DataFormat.Range.NormalMax.Raw = 
					(SaHpiUint32T)*raw++;
				sahpi_sensor_cap[i].SensorRec.DataFormat.Range.NormalMax.Raw =
					ntohl(sahpi_sensor_cap[i].SensorRec.DataFormat.Range.NormalMax.Raw);

				/* NORMAL_MIN */
				sahpi_sensor_cap[i].SensorRec.DataFormat.Range.NormalMin.Raw = 
					(SaHpiUint32T)*raw++;
				sahpi_sensor_cap[i].SensorRec.DataFormat.Range.NormalMin.Raw =
					ntohl(sahpi_sensor_cap[i].SensorRec.DataFormat.Range.NormalMin.Raw);
			} else
				printf("SA_HPI_SENSOR_RANGE_FLAGS:something terrible has happened\n");
			vars = vars->next_variable;
		}
		/* SA_HPI_SENSOR_RANGE_READING_INTERPRETED */
		SaHpiSensorInterpretedT *interpreted;
		for (i = 0; i < get_value.integer; i++) {
			if (vars->type == ASN_OCTET_STR) {
				interpreted = (SaHpiSensorInterpretedT *)vars->val.string;
				/* MAX */
				set_interpreted(
					&sahpi_sensor_cap[i].SensorRec.DataFormat.Range.Max.Interpreted,
					interpreted++);
				/* MIN */
				set_interpreted(
					&sahpi_sensor_cap[i].SensorRec.DataFormat.Range.Min.Interpreted,
					interpreted++);
				/* NOMINAL */
				set_interpreted(
					&sahpi_sensor_cap[i].SensorRec.DataFormat.Range.Nominal.Interpreted,
					interpreted++);

				/* NOMINAL_MAX */
				set_interpreted(
					&sahpi_sensor_cap[i].SensorRec.DataFormat.Range.NormalMax.Interpreted,
					interpreted++);

				/*NOMINAL_MIN */
				set_interpreted(
					&sahpi_sensor_cap[i].SensorRec.DataFormat.Range.NormalMax.Interpreted,
					interpreted++);

			} else
				printf("SA_HPI_SENSOR_RANGE_READING_INTERPRETED:something terrible has happened\n");
			vars = vars->next_variable;
		}
		/* SA_HPI_SENSOR_RANGE_READING_EVENT_SENSOR */
		SaHpiSensorEvtStatusT *event_status;
		for (i = 0; i < get_value.integer; i++) {
			if (vars->type == ASN_OCTET_STR) {
				event_status = (SaHpiSensorEvtStatusT *)vars->val.string;
				/* MAX */
				set_event(&sahpi_sensor_cap[i].SensorRec.DataFormat.Range.Max.EventStatus,
					  event_status++);
				/* MIN */
				set_event(&sahpi_sensor_cap[i].SensorRec.DataFormat.Range.Min.EventStatus,
						event_status++);
				/* NOMINAL */
				set_event(&sahpi_sensor_cap[i].SensorRec.DataFormat.Range.Nominal.EventStatus,
						event_status++);

				/* NOMINAL_MAX */
				set_event(&sahpi_sensor_cap[i].SensorRec.DataFormat.Range.NormalMax.EventStatus,
						event_status++);

				/*NOMINAL_MIN */
				set_event(&sahpi_sensor_cap[i].SensorRec.DataFormat.Range.NormalMax.EventStatus,
						event_status++);

			} else
				printf("SA_HPI_SENSOR_RANGE_READING_EVENT_SENSOR:something terrible has happened\n");
			vars = vars->next_variable;
		}
		/* SA_HPI_SENSOR_THRESHOLD_DEFINITION_IS_THRESHOLD */
		for (i = 0; i < get_value.integer; i++) {
			if (vars->type == ASN_INTEGER)
				sahpi_sensor_cap[i].SensorRec.ThresholdDefn.IsThreshold = 
					(*vars->val.integer == 1) ? SAHPI_TRUE : SAHPI_FALSE;
			else
				printf("SA_HPI_SENSOR_THRESHOLD_DEFINTION_IS_THRESHOLD:something terrible has happened\n");
			vars = vars->next_variable;
		}
		/* SA_HPI_SENSOR_THRESHOLD_DEFINITION_HOLD_CAPABILITIES */
		for (i = 0; i < get_value.integer; i++) {
			if (vars->type == ASN_UNSIGNED)
				sahpi_sensor_cap[i].SensorRec.ThresholdDefn.TholdCapabilities = 
					(SaHpiSensorThdCapT)*vars->val.integer;
			else
				printf("SA_HPI_SENSOR_THRESHOLD_DEFINITION_HOLD_CAPABILITIES:something terrible has happened\n");
			vars = vars->next_variable;
		}
		/* SA_HPI_SENSOR_THRESHOLD_DEFINITION_READ_THOLD */
		for (i = 0; i < get_value.integer; i++) {
			if (vars->type == ASN_UNSIGNED)
				sahpi_sensor_cap[i].SensorRec.ThresholdDefn.ReadThold = 
					(SaHpiSensorThdMaskT)ntohl(*vars->val.integer);
			else
				printf("SA_HPI_SENSOR_THRESHOLD_DEFINITION_READ_THOLD:something terrible has happened\n");
			vars = vars->next_variable;
		}
		/* SA_HPI_SENSOR_THRESHOLD_DEFINITION_WRITE_THOLD */
		for (i = 0; i < get_value.integer; i++) {
			if (vars->type == ASN_UNSIGNED)
				sahpi_sensor_cap[i].SensorRec.ThresholdDefn.WriteThold = 
	        			(SaHpiSensorThdMaskT)ntohl(*vars->val.integer);
			else
				printf("SA_HPI_SENSOR_THRESHOLD_DEFINITION_WRITE_THOLD:something terrible has happened\n");
			vars = vars->next_variable;
		}
		/* SA_HPI_SENSOR_THRESHOLD_DEFINITION_FIXED_THOLD */
		for (i = 0; i < get_value.integer; i++) {
			if (vars->type == ASN_UNSIGNED)
				sahpi_sensor_cap[i].SensorRec.ThresholdDefn.FixedThold = 
					(SaHpiSensorThdMaskT)ntohl(*vars->val.integer);
			else
				printf("SA_HPI_SENSOR_THRESHOLD_DEFINITION_FIXED_THOLD:something terrible has happened\n");
			vars = vars->next_variable;
		}
		/* SA_HPI_SENSOR_THRESHOLD_RAW */
		for (i = 0; i < get_value.integer; i++) {
			vars = vars->next_variable;
		}
		/* SA_HPI_SENSOR_THRESHOLD_INTERPRETED */
		for (i = 0; i < get_value.integer; i++) {
			vars = vars->next_variable;
		}
		/* SA_HPI_SENSOR_OEM */
		int Oem;
		for (i = 0; i < get_value.integer; i++) {
			if (vars->type == ASN_UNSIGNED)	{
				Oem = *vars->val.string;
				sahpi_sensor_cap[i].SensorRec.Oem = 
					(SaHpiUint32T)ntohl(Oem);
			} else
				printf("SA_HPI_SENSOR_OEM:something terrible has happened\n");
			vars = vars->next_variable;
		}
		/* SENSOR CAPABILITIES ADD TO rdr_cache */
		int ii = 0;
		for( i = 0; i < num_rdrs; i++) {
			if (rdr_cache[i].RdrType == SAHPI_SENSOR_RDR ) {
				rdr_cache[i].RdrTypeUnion.SensorRec = sahpi_sensor_cap[ii].SensorRec;
				ii++;
				if (ii > get_value.integer) {
					dbg("Number of RDRs of type CTRL exceeds discovered CTRL caps");
					status = SA_ERR_HPI_ERROR;
					break;
				}
			}
		}

 	} else {
		dbg("Sever Error: SA_HPI_SENSOR_COUNT value doesn't match Resource Capability Settings");
	}

	/* free the temporary rdr ctrl cap cache */
	if (sahpi_sensor_cap) 
		g_free(sahpi_sensor_cap);


	if (get_cap_response) 
		snmp_free_pdu(get_cap_response);
		
	return(status);

}


static void set_interpreted(SaHpiSensorInterpretedT *d, SaHpiSensorInterpretedT *s)
{
	d->Type = s->Type;

	switch (s->Type) {
	case SAHPI_SENSOR_INTERPRETED_TYPE_UINT8:
		d->Value.SensorUint8 = s->Value.SensorUint8;
		break;
	case SAHPI_SENSOR_INTERPRETED_TYPE_UINT16:
		d->Value.SensorUint16 = s->Value.SensorUint16;
		d->Value.SensorUint16 = ntohs(d->Value.SensorUint16);
		break;
	case SAHPI_SENSOR_INTERPRETED_TYPE_UINT32:
		d->Value.SensorUint32 = s->Value.SensorUint32;
		d->Value.SensorUint32 = ntohl(d->Value.SensorUint32);
		break;
	case SAHPI_SENSOR_INTERPRETED_TYPE_INT8:
		d->Value.SensorInt8 = s->Value.SensorInt8;
		break;
	case SAHPI_SENSOR_INTERPRETED_TYPE_INT16:
		d->Value.SensorInt16 = s->Value.SensorInt16;
		d->Value.SensorInt16 = ntohs(d->Value.SensorInt16);
		break;
	case SAHPI_SENSOR_INTERPRETED_TYPE_INT32:
		d->Value.SensorInt32 = s->Value.SensorInt32;
		d->Value.SensorInt32 = ntohl(d->Value.SensorInt32);
		break;
	case SAHPI_SENSOR_INTERPRETED_TYPE_FLOAT32:
		d->Value.SensorFloat32 = s->Value.SensorFloat32;
		d->Value.SensorFloat32 = ntohl(d->Value.SensorFloat32);
		break;
	case SAHPI_SENSOR_INTERPRETED_TYPE_BUFFER:
		memcpy(&d->Value.SensorBuffer, &s->Value.SensorBuffer, SAHPI_SENSOR_BUFFER_LENGTH);
		break;
	default:
		dbg("set_interpreted() ERROR: unknown SaHpiSensorInterpretedT .Type");
		break;
	}
}

static void set_event(SaHpiSensorEvtStatusT *d, SaHpiSensorEvtStatusT *e)
{
	/* set Event Status */
	d->EventStatus = e->EventStatus;

	/* set Sensor Status */
	d->SensorStatus = d->SensorStatus;
	d->SensorStatus = ntohs(d->SensorStatus);
}

int get_inventory_capabilities(struct snmp_session *ss, 
			  SaHpiRdrT *rdr_cache, 
			  int *rdr_type_count, 
			  int num_rdrs)
{
	struct snmp_pdu *get_cap_pdu = NULL;
	struct snmp_pdu *get_cap_response = NULL;
	int status = SA_OK;
	unsigned int i;
	struct variable_list *vars;
	struct snmp_value get_value;

	SaHpiRdrTypeUnionT *sahpi_inventory_cap = NULL;
	
	/* get SAHPI_INVENTORY_RDR data */
	snmp_get(ss, SA_HPI_INVENTORY_COUNT, &get_value);
	if (get_value.integer == rdr_type_count[SAHPI_INVENTORY_RDR]) {
		status = snmp_get_bulk(ss, SA_HPI_INVENTORY_ENTRY, get_cap_pdu, &get_cap_response);

		/* begin filling in table */
		sahpi_inventory_cap = g_malloc0(get_value.integer * sizeof(*sahpi_inventory_cap));		

		vars = get_cap_response->variables;

		/* SA_HPI_INVENTORY_EIRID */
		for (i = 0; i < get_value.integer; i++) {
			if (vars->type == ASN_UNSIGNED)	 {

/*	INDEX	{ saHpiDomainID, saHpiResourceID, saHpiInventoryEirId, saHpiInventoryIndex }   */
/* going to need this to store remote inventory record info like domain, resourceid, Eirid ... */

				sahpi_inventory_cap[i].InventoryRec.EirId = 
					(SaHpiEirIdT)*vars->val.integer;

printf(" caps str, EirID value %d\n", sahpi_inventory_cap[i].InventoryRec.EirId);
printf(" vars, EirID value %d\n", (SaHpiEirIdT)*vars->val.integer);

			} else
				printf("SA_HPI_INVENTORY_EIRID:something terrible has happened\n");
			vars = vars->next_variable;
		
		}

dbg("WHERES the OEM DATA COME FROM??????");
		/* SA_HPI_INVENTORY_OEM */ 
/*
		for (i = 0; i < get_value.integer; i++) {
			if (vars->type == ASN_UNSIGNED)	 {
				EirId = *vars->val.integer;
				sahpi_inventory_cap[i].InventoryRec.Oem = 
					(SaHpiUint32T)*vars->val.integer;
				sahpi_inventory_cap[i].InventoryRec.Oem = 
					ntohl(sahpi_inventory_cap[i].InventoryRec.Oem);
			} else
				printf("SA_HPI_INVENTORY_OEM:something terrible has happened\n");
			vars = vars->next_variable;
		}
*/

		/* INVENTORY CAPABILITIES ADD TO rdr_cache */
		int ii = 0;
		for( i = 0; i < num_rdrs; i++) {
			if (rdr_cache[i].RdrType == SAHPI_INVENTORY_RDR ) {
				rdr_cache[i].RdrTypeUnion.InventoryRec = sahpi_inventory_cap[ii].InventoryRec;
				ii++;
				if (ii > get_value.integer) {
					dbg("Number of RDRs of type CTRL exceeds discovered CTRL caps");
					status = SAHPI_INVENTORY_RDR;
					break;
				}
			}
		}


	}
	else {
		dbg("Sever Error: SA_HPI_INVENTORY_COUNT value doesn't match Resource Capability Settings");
	}

	/* free temporary sahpi_inventory_cap buffer */
	if (sahpi_inventory_cap) 
		g_free(sahpi_inventory_cap);


	if (get_cap_response) 
		snmp_free_pdu(get_cap_response);

	return(status);

}

int get_watchdog_capabilities(struct snmp_session *ss, 
			  SaHpiRdrT *rdr_cache, 
			  int *rdr_type_count, 
			  int num_rdrs)
{
	struct snmp_pdu *get_cap_pdu = NULL;
	struct snmp_pdu *get_cap_response = NULL;
	int status = SA_OK;
//	int i = 0;
	struct variable_list *vars;
	struct snmp_value get_value;

	SaHpiRdrTypeUnionT *sahpi_watchdog_cap = NULL;
	
	/* get SAHPI_WATCHDOG_RDR data */
	snmp_get(ss, SA_HPI_WATCHDOG_COUNT, &get_value);
	if (get_value.integer == rdr_type_count[SAHPI_WATCHDOG_RDR]) {
		status = snmp_get_bulk(ss, SA_HPI_WATCHDOG_ENTRY, get_cap_pdu, &get_cap_response);

		/* begin filling in table */
		sahpi_watchdog_cap = g_malloc0(get_value.integer * sizeof(*sahpi_watchdog_cap));
		vars = get_cap_response->variables;

	}
	else {
		dbg("Sever Error: SA_HPI_WATCHDOG_COUNT value doesn't match Resource Capability Settings");
	}

	/* free temporary sahpi_inventory_cap buffer */
	if (sahpi_watchdog_cap) 
		g_free(sahpi_watchdog_cap);

	if (get_cap_response) 
		snmp_free_pdu(get_cap_response);

	return(status);

}









