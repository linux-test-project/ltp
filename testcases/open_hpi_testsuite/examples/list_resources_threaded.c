#include <stdlib.h>	   
#include <stdio.h>
#include <SaHpi.h>
#include <unistd.h>
#include <string.h>
#include <glib.h>

#include <openhpi.h>
		     
#include <pthread.h>
#include <wait.h>
		     
/* debug macros */
#define warn(str) fprintf(stderr,"%s: " str "\n", __FUNCTION__)
#define error(str, e) fprintf(stderr,str ": %s\n", get_error_string(e))

/* Function prototypes */
void *discover_domain(void *arg);
const char * get_error_string(SaErrorT);
void display_entity_capabilities(SaHpiCapabilitiesT);
const char * severity2str(SaHpiSeverityT);
const char * type2string(SaHpiEntityTypeT type);
const char * rdrtype2str(SaHpiRdrTypeT type);
const char * rpt_cap2str(SaHpiCapabilitiesT ResourceCapabilities);
const char * get_sensor_type(SaHpiSensorTypeT type);
void list_rdr(SaHpiSessionIdT session_id, SaHpiResourceIdT resource_id);
void display_id_string(SaHpiTextBufferT string);

/* thread related resources */
#define NUM_PTHREAD 	100
#define NUM_RES_DIS	100
void *pthread_event_get(void *arg);
int done = 0;
int event_get_error_count = 0;
int discover_domain_err = FALSE;
int domain_discover_compare_err = 0;
pthread_t pthread_id_array[NUM_PTHREAD];
struct discover_domain_args {
	SaHpiDomainIdT 	domain_id; 
	SaHpiSessionIdT session_id; 
	SaHpiRptEntryT 	entry; 

};
int manage_rpt_entries(GSList **rpt_entries, GSList **last_rpt_entries, SaHpiRptEntryT *entry, int rpt_action);
enum {	ADD,COMPARE, SAVE_LAST_LIST
} RPT_ACTION;


/**
 * main
 * 
 * 
 * @return int 
 * @param argv
 */
int main(int arc, const char *argv[])
{
        SaErrorT 		err;
        SaHpiSessionIdT 	session_id = 0;
        SaHpiRptEntryT		entry;
        //SaHpiSelInfoT		Info;
	SaHpiVersionT		version;

	int i;	   

	struct discover_domain_args args;     

	if (g_thread_supported() == TRUE)  {
		printf("g_thread_supported TRUE\n");
		/* g_thread_init(NULL); */
	}
	else
		printf("g_thread_supported FALSE\n");
		

			/* First step in HPI and openhpi */
        err = saHpiInitialize(&version);
        if (SA_OK != err) {
                error("saHpiInitialize", err);
                exit(-1);
        }

        /* Every domain requires a new session */
	/* This example is for one domain, one session */
        err = saHpiSessionOpen(SAHPI_DEFAULT_DOMAIN_ID, &session_id, NULL);
        if (SA_OK != err) {
                error("saHpiSessionOpen", err);
                return -1;
        }


	args.domain_id = SAHPI_DEFAULT_DOMAIN_ID;
	args.session_id = session_id;
	memcpy(&args.entry, &entry, sizeof(SaHpiRptEntryT));
        
	for(i = 0; i < NUM_PTHREAD; i++) { 
		pthread_create((pthread_t *)&pthread_id_array[i], NULL, discover_domain, (void *)&args); 

		printf("******************\n");
		printf("pthread num: %d\n", (int)pthread_id_array[i]); 
		printf("Session ID: %d\n", args.session_id);
	}

	void *p =NULL;
	for(i = 0; i < NUM_PTHREAD; i++)  
		pthread_join(pthread_id_array[i], p);
	int chr;
	printf("Press Any Key to Continue\n");
	chr = getchar();


	pthread_t thread_id;
	pthread_create((pthread_t *)&thread_id, NULL, pthread_event_get, (void *)&args);
	printf("pthread id: %d\n", (int)thread_id);

	printf("Press Any Key to End\n");
	chr = getchar();
	done = 1;

	pthread_join(thread_id, p);
 	
        err = saHpiFinalize();
        if (SA_OK != err) {
                error("saHpiFinalize", err);
                exit(-1);

	}

	/* analysis of 100 thread discover_domain run */
	printf("***************************************\n");
	if (discover_domain_err == TRUE) {
		printf("discover_domain_err TRUE\n");
	}
	else {
		printf("discover_domain_err FALSE\n");
	}
	/* Total Data Access Mutex Collisions */
	printf("***************************************\n");
	printf("Total domain_discover_compare_err: %d\n", domain_discover_compare_err);

	/* analysis of pthread_event_get run */
	printf("***************************************\n");
	printf("saHpiEventGet Event Mis-Compares:%d\n", event_get_error_count);

	/* Total Data Access Mutex Collisions */
	printf("***************************************\n");
	printf("Total Data Access Mutex Collisions: %d\n", data_access_block_times());

	return 0;
}

