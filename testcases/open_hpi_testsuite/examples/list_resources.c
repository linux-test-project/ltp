/*      -*- linux-c -*-
 */

#include <stdlib.h>
#include <stdio.h>
#include <SaHpi.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <epath_utils.h>

/* debug macros */
#define warn(str) fprintf(stderr,"%s: " str "\n", __FUNCTION__)
#define error(str, e) fprintf(stderr,str ": %s\n", get_error_string(e))

/* Function prototypes */
SaErrorT discover_domain(SaHpiDomainIdT, SaHpiSessionIdT, SaHpiRptEntryT);
const char * get_error_string(SaErrorT);
void display_entity_capabilities(SaHpiCapabilitiesT);
const char * severity2str(SaHpiSeverityT);
const char * rdrtype2str(SaHpiRdrTypeT type);
void interpreted2str (SaHpiSensorInterpretedT interpreted);
void rpt_cap2str(SaHpiCapabilitiesT ResourceCapabilities);
const char * ctrldigital2str(SaHpiCtrlStateDigitalT digstate);
const char * get_sensor_type(SaHpiSensorTypeT type);
const char * get_control_type(SaHpiCtrlTypeT type);
const char * get_sensor_category(SaHpiEventCategoryT category);
void list_sel(SaHpiSessionIdT session_id, SaHpiResourceIdT resource_id);
void list_rdr(SaHpiSessionIdT session_id, SaHpiResourceIdT resource_id);
void display_textbuffer(SaHpiTextBufferT string);
void display_oembuffer(SaHpiUint32T length, SaHpiUint8T *string);
void printreading (SaHpiSensorReadingT reading);
void time2str( SaHpiTimeT time, char *str );
const char *eventtype2str(SaHpiEventTypeT type);
const char *hotswapstate2str(SaHpiHsStateT state);


/**
 * main: main program loop
 *
 *
 *
 * Return value: int 0
 **/
int main(int arc, const char *argv[])
{
        SaErrorT 		err;
        SaHpiVersionT		version;
        SaHpiSessionIdT 	session_id;
        SaHpiRptEntryT		entry;
        //SaHpiSelInfoT		Info;
        
	
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
                return err;
        }
        
        err = discover_domain(SAHPI_DEFAULT_DOMAIN_ID, session_id, entry);
        if (SA_OK != err) {
                warn("an error was encountered, results may be incomplete");
        }

        saHpiSessionClose(session_id);

        err = saHpiFinalize();
        if (SA_OK != err) {
                error("saHpiFinalize", err);
                exit(-1);

	}
	return 0;
}

SaErrorT discover_domain(SaHpiDomainIdT domain_id, SaHpiSessionIdT session_id, SaHpiRptEntryT entry)
{
	
	SaErrorT	err;
	SaHpiRptInfoT	rpt_info_before;
	SaHpiRptInfoT   rpt_info_after;
	SaHpiEntryIdT	current;
	SaHpiEntryIdT   next;

        err = saHpiResourcesDiscover(session_id);
        if (SA_OK != err) {
                error("saHpiResourcesDiscover", err);
                return err;
        }
 	warn("list_resources: discover done");       
        /* grab copy of the update counter before traversing RPT */
        err = saHpiRptInfoGet(session_id, &rpt_info_before);
        if (SA_OK != err) {
                error("saHpiRptInfoGet", err);
                return err;
        }
        
        warn("Scanning RPT...");
        next = SAHPI_FIRST_ENTRY;
        do {
                char tmp_epath[128];
                current = next;
                err = saHpiRptEntryGet(session_id, current, &next, &entry);
                if (SA_OK != err) {
                        if (current != SAHPI_FIRST_ENTRY) {
                                error("saHpiRptEntryGet", err);
                                return err;
                        } else {
                                warn("Empty RPT\n");
                                break;
                        }
                }

		printf("***Records:\n");
                printf("%s\n", (char *)entry.ResourceTag.Data);
                printf("Entry ID: %d\n", (int) entry.EntryId);
                printf("Resource ID: %d\n", (int) entry.ResourceId);
                printf("Domain ID: %d\n", (int) entry.DomainId);
                printf("Revision: %c\n", entry.ResourceInfo.ResourceRev);
                printf("Version: %c\n", entry.ResourceInfo.SpecificVer);
		printf("Device Support: %c\n", entry.ResourceInfo.DeviceSupport);
		printf("Manufacturer ID: %d\n", (int) entry.ResourceInfo.ManufacturerId);
		printf("Product ID: %d\n", (int) entry.ResourceInfo.ProductId);
		printf("Firmware Major, Minor, Aux: %c %c %c\n", 
		       entry.ResourceInfo.FirmwareMajorRev, 
		       entry.ResourceInfo.FirmwareMinorRev,
		       entry.ResourceInfo.AuxFirmwareRev);
                printf("Severity: %s\n",severity2str(entry.ResourceSeverity));
                
                rpt_cap2str(entry.ResourceCapabilities);
                                
                printf("Entity Path:\n");
                entitypath2string(&entry.ResourceEntity, tmp_epath, sizeof(tmp_epath));
                printf("\t%s\n", tmp_epath);
                printf("\tResourceTag: ");
                       display_textbuffer(entry.ResourceTag);

                if (entry.ResourceCapabilities & SAHPI_CAPABILITY_RDR)
                        list_rdr(session_id, entry.ResourceId);

                if (entry.ResourceCapabilities & SAHPI_CAPABILITY_SEL)
                        list_sel(session_id, entry.ResourceId);
#if 0
                /* if the resource is also a domain, then 
                 * traverse its RPT */        
                if (entry.ResourceCapabilities & SAHPI_CAPABILITY_DOMAIN) {
                        err = discover_domain(entry.DomainId);
                        if (SA_OK != err)
                                return err;
                }
#endif
                printf("\tEntryId: %d\n", next);
        } while (next != SAHPI_LAST_ENTRY);

	printf("SAHPI_LAST_ENTRY\n");
		
        /* wait for update in RPT */
        while (1) {
                err = saHpiRptInfoGet(session_id, &rpt_info_after);
                if (SA_OK != err) {
                        error("saHpiRptInfoGet", err);
                        return err;
                }
                if (rpt_info_before.UpdateCount != rpt_info_after.UpdateCount) {
                        rpt_info_before = rpt_info_after;
                } else
			break;
        };

        return SA_OK;
}

