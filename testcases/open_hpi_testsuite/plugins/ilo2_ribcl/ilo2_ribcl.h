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
#ifndef _INC_ILO2_RIBCL_H_
#define _INC_ILO2_RIBCL_H_

/***************
 * This header file contains all of the iLO2 RIBCL internal data structure
 * definitions. This file is intended to be included in all of the iLO2 RIBCL
 * source files.
***************/
#include <sys/time.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <SaHpi.h>
#include <oh_utils.h>
#include <oh_error.h>
#include <oh_handler.h>
#include <oh_ssl.h>

#include "ilo2_ribcl_cmnds.h"

/* The macro ILO2_RIBCL_SIMULATE_iLO2_RESPONSE is used to conditionally
 * compile code which reads a simulated iLO2 response from a local
 * file rather than communicating with an actual iLO2. This is used for
 * testing when specific responses are needed, or in an environment where
 * an iLO2 is not available. The filename for the response for each command
 * is specified in the openhpi.conf configuration file. The currently supported
 * names are:
 *	"discovery_responsefile" - file used by ilo2_ribcl_do_discovery().
 */ 
/* #define ILO2_RIBCL_SIMULATE_iLO2_RESPONSE */

/* 
 * ILO2 RIBCL User name
 * 	Maximum length - 39
 * 	any combination of printable characters
 * 	case sensitive
 * 	must not be blank
 */
#define ILO2_RIBCL_USER_NAME_MAX_LEN	39
#define ILO2_RIBCL_USER_NAME_MIN_LEN	1
/* 
 * ILO2 RIBCL password
 * 	Maximum length - 39
 * 	any combination of printable characters
 * 	case sensitive
 * 	Minimum length - 0
 * 	Minimum default length - 8
 */
#define ILO2_RIBCL_PASSWORD_MAX_LEN	39
#define ILO2_RIBCL_PASSWORD_MIN_LEN	1

/*
 * ILO2 RIBCL Hostname Length defines
 */
#define ILO2_HOST_NAME_MAX_LEN 256
/* A valid IP addr will have a min. len of 7 4 digts + 3 decimal points */
#define ILO2_HOST_NAME_MIN_LEN 7

/* Minimum length port string */
#define ILO2_MIN_PORT_STR_LEN	1

/*
 * RIBCL command and response buffer length - static for now.
 * Change to dynamic if need be.
 */
#define ILO2_RIBCL_BUFFER_LEN	4096 

/*
 * Power and reset status definitions
*/
#define ILO2_RIBCL_POWER_OFF	0
#define ILO2_RIBCL_POWER_ON	1
#define ILO2_RIBCL_RESET_SUCCESS	1	
#define ILO2_RIBCL_RESET_FAILED	0	

/*
 * For a oh_set_power_state() call with a state parameter of
 * SAHPI_POWER_CYCLE, we must wait until the server actually powers off
 * before powering it back on again. 
 * The total turnaround time for a RIBCL command is around 10 seconds,
 * so the total time spent waiting for the system to power off (in seconds)
 * will be ((ILO2_POWER_POLL_SLEEP_SECONDS + 10) * ILO2_MAX_POWER_POLLS)
 * The default timing below will wait a maximum of 200 seconds.
 */
#define ILO2_MAX_POWER_POLLS            10
#define ILO2_POWER_POLL_SLEEP_SECONDS   10

/* 
 * get_event return value when there are events pending to be processed.
 * OpenHPI framework doesn't provide mnemonic for this return value.
*/
#define ILO2_RIBCL_EVENTS_PENDING	1
/* 
 * Resource doesn't support managed hot swap. OpenHPI fails to define
 * mnemonic for this value
*/
#define ILO2_RIBCL_MANAGED_HOTSWAP_CAP_FALSE	0


/******************************************************************************
 * The following data structures and macros are used for our implementation
 * of Inventory Data Repositories.
 *
 * Currently, we only have one area in out IDRs. The plugin has been
 * written to easily add more areas in the future.
 */
#define I2R_MAX_AREA	1

#define I2R_MAX_FIELDS		4
#define I2R_MAX_FIELDCHARS	32

/* These are the index values for fields in the chassis IDR */
#define I2R_CHASSIS_IF_PRODNAME 	0
#define I2R_CHASSIS_IF_SERNUM		1
#define I2R_CHASSIS_IF_MANUFACT		2
#define I2R_CHASSIS_IF_ILO2VERS		3

/* These are the index values for fields in the memory DIMM IDR */
#define I2R_MEM_IF_SIZE		0
#define I2R_MEM_IF_SPEED	1

/* These are the index values for fields in the cpu IDR */
#define I2R_CPU_IF_SPEED	0