/**
 * pthread_event_get: 
 * @arg: struct contains all necessary data
 *
 * thread spawned after all 
 * discover_domain threads exit, 
 * this thread loops until any key is pressed
 * each loop queries plugin for doamin events
 *
 * Return Value: void
 */
void *pthread_event_get(void *arg)
{
	static int count;
	SaHpiEventT 	event;
	SaHpiEventT	event_last;
	SaHpiRdrT 	Rdr;
	SaHpiRptEntryT 	RptEntry;

	SaHpiDomainIdT domain_id;
	SaHpiSessionIdT session_id; 
	SaHpiRptEntryT entry;

	count++;
	
	struct discover_domain_args *ptr;
	ptr = (struct discover_domain_args *)arg;

	domain_id = ptr->domain_id;
	session_id = ptr->session_id;
	memcpy(&entry, &ptr->entry, sizeof(SaHpiRptEntryT));

	printf(" session id: %d", ptr->session_id);
	printf(" session id: %d", session_id);
	printf(" count: %d\n", count);
   
	SaErrorT val;
	int ft = TRUE;
	val = saHpiSubscribe(session_id, SAHPI_TRUE);
	if (val == SA_OK) {
               while (!done) {
		       printf("saHpiEventGet \n");
		       saHpiEventGet(session_id, SAHPI_TIMEOUT_BLOCK, &event, &Rdr, &RptEntry);
		       printf("*********************************************\n");
		       printf("***UserEvent:\n");
		       printf("UserEventData: %s\n", (char *)event.EventDataUnion.UserEvent.UserEventData);
		       printf("EventType: %x\n", (int) event.EventType);
		       printf("Severity: %x\n", (int) event.Severity);
		       printf("Resource ID: %x\n", (int) event.Source);
		       printf("Timestamp: %d\n", (int)event.Timestamp);
		       printf("*********************************************\n");
		       if (memcmp(&event_last, &event, sizeof(SaHpiEventT)) && ft != TRUE )
			       event_get_error_count++;
		       memcpy(&event_last, &event, sizeof(SaHpiEventT));
		       ft = FALSE;
		       sleep(0.1);
	       }
       }

       val = saHpiUnsubscribe(session_id);
   
   return (arg);
}



/**
 * thread function for first set fo threads spawned, 
 * this thread loops NUM_RES_DIS time, each time redisovering 
 * resources and rdrs, and compares the present result 
 * to previous loops results.
 * 
 * 
 * @return void *
 * @param arg, pointer to structure of data necessary for thread execution, 
 * see struct discover_domain_args
 */