const char * ctrldigital2str(SaHpiCtrlStateDigitalT digstate) 
{
        switch (digstate) {
	case SAHPI_CTRL_STATE_OFF:
                return "SAHPI_CTRL_STATE_OFF";
	case SAHPI_CTRL_STATE_ON:
                return "SAHPI_CTRL_STATE_ON";
	case SAHPI_CTRL_STATE_PULSE_OFF:
                return "SAHPI_CTRL_STATE_PULSE_OFF";
	case SAHPI_CTRL_STATE_PULSE_ON:
                return "SAHPI_CTRL_STATE_PULSE_ON";
	case SAHPI_CTRL_STATE_AUTO:
                return "SAHPI_CTRL_STATE_AUTO";
        default:
		printf("Invalid control digital state=%d\n", digstate);
                return "INVALID DIGITAL STATE";
        }
}

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
		printf("Invalid severity=%d\n", severity);
                return "INVALID SEVERITY";
        }
}

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
                case SA_ERR_HPI_UPDATING:
                        return "SA_ERR_HPI_UPDATING";
                case SA_ERR_HPI_INITIALIZING:
                        return "SA_ERR_HPI_INITIALIZING";
                case SA_ERR_HPI_UNKNOWN:
                        return "SA_ERR_HPI_UNKNOWN";
                case SA_ERR_HPI_INVALID_SESSION:
                        return "SA_ERR_HPI_INVALID_SESSION";
                case SA_ERR_HPI_INVALID_DOMAIN:
                        return "SA_ERR_HPI_INVALID_DOMAIN";
                case SA_ERR_HPI_INVALID_RESOURCE:
                        return "SA_ERR_HPI_INVALID_RESOURCE";
                case SA_ERR_HPI_INVALID_REQUEST:
                        return "SA_ERR_HPI_INVALID_REQUEST";
                case SA_ERR_HPI_ENTITY_NOT_PRESENT:
                        return "SA_ERR_HPI_ENTITY_NOT_PRESENT";
                case SA_ERR_HPI_UNINITIALIZED:
                        return "SA_ERR_HPI_UNINITIALIZED";
                default:
			printf("Invalid error code=%d\n", error);
                        return "(Invalid error code)";
        }
}

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
		printf("Invalid RDR type=%d\n", type);
                return "(Invalid RDR type)";
        }
        return "\0";
}