typedef struct {
	SaHpiIdrFieldTypeT	field_type;
	char			field_string[I2R_MAX_FIELDCHARS];
} I2R_FieldT;

typedef struct {
	SaHpiIdrAreaTypeT 	area_type;
	SaHpiUint32T		num_fields;	/* Number of used fields */
	I2R_FieldT		area_fields[I2R_MAX_FIELDS];
} I2R_AreaT;

struct ilo2_ribcl_idr_info {
	SaHpiUint32T	update_count;
	SaHpiUint32T	num_areas;
	I2R_AreaT	idr_areas[I2R_MAX_AREA];
};

/* The structure passed as a parameter to ilo2_ribcl_get_idr_allinfo() and
 * ilo2_ribcl_get_idr_allinfo_by_ep(), which collects all the associated
 * HPI data for an IDR
 */
struct ilo2_ribcl_idr_allinfo {
	SaHpiRptEntryT	*rpt;
	SaHpiRdrT	*rdr;
	struct ilo2_ribcl_idr_info *idrinfo;
};



/******************************************************************************
 * The following data structures and macros are used for our implementation
 * of Sensors.
 *
 * Currently, we only implement chassis sensors of event category 
 * SAHPI_EC_SEVERITY. The plugin has been written to add more sensor
 * categories in the future. */

/* These are the values for our severity level sensors */
#define I2R_SEN_VAL_OK		0
#define I2R_SEN_VAL_DEGRADED	1
#define I2R_SEN_VAL_FAILED	2
#define I2R_SEN_VAL_UNINITIALIZED  -1

/* The number of chassis sensors for our data structures. Note that
 * sensor numbers begin with 1, so this number may be one greater than
 * you would think. */
#define I2R_NUM_CHASSIS_SENSORS	4

/* These are the sensor numbers for the sensors on our chassis */
#define I2R_SEN_FANHEALTH	1
#define I2R_SEN_TEMPHEALTH	2
#define I2R_SEN_POWERHEALTH	3

/* These are the descriptions for the chassis sensors */
#define I2R_SEN_FANHEALTH_DESCRIPTION "System fans health indicator: Ok(0)/Degraded(1)/Failed(2)"
#define I2R_SEN_TEMPHEALTH_DESCRIPTION "System temperature health indicator: Ok(0)/Failed(2)"
#define I2R_SEN_POWERHEALTH_DESCRIPTION "System power supply health indicator: Ok(0)/Degraded(1)/Failed(2)"

/* The three state severity model (OK, Degraded, Fail) used for iLo2 RIBCL
 * sensors will support these sensor event states */
#define I2R_SEVERITY_THREESTATE_EV   (SAHPI_ES_OK | \
				      SAHPI_ES_MAJOR_FROM_LESS | \
				      SAHPI_ES_MAJOR_FROM_CRITICAL | \
				      SAHPI_ES_CRITICAL) 

/* The two state severity model (OK, Failed) used for iLo2 RIBCL sensors will
 * support these HPI sensor event states */
#define I2R_SEVERITY_TWOSTATE_EV     (SAHPI_ES_OK | SAHPI_ES_CRITICAL)


/* The private data for our sensor, associated with it's RDR. */ 
struct ilo2_ribcl_sensinfo {
	SaHpiSensorNumT	  sens_num;	       /* Sensor number */
	SaHpiEventStateT  sens_ev_state;       /* Current sensor event state */
	SaHpiEventStateT  prev_sens_ev_state;  /* Previous sensor event state */
	SaHpiEventStateT  event_sens_ev_state; /* ev state to send with event */
	SaHpiBoolT	  sens_enabled;	       /* Sensor enable */
	SaHpiBoolT	  sens_ev_enabled;     /* Sensor event enable */
	SaHpiEventStateT  sens_assertmask;
	SaHpiEventStateT  sens_deassertmask;
	int		  sens_value;
};

/* Structure used by ilo2_ribcl_get_sensor_rdr_data() to return a pointer to
 * the sensor RDR and it's associated private data. */
struct ilo2_ribcl_sens_allinfo {
	SaHpiRptEntryT *rpt;
	SaHpiRdrT *rdr;
	struct ilo2_ribcl_sensinfo *sens_dat;
};

/* These are the state for software state machines that process the iLo2
 * sensors. We currently only support Severity sensors, but more states
 * can be added later. */
typedef enum {
	I2R_INITIAL = 0,
	I2R_OK = 1,
	I2R_DEGRADED_FROM_OK = 2,
	I2R_DEGRADED_FROM_FAIL = 3,
	I2R_FAILED = 4,
	I2R_NO_EXIST = 0xFFFF
} I2R_SensorStateT;