void *discover_domain(void *arg)
{
	
	SaErrorT		err;
        SaHpiRptInfoT        	rpt_info_before;
        SaHpiEntryIdT        	current;
        SaHpiEntryIdT        	next;

	SaHpiDomainIdT domain_id;
	SaHpiSessionIdT session_id; 
	SaHpiRptEntryT entry;

	/* used for comparing runs of rpt_entry gets */
	GSList *rpt_entries = NULL;
	GSList *last_rpt_entries = NULL;


	struct discover_domain_args *ptr;
	ptr = (struct discover_domain_args *)arg;
	domain_id = ptr->domain_id;
	session_id = ptr->session_id;
	memcpy(&entry, &ptr->entry, sizeof(SaHpiRptEntryT));

	int c;
	int loop_times = NUM_RES_DIS;
	int ft = TRUE;
	for (c=1; c <= loop_times; c++) {

		err = saHpiResourcesDiscover(session_id);
		if (SA_OK != err) {
			error("saHpiResourcesDiscover", err);
			break;
		}
       
		/* grab copy of the update counter before traversing RPT */
		err = saHpiRptInfoGet(session_id, &rpt_info_before);
		if (SA_OK != err) {
			error("saHpiRptInfoGet", err);
			break;
		}
            
		warn("Scanning RPT...");
		next = SAHPI_FIRST_ENTRY;
		do {
			int i;
			current = next;
			err = saHpiRptEntryGet(session_id, current, &next, &entry);
			if (SA_OK != err) {
				if (current != SAHPI_FIRST_ENTRY) {
					error("saHpiRptEntryGet", err);
                             	   return NULL;
				} else {
					warn("Empty RPT\n");
					break;
				}
			}
      			printf("                       address   rpt_entries %p\n", rpt_entries);
			manage_rpt_entries(&rpt_entries, &last_rpt_entries, &entry, ADD);
			printf("                       address   rpt_entries %p\n", rpt_entries);

			
			printf("***Records:\n");
			printf("%s\n", (char *)entry.ResourceTag.Data);
			printf("Entry ID: %x\n", (int) entry.EntryId);
			printf("Resource ID: %x\n", (int) entry.ResourceId);
			printf("Domain ID: %x\n", (int) entry.DomainId);
			printf("Revision: %c\n", entry.ResourceInfo.ResourceRev);
			printf("Version: %c\n", entry.ResourceInfo.SpecificVer);
			printf("Severity: %s\n",severity2str(entry.ResourceSeverity));
			printf("Resource Capability: %s\n", rpt_cap2str(entry.ResourceCapabilities));

			printf("Entity Path:\n");
			for ( i=0; i<SAHPI_MAX_ENTITY_PATH; i++ )
			{
				SaHpiEntityT tmp = entry.ResourceEntity.Entry[i];
				if (tmp.EntityType <= SAHPI_ENT_UNSPECIFIED)
					break;

				printf("\t{%s, %i}\n", type2string(tmp.EntityType),tmp.EntityInstance);
			}

			if (entry.ResourceCapabilities & SAHPI_CAPABILITY_RDR) 
				list_rdr(session_id, entry.ResourceId);
                
		} while (next != SAHPI_LAST_ENTRY);

		printf("SAHPI_LAST_ENTRY\n");

       		if ((manage_rpt_entries(&rpt_entries, &last_rpt_entries, &entry, COMPARE) == -1) && (ft != TRUE))
			domain_discover_compare_err++;
		ft = FALSE;
       		manage_rpt_entries(&rpt_entries, &last_rpt_entries, &entry, SAVE_LAST_LIST);

	}

	/* identify if break; called for early termination of for loop */
	if (loop_times < 100 ) {
		discover_domain_err = TRUE;
	}

	printf("END OF THREAD END OF THREAD END OF THREAD END OF THREAD END OF THREAD END OF THREAD \n");
	printf("END OF THREAD END OF THREAD END OF THREAD END OF THREAD END OF THREAD END OF THREAD \n");
	printf("END OF THREAD END OF THREAD END OF THREAD END OF THREAD END OF THREAD END OF THREAD \n");
	printf("END OF THREAD END OF THREAD END OF THREAD END OF THREAD END OF THREAD END OF THREAD \n");
	printf("END OF THREAD END OF THREAD END OF THREAD END OF THREAD END OF THREAD END OF THREAD \n");

        return arg; 

}

/**
 * returns severity string
 *
 * 
 * @return severity type as const
 * @param severity
 */
const char * severity2str(SaHpiSeverityT severity)
{
        switch (severity) {
        case SAHPI_CRITICAL:
                return "CRITICAL";
        case SAHPI_MAJOR:
                return "MAJOR";
        case SAHPI_MINOR:
                return "MINOR";
        case SAHPI_INFORMATIONAL:
                return "INFORMATIONAL";               
        case SAHPI_OK:
                return "OK";
        case SAHPI_DEBUG:
                return "DEBUG";
        default:
                return "UNKNOWN SEVERITY";
        }
}

/**
 * gets error as a string
 * 
 * 
 * @return error as cont string
 * @param errro error value
 */
const char * get_error_string(SaErrorT error)
{
        switch(error) {
                case SA_ERR_HPI_ERROR:
                        return "SA_ERR_HPI_ERROR";
                case SA_ERR_HPI_UNSUPPORTED_API:
                        return "SA_ERR_UNSUPPORTED_API";
                case SA_ERR_HPI_BUSY:
                        return "SA_ERR_HPI_BUSY";
                case SA_ERR_HPI_INVALID:
                        return "SA_ERR_HPI_INVALID";
                case SA_ERR_HPI_INVALID_CMD:
                        return "SA_ERR_HPI_INVALID_CMD";
                case SA_ERR_HPI_TIMEOUT:
                        return "SA_ERR_HPI_TIMEOUT";
                case SA_ERR_HPI_OUT_OF_SPACE:
                        return "SA_ERR_HPI_OUT_OF_SPACE";
                case SA_ERR_HPI_DATA_TRUNCATED:
                        return "SA_ERR_HPI_DATA_TRUNCATED";
                case SA_ERR_HPI_DATA_LEN_INVALID:
                        return "SA_ERR_HPI_DATA_LEN_INVALID";
                case SA_ERR_HPI_DATA_EX_LIMITS:
                        return "SA_ERR_HPI_DATA_EX_LIMITS";
                case SA_ERR_HPI_INVALID_PARAMS:
                        return "SA_ERR_HPI_INVALID_PARAMS";
                case SA_ERR_HPI_INVALID_DATA:
                        return "SA_ERR_HPI_INVALID_DATA";
                case SA_ERR_HPI_NOT_PRESENT:
                        return "SA_ERR_HPI_NOT_PRESENT";
                case SA_ERR_HPI_INVALID_DATA_FIELD:
                        return "SA_ERR_HPI_INVALID_DATA_FIELD";
                case SA_ERR_HPI_INVALID_SENSOR_CMD:
                        return "SA_ERR_HPI_INVALID_SENSOR_CMD";
                case SA_ERR_HPI_NO_RESPONSE:
                        return "SA_ERR_HPI_NO_RESPONSE";
                case SA_ERR_HPI_DUPLICATE:
                        return "SA_ERR_HPI_DUPLICATE";
                case SA_ERR_HPI_INITIALIZING:
                        return "SA_ERR_HPI_INITIALIZING";
                case SA_ERR_HPI_UNKNOWN:
                        return "SA_ERR_HPI_UNKNOWN";
                case SA_ERR_HPI_INVALID_SESSION:
                        return "SA_ERR_HPI_INVALID_SESSION";
                case SA_ERR_HPI_INVALID_RESOURCE:
                        return "SA_ERR_HPI_INVALID_RESOURCE";
                case SA_ERR_HPI_INVALID_REQUEST:
                        return "SA_ERR_HPI_INVALID_REQUEST";
                case SA_ERR_HPI_ENTITY_NOT_PRESENT:
                        return "SA_ERR_HPI_ENTITY_NOT_PRESENT";
                case SA_ERR_HPI_UNINITIALIZED:
                        return "SA_ERR_HPI_UNINITIALIZED";
                default:
                        return "(invalid error code)";
        }
}
   