void list_rdr(SaHpiSessionIdT session_id, SaHpiResourceIdT resource_id)
{
        SaErrorT             	err;
        SaHpiEntryIdT        	current_rdr;
        SaHpiEntryIdT        	next_rdr;
        SaHpiRdrT            	rdr;

	SaHpiSensorReadingT	reading;
	SaHpiSensorTypeT	sensor_type;
	SaHpiSensorNumT		sensor_num;
	SaHpiEventCategoryT	category;
	SaHpiSensorThresholdsT	thres; 

	SaHpiCtrlNumT   	ctrl_num;
	SaHpiCtrlStateT 	state;
	SaHpiCtrlTypeT  	ctrl_type;

        char                    inbuff[10240];
        SaHpiEirIdT             l_eirid;
        SaHpiUint32T            l_buffersize = sizeof(inbuff);
        SaHpiUint32T            l_actualsize;
        SaHpiInventoryDataT*    l_inventdata;


        printf("RDR Info:\n");
        next_rdr = SAHPI_FIRST_ENTRY;
        do {
                char tmp_epath[128];
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
			SaErrorT val;
			
			sensor_num = rdr.RdrTypeUnion.SensorRec.Num;
			
			val = saHpiSensorTypeGet(session_id, resource_id, 
						 sensor_num, &sensor_type, 
						 &category);
			
			printf("\tSensor num: %i\n\tType: %s\n", sensor_num, get_sensor_type(sensor_type)); 
			printf("\tCategory: %s\n", get_sensor_category(category)); 

			memset(&reading, 0, sizeof(SaHpiSensorReadingT));

			err = saHpiSensorReadingGet(session_id, resource_id, sensor_num, &reading);
			if (err != SA_OK) {
				printf("Error=%d reading sensor data {sensor, %d}\n", err, sensor_num);
				continue;
			}

			if (reading.ValuesPresent & SAHPI_SRF_RAW) {
				printf("\tValues Present: RAW\n");
				printf("\t\tRaw value: %d\n", reading.Raw);
			}

			if (reading.ValuesPresent & SAHPI_SRF_INTERPRETED) {
				printf("\tValues Present: Interpreted\n");
                                printf("\t\t");
				interpreted2str(reading.Interpreted);
			}

			if (reading.ValuesPresent & SAHPI_SRF_EVENT_STATE) {
				printf("\tValues Present: Event State\n");
			}

			if (rdr.RdrTypeUnion.SensorRec.ThresholdDefn.IsThreshold == SAHPI_TRUE) {
                                memset(&thres, 0, sizeof(SaHpiSensorThresholdsT));
				err = saHpiSensorThresholdsGet(session_id, resource_id, sensor_num, &thres);
				if (err != SA_OK) {
					printf("Error=%d reading sensor thresholds {sensor, %d}\n", err, sensor_num);
					continue;
				}
				
				if (thres.LowCritical.ValuesPresent) {
					printf("\t\tThreshold:  Low Critical Values\n");
					printreading(thres.LowCritical);
				}
				if (thres.LowMajor.ValuesPresent) {
					printf("\t\tThreshold:  Low Major Values\n");
					printreading(thres.LowMajor);
				}
				if (thres.LowMinor.ValuesPresent) {
					printf("\t\tThreshold:  Low Minor Values\n");
					printreading(thres.LowMinor);
				}
				if (thres.UpCritical.ValuesPresent) {
					printf("\t\tThreshold:  Up Critical Values\n");
					printreading(thres.UpCritical);
				}
				if (thres.UpMajor.ValuesPresent) {
					printf("\t\tThreshold:  Up Major Values\n");
					printreading(thres.UpMajor);
				}
				if (thres.UpMinor.ValuesPresent) {
					printf("\t\tThreshold:  Up Minor Values\n");
					printreading(thres.UpMinor);
				}
				if (thres.PosThdHysteresis.ValuesPresent) {
					printf("\t\tThreshold:  Pos Threshold Hysteresis Values\n");
					printreading(thres.PosThdHysteresis);
				}
				if (thres.NegThdHysteresis.ValuesPresent) {
					printf("\t\tThreshold:  Neg Threshold Hysteresis Values\n");
					printreading(thres.NegThdHysteresis);
				}
			}
		}
		
		if (rdr.RdrType == SAHPI_CTRL_RDR)
		{    
			ctrl_num = rdr.RdrTypeUnion.CtrlRec.Num;
			err = saHpiControlTypeGet(session_id, resource_id, ctrl_num, &ctrl_type);
			if (err != SA_OK) {
				printf("Error=%d reading control type {control, %d}\n", err, ctrl_num);
				continue;
			}
			printf("\tControl num: %i\n\tType: %s\n", ctrl_num, get_control_type(ctrl_type)); 
		
			err = saHpiControlStateGet(session_id, resource_id, ctrl_num, &state);
			if (err != SA_OK) {
				printf("Error=%d reading control state {control, %d}\n", err, ctrl_num);
				continue;
			}
			if (ctrl_type != state.Type) {
				printf("Control Type mismatch between saHpiControlTypeGet=%d and saHpiControlStateGet = %d\n", 
				       ctrl_type, state.Type);
			}

			switch (state.Type) {
				case SAHPI_CTRL_TYPE_DIGITAL:
					printf("\t\tControl Digital State: %s\n", 
					       ctrldigital2str(state.StateUnion.Digital));
					break;
				case SAHPI_CTRL_TYPE_DISCRETE:
					printf("\t\tControl Discrete State: %x\n", state.StateUnion.Discrete);
					break;
				case SAHPI_CTRL_TYPE_ANALOG:
					printf("\t\tControl Analog State: %x\n", state.StateUnion.Analog);
					break;
				case SAHPI_CTRL_TYPE_STREAM:
					printf("\t\tControl Stream Repeat: %d\n", state.StateUnion.Stream.Repeat);
					printf("\t\tControl Stream Data: ");
					display_oembuffer(state.StateUnion.Stream.StreamLength, state.StateUnion.Stream.Stream);
					break;
				case SAHPI_CTRL_TYPE_TEXT:
					printf("\t\tControl Text Line Num: %c\n", state.StateUnion.Text.Line);
					display_textbuffer(state.StateUnion.Text.Text);
					break;
				case SAHPI_CTRL_TYPE_OEM:
					printf("\t\tControl OEM Manufacturer: %d\n", state.StateUnion.Oem.MId);
					printf("\t\tControl OEM Data: ");
					display_oembuffer((SaHpiUint32T)state.StateUnion.Oem.BodyLength, 
						  state.StateUnion.Oem.Body);
					break;
				default:
					printf("\t\tInvalid control type (%d) from saHpiControlStateGet\n", 
					       state.Type);
			}
                }

                if (rdr.RdrType == SAHPI_INVENTORY_RDR)
                {
                        l_eirid = rdr.RdrTypeUnion.InventoryRec.EirId;

                        l_inventdata = (SaHpiInventoryDataT *)&inbuff[0];
                        err = saHpiEntityInventoryDataRead(session_id, resource_id,
                                                            l_eirid, l_buffersize,
                                                            l_inventdata, &l_actualsize);

			if (err != SA_OK) {
				printf("Error=%d reading inventory type {EirId, %d}\n", err, l_eirid);
				continue;
			} else if (l_inventdata->Validity ==  SAHPI_INVENT_DATA_VALID) {
                        	printf("\tFound Inventory RDR with EirId: %x\n", l_eirid);
				printf("\tRDR l_buffersize = %d, actualsize = %d\n", l_buffersize, l_actualsize);
				switch (l_inventdata->DataRecords[0]->RecordType)
				{
					case SAHPI_INVENT_RECTYPE_INTERNAL_USE:
						printf( "Internal Use\n");
						break;
					case SAHPI_INVENT_RECTYPE_PRODUCT_INFO:
						printf( "Product Info\n");
						break;
					case SAHPI_INVENT_RECTYPE_CHASSIS_INFO:
						printf( "Chassis Info\n");
						break;
					case SAHPI_INVENT_RECTYPE_BOARD_INFO:
						printf( "Board Info\n");
						break;
					case SAHPI_INVENT_RECTYPE_OEM:
						printf( "OEM Record\n");
						break;
					default:
						printf(" Invalid Invent Rec Type =%x\n",
								l_inventdata->DataRecords[0]->RecordType);
						break;
				} 

			}
                }

                printf("\tEntity: \n");
                entitypath2string(&rdr.Entity, tmp_epath, sizeof(tmp_epath));
                printf("\t\t%s\n", tmp_epath);
                printf("\tIdString: ");
                display_textbuffer(rdr.IdString);
                printf("\n"); /* Produce blank line between rdrs. */
        }while(next_rdr != SAHPI_LAST_ENTRY);
}