/* All the types of sensor readings that we hande will be contained in this
 * union. */
typedef union {
	int intval;
} I2R_ReadingUnionT; 
 
/* Structure to store a sensor's dynamic reading in our private handler */
typedef struct {
	SaHpiResourceIdT rid;	/* So we can locate other HPI info from here */
	I2R_SensorStateT state;
	I2R_ReadingUnionT reading;
} I2R_SensorDataT;


/******************************************************************************
 * The following data structures are used to save the discovery data. 
 * This data cache will be used to implement APIs that require state
 * information and additional information on components such as speed,
 * and control capabilities. The index into this field, rid, and hotswap
 * state of each component will be added to the RPT private data cache
 * via oh_add_resource() when each component is discovered.
*/
/* Discover states for a resource */
enum ir_discoverstate { BLANK=0, OK=1, FAILED=2, REMOVED=3};

/* Values for flags fields in DiscoveryData structures */
#define IR_DISCOVERED		0x01
#define IR_EXISTED		0x02
#define IR_FAILED		0x04
#define IR_SPEED_UPDATED	0x08

typedef struct ir_cpudata {
	unsigned int cpuflags;
	enum ir_discoverstate dstate;
	char *label;
} ir_cpudata_t;

typedef struct ir_memdata {
	unsigned int memflags;
	enum ir_discoverstate dstate;
	char *label;
	char *memsize;
	char *speed;
} ir_memdata_t;

typedef struct ir_fandata{
	unsigned int fanflags;
	enum ir_discoverstate dstate;
	char *label;
	char *zone;
	char *status;
	int speed;
	char *speedunit;
} ir_fandata_t;

typedef struct ir_psudata{
	unsigned int psuflags;
	enum ir_discoverstate dstate;
	char *label;
	char *status;
} ir_psudata_t;

typedef struct ir_vrmdata{
	unsigned int vrmflags;
	enum ir_discoverstate dstate;
	char *label;
	char *status;
} ir_vrmdata_t;

typedef struct ir_tsdata{
	unsigned int tsflags;
	char *label;
	char *location;
	char *status;
	char *reading;
	char *readingunits;
} ir_tsdata_t;

/* Firmware Revision Information */
typedef struct ir_fwdata{
	char *version_string;
	SaHpiUint8T FirmwareMajorRev;
	SaHpiUint8T FirmwareMinorRev;
} ir_fwdata_t;

#define ILO2_RIBCL_DISCOVER_CPU_MAX 16
#define ILO2_RIBCL_DISCOVER_MEM_MAX 32
#define ILO2_RIBCL_DISCOVER_FAN_MAX 16
#define ILO2_RIBCL_DISCOVER_PSU_MAX 8
#define ILO2_RIBCL_DISCOVER_VRM_MAX 8
#define ILO2_RIBCL_DISCOVER_TS_MAX  48

#define ILO2_RIBCL_CHASSIS_INDEX    -1;	/* Index is not aplicable to chassis */
 
typedef struct ilo2_ribcl_DiscoveryData {
	char *product_name;
	char *serial_number;
	char *system_cpu_speed;
	ir_cpudata_t cpudata[ ILO2_RIBCL_DISCOVER_CPU_MAX+1];
	ir_memdata_t memdata[ ILO2_RIBCL_DISCOVER_MEM_MAX+1];
	ir_fandata_t fandata[ ILO2_RIBCL_DISCOVER_FAN_MAX+1];
	ir_psudata_t psudata[ ILO2_RIBCL_DISCOVER_PSU_MAX+1];
	ir_vrmdata_t vrmdata[ ILO2_RIBCL_DISCOVER_VRM_MAX+1];
	ir_tsdata_t tsdata[ ILO2_RIBCL_DISCOVER_TS_MAX+1];
	I2R_SensorDataT chassis_sensors[I2R_NUM_CHASSIS_SENSORS];
	ir_fwdata_t fwdata;
} ilo2_ribcl_DiscoveryData_t; 