/**
 * returns entity type
 * 
 * 
 * @return returns entity type as a const string
 * @param type
 */
const char * type2string(SaHpiEntityTypeT type)
{
        switch(type) {
        case SAHPI_ENT_UNSPECIFIED:
                return "SAHPI_ENT_UNSPECIFIED";
        case SAHPI_ENT_OTHER:
                return "SAHPI_ENT_OTHER";
        case SAHPI_ENT_UNKNOWN:
                return "SAHPI_ENT_UNKNOWN";
        case SAHPI_ENT_PROCESSOR:
                return "SAHPI_ENT_PROCESSOR";
        case SAHPI_ENT_DISK_BAY:
                return "SAHPI_ENT_DISK_BAY";
        case SAHPI_ENT_PERIPHERAL_BAY:
                return "SAHPI_ENT_PERIPHERAL_BAY";
        case SAHPI_ENT_SYS_MGMNT_MODULE:
                return "SAHPI_ENT_SYS_MGMNT_MODULE";
        case SAHPI_ENT_SYSTEM_BOARD:
                return "SAHPI_ENT_SYSTEM_BOARD";
        case SAHPI_ENT_MEMORY_MODULE:
                return "SAHPI_ENT_MEMORY_MODULE";
        case SAHPI_ENT_PROCESSOR_MODULE:
                return "SAHPI_ENT_PROCESSOR_MODULE";
        case SAHPI_ENT_POWER_SUPPLY:
                return "SAHPI_ENT_POWER_SUPPLY";
        case SAHPI_ENT_ADD_IN_CARD:
                return "SAHPI_ENT_ADD_IN_CARD";
        case SAHPI_ENT_FRONT_PANEL_BOARD:
                return "SAHPI_ENT_FRONT_PANEL_BOARD";
        case SAHPI_ENT_BACK_PANEL_BOARD:
                return "SAHPI_ENT_BACK_PANEL_BOARD";
        case SAHPI_ENT_POWER_SYSTEM_BOARD:
                return "SAHPI_ENT_POWER_SYSTEM_BOARD";
        case SAHPI_ENT_DRIVE_BACKPLANE:
                return "SAHPI_ENT_DRIVE_BACKPLANE";
        case SAHPI_ENT_SYS_EXPANSION_BOARD:
                return "SAHPI_ENT_SYS_EXPANSION_BOARD";
        case SAHPI_ENT_OTHER_SYSTEM_BOARD:
                return "SAHPI_ENT_OTHER_SYSTEM_BOARD";
        case SAHPI_ENT_PROCESSOR_BOARD:
                return "SAHPI_ENT_PROCESSOR_BOARD";
        case SAHPI_ENT_POWER_UNIT:
                return "SAHPI_ENT_POWER_UNIT";
        case SAHPI_ENT_POWER_MODULE:
                return "SAHPI_ENT_POWER_MODULE";
        case SAHPI_ENT_POWER_MGMNT:
                return "SAHPI_ENT_POWER_MGMNT";
        case SAHPI_ENT_CHASSIS_BACK_PANEL_BOARD:
                return "SAHPI_ENT_CHASSIS_BACK_PANEL_BOARD";
        case SAHPI_ENT_SYSTEM_CHASSIS:
                return "SAHPI_ENT_SYSTEM_CHASSIS";
        case SAHPI_ENT_SUB_CHASSIS:
                return "SAHPI_ENT_SUB_CHASSIS";
        case SAHPI_ENT_OTHER_CHASSIS_BOARD:
                return "SAHPI_ENT_OTHER_CHASSIS_BOARD";
        case SAHPI_ENT_DISK_DRIVE_BAY:
                return "SAHPI_ENT_DISK_DRIVE_BAY";
        case SAHPI_ENT_PERIPHERAL_BAY_2:
                return "SAHPI_ENT_PERIPHERAL_BAY_2";
        case SAHPI_ENT_DEVICE_BAY:
                return "SAHPI_ENT_DEVICE_BAY";
        case SAHPI_ENT_COOLING_DEVICE:
                return "SAHPI_ENT_COOLING_DEVICE";
        case SAHPI_ENT_COOLING_UNIT:
                return "SAHPI_ENT_COOLING_UNIT";
        case SAHPI_ENT_INTERCONNECT:
                return "SAHPI_ENT_INTERCONNECT";
        case SAHPI_ENT_MEMORY_DEVICE:
                return "SAHPI_ENT_MEMORY_DEVICE";
        case SAHPI_ENT_SYS_MGMNT_SOFTWARE:
                return "SAHPI_ENT_SYS_MGMNT_SOFTWARE";
        case SAHPI_ENT_BIOS:
                return "SAHPI_ENT_BIOS";
        case SAHPI_ENT_OPERATING_SYSTEM:
                return "SAHPI_ENT_OPERATING_SYSTEM";
        case SAHPI_ENT_SYSTEM_BUS:
                return "SAHPI_ENT_SYSTEM_BUS";
        case SAHPI_ENT_GROUP:
                return "SAHPI_ENT_GROUP";
        case SAHPI_ENT_REMOTE:
                return "SAHPI_ENT_REMOTE";
        case SAHPI_ENT_EXTERNAL_ENVIRONMENT:
                return "SAHPI_ENT_EXTERNAL_ENVIRONMENT";
        case SAHPI_ENT_BATTERY:
                return "SAHPI_ENT_BATTERY";
        case SAHPI_ENT_CHASSIS_SPECIFIC:
                return "SAHPI_ENT_CHASSIS_SPECIFIC";
        case SAHPI_ENT_BOARD_SET_SPECIFIC:
                return "SAHPI_ENT_BOARD_SET_SPECIFIC";
        case SAHPI_ENT_OEM_SYSINT_SPECIFIC:
                return "SAHPI_ENT_OEM_SYSINT_SPECIFIC";
        case SAHPI_ENT_ROOT:
                return "SAHPI_ENT_ROOT";
        case SAHPI_ENT_RACK:
                return "SAHPI_ENT_RACK";
        case SAHPI_ENT_SUBRACK:
                return "SAHPI_ENT_SUBRACK";
        case SAHPI_ENT_COMPACTPCI_CHASSIS:
                return "SAHPI_ENT_COMPACTPCI_CHASSIS";
        case SAHPI_ENT_ADVANCEDTCA_CHASSIS:
                return "SAHPI_ENT_ADVANCEDTCA_CHASSIS";
        case SAHPI_ENT_SYSTEM_SLOT:
                return "SAHPI_ENT_SYSTEM_SLOT";
        case SAHPI_ENT_SBC_BLADE:
                return "SAHPI_ENT_SBC_BLADE";
        case SAHPI_ENT_IO_BLADE:
                return "SAHPI_ENT_IO_BLADE";
        case SAHPI_ENT_DISK_BLADE:
                return "SAHPI_ENT_DISK_BLADE";
        case SAHPI_ENT_DISK_DRIVE:
                return "SAHPI_ENT_DISK_DRIVE";
        case SAHPI_ENT_FAN:
                return "SAHPI_ENT_FAN";
        case SAHPI_ENT_POWER_DISTRIBUTION_UNIT:
                return "SAHPI_ENT_POWER_DISTRIBUTION_UNIT";
        case SAHPI_ENT_SPEC_PROC_BLADE:
                return "SAHPI_ENT_SPEC_PROC_BLADE";
        case SAHPI_ENT_IO_SUBBOARD:
                return "SAHPI_ENT_IO_SUBBOARD";
        case SAHPI_ENT_SBC_SUBBOARD:
                return "SAHPI_ENT_SBC_SUBBOARD";
        case SAHPI_ENT_ALARM_MANAGER:
                return "SAHPI_ENT_ALARM_MANAGER";
        case SAHPI_ENT_ALARM_MANAGER_BLADE:
                return "SAHPI_ENT_ALARM_MANAGER_BLADE";
        case SAHPI_ENT_SUBBOARD_CARRIER_BLADE:
                return "SAHPI_ENT_SUBBOARD_CARRIER_BLADE";
        default:
                return "(invalid entity type)";
        }
        return "\0";
}