/**
 * eventtype2str:
 * @type: HPI event type
 *
 * Convert event type to string.
 **/

const char *eventtype2str(SaHpiEventTypeT type)
{
        switch( type ) {
        case SAHPI_ET_SENSOR:
                return "SAHPI_ET_SENSOR";
                
        case SAHPI_ET_HOTSWAP:
                return "SAHPI_ET_HOTSWAP";

        case SAHPI_ET_WATCHDOG:
                return "SAHPI_ET_WATCHDOG";

        case SAHPI_ET_OEM:
                return "SAHPI_ET_OEM";

        case SAHPI_ET_USER:
                return "SAHPI_ET_USER";
        }

        return "INVALID_EVENT_TYPE";
}


/**
 * time2str:
 * @time: HPI time
 * @str: time string
 *
 * Convert SaHpiTimeT into a string.
 **/

void time2str(SaHpiTimeT time, char *str)
{
        if (time == SAHPI_TIME_UNSPECIFIED) {
                strcpy( str, "SAHPI_TIME_UNSPECIFIED" );
                return;
        }

        int nano = time % 1000000000;

        time /= 1000000000;
        time_t t = (time_t)time;

        struct tm tm;
        localtime_r(&t, &tm);

        char s[30];
        strftime(s, 30, "%Y.%m.%d %H:%M:%S", &tm);

        sprintf(str, "%s.%09d", s, nano);
}


/**
 * list_sel:
 * @session_id: session id
 * @resource_id: resource id with capability SAHPI_CAPABILITY_SEL
 *
 * Print out information about event log.
 **/