/* iLO2 RIBCL plug-in handler structure */
typedef struct ilo2_ribcl_handler {
	char *entity_root;
	int first_discovery_done;

	/* Storehouse for data obtained during discovery */
	ilo2_ribcl_DiscoveryData_t DiscoveryData;

	/* RIBCL data */
	char *user_name;
	char *password;

	/* iLO2 hostname and port number information */
	char *ilo2_hostport;

#ifdef ILO2_RIBCL_SIMULATE_iLO2_RESPONSE
	/* Discovery response file for testing */
	char *discovery_responsefile;
#endif /* ILO2_RIBCL_SIMULATE_iLO2_RESPONSE */

	/* SSL connection status */
	SSL_CTX *ssl_ctx;
	/* SSL connection handler pointer */
	BIO *ssl_handler;

	/* Commands customized with the login and password for this system */
	char *ribcl_xml_cmd[ IR_NUM_COMMANDS];

	GSList *eventq;                 /* Event queue cache */

	/* During discovery, soem routines need a temporary buffer for
	 * struct ilo2_ribcl_idr_info. It's a bit too large to use as
	 * as local variable on the stack, and allocating/deallocating
	 * it frequently could fragment memory. So, we keep it here
	 * in our private handler for all routines to use. */

	struct ilo2_ribcl_idr_info tmp_idr;
 
} ilo2_ribcl_handler_t;

/* Define for uninitialized power_cur_state value. Power status is
   not queried during discovery and power_cur_state field will be
   initialized to reflect an uninitialized state.
*/
#define	ILO2_RIBCL_POWER_STATUS_UNKNOWN	-1

/* iLO2 RIBCL private resource data */
typedef struct ilo2_ribcl_resource_info {
	SaHpiResourceIdT rid;
	SaHpiHsStateT fru_cur_state;	/* current simple hotswap state of
					   FRU resources */
	int disc_data_idx;		/* resource index into the Discovery
					   Data Cache */
	SaHpiPowerStateT power_cur_state;	/* current power state */
} ilo2_ribcl_resource_info_t;

/*
	iLO2 RIBCL control type and index definitions.
*/
#define ILO2_RIBCL_CTL_UID	1
#define ILO2_RIBCL_CONTROL_1	1
#define ILO2_RIBCL_CTL_POWER_SAVER	2
#define ILO2_RIBCL_CONTROL_2	2
#define ILO2_RIBCL_CTL_AUTO_POWER	3
#define ILO2_RIBCL_CONTROL_3	3

/*
 * UID Control status definitions
*/
#define ILO2_RIBCL_UID_OFF	0
#define ILO2_RIBCL_UID_ON	1
#define ILO2_RIBCL_UID_SET_SUCCESS	1
#define ILO2_RIBCL_UID_SET_FAILED	0

/* Power Saver Control defines */
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
/* OS Control Mode (Disabled Mode for iLO) */
#define ILO2_RIBCL_MANUAL_OS_CONTROL_MODE 1
/* HP Static Low Power Mode */
#define ILO2_RIBCL_MANUAL_LOW_POWER_MODE 2
/* HP Dynamic Power Savings Mode. */
#define ILO2_RIBCL_AUTO_POWER_SAVE_MODE 3
/* HP Static High Performance Mode */
#define ILO2_RIBCL_MANUAL_HIGH_PERF_MODE 4

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
#define ILO2_RIBCL_AUTO_POWER_ENABLED 1
#define ILO2_RIBCL_AUTO_POWER_DISABLED 2
#define ILO2_RIBCL_AUTO_POWER_DELAY_RANDOM 3
#define ILO2_RIBCL_AUTO_POWER_DELAY_15 15
#define ILO2_RIBCL_AUTO_POWER_DELAY_30 30
#define ILO2_RIBCL_AUTO_POWER_DELAY_45 45
#define ILO2_RIBCL_AUTO_POWER_DELAY_60 60

/*
	iLO2 RIBCL plug-in intenal data structure to save plug-in private
	control data. This structure is used to save the current mode and
	current state information of a control.
*/
typedef struct ilo2_ribcl_cinfo {
	int ctl_type;	/* internal control type */
	SaHpiCtrlModeT cur_mode;
	SaHpiCtrlStateUnionT cur_state;
} ilo2_ribcl_cinfo_t;

/*****************************
	Prototypes for iLO2 RIBCL plug-in ABI functions
*****************************/

/* The following fucntions are defined in ilo2_ribcl.c */
extern void *ilo2_ribcl_open(GHashTable *, unsigned int , oh_evt_queue *);
extern void ilo2_ribcl_close(void *);
extern SaErrorT ilo2_ribcl_get_event(void *);

/* The following fucntions are defined in ilo2_ribcl_discover.c */
extern SaErrorT ilo2_ribcl_discover_resources(void *);

/* The following functions are defined in ilo2_ribcl_reset.c */
extern SaErrorT ilo2_ribcl_get_reset_state(void *, SaHpiResourceIdT,
	SaHpiResetActionT *);
extern SaErrorT ilo2_ribcl_set_reset_state(void *hnd, SaHpiResourceIdT rid,
				 SaHpiResetActionT act);