/**
 * returns rdr type from string
 * 
 * 
 * @return returns rdr type as a const string
 * @author type
 */
const char * rdrtype2str(SaHpiRdrTypeT type)
{
        switch (type) {
        case SAHPI_NO_RECORD:
                return "SAHPI_NO_RECORD";
        case SAHPI_CTRL_RDR:
                return "SAHPI_CTRL_RDR";
        case SAHPI_SENSOR_RDR:
                return "SAHPI_SENSOR_RDR";
        case SAHPI_INVENTORY_RDR:
                return "SAHPI_INVENTORY_RDR";
        case SAHPI_WATCHDOG_RDR:
                return "SAHPI_WATCHDOG_RDR";
        default:
                return "(invalid rdr type)";
        }
        return "\0";
}

/**
 * formats and outputs rdr
 * 
 * 
 * @return void
 * @param session_id
 * @param resource_id
 */
void list_rdr(SaHpiSessionIdT session_id, SaHpiResourceIdT resource_id)
{
        SaErrorT             err;
        SaHpiEntryIdT        current_rdr;
        SaHpiEntryIdT        next_rdr;
        SaHpiRdrT            rdr;

        printf("RDR Info:\n");
        next_rdr = SAHPI_FIRST_ENTRY;
        do {
                int i;
                current_rdr = next_rdr;
                err = saHpiRdrGet(session_id, resource_id, current_rdr, 
                                &next_rdr, &rdr);
                if (SA_OK != err) {
                        if (current_rdr == SAHPI_FIRST_ENTRY)
                                printf("Empty RDR table\n");
                        else
                                error("saHpiRdrGet", err);
                        return;                        
                }
                
                printf("\tRecordId: %x\n", rdr.RecordId);
                printf("\tRdrType: %s\n", rdrtype2str(rdr.RdrType));
		
		if (rdr.RdrType == SAHPI_SENSOR_RDR)
		{
			SaHpiSensorReadingT	reading;
			SaHpiSensorTypeT	type;
			SaHpiSensorNumT		num;
			SaHpiEventCategoryT 	category;
			//SaHpiSensorThresholdsT	thres;
			//SaHpiSensorReadingT 	converted;
			
			SaErrorT val;
			
			num = rdr.RdrTypeUnion.SensorRec.Num;
			
			val = saHpiSensorTypeGet(session_id, resource_id, num, &type, &category);
			
			printf("\tSensor num: %i\n\tType: %s\n", num, get_sensor_type(type)); 

			err = saHpiSensorReadingGet(session_id, resource_id, num, &reading);
			if (err != SA_OK) {
				printf("Error reading sensor data {sensor, %d}", num);
				continue;
			} else {
				if (reading.ValuesPresent == SAHPI_SRF_RAW)
					printf("\tValues Present: RAW\n");
				if (reading.ValuesPresent == SAHPI_SRF_INTERPRETED)
					printf("\tValues Present: Interpreted\n");
				if (reading.ValuesPresent == SAHPI_SRF_EVENT_STATE);
					printf("\tValues Present: Event State\n");
			}  		
		}

                printf("\tEntity: \n");
                for ( i=0; i<SAHPI_MAX_ENTITY_PATH; i++)
                {
                        SaHpiEntityT tmp = rdr.Entity.Entry[i];
                        if (tmp.EntityType <= SAHPI_ENT_UNSPECIFIED)
                                break;
                                printf("\t\t{%s, %i}\n", 
                                type2string(tmp.EntityType),
                                tmp.EntityInstance);
                }
                printf("\tIdString: ");
                       display_id_string(rdr.IdString);
        }while(next_rdr != SAHPI_LAST_ENTRY);
}