void list_sel(SaHpiSessionIdT session_id, SaHpiResourceIdT resource_id)
{
        char str[256];

        printf("SEL Info:\n");

        SaHpiSelInfoT info;        
        SaErrorT rv = saHpiEventLogInfoGet(session_id, resource_id, &info);

        if (rv != SA_OK) {
                printf( "Error=%d reading SEL info\n", rv);
                return;
        }

        printf("\tEntries in SEL: %d\n", info.Entries);
        printf("\tSize: %d\n", info.Size);

        time2str(info.UpdateTimestamp, str);
        printf("\tUpdateTimestamp: %s\n", str);

        time2str(info.CurrentTime, str);
        printf("\tCurrentTime    : %s\n", str);
        printf("\tEnabled: %s\n", info.Enabled ? "true" : "false");
        printf("\tOverflowFlag: %s\n", info.OverflowFlag ? "true" : "false");
        printf("\tOverflowAction: " );

        switch(info.OverflowAction) {
        case SAHPI_SEL_OVERFLOW_DROP:
                printf("SAHPI_SEL_OVERFLOW_DROP\n");
                break;

        case SAHPI_SEL_OVERFLOW_WRAP:
                printf("SAHPI_SEL_OVERFLOW_WRAP\n");
                break;

        case SAHPI_SEL_OVERFLOW_WRITELAST:
                printf("SAHPI_SEL_OVERFLOW_WRITELAST\n");
                break;

        default:
                printf("unknown(0x%x)\n", info.OverflowAction);
                break;
        }

        printf("\tDeleteEntrySupported: %s\n", info.DeleteEntrySupported ? "true" : "false" );

        // read sel records
        SaHpiSelEntryIdT current = SAHPI_OLDEST_ENTRY;

        do {
                SaHpiSelEntryIdT prev;
                SaHpiSelEntryIdT next;
                SaHpiSelEntryT   entry;
                SaHpiRdrT        rdr;
                SaHpiRptEntryT   res;

                rv = saHpiEventLogEntryGet(session_id, resource_id, current, &prev, &next, &entry, &rdr, &res);

                if (rv != SA_OK) {
                        printf( "Error=%d reading SEL entry %d\n", rv, current);
                        return;
                }

                printf("\t\tEntry %d, prev %d, next %d\n", entry.EntryId, prev, next);
                time2str(entry.Timestamp, str);
                printf("\t\t\tTimestamp:            %s\n", str);

                SaHpiRptEntryT rres;

                rv = saHpiRptEntryGetByResourceId(session_id, entry.Event.Source, &rres );

                if ( rv != SA_OK )
                printf("\t\t\tSource:               unknown\n" );
                else {
                        entitypath2string( &rres.ResourceEntity, str, sizeof(str));
                printf("\t\t\tSource:               %s\n", str );
                }

                printf("\t\t\tEventType:            %s\n", eventtype2str(entry.Event.EventType));
                time2str(entry.Timestamp, str);
                printf("\t\t\tEvent timestamp:      %s\n", str);
                printf("\t\t\tSeverity:             %s\n", severity2str( entry.Event.Severity ) );

                switch(entry.Event.EventType) {
                case SAHPI_ET_SENSOR:
                        {
                                SaHpiSensorEventT *se = &entry.Event.EventDataUnion.SensorEvent;
                printf("\t\t\tSensorNum:            %d\n", se->SensorNum );
                printf("\t\t\tSensorType:           %s\n", get_sensor_type(se->SensorType ) );
                printf("\t\t\tEventCategory:        %s\n", get_sensor_category( se->EventCategory ) );
                printf("\t\t\tAssertion:            %s\n", se->Assertion ? "TRUE" : "FALSE" );
                        }

                        break;

                case SAHPI_ET_HOTSWAP:
                        {
                                SaHpiHotSwapEventT *he = &entry.Event.EventDataUnion.HotSwapEvent;
                printf("\t\t\tHotSwapState:         %s\n",
                                      hotswapstate2str( he->HotSwapState ) );
                printf("\t\t\tPreviousHotSwapState: %s\n",
                                        hotswapstate2str( he->PreviousHotSwapState ) );
                        }

                        break;

                case SAHPI_ET_WATCHDOG:
                        break;

                case SAHPI_ET_OEM:
                        break;

                case SAHPI_ET_USER:
                        break;
                }
                
                current = next;
        } while(current != SAHPI_NO_MORE_ENTRIES);
}


void display_oembuffer(SaHpiUint32T length, SaHpiUint8T *string)
{
        int i;

	for (i = 0; i < length; i++) {
		printf("%c", string[i]);
	}

        printf("\n");
}


