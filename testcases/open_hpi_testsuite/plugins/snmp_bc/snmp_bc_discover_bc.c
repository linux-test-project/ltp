/*      -*- linux-c -*-
 *
 * (C) Copyright IBM Corp. 2004, 2006
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. This
 * file and program are licensed under a BSD style license. See
 * the Copying file included with the OpenHPI distribution for
 * full licensing terms.
 *
 * Author(s):
 *      Peter Phan <pdphan@sourceforge.net>
 *      Steve Sherman <stevees@us.ibm.com>
 */

#include <glib.h>
#include <unistd.h>
#include <snmp_bc_plugin.h>

static void free_hash_data(gpointer key, gpointer value, gpointer user_data);


struct SensorMibInfo snmp_bc_ipmi_sensors_temp[SNMP_BC_MAX_IPMI_TEMP_SENSORS] = {
	{ /* Generic IPMI Temp Sensor 1 */
		.not_avail_indicator_num = 0,
		.write_only = SAHPI_FALSE,
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.3.1.12.x",
		.threshold_oids = {
			.UpCritical = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.22.x",
			.UpMajor    = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.23.x",
		},
		.threshold_write_oids = {},
	},
	{ /* Generic IPMI Temp Sensor 2 */
		.not_avail_indicator_num = 0,
		.write_only = SAHPI_FALSE,
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.3.1.13.x",
		.threshold_oids = {
			.UpCritical = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.25.x",
			.UpMajor    = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.26.x",
		},
		.threshold_write_oids = {},
	},
	{ /* Generic IPMI Temp Sensor 3 */
		.not_avail_indicator_num = 0,
		.write_only = SAHPI_FALSE,
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.3.1.14.x",
		.threshold_oids = {
			.UpCritical = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.28.x",
			.UpMajor    = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.29.x",
		},
		.threshold_write_oids = {},
	},
	{ /* Generic IPMI Temp Sensor 4 */
		.not_avail_indicator_num = 0,
		.write_only = SAHPI_FALSE,
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.3.1.15.x",
		.threshold_oids = {
			.UpCritical = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.31.x",
			.UpMajor    = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.32.x",
		},
		.threshold_write_oids = {},
	},
	{ /* Generic IPMI Temp Sensor 5 */
		.not_avail_indicator_num = 0,
		.write_only = SAHPI_FALSE,
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.3.1.16.x",
		.threshold_oids = {
			.UpCritical = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.34.x",
			.UpMajor    = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.35.x",
		},
		.threshold_write_oids = {},
	},
	{ /* Generic IPMI Temp Sensor 6 */
		.not_avail_indicator_num = 0,
		.write_only = SAHPI_FALSE,
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.3.1.17.x",
		.threshold_oids = {
			.UpCritical = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.37.x",
			.UpMajor    = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.38.x",
		},
		.threshold_write_oids = {},
	},
};

struct SensorMibInfo snmp_bc_ipmi_sensors_voltage[SNMP_BC_MAX_IPMI_VOLTAGE_SENSORS] = {
	{ /* Generic IPMI Voltage Sensor 1 */
		.not_avail_indicator_num = 0,
		.write_only = SAHPI_FALSE,
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.15.x",
		.threshold_oids = {
			.UpMajor  = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.23.x",
			.LowMajor = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.24.x",
		},
		.threshold_write_oids = {},
	},
	{ /* Generic IPMI Voltage Sensor 2 */
		.not_avail_indicator_num = 0,
		.write_only = SAHPI_FALSE,
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.16.x",
		.threshold_oids = {
			.UpMajor  = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.25.x",
			.LowMajor = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.26.x",
		},
		.threshold_write_oids = {},
	},
	{ /* Generic IPMI Voltage Sensor 3 */
		.not_avail_indicator_num = 0,
		.write_only = SAHPI_FALSE,
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.17.x",
		.threshold_oids = {
			.UpMajor  = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.27.x",
			.LowMajor = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.28.x",
		},
		.threshold_write_oids = {},
	},
	{ /* Generic IPMI Voltage Sensor 4 */
		.not_avail_indicator_num = 0,
		.write_only = SAHPI_FALSE,
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.18.x",
		.threshold_oids = {
			.UpMajor  = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.29.x",
			.LowMajor = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.30.x",
		},
		.threshold_write_oids = {},
	},
	{ /* Generic IPMI Voltage Sensor 5 */
		.not_avail_indicator_num = 0,
		.write_only = SAHPI_FALSE,
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.19.x",
		.threshold_oids = {
			.UpMajor  = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.31.x",
			.LowMajor = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.32.x",
		},
		.threshold_write_oids = {},
	},
	{ /* Generic IPMI Voltage Sensor 6 */
		.not_avail_indicator_num = 0,
		.write_only = SAHPI_FALSE,
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.20.x",
		.threshold_oids = {
			.UpMajor  = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.33.x",
			.LowMajor = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.34.x",
		},
		.threshold_write_oids = {},
	},
	{ /* Generic IPMI Voltage Sensor 7 */
		.not_avail_indicator_num = 0,
		.write_only = SAHPI_FALSE,
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.21.x",
		.threshold_oids = {
			.UpMajor  = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.35.x",
			.LowMajor = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.36.x",
		},
		.threshold_write_oids = {},
	},
	{ /* Generic IPMI Voltage Sensor 8 */
		.not_avail_indicator_num = 0,
		.write_only = SAHPI_FALSE,
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.22.x",
		.threshold_oids = {
			.UpMajor  = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.37.x",
			.LowMajor = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.38.x",
		},
		.threshold_write_oids = {},
	},
	{ /* Generic IPMI Voltage Sensor 9 */
		.not_avail_indicator_num = 0,
		.write_only = SAHPI_FALSE,
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.23.x",
		.threshold_oids = {
			.UpMajor  = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.39.x",
			.LowMajor = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.40.x",
		},
		.threshold_write_oids = {},
	},
	{ /* Generic IPMI Voltage Sensor 10 */
		.not_avail_indicator_num = 0,
		.write_only = SAHPI_FALSE,
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.24.x",
		.threshold_oids = {
			.UpMajor  = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.41.x",
			.LowMajor = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.42.x",
		},
		.threshold_write_oids = {},
	},
	{ /* Generic IPMI Voltage Sensor 11 */
		.not_avail_indicator_num = 0,
		.write_only = SAHPI_FALSE,
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.25.x",
		.threshold_oids = {
			.UpMajor  = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.43.x",
			.LowMajor = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.44.x",
		},
		.threshold_write_oids = {},
	},
	{ /* Generic IPMI Voltage Sensor 12 */
		.not_avail_indicator_num = 0,
		.write_only = SAHPI_FALSE,
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.26.x",
		.threshold_oids = {
			.UpMajor  = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.45.x",
			.LowMajor = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.46.x",
		},
		.threshold_write_oids = {},
	},
	{ /* Generic IPMI Voltage Sensor 13 */
		.not_avail_indicator_num = 0,
		.write_only = SAHPI_FALSE,
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.27.x",
		.threshold_oids = {
			.UpMajor  = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.47.x",
			.LowMajor = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.48.x",
		},
		.threshold_write_oids = {},
	},
	{ /* Generic IPMI Voltage Sensor 14 */
		.not_avail_indicator_num = 0,
		.write_only = SAHPI_FALSE,
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.28.x",
		.threshold_oids = {
			.UpMajor  = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.49.x",
			.LowMajor = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.50.x",
		},
		.threshold_write_oids = {},
	},
	{ /* Generic IPMI Voltage Sensor 15 */
		.not_avail_indicator_num = 0,
		.write_only = SAHPI_FALSE,
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.29.x",
		.threshold_oids = {
			.UpMajor  = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.51.x",
			.LowMajor = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.52.x",
		},
		.threshold_write_oids = {},
	},
	{ /* Generic IPMI Voltage Sensor 16 */
		.not_avail_indicator_num = 0,
		.write_only = SAHPI_FALSE,
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.30.x",
		.threshold_oids = {
			.UpMajor  = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.53.x",
			.LowMajor = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.54.x",
		},
		.threshold_write_oids = {},
	},
	{ /* Generic IPMI Voltage Sensor 17 */
		.not_avail_indicator_num = 0,
		.write_only = SAHPI_FALSE,
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.31.x",
		.threshold_oids = {
			.UpMajor  = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.55.x",
			.LowMajor = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.56.x",
		},
		.threshold_write_oids = {},
	},
	{ /* Generic IPMI Voltage Sensor 18 */
		.not_avail_indicator_num = 0,
		.write_only = SAHPI_FALSE,
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.32.x",
		.threshold_oids = {
			.UpMajor  = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.57.x",
			.LowMajor = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.58.x",
		},
		.threshold_write_oids = {},
	},
	{ /* Generic IPMI Voltage Sensor 19 */
		.not_avail_indicator_num = 0,
		.write_only = SAHPI_FALSE,
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.33.x",
		.threshold_oids = {
			.UpMajor  = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.59.x",
			.LowMajor = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.60.x",
		},
		.threshold_write_oids = {},
	},
	{ /* Generic IPMI Voltage Sensor 20 */
		.not_avail_indicator_num = 0,
		.write_only = SAHPI_FALSE,
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.34.x",
		.threshold_oids = {
			.UpMajor  = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.61.x",
			.LowMajor = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.62.x",
		},
		.threshold_write_oids = {},
	},
	{ /* Generic IPMI Voltage Sensor 21 */
		.not_avail_indicator_num = 0,
		.write_only = SAHPI_FALSE,
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.35.x",
		.threshold_oids = {
			.UpMajor  = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.63.x",
			.LowMajor = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.64.x",
		},
		.threshold_write_oids = {},
	},
	{ /* Generic IPMI Voltage Sensor 22 */
		.not_avail_indicator_num = 0,
		.write_only = SAHPI_FALSE,
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.36.x",
		.threshold_oids = {
			.UpMajor  = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.65.x",
			.LowMajor = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.66.x",
		},
		.threshold_write_oids = {},
	},
	{ /* Generic IPMI Voltage Sensor 23 */
		.not_avail_indicator_num = 0,
		.write_only = SAHPI_FALSE,
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.37.x",
		.threshold_oids = {
			.UpMajor  = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.67.x",
			.LowMajor = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.68.x",
		},
		.threshold_write_oids = {},
	},
	{ /* Generic IPMI Voltage Sensor 24 */
		.not_avail_indicator_num = 0,
		.write_only = SAHPI_FALSE,
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.38.x",
		.threshold_oids = {
			.UpMajor  = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.69.x",
			.LowMajor = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.70.x",
		},
		.threshold_write_oids = {},
	},
	{ /* Generic IPMI Voltage Sensor 25 */
		.not_avail_indicator_num = 0,
		.write_only = SAHPI_FALSE,
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.39.x",
		.threshold_oids = {
			.UpMajor  = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.71.x",
			.LowMajor = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.72.x",
		},
		.threshold_write_oids = {},
	},
	{ /* Generic IPMI Voltage Sensor 26 */
		.not_avail_indicator_num = 0,
		.write_only = SAHPI_FALSE,
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.40.x",
		.threshold_oids = {
			.UpMajor  = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.73.x",
			.LowMajor = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.74.x",
		},
		.threshold_write_oids = {},
	},
	{ /* Generic IPMI Voltage Sensor 27 */
		.not_avail_indicator_num = 0,
		.write_only = SAHPI_FALSE,
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.41.x",
		.threshold_oids = {
			.UpMajor  = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.75.x",
			.LowMajor = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.76.x",
		},
		.threshold_write_oids = {},
	},
	{ /* Generic IPMI Voltage Sensor 28 */
		.not_avail_indicator_num = 0,
		.write_only = SAHPI_FALSE,
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.42.x",
		.threshold_oids = {
			.UpMajor  = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.77.x",
			.LowMajor = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.78.x",
		},
		.threshold_write_oids = {},
	},
	{ /* Generic IPMI Voltage Sensor 29 */
		.not_avail_indicator_num = 0,
		.write_only = SAHPI_FALSE,
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.43.x",
		.threshold_oids = {
			.UpMajor  = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.79.x",
			.LowMajor = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.80.x",
		},
		.threshold_write_oids = {},
	},
	{ /* Generic IPMI Voltage Sensor 30 */
		.not_avail_indicator_num = 0,
		.write_only = SAHPI_FALSE,
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.44.x",
		.threshold_oids = {
			.UpMajor  = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.81.x",
			.LowMajor = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.82.x",
		},
		.threshold_write_oids = {},
	},
};

static SaErrorT snmp_bc_discover_ipmi_sensors(struct oh_handler_state *handle,
					      struct snmp_bc_ipmi_sensor *sensor_array,
					      struct oh_event *res_oh_event);
/* Matching mmblade.mib definitions */		
/*	storageExpansion(1),        */
/* 	pciExpansion(2)             */
char *bladeexpansiondesc[] = {
		"Blade Expansion Module, BEM",
		"Blade Storage Expansion, BSE",
		"Blade PCI I/O Expansion, PEU"
};		

/**
 * snmp_bc_discover:
 * @handler: Pointer to handler's data.
 * @ep_root: Pointer to chassis Root Entity Path which comes from openhpi.conf.
 *
 * Discovers IBM BladeCenter resources and RDRs.
 *
 * Return values:
 * SA_OK - normal case
 * SA_ERR_HPI_DUPLICATE - There is no changes to BladeCenter resource masks; normal case for re-discovery.
 * SA_ERR_HPI_OUT_OF_MEMORY - Cannot allocate space for internal memory.
 * SA_ERR_HPI_INVALID_PARAMS - Pointer parameter(s) NULL.
 **/