/* The following functions are defined in ilo2_ribcl_reset.c */
extern SaErrorT ilo2_ribcl_get_power_state(void *hnd,
				 SaHpiResourceIdT rid,
				 SaHpiPowerStateT *state);
extern SaErrorT ilo2_ribcl_set_power_state(void *hnd,
				 SaHpiResourceIdT rid,
				 SaHpiPowerStateT state);

/* The following functions are defined in ilo2_ribcl_rpt.c */

extern SaErrorT ilo2_ribcl_set_resource_severity(void *, SaHpiResourceIdT,
	SaHpiSeverityT);
extern SaErrorT ilo2_ribcl_set_resource_tag(void *, SaHpiResourceIdT,
	SaHpiTextBufferT *);

/* The following functions are defined in ilo2_ribcl_control.c */
extern SaErrorT ilo2_ribcl_get_control_state(void *, SaHpiResourceIdT ,
	SaHpiCtrlNumT , SaHpiCtrlModeT *, SaHpiCtrlStateT *);
extern SaErrorT ilo2_ribcl_set_control_state(void *, SaHpiResourceIdT ,
	SaHpiCtrlNumT , SaHpiCtrlModeT , SaHpiCtrlStateT *);

/* The following functions are defined in ilo2_ribcl_idr.c */
extern SaErrorT ilo2_ribcl_get_idr_info(void *, SaHpiResourceIdT, SaHpiIdrIdT,
			SaHpiIdrInfoT *);

extern SaErrorT ilo2_ribcl_get_idr_area_header(void *, SaHpiResourceIdT,
			SaHpiIdrIdT, SaHpiIdrAreaTypeT, SaHpiEntryIdT,
			SaHpiEntryIdT *, SaHpiIdrAreaHeaderT *);

extern SaErrorT ilo2_ribcl_get_idr_field(void *, SaHpiResourceIdT, SaHpiIdrIdT,
			SaHpiEntryIdT, SaHpiIdrFieldTypeT, SaHpiEntryIdT,
			SaHpiEntryIdT *, SaHpiIdrFieldT *);

extern SaErrorT ilo2_ribcl_add_idr_area( void *, SaHpiResourceIdT, SaHpiIdrIdT,
			SaHpiIdrAreaTypeT, SaHpiEntryIdT *);

extern SaErrorT ilo2_ribcl_del_idr_area( void *, SaHpiResourceIdT, SaHpiIdrIdT,
			SaHpiEntryIdT);

extern SaErrorT ilo2_ribcl_add_idr_field( void *, SaHpiResourceIdT, SaHpiIdrIdT,
			SaHpiIdrFieldT *);

extern SaErrorT ilo2_ribcl_set_idr_field( void *, SaHpiResourceIdT, SaHpiIdrIdT,
			SaHpiIdrFieldT *);
extern SaErrorT ilo2_ribcl_del_idr_field( void *, SaHpiResourceIdT, SaHpiIdrIdT,
			SaHpiEntryIdT, SaHpiEntryIdT);

extern SaErrorT ilo2_ribcl_add_idr( struct oh_handler_state *,
			struct oh_event *, SaHpiIdrIdT,
			struct ilo2_ribcl_idr_info *, char *);

/* The following functions are defined in ilo2_ribcl_idr.c */
extern SaErrorT ilo2_ribcl_get_sensor_reading(void *, SaHpiResourceIdT,
				SaHpiSensorNumT, SaHpiSensorReadingT *,
				SaHpiEventStateT *);
extern SaErrorT ilo2_ribcl_get_sensor_enable(void *hnd, SaHpiResourceIdT,
				SaHpiSensorNumT, SaHpiBoolT *);
extern SaErrorT ilo2_ribcl_set_sensor_enable(void *, SaHpiResourceIdT,
				SaHpiSensorNumT, SaHpiBoolT);
extern SaErrorT ilo2_ribcl_get_sensor_event_enable(void *, SaHpiResourceIdT,
				SaHpiSensorNumT, SaHpiBoolT *);
extern SaErrorT ilo2_ribcl_set_sensor_event_enable(void *, SaHpiResourceIdT,
				SaHpiSensorNumT, SaHpiBoolT);
extern SaErrorT ilo2_ribcl_get_sensor_event_masks(void *, SaHpiResourceIdT,
				SaHpiSensorNumT, SaHpiEventStateT *,
				SaHpiEventStateT *);
extern SaErrorT ilo2_ribcl_set_sensor_event_masks(void *, SaHpiResourceIdT,
				SaHpiSensorNumT, SaHpiSensorEventMaskActionT,
				SaHpiEventStateT, SaHpiEventStateT);

#endif /* _INC_ILO2_RIBCL_H_ */