void display_textbuffer(SaHpiTextBufferT string)
{
        int i;
        switch(string.DataType) {
        case SAHPI_TL_TYPE_BINARY:
                for (i = 0; i < string.DataLength; i++)
                        printf("%x", string.Data[i]);
                break;
        case SAHPI_TL_TYPE_BCDPLUS:
                for (i = 0; i < string.DataLength; i++)
                        printf("%c", string.Data[i]);
                break;
        case SAHPI_TL_TYPE_ASCII6:
                for (i = 0; i < string.DataLength; i++)
                        printf("%c", string.Data[i]);
                break;
        case SAHPI_TL_TYPE_LANGUAGE:
                for (i = 0; i < string.DataLength; i++)
                        printf("%c", string.Data[i]);
                break;
        default:
                printf("Invalid string data type=%d", string.DataType);
        }
        printf("\n");
}

/**
 * interpreted2str:
 * @interpreted: structure with interpreted data for sensor
 *
 * Print a string containing the interpreted data from the
 * interpreted data structure given.
 **/

void interpreted2str (SaHpiSensorInterpretedT interpreted)
{
        char *str = calloc(SAHPI_SENSOR_BUFFER_LENGTH ,sizeof(char));

        printf("Interpreted value: ");
                
	switch (interpreted.Type) {
		case SAHPI_SENSOR_INTERPRETED_TYPE_UINT8:
			snprintf(str,SAHPI_SENSOR_BUFFER_LENGTH,"%d",interpreted.Value.SensorUint8);
			break;
		case SAHPI_SENSOR_INTERPRETED_TYPE_UINT16:
			snprintf(str,SAHPI_SENSOR_BUFFER_LENGTH,"%d",interpreted.Value.SensorUint16);
			break;
		case SAHPI_SENSOR_INTERPRETED_TYPE_UINT32:
			snprintf(str,SAHPI_SENSOR_BUFFER_LENGTH,"%d",interpreted.Value.SensorUint32);
			break;
		case SAHPI_SENSOR_INTERPRETED_TYPE_INT8:
			snprintf(str,SAHPI_SENSOR_BUFFER_LENGTH,"%d",interpreted.Value.SensorInt8);
			break;
		case SAHPI_SENSOR_INTERPRETED_TYPE_INT16:
			snprintf(str,SAHPI_SENSOR_BUFFER_LENGTH,"%d",interpreted.Value.SensorInt16);
			break;
		case SAHPI_SENSOR_INTERPRETED_TYPE_INT32:
			snprintf(str,SAHPI_SENSOR_BUFFER_LENGTH,"%d",interpreted.Value.SensorInt32);
			break;
		case SAHPI_SENSOR_INTERPRETED_TYPE_FLOAT32:
			snprintf(str,SAHPI_SENSOR_BUFFER_LENGTH,"%6.2f",interpreted.Value.SensorFloat32);
			break;
		case SAHPI_SENSOR_INTERPRETED_TYPE_BUFFER:
			snprintf(str,SAHPI_SENSOR_BUFFER_LENGTH,"%s",interpreted.Value.SensorBuffer);
			break;
		default:
			printf("Invalid interpreted type=%d\n", interpreted.Type);
			break;
	}
        printf("%s\n",str);
        
	free(str);
}

void rpt_cap2str (SaHpiCapabilitiesT ResourceCapabilities)
{
        printf("\n");
        if(ResourceCapabilities & SAHPI_CAPABILITY_DOMAIN)
                printf("\tSAHPI_CAPABILITY_DOMAIN\n");
        if(ResourceCapabilities & SAHPI_CAPABILITY_RESOURCE)
                printf("\tSAHPI_CAPABILITY_RESOURCE\n");
        if(ResourceCapabilities & SAHPI_CAPABILITY_SEL)
                printf("\tSAHPI_CAPABILITY_SEL\n");
        if(ResourceCapabilities & SAHPI_CAPABILITY_EVT_DEASSERTS)
                printf("\tSAHPI_CAPABILITY_EVT_DEASSERTS\n");
        if(ResourceCapabilities & SAHPI_CAPABILITY_AGGREGATE_STATUS)
                printf("\tSAHPI_CAPABILITY_AGGREGATE_STATUS\n");
        if(ResourceCapabilities & SAHPI_CAPABILITY_CONFIGURATION)
                printf("\tSAHPI_CAPABILITY_CONFIGURATION\n");
        if(ResourceCapabilities & SAHPI_CAPABILITY_MANAGED_HOTSWAP)
                printf("\tSAHPI_CAPABILITY_MANAGED_HOTSWAP\n");
        if(ResourceCapabilities & SAHPI_CAPABILITY_WATCHDOG)
                printf("\tSAHPI_CAPABILITY_WATCHDOG\n");
        if(ResourceCapabilities & SAHPI_CAPABILITY_CONTROL)
                printf("\tSAHPI_CAPABILITY_CONTROL\n");
        if(ResourceCapabilities & SAHPI_CAPABILITY_FRU)
                printf("\tSAHPI_CAPABILITY_FRU\n");
        if(ResourceCapabilities & SAHPI_CAPABILITY_INVENTORY_DATA)
                printf("\tSAHPI_CAPABILITY_INVENTORY_DATA\n");
        if(ResourceCapabilities & SAHPI_CAPABILITY_RDR)
                printf("\tSAHPI_CAPABILITY_RDR\n");
        if(ResourceCapabilities & SAHPI_CAPABILITY_SENSOR)
                printf("\tSAHPI_CAPABILITY_SENSOR\n");
        
}