/**
 * displays id string
 * 
 * 
 * @return vois
 * @param string
 */
void display_id_string(SaHpiTextBufferT string)
{
        int i;
        switch(string.DataType) {
        case SAHPI_TL_TYPE_ASCII6:
                for (i = 0; i < string.DataLength; i++)
                        printf("%c", string.Data[i]);
                break;
        default:
                printf("Unsupported string type");
        }
        printf("\n");
}

/**
 * returns capabilities as a const string
 * 
 * 
 * @return caps as a const string
 * @param ResourceCapabilities
 */
const char * rpt_cap2str (SaHpiCapabilitiesT ResourceCapabilities)
{
	switch (ResourceCapabilities) {
		case SAHPI_CAPABILITY_DOMAIN:
			return("SAHPI_CAPABILITY_DOMAIN");
		case SAHPI_CAPABILITY_RESOURCE:
			return("SAHPI_CAPABILITY_RESOURCE");
		case SAHPI_CAPABILITY_SEL:
			return("SAHPI_CAPABILITY_SEL");
		case SAHPI_CAPABILITY_EVT_DEASSERTS:
			return("SAHPI_CAPABILITY_EVT_DEASSERTS");
		case SAHPI_CAPABILITY_AGGREGATE_STATUS:
			return("#define SAHPI_CAPABILITY_AGGREGATE_STATUS");
		case SAHPI_CAPABILITY_CONFIGURATION:
			return("SAHPI_CAPABILITY_CONFIGURATION");
		case SAHPI_CAPABILITY_MANAGED_HOTSWAP:
			return("SAHPI_CAPABILITY_MANAGED_HOTSWAP");
		case SAHPI_CAPABILITY_WATCHDOG:
			return("SAHPI_CAPABILITY_WATCHDOG");
		case SAHPI_CAPABILITY_CONTROL:
			return("SAHPI_CAPABILITY_CONTROL");
		case SAHPI_CAPABILITY_FRU:
			return("SAHPI_CAPABILITY_FRU");
		case SAHPI_CAPABILITY_INVENTORY_DATA:
			return("SAHPI_CAPABILITY_INVENTORY_DATA");
		case SAHPI_CAPABILITY_RDR:
			return("SAHPI_CAPABILITY_RDR");
		case SAHPI_CAPABILITY_SENSOR:
			return("SAHPI_CAPABILITY_SENSOR");
		default:
			return("Unknown Capabilities");
	}
	return("\n");
}