SaErrorT snmp_bc_discover(struct oh_handler_state *handle,
			  SaHpiEntityPathT *ep_root)
{

	SaErrorT err;
	struct snmp_value get_value_blade, get_value_blower,
			  get_value_power_module, get_value_switch,
			  get_value_media, get_value_mm, 
			  get_value_tap, get_value_nc, 
			  get_value_mx, get_value_smi,
			  get_value_filter, get_value_mmi;
	struct snmp_bc_hnd *custom_handle;


	if (!handle || !ep_root) {
		err("Invalid parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}
		
	custom_handle = (struct snmp_bc_hnd *)handle->data;
	if (!custom_handle) {
		err("Invalid parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}


	/* If this is a rediscovery, then sync event log before all else.    */
	/* If event log synchronization process finds any hotswap event,     */
	/* eventlog synchronization process will kick off a targeted discovery*/
	/* for the hotswap resource. This is for both removal and installation*/
	if (custom_handle->isFirstDiscovery == SAHPI_FALSE) 
		err = snmp_bc_check_selcache(handle, 1, SAHPI_NEWEST_ENTRY);
	
	/**************************************************************
	 * Fetch various resource installation vectors from BladeCenter
	 **************************************************************/

	/* Fetch blade installed vector */
	get_installed_mask(SNMP_BC_PB_INSTALLED, get_value_blade);

	/* Fetch switch installed vector */
	get_installed_mask(SNMP_BC_SM_INSTALLED, get_value_switch);		 

	/* Fetch MM installed vector */
	get_installed_mask(SNMP_BC_MM_INSTALLED, get_value_mm);
		
	/* Fetch power module installed vector */
	get_installed_mask(SNMP_BC_PM_INSTALLED, get_value_power_module);

	/* Fetch media tray installed vector */
	/* get_dualmode_object(SNMP_BC_MT_INSTALLED, get_value_media); */
	snmp_bc_fetch_MT_install_mask(handle, &get_value_media);
	

	/* Fetch filter (front bezel) installed vector */
	get_dualmode_object(SNMP_BC_FILTER_INSTALLED, get_value_filter);
		
	/* Fetch blower installed vector  */
	get_installed_mask(SNMP_BC_BLOWER_INSTALLED, get_value_blower);
	
	/* Fetch telco-alarm-panel installed vector  */
	get_installed_mask(SNMP_BC_AP_INSTALLED, get_value_tap);
	
	/* Fetch network-clock-card installed vector  */
	get_installed_mask(SNMP_BC_NC_INSTALLED, get_value_nc);
	
	/* Fetch mux-card installed vector  */
	get_installed_mask(SNMP_BC_MX_INSTALLED, get_value_mx);
	
	/* Fetch switch interposer-card installed vector  */
	get_installed_mask(SNMP_BC_SMI_INSTALLED, get_value_smi);
	
	/* Fetch mm interposer-card installed vector  */
	get_installed_mask(SNMP_BC_MMI_INSTALLED, get_value_mmi);	
						
	if (  (g_ascii_strncasecmp(get_value_blade.string, custom_handle->installed_pb_mask, get_value_blade.str_len) == 0) &&
		(g_ascii_strncasecmp(get_value_blower.string, custom_handle->installed_blower_mask, get_value_blower.str_len) == 0) &&
		(g_ascii_strncasecmp(get_value_power_module.string, custom_handle->installed_pm_mask, get_value_power_module.str_len) == 0) &&
		(g_ascii_strncasecmp(get_value_switch.string, custom_handle->installed_sm_mask, get_value_switch.str_len) == 0) &&		
		(g_ascii_strncasecmp(get_value_mm.string, custom_handle->installed_mm_mask, get_value_mm.str_len) == 0) &&
		(g_ascii_strncasecmp(get_value_tap.string, custom_handle->installed_tap_mask, get_value_tap.str_len) == 0) &&
		(g_ascii_strncasecmp(get_value_nc.string, custom_handle->installed_nc_mask, get_value_nc.str_len) == 0) &&
		(g_ascii_strncasecmp(get_value_mx.string, custom_handle->installed_mx_mask, get_value_mx.str_len) == 0) &&
		(g_ascii_strncasecmp(get_value_mmi.string, custom_handle->installed_mmi_mask, get_value_smi.str_len) == 0) &&
		(g_ascii_strncasecmp(get_value_smi.string, custom_handle->installed_smi_mask, get_value_smi.str_len) == 0) &&
		(get_value_filter.integer == custom_handle->installed_filter_mask) &&
		(get_value_media.integer == custom_handle->installed_mt_mask) ) {
		
		
		/**************************************************** 
		 * If **ALL** the resource masks are still the same, 
		 * do not rediscover, return with special return code 
		 ****************************************************/
		return(SA_ERR_HPI_DUPLICATE);
	} else {
		/*************************************************************
                 * Set saved masks to the newly read values
		 * Use strcpy() instead of strncpy(), counting on snmp_utils.c
		 * to NULL terminate strings read from snmp agent
		 *************************************************************/
		err = snmp_bc_update_chassis_topo(handle);
		if (err != SA_OK) return (err);
		
		strncpy(custom_handle->installed_pb_mask, get_value_blade.string, SNMP_BC_MAX_RESOURCES_MASK);
		strncpy(custom_handle->installed_blower_mask, get_value_blower.string, SNMP_BC_MAX_RESOURCES_MASK);
		strncpy(custom_handle->installed_pm_mask, get_value_power_module.string, SNMP_BC_MAX_RESOURCES_MASK);
		strncpy(custom_handle->installed_smi_mask, get_value_smi.string, SNMP_BC_MAX_RESOURCES_MASK);
		strncpy(custom_handle->installed_sm_mask, get_value_switch.string, SNMP_BC_MAX_RESOURCES_MASK);
		strncpy(custom_handle->installed_mmi_mask, get_value_mmi.string, SNMP_BC_MAX_RESOURCES_MASK);		
		strncpy(custom_handle->installed_mm_mask, get_value_mm.string, SNMP_BC_MAX_RESOURCES_MASK);
		strncpy(custom_handle->installed_tap_mask, get_value_tap.string, SNMP_BC_MAX_RESOURCES_MASK);
		strncpy(custom_handle->installed_nc_mask, get_value_nc.string, SNMP_BC_MAX_RESOURCES_MASK);
		strncpy(custom_handle->installed_mx_mask, get_value_mx.string, SNMP_BC_MAX_RESOURCES_MASK);
		custom_handle->installed_mt_mask = get_value_media.integer;
		custom_handle->installed_filter_mask = get_value_filter.integer;		
	}
		
	/******************************
	 * Discover BladeCenter Chassis
	 ******************************/
	err = snmp_bc_discover_chassis(handle, ep_root);
	if (err != SA_OK) return(err);
															
	/******************************
	 * Discover ALL BladeCenter Slots
	 * Warning: 
	 *   Discovery of Physical Slots must come **before** discovery of sloted resources.
	 *   Discovery of slots sets Slot State Sensor to empty.
	 *   Subsequent resource discovery changes state of Slot State Sensor accordingly.   
	 ******************************/
	err = snmp_bc_discover_all_slots(handle, ep_root);
	if (err != SA_OK) return(err);
				  
	/***************** 
	 * Discover Blades
	 *****************/
	err = snmp_bc_discover_blade(handle, ep_root,get_value_blade.string);
	if (err != SA_OK) return(err);
				  			
        /******************
	 * Discover Blowers
	 ******************/
	err = snmp_bc_discover_blowers(handle, ep_root, get_value_blower.string);
	if (err != SA_OK) return(err);
	
        /************************
	 * Discover Power Modules
	 ************************/
	err = snmp_bc_discover_power_module(handle, ep_root, get_value_power_module.string);
	if (err != SA_OK) return(err);

	/*********************************** 
	 * Discover Switch Module Interposers (smi)
	 * It is **important** to update custom_handle->installed_smi_mask
	 *   **prior** to switch discovery (discover_sm)
	 ***********************************/		
	err = snmp_bc_discover_smi(handle, ep_root, get_value_smi.string);
	if (err != SA_OK) return(err);
		
	/******************* 
	 * Discover Switches
	 *******************/
	err = snmp_bc_discover_switch(handle, ep_root, get_value_switch.string);
	if (err != SA_OK) return(err);
	
	/**********************
	 * Discover Media Trays
	 **********************/
	err = snmp_bc_discover_media_tray(handle, ep_root, get_value_media.integer);
	if (err != SA_OK) return(err);
	
	/**********************
	 * Discover Filter (Front Bezel)
	 **********************/
	err = snmp_bc_discover_filter(handle, ep_root, get_value_filter.integer);
	if (err != SA_OK) return(err);
	
	/*********************************** 
	 * Discover Management Module Interposers (mmi)
	 * It is **important** to update custom_handle->installed_mmi_mask
	 *   **prior** to mm discovery (discover_mm)
	 ***********************************/		
	err = snmp_bc_discover_mmi(handle, ep_root, get_value_mmi.string);
	if (err != SA_OK) return(err);
		
	/*********************************** 
	 * Discover Management Modules (MMs)
	 ***********************************/
	err = snmp_bc_discover_mm(handle, ep_root, get_value_mm.string, SAHPI_TRUE);
	if (err != SA_OK) return(err);

	/*********************************** 
	 * Discover Telco Alarm Panel (TAPs)
	 ***********************************/
	err = snmp_bc_discover_tap(handle, ep_root, get_value_tap.string);
	if (err != SA_OK) return(err);

	/*********************************** 
	 * Discover Network Clock Cards (nc)
 	 ***********************************/		
	err = snmp_bc_discover_nc(handle, ep_root, get_value_nc.string);
	if (err != SA_OK) return(err);

	/*********************************** 
	 * Discover Mux Cards (mx)
	 ***********************************/		
	err = snmp_bc_discover_mx(handle, ep_root, get_value_mx.string);
	if (err != SA_OK) return(err);


	return(SA_OK);
}

/**
 * snmp_bc_update_chassis_topo:
 * @handler: Pointer to handler's data.
 *
 * Update OpenHPI snmp_bc chassis topology from BladeCenter.
 *
 * Return values:
 * SA_OK - normal case.
 **/
SaErrorT snmp_bc_update_chassis_topo(struct oh_handler_state *handle)
{
	SaErrorT err;
	struct snmp_value get_value;
	struct snmp_bc_hnd *custom_handle;

	if (!handle) {
		err("Invalid parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}
		
	custom_handle = (struct snmp_bc_hnd *)handle->data;
	if (!custom_handle) {
		err("Invalid parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}
	
	if (custom_handle->isFirstDiscovery == SAHPI_TRUE) {
	
		get_integer_object(SNMP_BC_NOS_PB_SUPPORTED, get_value);
		custom_handle->max_pb_supported = get_value.integer;		/* pb - processor blade   */

		get_integer_object(SNMP_BC_NOS_SMI_SUPPORTED, get_value);		
		custom_handle->max_smi_supported = get_value.integer;	        /* smi - switch interposer */
		
		get_integer_object(SNMP_BC_NOS_SM_SUPPORTED, get_value);		
		custom_handle->max_sm_supported = get_value.integer;		/* sm - switch module     */

		get_integer_object(SNMP_BC_NOS_MMI_SUPPORTED, get_value);		
		custom_handle->max_mmi_supported = get_value.integer;	        /* mmi - mm interposer    */
				
		get_integer_object(SNMP_BC_NOS_MM_SUPPORTED, get_value);		
		custom_handle->max_mm_supported = get_value.integer;		/* mm - management module */

		get_integer_object(SNMP_BC_NOS_PM_SUPPORTED, get_value);
		custom_handle->max_pm_supported = get_value.integer;		/* pm - power module      */
		
		get_integer_object(SNMP_BC_NOS_MT_SUPPORTED, get_value);		
		custom_handle->max_mt_supported = get_value.integer;		/* mt - media tray        */

		get_integer_object(SNMP_BC_NOS_BLOWER_SUPPORTED, get_value);		
		custom_handle->max_blower_supported = get_value.integer;	/* blower - blower        */

		get_integer_object(SNMP_BC_NOS_FILTER_SUPPORTED, get_value);		
		custom_handle->max_filter_supported = get_value.integer;	/* filter - front bezel   */

		get_integer_object(SNMP_BC_NOS_AP_SUPPORTED, get_value);		
		custom_handle->max_tap_supported = get_value.integer;	        /* ap - alarm panel       */

		get_integer_object(SNMP_BC_NOS_NC_SUPPORTED, get_value);		
		custom_handle->max_nc_supported = get_value.integer;	        /* nc - network clock     */

		get_integer_object(SNMP_BC_NOS_MX_SUPPORTED, get_value);		
		custom_handle->max_mx_supported = get_value.integer;	        /* mx - multiplex (mux)   */
				
	 }
	
	return(SA_OK);
}

/**
 * snmp_bc_discover_media_tray:
 * @handler: Pointer to handler's data.
 * @ep_root: Pointer to chassis Root Entity Path which comes from openhpi.conf.
 * @media_tray_installed: Media tray installed flag.
 *
 * Discovers media tray resources and their RDRs.
 *
 * Return values:
 * SA_OK - normal case.
 * SA_ERR_HPI_OUT_OF_MEMORY - Cannot allocate space for internal memory.
 * SA_ERR_HPI_INVALID_PARAMS - Pointer parameters are NULL.
 **/
SaErrorT snmp_bc_discover_media_tray(struct oh_handler_state *handle,
			  	     SaHpiEntityPathT *ep_root, 
				     int  media_tray_installed)
{

	SaErrorT err;
	guint mt_width;
        struct oh_event *e;
	struct snmp_value get_value;
	struct ResourceInfo *res_info_ptr;
	struct snmp_bc_hnd *custom_handle;


	if (!handle || !ep_root) {
		err("Invalid parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}
		
	custom_handle = (struct snmp_bc_hnd *)handle->data;
	if (!custom_handle) {
		err("Invalid parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}

	e = snmp_bc_alloc_oh_event();
	if (e == NULL) {
		err("Out of memory.");
		return(SA_ERR_HPI_OUT_OF_MEMORY);
	}
	
	/* ---------------------------------------- */
	/* Construct .resource of struct oh_event   */
	/* ---------------------------------------- */		
	e->resource = snmp_bc_rpt_array[BC_RPT_ENTRY_MEDIA_TRAY].rpt;
	oh_concat_ep(&(e->resource.ResourceEntity), ep_root);
	oh_set_ep_location(&(e->resource.ResourceEntity),
			   BLADECENTER_PERIPHERAL_BAY_SLOT, SNMP_BC_HPI_LOCATION_BASE);
	oh_set_ep_location(&(e->resource.ResourceEntity),
			   SAHPI_ENT_PERIPHERAL_BAY, SNMP_BC_HPI_LOCATION_BASE);
	e->resource.ResourceId = 
		oh_uid_from_entity_path(&(e->resource.ResourceEntity));
	snmp_bc_create_resourcetag(&(e->resource.ResourceTag),
				   snmp_bc_rpt_array[BC_RPT_ENTRY_MEDIA_TRAY].comment,
				   SNMP_BC_HPI_LOCATION_BASE);

	dbg("Discovered resource=%s; ID=%d",
	    e->resource.ResourceTag.Data,
	    e->resource.ResourceId);

	/* Create platform-specific info space to add to infra-structure */
	res_info_ptr = g_memdup(&(snmp_bc_rpt_array[BC_RPT_ENTRY_MEDIA_TRAY].res_info),
					sizeof(struct ResourceInfo));
	if (!res_info_ptr) {
		err("Out of memory.");
		snmp_bc_free_oh_event(e);
		return(SA_ERR_HPI_OUT_OF_MEMORY);
	}


	if (media_tray_installed < 10) {
		res_info_ptr->cur_state = SAHPI_HS_STATE_NOT_PRESENT;
		snmp_bc_discover_res_events(handle, &(e->resource.ResourceEntity), res_info_ptr);
		snmp_bc_free_oh_event(e);
		g_free(res_info_ptr);

	} else if (media_tray_installed  >= 10 ) {
		res_info_ptr->cur_state = SAHPI_HS_STATE_ACTIVE;

                /* Get UUID and convert to GUID */
                err = snmp_bc_get_guid(custom_handle, e, res_info_ptr);
		
		/* Add resource to resource */
		err = oh_add_resource(handle->rptcache, 
				      &(e->resource),
				      res_info_ptr, 0);
		if (err) {
			err("Failed to add resource. Error=%s.", oh_lookup_error(err));
			snmp_bc_free_oh_event(e);
			return(err);
		}
		
		/* Add resource event entries to event2hpi_hash table */
		snmp_bc_discover_res_events(handle, &(e->resource.ResourceEntity), res_info_ptr);

		/* ---------------------------------------- */
		/* Construct .rdrs of struct oh_event       */
		/* ---------------------------------------- */		
		/* Find resource's rdrs: sensors, controls, etc. */
		if (custom_handle->platform == SNMP_BC_PLATFORM_BCHT) {
			snmp_bc_discover_sensors(handle, snmp_bc_mediatray_sensors_faultled, e);	
		}
		else {
			snmp_bc_discover_sensors(handle, snmp_bc_mediatray_sensors_nofaultled, e);	
		}
		snmp_bc_discover_sensors(handle, snmp_bc_mediatray_sensors, e);
		snmp_bc_discover_controls(handle, snmp_bc_mediatray_controls, e);
		snmp_bc_discover_inventories(handle, snmp_bc_mediatray_inventories, e);

		mt_width = 1;    /* Default to 1-wide */
		if (res_info_ptr->mib.OidResourceWidth != NULL) {
			err = snmp_bc_oid_snmp_get(custom_handle,  &(e->resource.ResourceEntity), 0,
						   res_info_ptr->mib.OidResourceWidth, &get_value, SAHPI_TRUE);
			if (!err && (get_value.type == ASN_INTEGER)) {
					mt_width = get_value.integer;
			}
		}			

		res_info_ptr->resourcewidth = mt_width;
		err = snmp_bc_set_resource_slot_state_sensor(handle, e, mt_width);
	
		/* ---------------------------------------- */
		/* Construct .event of struct oh_event      */	
		/* ---------------------------------------- */
		snmp_bc_set_resource_add_oh_event(e, res_info_ptr);
		
		/* ---------------------------------------- */
		/* Place the event in tmpqueue              */
		/* ---------------------------------------- */	
		/*custom_handle->eventq = g_slist_append(custom_handle->eventq, e);*/
                e->hid = handle->hid;
                oh_evt_queue_push(handle->eventq, e);
		
	}

	/* ------------------------------------------------ */
	/* For BC-HT, we have to examine the 2nd media tray */
	/* ------------------------------------------------ */
	if (custom_handle->platform == SNMP_BC_PLATFORM_BCHT)
	{	
		e = snmp_bc_alloc_oh_event();
		if (e == NULL) {
			err("Out of memory.");
			return(SA_ERR_HPI_OUT_OF_MEMORY);
		}
	
		/* ---------------------------------------- */
		/* Construct .resource of struct oh_event   */
		/* ---------------------------------------- */		
		e->resource = snmp_bc_rpt_array[BC_RPT_ENTRY_MEDIA_TRAY_2].rpt;
		oh_concat_ep(&(e->resource.ResourceEntity), ep_root);
		oh_set_ep_location(&(e->resource.ResourceEntity),
				BLADECENTER_PERIPHERAL_BAY_SLOT, SNMP_BC_HPI_LOCATION_BASE+1);
		oh_set_ep_location(&(e->resource.ResourceEntity),
				SAHPI_ENT_PERIPHERAL_BAY, SNMP_BC_HPI_LOCATION_BASE + 1);
		e->resource.ResourceId = 
			oh_uid_from_entity_path(&(e->resource.ResourceEntity));
		snmp_bc_create_resourcetag(&(e->resource.ResourceTag),
				   snmp_bc_rpt_array[BC_RPT_ENTRY_MEDIA_TRAY_2].comment,
				   SNMP_BC_HPI_LOCATION_BASE + 1);

		dbg("Discovered resource=%s; ID=%d",
	       	    e->resource.ResourceTag.Data,
		    e->resource.ResourceId);

		/* Create platform-specific info space to add to infra-structure */
		res_info_ptr = g_memdup(&(snmp_bc_rpt_array[BC_RPT_ENTRY_MEDIA_TRAY_2].res_info),
						sizeof(struct ResourceInfo));
		if (!res_info_ptr) {
			err("Out of memory.");
			snmp_bc_free_oh_event(e);
			return(SA_ERR_HPI_OUT_OF_MEMORY);
		}


		if ((media_tray_installed != 01) && (media_tray_installed != 11)) {
			res_info_ptr->cur_state = SAHPI_HS_STATE_NOT_PRESENT;
			snmp_bc_discover_res_events(handle, &(e->resource.ResourceEntity), res_info_ptr);
			snmp_bc_free_oh_event(e);
			g_free(res_info_ptr);

		} else {
			res_info_ptr->cur_state = SAHPI_HS_STATE_ACTIVE;

                	/* Get UUID and convert to GUID */
                	err = snmp_bc_get_guid(custom_handle, e, res_info_ptr);
		
			/* Add resource to resource */
			err = oh_add_resource(handle->rptcache, 
					      &(e->resource),
					      res_info_ptr, 0);
			if (err) {
				err("Failed to add resource. Error=%s.", oh_lookup_error(err));
				snmp_bc_free_oh_event(e);
				return(err);
			}
		
			/* Add resource event entries to event2hpi_hash table */
			snmp_bc_discover_res_events(handle, &(e->resource.ResourceEntity), res_info_ptr);

			/* --------------------------------------------- */
			/*      Construct .rdrs of struct oh_event       */
			/* --------------------------------------------- */		
			/* Find resource's rdrs: sensors, controls, etc. */
			snmp_bc_discover_sensors(handle, snmp_bc_mediatray2_sensors, e);
			snmp_bc_discover_controls(handle, snmp_bc_mediatray2_controls, e);
			snmp_bc_discover_inventories(handle, snmp_bc_mediatray2_inventories, e);

			mt_width = 1;    /* Default to 1-wide */
			if (res_info_ptr->mib.OidResourceWidth != NULL) {
				err = snmp_bc_oid_snmp_get(custom_handle,  &(e->resource.ResourceEntity), 0,
							   res_info_ptr->mib.OidResourceWidth, &get_value, SAHPI_TRUE);
				if (!err && (get_value.type == ASN_INTEGER)) {
						mt_width = get_value.integer;
				}
			}			
			res_info_ptr->resourcewidth = mt_width;
			err = snmp_bc_set_resource_slot_state_sensor(handle, e, mt_width);
	
			/* ---------------------------------------- */
			/* Construct .event of struct oh_event      */	
			/* ---------------------------------------- */
			snmp_bc_set_resource_add_oh_event(e, res_info_ptr);
		
			/* ---------------------------------------- */
			/* Place the event in tmpqueue              */
			/* ---------------------------------------- */	
			/*handle->eventq = g_slist_append(handle->eventq, e);*/
			e->hid = handle->hid;
	                oh_evt_queue_push(handle->eventq, e);
		
		}
	
	}
  
	/* ---------------------------------------- */	
	return(SA_OK);
}

/**
 * snmp_bc_discover_filter:
 * @handler: Pointer to handler's data.
 * @ep_root: Pointer to chassis Root Entity Path which comes from openhpi.conf.
 * @media_tray_installed: Filter installed flag.
 *
 * Discovers filter resources and their RDRs.
 *
 * Return values:
 * SA_OK - normal case.
 * SA_ERR_HPI_OUT_OF_MEMORY - Cannot allocate space for internal memory.
 * SA_ERR_HPI_INVALID_PARAMS - Pointer parameters are NULL.
 **/
SaErrorT snmp_bc_discover_filter(struct oh_handler_state *handle,
			  	     SaHpiEntityPathT *ep_root, 
				     int  filter_installed)
{

	SaErrorT err;
        struct oh_event *e;
	struct ResourceInfo *res_info_ptr;
	struct snmp_bc_hnd *custom_handle;


	if (!handle || !ep_root) {
		err("Invalid parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}
		
	custom_handle = (struct snmp_bc_hnd *)handle->data;
	if (!custom_handle) {
		err("Invalid parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}

	e = snmp_bc_alloc_oh_event();
	if (e == NULL) {
		err("Out of memory.");
		return(SA_ERR_HPI_OUT_OF_MEMORY);
	}
	
	/* ---------------------------------------- */
	/* Construct .resource of struct oh_event   */
	/* ---------------------------------------- */		
	e->resource = snmp_bc_rpt_array[BC_RPT_ENTRY_AIR_FILTER].rpt;
	oh_concat_ep(&(e->resource.ResourceEntity), ep_root);
	oh_set_ep_location(&(e->resource.ResourceEntity),
			   (SAHPI_ENT_PHYSICAL_SLOT + 16), SNMP_BC_HPI_LOCATION_BASE);
	e->resource.ResourceId = 
		oh_uid_from_entity_path(&(e->resource.ResourceEntity));
	snmp_bc_create_resourcetag(&(e->resource.ResourceTag),
				   snmp_bc_rpt_array[BC_RPT_ENTRY_AIR_FILTER].comment,
				   SNMP_BC_HPI_LOCATION_BASE);

	dbg("Discovered resource=%s; ID=%d",
	    e->resource.ResourceTag.Data,
	    e->resource.ResourceId);

	/* Create platform-specific info space to add to infra-structure */
	res_info_ptr = g_memdup(&(snmp_bc_rpt_array[BC_RPT_ENTRY_AIR_FILTER].res_info),
					sizeof(struct ResourceInfo));
	if (!res_info_ptr) {
		err("Out of memory.");
		snmp_bc_free_oh_event(e);
		return(SA_ERR_HPI_OUT_OF_MEMORY);
	}


	if (filter_installed == 0) {
		res_info_ptr->cur_state = SAHPI_HS_STATE_NOT_PRESENT;
		snmp_bc_discover_res_events(handle, &(e->resource.ResourceEntity), res_info_ptr);
		snmp_bc_free_oh_event(e);
		g_free(res_info_ptr);

	} else {
		res_info_ptr->cur_state = SAHPI_HS_STATE_ACTIVE;

                /* Get UUID and convert to GUID */
                err = snmp_bc_get_guid(custom_handle, e, res_info_ptr);
		
		/* Add resource to resource */
		err = oh_add_resource(handle->rptcache, 
				      &(e->resource),
				      res_info_ptr, 0);
		if (err) {
			err("Failed to add resource. Error=%s.", oh_lookup_error(err));
			snmp_bc_free_oh_event(e);
			return(err);
		}
		
		/* Add resource event entries to event2hpi_hash table */
		snmp_bc_discover_res_events(handle, &(e->resource.ResourceEntity), res_info_ptr);

		/* ---------------------------------------- */
		/* Construct .rdrs of struct oh_event       */
		/* ---------------------------------------- */		
		/* Find resource's rdrs: sensors, controls, etc. */		
		snmp_bc_discover_sensors(handle, snmp_bc_filter_sensors, e);
		snmp_bc_discover_controls(handle, snmp_bc_filter_controls, e);
		snmp_bc_discover_inventories(handle, snmp_bc_filter_inventories, e);

		/* ---------------------------------------- */
		/* Construct .event of struct oh_event      */	
		/* ---------------------------------------- */
		snmp_bc_set_resource_add_oh_event(e, res_info_ptr);
		
		/* ---------------------------------------- */
		/* Place the event in tmpqueue              */
		/* ---------------------------------------- */	
		/*custom_handle->eventq = g_slist_append(custom_handle->eventq, e);*/
                e->hid = handle->hid;
                oh_evt_queue_push(handle->eventq, e);
		
	}
  
	/* ---------------------------------------- */	
	return(SA_OK);
}

/**
 * snmp_bc_discover_chassis:
 * @handler: Pointer to handler's data.
 * @ep_root: Pointer to chassis Root Entity Path which comes from openhpi.conf.
 *
 * Discovers the BladeCenter chassis resource and its RDRs.
 *
 * Return values:
 * SA_OK - normal case.
 * SA_ERR_HPI_OUT_OF_MEMORY - Cannot allocate space for internal memory.
 * SA_ERR_HPI_INVALID_PARAMS - Pointer parameter(s) NULL.
 **/
SaErrorT snmp_bc_discover_chassis(struct oh_handler_state *handle,
			  	  SaHpiEntityPathT *ep_root)
{

	SaErrorT err;
        struct oh_event *e;
	struct ResourceInfo *res_info_ptr;
	struct snmp_bc_hnd *custom_handle;


	if (!handle || !ep_root) {
		err("Invalid parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}
		
	custom_handle = (struct snmp_bc_hnd *)handle->data;
	if (!custom_handle) {
		err("Invalid parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}

	/****************** 
	 * Discover Chassis
	 ******************/
	e = snmp_bc_alloc_oh_event();
	if (e == NULL) {
		err("Out of memory.");
		return(SA_ERR_HPI_OUT_OF_MEMORY);
	}
	e->resource = snmp_bc_rpt_array[BC_RPT_ENTRY_CHASSIS].rpt;
	
	/* ---------------------------------------- */
	/* Construct .resource of struct oh_event   */
	/* ---------------------------------------- */	
	e->resource.ResourceEntity = *ep_root;
	e->resource.ResourceId = 
		oh_uid_from_entity_path(&(e->resource.ResourceEntity));
	{ /* Generate Chassis Resource Tag */
		SaHpiTextBufferT build_name;

		oh_init_textbuffer(&build_name);

		switch (custom_handle->platform) {
		case SNMP_BC_PLATFORM_BC:
			oh_append_textbuffer(&build_name, "BladeCenter Chassis");
			break;
		case SNMP_BC_PLATFORM_BCH:
			oh_append_textbuffer(&build_name, "BladeCenter H Chassis");
			break;
		case SNMP_BC_PLATFORM_BCT:
			oh_append_textbuffer(&build_name, "BladeCenter T Chassis");
			break;
		case SNMP_BC_PLATFORM_BCHT:
			oh_append_textbuffer(&build_name, "BladeCenter HT Chassis");
			break;			
		default:	
			oh_append_textbuffer(&build_name, "BladeCenter Chassis");
		}

		snmp_bc_create_resourcetag(&(e->resource.ResourceTag),
					   (char *)build_name.Data,
					   ep_root->Entry[0].EntityLocation);
	}

	dbg("Discovered resource=%s; ID=%d",
	    e->resource.ResourceTag.Data,
	    e->resource.ResourceId);

	/* Create platform-specific info space  */
	res_info_ptr = g_memdup(&(snmp_bc_rpt_array[BC_RPT_ENTRY_CHASSIS].res_info),
				sizeof(struct ResourceInfo));
	if (!res_info_ptr) {
		err("Out of memory.");
		snmp_bc_free_oh_event(e);
		return(SA_ERR_HPI_OUT_OF_MEMORY);
	}

	res_info_ptr->cur_state = SAHPI_HS_STATE_ACTIVE;

        /* Get UUID and convert to GUID */
        err = snmp_bc_get_guid(custom_handle, e, res_info_ptr);

	/* Add resource to resource */
	err = oh_add_resource(handle->rptcache, 
			      &(e->resource), 
			      res_info_ptr, 0);
	if (err) {
		err("Cannot add resource. Error=%s.", oh_lookup_error(err));
		snmp_bc_free_oh_event(e);
		return(err);
	}
	
	/* Add resource event entries to event2hpi_hash table */
	snmp_bc_discover_res_events(handle, &(e->resource.ResourceEntity), res_info_ptr);
		
	/* ---------------------------------------- */
	/* Construct .rdrs of struct oh_event       */
	/* ---------------------------------------- */		
	/* Find resource's rdrs: sensors, controls, etc. */
	snmp_bc_discover_sensors(handle, snmp_bc_chassis_sensors, e);
	if ( (custom_handle->platform == SNMP_BC_PLATFORM_BCT) ) {
		snmp_bc_discover_sensors(handle, snmp_bc_chassis_sensors_bct_filter, e);
	}
	if ( (custom_handle->platform == SNMP_BC_PLATFORM_BCT) || 
	     (custom_handle->platform == SNMP_BC_PLATFORM_BCHT) ){
		snmp_bc_discover_controls(handle, snmp_bc_chassis_controls_bct, e);
	}
	else if ( (custom_handle->platform == SNMP_BC_PLATFORM_BC) || 
		  (custom_handle->platform == SNMP_BC_PLATFORM_BCH) ) {
		snmp_bc_discover_controls(handle, snmp_bc_chassis_controls_bc, e);
	}

	snmp_bc_discover_inventories(handle, snmp_bc_chassis_inventories, e);
	
	
	/* ---------------------------------------- */
	/* Construct .event of struct oh_event      */	
	/* ---------------------------------------- */
	snmp_bc_set_resource_add_oh_event(e, res_info_ptr);

	/* ---------------------------------------- */
	/* Place the event in queue                 */
	/* ---------------------------------------- */	
	/*custom_handle->eventq = g_slist_append(custom_handle->eventq, e);*/
        e->hid = handle->hid;
        oh_evt_queue_push(handle->eventq, e);
	
	/* ---------------------------------------- */
	/* ---------------------------------------- */
	return(SA_OK);

}


/**
 * snmp_bc_discover_blade:
 * @handler: Pointer to handler's data.
 * @ep_root: Pointer to chassis Root Entity Path which comes from openhpi.conf.
 * @blade_vector: Bitmap vector of installed blades.
 *
 * Discovers blade resources and their RDRs.
 *
 * Return values:
 * SA_OK - normal case.
 * SA_ERR_HPI_OUT_OF_MEMORY - Cannot allocate space for internal memory.
 * SA_ERR_HPI_INVALID_PARAMS - Pointer parameter(s) NULL.
 **/
SaErrorT snmp_bc_discover_blade(struct oh_handler_state *handle,
			  SaHpiEntityPathT *ep_root, char *blade_vector)
{

	guint i;
	SaErrorT err;
        struct oh_event *e;
	struct ResourceInfo *res_info_ptr;
	struct snmp_bc_hnd *custom_handle;


	if (!handle || !blade_vector) {
		err("Invalid parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}
		
	custom_handle = (struct snmp_bc_hnd *)handle->data;
	if (!custom_handle) {
		err("Invalid parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}

	e = NULL;
	res_info_ptr = NULL;	
	for (i=0; i < strlen(blade_vector); i++) {
	
		if ((blade_vector[i] == '1') || (custom_handle->isFirstDiscovery == SAHPI_TRUE))
		{
			e = snmp_bc_alloc_oh_event();
			if (e == NULL) {
				err("Out of memory.");
				return(SA_ERR_HPI_OUT_OF_MEMORY);
			}
									
			/* ---------------------------------------- */
			/* Construct .resource of struct oh_event   */
			/* res_info is malloc in the construcion    */
			/* ---------------------------------------- */	
			err = snmp_bc_construct_blade_rpt(e, &res_info_ptr, ep_root, i);
			if (err) {
				snmp_bc_free_oh_event(e);
				return(err);
			}
		}
			
		if ((blade_vector[i] == '0') && 
		    (custom_handle->isFirstDiscovery == SAHPI_TRUE)) 
		{
			/* Make sure that we have static infomation 
			 * for this **empty** blade slot in hash during HPI initialization
			 */ 
			res_info_ptr->cur_state = SAHPI_HS_STATE_NOT_PRESENT;
			snmp_bc_discover_res_events(handle, &(e->resource.ResourceEntity), res_info_ptr);
			snmp_bc_free_oh_event(e);
			g_free(res_info_ptr);
			
		} else if (blade_vector[i] == '1') {
		
			err = snmp_bc_add_blade_rptcache(handle, e, res_info_ptr, i);
			if (err == SA_OK) {
				/* ---------------------------------------- */
				/* Construct .event of struct oh_event      */	
				/* ---------------------------------------- */
				snmp_bc_set_resource_add_oh_event(e, res_info_ptr);
			
				/* ---------------------------------------- */
				/* Place the event in tmpqueue              */
				/* ---------------------------------------- */					
				/*custom_handle->eventq = g_slist_append(custom_handle->eventq, e);*/
                                if (e) e->hid = handle->hid;
                                oh_evt_queue_push(handle->eventq, e);
				/********************************** 
	 			 * Discover Blade Expansion Modules
	 			 **********************************/
				err = snmp_bc_discover_blade_expansion(handle, ep_root, i);
				
			} else {
				snmp_bc_free_oh_event(e);
                        }
		}
	}

	return(SA_OK);
}


/**
 * snmp_bc_discover_blade_expansion:
 * @handler: Pointer to handler's data.
 * @ep_root: Pointer to chassis Root Entity Path which comes from openhpi.conf.
 * @blade_index: Index of the main blade 
 *
 * Discovers blade expansion resources and their RDRs, if any.
 *
 * Return values:
 * SA_OK - normal case.
 * SA_ERR_HPI_OUT_OF_MEMORY - Cannot allocate space for internal memory.
 * SA_ERR_HPI_INVALID_PARAMS - Pointer parameter(s) NULL.
 **/
SaErrorT snmp_bc_discover_blade_expansion(struct oh_handler_state *handle,
			  		  SaHpiEntityPathT *ep_root, 
					  guint blade_index)
{

	SaErrorT err;
	gint i, j;
	SaHpiEntityPathT ep;
	struct snmp_value get_value;
	BCExpansionTypeT expansionType;
	struct snmp_bc_hnd *custom_handle;


	if (!handle) {
		err("Invalid parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}
		
	custom_handle = (struct snmp_bc_hnd *)handle->data;
	if (!custom_handle) {
		err("Invalid parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}
			
	ep = snmp_bc_rpt_array[BC_RPT_ENTRY_BLADE_EXPANSION_CARD].rpt.ResourceEntity;
	oh_concat_ep(&ep, ep_root);
	oh_set_ep_location(&ep, SAHPI_ENT_PHYSICAL_SLOT, blade_index + SNMP_BC_HPI_LOCATION_BASE);
	oh_set_ep_location(&ep, SAHPI_ENT_SBC_BLADE, blade_index + SNMP_BC_HPI_LOCATION_BASE);

	/* Determine which scheme to detect blade expansion */
	
	/* Set entity_path index to the first entry (SNMP_BC_HPI_LOCATION_BASE) in table */
	oh_set_ep_location(&ep, SAHPI_ENT_SYS_EXPANSION_BOARD, SNMP_BC_HPI_LOCATION_BASE);
	/* Go get value at (SNMP_BC_HPI_LOCATION_BASE + offset 0) */	
	err = snmp_bc_oid_snmp_get(custom_handle, &ep, 0,
				   SNMP_BC_BLADE_EXP_BLADE_BAY, &get_value, SAHPI_TRUE);

	j = 0;
	if (err ==  SA_ERR_HPI_NOT_PRESENT) {
		
		/* No object exists with SNMP_BC_BLADE_EXP_BLADE_BAY oid */
		/* Ether the target is running with older MM mib version,*/
		/* or there is no expansion board at all in system.      */
		/* Use old scheme to discover BladeExpandion resource.   */
		
		/* Set entity_path index to the desired entry  */
		/* (blade_index + SNMP_BC_HPI_LOCATION_BASE) in table */	   	
		oh_set_ep_location(&ep, SAHPI_ENT_SYS_EXPANSION_BOARD, 
							blade_index + SNMP_BC_HPI_LOCATION_BASE);
		/* Go get value at (SNMP_BC_HPI_LOCATION_BASE + offset 0) */								
		err = snmp_bc_oid_snmp_get(custom_handle, &ep, 0,
					SNMP_BC_BLADE_EXPANSION_VECTOR, &get_value, SAHPI_TRUE);

		/* With the old scheme, we can only detect one of the blade expansion board */ 
                /* For example, if a blade has BSE and PEU, we see only one with this scheme*/
		oh_set_ep_location(&ep, SAHPI_ENT_SYS_EXPANSION_BOARD, j + SNMP_BC_HPI_LOCATION_BASE);

		if (!err && get_value.integer != 0) {
			err = snmp_bc_add_blade_expansion_resource(handle, &ep, blade_index,
				 DEFAULT_BLADE_EXPANSION_CARD_TYPE, j);
		}
	} else if(err ==  SA_OK) {
	
		/* New scheme; i == index for Processor Blade, j == index for Blade Expansion for each Processor Blade  */
		for (i=0; i < (custom_handle->max_pb_supported ); i++) {

			/* Set entity_path index to the first entry (SNMP_BC_HPI_LOCATION_BASE) in table */
			oh_set_ep_location(&ep, SAHPI_ENT_SYS_EXPANSION_BOARD, 
							    SNMP_BC_HPI_LOCATION_BASE);

			/* Go get value at (SNMP_BC_HPI_LOCATION_BASE + offset i) */
			err = snmp_bc_oid_snmp_get(custom_handle, &ep, i,
				   SNMP_BC_BLADE_EXP_BLADE_BAY, &get_value, SAHPI_TRUE);
				   
			if (err == SA_OK) {
				if (get_value.type != ASN_OCTET_STR) continue;
				

				if ( atoi(get_value.string) == (blade_index + SNMP_BC_HPI_LOCATION_BASE)) {
				
					/* Go get value at (SNMP_BC_HPI_LOCATION_BASE + offset i) */
					err = snmp_bc_oid_snmp_get(custom_handle, &ep, i,
				   			SNMP_BC_BLADE_EXP_TYPE, &get_value, SAHPI_TRUE);
					
					if ((err == SA_OK) && (get_value.type == ASN_INTEGER)) {
						/*		
						storageExpansion(1),
                    				pciExpansion(2)
						*/	
						expansionType = get_value.integer;
					} else 	{
						err(" Error reading Expansion Board Type\n");
						expansionType = DEFAULT_BLADE_EXPANSION_CARD_TYPE;
					}
					
					
					oh_set_ep_location(&ep, SAHPI_ENT_SYS_EXPANSION_BOARD, 
										j + SNMP_BC_HPI_LOCATION_BASE);
																
				 	err = snmp_bc_add_blade_expansion_resource(handle, &ep, blade_index, expansionType, j);
					j++;
			
				}
			}
		
		} /* end for custom_handle->max_pb_supported */
	} 

	return(SA_OK);
}


/**
 * snmp_bc_add_blade_expansion_resource:
 * @handler: Pointer to handler's data.
 * @ep_root: Pointer to chassis Root Entity Path which comes from openhpi.conf.
 * @blade_index: Index of the main blade 
 *
 * Discovers blade expansion resources and their RDRs, if any.
 *
 * Return values:
 * SA_OK - normal case.
 * SA_ERR_HPI_OUT_OF_MEMORY - Cannot allocate space for internal memory.
 * SA_ERR_HPI_INVALID_PARAMS - Pointer parameter(s) NULL.
 **/
SaErrorT snmp_bc_add_blade_expansion_resource(struct oh_handler_state *handle,
			  				SaHpiEntityPathT *ep, 
							guint blade_index,
							BCExpansionTypeT expansionType,
							guint expansionindex)
{

	SaErrorT err;
	struct oh_event *e;
	struct ResourceInfo *res_info_ptr;
	struct snmp_bc_hnd *custom_handle;


	if (!handle) {
		err("Invalid parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}
		
	custom_handle = (struct snmp_bc_hnd *)handle->data;
	if (!custom_handle) {
		err("Invalid parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}
	
	{
		e = snmp_bc_alloc_oh_event();
		if (e == NULL) {
			err("Out of memory.");
			return(SA_ERR_HPI_OUT_OF_MEMORY);
		}

									
		/* ---------------------------------------- */
		/* Construct .resource of struct oh_event   */
		/* ---------------------------------------- */	
		e->resource = snmp_bc_rpt_array[BC_RPT_ENTRY_BLADE_EXPANSION_CARD].rpt;
		e->resource.ResourceEntity = *ep;
		e->resource.ResourceId = oh_uid_from_entity_path(ep);
			
		{
		SaHpiTextBufferT  working, working2;
		snmp_bc_create_resourcetag(&working, "Blade", blade_index + SNMP_BC_HPI_LOCATION_BASE);
		snmp_bc_create_resourcetag(&working2,
				 	bladeexpansiondesc[expansionType],
				 	SNMP_BC_HPI_LOCATION_BASE + expansionindex);
		oh_init_textbuffer(&(e->resource.ResourceTag));
		oh_append_textbuffer(&(e->resource.ResourceTag), (char *)working.Data);
		oh_append_textbuffer(&(e->resource.ResourceTag), " ");
		oh_append_textbuffer(&(e->resource.ResourceTag), (char *)working2.Data);
		}

		dbg("Discovered resource=%s; ID=%d",
		    e->resource.ResourceTag.Data,
		    e->resource.ResourceId);

		/* Create platform-specific info space to add to infra-structure */
		res_info_ptr = g_memdup(&(snmp_bc_rpt_array[BC_RPT_ENTRY_BLADE_EXPANSION_CARD].res_info),
						sizeof(struct ResourceInfo));
		if (!res_info_ptr) {
			err("Out of memory.");
			snmp_bc_free_oh_event(e);
			return(SA_ERR_HPI_OUT_OF_MEMORY);
		}

		res_info_ptr->cur_state = SAHPI_HS_STATE_ACTIVE;

                /* Get UUID and convert to GUID */
                err = snmp_bc_get_guid(custom_handle, e, res_info_ptr);

		/* Add resource to resource cache */
		err = oh_add_resource(handle->rptcache, 
					&(e->resource),
					res_info_ptr, 0);
		if (err) {
			err("Failed to add resource. Error=%s.", oh_lookup_error(err));
			snmp_bc_free_oh_event(e);
			return(err);
		}
			
		/* ---------------------------------------- */
		/* Construct .rdrs of struct oh_event       */
		/* ---------------------------------------- */								
		/* Find resource's events, sensors, controls, etc. */
		snmp_bc_discover_res_events(handle, &(e->resource.ResourceEntity), res_info_ptr);
		snmp_bc_discover_sensors(handle, snmp_bc_bem_sensors, e);
		snmp_bc_discover_ipmi_sensors(handle, snmp_bc_bem_ipmi_sensors, e);
		snmp_bc_discover_controls(handle, snmp_bc_bem_controls, e);
		snmp_bc_discover_inventories(handle, snmp_bc_bem_inventories, e);
			
		/* ---------------------------------------- */
		/* Construct .event of struct oh_event      */	
		/* ---------------------------------------- */
		snmp_bc_set_resource_add_oh_event(e, res_info_ptr);

		/* ---------------------------------------- */
		/* Place the event in queue                 */
		/* ---------------------------------------- */					
		/*custom_handle->eventq = g_slist_append(custom_handle->eventq, e);*/
                e->hid = handle->hid;
                oh_evt_queue_push(handle->eventq, e);
		
	}
			
	return(SA_OK);

}
						 												
/**
 * snmp_bc_discover_blowers:
 * @handler: Pointer to handler's data.
 * @ep_root: Pointer to chassis Root Entity Path which comes from openhpi.conf.
 * @blower_vector: Bitmap vector of installed blowers.
 *
 * Discovers blower resources and their RDRs.
 *
 * Return values:
 * SA_OK - normal case.
 * SA_ERR_HPI_OUT_OF_MEMORY - Cannot allocate space for internal memory.
 * SA_ERR_HPI_INVALID_PARAMS - Pointer paramter(s) NULL.
 **/
SaErrorT snmp_bc_discover_blowers(struct oh_handler_state *handle,
			  SaHpiEntityPathT *ep_root, char *blower_vector)
{

	guint i;
	SaErrorT err;
        struct oh_event *e;
	struct ResourceInfo *res_info_ptr;
	struct snmp_bc_hnd *custom_handle;


	if (!handle || !blower_vector) {
		err("Invalid parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}
		
	custom_handle = (struct snmp_bc_hnd *)handle->data;
	if (!custom_handle) {
		err("Invalid parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}
	
	e= NULL;
	res_info_ptr = NULL;
	
	for (i=0; i < strlen(blower_vector); i++) {
	
		if ((blower_vector[i] == '1') || (custom_handle->isFirstDiscovery == SAHPI_TRUE))
		{
			e = snmp_bc_alloc_oh_event();
			if (e == NULL) {
				err("Out of memory.");
				return(SA_ERR_HPI_OUT_OF_MEMORY);
			}
									
			/* ---------------------------------------- */
			/* Construct .resource of struct oh_event   */
			/* ---------------------------------------- */	
			err = snmp_bc_construct_blower_rpt(e, &res_info_ptr, ep_root, i);
			if (err) {
				snmp_bc_free_oh_event(e);
				return(err);
			}
		}
		
		if ((blower_vector[i] == '0') && (custom_handle->isFirstDiscovery == SAHPI_TRUE))
		{
			res_info_ptr->cur_state = SAHPI_HS_STATE_NOT_PRESENT;
			snmp_bc_discover_res_events(handle, &(e->resource.ResourceEntity), res_info_ptr);
			snmp_bc_free_oh_event(e);
			g_free(res_info_ptr);
			
		} else if (blower_vector[i] == '1') {

			err = snmp_bc_add_blower_rptcache(handle, e, res_info_ptr, i); 
			
			if (err == SA_OK) {			
				/* ---------------------------------------- */
				/* Construct .event of struct oh_event      */	
				/* ---------------------------------------- */
				snmp_bc_set_resource_add_oh_event(e, res_info_ptr);

				/* ---------------------------------------- */
				/* Place the event in tmpqueue              */
				/* ---------------------------------------- */					
				/*custom_handle->eventq = g_slist_append(custom_handle->eventq, e);*/
                                if (e) e->hid = handle->hid;
                                oh_evt_queue_push(handle->eventq, e);
			} else {
				snmp_bc_free_oh_event(e);
			}
		}
	}
	return(SA_OK);
}

/**
 * snmp_bc_discover_tap:
 * @handler: Pointer to handler's data.
 * @ep_root: Pointer to chassis Root Entity Path which comes from openhpi.conf.
 * @blower_vector: Bitmap vector of installed Telco Alarm Panel.
 *
 * Discovers Telco Alarm Panel resources and their RDRs.
 *
 * Return values:
 * SA_OK - normal case.
 * SA_ERR_HPI_OUT_OF_MEMORY - Cannot allocate space for internal memory.
 * SA_ERR_HPI_INVALID_PARAMS - Pointer paramter(s) NULL.
 **/
SaErrorT snmp_bc_discover_tap(struct oh_handler_state *handle,
			  SaHpiEntityPathT *ep_root, char *tap_vector)
{

	guint i;
	SaErrorT err;
        struct oh_event *e;
	struct ResourceInfo *res_info_ptr;
	struct snmp_bc_hnd *custom_handle;


	if (!handle || !tap_vector) {
		err("Invalid parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}
		
	custom_handle = (struct snmp_bc_hnd *)handle->data;
	if (!custom_handle) {
		err("Invalid parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}
	
	e= NULL;
	res_info_ptr = NULL;
	
	for (i=0; i < strlen(tap_vector); i++) {
	
		if ((tap_vector[i] == '1') || (custom_handle->isFirstDiscovery == SAHPI_TRUE))
		{
			e = snmp_bc_alloc_oh_event();
			if (e == NULL) {
				err("Out of memory.");
				return(SA_ERR_HPI_OUT_OF_MEMORY);
			}
									
			/* ---------------------------------------- */
			/* Construct .resource of struct oh_event   */
			/* ---------------------------------------- */	
			err = snmp_bc_construct_tap_rpt(e, &res_info_ptr, ep_root, i);
			if (err) {
				snmp_bc_free_oh_event(e);
				return(err);
			}
		}
		
		if ((tap_vector[i] == '0') && (custom_handle->isFirstDiscovery == SAHPI_TRUE))
		{
			res_info_ptr->cur_state = SAHPI_HS_STATE_NOT_PRESENT;
			snmp_bc_discover_res_events(handle, &(e->resource.ResourceEntity), res_info_ptr);
			snmp_bc_free_oh_event(e);
			g_free(res_info_ptr);
			
		} else if (tap_vector[i] == '1') {

			err = snmp_bc_add_tap_rptcache(handle, e, res_info_ptr, i); 
			
			if (err == SA_OK) {			
				/* ---------------------------------------- */
				/* Construct .event of struct oh_event      */	
				/* ---------------------------------------- */
				snmp_bc_set_resource_add_oh_event(e, res_info_ptr);

				/* ---------------------------------------- */
				/* Place the event in tmpqueue              */
				/* ---------------------------------------- */					
				/*custom_handle->eventq = g_slist_append(custom_handle->eventq, e);*/
                                if (e) e->hid = handle->hid;
                                oh_evt_queue_push(handle->eventq, e);
			} else {
				snmp_bc_free_oh_event(e);
			}
		}
	}
	return(SA_OK);
}

/**
 * snmp_bc_construct_tap_rpt:
 * @e: Pointer to oh_event struct.
 * @res_info_ptr: Pointer to pointer of res_info_ptr
 * @ep_root: Pointer to chassis Root Entity Path which comes from openhpi.conf.
 * @tap_index: Index of discovered tap.
 *
 * Build rpt structure for a blade resource using model data 
 *
 * Return values:
 * SA_OK - normal case.
 * SA_ERR_HPI_OUT_OF_MEMORY - Cannot allocate space for internal memory.
 * SA_ERR_HPI_INVALID_PARAMS - Pointer parameter(s) NULL.
 **/
SaErrorT snmp_bc_construct_tap_rpt(struct oh_event* e, 
				      struct ResourceInfo **res_info_ptr,
				      SaHpiEntityPathT *ep_root, 
				      guint tap_index)
{

	if (!e || !res_info_ptr) return (SA_ERR_HPI_INVALID_PARAMS);
	
	e->resource = snmp_bc_rpt_array[BC_RPT_ENTRY_ALARM_PANEL].rpt;
	oh_concat_ep(&(e->resource.ResourceEntity), ep_root);
	oh_set_ep_location(&(e->resource.ResourceEntity),
			   BLADECENTER_ALARM_PANEL_SLOT, tap_index + SNMP_BC_HPI_LOCATION_BASE);
	oh_set_ep_location(&(e->resource.ResourceEntity),
			   SAHPI_ENT_DISPLAY_PANEL, tap_index + SNMP_BC_HPI_LOCATION_BASE);
	e->resource.ResourceId = 
		oh_uid_from_entity_path(&(e->resource.ResourceEntity));
	snmp_bc_create_resourcetag(&(e->resource.ResourceTag),
				   snmp_bc_rpt_array[BC_RPT_ENTRY_ALARM_PANEL].comment,
				   tap_index + SNMP_BC_HPI_LOCATION_BASE);

	dbg("Discovered resource=%s; ID=%d",
	    e->resource.ResourceTag.Data,
	    e->resource.ResourceId);

	/* Create platform-specific info space to add to infra-structure */
	*res_info_ptr = g_memdup(&(snmp_bc_rpt_array[BC_RPT_ENTRY_ALARM_PANEL].res_info),
						sizeof(struct ResourceInfo));
	if (!(*res_info_ptr)) {
		err("Out of memory.");
		return(SA_ERR_HPI_OUT_OF_MEMORY);
	}
	
	return(SA_OK);

}

/**
 * snmp_bc_add_tap_rptcache:
 * @handle: Pointer to hpi handle
 * @e: Pointer to oh_event struct.
 * @res_info_ptr: Pointer to pointer of res_info_ptr
 * @tap_index: Index of discovered tap.
 *
 * Build rpt and rdrs for a tap (Telco Alarm Panel) then add to rptcache 
 *
 * Return values:
 * SA_OK - normal case.
 * SA_ERR_HPI_OUT_OF_MEMORY - Cannot allocate space for internal memory.
 * SA_ERR_HPI_INVALID_PARAMS - Pointer parameter(s) NULL.
 **/
SaErrorT snmp_bc_add_tap_rptcache(struct oh_handler_state *handle, 
				  struct oh_event *e, 
				  struct ResourceInfo *res_info_ptr,
				  guint tap_index) 

{
	SaErrorT err;
	guint tap_width;
	struct snmp_value get_value;
	struct snmp_bc_hnd *custom_handle;


	if (!handle || !e || !res_info_ptr) {
		err("Invalid parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}
		
	custom_handle = (struct snmp_bc_hnd *)handle->data;
	if (!custom_handle) {
		err("Invalid parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}

	dbg("Discovering Telco Alarm Panel %d resource.\n", tap_index);
	res_info_ptr->cur_state = SAHPI_HS_STATE_ACTIVE;

        /* Get UUID and convert to GUID */
        err = snmp_bc_get_guid(custom_handle, e, res_info_ptr);

	/* Add resource to temporary event cache/queue */
	err = oh_add_resource(handle->rptcache, 
				      &(e->resource),
				      res_info_ptr, 0);
	if (err) {
		err("Failed to add resource. Error=%s.", oh_lookup_error(err));
		return(err);
	}
			
	/* ---------------------------------------- */
	/* Construct .rdrs of struct oh_event       */
	/* ---------------------------------------- */								
	/* Find resource's events, sensors, controls, etc. */
	snmp_bc_discover_res_events(handle, &(e->resource.ResourceEntity), res_info_ptr);
	snmp_bc_discover_sensors(handle, snmp_bc_alarm_sensors, e);
	snmp_bc_discover_controls(handle, snmp_bc_alarm_controls, e);
	snmp_bc_discover_inventories(handle, snmp_bc_alarm_inventories, e);
		
	tap_width = 1;    /* Default to 1-wide blade */
	if (res_info_ptr->mib.OidResourceWidth != NULL) {
		err = snmp_bc_oid_snmp_get(custom_handle,  &(e->resource.ResourceEntity), 0,
		   		res_info_ptr->mib.OidResourceWidth, &get_value, SAHPI_TRUE);
		if (!err && (get_value.type == ASN_INTEGER)) {
			tap_width = get_value.integer;
		}
	}			
	res_info_ptr->resourcewidth = tap_width;
	err = snmp_bc_set_resource_slot_state_sensor(handle, e, tap_width);
	return(err);
}

/**
 * snmp_bc_discover_tap_i:
 * @handle: Pointer to hpi handle
 * @ep_root: Pointer to .
 * @tap_index: Index of discovered tap.
 *
 * Discover a particular Telco Alarm Card at index tap_index.
 * This routine is used to rediscover a Telco Alarm Panel (tap). 
 * Blower rpt and rdrs will be added to rptcache.
 * No event will be generated.  The event is handled separately in log2event.
 *
 * Return values:
 * SA_OK - normal case.
 * SA_ERR_HPI_OUT_OF_MEMORY - Cannot allocate space for internal memory.
 * SA_ERR_HPI_INVALID_PARAMS - Pointer parameter(s) NULL.
 **/
SaErrorT snmp_bc_discover_tap_i(struct oh_handler_state *handle,
			  	   SaHpiEntityPathT *ep_root, 
				   guint tap_index)
{

	SaErrorT err;
        struct oh_event *e;
	struct ResourceInfo *res_info_ptr;

	if (!handle) {
		err("Invalid parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}
		
	e= NULL;
	res_info_ptr = NULL;
	
	e = snmp_bc_alloc_oh_event();
	if (e == NULL) {
		err("Out of memory.");
		return(SA_ERR_HPI_OUT_OF_MEMORY);
	}
									
	/* ---------------------------------------- */
	/* Construct .resource of struct oh_event   */
	/* ---------------------------------------- */	
	err = snmp_bc_construct_tap_rpt(e, &res_info_ptr, ep_root, tap_index);
	if (err) {
		snmp_bc_free_oh_event(e);
		return(err);
	}
	
	/* ---------------------------------------- */
	/* Discover rdrs.                           */
	/* Add rpt and rdrs to rptcache.            */
	/* ---------------------------------------- */				
	err = snmp_bc_add_tap_rptcache(handle, e, res_info_ptr, tap_index); 
			
	snmp_bc_free_oh_event(e);
			
	return(SA_OK);
}

/**
 * snmp_bc_discover_smi:
 * @handler: Pointer to handler's data.
 * @ep_root: Pointer to chassis Root Entity Path which comes from openhpi.conf.
 * @smi_vector: Bitmap vector of installed Switch Module Interposers (smi).
 *
 * Discovers Switch Module Interposers resources and their RDRs.
 *
 * Return values:
 * SA_OK - normal case.
 * SA_ERR_HPI_OUT_OF_MEMORY - Cannot allocate space for internal memory.
 * SA_ERR_HPI_INVALID_PARAMS - Pointer paramter(s) NULL.
 **/
SaErrorT snmp_bc_discover_smi(struct oh_handler_state *handle,
			      SaHpiEntityPathT *ep_root, char *smi_vector)
{

	guint i;
	SaErrorT err;
        struct oh_event *e;
	struct ResourceInfo *res_info_ptr;
	struct snmp_bc_hnd *custom_handle;


	if (!handle || !smi_vector) {
		err("Invalid parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}
		
	custom_handle = (struct snmp_bc_hnd *)handle->data;
	if (!custom_handle) {
		err("Invalid parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}
	
	e= NULL;
	res_info_ptr = NULL;
	
	for (i=0; i < strlen(smi_vector); i++) {
	
		if ((smi_vector[i] == '1') || (custom_handle->isFirstDiscovery == SAHPI_TRUE))
		{
			e = snmp_bc_alloc_oh_event();
			if (e == NULL) {
				err("Out of memory.");
				return(SA_ERR_HPI_OUT_OF_MEMORY);
			}
									
			/* ---------------------------------------- */
			/* Construct .resource of struct oh_event   */
			/* ---------------------------------------- */	
			err = snmp_bc_construct_smi_rpt(e, &res_info_ptr, ep_root, i);
			if (err) {
				snmp_bc_free_oh_event(e);
				return(err);
			}
		}
		
		if ((smi_vector[i] == '0') && (custom_handle->isFirstDiscovery == SAHPI_TRUE))
		{
			res_info_ptr->cur_state = SAHPI_HS_STATE_NOT_PRESENT;
			snmp_bc_discover_res_events(handle, &(e->resource.ResourceEntity), res_info_ptr);
			snmp_bc_free_oh_event(e);
			g_free(res_info_ptr);
			
		} else if (smi_vector[i] == '1') {

			err = snmp_bc_add_smi_rptcache(handle, e, res_info_ptr, i); 
			
			if (err == SA_OK) {			
				/* ---------------------------------------- */
				/* Construct .event of struct oh_event      */	
				/* ---------------------------------------- */
				snmp_bc_set_resource_add_oh_event(e, res_info_ptr);

				/* ---------------------------------------- */
				/* Place the event in tmpqueue              */
				/* ---------------------------------------- */					
				/*custom_handle->eventq = g_slist_append(custom_handle->eventq, e);*/
                                if (e) e->hid = handle->hid;
                                oh_evt_queue_push(handle->eventq, e);
			} else {
				snmp_bc_free_oh_event(e);
			}
		}
	}
	return(SA_OK);
}

/**
 * snmp_bc_construct_smi_rpt:
 * @e: Pointer to oh_event struct.
 * @res_info_ptr: Pointer to pointer of res_info_ptr
 * @ep_root: Pointer to chassis Root Entity Path which comes from openhpi.conf.
 * @smi_index: Index of discovered smi.
 *
 * Build rpt structure for a blade resource using model data 
 *
 * Return values:
 * SA_OK - normal case.
 * SA_ERR_HPI_OUT_OF_MEMORY - Cannot allocate space for internal memory.
 * SA_ERR_HPI_INVALID_PARAMS - Pointer parameter(s) NULL.
 **/
SaErrorT snmp_bc_construct_smi_rpt(struct oh_event* e, 
				      struct ResourceInfo **res_info_ptr,
				      SaHpiEntityPathT *ep_root, 
				      guint smi_index)
{

	if (!e || !res_info_ptr) return (SA_ERR_HPI_INVALID_PARAMS);
	
	e->resource = snmp_bc_rpt_array[BC_RPT_ENTRY_INTERPOSER_SWITCH].rpt;
	oh_concat_ep(&(e->resource.ResourceEntity), ep_root);
	oh_set_ep_location(&(e->resource.ResourceEntity),
			   BLADECENTER_SWITCH_SLOT, smi_index + SNMP_BC_HPI_LOCATION_BASE);
	oh_set_ep_location(&(e->resource.ResourceEntity),
			   SAHPI_ENT_INTERCONNECT, smi_index + SNMP_BC_HPI_LOCATION_BASE);
	e->resource.ResourceId = 
		oh_uid_from_entity_path(&(e->resource.ResourceEntity));
	snmp_bc_create_resourcetag(&(e->resource.ResourceTag),
				   snmp_bc_rpt_array[BC_RPT_ENTRY_INTERPOSER_SWITCH].comment,
				   smi_index + SNMP_BC_HPI_LOCATION_BASE);

	dbg("Discovered resource=%s; ID=%d",
	    e->resource.ResourceTag.Data,
	    e->resource.ResourceId);

	/* Create platform-specific info space to add to infra-structure */
	*res_info_ptr = g_memdup(&(snmp_bc_rpt_array[BC_RPT_ENTRY_INTERPOSER_SWITCH].res_info),
						sizeof(struct ResourceInfo));
	if (!(*res_info_ptr)) {
		err("Out of memory.");
		return(SA_ERR_HPI_OUT_OF_MEMORY);
	}
	
	return(SA_OK);

}

/**
 * snmp_bc_add_smi_rptcache:
 * @handle: Pointer to hpi handle
 * @e: Pointer to oh_event struct.
 * @res_info_ptr: Pointer to pointer of res_info_ptr
 * @smi_index: Index of discovered smi.
 *
 * Build rpt and rdrs for a smi (Switch Module interposer) then add to rptcache 
 *
 * Return values:
 * SA_OK - normal case.
 * SA_ERR_HPI_OUT_OF_MEMORY - Cannot allocate space for internal memory.
 * SA_ERR_HPI_INVALID_PARAMS - Pointer parameter(s) NULL.
 **/
SaErrorT snmp_bc_add_smi_rptcache(struct oh_handler_state *handle, 
				  struct oh_event *e, 
				  struct ResourceInfo *res_info_ptr,
				  guint smi_index) 

{
	SaErrorT err;
	struct snmp_bc_hnd *custom_handle;


	if (!handle || !e || !res_info_ptr) {
		err("Invalid parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}
		
	custom_handle = (struct snmp_bc_hnd *)handle->data;
	if (!custom_handle) {
		err("Invalid parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}

	dbg("Discovering Switch Module Interposer %d resource.\n", smi_index);
	res_info_ptr->cur_state = SAHPI_HS_STATE_ACTIVE;

        /* Get UUID and convert to GUID */
        err = snmp_bc_get_guid(custom_handle, e, res_info_ptr);

	/* Add resource to temporary event cache/queue */
	err = oh_add_resource(handle->rptcache, 
				      &(e->resource),
				      res_info_ptr, 0);
	if (err) {
		err("Failed to add resource. Error=%s.", oh_lookup_error(err));
		return(err);
	}
			
	/* ---------------------------------------- */
	/* Construct .rdrs of struct oh_event       */
	/* ---------------------------------------- */								
	/* Find resource's events, sensors, controls, etc. */
	snmp_bc_discover_res_events(handle, &(e->resource.ResourceEntity), res_info_ptr);
	//snmp_bc_discover_sensors(handle, snmp_bc_alarm_sensors, e);
	//snmp_bc_discover_controls(handle, snmp_bc_alarm_controls, e);
	snmp_bc_discover_inventories(handle, snmp_bc_interposer_switch_inventories, e);

	return(err);
}

/**
 * snmp_bc_discover_smi_i:
 * @handle: Pointer to hpi handle
 * @ep_root: Pointer to .
 * @smi_index: Index of discovered smi.
 *
 * Discover a Switch Module Interposer card at index mmi_index.
 * This routine is used to rediscover a Network Clock card (nc). 
 * Blower rpt and rdrs will be added to rptcache.
 * No event will be generated.  The event is handled separately in log2event.
 *
 * Return values:
 * SA_OK - normal case.
 * SA_ERR_HPI_OUT_OF_MEMORY - Cannot allocate space for internal memory.
 * SA_ERR_HPI_INVALID_PARAMS - Pointer parameter(s) NULL.
 **/
SaErrorT snmp_bc_discover_smi_i(struct oh_handler_state *handle,
			  	   SaHpiEntityPathT *ep_root, 
				   guint smi_index)
{

	SaErrorT err;
        struct oh_event *e;
	struct ResourceInfo *res_info_ptr;

	if (!handle) {
		err("Invalid parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}
		
	e= NULL;
	res_info_ptr = NULL;
	
	e = snmp_bc_alloc_oh_event();
	if (e == NULL) {
		err("Out of memory.");
		return(SA_ERR_HPI_OUT_OF_MEMORY);
	}
									
	/* ---------------------------------------- */
	/* Construct .resource of struct oh_event   */
	/* ---------------------------------------- */	
	err = snmp_bc_construct_smi_rpt(e, &res_info_ptr, ep_root, smi_index);
	if (err) {
		snmp_bc_free_oh_event(e);
		return(err);
	}
	
	/* ---------------------------------------- */
	/* Discover rdrs.                           */
	/* Add rpt and rdrs to rptcache.            */
	/* ---------------------------------------- */				
	err = snmp_bc_add_smi_rptcache(handle, e, res_info_ptr, smi_index); 
			
	snmp_bc_free_oh_event(e);
			
	return(SA_OK);
}

/**
 * snmp_bc_discover_mmi:
 * @handler: Pointer to handler's data.
 * @ep_root: Pointer to chassis Root Entity Path which comes from openhpi.conf.
 * @mmi_vector: Bitmap vector of installed Management Module Interposers (mmi).
 *
 * Discovers Management Module Interposers resources and their RDRs.
 *
 * Return values:
 * SA_OK - normal case.
 * SA_ERR_HPI_OUT_OF_MEMORY - Cannot allocate space for internal memory.
 * SA_ERR_HPI_INVALID_PARAMS - Pointer paramter(s) NULL.
 **/
SaErrorT snmp_bc_discover_mmi(struct oh_handler_state *handle,
			      SaHpiEntityPathT *ep_root, 
			      char *mmi_vector)
{

	guint i;
	SaErrorT err;
        struct oh_event *e;
	struct ResourceInfo *res_info_ptr;
	struct snmp_bc_hnd *custom_handle;


	if (!handle || !mmi_vector) {
		err("Invalid parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}
		
	custom_handle = (struct snmp_bc_hnd *)handle->data;
	if (!custom_handle) {
		err("Invalid parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}
	
	e= NULL;
	res_info_ptr = NULL;
	
	for (i=0; i < strlen(mmi_vector); i++) {
	
		if ((mmi_vector[i] == '1') || (custom_handle->isFirstDiscovery == SAHPI_TRUE))
		{
			e = snmp_bc_alloc_oh_event();
			if (e == NULL) {
				err("Out of memory.");
				return(SA_ERR_HPI_OUT_OF_MEMORY);
			}
									
			/* ---------------------------------------- */
			/* Construct .resource of struct oh_event   */
			/* ---------------------------------------- */	
			err = snmp_bc_construct_mmi_rpt(e, &res_info_ptr, ep_root, i);
			if (err) {
				snmp_bc_free_oh_event(e);
				return(err);
			}
		}
		
		if ((mmi_vector[i] == '0') && (custom_handle->isFirstDiscovery == SAHPI_TRUE))
		{
			res_info_ptr->cur_state = SAHPI_HS_STATE_NOT_PRESENT;
			snmp_bc_discover_res_events(handle, &(e->resource.ResourceEntity), res_info_ptr);
			snmp_bc_free_oh_event(e);
			g_free(res_info_ptr);
			
		} else if (mmi_vector[i] == '1') {

			err = snmp_bc_add_mmi_rptcache(handle, e, res_info_ptr, i); 
			
			if (err == SA_OK) {			
				/* ---------------------------------------- */
				/* Construct .event of struct oh_event      */	
				/* ---------------------------------------- */
				snmp_bc_set_resource_add_oh_event(e, res_info_ptr);

				/* ---------------------------------------- */
				/* Place the event in tmpqueue              */
				/* ---------------------------------------- */					
				/*custom_handle->eventq = g_slist_append(custom_handle->eventq, e);*/
                                if (e) e->hid = handle->hid;
                                oh_evt_queue_push(handle->eventq, e);
			} else {
				snmp_bc_free_oh_event(e);
			}
		}
	}
	return(SA_OK);
}

/**
 * snmp_bc_construct_mmi_rpt:
 * @e: Pointer to oh_event struct.
 * @res_info_ptr: Pointer to pointer of res_info_ptr
 * @ep_root: Pointer to chassis Root Entity Path which comes from openhpi.conf.
 * @mmi_index: Index of discovered mmi.
 *
 * Build rpt structure for a blade resource using model data 
 *
 * Return values:
 * SA_OK - normal case.
 * SA_ERR_HPI_OUT_OF_MEMORY - Cannot allocate space for internal memory.
 * SA_ERR_HPI_INVALID_PARAMS - Pointer parameter(s) NULL.
 **/
SaErrorT snmp_bc_construct_mmi_rpt(struct oh_event* e, 
				      struct ResourceInfo **res_info_ptr,
				      SaHpiEntityPathT *ep_root, 
				      guint mmi_index)
{

	if (!e || !res_info_ptr) return (SA_ERR_HPI_INVALID_PARAMS);
	
	e->resource = snmp_bc_rpt_array[BC_RPT_ENTRY_INTERPOSER_MM].rpt;
	oh_concat_ep(&(e->resource.ResourceEntity), ep_root);
	oh_set_ep_location(&(e->resource.ResourceEntity),
			   BLADECENTER_SYS_MGMNT_MODULE_SLOT, mmi_index + SNMP_BC_HPI_LOCATION_BASE);
	oh_set_ep_location(&(e->resource.ResourceEntity),
			   SAHPI_ENT_INTERCONNECT, mmi_index + SNMP_BC_HPI_LOCATION_BASE);
	e->resource.ResourceId = 
		oh_uid_from_entity_path(&(e->resource.ResourceEntity));
	snmp_bc_create_resourcetag(&(e->resource.ResourceTag),
				   snmp_bc_rpt_array[BC_RPT_ENTRY_INTERPOSER_MM].comment,
				   mmi_index + SNMP_BC_HPI_LOCATION_BASE);

	dbg("Discovered resource=%s; ID=%d",
	    e->resource.ResourceTag.Data,
	    e->resource.ResourceId);

	/* Create platform-specific info space to add to infra-structure */
	*res_info_ptr = g_memdup(&(snmp_bc_rpt_array[BC_RPT_ENTRY_INTERPOSER_MM].res_info),
						sizeof(struct ResourceInfo));
	if (!(*res_info_ptr)) {
		err("Out of memory.");
		return(SA_ERR_HPI_OUT_OF_MEMORY);
	}
	
	return(SA_OK);

}

/**
 * snmp_bc_add_mmi_rptcache:
 * @handle: Pointer to hpi handle
 * @e: Pointer to oh_event struct.
 * @res_info_ptr: Pointer to pointer of res_info_ptr
 * @mmi_index: Index of discovered mmi.
 *
 * Build rpt and rdrs for a mmi (Management Module interposer) then add to rptcache 
 *
 * Return values:
 * SA_OK - normal case.
 * SA_ERR_HPI_OUT_OF_MEMORY - Cannot allocate space for internal memory.
 * SA_ERR_HPI_INVALID_PARAMS - Pointer parameter(s) NULL.
 **/
SaErrorT snmp_bc_add_mmi_rptcache(struct oh_handler_state *handle, 
				  struct oh_event *e, 
				  struct ResourceInfo *res_info_ptr,
				  guint mmi_index) 

{
	SaErrorT err;
	struct snmp_bc_hnd *custom_handle;


	if (!handle || !e || !res_info_ptr) {
		err("Invalid parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}
		
	custom_handle = (struct snmp_bc_hnd *)handle->data;
	if (!custom_handle) {
		err("Invalid parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}

	dbg("Discovering Management Module Interposer %d resource.\n", mmi_index);
	res_info_ptr->cur_state = SAHPI_HS_STATE_ACTIVE;

        /* Get UUID and convert to GUID */
        err = snmp_bc_get_guid(custom_handle, e, res_info_ptr);

	/* Add resource to temporary event cache/queue */
	err = oh_add_resource(handle->rptcache, 
				      &(e->resource),
				      res_info_ptr, 0);
	if (err) {
		err("Failed to add resource. Error=%s.", oh_lookup_error(err));
		return(err);
	}
			
	/* ---------------------------------------- */
	/* Construct .rdrs of struct oh_event       */
	/* ---------------------------------------- */								
	/* Find resource's events, sensors, controls, etc. */
	snmp_bc_discover_res_events(handle, &(e->resource.ResourceEntity), res_info_ptr);
	//snmp_bc_discover_sensors(handle, snmp_bc_alarm_sensors, e);
	//snmp_bc_discover_controls(handle, snmp_bc_alarm_controls, e);
	snmp_bc_discover_inventories(handle, snmp_bc_interposer_mm_inventories, e);

	return(err);
}

/**
 * snmp_bc_discover_mmi_i:
 * @handle: Pointer to hpi handle
 * @ep_root: Pointer to .
 * @mmi_index: Index of discovered mmi.
 *
 * Discover a Management Module Interposer card at index mmi_index.
 * This routine is used to rediscover a Network Clock card (nc). 
 * Blower rpt and rdrs will be added to rptcache.
 * No event will be generated.  The event is handled separately in log2event.
 *
 * Return values:
 * SA_OK - normal case.
 * SA_ERR_HPI_OUT_OF_MEMORY - Cannot allocate space for internal memory.
 * SA_ERR_HPI_INVALID_PARAMS - Pointer parameter(s) NULL.
 **/
SaErrorT snmp_bc_discover_mmi_i(struct oh_handler_state *handle,
			  	   SaHpiEntityPathT *ep_root, 
				   guint mmi_index)
{

	SaErrorT err;
        struct oh_event *e;
	struct ResourceInfo *res_info_ptr;
	

	if (!handle) {
		err("Invalid parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}
		
	e= NULL;
	res_info_ptr = NULL;
	
	e = snmp_bc_alloc_oh_event();
	if (e == NULL) {
		err("Out of memory.");
		return(SA_ERR_HPI_OUT_OF_MEMORY);
	}
									
	/* ---------------------------------------- */
	/* Construct .resource of struct oh_event   */
	/* ---------------------------------------- */	
	err = snmp_bc_construct_mmi_rpt(e, &res_info_ptr, ep_root, mmi_index);
	if (err) {
		snmp_bc_free_oh_event(e);
		return(err);
	}
	
	/* ---------------------------------------- */
	/* Discover rdrs.                           */
	/* Add rpt and rdrs to rptcache.            */
	/* ---------------------------------------- */				
	err = snmp_bc_add_mmi_rptcache(handle, e, res_info_ptr, mmi_index); 
			
	snmp_bc_free_oh_event(e);
			
	return(SA_OK);
}

/**
 * snmp_bc_discover_nc:
 * @handler: Pointer to handler's data.
 * @ep_root: Pointer to chassis Root Entity Path which comes from openhpi.conf.
 * @nc_vector: Bitmap vector of installed Network Clock (nc) cards.
 *
 * Discovers Network Clock Card resources and their RDRs.
 *
 * Return values:
 * SA_OK - normal case.
 * SA_ERR_HPI_OUT_OF_MEMORY - Cannot allocate space for internal memory.
 * SA_ERR_HPI_INVALID_PARAMS - Pointer paramter(s) NULL.
 **/
SaErrorT snmp_bc_discover_nc(struct oh_handler_state *handle,
			  SaHpiEntityPathT *ep_root, char *nc_vector)
{

	guint i;
	SaErrorT err;
        struct oh_event *e;
	struct ResourceInfo *res_info_ptr;
	struct snmp_bc_hnd *custom_handle;


	if (!handle || !nc_vector) {
		err("Invalid parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}
		
	custom_handle = (struct snmp_bc_hnd *)handle->data;
	if (!custom_handle) {
		err("Invalid parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}
	
	e= NULL;
	res_info_ptr = NULL;
	
	for (i=0; i < strlen(nc_vector); i++) {
	
		if ((nc_vector[i] == '1') || (custom_handle->isFirstDiscovery == SAHPI_TRUE))
		{
			e = snmp_bc_alloc_oh_event();
			if (e == NULL) {
				err("Out of memory.");
				return(SA_ERR_HPI_OUT_OF_MEMORY);
			}
									
			/* ---------------------------------------- */
			/* Construct .resource of struct oh_event   */
			/* ---------------------------------------- */	
			err = snmp_bc_construct_nc_rpt(e, &res_info_ptr, ep_root, i);
			if (err) {
				snmp_bc_free_oh_event(e);
				return(err);
			}
		}
		
		if ((nc_vector[i] == '0') && (custom_handle->isFirstDiscovery == SAHPI_TRUE))
		{
			res_info_ptr->cur_state = SAHPI_HS_STATE_NOT_PRESENT;
			snmp_bc_discover_res_events(handle, &(e->resource.ResourceEntity), res_info_ptr);
			snmp_bc_free_oh_event(e);
			g_free(res_info_ptr);
			
		} else if (nc_vector[i] == '1') {

			err = snmp_bc_add_nc_rptcache(handle, e, res_info_ptr, i); 
			
			if (err == SA_OK) {			
				/* ---------------------------------------- */
				/* Construct .event of struct oh_event      */	
				/* ---------------------------------------- */
				snmp_bc_set_resource_add_oh_event(e, res_info_ptr);

				/* ---------------------------------------- */
				/* Place the event in tmpqueue              */
				/* ---------------------------------------- */					
				/*custom_handle->eventq = g_slist_append(custom_handle->eventq, e);*/
                                if (e) e->hid = handle->hid;
                                oh_evt_queue_push(handle->eventq, e);
			} else {
				snmp_bc_free_oh_event(e);
			}
		}
	}
	return(SA_OK);
}

/**
 * snmp_bc_construct_nc_rpt:
 * @e: Pointer to oh_event struct.
 * @res_info_ptr: Pointer to pointer of res_info_ptr
 * @ep_root: Pointer to chassis Root Entity Path which comes from openhpi.conf.
 * @nc_index: Index of discovered nc.
 *
 * Build rpt structure for a blade resource using model data 
 *
 * Return values:
 * SA_OK - normal case.
 * SA_ERR_HPI_OUT_OF_MEMORY - Cannot allocate space for internal memory.
 * SA_ERR_HPI_INVALID_PARAMS - Pointer parameter(s) NULL.
 **/
SaErrorT snmp_bc_construct_nc_rpt(struct oh_event* e, 
				      struct ResourceInfo **res_info_ptr,
				      SaHpiEntityPathT *ep_root, 
				      guint nc_index)
{

	if (!e || !res_info_ptr) return (SA_ERR_HPI_INVALID_PARAMS);
	
	e->resource = snmp_bc_rpt_array[BC_RPT_ENTRY_CLOCK_MODULE].rpt;
	oh_concat_ep(&(e->resource.ResourceEntity), ep_root);
	oh_set_ep_location(&(e->resource.ResourceEntity),
			   BLADECENTER_CLOCK_SLOT, nc_index + SNMP_BC_HPI_LOCATION_BASE);
	oh_set_ep_location(&(e->resource.ResourceEntity),
			   (SAHPI_ENT_BATTERY + 13), nc_index + SNMP_BC_HPI_LOCATION_BASE);
	e->resource.ResourceId = 
		oh_uid_from_entity_path(&(e->resource.ResourceEntity));
	snmp_bc_create_resourcetag(&(e->resource.ResourceTag),
				   snmp_bc_rpt_array[BC_RPT_ENTRY_CLOCK_MODULE].comment,
				   nc_index + SNMP_BC_HPI_LOCATION_BASE);

	dbg("Discovered resource=%s; ID=%d",
	    e->resource.ResourceTag.Data,
	    e->resource.ResourceId);

	/* Create platform-specific info space to add to infra-structure */
	*res_info_ptr = g_memdup(&(snmp_bc_rpt_array[BC_RPT_ENTRY_CLOCK_MODULE].res_info),
						sizeof(struct ResourceInfo));
	if (!(*res_info_ptr)) {
		err("Out of memory.");
		return(SA_ERR_HPI_OUT_OF_MEMORY);
	}
	
	return(SA_OK);

}

/**
 * snmp_bc_add_nc_rptcache:
 * @handle: Pointer to hpi handle
 * @e: Pointer to oh_event struct.
 * @res_info_ptr: Pointer to pointer of res_info_ptr
 * @nc_index: Index of discovered nc.
 *
 * Build rpt and rdrs for a nc (Network Clock Card) then add to rptcache 
 *
 * Return values:
 * SA_OK - normal case.
 * SA_ERR_HPI_OUT_OF_MEMORY - Cannot allocate space for internal memory.
 * SA_ERR_HPI_INVALID_PARAMS - Pointer parameter(s) NULL.
 **/
SaErrorT snmp_bc_add_nc_rptcache(struct oh_handler_state *handle, 
				  struct oh_event *e, 
				  struct ResourceInfo *res_info_ptr,
				  guint nc_index) 

{
	SaErrorT err;
	guint nc_width;
	struct snmp_value get_value;
	struct snmp_bc_hnd *custom_handle;


	if (!handle || !e || !res_info_ptr) {
		err("Invalid parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}
		
	custom_handle = (struct snmp_bc_hnd *)handle->data;
	if (!custom_handle) {
		err("Invalid parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}

	dbg("Discovering Network Clocd Card %d resource.\n", nc_index);
	res_info_ptr->cur_state = SAHPI_HS_STATE_ACTIVE;

        /* Get UUID and convert to GUID */
        err = snmp_bc_get_guid(custom_handle, e, res_info_ptr);

	/* Add resource to temporary event cache/queue */
	err = oh_add_resource(handle->rptcache, 
				      &(e->resource),
				      res_info_ptr, 0);
	if (err) {
		err("Failed to add resource. Error=%s.", oh_lookup_error(err));
		return(err);
	}
			
	/* ---------------------------------------- */
	/* Construct .rdrs of struct oh_event       */
	/* ---------------------------------------- */								
	/* Find resource's events, sensors, controls, etc. */
	snmp_bc_discover_res_events(handle, &(e->resource.ResourceEntity), res_info_ptr);
	snmp_bc_discover_sensors(handle, snmp_bc_clock_sensors, e);
	snmp_bc_discover_controls(handle, snmp_bc_clock_controls, e);
	snmp_bc_discover_inventories(handle, snmp_bc_clock_inventories, e);
		
	nc_width = 1;    /* Default to 1-wide blade */
	if (res_info_ptr->mib.OidResourceWidth != NULL) {
		err = snmp_bc_oid_snmp_get(custom_handle,  &(e->resource.ResourceEntity), 0,
		   		res_info_ptr->mib.OidResourceWidth, &get_value, SAHPI_TRUE);
		if (!err && (get_value.type == ASN_INTEGER)) {
			nc_width = get_value.integer;
		}
	}			
	res_info_ptr->resourcewidth = nc_width;
	err = snmp_bc_set_resource_slot_state_sensor(handle, e, nc_width);
	return(err);
}

/**
 * snmp_bc_discover_nc_i:
 * @handle: Pointer to hpi handle
 * @ep_root: Pointer to .
 * @nc_index: Index of discovered nc.
 *
 * Discover a Network Clock card at index nc_index.
 * This routine is used to rediscover a Network Clock card (nc). 
 * Blower rpt and rdrs will be added to rptcache.
 * No event will be generated.  The event is handled separately in log2event.
 *
 * Return values:
 * SA_OK - normal case.
 * SA_ERR_HPI_OUT_OF_MEMORY - Cannot allocate space for internal memory.
 * SA_ERR_HPI_INVALID_PARAMS - Pointer parameter(s) NULL.
 **/
SaErrorT snmp_bc_discover_nc_i(struct oh_handler_state *handle,
			  	   SaHpiEntityPathT *ep_root, 
				   guint nc_index)
{

	SaErrorT err;
        struct oh_event *e;
	struct ResourceInfo *res_info_ptr;

	if (!handle) {
		err("Invalid parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}
		
	e= NULL;
	res_info_ptr = NULL;
	
	e = snmp_bc_alloc_oh_event();
	if (e == NULL) {
		err("Out of memory.");
		return(SA_ERR_HPI_OUT_OF_MEMORY);
	}
									
	/* ---------------------------------------- */
	/* Construct .resource of struct oh_event   */
	/* ---------------------------------------- */	
	err = snmp_bc_construct_nc_rpt(e, &res_info_ptr, ep_root, nc_index);
	if (err) {
		snmp_bc_free_oh_event(e);
		return(err);
	}
	
	/* ---------------------------------------- */
	/* Discover rdrs.                           */
	/* Add rpt and rdrs to rptcache.            */
	/* ---------------------------------------- */				
	err = snmp_bc_add_nc_rptcache(handle, e, res_info_ptr, nc_index); 
			
	snmp_bc_free_oh_event(e);
			
	return(SA_OK);
}

/**
 * snmp_bc_discover_mx:
 * @handler: Pointer to handler's data.
 * @ep_root: Pointer to chassis Root Entity Path which comes from openhpi.conf.
 * @blower_vector: Bitmap vector of installed Multiplex (mx) cards.
 *
 * Discovers Multiplex Card resources and their RDRs.
 *
 * Return values:
 * SA_OK - normal case.
 * SA_ERR_HPI_OUT_OF_MEMORY - Cannot allocate space for internal memory.
 * SA_ERR_HPI_INVALID_PARAMS - Pointer paramter(s) NULL.
 **/
SaErrorT snmp_bc_discover_mx(struct oh_handler_state *handle,
			     SaHpiEntityPathT *ep_root, char *mx_vector)
{

	guint i;
	SaErrorT err;
        struct oh_event *e;
	struct ResourceInfo *res_info_ptr;
	struct snmp_bc_hnd *custom_handle;


	if (!handle || !mx_vector) {
		err("Invalid parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}
		
	custom_handle = (struct snmp_bc_hnd *)handle->data;
	if (!custom_handle) {
		err("Invalid parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}
	
	e= NULL;
	res_info_ptr = NULL;
	
	for (i=0; i < strlen(mx_vector); i++) {
	
		if ((mx_vector[i] == '1') || (custom_handle->isFirstDiscovery == SAHPI_TRUE))
		{
			e = snmp_bc_alloc_oh_event();
			if (e == NULL) {
				err("Out of memory.");
				return(SA_ERR_HPI_OUT_OF_MEMORY);
			}
									
			/* ---------------------------------------- */
			/* Construct .resource of struct oh_event   */
			/* ---------------------------------------- */	
			err = snmp_bc_construct_mx_rpt(e, &res_info_ptr, ep_root, i);
			if (err) {
				snmp_bc_free_oh_event(e);
				return(err);
			}
		}
		
		if ((mx_vector[i] == '0') && (custom_handle->isFirstDiscovery == SAHPI_TRUE))
		{
			res_info_ptr->cur_state = SAHPI_HS_STATE_NOT_PRESENT;
			snmp_bc_discover_res_events(handle, &(e->resource.ResourceEntity), res_info_ptr);
			snmp_bc_free_oh_event(e);
			g_free(res_info_ptr);
			
		} else if (mx_vector[i] == '1') {

			err = snmp_bc_add_mx_rptcache(handle, e, res_info_ptr, i); 
			
			if (err == SA_OK) {			
				/* ---------------------------------------- */
				/* Construct .event of struct oh_event      */	
				/* ---------------------------------------- */
				snmp_bc_set_resource_add_oh_event(e, res_info_ptr);

				/* ---------------------------------------- */
				/* Place the event in tmpqueue              */
				/* ---------------------------------------- */					
				/*custom_handle->eventq = g_slist_append(custom_handle->eventq, e);*/
                                if (e) e->hid = handle->hid;
                                oh_evt_queue_push(handle->eventq, e);
			} else {
				snmp_bc_free_oh_event(e);
			}
		}
	}
	return(SA_OK);
}

/**
 * snmp_bc_construct_mx_rpt:
 * @e: Pointer to oh_event struct.
 * @res_info_ptr: Pointer to pointer of res_info_ptr
 * @ep_root: Pointer to chassis Root Entity Path which comes from openhpi.conf.
 * @blower_index: Index of discovered mx.
 *
 * Build rpt structure for a blade resource using model data 
 *
 * Return values:
 * SA_OK - normal case.
 * SA_ERR_HPI_OUT_OF_MEMORY - Cannot allocate space for internal memory.
 * SA_ERR_HPI_INVALID_PARAMS - Pointer parameter(s) NULL.
 **/
SaErrorT snmp_bc_construct_mx_rpt(struct oh_event* e, 
				      struct ResourceInfo **res_info_ptr,
				      SaHpiEntityPathT *ep_root, 
				      guint mx_index)
{

	if (!e || !res_info_ptr) return (SA_ERR_HPI_INVALID_PARAMS);
	
	e->resource = snmp_bc_rpt_array[BC_RPT_ENTRY_MUX_MODULE].rpt;
	oh_concat_ep(&(e->resource.ResourceEntity), ep_root);
	oh_set_ep_location(&(e->resource.ResourceEntity),
			   BLADECENTER_MUX_SLOT, mx_index + SNMP_BC_HPI_LOCATION_BASE);
	oh_set_ep_location(&(e->resource.ResourceEntity),
			   SAHPI_ENT_OTHER_CHASSIS_BOARD, mx_index + SNMP_BC_HPI_LOCATION_BASE);
	e->resource.ResourceId = 
		oh_uid_from_entity_path(&(e->resource.ResourceEntity));
	snmp_bc_create_resourcetag(&(e->resource.ResourceTag),
				   snmp_bc_rpt_array[BC_RPT_ENTRY_MUX_MODULE].comment,
				   mx_index + SNMP_BC_HPI_LOCATION_BASE);

	dbg("Discovered resource=%s; ID=%d",
	    e->resource.ResourceTag.Data,
	    e->resource.ResourceId);

	/* Create platform-specific info space to add to infra-structure */
	*res_info_ptr = g_memdup(&(snmp_bc_rpt_array[BC_RPT_ENTRY_MUX_MODULE].res_info),
						sizeof(struct ResourceInfo));
	if (!(*res_info_ptr)) {
		err("Out of memory.");
		return(SA_ERR_HPI_OUT_OF_MEMORY);
	}
	
	return(SA_OK);

}

/**
 * snmp_bc_add_mx_rptcache:
 * @handle: Pointer to hpi handle
 * @e: Pointer to oh_event struct.
 * @res_info_ptr: Pointer to pointer of res_info_ptr
 * @blower_index: Index of discovered mx.
 *
 * Build rpt and rdrs for a mx (Multiplex Card) then add to rptcache 
 *
 * Return values:
 * SA_OK - normal case.
 * SA_ERR_HPI_OUT_OF_MEMORY - Cannot allocate space for internal memory.
 * SA_ERR_HPI_INVALID_PARAMS - Pointer parameter(s) NULL.
 **/
SaErrorT snmp_bc_add_mx_rptcache(struct oh_handler_state *handle, 
				  struct oh_event *e, 
				  struct ResourceInfo *res_info_ptr,
				  guint mx_index) 

{
	SaErrorT err;
	guint mx_width;
	struct snmp_value get_value;
	struct snmp_bc_hnd *custom_handle;


	if (!handle || !e || !res_info_ptr) {
		err("Invalid parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}
		
	custom_handle = (struct snmp_bc_hnd *)handle->data;
	if (!custom_handle) {
		err("Invalid parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}

	dbg("Discovering Mux Card %d resource.\n", mx_index);
	res_info_ptr->cur_state = SAHPI_HS_STATE_ACTIVE;

        /* Get UUID and convert to GUID */
        err = snmp_bc_get_guid(custom_handle, e, res_info_ptr);

	/* Add resource to temporary event cache/queue */
	err = oh_add_resource(handle->rptcache, 
				      &(e->resource),
				      res_info_ptr, 0);
	if (err) {
		err("Failed to add resource. Error=%s.", oh_lookup_error(err));
		return(err);
	}
			
	/* ---------------------------------------- */
	/* Construct .rdrs of struct oh_event       */
	/* ---------------------------------------- */								
	/* Find resource's events, sensors, controls, etc. */
	snmp_bc_discover_res_events(handle, &(e->resource.ResourceEntity), res_info_ptr);
	snmp_bc_discover_sensors(handle, snmp_bc_mux_sensors, e);
	snmp_bc_discover_controls(handle, snmp_bc_mux_controls, e);
	snmp_bc_discover_inventories(handle, snmp_bc_mux_inventories, e);
		
	mx_width = 1;    /* Default to 1-wide blade */
	if (res_info_ptr->mib.OidResourceWidth != NULL) {
		err = snmp_bc_oid_snmp_get(custom_handle,  &(e->resource.ResourceEntity), 0,
		   		res_info_ptr->mib.OidResourceWidth, &get_value, SAHPI_TRUE);
		if (!err && (get_value.type == ASN_INTEGER)) {
			mx_width = get_value.integer;
		}
	}			
	res_info_ptr->resourcewidth = mx_width;
	err = snmp_bc_set_resource_slot_state_sensor(handle, e, mx_width);
	return(err);
}

/**
 * snmp_bc_discover_mx_i:
 * @handle: Pointer to hpi handle
 * @ep_root: Pointer to .
 * @blower_index: Index of discovered mx.
 *
 * Discover a particular MUX at index mx_index.
 * This routine is used to rediscover a Multiplexer Card (mx). 
 * Blower rpt and rdrs will be added to rptcache.
 * No event will be generated.  The event is handled separately in log2event.
 *
 * Return values:
 * SA_OK - normal case.
 * SA_ERR_HPI_OUT_OF_MEMORY - Cannot allocate space for internal memory.
 * SA_ERR_HPI_INVALID_PARAMS - Pointer parameter(s) NULL.
 **/
SaErrorT snmp_bc_discover_mx_i(struct oh_handler_state *handle,
			  	   SaHpiEntityPathT *ep_root, 
				   guint mx_index)
{

	SaErrorT err;
        struct oh_event *e;
	struct ResourceInfo *res_info_ptr;

	if (!handle) {
		err("Invalid parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}
		
	e= NULL;
	res_info_ptr = NULL;
	
	e = snmp_bc_alloc_oh_event();
	if (e == NULL) {
		err("Out of memory.");
		return(SA_ERR_HPI_OUT_OF_MEMORY);
	}
									
	/* ---------------------------------------- */
	/* Construct .resource of struct oh_event   */
	/* ---------------------------------------- */	
	err = snmp_bc_construct_mx_rpt(e, &res_info_ptr, ep_root, mx_index);
	if (err) {
		snmp_bc_free_oh_event(e);
		return(err);
	}
	
	/* ---------------------------------------- */
	/* Discover rdrs.                           */
	/* Add rpt and rdrs to rptcache.            */
	/* ---------------------------------------- */				
	err = snmp_bc_add_mx_rptcache(handle, e, res_info_ptr, mx_index); 
			
	snmp_bc_free_oh_event(e);
			
	return(SA_OK);
}

/**
 * snmp_bc_discover_power_module:
 * @handler: Pointer to handler's data.
 * @ep_root: Pointer to chassis Root Entity Path which comes from openhpi.conf.
 * @power_module_vector: Bitmap vector of installed power modules.
 *
 * Discovers power module resources and their RDRs.
 *
 * Return values:
 * SA_OK - normal case.
 * SA_ERR_HPI_OUT_OF_MEMORY - Cannot allocate space for internal memory.
 * SA_ERR_HPI_INVALID_PARAMS - Pointer parameter(s) NULL.
 **/
SaErrorT snmp_bc_discover_power_module(struct oh_handler_state *handle,
			  SaHpiEntityPathT *ep_root, char *power_module_vector)
{

	int i;
	SaErrorT err;
        struct oh_event *e;
	struct ResourceInfo *res_info_ptr;
	struct snmp_bc_hnd *custom_handle;


	if (!handle || !power_module_vector) {
		err("Invalid parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}
		
	custom_handle = (struct snmp_bc_hnd *)handle->data;
	if (!custom_handle) {
		err("Invalid parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}

	e = NULL;
	res_info_ptr = NULL;
	
	for (i=0; i < strlen(power_module_vector); i++) {

		if ((power_module_vector[i] == '1') || (custom_handle->isFirstDiscovery == SAHPI_TRUE)) 
		{
			e = snmp_bc_alloc_oh_event();
			if (e == NULL) {
				err("Out of memory.");
				return(SA_ERR_HPI_OUT_OF_MEMORY);
			}

									
			/* ---------------------------------------- */
			/* Construct .resource of struct oh_event   */
			/* ---------------------------------------- */	
			err = snmp_bc_construct_pm_rpt(e, &res_info_ptr, ep_root, i);
			if (err) {
				snmp_bc_free_oh_event(e);
				return(err);
			}
		}
	
		if ((power_module_vector[i] == '0') && (custom_handle->isFirstDiscovery == SAHPI_TRUE)) 
		{
			res_info_ptr->cur_state = SAHPI_HS_STATE_NOT_PRESENT;
			snmp_bc_discover_res_events(handle, &(e->resource.ResourceEntity), res_info_ptr);
			snmp_bc_free_oh_event(e);
			g_free(res_info_ptr);
			
		} else if (power_module_vector[i] == '1') {
			err = snmp_bc_add_pm_rptcache(handle, e, res_info_ptr, i); 

			if (err == SA_OK) {
				/* ---------------------------------------- */
				/* Construct .event of struct oh_event      */	
				/* ---------------------------------------- */
				snmp_bc_set_resource_add_oh_event(e, res_info_ptr);

				/* ---------------------------------------- */
				/* Place the event in tmpqueue              */
				/* ---------------------------------------- */					
				/*custom_handle->eventq = g_slist_append(custom_handle->eventq, e);*/
                                e->hid = handle->hid;
                                oh_evt_queue_push(handle->eventq, e);
			} else {
				snmp_bc_free_oh_event(e);
                        }
		}
	}
	return(SA_OK);
}

/**
 * snmp_bc_discover_switch:
 * @handler: Pointer to handler's data.
 * @ep_root: Pointer to chassis Root Entity Path which comes from openhpi.conf.
 * @switch_vector: Bitmap vector of installed I/O modules.
 *
 * Discovers I/O module resources and their RDRs.
 *
 * Return values:
 * SA_OK - normal case.
 * SA_ERR_HPI_OUT_OF_MEMORY - Cannot allocate space for internal memory.
 * SA_ERR_HPI_INVALID_PARAMS - Pointer parameter(s) NULL.
 **/
SaErrorT snmp_bc_discover_switch(struct oh_handler_state *handle,
			         SaHpiEntityPathT *ep_root, 
				 char *switch_vector)
{

	int i;
	SaErrorT err;
        struct oh_event *e;
	struct ResourceInfo *res_info_ptr;
	struct snmp_bc_hnd *custom_handle;

	if (!handle || !switch_vector) {
		err("Invalid parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}
		
	custom_handle = (struct snmp_bc_hnd *)handle->data;
	if (!custom_handle) {
		err("Invalid parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}

	e = NULL;
	res_info_ptr = NULL;
	
	for (i=0; i < strlen(switch_vector); i++) {
	
		if ((switch_vector[i] == '1') || (custom_handle->isFirstDiscovery == SAHPI_TRUE))
		{
			e = snmp_bc_alloc_oh_event();
			if (e == NULL) {
				err("Out of memory.");
				return(SA_ERR_HPI_OUT_OF_MEMORY);
			}

									
			/* ---------------------------------------- */
			/* Construct .resource of struct oh_event   */
			/* ---------------------------------------- */	
			err = snmp_bc_construct_sm_rpt(e, &res_info_ptr, ep_root, i, custom_handle->installed_smi_mask);
			if (err) {
				snmp_bc_free_oh_event(e);
				return(err);
			}
			 
		}
		
		if ((switch_vector[i] == '0') && (custom_handle->isFirstDiscovery == SAHPI_TRUE))
		{
			res_info_ptr->cur_state = SAHPI_HS_STATE_NOT_PRESENT;
			snmp_bc_discover_res_events(handle, &(e->resource.ResourceEntity), res_info_ptr);
			snmp_bc_free_oh_event(e);
			g_free(res_info_ptr);
			
		} else if (switch_vector[i] == '1') {
			err = snmp_bc_add_switch_rptcache(handle, e, res_info_ptr, i); 
			
			if (err == SA_OK) {
				/* ---------------------------------------- */
				/* Construct .event of struct oh_event      */	
				/* ---------------------------------------- */
				snmp_bc_set_resource_add_oh_event(e, res_info_ptr);

				/* ---------------------------------------- */
				/* Place the event in tmpqueue              */
				/* ---------------------------------------- */					
				/*custom_handle->eventq = g_slist_append(custom_handle->eventq, e);*/
                                if (e) e->hid = handle->hid;
                                oh_evt_queue_push(handle->eventq, e);
			} else {
				snmp_bc_free_oh_event(e);
                        }
		}
	}
	return(SA_OK);
}


/**
 * snmp_bc_discover_mm:
 * @handler: Pointer to handler's data.
 * @ep_root: Pointer to chassis Root Entity Path which comes from openhpi.conf.
 * @mm_vector: Bitmap vector of installed MMs.
 * @global_discovery: Also include Virtual MM in the discovery
 *
 * Discovers management module (MM) resources and their RDRs.
 *
 * Return values:
 * SA_OK - normal case.
 * SA_ERR_HPI_OUT_OF_MEMORY - Cannot allocate space for internal memory.
 * SA_ERR_HPI_INVALID_PARAMS - Pointer parameter(s) NULL.
 **/
SaErrorT snmp_bc_discover_mm(struct oh_handler_state *handle,
			     SaHpiEntityPathT *ep_root, char *mm_vector,
			     SaHpiBoolT global_discovery)
{

	guint i;
	SaErrorT err;
        struct oh_event *e;
	struct snmp_value get_value;
	SaHpiRdrT *rdr;
	struct SensorInfo *sinfo;
	struct ResourceInfo *res_info_ptr;
	struct snmp_bc_hnd *custom_handle;


	if (!handle || !mm_vector) {
		err("Invalid parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}
		
	custom_handle = (struct snmp_bc_hnd *)handle->data;
	if (!custom_handle) {
		err("Invalid parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}
	
	e = NULL;
	res_info_ptr = NULL;
	
	/* Discover Virtual MM */
	if (global_discovery == SAHPI_TRUE)
	{			
		e = snmp_bc_alloc_oh_event();
		if (e == NULL) {
			err("Out of memory.");
			return(SA_ERR_HPI_OUT_OF_MEMORY);
		}

									
		/* ---------------------------------------- */
		/* Construct .resource of struct oh_event   */
		/* ---------------------------------------- */	
                e->resource = snmp_bc_rpt_array[BC_RPT_ENTRY_VIRTUAL_MGMNT_MODULE].rpt;
                oh_concat_ep(&(e->resource.ResourceEntity), ep_root);
                oh_set_ep_location(&(e->resource.ResourceEntity),
                                       SAHPI_ENT_SYS_MGMNT_MODULE, 0);
                e->resource.ResourceId =
                       	oh_uid_from_entity_path(&(e->resource.ResourceEntity));
                snmp_bc_create_resourcetag(&(e->resource.ResourceTag),
                                             snmp_bc_rpt_array[BC_RPT_ENTRY_VIRTUAL_MGMNT_MODULE].comment,
                                             0);

                dbg("Discovered resource=%s; ID=%d",
                    e->resource.ResourceTag.Data,
                    e->resource.ResourceId);

                /* Create platform-specific info space to add to infra-structure */
                res_info_ptr = g_memdup(&(snmp_bc_rpt_array[BC_RPT_ENTRY_VIRTUAL_MGMNT_MODULE].res_info),
                        	                sizeof(struct ResourceInfo));
                if (!res_info_ptr) {
                        err("Out of memory.");
                        snmp_bc_free_oh_event(e);
                        return(SA_ERR_HPI_OUT_OF_MEMORY);
                }

                /* Add resource to resource cache */
                err = oh_add_resource(handle->rptcache,
                        	        &(e->resource),
                                	        res_info_ptr, 0);
                if (err) {
                        err("Failed to add resource. Error=%s.", oh_lookup_error(err));
                        snmp_bc_free_oh_event(e);
                        return(err);
                }
		/* ---------------------------------------- */
		/* Construct .rdrs of struct oh_event       */
		/* ---------------------------------------- */						
		/* Find resource's events, sensors, controls, etc. */
                snmp_bc_discover_res_events(handle, &(e->resource.ResourceEntity), res_info_ptr);
                snmp_bc_discover_sensors(handle, snmp_bc_virtual_mgmnt_sensors, e);
                snmp_bc_discover_controls(handle, snmp_bc_virtual_mgmnt_controls, e);
                snmp_bc_discover_inventories(handle, snmp_bc_virtual_mgmnt_inventories, e);
		
		/* -------------------------------------------- */
		/* Adjust initial state of VMM Redudancy Sensor */
		/* -------------------------------------------- */
		rdr =  oh_get_rdr_by_type(handle->rptcache, e->resource.ResourceId,
                              		   SAHPI_SENSOR_RDR, 
					   BLADECENTER_SENSOR_NUM_MGMNT_REDUNDANCY);
		
		if (rdr) { 
			sinfo = (struct SensorInfo *)oh_get_rdr_data(handle->rptcache, 
								e->resource.ResourceId, rdr->RecordId);
			if ((strncmp(mm_vector, "11", 2) == 0)) {
				sinfo->cur_state = SAHPI_ES_FULLY_REDUNDANT;
			} else {
				sinfo->cur_state = SAHPI_ES_NON_REDUNDANT_INSUFFICIENT_RESOURCES;
			}
			sinfo->cur_child_rid = 	e->resource.ResourceId;
			
			err = oh_add_rdr(handle->rptcache,
					 e->resource.ResourceId,
					 rdr,
				 	 sinfo, 0);				
		}

		rdr =  oh_get_rdr_by_type(handle->rptcache, e->resource.ResourceId,
                              		   SAHPI_SENSOR_RDR, 
					   BLADECENTER_SENSOR_NUM_MGMNT_STANDBY);
		
		if (rdr) { 
			sinfo = (struct SensorInfo *)oh_get_rdr_data(handle->rptcache, 
								e->resource.ResourceId, rdr->RecordId);
			if ((strncmp(mm_vector, "11", 2) == 0)) {
				sinfo->cur_state = SAHPI_ES_PRESENT;
			} else {
				sinfo->cur_state = SAHPI_ES_ABSENT;
			}
			sinfo->cur_child_rid = 	e->resource.ResourceId;
			
			err = oh_add_rdr(handle->rptcache,
					 e->resource.ResourceId,
					 rdr,
				 	 sinfo, 0);				
		}

		/* ---------------------------------------- */
		/* Construct .event of struct oh_event      */	
		/* ---------------------------------------- */
		snmp_bc_set_resource_add_oh_event(e, res_info_ptr);

		/* ---------------------------------------- */
		/* Place the event in tmpqueue              */
		/* ---------------------------------------- */					
		/*custom_handle->eventq = g_slist_append(custom_handle->eventq, e);*/
                e->hid = handle->hid;
                oh_evt_queue_push(handle->eventq, e);
				
	}		
			
	/* Discover Physical MM */                				
	for (i=0; i < strlen(mm_vector); i++) {
		dbg("Management Module installed bit map %s", get_value.string);
		if ((mm_vector[i] == '1') || (custom_handle->isFirstDiscovery == SAHPI_TRUE))
		{
			e = snmp_bc_alloc_oh_event();
			if (e == NULL) {
				err("Out of memory.");
				return(SA_ERR_HPI_OUT_OF_MEMORY);
			}
									
			/* ---------------------------------------- */
			/* Construct .resource of struct oh_event   */
			/* ---------------------------------------- */
			err = snmp_bc_construct_mm_rpt(e, &res_info_ptr, ep_root, i, custom_handle->installed_mmi_mask);
			if (err) {
				snmp_bc_free_oh_event(e);
				return(err);
			}
			
		}

		if ((mm_vector[i] == '0') && (custom_handle->isFirstDiscovery == SAHPI_TRUE))
		{
		        res_info_ptr->cur_state = SAHPI_HS_STATE_NOT_PRESENT;
			snmp_bc_discover_res_events(handle, &(e->resource.ResourceEntity), res_info_ptr);
			snmp_bc_free_oh_event(e);
			g_free(res_info_ptr);
			
		} else if (mm_vector[i] == '1'){
			err = snmp_bc_add_mm_rptcache(handle, e, res_info_ptr, i); 
			
			if (err == SA_OK) {
				/* ---------------------------------------- */
				/* Construct .event of struct oh_event      */	
				/* ---------------------------------------- */
				snmp_bc_set_resource_add_oh_event(e, res_info_ptr);

				/* ---------------------------------------- */
				/* Place the event in tmpqueue              */
				/* ---------------------------------------- */					
				/*custom_handle->eventq = g_slist_append(custom_handle->eventq, e);*/
                                if (e) e->hid = handle->hid;
                                oh_evt_queue_push(handle->eventq, e);
			} else 
				snmp_bc_free_oh_event(e);
			
		}
	}
	return(SA_OK);
}

/**
 * snmp_bc_discover_ipmi_sensors:
 * @handler: Pointer to handler's data.
 * @sensor_array: Pointer to resource's static sensor data array.
 * @parent_res_event: Pointer to resource's event structure.
 *
 * Discovers resource's available IPMI sensors and their events.
 *
 * Return values:
 * Adds sensor RDRs to internal Infra-structure queues - normal case
 * SA_ERR_HPI_OUT_OF_MEMORY - Cannot allocate space for internal memory
 **/
static SaErrorT snmp_bc_discover_ipmi_sensors(struct oh_handler_state *handle,
					      struct snmp_bc_ipmi_sensor *sensor_array,
					      struct oh_event *res_oh_event)
{
	int i;
	GHashTable *ipmi_sensor_hash;
	SaErrorT err, rtn_code = SA_OK;
	struct SensorMibInfo *mib_info;
	struct snmp_bc_hnd *custom_handle;
	struct snmp_value get_value;
	SaHpiRdrT *rdrptr;
	struct SensorInfo *sinfo;
	
	custom_handle = (struct snmp_bc_hnd *)handle->data;

	/* Check if this is an IPMI blade */
	err = snmp_bc_oid_snmp_get(custom_handle,
				   &(res_oh_event->resource.ResourceEntity), 0,
				   SNMP_BC_IPMI_TEMP_BLADE_OID, &get_value, SAHPI_FALSE);
				  
        if (err || get_value.type != ASN_INTEGER) {
		err("Cannot get OID=%s; Received Type=%d; Error=%s.",
		   SNMP_BC_IPMI_TEMP_BLADE_OID, get_value.type, oh_lookup_error(err));
		if (err) { return(err); }
		else { return(SA_ERR_HPI_INTERNAL_ERROR); }
        }
	if (get_value.integer == 0) return(SA_OK); /* Not an IPMI Blade */

	/* Create an temporary hash table and populate with all of 
           the blade's active IPMI sensors */
	ipmi_sensor_hash = g_hash_table_new(g_str_hash, g_str_equal);
	
	if (ipmi_sensor_hash == NULL) {
		err("Out of memory.");
		return(SA_ERR_HPI_OUT_OF_MEMORY);
	}

	/***************************************** 
	 * Search for all the defined IPMI sensors
         *****************************************/

	/* Find blade's defined temperature IPMI sensors */
	for (i=0; i<SNMP_BC_MAX_IPMI_TEMP_SENSORS; i++) {
		err = snmp_bc_oid_snmp_get(custom_handle,
					   &(res_oh_event->resource.ResourceEntity), 0,
					   snmp_bc_ipmi_sensors_temp[i].oid, &get_value, SAHPI_FALSE);
		if (!err) {
			char *hash_existing_key, *hash_value;
			gchar  **strparts = NULL;
			gchar  *s, *ipmi_tag;
			
			/* Find IPMI tag in returned value */
			strparts = g_strsplit(get_value.string, SNMP_BC_IPMI_STRING_DELIMITER, -1);
			if (strparts == NULL || strparts[0] == '\0') {
				err("Cannot split IPMI temp returned value=%s.", get_value.string);
				g_strfreev(strparts);
				continue;
			}
			ipmi_tag = g_strstrip(g_strdup(strparts[0]));
			g_strfreev(strparts);
			if (ipmi_tag == NULL || ipmi_tag[0] == '\0') {
				err("Stripped IPMI tag is NULL"); 
				g_free(ipmi_tag);
				continue;
			}
			
			/* Change IPMI Tag to upper case */
			for (s=ipmi_tag; *s; s++) { *s = g_ascii_toupper(*s); }
			
			dbg("Found OID IPMI sensor=%s", ipmi_tag);

			/* Insert tag and OID info in temporary hash */
			if (!g_hash_table_lookup_extended(ipmi_sensor_hash,
							  ipmi_tag,
							  (gpointer)&hash_existing_key,
							  (gpointer)&hash_value)) {

				mib_info = g_memdup(&(snmp_bc_ipmi_sensors_temp[i]),
						    sizeof(struct SensorMibInfo));
				if (!mib_info) {
					err("Out of memory.");
					g_free(ipmi_tag);
					rtn_code = SA_ERR_HPI_OUT_OF_MEMORY;
					goto CLEANUP;
				}
				g_hash_table_insert(ipmi_sensor_hash, ipmi_tag, mib_info);
			}
			else { /* Already exists */
				err("Duplicate IPMI OID=%s.", snmp_bc_ipmi_sensors_temp[i].oid);
				g_free(ipmi_tag);
			}
		}
	}

	/* Find blade's voltage IPMI sensors */
	for (i=0; i<SNMP_BC_MAX_IPMI_VOLTAGE_SENSORS; i++) {
		err = snmp_bc_oid_snmp_get(custom_handle,
					   &(res_oh_event->resource.ResourceEntity), 0,
					   snmp_bc_ipmi_sensors_voltage[i].oid, &get_value, SAHPI_FALSE);
		if (!err) {
			char *hash_existing_key, *hash_value;
			gchar  **strparts = NULL;
			gchar  *s, *ipmi_tag;
			
			/* Find IPMI tag in returned value */
			strparts = g_strsplit(get_value.string, SNMP_BC_IPMI_STRING_DELIMITER, -1);
			if (strparts == NULL || strparts[0] == '\0') {
				err("Cannot split IPMI voltage returned value=%s.", get_value.string);
				g_strfreev(strparts);
				continue;
			}
			ipmi_tag = g_strstrip(g_strdup(strparts[0]));
			g_strfreev(strparts);
			if (ipmi_tag == NULL || ipmi_tag[0] == '\0') {
				err("Stripped IPMI tag is NULL"); 
				g_free(ipmi_tag);
				continue;
			}

			/* Change IPMI Tag to upper case */
			for (s=ipmi_tag; *s; s++) { *s = g_ascii_toupper(*s); }
			
			dbg("Found OID IPMI sensor=%s", ipmi_tag);

			/* Insert tag and OID info in temporary hash */
			if (!g_hash_table_lookup_extended(ipmi_sensor_hash,
							  ipmi_tag,
							  (gpointer)&hash_existing_key,
							  (gpointer)&hash_value)) {

				mib_info = g_memdup(&(snmp_bc_ipmi_sensors_voltage[i]), sizeof(struct SensorMibInfo));
				if (!mib_info) {
					err("Out of memory.");
					g_free(ipmi_tag);
					rtn_code = SA_ERR_HPI_OUT_OF_MEMORY;
					goto CLEANUP;
				}
				g_hash_table_insert(ipmi_sensor_hash, ipmi_tag, mib_info);
			}
			else { /* Already exists */
				dbg("Duplicate IPMI OID=%s.", snmp_bc_ipmi_sensors_voltage[i].oid);
				g_free(ipmi_tag);
			}
		}
	}

	/* Iterate thru all the possible IPMI sensors, if it's defined for this blade,
	   push up its RDR info to Infra-structure */
	for (i=0; sensor_array[i].ipmi.index != 0; i++) {
		mib_info = (struct SensorMibInfo *)g_hash_table_lookup(ipmi_sensor_hash, sensor_array[i].ipmi_tag);
		
		/* See if the tag has an alias */
		if (!mib_info && (sensor_array[i].ipmi_tag_alias1 && sensor_array[i].ipmi_tag_alias1[0] != '\0')) {
			mib_info = (struct SensorMibInfo *)g_hash_table_lookup(ipmi_sensor_hash, sensor_array[i].ipmi_tag_alias1);	
		}

		if (mib_info) {

			rdrptr = (SaHpiRdrT *)g_malloc0(sizeof(SaHpiRdrT));
			if (rdrptr == NULL) {
				err("Out of memory.");
				rtn_code = SA_ERR_HPI_OUT_OF_MEMORY;
				goto CLEANUP;
			}

			rdrptr->RdrType = SAHPI_SENSOR_RDR;
			rdrptr->Entity = res_oh_event->resource.ResourceEntity;
			err = snmp_bc_mod_sensor_ep(rdrptr, sensor_array, i);
			rdrptr->RdrTypeUnion.SensorRec = sensor_array[i].ipmi.sensor;

			dbg("Blade Found IPMI Sensor=%s", sensor_array[i].ipmi.comment);

			oh_init_textbuffer(&(rdrptr->IdString));
			oh_append_textbuffer(&(rdrptr->IdString), sensor_array[i].ipmi.comment);
			
			sinfo = g_memdup(&(sensor_array[i].ipmi.sensor_info), sizeof(struct SensorInfo));
			if (!sinfo) {
				err("Out of memory.");
				rtn_code = SA_ERR_HPI_OUT_OF_MEMORY;
				g_free(rdrptr);
				goto CLEANUP;
			}

			sinfo->mib = *mib_info;
			
			/*  Add rdr to resource cache */
			err = oh_add_rdr(handle->rptcache,
					 res_oh_event->resource.ResourceId,
					 rdrptr,
					 sinfo, 0);
			if (err) {
				err("Cannot add RDR. Error=%s.", oh_lookup_error(err));
				g_free(rdrptr);
			}
			else {
				res_oh_event->rdrs = g_slist_append(res_oh_event->rdrs, rdrptr);
				snmp_bc_discover_sensor_events(handle,
							       &(res_oh_event->resource.ResourceEntity),
							       sensor_array[i].ipmi.sensor.Num,
							       &(sensor_array[i].ipmi));
			}
		}
	}

 CLEANUP:
        /* Destroy temporary hash table */
	g_hash_table_foreach(ipmi_sensor_hash, free_hash_data, NULL);
	g_hash_table_destroy(ipmi_sensor_hash);

	return(rtn_code);
}

static void free_hash_data(gpointer key, gpointer value, gpointer user_data)
{
        g_free(key);
        g_free(value);
}

/**
 * snmp_bc_rediscover: 
 * @handler: Pointer to handler's data.
 * @event:   Pointer to event being processed.
 *
 * Check install masks and target discovery 
 *   -- If resource is removed, then remove rpt and associated rdr's
 *   -- In resource is newly installed, then rediscover ...
 *
 * Return values:
 *
 **/
SaErrorT snmp_bc_rediscover(struct oh_handler_state *handle,
			  		SaHpiEventT *working_event, 
					LogSource2ResourceT *logsrc2res)
{
	SaErrorT err;
	gint i, j;
        SaHpiRptEntryT *res;
	guint rediscovertype;
	SaHpiBoolT foundit, isSMI;
	struct snmp_value get_value;	
	struct snmp_bc_hnd *custom_handle;
	char *root_tuple;
	struct ResourceInfo *resinfo;
	char resource_mask[SNMP_BC_MAX_RESOURCES_MASK];
	SaHpiEntityPathT     ep_root;
	SaHpiEntityTypeT     hotswap_entitytype;
    	SaHpiEntityLocationT hotswap_entitylocation;


	if (!handle) {
		err("Invalid parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}
		
	custom_handle = (struct snmp_bc_hnd *)handle->data;
	if (!custom_handle) {
		err("Invalid parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}

	memset(&resource_mask, 0, SNMP_BC_MAX_RESOURCES_MASK);
			
	rediscovertype = snmp_bc_isrediscover(working_event);
	
	/* ------------------------------------------------------------------ */
	/* Parse EntityPath to find out the type of resource being hotswapped */
	/* ------------------------------------------------------------------ */
	memset(resource_mask, '\0', SNMP_BC_MAX_RESOURCES_MASK);
	isSMI   = SAHPI_FALSE;
	foundit = SAHPI_FALSE;
	hotswap_entitytype = SAHPI_ENT_UNKNOWN;
	hotswap_entitylocation = SNMP_BC_NOT_VALID;   /* Invalid location                   */
						 /* Do not use 0 for invalid location  */
						 /* because virtual resource has loc 0 */
	for (i=0; logsrc2res->ep.Entry[i].EntityType != SAHPI_ENT_SYSTEM_CHASSIS; i++) {

		switch (logsrc2res->ep.Entry[i].EntityType) {
			case SAHPI_ENT_SBC_BLADE: 
			case SAHPI_ENT_FAN: 
			case SAHPI_ENT_POWER_SUPPLY: 
			case SAHPI_ENT_SWITCH: 
			case SAHPI_ENT_SYS_MGMNT_MODULE:
			case SAHPI_ENT_PERIPHERAL_BAY:
			case SAHPI_ENT_DISPLAY_PANEL:
			case SAHPI_ENT_OTHER_CHASSIS_BOARD:
			case (SAHPI_ENT_BATTERY + 13):
			case (SAHPI_ENT_PHYSICAL_SLOT + 16):
			case SAHPI_ENT_INTERCONNECT:
				foundit = SAHPI_TRUE;
				hotswap_entitytype = logsrc2res->ep.Entry[i].EntityType;
 				hotswap_entitylocation = logsrc2res->ep.Entry[i].EntityLocation;
				for (j=0; j < SNMP_BC_MAX_RESOURCES_MASK; j++) {
					if (  j != (hotswap_entitylocation - 1) ) 
							resource_mask[j] = '0';
					else resource_mask[j] = '1';
				}
				
                                isSMI = SAHPI_TRUE;
                                if ( logsrc2res->ep.Entry[i].EntityType == SAHPI_ENT_INTERCONNECT) {
                                        if (logsrc2res->ep.Entry[i+1].EntityType == BLADECENTER_SYS_MGMNT_MODULE_SLOT) {
                                                isSMI = SAHPI_FALSE;
                                        }
                                }		
				break;
			default:
				break;
		}
		
		if (foundit) break;
	}
	
	if ( (!foundit)  || ( hotswap_entitylocation == 0xFF) ) {
		err("Hotswap event for non hotswap-able resource\n");
		return(SA_OK);
	}

	/* ------------------------------------------------------------------ */
	/* Hotswap: removal ...                                               */ 
	/* remove rpt and associated rdrs of the removed resource             */ 
	/* ------------------------------------------------------------------ */
	
	if (rediscovertype == SNMP_BC_RESOURCE_REMOVED ) {
		
		res = oh_get_resource_by_id(handle->rptcache, working_event->Source);
		resinfo = (struct ResourceInfo *)oh_get_resource_data(handle->rptcache, working_event->Source);
					
        	if (res)  {
			/* Remove resource, rpt and rdr's, from plugin copy of rptcache */
			/* No longer need to generate OH_ET_RESOURCE_DEL event,         */
			/* because the new oh_event struct handler will remove infrastru*/
			/* rpt and rdr's based on hotswap event state.                  */

			if (resinfo)
				resinfo->cur_state = SAHPI_HS_STATE_NOT_PRESENT;

			err = snmp_bc_reset_resource_slot_state_sensor(handle, res);
                	oh_remove_resource(handle->rptcache, res->ResourceId);

			switch (hotswap_entitytype) {
				case SAHPI_ENT_SBC_BLADE:
					/* Fetch blade installed vector */
					get_installed_mask(SNMP_BC_PB_INSTALLED, get_value);
					strncpy(custom_handle->installed_pb_mask, get_value.string, SNMP_BC_MAX_RESOURCES_MASK);
					break;
				case SAHPI_ENT_FAN:
					/* Fetch blower installed vector */
					get_installed_mask(SNMP_BC_BLOWER_INSTALLED, get_value);
					strncpy(custom_handle->installed_blower_mask, get_value.string, SNMP_BC_MAX_RESOURCES_MASK);
					break;
				case SAHPI_ENT_POWER_SUPPLY:
					/* Fetch power module installed vector */
					get_installed_mask(SNMP_BC_PM_INSTALLED, get_value);
					strncpy(custom_handle->installed_pm_mask, get_value.string, SNMP_BC_MAX_RESOURCES_MASK);
					break;
				case SAHPI_ENT_SWITCH:
					/* Fetch switch installed vector */
					get_installed_mask(SNMP_BC_SM_INSTALLED, get_value);
					strncpy(custom_handle->installed_sm_mask, get_value.string, SNMP_BC_MAX_RESOURCES_MASK);
					break;
				case SAHPI_ENT_SYS_MGMNT_MODULE:
					/* Fetch MMs installed vector */
					get_installed_mask(SNMP_BC_MM_INSTALLED, get_value);
					strncpy(custom_handle->installed_mm_mask, get_value.string, SNMP_BC_MAX_RESOURCES_MASK);
					break;
				case SAHPI_ENT_PERIPHERAL_BAY:
					/* get_dualmode_object(SNMP_BC_MT_INSTALLED, get_value); */
					snmp_bc_fetch_MT_install_mask(handle, &get_value);
					custom_handle->installed_mt_mask = get_value.integer;
					break;
				case (SAHPI_ENT_PHYSICAL_SLOT + 16):
					/* Fetch filter (front bezel) installed vector */
					get_dualmode_object(SNMP_BC_FILTER_INSTALLED, get_value);
					custom_handle->installed_filter_mask = get_value.integer;		
					break;
				case SAHPI_ENT_DISPLAY_PANEL:				
					/* Fetch telco-alarm-panel installed vector  */
					get_installed_mask(SNMP_BC_AP_INSTALLED, get_value);
					strncpy(custom_handle->installed_tap_mask, get_value.string, SNMP_BC_MAX_RESOURCES_MASK);
					break;
				case (SAHPI_ENT_BATTERY + 13):
					/* Fetch network-clock-card installed vector  */
					get_installed_mask(SNMP_BC_NC_INSTALLED, get_value);
					strncpy(custom_handle->installed_nc_mask, get_value.string, SNMP_BC_MAX_RESOURCES_MASK);
					break;
				case SAHPI_ENT_OTHER_CHASSIS_BOARD:
					/* Fetch mux-card installed vector  */
					get_installed_mask(SNMP_BC_MX_INSTALLED, get_value);
					strncpy(custom_handle->installed_mx_mask, get_value.string, SNMP_BC_MAX_RESOURCES_MASK);
					break;
				case SAHPI_ENT_INTERCONNECT:
                                        /* Fetch new interposer-card installed vector  */
                                        if(isSMI) {
                                                get_installed_mask(SNMP_BC_SMI_INSTALLED, get_value);
                                                strncpy(custom_handle->installed_smi_mask, get_value.string, SNMP_BC_MAX_RESOURCES_MASK);
                                        } else {
                                                get_installed_mask(SNMP_BC_MMI_INSTALLED, get_value);
                                                strncpy(custom_handle->installed_mmi_mask, get_value.string, SNMP_BC_MAX_RESOURCES_MASK);
                                        }
                                        break;
										
				default: 
					err("Unrecognize Hotswap Entity %d\n", hotswap_entitytype);
					break;
			}
        	} else err("No valid resource at hand. Could not remove resource.");
		
		return(SA_OK);
	}
	
	if ( rediscovertype == SNMP_BC_RESOURCE_INSTALLED)
	{

		oh_init_ep(&ep_root);
		root_tuple = (gchar *)g_hash_table_lookup(handle->config, "entity_root");
        	oh_encode_entitypath(root_tuple, &ep_root);

		/* Initialize tmpqueue for temporary RDR cache */
		/* tmpqueue is no longer used, 08/16/06        */ 
		/* custom_handle->tmpqueue = NULL;             */

		/* --------------------------------------------------------- */
		/* Fetch various resource installation maps from BladeCenter */
		/* --------------------------------------------------------- */	

		switch (hotswap_entitytype) {
			case SAHPI_ENT_SBC_BLADE:
				/* Fetch blade installed vector */
				get_installed_mask(SNMP_BC_PB_INSTALLED, get_value);
				for (i=0; i < strlen(get_value.string); i++) {
					if ( custom_handle->installed_pb_mask[i] != 
								get_value.string[i] ) {
						err = snmp_bc_discover_blade_i(handle, &ep_root,i);								
					}		
				}
				strncpy(custom_handle->installed_pb_mask, get_value.string, SNMP_BC_MAX_RESOURCES_MASK);
				break;
			case SAHPI_ENT_FAN:
				/* Fetch blower installed vector */
				get_installed_mask(SNMP_BC_BLOWER_INSTALLED, get_value);
				for (i=0; i < strlen(get_value.string); i++) {
					if ( custom_handle->installed_blower_mask[i] != 
								get_value.string[i] ) {
						err = snmp_bc_discover_blower_i(handle, &ep_root,i);								
					}		
				}				
				strncpy(custom_handle->installed_blower_mask, get_value.string, SNMP_BC_MAX_RESOURCES_MASK);
				break;
			case SAHPI_ENT_POWER_SUPPLY:
				/* Fetch power module installed vector */
				get_installed_mask(SNMP_BC_PM_INSTALLED, get_value);
				for (i=0; i < strlen(get_value.string); i++) {
					if ( custom_handle->installed_pm_mask[i] != 
								get_value.string[i] ) {
						err = snmp_bc_discover_pm_i(handle, &ep_root,i);								
					}		
				}								
				strncpy(custom_handle->installed_pm_mask, get_value.string, SNMP_BC_MAX_RESOURCES_MASK);
				break;
			case SAHPI_ENT_SWITCH:
				/* Fetch switch installed vector */
				get_installed_mask(SNMP_BC_SM_INSTALLED, get_value);
				for (i=0; i < strlen(get_value.string); i++) {
					if ( custom_handle->installed_sm_mask[i] != 
								get_value.string[i] ) {
						err = snmp_bc_discover_switch_i(handle, &ep_root,i);								
					}		
				}												
				strncpy(custom_handle->installed_sm_mask, get_value.string, SNMP_BC_MAX_RESOURCES_MASK);
				break;
			case SAHPI_ENT_SYS_MGMNT_MODULE:
				/* Fetch MMs installed vector */
				get_installed_mask(SNMP_BC_MM_INSTALLED, get_value);
				for (i=0; i < strlen(get_value.string); i++) {
					if ( custom_handle->installed_mm_mask[i] != 
								get_value.string[i] ) {
						err = snmp_bc_discover_mm_i(handle, &ep_root,i);
					}
				}
				strncpy(custom_handle->installed_mm_mask, get_value.string, SNMP_BC_MAX_RESOURCES_MASK);
				break;
			case SAHPI_ENT_PERIPHERAL_BAY:
				/* get_dualmode_object(SNMP_BC_MT_INSTALLED, get_value); */
				snmp_bc_fetch_MT_install_mask(handle, &get_value);
				custom_handle->installed_mt_mask = get_value.integer;
				switch (hotswap_entitylocation)
				{
					case 1:
						err = snmp_bc_discover_media_tray(handle, &ep_root, 10);
						break;
					case 2:
						err = snmp_bc_discover_media_tray(handle, &ep_root, 01);
						break;					
					default:
						break;
				}
				
				break;
			case SAHPI_ENT_DISPLAY_PANEL:
				get_installed_mask(SNMP_BC_AP_INSTALLED, get_value);
				for (i=0; i < strlen(get_value.string); i++) {
					if ( custom_handle->installed_tap_mask[i] != 
								get_value.string[i] ) {
						err = snmp_bc_discover_tap_i(handle, &ep_root,i);
					}
				}
				strncpy(custom_handle->installed_tap_mask, get_value.string, SNMP_BC_MAX_RESOURCES_MASK);
				break;
			case SAHPI_ENT_OTHER_CHASSIS_BOARD:			
				/* Fetch mux-card installed vector  */
				get_installed_mask(SNMP_BC_MX_INSTALLED, get_value);
				for (i=0; i < strlen(get_value.string); i++) {
					if ( custom_handle->installed_mx_mask[i] != 
								get_value.string[i] ) {
						err = snmp_bc_discover_mx_i(handle, &ep_root,i);
					}
				}
				strncpy(custom_handle->installed_mx_mask, get_value.string, SNMP_BC_MAX_RESOURCES_MASK);
				break;
			case (SAHPI_ENT_BATTERY + 13):
				/* Fetch network-clock-card installed vector  */
				get_installed_mask(SNMP_BC_NC_INSTALLED, get_value);
				for (i=0; i < strlen(get_value.string); i++) {
					if ( custom_handle->installed_nc_mask[i] != 
								get_value.string[i] ) {
						err = snmp_bc_discover_nc_i(handle, &ep_root,i);
					}
				}				
				strncpy(custom_handle->installed_nc_mask, get_value.string, SNMP_BC_MAX_RESOURCES_MASK);
				break;
			case (SAHPI_ENT_PHYSICAL_SLOT + 16):
				/* Fetch filter (front bezel) installed vector */
				get_dualmode_object(SNMP_BC_FILTER_INSTALLED, get_value);
				err = snmp_bc_discover_filter(handle, &ep_root, get_value.integer);
				custom_handle->installed_filter_mask = get_value.integer;		
				break;
			case SAHPI_ENT_INTERCONNECT:
                                if (isSMI) {
                                        /* Fetch Switch Module Interposer installed vector  */
                                        get_installed_mask(SNMP_BC_SMI_INSTALLED, get_value);
                                        for (i=0; i < strlen(get_value.string); i++) {
                                                if ( custom_handle->installed_smi_mask[i] !=
                                                                get_value.string[i] ) {
                                                        err = snmp_bc_discover_smi_i(handle, &ep_root,i);
                                                }
                                        }
                                        strncpy(custom_handle->installed_smi_mask, get_value.string, SNMP_BC_MAX_RESOURCES_MASK);

                                } else {
                                        /* Fetch Management Module Interposer installed vector  */
                                        get_installed_mask(SNMP_BC_MMI_INSTALLED, get_value);
                                        for (i=0; i < strlen(get_value.string); i++) {
                                                if ( custom_handle->installed_mmi_mask[i] !=
                                                                get_value.string[i] ) {
                                                        err = snmp_bc_discover_mmi_i(handle, &ep_root,i);
                                                }
                                        }
                                        strncpy(custom_handle->installed_mmi_mask, get_value.string, SNMP_BC_MAX_RESOURCES_MASK);
                                }
                                break;
			default: 
				err("Unrecognize Hotswap Entity %d\n", hotswap_entitytype);
				break;
		}

		
		/** 
		 **  Before returning, see if we need to readjust current Hotswap state.
		 **  (1) Previously, snmp_bc_log2event()/snmp_bc_set_cur_prev_event_states() set 
		 **      HotSwapState =  SAHPI_HS_STATE_INACTIVE by default if there **was** no rpt,
		 **      no resinfo.   
		 **  (2) Now that rediscovery is complete, check handle->rptcache for this resource
		 **      CAPABILITY.  If it is Managed Hotswap, then INACTIVE HotSwapState is OK.
		 **      If it is Simple Hotswap, then HotSwapState needs to be set to ACTIVE in both event
		 **      and resinfo. 
		 **/
		 res = oh_get_resource_by_ep(handle->rptcache, &logsrc2res->ep);
		 if (res) {
		 	if ( (working_event->EventType == SAHPI_ET_HOTSWAP) 
					&& (working_event->EventDataUnion.HotSwapEvent.HotSwapState == SAHPI_HS_STATE_INACTIVE) ) 
			{
				if (!(res->ResourceCapabilities & SAHPI_CAPABILITY_MANAGED_HOTSWAP))
				{
					resinfo = (struct ResourceInfo *)oh_get_resource_data(handle->rptcache, 
											working_event->Source);
			
					resinfo->cur_state =
						working_event->EventDataUnion.HotSwapEvent.HotSwapState = 
										SAHPI_HS_STATE_ACTIVE;
				}
			}
		}
	}	
	return(SA_OK);
}

/**
 * snmp_bc_discover_all_slots
 * @handler: Pointer to handler's data.
 * @ep_root: Pointer to chassis Root Entity Path which comes from openhpi.conf.
 *
 * Discovers all BladeCenter physical slots.                 
 *
 * Return values:
 * SA_OK - normal case.
 * SA_ERR_HPI_OUT_OF_MEMORY - Cannot allocate space for internal memory.
 * SA_ERR_HPI_INVALID_PARAMS - Pointer parameter(s) NULL.
  **/
SaErrorT snmp_bc_discover_all_slots(struct oh_handler_state *handle,
					SaHpiEntityPathT *ep_root)
{

	guint i;
	SaErrorT err;
	struct snmp_bc_hnd *custom_handle;

	if (!handle || !ep_root) {
		err("Invalid parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}
		
	custom_handle = (struct snmp_bc_hnd *)handle->data;
	if (!custom_handle) {
		err("Invalid parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}

	for (i = 0; i < custom_handle->max_pb_supported; i++) {
		err = snmp_bc_discover_slot(handle, ep_root, SAHPI_ENT_PHYSICAL_SLOT,i); 
	}							

	for (i = 0; i < custom_handle->max_blower_supported; i++) {
		err = snmp_bc_discover_slot(handle, ep_root, BLADECENTER_BLOWER_SLOT,i);
	}							
	for (i = 0; i < custom_handle->max_pm_supported; i++) {
		err = snmp_bc_discover_slot(handle, ep_root, BLADECENTER_POWER_SUPPLY_SLOT,i);
	}
								
	for (i = 0; i < custom_handle->max_sm_supported; i++) {
		err = snmp_bc_discover_slot(handle, ep_root, BLADECENTER_SWITCH_SLOT,i);
	}
								
	for (i = 0; i < custom_handle->max_mm_supported; i++) {
		err = snmp_bc_discover_slot(handle, ep_root, BLADECENTER_SYS_MGMNT_MODULE_SLOT,i);
	}
								
	for (i = 0; i < custom_handle->max_mt_supported; i++) {
		err = snmp_bc_discover_slot(handle, ep_root, BLADECENTER_PERIPHERAL_BAY_SLOT,i);
	}
									
	for (i = 0; i < custom_handle->max_tap_supported; i++) {
		err = snmp_bc_discover_slot(handle, ep_root, BLADECENTER_ALARM_PANEL_SLOT,i);
	}

	for (i = 0; i < custom_handle->max_nc_supported; i++) {
		err = snmp_bc_discover_slot(handle, ep_root, BLADECENTER_CLOCK_SLOT,i);
	}
	
	for (i = 0; i < custom_handle->max_mx_supported; i++) {
		err = snmp_bc_discover_slot(handle, ep_root, BLADECENTER_MUX_SLOT,i);
	}
		
	return(SA_OK);							
}

/**
 * snmp_bc_discovery_slot:
 * @handler: Pointer to handler's data.
 * @ep_root: Pointer to chassis Root Entity Path which comes from openhpi.conf.
 * @entitytype: Resource type of the slot.
 * @entitylocation: Slot location of the resource.
 *
 * Discovers slot resources and their RDRs.
 *
 * Return values:
 * SA_OK - normal case.
 * SA_ERR_HPI_OUT_OF_MEMORY - Cannot allocate space for internal memory.
 * SA_ERR_HPI_INVALID_PARAMS - Pointer parameter(s) NULL.
  **/
SaErrorT snmp_bc_discover_slot( struct oh_handler_state *handle,
			         SaHpiEntityPathT *ep_root,
				 SaHpiEntityTypeT entitytype,
				 guint		  entitylocation) 
{

	SaErrorT err;
	char *comment;
        struct oh_event *e;
	struct snmp_bc_hnd *custom_handle;
	struct ResourceInfo *res_info_ptr;

	if (!handle) {
		err("Invalid parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}
		
	custom_handle = (struct snmp_bc_hnd *)handle->data;
	if (!custom_handle) {
		err("Invalid parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}

	e = NULL;
	res_info_ptr = NULL;
	
	e = snmp_bc_alloc_oh_event();
	if (e == NULL) {
		err("Out of memory.");
		return(SA_ERR_HPI_OUT_OF_MEMORY);
	}
		
	/* ---------------------------------------- */
	/* Construct .resource of struct oh_event   */
	/* ---------------------------------------- */	
	e->resource = snmp_bc_rpt_array[BC_RPT_ENTRY_PHYSICAL_SLOT].rpt;
	oh_concat_ep(&(e->resource.ResourceEntity), ep_root);
	oh_set_ep_location(&(e->resource.ResourceEntity),
			SAHPI_ENT_CHASSIS_SPECIFIC, entitylocation + SNMP_BC_HPI_LOCATION_BASE);
			
			
	switch (entitytype) {
		case SAHPI_ENT_PHYSICAL_SLOT:
			e->resource.ResourceEntity.Entry[0].EntityType = SAHPI_ENT_PHYSICAL_SLOT;
			comment = SNMP_BC_PHYSICAL_SLOT;
			break;
			
		case BLADECENTER_SWITCH_SLOT:
			e->resource.ResourceEntity.Entry[0].EntityType = BLADECENTER_SWITCH_SLOT;
			comment = SNMP_BC_SWITCH_SLOT;
			break;
			
		case BLADECENTER_POWER_SUPPLY_SLOT:
			e->resource.ResourceEntity.Entry[0].EntityType = BLADECENTER_POWER_SUPPLY_SLOT;
			comment = SNMP_BC_POWER_SUPPLY_SLOT;
			break;
			
		case BLADECENTER_PERIPHERAL_BAY_SLOT:
			e->resource.ResourceEntity.Entry[0].EntityType = BLADECENTER_PERIPHERAL_BAY_SLOT;
			comment = SNMP_BC_PERIPHERAL_BAY_SLOT;
			break;
			
		case BLADECENTER_SYS_MGMNT_MODULE_SLOT:
			e->resource.ResourceEntity.Entry[0].EntityType = BLADECENTER_SYS_MGMNT_MODULE_SLOT;
			comment = SNMP_BC_SYS_MGMNT_MODULE_SLOT;
			break;
			
		case BLADECENTER_BLOWER_SLOT:
			e->resource.ResourceEntity.Entry[0].EntityType = BLADECENTER_BLOWER_SLOT;
			comment = SNMP_BC_BLOWER_SLOT;
			break;
			
		case BLADECENTER_ALARM_PANEL_SLOT:
			e->resource.ResourceEntity.Entry[0].EntityType = BLADECENTER_ALARM_PANEL_SLOT;
			comment = SNMP_BC_ALARM_PANEL_SLOT;
			break;
			
		case BLADECENTER_CLOCK_SLOT:
			e->resource.ResourceEntity.Entry[0].EntityType = BLADECENTER_CLOCK_SLOT;
			comment = SNMP_BC_CLOCK_SLOT;
			break;
			
		case BLADECENTER_MUX_SLOT:
			e->resource.ResourceEntity.Entry[0].EntityType = BLADECENTER_MUX_SLOT;
			comment = SNMP_BC_MUX_SLOT;
			break;	
		
		default:
			err("Invalid slot resource type\n");
			return(SA_ERR_HPI_INVALID_PARAMS);
	}
	
	
	e->resource.ResourceId =
		oh_uid_from_entity_path(&(e->resource.ResourceEntity));
		
	snmp_bc_create_resourcetag(&(e->resource.ResourceTag),
				     comment,
				     entitylocation + SNMP_BC_HPI_LOCATION_BASE);
				     
	res_info_ptr = g_memdup(&(snmp_bc_rpt_array[BC_RPT_ENTRY_PHYSICAL_SLOT].res_info),
						sizeof(struct ResourceInfo));		
	if (!res_info_ptr) {
		err("Out of memory.");
		g_free(e);
		return(SA_ERR_HPI_OUT_OF_MEMORY);
	}
	
	err = oh_add_resource(handle->rptcache,
				&(e->resource),
				res_info_ptr, 0);
	if (err) { 
		err("Failed to add resource. Error=%s.", oh_lookup_error(err));
		g_free(e);
		return(err);
	}

	/* ---------------------------------------- */
	/* Construct .rdrs of struct oh_event       */
	/* ---------------------------------------- */		
	/* Find resource's rdrs: sensors, controls, etc. */		
        snmp_bc_discover_res_events(handle, &(e->resource.ResourceEntity), res_info_ptr);
        snmp_bc_discover_sensors(handle, snmp_bc_slot_sensors, e);
        snmp_bc_discover_controls(handle, snmp_bc_slot_controls, e);
        snmp_bc_discover_inventories(handle, snmp_bc_slot_inventories, e);
	
	/* ---------------------------------------- */
	/* Construct .event of struct oh_event      */	
	/* ---------------------------------------- */
	snmp_bc_set_resource_add_oh_event(e, res_info_ptr);

	/* ---------------------------------------- */
	/* Place the event in event queue           */
	/* ---------------------------------------- */		
	/*custom_handle->eventq = g_slist_append(custom_handle->eventq, e);*/
        e->hid = handle->hid;
        oh_evt_queue_push(handle->eventq, e);

	/* ---------------------------------------- */
	/* ---------------------------------------- */												
	return(SA_OK);
}

/**
 * snmp_bc_isrediscover:
 * @e: Pointer to event structure.
 *
 * Examine event and determine if this is hotswap install, remove or none
 *
 * Return values:
 * 0 - Neither hotswap install not remove.
 * SNMP_BC_RESOURCE_INSTALLED - This is a hotswap install event.
 * SNMP_BC_RESOURCE_REMOVED - This is a hotswap remove event.
 **/
guint snmp_bc_isrediscover(SaHpiEventT *working_event) 
{
	guint rediscovertype;
	  
	rediscovertype = 0; /* Default - do nothing */
	if (working_event->EventType == SAHPI_ET_HOTSWAP) {
		if (working_event->EventDataUnion.HotSwapEvent.PreviousHotSwapState == SAHPI_HS_STATE_NOT_PRESENT)
		{
			if (working_event->EventDataUnion.HotSwapEvent.HotSwapState == SAHPI_HS_STATE_NOT_PRESENT)
				err("Sanity check FAILED! PreviousHotSwapState = HotSwapState == SAHPI_HS_STATE_NOT_PRESENT\n");
			rediscovertype = SNMP_BC_RESOURCE_INSTALLED;  /* New resource is installed  */			
		}
		else if (working_event->EventDataUnion.HotSwapEvent.HotSwapState == SAHPI_HS_STATE_NOT_PRESENT)
		{ 
			if (working_event->EventDataUnion.HotSwapEvent.PreviousHotSwapState == SAHPI_HS_STATE_NOT_PRESENT)
				err("Sanity check FAILED! PreviousHotSwapState = HotSwapState == SAHPI_HS_STATE_NOT_PRESENT\n");
			rediscovertype = SNMP_BC_RESOURCE_REMOVED;  /* resource is removed  */					
		} 
	 }

	return(rediscovertype);
}

/**
 * snmp_bc_construct_blade_rpt:
 * @e: Pointer to oh_event struct.
 * @res_info_ptr: Pointer to pointer of res_info_ptr
 * @ep_root: Pointer to chassis Root Entity Path which comes from openhpi.conf.
 * @blade_index: Index of discovered blade.
 *
 * Build rpt structure for a blade resource using model data 
 *
 * Return values:
 * SA_OK - normal case.
 * SA_ERR_HPI_OUT_OF_MEMORY - Cannot allocate space for internal memory.
 * SA_ERR_HPI_INVALID_PARAMS - Pointer parameter(s) NULL.
 **/
SaErrorT snmp_bc_construct_blade_rpt(struct oh_event *e, 
				     struct ResourceInfo **res_info_ptr,
				     SaHpiEntityPathT *ep_root, 
				     guint blade_index)
{

	if (!e || !res_info_ptr) return (SA_ERR_HPI_INVALID_PARAMS);

	e->resource = snmp_bc_rpt_array[BC_RPT_ENTRY_BLADE].rpt;
	oh_concat_ep(&(e->resource.ResourceEntity), ep_root);
	oh_set_ep_location(&(e->resource.ResourceEntity),
			  SAHPI_ENT_PHYSICAL_SLOT, blade_index + SNMP_BC_HPI_LOCATION_BASE);
	oh_set_ep_location(&(e->resource.ResourceEntity),
			   SAHPI_ENT_SBC_BLADE, blade_index + SNMP_BC_HPI_LOCATION_BASE);
	e->resource.ResourceId = 
		oh_uid_from_entity_path(&(e->resource.ResourceEntity));
	*res_info_ptr = g_memdup(&(snmp_bc_rpt_array[BC_RPT_ENTRY_BLADE].res_info),
						sizeof(struct ResourceInfo));
	if (!(*res_info_ptr)) {
		err("Out of memory.");
		return(SA_ERR_HPI_OUT_OF_MEMORY);
	}
	
	return(SA_OK);	

}

/**
 * snmp_bc_construct_blower_rpt:
 * @e: Pointer to oh_event struct.
 * @res_info_ptr: Pointer to pointer of res_info_ptr
 * @ep_root: Pointer to chassis Root Entity Path which comes from openhpi.conf.
 * @blower_index: Index of discovered blower.
 *
 * Build rpt structure for a blade resource using model data 
 *
 * Return values:
 * SA_OK - normal case.
 * SA_ERR_HPI_OUT_OF_MEMORY - Cannot allocate space for internal memory.
 * SA_ERR_HPI_INVALID_PARAMS - Pointer parameter(s) NULL.
 **/
SaErrorT snmp_bc_construct_blower_rpt(struct oh_event* e, 
				      struct ResourceInfo **res_info_ptr,
				      SaHpiEntityPathT *ep_root, 
				      guint blower_index)
{

	if (!e || !res_info_ptr) return (SA_ERR_HPI_INVALID_PARAMS);
	
	e->resource = snmp_bc_rpt_array[BC_RPT_ENTRY_BLOWER_MODULE].rpt;
	oh_concat_ep(&(e->resource.ResourceEntity), ep_root);
	oh_set_ep_location(&(e->resource.ResourceEntity),
			   BLADECENTER_BLOWER_SLOT, blower_index + SNMP_BC_HPI_LOCATION_BASE);
	oh_set_ep_location(&(e->resource.ResourceEntity),
			   SAHPI_ENT_FAN, blower_index + SNMP_BC_HPI_LOCATION_BASE);
	e->resource.ResourceId = 
		oh_uid_from_entity_path(&(e->resource.ResourceEntity));
	snmp_bc_create_resourcetag(&(e->resource.ResourceTag),
				   snmp_bc_rpt_array[BC_RPT_ENTRY_BLOWER_MODULE].comment,
				   blower_index + SNMP_BC_HPI_LOCATION_BASE);

	dbg("Discovered resource=%s; ID=%d",
	    e->resource.ResourceTag.Data,
	    e->resource.ResourceId);

	/* Create platform-specific info space to add to infra-structure */
	*res_info_ptr = g_memdup(&(snmp_bc_rpt_array[BC_RPT_ENTRY_BLOWER_MODULE].res_info),
						sizeof(struct ResourceInfo));
	if (!(*res_info_ptr)) {
		err("Out of memory.");
		return(SA_ERR_HPI_OUT_OF_MEMORY);
	}
	
	return(SA_OK);

}


/**
 * snmp_bc_construct_pm_rpt:
 * @e: Pointer to oh_event struct.
 * @res_info_ptr: Pointer to pointer of res_info_ptr
 * @ep_root: Pointer to chassis Root Entity Path which comes from openhpi.conf.
 * @pm_index: Index of discovered power module.
 *
 * Build rpt structure for a power module resource using model data 
 *
 * Return values:
 * SA_OK - normal case.
 * SA_ERR_HPI_OUT_OF_MEMORY - Cannot allocate space for internal memory.
 * SA_ERR_HPI_INVALID_PARAMS - Pointer parameter(s) NULL.
 **/
SaErrorT snmp_bc_construct_pm_rpt(struct oh_event *e, 
				  struct ResourceInfo **res_info_ptr, 
				  SaHpiEntityPathT *ep_root,
				  guint pm_index)
{

	if (!e || !res_info_ptr) return(SA_ERR_HPI_INVALID_PARAMS);
	
	e->resource = snmp_bc_rpt_array[BC_RPT_ENTRY_POWER_MODULE].rpt;
	oh_concat_ep(&(e->resource.ResourceEntity), ep_root);
	oh_set_ep_location(&(e->resource.ResourceEntity),
		   BLADECENTER_POWER_SUPPLY_SLOT, pm_index + SNMP_BC_HPI_LOCATION_BASE);
	oh_set_ep_location(&(e->resource.ResourceEntity),
			   SAHPI_ENT_POWER_SUPPLY, pm_index + SNMP_BC_HPI_LOCATION_BASE);
	e->resource.ResourceId = 
			oh_uid_from_entity_path(&(e->resource.ResourceEntity));
	snmp_bc_create_resourcetag(&(e->resource.ResourceTag),
			   snmp_bc_rpt_array[BC_RPT_ENTRY_POWER_MODULE].comment,
						   pm_index + SNMP_BC_HPI_LOCATION_BASE);

	dbg("Discovered resource=%s; ID=%d",
	    e->resource.ResourceTag.Data,
	    e->resource.ResourceId);

	/* Create platform-specific info space to add to infra-structure */
	*res_info_ptr = g_memdup(&(snmp_bc_rpt_array[BC_RPT_ENTRY_POWER_MODULE].res_info),
						sizeof(struct ResourceInfo));
	if (!(*res_info_ptr)) {
		err("Out of memory.");
		return(SA_ERR_HPI_OUT_OF_MEMORY);
	}
	
	return(SA_OK);
}
/**
 * snmp_bc_construct_sm_rpt:
 * @e: Pointer to oh_event struct.
 * @res_info_ptr: Pointer to pointer of res_info_ptr
 * @ep_root: Pointer to chassis Root Entity Path which comes from openhpi.conf.
 * @sm_index: Index of discovered switch module.
 *
 * Build rpt structure for a switch module resource using model data 
 *
 * Return values:
 * SA_OK - normal case.
 * SA_ERR_HPI_OUT_OF_MEMORY - Cannot allocate space for internal memory.
 * SA_ERR_HPI_INVALID_PARAMS - Pointer parameter(s) NULL.
 **/
SaErrorT snmp_bc_construct_sm_rpt(struct oh_event *e, 
				  struct ResourceInfo **res_info_ptr,
				  SaHpiEntityPathT *ep_root, 
				  guint sm_index, 
				  char *interposer_install_mask)
{
	if (!e || !res_info_ptr) return(SA_ERR_HPI_INVALID_PARAMS);
	 

	e->resource = snmp_bc_rpt_array[BC_RPT_ENTRY_SWITCH_MODULE].rpt;
	/* Adjust entity path, if there is a switch interposer installed in this slot */
	snmp_bc_extend_ep(e, sm_index, interposer_install_mask);
	
	/*    Setting entity path for this instance        */
	/* oh_set_ep_location() does nothing if it can not */
	/* find the specified entity type in ep structure  */
	oh_concat_ep(&(e->resource.ResourceEntity), ep_root);
	oh_set_ep_location(&(e->resource.ResourceEntity),
		           BLADECENTER_SWITCH_SLOT, sm_index + SNMP_BC_HPI_LOCATION_BASE);
	oh_set_ep_location(&(e->resource.ResourceEntity),
			   SAHPI_ENT_INTERCONNECT, sm_index + SNMP_BC_HPI_LOCATION_BASE);
	oh_set_ep_location(&(e->resource.ResourceEntity),
			   SAHPI_ENT_SWITCH, sm_index + SNMP_BC_HPI_LOCATION_BASE);
	e->resource.ResourceId = 
		oh_uid_from_entity_path(&(e->resource.ResourceEntity));
	snmp_bc_create_resourcetag(&(e->resource.ResourceTag),
				   snmp_bc_rpt_array[BC_RPT_ENTRY_SWITCH_MODULE].comment,
					   sm_index + SNMP_BC_HPI_LOCATION_BASE);

	dbg("Discovered resource=%s; ID=%d",
	    e->resource.ResourceTag.Data,
	    e->resource.ResourceId);

	/* Create platform-specific info space to add to infra-structure */
	*res_info_ptr = g_memdup(&(snmp_bc_rpt_array[BC_RPT_ENTRY_SWITCH_MODULE].res_info),
						sizeof(struct ResourceInfo));
	if (!(*res_info_ptr)) {
		err("Out of memory.");
		return(SA_ERR_HPI_OUT_OF_MEMORY);
	}
	
	return(SA_OK);

} 
/**
 * snmp_bc_construct_mm_rpt:
 * @e: Pointer to oh_event struct.
 * @res_info_ptr: Pointer to pointer of res_info_ptr
 * @ep_root: Pointer to chassis Root Entity Path which comes from openhpi.conf.
 * @mm_index: Index of discovered management module.
 *
 * Build rpt structure for a management module resource using model data 
 *
 * Return values:
 * SA_OK - normal case.
 * SA_ERR_HPI_OUT_OF_MEMORY - Cannot allocate space for internal memory.
 * SA_ERR_HPI_INVALID_PARAMS - Pointer parameter(s) NULL.
 **/
SaErrorT snmp_bc_construct_mm_rpt(struct oh_event *e, 
				  struct ResourceInfo **res_info_ptr,
				  SaHpiEntityPathT *ep_root, 
				  guint mm_index, 
				  char *interposer_install_mask)
{
	if (!e || !res_info_ptr) return(SA_ERR_HPI_INVALID_PARAMS);
				
        e->resource = snmp_bc_rpt_array[BC_RPT_ENTRY_MGMNT_MODULE].rpt;
	/* Adjust entity path, if there is a switch interposer installed in this slot */
	snmp_bc_extend_ep(e, mm_index, interposer_install_mask);
	
	/*    Setting entity path for this instance        */
	/* oh_set_ep_location() does nothing if it can not */
	/* find the specified entity type in ep structure  */	
        oh_concat_ep(&(e->resource.ResourceEntity), ep_root);
        oh_set_ep_location(&(e->resource.ResourceEntity),
                BLADECENTER_SYS_MGMNT_MODULE_SLOT, mm_index + SNMP_BC_HPI_LOCATION_BASE);
	oh_set_ep_location(&(e->resource.ResourceEntity),
			   SAHPI_ENT_INTERCONNECT, mm_index + SNMP_BC_HPI_LOCATION_BASE);		
        oh_set_ep_location(&(e->resource.ResourceEntity),
                       SAHPI_ENT_SYS_MGMNT_MODULE, mm_index + SNMP_BC_HPI_LOCATION_BASE);
        e->resource.ResourceId =
             		oh_uid_from_entity_path(&(e->resource.ResourceEntity));
        snmp_bc_create_resourcetag(&(e->resource.ResourceTag),
                        snmp_bc_rpt_array[BC_RPT_ENTRY_MGMNT_MODULE].comment,
                                               mm_index + SNMP_BC_HPI_LOCATION_BASE);

        dbg("Discovered resource=%s; ID=%d",
            e->resource.ResourceTag.Data,
            e->resource.ResourceId);

        /* Create platform-specific info space to add to infra-structure */
        *res_info_ptr = g_memdup(&(snmp_bc_rpt_array[BC_RPT_ENTRY_MGMNT_MODULE].res_info),
                        	                sizeof(struct ResourceInfo));
        if (!(*res_info_ptr)) {
        	err("Out of memory.");
                return(SA_ERR_HPI_OUT_OF_MEMORY);
        }
	
	return(SA_OK);

	
}
						
/**
 * snmp_bc_add_blade_rptcache:
 * @handle: Pointer to hpi handle
 * @e: Pointer to oh_event struct.
 * @res_info_ptr: Pointer to pointer of res_info_ptr
 * @blade_index: Index of discovered blade.
 *
 * Build rpt and rdrs for a blade then add to rptcache 
 *
 * Return values:
 * SA_OK - normal case.
 * SA_ERR_HPI_OUT_OF_MEMORY - Cannot allocate space for internal memory.
 * SA_ERR_HPI_INVALID_PARAMS - Pointer parameter(s) NULL.
 **/
SaErrorT snmp_bc_add_blade_rptcache(struct oh_handler_state *handle, 
				  struct oh_event *e, 
				  struct ResourceInfo *res_info_ptr,
				  guint blade_index) 
{
	SaErrorT err;
	guint blade_width;
	guint local_retry;
	struct snmp_value get_value, get_blade_resourcetag;
	struct snmp_bc_hnd *custom_handle;


	if (!handle) {
		err("Invalid parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}
		
	custom_handle = (struct snmp_bc_hnd *)handle->data;
	if (!custom_handle) {
		err("Invalid parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}
	
        local_retry = 0;
	while(1) {
		err = snmp_bc_oid_snmp_get(custom_handle,
					    &(e->resource.ResourceEntity), 0,
						snmp_bc_rpt_array[BC_RPT_ENTRY_BLADE].OidResourceTag,
						   &get_blade_resourcetag, SAHPI_TRUE);
				
		/* If MM is busy discovering blade, we gives MM  */
		/* 3 seconds to collect blade info then try again*/
		if ( (get_blade_resourcetag.type == ASN_OCTET_STR) && 
			( g_ascii_strncasecmp(get_blade_resourcetag.string, LOG_DISCOVERING, 
								sizeof(LOG_DISCOVERING)) == 0 ) )
		{
			/* Give the AMM 12 sec max to discover this resource */
			/* Give up on this resource if AMM can not find it after 12 sec */
			if (local_retry < 4) local_retry++;
			else break;
			
			sleep(3);
		} else break;
	}
			
	snmp_bc_create_resourcetag(&(e->resource.ResourceTag),
				   snmp_bc_rpt_array[BC_RPT_ENTRY_BLADE].comment,
				   blade_index + SNMP_BC_HPI_LOCATION_BASE);
			
	/* Tack on MM's defined blade name */
	if (!err && (get_blade_resourcetag.type == ASN_OCTET_STR)) {
		oh_append_textbuffer(&(e->resource.ResourceTag), " - ");
		oh_append_textbuffer(&(e->resource.ResourceTag), get_blade_resourcetag.string);
	}

	dbg("Discovered resource=%s; ID=%d", 
	    e->resource.ResourceTag.Data,
	    e->resource.ResourceId);

	/* Create platform-specific info space to add to infra-structure */
	
	/* Determine and set current resource state  */			
	res_info_ptr->cur_state = SAHPI_HS_STATE_ACTIVE;  /* Default to ACTIVE */
	if (res_info_ptr->mib.OidPowerState != NULL) {
		/* Read power state of resource */
		err = snmp_bc_oid_snmp_get(custom_handle, &(e->resource.ResourceEntity), 0,
				   	res_info_ptr->mib.OidPowerState, &get_value, SAHPI_TRUE);
		if (!err && (get_value.type == ASN_INTEGER)) {
			if (get_value.integer == 0)   /*  state = SAHPI_POWER_OFF */
					res_info_ptr->cur_state = SAHPI_HS_STATE_INACTIVE;
		}
	}	
	
        /* Get UUID and convert to GUID */
        err = snmp_bc_get_guid(custom_handle, e, res_info_ptr);

	/* Add resource to resource */
	err = oh_add_resource(handle->rptcache, 
				&(e->resource),
				res_info_ptr, 0);
	if (err) {
		err("Failed to add resource. Error=%s.", oh_lookup_error(err));
		return(err);
	}
			
	/* ---------------------------------------- */
	/* Construct .rdrs of struct oh_event       */
	/* ---------------------------------------- */								
	/* Find resource's events, sensors, controls, etc. */
	snmp_bc_discover_res_events(handle, &(e->resource.ResourceEntity), res_info_ptr);
	snmp_bc_discover_sensors(handle, snmp_bc_blade_sensors, e);
	snmp_bc_discover_ipmi_sensors(handle, snmp_bc_blade_ipmi_sensors, e);
	snmp_bc_discover_controls(handle, snmp_bc_blade_controls, e);
	snmp_bc_discover_inventories(handle, snmp_bc_blade_inventories, e);
			
	blade_width = 1;    /* Default to 1-wide blade */
	if (res_info_ptr->mib.OidResourceWidth != NULL) {
		err = snmp_bc_oid_snmp_get(custom_handle,  &(e->resource.ResourceEntity), 0,
		   		res_info_ptr->mib.OidResourceWidth, &get_value, SAHPI_TRUE);
		if (!err && (get_value.type == ASN_INTEGER)) {
			blade_width = get_value.integer;
		}
	}			
	res_info_ptr->resourcewidth = blade_width;
	err = snmp_bc_set_resource_slot_state_sensor(handle, e, blade_width);

	/********************************** 
	 * Discover Blade Expansion Modules
	 * err = snmp_bc_discover_blade_expansion(handle, 
	 *			                  ep_root, 
	 *                                        blade_index);
	 **********************************/

	return(err);
}

/**
 * snmp_bc_add_blower_rptcache:
 * @handle: Pointer to hpi handle
 * @e: Pointer to oh_event struct.
 * @res_info_ptr: Pointer to pointer of res_info_ptr
 * @blower_index: Index of discovered blower.
 *
 * Build rpt and rdrs for a blower then add to rptcache 
 *
 * Return values:
 * SA_OK - normal case.
 * SA_ERR_HPI_OUT_OF_MEMORY - Cannot allocate space for internal memory.
 * SA_ERR_HPI_INVALID_PARAMS - Pointer parameter(s) NULL.
 **/
SaErrorT snmp_bc_add_blower_rptcache(struct oh_handler_state *handle, 
				  struct oh_event *e, 
				  struct ResourceInfo *res_info_ptr,
				  guint blower_index) 

{
	SaErrorT err;
	guint blower_width;
	struct snmp_value get_value;
	struct snmp_bc_hnd *custom_handle;


	if (!handle || !e || !res_info_ptr) {
		err("Invalid parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}
		
	custom_handle = (struct snmp_bc_hnd *)handle->data;
	if (!custom_handle) {
		err("Invalid parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}

	dbg("Discovering blower %d resource.\n", blower_index);
	res_info_ptr->cur_state = SAHPI_HS_STATE_ACTIVE;

        /* Get UUID and convert to GUID */
        err = snmp_bc_get_guid(custom_handle, e, res_info_ptr);

	/* Add resource to temporary event cache/queue */
	err = oh_add_resource(handle->rptcache, 
				      &(e->resource),
				      res_info_ptr, 0);
	if (err) {
		err("Failed to add resource. Error=%s.", oh_lookup_error(err));
		return(err);
	}
			
	/* ---------------------------------------- */
	/* Construct .rdrs of struct oh_event       */
	/* ---------------------------------------- */								
	/* Find resource's events, sensors, controls, etc. */
	snmp_bc_discover_res_events(handle, &(e->resource.ResourceEntity), res_info_ptr);
	snmp_bc_discover_sensors(handle, snmp_bc_blower_sensors, e);
	if ( (custom_handle->platform == SNMP_BC_PLATFORM_BCH) ||
	     (custom_handle->platform == SNMP_BC_PLATFORM_BCHT) ){
		snmp_bc_discover_sensors(handle, snmp_bc_blower_sensors_bch, e);	
	}
	snmp_bc_discover_controls(handle, snmp_bc_blower_controls, e);
	snmp_bc_discover_inventories(handle, snmp_bc_blower_inventories, e);
		
	blower_width = 1;    /* Default to 1-wide blade */
	if (res_info_ptr->mib.OidResourceWidth != NULL) {
		err = snmp_bc_oid_snmp_get(custom_handle,  &(e->resource.ResourceEntity), 0,
		   		res_info_ptr->mib.OidResourceWidth, &get_value, SAHPI_TRUE);
		if (!err && (get_value.type == ASN_INTEGER)) {
			blower_width = get_value.integer;
		}
	}			

	res_info_ptr->resourcewidth = blower_width;
	err = snmp_bc_set_resource_slot_state_sensor(handle, e, blower_width);
	return(err);
}

/**
 * snmp_bc_add_pm_rptcache:
 * @handle: Pointer to hpi handle
 * @e: Pointer to oh_event struct.
 * @res_info_ptr: Pointer to pointer of res_info_ptr
 * @pm_index: Index of discovered power module.
 *
 * Build rpt and rdrs for a power module then add to rptcache 
 *
 * Return values:
 * SA_OK - normal case.
 * SA_ERR_HPI_OUT_OF_MEMORY - Cannot allocate space for internal memory.
 * SA_ERR_HPI_INVALID_PARAMS - Pointer parameter(s) NULL.
 **/
SaErrorT snmp_bc_add_pm_rptcache(struct oh_handler_state *handle, 
				  struct oh_event *e, 
				  struct ResourceInfo *res_info_ptr,
				  guint pm_index)
{
	SaErrorT err;
	guint pm_width;
	struct snmp_value get_value;
	struct snmp_bc_hnd *custom_handle;

	if (!handle || !e || !res_info_ptr) {
		err("Invalid parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}
		
	custom_handle = (struct snmp_bc_hnd *)handle->data;
	if (!custom_handle) {
		err("Invalid parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}

	dbg("Discovering power module %d resource.\n", pm_index);


	res_info_ptr->cur_state = SAHPI_HS_STATE_ACTIVE;

        /* Get UUID and convert to GUID */
        err = snmp_bc_get_guid(custom_handle, e, res_info_ptr);

	/* Add resource to temporary event cache/queue */
	err = oh_add_resource(handle->rptcache, 
				      &(e->resource),
				      res_info_ptr, 0);
	if (err) {
		err("Failed to add resource. Error=%s.", oh_lookup_error(err));
		return(err);
	}
	/* ---------------------------------------- */
	/* Construct .rdrs of struct oh_event       */
	/* ---------------------------------------- */
	/* Find resource's events, sensors, controls, etc. */
	snmp_bc_discover_res_events(handle, &(e->resource.ResourceEntity), res_info_ptr);
	snmp_bc_discover_sensors(handle, snmp_bc_power_sensors, e);

	if ( (custom_handle->platform == SNMP_BC_PLATFORM_BCH) ||
	     (custom_handle->platform == SNMP_BC_PLATFORM_BCHT) ){
		snmp_bc_discover_sensors(handle, snmp_bc_power_sensors_bch, e);	
	}

	snmp_bc_discover_controls(handle, snmp_bc_power_controls, e);
	snmp_bc_discover_inventories(handle, snmp_bc_power_inventories, e);
			
	pm_width = 1;    /* Default to 1-wide */
	if (res_info_ptr->mib.OidResourceWidth != NULL) {
		err = snmp_bc_oid_snmp_get(custom_handle,  &(e->resource.ResourceEntity), 0,
				   	res_info_ptr->mib.OidResourceWidth, &get_value, SAHPI_TRUE);
		if (!err && (get_value.type == ASN_INTEGER)) {
			pm_width = get_value.integer;
		}
	}			

	res_info_ptr->resourcewidth = pm_width;
	err = snmp_bc_set_resource_slot_state_sensor(handle, e, pm_width);
	return(err);
}

/**
 * snmp_bc_add_switch_rptcache:
 * @handle: Pointer to hpi handle
 * @e: Pointer to oh_event struct.
 * @res_info_ptr: Pointer to pointer of res_info_ptr
 * @switch_index: Index of discovered power module.
 *
 * Build rpt and rdrs for a switch module then add to rptcache 
 *
 * Return values:
 * SA_OK - normal case.
 * SA_ERR_HPI_OUT_OF_MEMORY - Cannot allocate space for internal memory.
 * SA_ERR_HPI_INVALID_PARAMS - Pointer parameter(s) NULL.
 **/
SaErrorT snmp_bc_add_switch_rptcache(struct oh_handler_state *handle, 
				  struct oh_event *e, 
				  struct ResourceInfo *res_info_ptr,
				  guint switch_index)
{
	SaErrorT err;
	guint sw_width;
	struct snmp_value get_value;
	struct snmp_bc_hnd *custom_handle;

	if (!handle || !e || !res_info_ptr) {
		err("Invalid parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}
		
	custom_handle = (struct snmp_bc_hnd *)handle->data;
	if (!custom_handle) {
		err("Invalid parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}

	dbg("Discovering switch module %d resource.\n", switch_index);

	res_info_ptr->cur_state = SAHPI_HS_STATE_ACTIVE;  /* Default to ACTIVE */
	if (res_info_ptr->mib.OidPowerState != NULL) {
		/* Read power state of resource */
		err = snmp_bc_oid_snmp_get(custom_handle,  &(e->resource.ResourceEntity), 0,
			   		res_info_ptr->mib.OidPowerState, &get_value, SAHPI_TRUE);
		if (!err && (get_value.type == ASN_INTEGER)) {
			if (get_value.integer == 0)   /*  state = SAHPI_POWER_OFF */
				res_info_ptr->cur_state = SAHPI_HS_STATE_INACTIVE;
		}
	}

        /* Get UUID and convert to GUID */
        err = snmp_bc_get_guid(custom_handle, e, res_info_ptr);

	/* Add resource to temporary event cache  */
	err = oh_add_resource(handle->rptcache, 
				      &(e->resource),
				      res_info_ptr, 0);
	if (err) {
		err("Failed to add resource. Error=%s.", oh_lookup_error(err));
		return(err);
	}
	
	/* ---------------------------------------- */
	/* Construct .rdrs of struct oh_event       */
	/* ---------------------------------------- */		
	/* Find resource's events, sensors, controls, etc. */
	snmp_bc_discover_res_events(handle, &(e->resource.ResourceEntity), res_info_ptr);
	snmp_bc_discover_sensors(handle, snmp_bc_switch_sensors, e);
	snmp_bc_discover_controls(handle, snmp_bc_switch_controls, e);
	snmp_bc_discover_inventories(handle, snmp_bc_switch_inventories, e);
	
	sw_width = 1;    /* Default to 1-wide */
	if (res_info_ptr->mib.OidResourceWidth != NULL) {
		err = snmp_bc_oid_snmp_get(custom_handle,  &(e->resource.ResourceEntity), 0,
			   		res_info_ptr->mib.OidResourceWidth, &get_value, SAHPI_TRUE);
		if (!err && (get_value.type == ASN_INTEGER)) {
			sw_width = get_value.integer;
		}
	}			
	res_info_ptr->resourcewidth = sw_width;
	err = snmp_bc_set_resource_slot_state_sensor(handle, e, sw_width);
	return(err);			
}

/**
 * snmp_bc_add_mm_rptcache:
 * @handle: Pointer to hpi handle
 * @e: Pointer to oh_event struct.
 * @res_info_ptr: Pointer to pointer of res_info_ptr
 * @mm_index: Index of discovered management module.
 *
 * Build rpt and rdrs for a management module then add to rptcache 
 *
 * Return values:
 * SA_OK - normal case.
 * SA_ERR_HPI_OUT_OF_MEMORY - Cannot allocate space for internal memory.
 * SA_ERR_HPI_INVALID_PARAMS - Pointer parameter(s) NULL.
 **/
SaErrorT snmp_bc_add_mm_rptcache(struct oh_handler_state *handle, 
				  struct oh_event *e, 
				  struct ResourceInfo *res_info_ptr,
				  guint mm_index)
{
	SaErrorT err;
	guint mm_width;
	struct snmp_value get_value, get_active;
	struct snmp_bc_hnd *custom_handle;

	if (!handle || !e || !res_info_ptr) {
		err("Invalid parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}
		
	custom_handle = (struct snmp_bc_hnd *)handle->data;
	if (!custom_handle) {
		err("Invalid parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}

	dbg("Discovering management module %d resource.\n", mm_index);
	err = snmp_bc_snmp_get(custom_handle, SNMP_BC_MGMNT_ACTIVE, &get_active, SAHPI_TRUE);
	if (err || get_active.type != ASN_INTEGER) {
		err("Cannot get OID=%s; Received Type=%d; Error=%s.",
	      		SNMP_BC_MGMNT_ACTIVE, get_active.type, oh_lookup_error(err));
		if (err) { return(err); }
		else { return(SA_ERR_HPI_INTERNAL_ERROR); }
	}
			
        /* Set active MM location in handler's custom data  */
	/* - used to override duplicate MM events in snmp_bc_event.c */
        custom_handle->active_mm = get_active.integer;
	if (custom_handle->active_mm == mm_index+1) 
              	res_info_ptr->cur_state = SAHPI_HS_STATE_ACTIVE;
	else 
		res_info_ptr->cur_state = SAHPI_HS_STATE_INACTIVE;
				
        /* Get UUID and convert to GUID */
        err = snmp_bc_get_guid(custom_handle, e, res_info_ptr);

        /* Add resource to resource */
        err = oh_add_resource(handle->rptcache,
                             &(e->resource),
                               res_info_ptr, 0);
        if (err) {
        	err("Failed to add resource. Error=%s.", oh_lookup_error(err));
                return(err);
        }
                
         /* Find resource's events, sensors, controls, etc. */
         snmp_bc_discover_res_events(handle, &(e->resource.ResourceEntity), res_info_ptr);
			
	/* ---------------------------------------- */
	/* Construct .rdrs of struct oh_event       */
	/* ---------------------------------------- */						
			
	/* See if mmHeathState OID is supported; If it is a readable Operational State sensor
           is supported; else the Operational State sensor is event only */
	{
		struct snmp_value value;

		err = snmp_bc_snmp_get(custom_handle, SNMP_BC_MM_HEALTH_OID, &value, SAHPI_TRUE);
		if (err) {  
			snmp_bc_discover_sensors(handle, snmp_bc_mgmnt_sensors, e);
		}
		else { 
			snmp_bc_discover_sensors(handle, snmp_bc_mgmnt_health_sensors, e);
		}
	}

        snmp_bc_discover_controls(handle, snmp_bc_mgmnt_controls, e);
	snmp_bc_discover_inventories(handle, snmp_bc_mgmnt_inventories, e);

	mm_width = 1;    /* Default to 1-wide */
	if (res_info_ptr->mib.OidResourceWidth != NULL) {
		err = snmp_bc_oid_snmp_get(custom_handle,  &(e->resource.ResourceEntity), 0,
					   res_info_ptr->mib.OidResourceWidth, &get_value, SAHPI_TRUE);
		if (!err && (get_value.type == ASN_INTEGER)) {
			mm_width = get_value.integer;
		}
	}
	
	res_info_ptr->resourcewidth = mm_width;							
	err = snmp_bc_set_resource_slot_state_sensor(handle, e, mm_width);
	return(err);
}					

/**
 * snmp_bc_discover_blade_i:
 * @handle: Pointer to hpi handle
 * @ep_root: Pointer to .
 * @blade_index: Index of discovered blade.
 *
 * Discover a particular blade at index blade_index.
 * This routine is used to rediscover a blade. 
 * Blade rpt and rdrs will be added to rptcache.
 * No event will be generated.  The event is handled separately in log2event.
 *
 * Return values:
 * SA_OK - normal case.
 * SA_ERR_HPI_OUT_OF_MEMORY - Cannot allocate space for internal memory.
 * SA_ERR_HPI_INVALID_PARAMS - Pointer parameter(s) NULL.
 **/
SaErrorT snmp_bc_discover_blade_i(struct oh_handler_state *handle,
			  	  SaHpiEntityPathT *ep_root, 
				  guint blade_index)
{

	SaErrorT err;
        struct oh_event *e;
	struct ResourceInfo *res_info_ptr;

	if (!handle) {
		err("Invalid parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}
		
	e = NULL;
	res_info_ptr = NULL;	

	e = snmp_bc_alloc_oh_event();
	if (e == NULL) {
		err("Out of memory.");
		return(SA_ERR_HPI_OUT_OF_MEMORY);
	}
									
	/* ---------------------------------------- */
	/* Construct .resource of struct oh_event   */
	/* ---------------------------------------- */	
	err = snmp_bc_construct_blade_rpt(e, &res_info_ptr, ep_root, blade_index);
	if (err) {
		snmp_bc_free_oh_event(e);
		return(err);
	}
	/* ---------------------------------------- */
	/* Discover rdrs.                           */
	/* Add rpt and rdrs to rptcache.            */
	/* ---------------------------------------- */				
	err = snmp_bc_add_blade_rptcache(handle, e, res_info_ptr, blade_index);				
	snmp_bc_free_oh_event(e);
	
	/********************************** 
	 * Discover Blade Expansion Modules
	 **********************************/
	err = snmp_bc_discover_blade_expansion(handle, ep_root, blade_index);

	return(SA_OK);

}

/**
 * snmp_bc_discover_blower_i:
 * @handle: Pointer to hpi handle
 * @ep_root: Pointer to .
 * @blower_index: Index of discovered blower.
 *
 * Discover a particular blower at index blower_index.
 * This routine is used to rediscover a blower. 
 * Blower rpt and rdrs will be added to rptcache.
 * No event will be generated.  The event is handled separately in log2event.
 *
 * Return values:
 * SA_OK - normal case.
 * SA_ERR_HPI_OUT_OF_MEMORY - Cannot allocate space for internal memory.
 * SA_ERR_HPI_INVALID_PARAMS - Pointer parameter(s) NULL.
 **/
SaErrorT snmp_bc_discover_blower_i(struct oh_handler_state *handle,
			  	   SaHpiEntityPathT *ep_root, 
				   guint blower_index)
{

	SaErrorT err;
        struct oh_event *e;
	struct ResourceInfo *res_info_ptr;

	if (!handle) {
		err("Invalid parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}
		
	e= NULL;
	res_info_ptr = NULL;
	
	e = snmp_bc_alloc_oh_event();
	if (e == NULL) {
		err("Out of memory.");
		return(SA_ERR_HPI_OUT_OF_MEMORY);
	}
									
	/* ---------------------------------------- */
	/* Construct .resource of struct oh_event   */
	/* ---------------------------------------- */	
	err = snmp_bc_construct_blower_rpt(e, &res_info_ptr, ep_root, blower_index);
	if (err) {
		snmp_bc_free_oh_event(e);
		return(err);
	}
	
	/* ---------------------------------------- */
	/* Discover rdrs.                           */
	/* Add rpt and rdrs to rptcache.            */
	/* ---------------------------------------- */				
	err = snmp_bc_add_blower_rptcache(handle, e, res_info_ptr, blower_index); 
			
	snmp_bc_free_oh_event(e);
			
	return(SA_OK);
}

/**
 * snmp_bc_discover_pm_i:
 * @handle: Pointer to hpi handle
 * @ep_root: Pointer to .
 * @pm_index: Index of discovered power module.
 *
 * Discover a particular power module at index pm_index.
 * This routine is used to rediscover a power module. 
 * Power module rpt and rdrs will be added to rptcache.
 * No event will be generated.  The event is handled separately in log2event.
 *
 * Return values:
 * SA_OK - normal case.
 * SA_ERR_HPI_OUT_OF_MEMORY - Cannot allocate space for internal memory.
 * SA_ERR_HPI_INVALID_PARAMS - Pointer parameter(s) NULL.
 **/
SaErrorT snmp_bc_discover_pm_i(struct oh_handler_state *handle,
			       SaHpiEntityPathT *ep_root, 
			       guint pm_index)
{


	SaErrorT err;
        struct oh_event *e;
	struct ResourceInfo *res_info_ptr;

	if (!handle) {
		err("Invalid parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}
		
	e = NULL;
	res_info_ptr = NULL;
	
	e = snmp_bc_alloc_oh_event();
	if (e == NULL) {
		err("Out of memory.");
		return(SA_ERR_HPI_OUT_OF_MEMORY);
	}

									
	/* ---------------------------------------- */
	/* Construct .resource of struct oh_event   */
	/* ---------------------------------------- */	
	err = snmp_bc_construct_pm_rpt(e, &res_info_ptr, ep_root, pm_index);
	if (err) {
		snmp_bc_free_oh_event(e);
		return(err);
	}
	
	/* ---------------------------------------- */
	/* Discover rdrs.                           */
	/* Add rpt and rdrs to rptcache.            */
	/* ---------------------------------------- */		
	err = snmp_bc_add_pm_rptcache(handle, e, res_info_ptr, pm_index); 

	snmp_bc_free_oh_event(e);	
	return(SA_OK);
}

/**
 * snmp_bc_discover_switch_i:
 * @handle: Pointer to hpi handle
 * @ep_root: Pointer to .
 * @sm_index: Index of discovered switch module.
 *
 * Discover a particular switch module at index sm_index.
 * This routine is used to rediscover a switch module. 
 * Switch module rpt and rdrs will be added to rptcache.
 * No event will be generated.  The event is handled separately in log2event.
 *
 * Return values:
 * SA_OK - normal case.
 * SA_ERR_HPI_OUT_OF_MEMORY - Cannot allocate space for internal memory.
 * SA_ERR_HPI_INVALID_PARAMS - Pointer parameter(s) NULL.
 **/
SaErrorT snmp_bc_discover_switch_i(struct oh_handler_state *handle,
			  	   SaHpiEntityPathT *ep_root, 
				   guint sm_index)
{

	SaErrorT err;
        struct oh_event *e;
	struct ResourceInfo *res_info_ptr;
	struct snmp_bc_hnd *custom_handle;
	struct snmp_value get_value;	
	
	if (!handle) {
		err("Invalid parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}

	custom_handle = (struct snmp_bc_hnd *)handle->data;
	if (!custom_handle) {
		err("Invalid parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}

	e = NULL;
	res_info_ptr = NULL;
	
	e = snmp_bc_alloc_oh_event();
	if (e == NULL) {
		err("Out of memory.");
		return(SA_ERR_HPI_OUT_OF_MEMORY);
	}
								
	/* ---------------------------------------- */
	/* Construct .resource of struct oh_event   */
	/* ---------------------------------------- */	
	
	/* Fetch switch interposer-card installed vector  */
	/* Have to fetch from hardware in case we have yet to rediscover the SMI */	
	get_installed_mask(SNMP_BC_SMI_INSTALLED, get_value);
	
	err = snmp_bc_construct_sm_rpt(e, &res_info_ptr, ep_root, sm_index, get_value.string);

	if (err) {
		snmp_bc_free_oh_event(e);
		return(err);
	}		
	
	/* ---------------------------------------- */
	/* Discover rdrs.                           */
	/* Add rpt and rdrs to rptcache.            */
	/* ---------------------------------------- */			
	err = snmp_bc_add_switch_rptcache(handle, e, res_info_ptr, sm_index); 
			
	snmp_bc_free_oh_event(e);
	return(SA_OK);
}
 
/**
 * snmp_bc_discover_mm_i:
 * @handle: Pointer to hpi handle
 * @ep_root: Pointer to .
 * @mm_index: Index of discovered management module.
 *
 * Discover a particular management module at index mm_index.
 * This routine is used to rediscover a management module. 
 * Management module rpt and rdrs will be added to rptcache.
 * No event will be generated.  The event is handled separately in log2event.
 *
 * Return values:
 * SA_OK - normal case.
 * SA_ERR_HPI_OUT_OF_MEMORY - Cannot allocate space for internal memory.
 * SA_ERR_HPI_INVALID_PARAMS - Pointer parameter(s) NULL.
 **/
SaErrorT snmp_bc_discover_mm_i(struct oh_handler_state *handle,
			  	SaHpiEntityPathT *ep_root, 
				guint mm_index)
{

	SaErrorT err;
        struct oh_event *e;
	struct ResourceInfo *res_info_ptr;
	struct snmp_bc_hnd *custom_handle;	
	struct snmp_value get_value;
	
	if (!handle) {
		err("Invalid parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}

	custom_handle = (struct snmp_bc_hnd *)handle->data;
	if (!custom_handle) {
		err("Invalid parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}
		
	e = NULL;
	res_info_ptr = NULL;
	e = snmp_bc_alloc_oh_event();
	if (e == NULL) {
		err("Out of memory.");
		return(SA_ERR_HPI_OUT_OF_MEMORY);
	}
									
	/* ---------------------------------------- */
	/* Construct .resource of struct oh_event   */
	/* ---------------------------------------- */
	
	
	/* Fetch mm interposer-card installed vector  */
	/* Have to fetch from hardware in case we have yet to rediscover the MMI */
	get_installed_mask(SNMP_BC_MMI_INSTALLED, get_value);

	err = snmp_bc_construct_mm_rpt(e, &res_info_ptr, ep_root, mm_index, get_value.string);

	if (err) {
		snmp_bc_free_oh_event(e);
		return(err);
	}
	
	/* ---------------------------------------- */
	/* Discover rdrs.                           */
	/* Add rpt and rdrs to rptcache.            */
	/* ---------------------------------------- */			
	err = snmp_bc_add_mm_rptcache(handle, e, res_info_ptr, mm_index); 

	snmp_bc_free_oh_event(e);
	return(SA_OK);
}

/**
 * snmp_bc_fetch_MT_install_mask:
 * @handle: Pointer to hpi handle
 * @getintvalue: Pointer to a struct snmp_value
 *
 * The MM presents MediaTray Installed Mask in three different ways, 
 * depending on the code level.  
 *
 * Older code: chassisMTInstalled is an integer.
 *             It is either 0 (no media tray) or 1 (one media tray)
 *
 * Intermediate Code: chassisMTInstalled is an OCT_STR bitmap of installed mediatray
 *
 * Later Code: chassisMTInstalled is an integer. 0 (no media tray), 1(yes media tray)
 *  		chassisMTInstalled is qualified by chassisNoOfMTsInstalled, a media tray installed bitmap
 *
 * This module is designed to work with any of the Media Tray repesentation above
 *
 * Return values:
 * SA_OK - normal case.
 * SA_ERR_HPI_INVALID_PARAMS - Pointer parameter(s) NULL.
 **/
SaErrorT snmp_bc_fetch_MT_install_mask(struct oh_handler_state *handle, 
			               struct snmp_value *getintvalue)
{

	SaErrorT err; 	
	struct snmp_value get_value, get_value2;
	struct snmp_bc_hnd *custom_handle;	
	
	if (!handle) {
		err("Invalid parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}

	custom_handle = (struct snmp_bc_hnd *)handle->data;
	if (!custom_handle) {
		err("Invalid parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}
	
	
	err = SA_OK;
	getintvalue->type = ASN_INTEGER;
	
	err = snmp_bc_snmp_get(custom_handle, SNMP_BC_MT_INSTALLED, &get_value, SAHPI_TRUE);
	if (err == SA_ERR_HPI_NOT_PRESENT) {getintvalue->type = ASN_INTEGER; getintvalue->integer = 0;}
	
        else if (err != SA_OK) {
		dbg("Cannot get OID=%s; Received Type=%d; Error=%s.",
		    SNMP_BC_MT_INSTALLED, get_value.type, oh_lookup_error(err));
		if (err) { return(err); }
		else { return(SA_ERR_HPI_INTERNAL_ERROR); }
	} else { 
        	if (get_value.type == ASN_OCTET_STR) {
			getintvalue->integer = atoi(get_value.string);
        	} else if (get_value.type == ASN_INTEGER) {
			if (get_value.integer == 0) { 
				getintvalue->integer = 0;
			} else {
				err = snmp_bc_snmp_get(custom_handle, SNMP_BC_NOS_MT_INSTALLED, &get_value2, SAHPI_TRUE);
				if (err == SA_ERR_HPI_NOT_PRESENT) { 
					getintvalue->integer = get_value.integer;
					if (getintvalue->integer == 1) getintvalue->integer = 10;
				} else if (err != SA_OK) {
					if (err) { return(err); }
					else { return(SA_ERR_HPI_INTERNAL_ERROR); }
				} else {
					if (get_value2.type == ASN_OCTET_STR) 
					     getintvalue->integer = atoi(get_value2.string);
					else 
					     getintvalue->integer = 0;
				}				
			}
			
		}
	}
	return(err);
}