const char * get_control_type(SaHpiCtrlTypeT type) 
{
	switch(type) {
		case SAHPI_CTRL_TYPE_DIGITAL:
			return "DIGITAL CONTROL";
		case SAHPI_CTRL_TYPE_DISCRETE:
			return "DISCRETE CONTROL";
		case SAHPI_CTRL_TYPE_ANALOG:
			return "ANALOG CONTROL";
		case SAHPI_CTRL_TYPE_STREAM:
			return "STREAM CONTROL";
		case SAHPI_CTRL_TYPE_TEXT:
			return "TEXT CONTROL";
		case SAHPI_CTRL_TYPE_OEM:
			return "OEM CONTROL";
		default:
			printf("Invalid control type=%d\n", type);
			return "Invalid Control Type";
	}

	return "\0";

}

const char * get_sensor_type(SaHpiSensorTypeT type) 
{
	switch(type) {
		case SAHPI_TEMPERATURE:
			return "TEMPERATURE SENSOR";
		case SAHPI_VOLTAGE:
			return "VOLTAGE SENSOR";
		case SAHPI_CURRENT:
			return "CURRENT SENSOR";
		case SAHPI_FAN:
			return "FAN SENSOR";
		case SAHPI_PHYSICAL_SECURITY:
			return "PHYSICAL SECURITY";
		case SAHPI_PLATFORM_VIOLATION:
			return "PLATFORM_VIOLATION";
		case SAHPI_PROCESSOR:
			return "PROCESSOR";
		case SAHPI_POWER_SUPPLY:
			return "POWER SUPPLY";
		case SAHPI_POWER_UNIT:
			return "POWER UNIT";
		case SAHPI_COOLING_DEVICE:
			return "COOLING DEVICE";
		case SAHPI_OTHER_UNITS_BASED_SENSOR:
			return "OTHER UNITS BASED SENSOR";
		case SAHPI_MEMORY:
			return "MEMORY";
    		case SAHPI_DRIVE_SLOT:
			return "SAHPI_DRIVE_SLOT"; 
    		case SAHPI_POST_MEMORY_RESIZE:
			return "SAHPI_POST_MEMORY_RESIZE"; 
    		case SAHPI_SYSTEM_FW_PROGRESS:
			return "SAHPI_SYSTEM_FW_PROGRESS"; 
    		case SAHPI_EVENT_LOGGING_DISABLED:
			return "SAHPI_EVENT_LOGGING_DISABLED"; 
    		case SAHPI_RESERVED1:
			return "SAHPI_RESERVED1"; 
    		case SAHPI_SYSTEM_EVENT:
			return "SAHPI_SYSTEM_EVENT"; 
    		case SAHPI_CRITICAL_INTERRUPT:
			return "SAHPI_CRITICAL_INTERRUPT"; 
    		case SAHPI_BUTTON:
			return "SAHPI_BUTTON"; 
    		case SAHPI_MODULE_BOARD:
			return "SAHPI_MODULE_BOARD"; 
    		case SAHPI_MICROCONTROLLER_COPROCESSOR:
			return "SAHPI_MICROCONTROLLER_COPROCESSOR"; 
    		case SAHPI_ADDIN_CARD:
			return "SAHPI_ADDIN_CARD"; 
    		case SAHPI_CHASSIS:
			return "SAHPI_CHASSIS"; 
    		case SAHPI_CHIP_SET:
			return "SAHPI_CHIP_SET"; 
    		case SAHPI_OTHER_FRU:
			return "SAHPI_OTHER_FRU"; 
    		case SAHPI_CABLE_INTERCONNECT:
			return "SAHPI_CABLE_INTERCONNECT"; 
    		case SAHPI_TERMINATOR:
			return "SAHPI_TERMINATOR"; 
    		case SAHPI_SYSTEM_BOOT_INITIATED:
			return "SAHPI_SYSTEM_BOOT_INITIATED"; 
    		case SAHPI_BOOT_ERROR:
			return "SAHPI_BOOT_ERROR"; 
    		case SAHPI_OS_BOOT:
			return "SAHPI_OS_BOOT"; 
    		case SAHPI_OS_CRITICAL_STOP:
			return "SAHPI_OS_CRITICAL_STOP"; 
    		case SAHPI_SLOT_CONNECTOR:
			return "SAHPI_SLOT_CONNECTOR"; 
    		case SAHPI_SYSTEM_ACPI_POWER_STATE:
			return "SAHPI_SYSTEM_ACPI_POWER_STATE"; 
    		case SAHPI_RESERVED2:
			return "SAHPI_RESERVED2"; 
    		case SAHPI_PLATFORM_ALERT:
			return "SAHPI_PLATFORM_ALERT"; 
    		case SAHPI_ENTITY_PRESENCE:
			return "SAHPI_ENTITY_PRESENCE"; 
    		case SAHPI_MONITOR_ASIC_IC:
			return "SAHPI_MONITOR_ASIC_IC"; 
		case SAHPI_LAN:
			return "SAHPI_LAN";
    		case SAHPI_MANAGEMENT_SUBSYSTEM_HEALTH:
			return "SAHPI_MANAGEMENT_SUBSYSTEM_HEALTH"; 
    		case SAHPI_BATTERY:
			return "SAHPI_BATTERY"; 
    		case SAHPI_OPERATIONAL:
			return "SAHPI_OPERATIONAL=0xA"; 
    		case SAHPI_OEM_SENSOR:
			return "SAHPI_OEM_SENSOR=0xC"; 
		default:
			printf("Invalid sensor type=%d\n", type);
			return "Invalid Sensor Type";
	}

return "\0";

}