/**
 * returns sensor type as a const string from type
 * 
 * 
 * @return a const string
 * @param type, sensor type
 */
const char * get_sensor_type(SaHpiSensorTypeT type) 
{
	switch(type) {
		case SAHPI_TEMPERATURE:
			return "TEMPERATURE SENSOR";
		case SAHPI_VOLTAGE:
			return "VOLTAGE SENSOR";
		case SAHPI_FAN:
			return "FAN SENSOR";
		case SAHPI_PROCESSOR:
			return "PROCESSOR";
		case SAHPI_POWER_SUPPLY:
			return "POWER SUPPLY";
		case SAHPI_POWER_UNIT:
			return "POWER UNIT";
		case SAHPI_COOLING_DEVICE:
			return "COOLING DEVICE";
		case SAHPI_MEMORY:
			return "MEMORY";
		case SAHPI_OTHER_UNITS_BASED_SENSOR:
			return "OTHER UNITS BASED SENSOR";
		case SAHPI_LAN:
			return "Lan Sensor";
    		case SAHPI_PHYSICAL_SECURITY:
			return("SAHPI_PHYSICAL_SECURITY"); 
    		case SAHPI_PLATFORM_VIOLATION:
			return("SAHPI_PLATFORM_VIOLATION"); 
    		case SAHPI_DRIVE_SLOT:
			return("SAHPI_DRIVE_SLOT"); 
    		case SAHPI_POST_MEMORY_RESIZE:
			return("SAHPI_POST_MEMORY_RESIZE"); 
    		case SAHPI_SYSTEM_FW_PROGRESS:
			return("SAHPI_SYSTEM_FW_PROGRESS"); 
    		case SAHPI_EVENT_LOGGING_DISABLED:
			return("SAHPI_EVENT_LOGGING_DISABLED"); 
    		case SAHPI_RESERVED1:
			return("SAHPI_RESERVED1"); 
    		case SAHPI_SYSTEM_EVENT:
			return("SAHPI_SYSTEM_EVENT"); 
    		case SAHPI_CRITICAL_INTERRUPT:
			return("SAHPI_CRITICAL_INTERRUPT"); 
    		case SAHPI_BUTTON:
			return("SAHPI_BUTTON"); 
    		case SAHPI_MODULE_BOARD:
			return("SAHPI_MODULE_BOARD"); 
    		case SAHPI_MICROCONTROLLER_COPROCESSOR:
			return("SAHPI_MICROCONTROLLER_COPROCESSOR"); 
    		case SAHPI_ADDIN_CARD:
			return("SAHPI_ADDIN_CARD"); 
    		case SAHPI_CHASSIS:
			return("SAHPI_CHASSIS"); 
    		case SAHPI_CHIP_SET:
			return("SAHPI_CHIP_SET"); 
    		case SAHPI_OTHER_FRU:
			return("SAHPI_OTHER_FRU"); 
    		case SAHPI_CABLE_INTERCONNECT:
			return("SAHPI_CABLE_INTERCONNECT"); 
    		case SAHPI_TERMINATOR:
			return("SAHPI_TERMINATOR"); 
    		case SAHPI_SYSTEM_BOOT_INITIATED:
			return("SAHPI_SYSTEM_BOOT_INITIATED"); 
    		case SAHPI_BOOT_ERROR:
			return("SAHPI_BOOT_ERROR"); 
    		case SAHPI_OS_BOOT:
			return("SAHPI_OS_BOOT"); 
    		case SAHPI_OS_CRITICAL_STOP:
			return("SAHPI_OS_CRITICAL_STOP"); 
    		case SAHPI_SLOT_CONNECTOR:
			return("SAHPI_SLOT_CONNECTOR"); 
    		case SAHPI_SYSTEM_ACPI_POWER_STATE:
			return("SAHPI_SYSTEM_ACPI_POWER_STATE"); 
    		case SAHPI_RESERVED2:
			return("SAHPI_RESERVED2"); 
    		case SAHPI_PLATFORM_ALERT:
			return("SAHPI_PLATFORM_ALERT"); 
    		case SAHPI_ENTITY_PRESENCE:
			return("SAHPI_ENTITY_PRESENCE"); 
    		case SAHPI_MONITOR_ASIC_IC:
			return("SAHPI_MONITOR_ASIC_IC"); 
    		case SAHPI_MANAGEMENT_SUBSYSTEM_HEALTH:
			return("SAHPI_MANAGEMENT_SUBSYSTEM_HEALTH"); 
    		case SAHPI_BATTERY:
			return("SAHPI_BATTERY"); 
    		case SAHPI_OEM_SENSOR && SAHPI_OEM_SENSOR == 0xC:
			return("SAHPI_OEM_SENSOR=0xC"); 
		default:
			return "Other";
	}

return "\0";

}