const char *hotswapstate2str( SaHpiHsStateT state )
{
        switch(state)
        {
        case SAHPI_HS_STATE_INACTIVE:
                return "SAHPI_HS_STATE_INACTIVE";
                
        case SAHPI_HS_STATE_INSERTION_PENDING:
                return "SAHPI_HS_STATE_INSERTION_PENDING";

        case SAHPI_HS_STATE_ACTIVE_HEALTHY:
                return "SAHPI_HS_STATE_ACTIVE_HEALTHY";

        case SAHPI_HS_STATE_ACTIVE_UNHEALTHY:
                return "SAHPI_HS_STATE_ACTIVE_UNHEALTHY";

        case SAHPI_HS_STATE_EXTRACTION_PENDING:
                return "SAHPI_HS_STATE_EXTRACTION_PENDING";

        case SAHPI_HS_STATE_NOT_PRESENT:
                return "SAHPI_HS_STATE_NOT_PRESENT";
        }

        return "Invalid hotswap state";
}


const char * get_sensor_category (SaHpiEventCategoryT category)
{
	switch (category)
	{
		case SAHPI_EC_UNSPECIFIED:
			return "SAHPI_EC_UNSPECIFIED";
		case SAHPI_EC_THRESHOLD:
			return "SAHPI_EC_THRESHOLD"; 
		case SAHPI_EC_USAGE:
			return "SAHPI_EC_USAGE"; 
		case SAHPI_EC_STATE:
			return "SAHPI_EC_STATE"; 
		case SAHPI_EC_PRED_FAIL:
			return "SAHPI_EC_PRED_FAIL"; 
		case SAHPI_EC_LIMIT:
			return "SAHPI_EC_LIMIT";
		case SAHPI_EC_PERFORMANCE:
			return "SAHPI_EC_PERFORMANCE"; 
		case SAHPI_EC_SEVERITY:
		   	return "SAHPI_EC_SEVERITY"; 
		case SAHPI_EC_PRESENCE:
			return "SAHPI_EC_PRESENSE";
		case SAHPI_EC_ENABLE:
			return "SAHPI_EC_ENABLE";
		case SAHPI_EC_AVAILABILITY:
			return "SAHPI_EC_AVAILABILITY";
		case SAHPI_EC_REDUNDANCY:
			return "SAHPI_EC_REDUNDANCY";
		case SAHPI_EC_USER:
			return "SAHPI_EC_USER";
		case SAHPI_EC_GENERIC:
			return "SAHPI_EC_GENERIC";
                default:
			printf("Invalid sensor category=%d\n", category);
			return "Invalid Category";
	}

	return("\0");
}

/**
 * printreading:
 * @reading: structure with data to print
 *
 * Print out data from a sensor reading (used when
 * printing out threshold data).
 **/

void printreading (SaHpiSensorReadingT reading)
{
	if (reading.ValuesPresent & SAHPI_SRF_RAW) {
		printf("\t\t\tValues Present: RAW\n");
		printf("\t\t\t\tRaw value: %d\n", reading.Raw);
	}
	
	
	if (reading.ValuesPresent & SAHPI_SRF_INTERPRETED)
		printf("\t\t\tValues Present: Interpreted\n");

        printf("\t\t\t\t");
        interpreted2str(reading.Interpreted);

	if (reading.ValuesPresent & SAHPI_SRF_EVENT_STATE)
		printf("\t\t\tValues Present: Event State\n");
}