/**
 * Manages two lists for comapre present to last discovery of resources
 * @return error or successful
 * @param rpt_entries
 * @param last_rpt_entries
 * @param entry
 * @param rpt_action
 */
int manage_rpt_entries(GSList **rpt_entries, GSList **last_rpt_entries, SaHpiRptEntryT *entry, int rpt_action)
{	
	SaHpiRptEntryT *ep;
	SaHpiRptEntryT *ep2;
	GSList *i;
	GSList *i2;
	guint len = 0;
	int compare = 0;



	if (rpt_action == ADD) {
		printf("******************************************************ADD\n");
		printf("thread_self %d\n", (int)pthread_self());
		ep = (SaHpiRptEntryT *)malloc(sizeof(SaHpiRptEntryT));
		if (!ep) {
			return(-1);
		}
		memcpy((void *)ep, entry, sizeof(SaHpiRptEntryT));

		*rpt_entries = g_slist_append(*rpt_entries, ep);
		printf("                       address   rpt_entries %p\n", *rpt_entries);
		printf("                       lenght of rpt_entries %d\n", g_slist_length(*rpt_entries));
		printf("                       lenght of last_rpt_entries %d\n", g_slist_length(*last_rpt_entries));
	}

	else if (rpt_action == COMPARE) {
		printf("******************************************************COMPARE\n");
		if (g_slist_length(*rpt_entries) == g_slist_length(*last_rpt_entries)) {
			printf("rpt_entries len: %d\n", g_slist_length(*rpt_entries));
			printf("last_rpt_entries len: %d\n", g_slist_length(*last_rpt_entries));
			len = g_slist_length(*rpt_entries);
			i2 = *last_rpt_entries;
			g_slist_for_each(i, *rpt_entries) {
				if (memcmp(i2->data, i->data, sizeof(SaHpiRptEntryT)) )	{
					printf("******ERROR************************************************COMPARE\n");
					compare = -1;
				}
				i2 = g_slist_next(i2);
			}
		}
		else {
			printf("******WOEW  FAILED************************************************COMPARE\n");
			compare = -1;
		}
	}

	else if (rpt_action == SAVE_LAST_LIST) {
		printf("******************************************************SAVE_LAST_LIST\n");

		printf("CLEAR LAST_RPT_ENTRIES ************************************************\n");
		/* clear last_rpt_entries list and free memory */
		i  = NULL;
		i2 = NULL;
		g_slist_for_each_safe(i, i2, *last_rpt_entries) {
			ep = i->data;
			*last_rpt_entries = g_slist_remove_link(*last_rpt_entries, i);
			free(ep);                      
		}

		printf("COPY RPT_ENTRIES INTO LAST_RPT_ENTRIES ********************************\n");
		/* copy pointers for data/memory allocated and manatined by rpt_entries */
		g_slist_for_each(i, *rpt_entries) {
		
		ep2 = (SaHpiRptEntryT *)malloc(sizeof(SaHpiRptEntryT));
		if (!ep2) return(-1);
		memcpy((void *)ep2, i->data, sizeof(SaHpiRptEntryT));

		    *last_rpt_entries = g_slist_append(*last_rpt_entries, ep2);
		}


		printf("CLEAR RPT_ENTRIES ******************************************************\n");
		/* clear rpt_entries list */
		i  = NULL;
		i2 = NULL;
		g_slist_for_each_safe(i, i2, *rpt_entries) {
			ep = i->data;
			*rpt_entries = g_slist_remove_link(*rpt_entries, i);
			free(ep);                      
		}
	}
	return(compare);

}


