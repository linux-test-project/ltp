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
 *      Steve Sherman <stevees@us.ibm.com>
 */

/**************************************************************************
 * This source file defines the resource arrays declared in sim_resources.h
 *************************************************************************/

#include <glib.h>
#include <snmp_util.h>
#include <sim_resources.h>

struct snmp_bc_data sim_resource_array[] = {
        {
		/* DATETIME */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.4.4.1.0",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "12/25/2003,06:30:00",
			},
		},
	},
	/* Add more example event log messages */
        {
		/* Event Log Index Number */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.3.4.2.1.1.1",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 1,
			},
		},
	},
        {
		/* Event Log Index Number */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.3.4.2.1.1.2",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 1,
			},
		},
	},
        {
		/* Event Log Message */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.3.4.2.1.2.1",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "Severity:INFO  Source:SERVPROC  Name:WMN315702424  Date:10/11/03  Time:09:09:48  Text:Remote Login Successful. Login ID:'(SNMP Manager at IP@=192.168.64.5 authenticated).'",
			},
		},
	},
        {
		/* Event Log Message */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.3.4.2.1.2.2",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "Severity:INFO  Source:SERVPROC  Name:WMN315702424  Date:10/11/03  Time:09:09:48  Text:Remote Login Successful. Login ID:'(SNMP Manager at IP@=192.168.64.5 authenticated).'",
			},
		},
	},
        {
		/* Clear Event Log */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.3.4.3",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 1, /* write-only */
			},
		},
	},
        {
		/* SNMP_BC_BLADE_VECTOR */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.5.2.49.0",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "11111111111111",
			},
		},
	},
	/* If there ever is a Add-in vector added to SNMP, have to add these below to DASD 1 temperature */
        {
		/* SNMP_BC_BLADE_ADDIN_VECTOR Blade 1 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.3.1.8.1",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = " 199.99 Centigrade",
			},
		},
	},
        {
		/* SNMP_BC_BLADE_ADDIN_VECTOR Blade 2 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.3.1.8.2",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "   9.99 Centigrade",
			},
		},
	},
        {
		/* SNMP_BC_BLADE_ADDIN_VECTOR Blade 3 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.3.1.8.3",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "-199.99 Centigrade",
			},
		},
	},
        {
		/* SNMP_BC_BLADE_ADDIN_VECTOR Blade 4 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.3.1.8.4",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "-  9.99 Centigrade",
			},
		},
	},
        {
		/* SNMP_BC_BLADE_ADDIN_VECTOR Blade 5 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.3.1.8.5",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "   0.00 Centigrade",
			},
		},
	},
        {
		/* SNMP_BC_BLADE_ADDIN_VECTOR Blade 6 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.3.1.8.6",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "-  0.00 Centigrade",
			},
		},
	},
        {
		/* SNMP_BC_BLADE_ADDIN_VECTOR Blade 7 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.3.1.8.7",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "Not Readable!",
			},
		},
	},
        {
		/* SNMP_BC_BLADE_ADDIN_VECTOR Blade 8 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.3.1.8.8",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "Not Readable!",
			},
		},
	},
        {
		/* SNMP_BC_BLADE_ADDIN_VECTOR Blade 9 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.3.1.8.9",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "Not Readable!",
			},
		},
	},
        {
		/* SNMP_BC_BLADE_ADDIN_VECTOR Blade 10 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.3.1.8.10",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "Not Readable!",
			},
		},
	},
        {
		/* SNMP_BC_BLADE_ADDIN_VECTOR Blade 11 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.3.1.8.11",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "Not Readable!",
			},
		},
	},
        {
		/* SNMP_BC_BLADE_ADDIN_VECTOR Blade 12 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.3.1.8.12",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "Not Readable!",
			},
		},
	},
        {
		/* SNMP_BC_BLADE_ADDIN_VECTOR Blade 13 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.3.1.8.13",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "Not Readable!",
			},
		},
	},
        {
		/* SNMP_BC_BLADE_ADDIN_VECTOR Blade 14 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.3.1.8.14",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "Not Readable!",
			},
		},
	},
        {
		/* SNMP_BC_FAN_VECTOR */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.5.2.73.0",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "1111",
			},
		},
	},
        {
		/* SNMP_BC_MGMNT_VECTOR */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.4.30.0",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "11",
			},
		},
	},
        {
		/* SNMP_BC_MGMNT_ACTIVE */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.4.34.0",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 1, /* number of active MM; 0 if none ??? */
			},
		},
	},
        {
		/* SNMP_BC_MEDIATRAY_EXISTS */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.5.2.74.0",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 1, /* 0=no; 1=yes */
			},
		},
	},
        {
		/* SNMP_BC_POWER_VECTOR */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.5.2.89.0",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "1111",
			},
		},
	},
        {
		/* SNMP_BC_SWITCH_VECTOR */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.5.2.113.0",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "1111",
			},
		},
	},
        {
		/*  Management module - Reset */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.7.4.0", /* write-only */
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 1, /* 1=execute */
			},
		},
 	},
	{
		/* Switch Reset - Switch 1 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.3.1.1.1.8.1", /* write-only */
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 1, /* 1=execute */
			},
		},
	},
	{
		/* Switch Reset - Switch 2 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.3.1.1.1.8.2", /* write-only */
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 1, /* 1=execute */
			},
		},
	},
	{
		/* Switch Reset - Switch 3 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.3.1.1.1.8.3", /* write-only */
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 1, /* 1=execute */
			},
		},
	},
	{
		/* Switch Reset - Switch 4 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.3.1.1.1.8.4", /* write-only */
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 1, /* 1=execute */
			},
		},
	},
	{
		/* Switch Power State - Switch 1 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.3.1.1.1.7.1",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 1, /* poweroff=0; poweron=1 */
			},
		},
	},
	{
		/* Switch Power State - Switch 2 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.3.1.1.1.7.2",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 1, /* poweroff=0; poweron=1 */
			},
		},
	},
	{
		/* Switch Power State - Switch 3 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.3.1.1.1.7.3",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 1, /* poweroff=0; poweron=1 */
			},
		},
	},
	{
		/* Switch Power State - Switch 4 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.3.1.1.1.7.4",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 1, /* poweroff=0; poweron=1 */
			},
		},
	},
#if 0 
/* Simulator currently checks for duplicate OIDs - same as Power State */
 	{
		/* Switch Power On/Off - Switch 1 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.3.1.1.1.7.1",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 1, /* poweroff=0; poweron=1 */
			},
		},
	},
	{
		/* Switch Power On/Off - Switch 2 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.3.1.1.1.7.2",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 1, /* poweroff=0; poweron=1 */
			},
		},
	},
	{
		/* Switch Power On/Off - Switch 3 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.3.1.1.1.7.3",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 1, /* poweroff=0; poweron=1 */
			},
		},
	},
	{
		/* Switch Power On/Off - Switch 4 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.3.1.1.1.7.4",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 1, /* poweroff=0; poweron=1 */
			},
		},
	},
#endif
	{
		/* Blade Health - Blade 1 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.1.1.5.1",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 1, /* unknown=0; good=1; warning=2; bad=3 */
			},
		},
	},
	{
		/* Blade Health - Blade 2 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.1.1.5.2",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 1, /* unknown=0; good=1; warning=2; bad=3 */
			},
		},
	},
	{
		/* Blade Health - Blade 3 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.1.1.5.3",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 1, /* unknown=0; good=1; warning=2; bad=3 */
			},
		},
	},
	{
		/* Blade Health - Blade 4 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.1.1.5.4",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 1, /* unknown=0; good=1; warning=2; bad=3 */
			},
		},
	},
	{
		/* Blade Health - Blade 5 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.1.1.5.5",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 1, /* unknown=0; good=1; warning=2; bad=3 */
			},
		},
	},
	{
		/* Blade Health - Blade 6 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.1.1.5.6",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 1, /* unknown=0; good=1; warning=2; bad=3 */
			},
		},
	},
	{
		/* Blade Health - Blade 7 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.1.1.5.7",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 1, /* unknown=0; good=1; warning=2; bad=3 */
			},
		},
	},
	{
		/* Blade Health - Blade 8 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.1.1.5.8",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 1, /* unknown=0; good=1; warning=2; bad=3 */
			},
		},
	},
	{
		/* Blade Health - Blade 9 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.1.1.5.9",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 1, /* unknown=0; good=1; warning=2; bad=3 */
			},
		},
	},
	{
		/* Blade Health - Blade 10 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.1.1.5.10",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 1, /* unknown=0; good=1; warning=2; bad=3 */
			},
		},
	},
	{
		/* Blade Health - Blade 11 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.1.1.5.11",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 1, /* unknown=0; good=1; warning=2; bad=3 */
			},
		},
	},
	{
		/* Blade Health - Blade 12 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.1.1.5.12",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 1, /* unknown=0; good=1; warning=2; bad=3 */
			},
		},
	},
	{
		/* Blade Health - Blade 13 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.1.1.5.13",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 1, /* unknown=0; good=1; warning=2; bad=3 */
			},
		},
	},
	{
		/* Blade Health - Blade 14 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.1.1.5.14",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 1, /* unknown=0; good=1; warning=2; bad=3 */
			},
		},
	},
	{
		/* Blade Restart - Blade 1 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.6.1.1.8.1", /* write-only */
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 1, /* execute=1 */
			},
		},
	},
	{
		/* Blade Restart - Blade 2 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.6.1.1.8.2", /* write-only */
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 1, /* execute=1 */
			},
		},
	},
	{
		/* Blade Restart - Blade 3 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.6.1.1.8.3", /* write-only */
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 1, /* execute=1 */
			},
		},
	},
	{
		/* Blade Restart - Blade 4 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.6.1.1.8.4", /* write-only */
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 1, /* execute=1 */
			},
		},
	},
	{
		/* Blade Restart - Blade 5 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.6.1.1.8.5", /* write-only */
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 1, /* execute=1 */
			},
		},
	},
	{
		/* Blade Restart - Blade 6 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.6.1.1.8.6", /* write-only */
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 1, /* execute=1 */
			},
		},
	},
	{
		/* Blade Restart - Blade 7 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.6.1.1.8.7", /* write-only */
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 1, /* execute=1 */
			},
		},
	},
	{
		/* Blade Restart - Blade 8 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.6.1.1.8.8", /* write-only */
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 1, /* execute=1 */
			},
		},
	},
	{
		/* Blade Restart - Blade 9 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.6.1.1.8.9", /* write-only */
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 1, /* execute=1 */
			},
		},
	},
	{
		/* Blade Restart - Blade 10 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.6.1.1.8.10", /* write-only */
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 1, /* execute=1 */
			},
		},
	},
	{
		/* Blade Restart - Blade 11 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.6.1.1.8.11", /* write-only */
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 1, /* execute=1 */
			},
		},
	},
	{
		/* Blade Restart - Blade 12 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.6.1.1.8.12", /* write-only */
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 1, /* execute=1 */
			},
		},
	},
	{
		/* Blade Restart - Blade 13 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.6.1.1.8.13", /* write-only */
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 1, /* execute=1 */
			},
		},
	},
	{
		/* Blade Restart - Blade 14 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.6.1.1.8.14", /* write-only */
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 1, /* execute=1 */
			},
		},
	},
	{
		/* Blade Power State - Blade 1 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.6.1.1.4.1",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 1, /* off=0; on=1 */
			},
		},
	},
	{
		/* Blade Power State - Blade 2 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.6.1.1.4.2",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 1, /* off=0; on=1 */
			},
		},
	},
	{
		/* Blade Power State - Blade 3 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.6.1.1.4.3",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 1, /* off=0; on=1 */
			},
		},
	},
	{
		/* Blade Power State - Blade 4 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.6.1.1.4.4",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 1, /* off=0; on=1 */
			},
		},
	},
	{
		/* Blade Power State - Blade 5 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.6.1.1.4.5",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 1, /* off=0; on=1 */
			},
		},
	},
	{
		/* Blade Power State - Blade 6 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.6.1.1.4.6",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 1, /* off=0; on=1 */
			},
		},
	},
	{
		/* Blade Power State - Blade 7 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.6.1.1.4.7",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 1, /* off=0; on=1 */
			},
		},
	},
	{
		/* Blade Power State - Blade 8 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.6.1.1.4.8",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 1, /* off=0; on=1 */
			},
		},
	},
	{
		/* Blade Power State - Blade 9 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.6.1.1.4.9",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 1, /* off=0; on=1 */
			},
		},
	},
	{
		/* Blade Power State - Blade 10 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.6.1.1.4.10",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 1, /* off=0; on=1 */
			},
		},
	},
	{
		/* Blade Power State - Blade 11 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.6.1.1.4.11",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 1, /* off=0; on=1 */
			},
		},
	},
	{
		/* Blade Power State - Blade 12 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.6.1.1.4.12",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 1, /* off=0; on=1 */
			},
		},
	},
	{
		/* Blade Power State - Blade 13 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.6.1.1.4.13",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 1, /* off=0; on=1 */
			},
		},
	},
	{
		/* Blade Power State - Blade 14 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.6.1.1.4.14",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 1, /* off=0; on=1 */
			},
		},
	},
	{
		/* Blade Power On/Off - Blade 1 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.6.1.1.7.1", /* write-only */
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 1, /* off=0; on=1 */
			},
		},
	},
	{
		/* Blade Power On/Off - Blade 2 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.6.1.1.7.2", /* write-only */
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 1, /* off=0; on=1 */
			},
		},
	},
	{
		/* Blade Power On/Off - Blade 3 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.6.1.1.7.3", /* write-only */
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 1, /* off=0; on=1 */
			},
		},
	},
	{
		/* Blade Power On/Off - Blade 4 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.6.1.1.7.4", /* write-only */
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 1, /* off=0; on=1 */
			},
		},
	},
	{
		/* Blade Power On/Off - Blade 5 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.6.1.1.7.5", /* write-only */
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 1, /* off=0; on=1 */
			},
		},
	},
	{
		/* Blade Power On/Off - Blade 6 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.6.1.1.7.6", /* write-only */
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 1, /* off=0; on=1 */
			},
		},
	},
	{
		/* Blade Power On/Off - Blade 7 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.6.1.1.7.7", /* write-only */
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 1, /* off=0; on=1 */
			},
		},
	},
	{
		/* Blade Power On/Off - Blade 8 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.6.1.1.7.8", /* write-only */
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 1, /* off=0; on=1 */
			},
		},
	},
	{
		/* Blade Power On/Off - Blade 9 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.6.1.1.7.9", /* write-only */
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 1, /* off=0; on=1 */
			},
		},
	},
	{
		/* Blade Power On/Off - Blade 10 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.6.1.1.7.10", /* write-only */
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 1, /* off=0; on=1 */
			},
		},
	},
	{
		/* Blade Power On/Off - Blade 11 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.6.1.1.7.11", /* write-only */
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 1, /* off=0; on=1 */
			},
		},
	},
	{
		/* Blade Power On/Off - Blade 12 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.6.1.1.7.12", /* write-only */
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 1, /* off=0; on=1 */
			},
		},
	},
	{
		/* Blade Power On/Off - Blade 13 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.6.1.1.7.13", /* write-only */
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 1, /* off=0; on=1 */
			},
		},
	},
	{
		/* Blade Power On/Off - Blade 14 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.6.1.1.7.14", /* write-only */
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 1, /* off=0; on=1 */
			},
		},
	},
	{
		/* Front Panel LED - System Error */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.8.1.1.0",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 0, /* off=0; on=1 */
			},
		},
	},
	{
		/* Front Panel LED - Temperature */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.8.1.3.0",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 0, /* off=0; on=1 */
			},
		},
	},
	{
		/* Ambient temperature */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.1.5.1.0",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  50.00 Centigrade",
			},
		},
	},
	{
		/* Management module temperature */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.1.1.2.0",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  50.00 Centigrade",
			},
		},
	},
	{
		/* Chassis Plus 1.8 Volt */
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.2.1.8.0",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 1.80 Volts",
			},
		},
	},
	{
		/* Chassis Plus 1.8 Volt - Low Minor */
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.20.2.1.1.10.6",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 1.62 Volts",
			},
		},
	},
	{
		/* Chassis Plus 1.8 Volt - Up Minor */
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.20.2.1.1.6.6",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 1.89 Volts",
			},
		},
	},
	{
		/* Chassis Plus 1.8 Volt - Low Hysteresis */
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.20.2.1.1.11.6",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 1.74 Volts",
			},
		},
	},
	{
		/* Chassis Plus 1.8 Volt - Up Hysteresis */
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.20.2.1.1.7.6",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 1.86 Volts",
			},
		},
	},
	{
		/* Chassis Plus 2.5 Volt */
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.2.1.6.0",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 2.50 Volts",
			},
		},
	},
	{
		/* Chassis Plus 2.5 Volt - Low Minor */
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.20.2.1.1.10.5",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 2.25 Volts",
			},
		},
	},
	{
		/* Chassis Plus 2.5 Volt - Up Minor */
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.20.2.1.1.6.5",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 2.63 Volts",
			},
		},
	},
	{
		/* Chassis Plus 2.5 Volt - Low Hysteresis */
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.20.2.1.1.11.5",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 2.42 Volts",
			},
		},
	},
	{
		/* Chassis Plus 2.5 Volt - Up Hysteresis */
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.20.2.1.1.7.5",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 2.58 Volts",
			},
		},
	},
	{
		/* Chassis Plus 3.3 Volt */
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.2.1.2.0",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 3.30 Volts",
			},
		},
	},
	{
		/* Chassis Plus 3.3 Volt - Low Minor */
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.20.2.1.1.10.2",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 3.00 Volts",
			},
		},
	},
	{
		/* Chassis Plus 3.3 Volt - Up Minor */
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.20.2.1.1.6.2",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 3.47 Volts",
			},
		},
	},
	{
		/* Chassis Plus 3.3 Volt - Low Hysteresis */
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.20.2.1.1.11.2",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 3.20 Volts",
			},
		},
	},
	{
		/* Chassis Plus 3.3 Volt - Up Hysteresis */
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.20.2.1.1.7.2",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 3.40 Volts",
			},
		},
	},
	{
		/* Chassis Plus 5 Volt */
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.2.1.1.0",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 5.00 Volts",
			},
		},
	},
	{
		/* Chassis Plus 5 Volt - Low Minor */
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.20.2.1.1.10.1",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 4.50 Volts",
			},
		},
	},
	{
		/* Chassis Plus 5 Volt - Up Minor */
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.20.2.1.1.6.1",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 5.25 Volts",
			},
		},
	},
	{
		/* Chassis Plus 5 Volt - Low Hysteresis */
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.20.2.1.1.11.1",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 4.85 Volts",
			},
		},
	},
	{
		/* Chassis Plus 5 Volt - Up Hysteresis */
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.20.2.1.1.7.1",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 5.15 Volts",
			},
		},
	},
	{
		/* Chassis Negative 5 Volt */
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.2.1.5.0",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "- 5.00 Volts",
			},
		},
	},
	{
		/* Chassis Negative 5 Volt - Low Minor */
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.20.2.1.1.10.4",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "- 5.50 Volts",
			},
		},
	},
	{
		/* Chassis Negative 5 Volt - Up Minor */
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.20.2.1.1.6.4",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "- 4.75 Volts",
			},
		},
	},
	{
		/* Chassis Negative 5 Volt - Low Hysteresis */
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.20.2.1.1.11.4",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "- 5.15 Volts",
			},
		},
	},
	{
		/* Chassis Negitive 5 Volt - Up Hysteresis */
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.20.2.1.1.7.4",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "- 4.85 Volts",
			},
		},
	},
	{
		/* Chassis Plus 12 Volt */
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.2.1.3.0",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+12.00 Volts",
			},
		},
	},
	{
		/* Chassis Plus 12 Volt - Low Minor */
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.20.2.1.1.10.3",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+10.80 Volts",
			},
		},
	},
	{
		/* Chassis Plus 12 Volt - Up Minor */
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.20.2.1.1.6.3",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+12.60 Volts",
			},
		},
	},
	{
		/* Chassis Plus 12 Volt - Low Hysteresis */
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.20.2.1.1.11.3",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+11.64 Volts",
			},
		},
	},
	{
		/* Chassis Plus 12 Volt - Up Hysteresis */
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.20.2.1.1.7.3",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+12.36 Volts",
			},
		},
	},
	{
		/* Chassis Health */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.7.1.0",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 255, /* critical=0; nonCritical=2; systemLevel=4; normal=255 */
			},
		},
	},
	{
		/* Blade Error LED - Blade 1 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.8.2.1.1.7.1",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 0, /* off=0; on=1 */
			},
		},
	},
	{
		/* Blade Error LED - Blade 2 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.8.2.1.1.7.2",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 0, /* off=0; on=1 */
			},
		},
	},
	{
		/* Blade Error LED - Blade 3 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.8.2.1.1.7.3",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 0, /* off=0; on=1 */
			},
		},
	},
	{
		/* Blade Error LED - Blade 4 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.8.2.1.1.7.4",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 0, /* off=0; on=1 */
			},
		},
	},
	{
		/* Blade Error LED - Blade 5 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.8.2.1.1.7.5",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 0, /* off=0; on=1 */
			},
		},
	},
	{
		/* Blade Error LED - Blade 6 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.8.2.1.1.7.6",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 0, /* off=0; on=1 */
			},
		},
	},
	{
		/* Blade Error LED - Blade 7 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.8.2.1.1.7.7",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 0, /* off=0; on=1 */
			},
		},
	},
	{
		/* Blade Error LED - Blade 8 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.8.2.1.1.7.8",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 0, /* off=0; on=1 */
			},
		},
	},
	{
		/* Blade Error LED - Blade 9 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.8.2.1.1.7.9",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 0, /* off=0; on=1 */
			},
		},
	},
	{
		/* Blade Error LED - Blade 10 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.8.2.1.1.7.10",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 0, /* off=0; on=1 */
			},
		},
	},
	{
		/* Blade Error LED - Blade 11 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.8.2.1.1.7.11",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 0, /* off=0; on=1 */
			},
		},
	},
	{
		/* Blade Error LED - Blade 12 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.8.2.1.1.7.12",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 0, /* off=0; on=1 */
			},
		},
	},
	{
		/* Blade Error LED - Blade 13 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.8.2.1.1.7.13",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 0, /* off=0; on=1 */
			},
		},
	},
	{
		/* Blade Error LED - Blade 14 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.8.2.1.1.7.14",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 0, /* off=0; on=1 */
			},
		},
	},
	{
		/* Blade KVM Usage LED - Blade 1 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.8.2.1.1.9.1",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 0, /* off=0; on=1; blinking=2 */
			},
		},
	},
	{
		/* Blade KVM Usage LED - Blade 2 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.8.2.1.1.9.2",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 0, /* off=0; on=1; blinking=2 */
			},
		},
	},
	{
		/* Blade KVM Usage LED - Blade 3 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.8.2.1.1.9.3",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 0, /* off=0; on=1; blinking=2 */
			},
		},
	},
	{
		/* Blade KVM Usage LED - Blade 4 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.8.2.1.1.9.4",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 0, /* off=0; on=1; blinking=2 */
			},
		},
	},
	{
		/* Blade KVM Usage LED - Blade 5 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.8.2.1.1.9.5",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 0, /* off=0; on=1; blinking=2 */
			},
		},
	},
	{
		/* Blade KVM Usage LED - Blade 6 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.8.2.1.1.9.6",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 0, /* off=0; on=1; blinking=2 */
			},
		},
	},
	{
		/* Blade KVM Usage LED - Blade 7 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.8.2.1.1.9.7",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 0, /* off=0; on=1; blinking=2 */
			},
		},
	},
	{
		/* Blade KVM Usage LED - Blade 8 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.8.2.1.1.9.8",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 0, /* off=0; on=1; blinking=2 */
			},
		},
	},
	{
		/* Blade KVM Usage LED - Blade 9 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.8.2.1.1.9.9",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 0, /* off=0; on=1; blinking=2 */
			},
		},
	},
	{
		/* Blade KVM Usage LED - Blade 10 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.8.2.1.1.9.10",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 0, /* off=0; on=1; blinking=2 */
			},
		},
	},
	{
		/* Blade KVM Usage LED - Blade 11 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.8.2.1.1.9.11",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 0, /* off=0; on=1; blinking=2 */
			},
		},
	},
	{
		/* Blade KVM Usage LED - Blade 12 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.8.2.1.1.9.12",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 0, /* off=0; on=1; blinking=2 */
			},
		},
	},
	{
		/* Blade KVM Usage LED - Blade 13 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.8.2.1.1.9.13",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 0, /* off=0; on=1; blinking=2 */
			},
		},
	},
	{
		/* Blade KVM Usage LED - Blade 14 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.8.2.1.1.9.14",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 0, /* off=0; on=1; blinking=2 */
			},
		},
	},
	{
		/* Blade Media Tray Usage - Blade 1 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.8.2.1.1.10.1",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 0, /* off=0; on=1; blinking=2 */
			},
		},
	},
	{
		/* Blade Media Tray Usage - Blade 2 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.8.2.1.1.10.2",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 0, /* off=0; on=1; blinking=2 */
			},
		},
	},
	{
		/* Blade Media Tray Usage - Blade 3 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.8.2.1.1.10.3",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 0, /* off=0; on=1; blinking=2 */
			},
		},
	},
	{
		/* Blade Media Tray Usage - Blade 4 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.8.2.1.1.10.4",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 0, /* off=0; on=1; blinking=2 */
			},
		},
	},
	{
		/* Blade Media Tray Usage - Blade 5 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.8.2.1.1.10.5",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 0, /* off=0; on=1; blinking=2 */
			},
		},
	},
	{
		/* Blade Media Tray Usage - Blade 6 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.8.2.1.1.10.6",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 0, /* off=0; on=1; blinking=2 */
			},
		},
	},
	{
		/* Blade Media Tray Usage - Blade 7 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.8.2.1.1.10.7",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 0, /* off=0; on=1; blinking=2 */
			},
		},
	},
	{
		/* Blade Media Tray Usage - Blade 8 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.8.2.1.1.10.8",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 0, /* off=0; on=1; blinking=2 */
			},
		},
	},
	{
		/* Blade Media Tray Usage - Blade 9 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.8.2.1.1.10.9",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 0, /* off=0; on=1; blinking=2 */
			},
		},
	},
	{
		/* Blade Media Tray Usage - Blade 10 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.8.2.1.1.10.10",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 0, /* off=0; on=1; blinking=2 */
			},
		},
	},
	{
		/* Blade Media Tray Usage - Blade 11 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.8.2.1.1.10.11",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 0, /* off=0; on=1; blinking=2 */
			},
		},
	},
	{
		/* Blade Media Tray Usage - Blade 12 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.8.2.1.1.10.12",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 0, /* off=0; on=1; blinking=2 */
			},
		},
	},
	{
		/* Blade Media Tray Usage - Blade 13 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.8.2.1.1.10.13",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 0, /* off=0; on=1; blinking=2 */
			},
		},
	},
	{
		/* Blade Media Tray Usage - Blade 14 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.8.2.1.1.10.14",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 0, /* off=0; on=1; blinking=2 */
			},
		},
	},
        {
		/* Blade CPU 1 temperature - Blade 1 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.3.1.6.1",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  50.00 Centigrade",
			},
		},
	},
        {
		/* Blade CPU 1 temperature - Blade 2 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.3.1.6.2",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  50.00 Centigrade",
			},
		},
	},
        {
		/* Blade CPU 1 temperature - Blade 3 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.3.1.6.3",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  50.00 Centigrade",
			},
		},
	},
        {
		/* Blade CPU 1 temperature - Blade 4 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.3.1.6.4",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  50.00 Centigrade",
			},
		},
	},
        {
		/* Blade CPU 1 temperature - Blade 5 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.3.1.6.5",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  50.00 Centigrade",
			},
		},
	},
        {
		/* Blade CPU 1 temperature - Blade 6 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.3.1.6.6",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  50.00 Centigrade",
			},
		},
	},
        {
		/* Blade CPU 1 temperature - Blade 7 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.3.1.6.7",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  50.00 Centigrade",
			},
		},
	},
        {
		/* Blade CPU 1 temperature - Blade 8 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.3.1.6.8",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  50.00 Centigrade",
			},
		},
	},
        {
		/* Blade CPU 1 temperature - Blade 9 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.3.1.6.9",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  50.00 Centigrade",
			},
		},
	},
        {
		/* Blade CPU 1 temperature - Blade 10 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.3.1.6.10",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  50.00 Centigrade",
			},
		},
	},
        {
		/* Blade CPU 1 temperature - Blade 11 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.3.1.6.11",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  50.00 Centigrade",
			},
		},
	},
        {
		/* Blade CPU 1 temperature - Blade 12 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.3.1.6.12",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  50.00 Centigrade",
			},
		},
	},
        {
		/* Blade CPU 1 temperature - Blade 13 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.3.1.6.13",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  50.00 Centigrade",
			},
		},
	},
        {
		/* Blade CPU 1 temperature - Blade 14 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.3.1.6.14",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  50.00 Centigrade",
			},
		},
	},
        {
		/* Blade CPU 1 Up Critical temperature - Blade 1 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.6.1",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  95.00 Centigrade",
			},
		},
	},
        {
		/* Blade CPU 1 Up Critical temperature - Blade 2 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.6.2",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  95.00 Centigrade",
			},
		},
	},
        {
		/* Blade CPU 1 Up Critical temperature - Blade 3 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.6.3",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  95.00 Centigrade",
			},
		},
	},
        {
		/* Blade CPU 1 Up Critical temperature - Blade 4 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.6.4",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  95.00 Centigrade",
			},
		},
	},
        {
		/* Blade CPU 1 Up Critical temperature - Blade 5 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.6.5",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  95.00 Centigrade",
			},
		},
	},
        {
		/* Blade CPU 1 Up Critical temperature - Blade 6 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.6.6",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  95.00 Centigrade",
			},
		},
	},
        {
		/* Blade CPU 1 Up Critical temperature - Blade 7 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.6.7",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  95.00 Centigrade",
			},
		},
	},
        {
		/* Blade CPU 1 Up Critical temperature - Blade 8 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.6.8",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  95.00 Centigrade",
			},
		},
	},
        {
		/* Blade CPU 1 Up Critical temperature - Blade 9 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.6.9",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  95.00 Centigrade",
			},
		},
	},
        {
		/* Blade CPU 1 Up Critical temperature - Blade 10 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.6.10",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  95.00 Centigrade",
			},
		},
	},
        {
		/* Blade CPU 1 Up Critical temperature - Blade 11 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.6.11",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  95.00 Centigrade",
			},
		},
	},
        {
		/* Blade CPU 1 Up Critical temperature - Blade 12 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.6.12",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  95.00 Centigrade",
			},
		},
	},
        {
		/* Blade CPU 1 Up Critical temperature - Blade 13 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.6.13",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  95.00 Centigrade",
			},
		},
	},
        {
		/* Blade CPU 1 Up Critical temperature - Blade 14 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.6.14",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  95.00 Centigrade",
			},
		},
	},
        {
		/* Blade CPU 1 Up Minor temperature - Blade 1 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.7.1",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  85.00 Centigrade",
			},
		},
	},
        {
		/* Blade CPU 1 Up Minor temperature - Blade 2 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.7.2",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  85.00 Centigrade",
			},
		},
	},
        {
		/* Blade CPU 1 Up Minor temperature - Blade 3 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.7.3",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  85.00 Centigrade",
			},
		},
	},
        {	
		/* Blade CPU 1 Up Minor temperature - Blade 4 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.7.4",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  85.00 Centigrade",
			},
		},
	},
        {	
		/* Blade CPU 1 Up Minor temperature - Blade 5 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.7.5",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  85.00 Centigrade",
			},
		},
	},
        {	
		/* Blade CPU 1 Up Minor temperature - Blade 6 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.7.6",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  85.00 Centigrade",
			},
		},
	},
        {	
		/* Blade CPU 1 Up Minor temperature - Blade 7 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.7.7",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  85.00 Centigrade",
			},
		},
	},
        {	
		/* Blade CPU 1 Up Minor temperature - Blade 8 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.7.8",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  85.00 Centigrade",
			},
		},
	},
        {
		/* Blade CPU 1 Up Minor temperature - Blade 9 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.7.9",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  85.00 Centigrade",
			},
		},
	},
        {
		/* Blade CPU 1 Up Minor temperature - Blade 10 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.7.10",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  85.00 Centigrade",
			},
		},
	},
        {
		/* Blade CPU 1 Up Minor temperature - Blade 11 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.7.11",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  85.00 Centigrade",
			},
		},
	},
	{
		/* Blade CPU 1 Up Minor temperature - Blade 12 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.7.12",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  85.00 Centigrade",
			},
		},
	},
        {
		/* Blade CPU 1 Up Minor temperature - Blade 13 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.7.13",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  85.00 Centigrade",
			},
		},
	},
        {
		/* Blade CPU 1 Up Minor temperature - Blade 14 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.7.14",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  85.00 Centigrade",
			},
		},
	},
        {
		/* Blade CPU 1 Up Hysteresis temperature - Blade 1 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.8.1",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  78.00 Centigrade",
			},
		},
	},
        {
		/* Blade CPU 1 Up Hysteresis temperature - Blade 2 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.8.2",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  78.00 Centigrade",
			},
		},
	},
        {
		/* Blade CPU 1 Up Hysteresis temperature - Blade 3 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.8.3",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  78.00 Centigrade",
			},
		},
	},
        {
		/* Blade CPU 1 Up Hysteresis temperature - Blade 4 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.8.4",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  78.00 Centigrade",
			},
		},
	},
        {
		/* Blade CPU 1 Up Hysteresis temperature - Blade 5 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.8.5",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  78.00 Centigrade",
			},
		},
	},
        {
		/* Blade CPU 1 Up Hysteresis temperature - Blade 6 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.8.6",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  78.00 Centigrade",
			},
		},
	},
        {
		/* Blade CPU 1 Up Hysteresis temperature - Blade 7 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.8.7",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  78.00 Centigrade",
			},
		},
	},
        {
		/* Blade CPU 1 Up Hysteresis temperature - Blade 8 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.8.8",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  78.00 Centigrade",
			},
		},
	},
        {
		/* Blade CPU 1 Up Hysteresis temperature - Blade 9 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.8.9",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  78.00 Centigrade",
			},
		},
	},
        {
		/* Blade CPU 1 Up Hysteresis temperature - Blade 10 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.8.10",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  78.00 Centigrade",
			},
		},
	},
        {
		/* Blade CPU 1 Up Hysteresis temperature - Blade 11 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.8.11",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  78.00 Centigrade",
			},
		},
	},
        {
		/* Blade CPU 1 Up Hysteresis temperature - Blade 12 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.8.12",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  78.00 Centigrade",
			},
		},
	},
        {
		/* Blade CPU 1 Up Hysteresis temperature - Blade 13 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.8.13",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  78.00 Centigrade",
			},
		},
	},
        {
		/* Blade CPU 1 Up Hysteresis temperature - Blade 14 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.8.14",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  78.00 Centigrade",
			},
		},
	},
        {
		/* Blade CPU 2 temperature - Blade 1 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.3.1.7.1",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  50.00 Centigrade",
			},
		},
	},
        {
		/* Blade CPU 2 temperature - Blade 2 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.3.1.7.2",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  50.00 Centigrade",
			},
		},
	},
        {
		/* Blade CPU 2 temperature - Blade 3 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.3.1.7.3",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  50.00 Centigrade",
			},
		},
	},
        {
		/* Blade CPU 2 temperature - Blade 4 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.3.1.7.4",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  50.00 Centigrade",
			},
		},
	},
        {
		/* Blade CPU 2 temperature - Blade 5 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.3.1.7.5",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  50.00 Centigrade",
			},
		},
	},
        {
		/* Blade CPU 2 temperature - Blade 6 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.3.1.7.6",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  50.00 Centigrade",
			},
		},
	},
        {
		/* Blade CPU 2 temperature - Blade 7 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.3.1.7.7",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  50.00 Centigrade",
			},
		},
	},
        {
		/* Blade CPU 2 temperature - Blade 8 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.3.1.7.8",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  50.00 Centigrade",
			},
		},
	},
        {
		/* Blade CPU 2 temperature - Blade 9 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.3.1.7.9",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  50.00 Centigrade",
			},
		},
	},
        {
		/* Blade CPU 2 temperature - Blade 10 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.3.1.7.10",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  50.00 Centigrade",
			},
		},
	},
        {
		/* Blade CPU 2 temperature - Blade 11 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.3.1.7.11",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  50.00 Centigrade",
			},
		},
	},
        {
		/* Blade CPU 2 temperature - Blade 12 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.3.1.7.12",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  50.00 Centigrade",
			},
		},
	},
        {
		/* Blade CPU 2 temperature - Blade 13 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.3.1.7.13",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  50.00 Centigrade",
			},
		},
	},
        {
		/* Blade CPU 2 temperature - Blade 14 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.3.1.7.14",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  50.00 Centigrade",
			},
		},
	},
        {
		/* Blade CPU 2 Up Critical temperature - Blade 1 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.9.1",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  95.00 Centigrade",
			},
		},
	},
        {
		/* Blade CPU 2 Up Critical temperature - Blade 2 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.9.2",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  95.00 Centigrade",
			},
		},
	},
        {
		/* Blade CPU 2 Up Critical temperature - Blade 3 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.9.3",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  95.00 Centigrade",
			},
		},
	},
        {
		/* Blade CPU 2 Up Critical temperature - Blade 4 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.9.4",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  95.00 Centigrade",
			},
		},
	},
        {
		/* Blade CPU 2 Up Critical temperature - Blade 5 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.9.5",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  95.00 Centigrade",
			},
		},
	},
        {
		/* Blade CPU 2 Up Critical temperature - Blade 6 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.9.6",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  95.00 Centigrade",
			},
		},
	},
        {
		/* Blade CPU 2 Up Critical temperature - Blade 7 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.9.7",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  95.00 Centigrade",
			},
		},
	},
        {
		/* Blade CPU 2 Up Critical temperature - Blade 8 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.9.8",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  95.00 Centigrade",
			},
		},
	},
        {
		/* Blade CPU 2 Up Critical temperature - Blade 9 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.9.9",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  95.00 Centigrade",
			},
		},
	},
        {
		/* Blade CPU 2 Up Critical temperature - Blade 10 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.9.10",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  95.00 Centigrade",
			},
		},
	},
        {
		/* Blade CPU 2 Up Critical temperature - Blade 11 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.9.11",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  95.00 Centigrade",
			},
		},
	},
        {
		/* Blade CPU 2 Up Critical temperature - Blade 12 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.9.12",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  95.00 Centigrade",
			},
		},
	},
        {
		/* Blade CPU 2 Up Critical temperature - Blade 13 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.9.13",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  95.00 Centigrade",
			},
		},
	},
        {
		/* Blade CPU 2 Up Critical temperature - Blade 14 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.9.14",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  95.00 Centigrade",
			},
		},
	},
        {
		/* Blade CPU 2 Up Minor temperature - Blade 1 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.10.1",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  85.00 Centigrade",
			},
		},
	},
        {
		/* Blade CPU 2 Up Minor temperature - Blade 2 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.10.2",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  85.00 Centigrade",
			},
		},
	},
        {
		/* Blade CPU 2 Up Minor temperature - Blade 3 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.10.3",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  85.00 Centigrade",
			},
		},
	},
        {	
		/* Blade CPU 2 Up Minor temperature - Blade 4 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.10.4",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  85.00 Centigrade",
			},
		},
	},
        {	
		/* Blade CPU 2 Up Minor temperature - Blade 5 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.10.5",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  85.00 Centigrade",
			},
		},
	},
        {	
		/* Blade CPU 2 Up Minor temperature - Blade 6 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.10.6",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  85.00 Centigrade",
			},
		},
	},
        {	
		/* Blade CPU 2 Up Minor temperature - Blade 7 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.10.7",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  85.00 Centigrade",
			},
		},
	},
        {	
		/* Blade CPU 2 Up Minor temperature - Blade 8 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.10.8",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  85.00 Centigrade",
			},
		},
	},
        {
		/* Blade CPU 2 Up Minor temperature - Blade 9 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.10.9",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  85.00 Centigrade",
			},
		},
	},
        {
		/* Blade CPU 2 Up Minor temperature - Blade 10 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.10.10",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  85.00 Centigrade",
			},
		},
	},
        {
		/* Blade CPU 2 Up Minor temperature - Blade 11 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.10.11",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  85.00 Centigrade",
			},
		},
	},
	{
		/* Blade CPU 2 Up Minor temperature - Blade 12 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.10.12",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  85.00 Centigrade",
			},
		},
	},
        {
		/* Blade CPU 2 Up Minor temperature - Blade 13 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.10.13",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  85.00 Centigrade",
			},
		},
	},
        {
		/* Blade CPU 2 Up Minor temperature - Blade 14 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.10.14",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  85.00 Centigrade",
			},
		},
	},
        {
		/* Blade CPU 2 Up Hysteresis temperature - Blade 1 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.11.1",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  78.00 Centigrade",
			},
		},
	},
        {
		/* Blade CPU 2 Up Hysteresis temperature - Blade 2 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.11.2",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  78.00 Centigrade",
			},
		},
	},
        {
		/* Blade CPU 2 Up Hysteresis temperature - Blade 3 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.11.3",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  78.00 Centigrade",
			},
		},
	},
        {
		/* Blade CPU 2 Up Hysteresis temperature - Blade 4 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.11.4",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  78.00 Centigrade",
			},
		},
	},
        {
		/* Blade CPU 2 Up Hysteresis temperature - Blade 5 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.11.5",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  78.00 Centigrade",
			},
		},
	},
        {
		/* Blade CPU 2 Up Hysteresis temperature - Blade 6 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.11.6",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  78.00 Centigrade",
			},
		},
	},
        {
		/* Blade CPU 2 Up Hysteresis temperature - Blade 7 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.11.7",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  78.00 Centigrade",
			},
		},
	},
        {
		/* Blade CPU 2 Up Hysteresis temperature - Blade 8 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.11.8",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  78.00 Centigrade",
			},
		},
	},
        {
		/* Blade CPU 2 Up Hysteresis temperature - Blade 9 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.11.9",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  78.00 Centigrade",
			},
		},
	},
        {
		/* Blade CPU 2 Up Hysteresis temperature - Blade 10 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.11.10",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  78.00 Centigrade",
			},
		},
	},
        {
		/* Blade CPU 2 Up Hysteresis temperature - Blade 11 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.11.11",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  78.00 Centigrade",
			},
		},
	},
        {
		/* Blade CPU 2 Up Hysteresis temperature - Blade 12 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.11.12",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  78.00 Centigrade",
			},
		},
	},
        {
		/* Blade CPU 2 Up Hysteresis temperature - Blade 13 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.11.13",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  78.00 Centigrade",
			},
		},
	},
        {
		/* Blade CPU 2 Up Hysteresis temperature - Blade 14 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.11.14",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  78.00 Centigrade",
			},
		},
	},
	/* Blade DASD 1 temperature - already defined above for Add-in card vector */
        {
		/* Blade DASD 1 Up Critical temperature - Blade 1 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.12.1",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  95.00 Centigrade",
			},
		},
	},
        {
		/* Blade DASD 1 Up Critical temperature - Blade 2 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.12.2",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  95.00 Centigrade",
			},
		},
	},
        {
		/* Blade DASD 1 Up Critical temperature - Blade 3 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.12.3",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  95.00 Centigrade",
			},
		},
	},
        {
		/* Blade DASD 1 Up Critical temperature - Blade 4 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.12.4",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  95.00 Centigrade",
			},
		},
	},
        {
		/* Blade DASD 1 Up Critical temperature - Blade 5 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.12.5",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  95.00 Centigrade",
			},
		},
	},
        {
		/* Blade DASD 1 Up Critical temperature - Blade 6 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.12.6",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  95.00 Centigrade",
			},
		},
	},
        {
		/* Blade DASD 1 Up Critical temperature - Blade 7 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.12.7",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  95.00 Centigrade",
			},
		},
	},
        {
		/* Blade DASD 1 Up Critical temperature - Blade 8 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.12.8",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  95.00 Centigrade",
			},
		},
	},
        {
		/* Blade DASD 1 Up Critical temperature - Blade 9 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.12.9",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  95.00 Centigrade",
			},
		},
	},
        {
		/* Blade DASD 1 Up Critical temperature - Blade 10 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.12.10",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  95.00 Centigrade",
			},
		},
	},
        {
		/* Blade DASD 1 Up Critical temperature - Blade 11 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.12.11",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  95.00 Centigrade",
			},
		},
	},
        {
		/* Blade DASD 1 Up Critical temperature - Blade 12 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.12.12",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  95.00 Centigrade",
			},
		},
	},
        {
		/* Blade DASD 1 Up Critical temperature - Blade 13 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.12.13",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  95.00 Centigrade",
			},
		},
	},
        {
		/* Blade DASD 1 Up Critical temperature - Blade 14 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.12.14",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  95.00 Centigrade",
			},
		},
	},
        {
		/* Blade DASD 1 Up Minor temperature - Blade 1 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.13.1",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  85.00 Centigrade",
			},
		},
	},
        {
		/* Blade DASD 1 Up Minor temperature - Blade 2 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.13.2",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  85.00 Centigrade",
			},
		},
	},
        {
		/* Blade DASD 1 Up Minor temperature - Blade 3 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.13.3",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  85.00 Centigrade",
			},
		},
	},
        {	
		/* Blade DASD 1 Up Minor temperature - Blade 4 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.13.4",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  85.00 Centigrade",
			},
		},
	},
        {	
		/* Blade DASD 1 Up Minor temperature - Blade 5 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.13.5",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  85.00 Centigrade",
			},
		},
	},
        {	
		/* Blade DASD 1 Up Minor temperature - Blade 6 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.13.6",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  85.00 Centigrade",
			},
		},
	},
        {	
		/* Blade DASD 1 Up Minor temperature - Blade 7 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.13.7",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  85.00 Centigrade",
			},
		},
	},
        {	
		/* Blade DASD 1 Up Minor temperature - Blade 8 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.13.8",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  85.00 Centigrade",
			},
		},
	},
        {
		/* Blade DASD 1 Up Minor temperature - Blade 9 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.13.9",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  85.00 Centigrade",
			},
		},
	},
        {
		/* Blade DASD 1 Up Minor temperature - Blade 10 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.13.10",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  85.00 Centigrade",
			},
		},
	},
        {
		/* Blade DASD 1 Up Minor temperature - Blade 11 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.13.11",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  85.00 Centigrade",
			},
		},
	},
	{
		/* Blade DASD 1 Up Minor temperature - Blade 12 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.13.12",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  85.00 Centigrade",
			},
		},
	},
        {
		/* Blade DASD 1 Up Minor temperature - Blade 13 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.13.13",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  85.00 Centigrade",
			},
		},
	},
        {
		/* Blade DASD 1 Up Minor temperature - Blade 14 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.13.14",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  85.00 Centigrade",
			},
		},
	},
        {
		/* Blade DASD 1 Up Hysteresis temperature - Blade 1 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.14.1",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  78.00 Centigrade",
			},
		},
	},
        {
		/* Blade DASD 1 Up Hysteresis temperature - Blade 2 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.14.2",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  78.00 Centigrade",
			},
		},
	},
        {
		/* Blade DASD 1 Up Hysteresis temperature - Blade 3 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.14.3",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  78.00 Centigrade",
			},
		},
	},
        {
		/* Blade DASD 1 Up Hysteresis temperature - Blade 4 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.14.4",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  78.00 Centigrade",
			},
		},
	},
        {
		/* Blade DASD 1 Up Hysteresis temperature - Blade 5 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.14.5",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  78.00 Centigrade",
			},
		},
	},
        {
		/* Blade DASD 1 Up Hysteresis temperature - Blade 6 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.14.6",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  78.00 Centigrade",
			},
		},
	},
        {
		/* Blade DASD 1 Up Hysteresis temperature - Blade 7 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.14.7",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  78.00 Centigrade",
			},
		},
	},
        {
		/* Blade DASD 1 Up Hysteresis temperature - Blade 8 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.14.8",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  78.00 Centigrade",
			},
		},
	},
        {
		/* Blade DASD 1 Up Hysteresis temperature - Blade 9 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.14.9",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  78.00 Centigrade",
			},
		},
	},
        {
		/* Blade DASD 1 Up Hysteresis temperature - Blade 10 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.14.10",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  78.00 Centigrade",
			},
		},
	},
        {
		/* Blade DASD 1 Up Hysteresis temperature - Blade 11 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.14.11",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  78.00 Centigrade",
			},
		},
	},
        {
		/* Blade DASD 1 Up Hysteresis temperature - Blade 12 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.14.12",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  78.00 Centigrade",
			},
		},
	},
        {
		/* Blade DASD 1 Up Hysteresis temperature - Blade 13 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.14.13",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  78.00 Centigrade",
			},
		},
	},
        {
		/* Blade DASD 1 Up Hysteresis temperature - Blade 14 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.14.14",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  78.00 Centigrade",
			},
		},
	},
        {
		/* Blade 5V - Blade 1 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.6.1",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 5.00 Volts",
			},
		},
	},
        {
		/* Blade 5V - Blade 2 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.6.2",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 5.00 Volts",
			},
		},
	},
        {
		/* Blade 5V - Blade 3 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.6.3",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 5.00 Volts",
			},
		},
	},
        {
		/* Blade 5V - Blade 4 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.6.4",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 5.00 Volts",
			},
		},
	},
        {
		/* Blade 5V - Blade 5 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.6.5",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 5.00 Volts",
			},
		},
	},
        {
		/* Blade 5V - Blade 6 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.6.6",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 5.00 Volts",
			},
		},
	},
        {
		/* Blade 5V - Blade 7 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.6.7",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 5.00 Volts",
			},
		},
	},
        {
		/* Blade 5V - Blade 8 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.6.8",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 5.00 Volts",
			},
		},
	},
        {
		/* Blade 5V - Blade 9 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.6.9",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 5.00 Volts",
			},
		},
	},
        {
		/* Blade 5V - Blade 10 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.6.10",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 5.00 Volts",
			},
		},
	},
        {
		/* Blade 5V - Blade 11 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.6.11",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 5.00 Volts",
			},
		},
	},
        {
		/* Blade 5V - Blade 12 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.6.12",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 5.00 Volts",
			},
		},
	},
        {
		/* Blade 5V - Blade 13 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.6.13",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 5.00 Volts",
			},
		},
	},
        {
		/* Blade 5V - Blade 14 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.6.14",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 5.00 Volts",
			},
		},
	},
        {
		/* Blade 5V Low Minor - Blade 1 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.7.1",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 4.40 Volts",
			},
		},
	},
        {
		/* Blade 5V Low Minor - Blade 2 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.7.2",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 4.40 Volts",
			},
		},
	},
        {
		/* Blade 5V Low Minor - Blade 3 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.7.3",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 4.40 Volts",
			},
		},
	},
        {
		/* Blade 5V Low Minor - Blade 4 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.7.4",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 4.40 Volts",
			},
		},
	},
        {
		/* Blade 5V Low Minor - Blade 5 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.7.5",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 4.40 Volts",
			},
		},
	},
        {
		/* Blade 5V Low Minor - Blade 6 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.7.6",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 4.40 Volts",
			},
		},
	},
        {
		/* Blade 5V Low Minor - Blade 7 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.7.7",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 4.40 Volts",
			},
		},
	},
        {
		/* Blade 5V Low Minor - Blade 8 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.7.8",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 4.40 Volts",
			},
		},
	},
        {
		/* Blade 5V Low Minor - Blade 9 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.7.9",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 4.40 Volts",
			},
		},
	},
        {
		/* Blade 5V Low Minor - Blade 10 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.7.10",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 4.40 Volts",
			},
		},
	},
        {
		/* Blade 5V Low Minor - Blade 11 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.7.11",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 4.40 Volts",
			},
		},
	},
        {
		/* Blade 5V Low Minor - Blade 12 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.7.12",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 4.40 Volts",
			},
		},
	},
        {
		/* Blade 5V Low Minor - Blade 13 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.7.13",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 4.40 Volts",
			},
		},
	},
        {
		/* Blade 5V Low Minor - Blade 14 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.7.14",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 4.40 Volts",
			},
		},
	},
        {
		/* Blade 5V Up Minor - Blade 1 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.6.1",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 5.50 Volts",
			},
		},
	},
        {
		/* Blade 5V Up Minor - Blade 2 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.6.2",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 5.50 Volts",
			},
		},
	},
        {
		/* Blade 5V Up Minor - Blade 3 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.6.3",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 5.50 Volts",
			},
		},
	},
        {
		/* Blade 5V Up Minor - Blade 4 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.6.4",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 5.50 Volts",
			},
		},
	},
        {
		/* Blade 5V Up Minor - Blade 5 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.6.5",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 5.50 Volts",
			},
		},
	},
        {
		/* Blade 5V Up Minor - Blade 6 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.6.6",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 5.50 Volts",
			},
		},
	},
        {
		/* Blade 5V Up Minor - Blade 7 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.6.7",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 5.50 Volts",
			},
		},
	},
        {
		/* Blade 5V Up Minor - Blade 8 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.6.8",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 5.50 Volts",
			},
		},
	},
        {
		/* Blade 5V Up Minor - Blade 9 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.6.9",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 5.50 Volts",
			},
		},
	},
        {
		/* Blade 5V Up Minor - Blade 10 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.6.10",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 5.50 Volts",
			},
		},
	},
        {
		/* Blade 5V Up Minor - Blade 11 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.6.11",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 5.50 Volts",
			},
		},
	},
        {
		/* Blade 5V Up Minor - Blade 12 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.6.12",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 5.50 Volts",
			},
		},
	},
        {
		/* Blade 5V Up Minor - Blade 13 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.6.13",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 5.50 Volts",
			},
		},
	},
        {
		/* Blade 5V Up Minor - Blade 14 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.6.14",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 5.50 Volts",
			},
		},
	},
        {
		/* Blade 3.3 Volts - Blade 1 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.7.1",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 3.30 Volts",
			},
		},
	},
        {
		/* Blade 3.3 Volts - Blade 2 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.7.2",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 3.30 Volts",
			},
		},
	},
        {
		/* Blade 3.3 Volts - Blade 3 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.7.3",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 3.30 Volts",
			},
		},
	},
        {
		/* Blade 3.3 Volts - Blade 4 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.7.4",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 3.30 Volts",
			},
		},
	},
        {
		/* Blade 3.3 Volts - Blade 5 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.7.5",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 3.30 Volts",
			},
		},
	},
        {
		/* Blade 3.3 Volts - Blade 6 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.7.6",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 3.30 Volts",
			},
		},
	},
        {
		/* Blade 3.3 Volts - Blade 7 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.7.7",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 3.30 Volts",
			},
		},
	},
        {
		/* Blade 3.3 Volts - Blade 8 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.7.8",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 3.30 Volts",
			},
		},
	},
        {
		/* Blade 3.3 Volts - Blade 9 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.7.9",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 3.30 Volts",
			},
		},
	},
        {
		/* Blade 3.3 Volts - Blade 10 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.7.10",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 3.30 Volts",
			},
		},
	},
        {
		/* Blade 3.3 Volts - Blade 11 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.7.11",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 3.30 Volts",
			},
		},
	},
        {
		/* Blade 3.3 Volts - Blade 12 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.7.12",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 3.30 Volts",
			},
		},
	},
        {
		/* Blade 3.3 Volts - Blade 13 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.7.13",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 3.30 Volts",
			},
		},
	},
        {
		/* Blade 3.3 Volts - Blade 14 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.7.14",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 3.30 Volts",
			},
		},
	},
        {
		/* Blade 3.3 Volts Low Minor - Blade 1 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.9.1",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 2.97 Volts",
			},
		},
	},
        {
		/* Blade 3.3 Volts Low Minor - Blade 2 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.9.2",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 2.97 Volts",
			},
		},
	},
        {
		/* Blade 3.3 Volts Low Minor - Blade 3 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.9.3",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 2.97 Volts",
			},
		},
	},
        {
		/* Blade 3.3 Volts Low Minor - Blade 4 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.9.4",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 2.97 Volts",
			},
		},
	},
        {
		/* Blade 3.3 Volts Low Minor - Blade 5 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.9.5",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 2.97 Volts",
			},
		},
	},
        {
		/* Blade 3.3 Volts Low Minor - Blade 6 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.9.6",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 2.97 Volts",
			},
		},
	},
        {
		/* Blade 3.3 Volts Low Minor - Blade 7 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.9.7",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 2.97 Volts",
			},
		},
	},
        {
		/* Blade 3.3 Volts Low Minor - Blade 8 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.9.8",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 2.97 Volts",
			},
		},
	},
        {
		/* Blade 3.3 Volts Low Minor - Blade 9 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.9.9",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 2.97 Volts",
			},
		},
	},
        {
		/* Blade 3.3 Volts Low Minor - Blade 10 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.9.10",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 2.97 Volts",
			},
		},
	},
        {
		/* Blade 3.3 Volts Low Minor - Blade 11 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.9.11",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 2.97 Volts",
			},
		},
	},
        {
		/* Blade 3.3 Volts Low Minor - Blade 12 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.9.12",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 2.97 Volts",
			},
		},
	},
        {
		/* Blade 3.3 Volts Low Minor - Blade 13 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.9.13",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 2.97 Volts",
			},
		},
	},
        {
		/* Blade 3.3 Volts Low Minor - Blade 14 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.9.14",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 2.97 Volts",
			},
		},
	},
        {
		/* Blade 3.3 Volts Up Minor - Blade 1 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.8.1",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 3.63 Volts",
			},
		},
	},
        {
		/* Blade 3.3 Volts Up Minor - Blade 2 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.8.2",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 3.63 Volts",
			},
		},
	},
        {
		/* Blade 3.3 Volts Up Minor - Blade 3 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.8.3",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 3.63 Volts",
			},
		},
	},
        {
		/* Blade 3.3 Volts Up Minor - Blade 4 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.8.4",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 3.63 Volts",
			},
		},
	},
        {
		/* Blade 3.3 Volts Up Minor - Blade 5 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.8.5",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 3.63 Volts",
			},
		},
	},
        {
		/* Blade 3.3 Volts Up Minor - Blade 6 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.8.6",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 3.63 Volts",
			},
		},
	},
        {
		/* Blade 3.3 Volts Up Minor - Blade 7 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.8.7",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 3.63 Volts",
			},
		},
	},
        {
		/* Blade 3.3 Volts Up Minor - Blade 8 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.8.8",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 3.63 Volts",
			},
		},
	},
        {
		/* Blade 3.3 Volts Up Minor - Blade 9 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.8.9",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 3.63 Volts",
			},
		},
	},
        {
		/* Blade 3.3 Volts Up Minor - Blade 10 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.8.10",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 3.63 Volts",
			},
		},
	},
        {
		/* Blade 3.3 Volts Up Minor - Blade 11 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.8.11",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 3.63 Volts",
			},
		},
	},
        {
		/* Blade 3.3 Volts Up Minor - Blade 12 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.8.12",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 3.63 Volts",
			},
		},
	},
        {
		/* Blade 3.3 Volts Up Minor - Blade 13 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.8.13",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 3.63 Volts",
			},
		},
	},
        {
		/* Blade 3.3 Volts Up Minor - Blade 14 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.8.14",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 3.63 Volts",
			},
		},
	},
        {
		/* Blade 12 Volts - Blade 1 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.8.1",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+12.00 Volts",
			},
		},
	},
        {
		/* Blade 12 Volts - Blade 2 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.8.2",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+12.00 Volts",
			},
		},
	},
        {
		/* Blade 12 Volts - Blade 3 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.8.3",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+12.00 Volts",
			},
		},
	},
        {
		/* Blade 12 Volts - Blade 4 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.8.4",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+12.00 Volts",
			},
		},
	},
        {
		/* Blade 12 Volts - Blade 5 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.8.5",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+12.00 Volts",
			},
		},
	},
        {
		/* Blade 12 Volts - Blade 6 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.8.6",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+12.00 Volts",
			},
		},
	},
        {
		/* Blade 12 Volts - Blade 7 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.8.7",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+12.00 Volts",
			},
		},
	},
        {
		/* Blade 12 Volts - Blade 8 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.8.8",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+12.00 Volts",
			},
		},
	},
        {
		/* Blade 12 Volts - Blade 9 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.8.9",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+12.00 Volts",
			},
		},
	},
        {
		/* Blade 12 Volts - Blade 10 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.8.10",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+12.00 Volts",
			},
		},
	},
        {
		/* Blade 12 Volts - Blade 11 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.8.11",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+12.00 Volts",
			},
		},
	},
        {
		/* Blade 12 Volts - Blade 12 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.8.12",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+12.00 Volts",
			},
		},
	},
        {
		/* Blade 12 Volts - Blade 13 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.8.13",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+12.00 Volts",
			},
		},
	},
        {
		/* Blade 12 Volts - Blade 14 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.8.14",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+12.00 Volts",
			},
		},
	},
        {
		/* Blade 12 Volts Low Minor - Blade 1 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.11.1",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+10.80 Volts",
			},
		},
	},
        {
		/* Blade 12 Volts Low Minor - Blade 2 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.11.2",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+10.80 Volts",
			},
		},
	},
        {
		/* Blade 12 Volts Low Minor - Blade 3 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.11.3",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+10.80 Volts",
			},
		},
	},
        {
		/* Blade 12 Volts Low Minor - Blade 4 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.11.4",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+10.80 Volts",
			},
		},
	},
        {
		/* Blade 12 Volts Low Minor - Blade 5 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.11.5",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+10.80 Volts",
			},
		},
	},
        {
		/* Blade 12 Volts Low Minor - Blade 6 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.11.6",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+10.80 Volts",
			},
		},
	},
        {
		/* Blade 12 Volts Low Minor - Blade 7 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.11.7",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+10.80 Volts",
			},
		},
	},
        {
		/* Blade 12 Volts Low Minor - Blade 8 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.11.8",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+10.80 Volts",
			},
		},
	},
        {
		/* Blade 12 Volts Low Minor - Blade 9 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.11.9",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+10.80 Volts",
			},
		},
	},
        {
		/* Blade 12 Volts Low Minor - Blade 10 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.11.10",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+10.80 Volts",
			},
		},
	},
        {
		/* Blade 12 Volts Low Minor - Blade 11 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.11.11",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+10.80 Volts",
			},
		},
	},
        {
		/* Blade 12 Volts Low Minor - Blade 12 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.11.12",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+10.80 Volts",
			},
		},
	},
        {
		/* Blade 12 Volts Low Minor - Blade 13 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.11.13",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+10.80 Volts",
			},
		},
	},
        {
		/* Blade 12 Volts Low Minor - Blade 14 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.11.14",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+10.80 Volts",
			},
		},
	},
        {
		/* Blade 12 Volts Up Minor - Blade 1 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.10.1",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+13.20 Volts",
			},
		},
	},
        {
		/* Blade 12 Volts Up Minor - Blade 2 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.10.2",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+13.20 Volts",
			},
		},
	},
        {
		/* Blade 12 Volts Up Minor - Blade 3 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.10.3",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+13.20 Volts",
			},
		},
	},
        {
		/* Blade 12 Volts Up Minor - Blade 4 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.10.4",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+13.20 Volts",
			},
		},
	},
        {
		/* Blade 12 Volts Up Minor - Blade 5 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.10.5",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+13.20 Volts",
			},
		},
	},
        {
		/* Blade 12 Volts Up Minor - Blade 6 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.10.6",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+13.20 Volts",
			},
		},
	},
        {
		/* Blade 12 Volts Up Minor - Blade 7 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.10.7",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+13.20 Volts",
			},
		},
	},
        {
		/* Blade 12 Volts Up Minor - Blade 8 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.10.8",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+13.20 Volts",
			},
		},
	},
        {
		/* Blade 12 Volts Up Minor - Blade 9 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.10.9",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+13.20 Volts",
			},
		},
	},
        {
		/* Blade 12 Volts Up Minor - Blade 10 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.10.10",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+13.20 Volts",
			},
		},
	},
        {
		/* Blade 12 Volts Up Minor - Blade 11 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.10.11",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+13.20 Volts",
			},
		},
	},
        {
		/* Blade 12 Volts Up Minor - Blade 12 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.10.12",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+13.20 Volts",
			},
		},
	},
        {
		/* Blade 12 Volts Up Minor - Blade 13 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.10.13",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+13.20 Volts",
			},
		},
	},
        {
		/* Blade 12 Volts Up Minor - Blade 14 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.10.14",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+13.20 Volts",
			},
		},
	},
        {
		/* Blade 2.5 Volts - Blade 1 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.10.1",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 2.50 Volts",
			},
		},
	},
        {
		/* Blade 2.5 Volts - Blade 2 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.10.2",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 2.50 Volts",
			},
		},
	},
        {
		/* Blade 2.5 Volts - Blade 3 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.10.3",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 2.50 Volts",
			},
		},
	},
        {
		/* Blade 2.5 Volts - Blade 4 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.10.4",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 2.50 Volts",
			},
		},
	},
        {
		/* Blade 2.5 Volts - Blade 5 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.10.5",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 2.50 Volts",
			},
		},
	},
        {
		/* Blade 2.5 Volts - Blade 6 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.10.6",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 2.50 Volts",
			},
		},
	},
        {
		/* Blade 2.5 Volts - Blade 7 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.10.7",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 2.50 Volts",
			},
		},
	},
        {
		/* Blade 2.5 Volts - Blade 8 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.10.8",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 2.50 Volts",
			},
		},
	},
        {
		/* Blade 2.5 Volts - Blade 9 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.10.9",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 2.50 Volts",
			},
		},
	},
        {
		/* Blade 2.5 Volts - Blade 10 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.10.10",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 2.50 Volts",
			},
		},
	},
        {
		/* Blade 2.5 Volts - Blade 11 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.10.11",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 2.50 Volts",
			},
		},
	},
        {
		/* Blade 2.5 Volts - Blade 12 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.10.12",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 2.50 Volts",
			},
		},
	},
        {
		/* Blade 2.5 Volts - Blade 13 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.10.13",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 2.50 Volts",
			},
		},
	},
        {
		/* Blade 2.5 Volts - Blade 14 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.10.14",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 2.50 Volts",
			},
		},
	},
        {
		/* Blade 2.5 Volts Low Minor - Blade 1 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.15.1",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 2.25 Volts",
			},
		},
	},
        {
		/* Blade 2.5 Volts Low Minor - Blade 2 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.15.2",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 2.25 Volts",
			},
		},
	},
        {
		/* Blade 2.5 Volts Low Minor - Blade 3 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.15.3",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 2.25 Volts",
			},
		},
	},
        {
		/* Blade 2.5 Volts Low Minor - Blade 4 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.15.4",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 2.25 Volts",
			},
		},
	},
        {
		/* Blade 2.5 Volts Low Minor - Blade 5 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.15.5",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 2.25 Volts",
			},
		},
	},
        {
		/* Blade 2.5 Volts Low Minor - Blade 6 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.15.6",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 2.25 Volts",
			},
		},
	},
        {
		/* Blade 2.5 Volts Low Minor - Blade 7 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.15.7",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 2.25 Volts",
			},
		},
	},
        {
		/* Blade 2.5 Volts Low Minor - Blade 8 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.15.8",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 2.25 Volts",
			},
		},
	},
        {
		/* Blade 2.5 Volts Low Minor - Blade 9 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.15.9",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 2.25 Volts",
			},
		},
	},
        {
		/* Blade 2.5 Volts Low Minor - Blade 10 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.15.10",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 2.25 Volts",
			},
		},
	},
        {
		/* Blade 2.5 Volts Low Minor - Blade 11 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.15.11",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 2.25 Volts",
			},
		},
	},
        {
		/* Blade 2.5 Volts Low Minor - Blade 12 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.15.12",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 2.25 Volts",
			},
		},
	},
        {
		/* Blade 2.5 Volts Low Minor - Blade 13 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.15.13",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 2.25 Volts",
			},
		},
	},
        {
		/* Blade 2.5 Volts Low Minor - Blade 14 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.15.14",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 2.25 Volts",
			},
		},
	},
        {
		/* Blade 2.5 Volts Up Minor - Blade 1 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.14.1",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 2.95 Volts",
			},
		},
	},
        {
		/* Blade 2.5 Volts Up Minor - Blade 2 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.14.2",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 2.95 Volts",
			},
		},
	},
        {
		/* Blade 2.5 Volts Up Minor - Blade 3 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.14.3",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 2.95 Volts",
			},
		},
	},
        {
		/* Blade 2.5 Volts Up Minor - Blade 4 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.14.4",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 2.95 Volts",
			},
		},
	},
        {
		/* Blade 2.5 Volts Up Minor - Blade 5 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.14.5",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 2.95 Volts",
			},
		},
	},
        {
		/* Blade 2.5 Volts Up Minor - Blade 6 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.14.6",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 2.95 Volts",
			},
		},
	},
        {
		/* Blade 2.5 Volts Up Minor - Blade 7 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.14.7",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 2.95 Volts",
			},
		},
	},
        {
		/* Blade 2.5 Volts Up Minor - Blade 8 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.14.8",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 2.95 Volts",
			},
		},
	},
        {
		/* Blade 2.5 Volts Up Minor - Blade 9 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.14.9",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 2.95 Volts",
			},
		},
	},
        {
		/* Blade 2.5 Volts Up Minor - Blade 10 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.14.10",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 2.95 Volts",
			},
		},
	},
        {
		/* Blade 2.5 Volts Up Minor - Blade 11 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.14.11",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 2.95 Volts",
			},
		},
	},
        {
		/* Blade 2.5 Volts Up Minor - Blade 12 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.14.12",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 2.95 Volts",
			},
		},
	},
        {
		/* Blade 2.5 Volts Up Minor - Blade 13 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.14.13",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 2.95 Volts",
			},
		},
	},
        {
		/* Blade 2.5 Volts Up Minor - Blade 14 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.14.14",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 2.95 Volts",
			},
		},
	},
        {
		/* Blade 1.5 Volts - Blade 1 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.11.1",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 1.50 Volts",
			},
		},
	},
        {
		/* Blade 1.5 Volts - Blade 2 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.11.2",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 1.50 Volts",
			},
		},
	},
        {
		/* Blade 1.5 Volts - Blade 3 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.11.3",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 1.50 Volts",
			},
		},
	},
        {
		/* Blade 1.5 Volts - Blade 4 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.11.4",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 1.50 Volts",
			},
		},
	},
        {
		/* Blade 1.5 Volts - Blade 5 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.11.5",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 1.50 Volts",
			},
		},
	},
        {
		/* Blade 1.5 Volts - Blade 6 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.11.6",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 1.50 Volts",
			},
		},
	},
        {
		/* Blade 1.5 Volts - Blade 7 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.11.7",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 1.50 Volts",
			},
		},
	},
        {
		/* Blade 1.5 Volts - Blade 8 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.11.8",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 1.50 Volts",
			},
		},
	},
        {
		/* Blade 1.5 Volts - Blade 9 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.11.9",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 1.50 Volts",
			},
		},
	},
        {
		/* Blade 1.5 Volts - Blade 10 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.11.10",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 1.50 Volts",
			},
		},
	},
        {
		/* Blade 1.5 Volts - Blade 11 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.11.11",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 1.50 Volts",
			},
		},
	},
        {
		/* Blade 1.5 Volts - Blade 12 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.11.12",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 1.50 Volts",
			},
		},
	},
        {
		/* Blade 1.5 Volts - Blade 13 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.11.13",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 1.50 Volts",
			},
		},
	},
        {
		/* Blade 1.5 Volts - Blade 14 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.11.14",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 1.50 Volts",
			},
		},
	},
        {
		/* Blade 1.5 Volts Low Minor - Blade 1 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.17.1",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 1.32 Volts",
			},
		},
	},
        {
		/* Blade 1.5 Volts Low Minor - Blade 2 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.17.2",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 1.32 Volts",
			},
		},
	},
        {
		/* Blade 1.5 Volts Low Minor - Blade 3 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.17.3",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 1.32 Volts",
			},
		},
	},
        {
		/* Blade 1.5 Volts Low Minor - Blade 4 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.17.4",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 1.32 Volts",
			},
		},
	},
        {
		/* Blade 1.5 Volts Low Minor - Blade 5 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.17.5",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 1.32 Volts",
			},
		},
	},
        {
		/* Blade 1.5 Volts Low Minor - Blade 6 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.17.6",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 1.32 Volts",
			},
		},
	},
        {
		/* Blade 1.5 Volts Low Minor - Blade 7 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.17.7",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 1.32 Volts",
			},
		},
	},
        {
		/* Blade 1.5 Volts Low Minor - Blade 8 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.17.8",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 1.32 Volts",
			},
		},
	},
        {
		/* Blade 1.5 Volts Low Minor - Blade 9 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.17.9",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 1.32 Volts",
			},
		},
	},
        {
		/* Blade 1.5 Volts Low Minor - Blade 10 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.17.10",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 1.32 Volts",
			},
		},
	},
        {
		/* Blade 1.5 Volts Low Minor - Blade 11 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.17.11",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 1.32 Volts",
			},
		},
	},
        {
		/* Blade 1.5 Volts Low Minor - Blade 12 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.17.12",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 1.32 Volts",
			},
		},
	},
        {
		/* Blade 1.5 Volts Low Minor - Blade 13 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.17.13",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 1.32 Volts",
			},
		},
	},
        {
		/* Blade 1.5 Volts Low Minor - Blade 14 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.17.14",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 1.32 Volts",
			},
		},
	},
        {
		/* Blade 1.5 Volts Up Minor - Blade 1 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.16.1",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 1.68 Volts",
			},
		},
	},
        {
		/* Blade 1.5 Volts Up Minor - Blade 2 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.16.2",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 1.68 Volts",
			},
		},
	},
        {
		/* Blade 1.5 Volts Up Minor - Blade 3 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.16.3",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 1.68 Volts",
			},
		},
	},
        {
		/* Blade 1.5 Volts Up Minor - Blade 4 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.16.4",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 1.68 Volts",
			},
		},
	},
        {
		/* Blade 1.5 Volts Up Minor - Blade 5 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.16.5",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 1.68 Volts",
			},
		},
	},
        {
		/* Blade 1.5 Volts Up Minor - Blade 6 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.16.6",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 1.68 Volts",
			},
		},
	},
        {
		/* Blade 1.5 Volts Up Minor - Blade 7 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.16.7",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 1.68 Volts",
			},
		},
	},
        {
		/* Blade 1.5 Volts Up Minor - Blade 8 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.16.8",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 1.68 Volts",
			},
		},
	},
        {
		/* Blade 1.5 Volts Up Minor - Blade 9 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.16.9",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 1.68 Volts",
			},
		},
	},
        {
		/* Blade 1.5 Volts Up Minor - Blade 10 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.16.10",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 1.68 Volts",
			},
		},
	},
        {
		/* Blade 1.5 Volts Up Minor - Blade 11 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.16.11",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 1.68 Volts",
			},
		},
	},
        {
		/* Blade 1.5 Volts Up Minor - Blade 12 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.16.12",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 1.68 Volts",
			},
		},
	},
        {
		/* Blade 1.5 Volts Up Minor - Blade 13 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.16.13",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 1.68 Volts",
			},
		},
	},
        {
		/* Blade 1.5 Volts Up Minor - Blade 14 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.16.14",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 1.68 Volts",
			},
		},
	},
        {
		/* Blade 1.25 Volts - Blade 1 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.12.1",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 1.25 Volts",
			},
		},
	},
        {
		/* Blade 1.25 Volts - Blade 2 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.12.2",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 1.25 Volts",
			},
		},
	},
        {
		/* Blade 1.25 Volts - Blade 3 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.12.3",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 1.25 Volts",
			},
		},
	},
        {
		/* Blade 1.25 Volts - Blade 4 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.12.4",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 1.25 Volts",
			},
		},
	},
        {
		/* Blade 1.25 Volts - Blade 5 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.12.5",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 1.25 Volts",
			},
		},
	},
        {
		/* Blade 1.25 Volts - Blade 6 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.12.6",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 1.25 Volts",
			},
		},
	},
        {
		/* Blade 1.25 Volts - Blade 7 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.12.7",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 1.25 Volts",
			},
		},
	},
        {
		/* Blade 1.25 Volts - Blade 8 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.12.8",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 1.25 Volts",
			},
		},
	},
        {
		/* Blade 1.25 Volts - Blade 9 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.12.9",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 1.25 Volts",
			},
		},
	},
        {
		/* Blade 1.25 Volts - Blade 10 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.12.10",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 1.25 Volts",
			},
		},
	},
        {
		/* Blade 1.25 Volts - Blade 11 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.12.11",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 1.25 Volts",
			},
		},
	},
        {
		/* Blade 1.25 Volts - Blade 12 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.12.12",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 1.25 Volts",
			},
		},
	},
        {
		/* Blade 1.25 Volts - Blade 13 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.12.13",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 1.25 Volts",
			},
		},
	},
        {
		/* Blade 1.25 Volts - Blade 14 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.12.14",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 1.25 Volts",
			},
		},
	},
        {
		/* Blade 1.25 Volts Low Minor - Blade 1 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.19.1",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 1.10 Volts",
			},
		},
	},
        {
		/* Blade 1.25 Volts Low Minor - Blade 2 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.19.2",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 1.10 Volts",
			},
		},
	},
        {
		/* Blade 1.25 Volts Low Minor - Blade 3 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.19.3",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 1.10 Volts",
			},
		},
	},
        {
		/* Blade 1.25 Volts Low Minor - Blade 4 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.19.4",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 1.10 Volts",
			},
		},
	},
        {
		/* Blade 1.25 Volts Low Minor - Blade 5 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.19.5",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 1.10 Volts",
			},
		},
	},
        {
		/* Blade 1.25 Volts Low Minor - Blade 6 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.19.6",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 1.10 Volts",
			},
		},
	},
        {
		/* Blade 1.25 Volts Low Minor - Blade 7 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.19.7",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 1.10 Volts",
			},
		},
	},
        {
		/* Blade 1.25 Volts Low Minor - Blade 8 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.19.8",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 1.10 Volts",
			},
		},
	},
        {
		/* Blade 1.25 Volts Low Minor - Blade 9 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.19.9",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 1.10 Volts",
			},
		},
	},
        {
		/* Blade 1.25 Volts Low Minor - Blade 10 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.19.10",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 1.10 Volts",
			},
		},
	},
        {
		/* Blade 1.25 Volts Low Minor - Blade 11 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.19.11",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 1.10 Volts",
			},
		},
	},
        {
		/* Blade 1.25 Volts Low Minor - Blade 12 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.19.12",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 1.10 Volts",
			},
		},
	},
        {
		/* Blade 1.25 Volts Low Minor - Blade 13 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.19.13",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 1.10 Volts",
			},
		},
	},
        {
		/* Blade 1.25 Volts Low Minor - Blade 14 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.19.14",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 1.10 Volts",
			},
		},
	},
        {
		/* Blade 1.25 Volts Up Minor - Blade 1 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.18.1",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 1.40 Volts",
			},
		},
	},
        {
		/* Blade 1.25 Volts Up Minor - Blade 2 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.18.2",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 1.40 Volts",
			},
		},
	},
        {
		/* Blade 1.25 Volts Up Minor - Blade 3 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.18.3",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 1.40 Volts",
			},
		},
	},
        {
		/* Blade 1.25 Volts Up Minor - Blade 4 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.18.4",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 1.40 Volts",
			},
		},
	},
        {
		/* Blade 1.25 Volts Up Minor - Blade 5 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.18.5",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 1.40 Volts",
			},
		},
	},
        {
		/* Blade 1.25 Volts Up Minor - Blade 6 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.18.6",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 1.40 Volts",
			},
		},
	},
        {
		/* Blade 1.25 Volts Up Minor - Blade 7 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.18.7",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 1.40 Volts",
			},
		},
	},
        {
		/* Blade 1.25 Volts Up Minor - Blade 8 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.18.8",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 1.40 Volts",
			},
		},
	},
        {
		/* Blade 1.25 Volts Up Minor - Blade 9 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.18.9",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 1.40 Volts",
			},
		},
	},
        {
		/* Blade 1.25 Volts Up Minor - Blade 10 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.18.10",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 1.40 Volts",
			},
		},
	},
        {
		/* Blade 1.25 Volts Up Minor - Blade 11 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.18.11",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 1.40 Volts",
			},
		},
	},
        {
		/* Blade 1.25 Volts Up Minor - Blade 12 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.18.12",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 1.40 Volts",
			},
		},
	},
        {
		/* Blade 1.25 Volts Up Minor - Blade 13 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.18.13",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 1.40 Volts",
			},
		},
	},
        {
		/* Blade 1.25 Volts Up Minor - Blade 14 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.18.14",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 1.40 Volts",
			},
		},
	},
        {
		/* Blade VRM 1 Volts - Blade 1 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.13.1",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 1.49 Volts",
			},
		},
	},
        {
		/* Blade VRM 1 Volts - Blade 2 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.13.2",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 1.49 Volts",
			},
		},
	},
        {
		/* Blade VRM 1 Volts - Blade 3 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.13.3",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 1.49 Volts",
			},
		},
	},
        {
		/* Blade VRM 1 Volts - Blade 4 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.13.4",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 1.49 Volts",
			},
		},
	},
        {
		/* Blade VRM 1 Volts - Blade 5 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.13.5",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 1.49 Volts",
			},
		},
	},
        {
		/* Blade VRM 1 Volts - Blade 6 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.13.6",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 1.49 Volts",
			},
		},
	},
        {
		/* Blade VRM 1 Volts - Blade 7 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.13.7",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 1.49 Volts",
			},
		},
	},
        {
		/* Blade VRM 1 Volts - Blade 8 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.13.8",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 1.49 Volts",
			},
		},
	},
        {
		/* Blade VRM 1 Volts - Blade 9 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.13.9",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 1.49 Volts",
			},
		},
	},
        {
		/* Blade VRM 1 Volts - Blade 10 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.13.10",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 1.49 Volts",
			},
		},
	},
        {
		/* Blade VRM 1 Volts - Blade 11 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.13.11",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 1.49 Volts",
			},
		},
	},
        {
		/* Blade VRM 1 Volts - Blade 12 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.13.12",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 1.49 Volts",
			},
		},
	},
        {
		/* Blade VRM 1 Volts - Blade 13 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.13.13",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 1.49 Volts",
			},
		},
	},
        {
		/* Blade VRM 1 Volts - Blade 14 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.13.14",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "+ 1.49 Volts",
			},
		},
	},
        {
		/* Bower Fan Speed - Blower 1 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.3.1.0",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = " 67% of maximum",
			},
		},
	},
        {
		/* Bower Fan Speed - Blower 2 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.3.2.0",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "100% of maximum",
			},
		},
	},
        {
		/* Bower Fan Speed - Blower 3 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.3.3.0",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "  7% of maximum",
			},
		},
	},
        {
		/* Bower Fan Speed - Blower 4 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.3.4.0",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "Not Readable!",
			},
		},
	},
        {
		/* Chassis Information LED */
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.8.1.2.0",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 0, /* off=0; on=1 */
			},
		},
	},
        {
		/* Chassis Identity LED */
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.8.1.4.0",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 0, /* off=0; on=1; blinking=2; NA=3 */
			},
		},
	},
	{
		/* Blade Information LED - Blade 1 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.8.2.1.1.8.1",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 0, /* off=0; on=1 */
			},
		},
	},
	{
		/* Blade Information LED - Blade 2 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.8.2.1.1.8.2",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 0, /* off=0; on=1 */
			},
		},
	},
	{
		/* Blade Information LED - Blade 3 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.8.2.1.1.8.3",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 0, /* off=0; on=1 */
			},
		},
	},
	{
		/* Blade Information LED - Blade 4 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.8.2.1.1.8.4",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 0, /* off=0; on=1 */
			},
		},
	},
	{
		/* Blade Information LED - Blade 5 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.8.2.1.1.8.5",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 0, /* off=0; on=1 */
			},
		},
	},
	{
		/* Blade Information LED - Blade 6 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.8.2.1.1.8.6",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 0, /* off=0; on=1 */
			},
		},
	},
	{
		/* Blade Information LED - Blade 7 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.8.2.1.1.8.7",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 0, /* off=0; on=1 */
			},
		},
	},
	{
		/* Blade Information LED - Blade 8 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.8.2.1.1.8.8",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 0, /* off=0; on=1 */
			},
		},
	},
	{
		/* Blade Information LED - Blade 9 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.8.2.1.1.8.9",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 0, /* off=0; on=1 */
			},
		},
	},
	{
		/* Blade Information LED - Blade 10 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.8.2.1.1.8.10",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 0, /* off=0; on=1 */
			},
		},
	},
	{
		/* Blade Information LED - Blade 11 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.8.2.1.1.8.11",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 0, /* off=0; on=1 */
			},
		},
	},
	{
		/* Blade Information LED - Blade 12 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.8.2.1.1.8.12",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 0, /* off=0; on=1 */
			},
		},
	},
	{
		/* Blade Information LED - Blade 13 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.8.2.1.1.8.13",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 0, /* off=0; on=1 */
			},
		},
	},
	{
		/* Blade Information LED - Blade 14 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.8.2.1.1.8.14",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 0, /* off=0; on=1 */
			},
		},
	},
	{
		/* Blade Identity LED - Blade 1 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.8.2.1.1.11.1",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 0, /* off=0; on=1; blinking=2; NA=3 */
			},
		},
	},
	{
		/* Blade Identity LED - Blade 2 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.8.2.1.1.11.2",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 0, /* off=0; on=1; blinking=2; NA=3 */
			},
		},
	},
	{
		/* Blade Identity LED - Blade 3 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.8.2.1.1.11.3",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 0, /* off=0; on=1; blinking=2; NA=3 */
			},
		},
	},
	{
		/* Blade Identity LED - Blade 4 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.8.2.1.1.11.4",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 0, /* off=0; on=1; blinking=2; NA=3 */
			},
		},
	},
	{
		/* Blade Identity LED - Blade 5 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.8.2.1.1.11.5",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 0, /* off=0; on=1; blinking=2; NA=3 */
			},
		},
	},
	{
		/* Blade Identity LED - Blade 6 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.8.2.1.1.11.6",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 0, /* off=0; on=1; blinking=2; NA=3 */
			},
		},
	},
	{
		/* Blade Identity LED - Blade 7 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.8.2.1.1.11.7",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 0, /* off=0; on=1; blinking=2; NA=3 */
			},
		},
	},
	{
		/* Blade Identity LED - Blade 8 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.8.2.1.1.11.8",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 0, /* off=0; on=1; blinking=2; NA=3 */
			},
		},
	},
	{
		/* Blade Identity LED - Blade 9 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.8.2.1.1.11.9",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 0, /* off=0; on=1; blinking=2; NA=3 */
			},
		},
	},
	{
		/* Blade Identity LED - Blade 10 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.8.2.1.1.11.10",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 0, /* off=0; on=1; blinking=2; NA=3 */
			},
		},
	},
	{
		/* Blade Identity LED - Blade 11 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.8.2.1.1.11.11",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 0, /* off=0; on=1; blinking=2; NA=3 */
			},
		},
	},
	{
		/* Blade Identity LED - Blade 12 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.8.2.1.1.11.12",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 0, /* off=0; on=1; blinking=2; NA=3 */
			},
		},
	},
	{
		/* Blade Identity LED - Blade 13 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.8.2.1.1.11.13",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 0, /* off=0; on=1; blinking=2; NA=3 */
			},
		},
	},
	{
		/* Blade Identity LED - Blade 14 */
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.8.2.1.1.11.14",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 0, /* off=0; on=1; blinking=2; NA=3 */
			},
		},
	},

/* Need to test 0 length, maximum, and max +1 strings here */

        {
		/* Chassis Manufacturer VPD */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.1.1.5.0",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "SLRM",
			},
		},
	},
        {
		/* Chassis Product Name VPD */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.1.1.1.0",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "8677",
			},
		},
	},
        {
		/* Chassis Product Version VPD */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.1.1.6.0",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 5,
			},
		},
	},
        {
		/* Chassis Model Number VPD */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.1.1.2.0",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "1XX",
			},
		},
	},
        {
		/* Chassis Serial Number VPD */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.1.1.3.0",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "78F9128",
			},
		},
	},
        {
		/* Chassis Part Number VPD */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.1.1.7.0",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "59P6609",
			},
		},
	},
        {
		/* Management Module Manufacturer VPD - MM 1 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.2.1.1.3.1",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "SLRM",
			},
		},
	},
        {
		/* Management Module Manufacturer VPD - MM 2 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.2.1.1.3.2",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "SLRM",
			},
		},
	},
        {
		/* Management Module Product Version VPD - MM 1 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.2.1.1.5.1",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 4,
			},
		},
	},
        {
		/* Management Module Product Version VPD - MM 2 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.2.1.1.5.2",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 4,
			},
		},
	},
        {
		/* Management Module Part Number VPD - MM 1 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.2.1.1.4.1",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "73P9273",
			},
		},
	},
        {
		/* Management Module Part Number VPD - MM 2 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.2.1.1.4.2",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "73P9273",
			},
		},
	},
        {
		/* Switch Manufacturer VPD - Switch 1 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.6.1.1.3.1",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "DLNK",
			},
		},
	},
        {
		/* Switch Manufacturer VPD - Switch 2 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.6.1.1.3.2",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "DLNK",
			},
		},
	},
        {
		/* Switch Manufacturer VPD - Switch 3 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.6.1.1.3.3",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "DLNK",
			},
		},
	},
        {
		/* Switch Manufacturer VPD - Switch 4 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.6.1.1.3.4",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "DLNK",
			},
		},
	},
        {
		/* Switch Product Version VPD - Switch 1 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.6.1.1.5.1",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 2,
			},
		},
	},
        {
		/* Switch Product Version VPD - Switch 2 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.6.1.1.5.2",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 2,
			},
		},
	},
        {
		/* Switch Product Version VPD - Switch 3 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.6.1.1.5.3",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 2,
			},
		},
	},
        {
		/* Switch Product Version VPD - Switch 4 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.6.1.1.5.4",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 2,
			},
		},
	},
        {
		/* Switch Part Number VPD - Switch 1 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.6.1.1.4.1",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "59P6620",
			},
		},
	},
        {
		/* Switch Part Number VPD - Switch 2 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.6.1.1.4.2",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "59P6620",
			},
		},
	},
        {
		/* Switch Part Number VPD - Switch 3 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.6.1.1.4.3",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "59P6620",
			},
		},
	},
        {
		/* Switch Part Number VPD - Switch 4 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.6.1.1.4.4",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "59P6620",
			},
		},
	},
        {
		/* Blade Manufacturer VPD - Blade 1 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.3.1",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "SLRM",
			},
		},
	},
        {
		/* Blade Manufacturer VPD - Blade 2 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.3.2",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "SLRM",
			},
		},
	},
        {
		/* Blade Manufacturer VPD - Blade 3 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.3.3",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "SLRM",
			},
		},
	},
        {
		/* Blade Manufacturer VPD - Blade 4 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.3.4",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "SLRM",
			},
		},
	},
        {
		/* Blade Manufacturer VPD - Blade 5 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.3.5",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "SLRM",
			},
		},
	},
        {
		/* Blade Manufacturer VPD - Blade 6 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.3.6",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "SLRM",
			},
		},
	},
        {
		/* Blade Manufacturer VPD - Blade 7 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.3.7",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "SLRM",
			},
		},
	},
        {
		/* Blade Manufacturer VPD - Blade 8 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.3.8",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "SLRM",
			},
		},
	},
        {
		/* Blade Manufacturer VPD - Blade 9 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.3.9",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "SLRM",
			},
		},
	},
        {
		/* Blade Manufacturer VPD - Blade 10 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.3.10",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "SLRM",
			},
		},
	},
        {
		/* Blade Manufacturer VPD - Blade 11 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.3.11",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "SLRM",
			},
		},
	},
        {
		/* Blade Manufacturer VPD - Blade 12 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.3.12",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "SLRM",
			},
		},
	},
        {
		/* Blade Manufacturer VPD - Blade 13 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.3.13",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "SLRM",
			},
		},
	},
        {
		/* Blade Manufacturer VPD - Blade 14 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.3.14",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "SLRM",
			},
		},
	},
        {
		/* Blade Product Name VPD - Blade 1 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.7.1",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "867821Z",
			},
		},
	},
        {
		/* Blade Product Name VPD - Blade 2 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.7.2",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "867821Z",
			},
		},
	},
        {
		/* Blade Product Name VPD - Blade 3 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.7.3",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "867821Z",
			},
		},
	},
        {
		/* Blade Product Name VPD - Blade 4 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.7.4",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "867821Z",
			},
		},
	},
        {
		/* Blade Product Name VPD - Blade 5 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.7.5",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "867821Z",
			},
		},
	},
        {
		/* Blade Product Name VPD - Blade 6 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.7.6",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "867821Z",
			},
		},
	},
        {
		/* Blade Product Name VPD - Blade 7 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.7.7",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "867821Z",
			},
		},
	},
        {
		/* Blade Product Name VPD - Blade 8 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.7.8",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "867821Z",
			},
		},
	},
        {
		/* Blade Product Name VPD - Blade 9 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.7.9",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "867821Z",
			},
		},
	},
        {
		/* Blade Product Name VPD - Blade 10 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.7.10",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "867821Z",
			},
		},
	},
        {
		/* Blade Product Name VPD - Blade 11 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.7.11",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "867821Z",
			},
		},
	},
        {
		/* Blade Product Name VPD - Blade 12 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.7.12",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "867821Z",
			},
		},
	},
        {
		/* Blade Product Name VPD - Blade 13 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.7.13",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "867821Z",
			},
		},
	},
        {
		/* Blade Product Name VPD - Blade 14 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.7.14",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "867821Z",
			},
		},
	},
        {
		/* Blade Product Version VPD - Blade 1 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.5.1",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 0,
			},
		},
	},
        {
		/* Blade Product Version VPD - Blade 2 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.5.2",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 0,
			},
		},
	},
        {
		/* Blade Product Version VPD - Blade 3 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.5.3",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 0,
			},
		},
	},
        {
		/* Blade Product Version VPD - Blade 4 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.5.4",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 0,
			},
		},
	},
        {
		/* Blade Product Version VPD - Blade 5 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.5.5",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 0,
			},
		},
	},
        {
		/* Blade Product Version VPD - Blade 6 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.5.6",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 0,
			},
		},
	},
        {
		/* Blade Product Version VPD - Blade 7 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.5.7",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 0,
			},
		},
	},
        {
		/* Blade Product Version VPD - Blade 8 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.5.8",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 0,
			},
		},
	},
        {
		/* Blade Product Version VPD - Blade 9 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.5.9",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 0,
			},
		},
	},
        {
		/* Blade Product Version VPD - Blade 10 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.5.10",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 0,
			},
		},
	},
        {
		/* Blade Product Version VPD - Blade 11 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.5.11",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 0,
			},
		},
	},
        {
		/* Blade Product Version VPD - Blade 12 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.5.12",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 0,
			},
		},
	},
        {
		/* Blade Product Version VPD - Blade 13 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.5.13",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 0,
			},
		},
	},
        {
		/* Blade Product Version VPD - Blade 14 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.5.14",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 0,
			},
		},
	},
        {
		/* Blade Serial Number VPD - Blade 1 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.6.1",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "J1P4A28Y192",
			},
		},
	},
        {
		/* Blade Serial Number VPD - Blade 2 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.6.2",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "J1P4A28Y192",
			},
		},
	},
        {
		/* Blade Serial Number VPD - Blade 3 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.6.3",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "J1P4A28Y192",
			},
		},
	},
        {
		/* Blade Serial Number VPD - Blade 4 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.6.4",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "J1P4A28Y192",
			},
		},
	},
        {
		/* Blade Serial Number VPD - Blade 5 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.6.5",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "J1P4A28Y192",
			},
		},
	},
        {
		/* Blade Serial Number VPD - Blade 6 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.6.6",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "J1P4A28Y192",
			},
		},
	},
        {
		/* Blade Serial Number VPD - Blade 7 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.6.7",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "J1P4A28Y192",
			},
		},
	},
        {
		/* Blade Serial Number VPD - Blade 8 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.6.8",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "J1P4A28Y192",
			},
		},
	},
        {
		/* Blade Serial Number VPD - Blade 9 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.6.9",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "J1P4A28Y192",
			},
		},
	},
        {
		/* Blade Serial Number VPD - Blade 10 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.6.10",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "J1P4A28Y192",
			},
		},
	},
        {
		/* Blade Serial Number VPD - Blade 11 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.6.11",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "J1P4A28Y192",
			},
		},
	},
        {
		/* Blade Serial Number VPD - Blade 12 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.6.12",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "J1P4A28Y192",
			},
		},
	},
        {
		/* Blade Serial Number VPD - Blade 13 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.6.13",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "J1P4A28Y192",
			},
		},
	},
        {
		/* Blade Serial Number VPD - Blade 14 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.6.14",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "J1P4A28Y192",
			},
		},
	},
        {
		/* Blade Part Number VPD - Blade 1 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.4.1",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "596610",
			},
		},
	},
        {
		/* Blade Part Number VPD - Blade 2 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.4.2",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "596610",
			},
		},
	},
        {
		/* Blade Part Number VPD - Blade 3 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.4.3",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "596610",
			},
		},
	},
        {
		/* Blade Part Number VPD - Blade 4 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.4.4",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "596610",
			},
		},
	},
        {
		/* Blade Part Number VPD - Blade 5 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.4.5",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "596610",
			},
		},
	},
        {
		/* Blade Part Number VPD - Blade 6 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.4.6",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "596610",
			},
		},
	},
        {
		/* Blade Part Number VPD - Blade 7 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.4.7",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "596610",
			},
		},
	},
        {
		/* Blade Part Number VPD - Blade 8 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.4.8",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "596610",
			},
		},
	},
        {
		/* Blade Part Number VPD - Blade 9 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.4.9",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "596610",
			},
		},
	},
        {
		/* Blade Part Number VPD - Blade 10 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.4.10",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "596610",
			},
		},
	},
        {
		/* Blade Part Number VPD - Blade 11 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.4.11",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "596610",
			},
		},
	},
        {
		/* Blade Part Number VPD - Blade 12 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.4.12",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "596610",
			},
		},
	},
        {
		/* Blade Part Number VPD - Blade 13 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.4.13",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "596610",
			},
		},
	},
        {
		/* Blade Part Number VPD - Blade 14 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.4.14",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "596610",
			},
		},
	},
        {
		/* Blade Add-on Manufacturer VPD - Blade Add-on 1 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.10.1",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "SLRM",
			},
		},
	},
        {
		/* Blade Add-on Manufacturer VPD - Blade Add-on 2 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.10.2",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "SLRM",
			},
		},
	},
        {
		/* Blade Add-on Manufacturer VPD - Blade Add-on 3 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.10.3",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "SLRM",
			},
		},
	},
        {
		/* Blade Add-on Manufacturer VPD - Blade Add-on 4 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.10.4",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "SLRM",
			},
		},
	},
        {
		/* Blade Add-on Manufacturer VPD - Blade Add-on 5 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.10.5",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "SLRM",
			},
		},
	},
        {
		/* Blade Add-on Manufacturer VPD - Blade Add-on 6 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.10.6",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "SLRM",
			},
		},
	},
        {
		/* Blade Add-on Manufacturer VPD - Blade Add-on 7 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.10.7",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "SLRM",
			},
		},
	},
        {
		/* Blade Add-on Manufacturer VPD - Blade Add-on 8 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.10.8",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "SLRM",
			},
		},
	},
        {
		/* Blade Add-on Manufacturer VPD - Blade Add-on 9 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.10.9",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "SLRM",
			},
		},
	},
        {
		/* Blade Add-on Manufacturer VPD - Blade Add-on 10 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.10.10",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "SLRM",
			},
		},
	},
        {
		/* Blade Add-on Manufacturer VPD - Blade Add-on 11 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.10.11",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "SLRM",
			},
		},
	},
        {
		/* Blade Add-on Manufacturer VPD - Blade Add-on 12 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.10.12",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "SLRM",
			},
		},
	},
        {
		/* Blade Add-on Manufacturer VPD - Blade Add-on 13 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.10.13",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "SLRM",
			},
		},
	},
        {
		/* Blade Add-on Manufacturer VPD - Blade Add-on 14 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.10.14",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "SLRM",
			},
		},
	},
        {
		/* Blade Add-on Product Name VPD - Blade Add-on 1 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.9.1",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "867821Z",
			},
		},
	},
        {
		/* Blade Add-on Product Name VPD - Blade Add-on 2 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.9.2",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "867821Z",
			},
		},
	},
        {
		/* Blade Add-on Product Name VPD - Blade Add-on 3 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.9.3",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "867821Z",
			},
		},
	},
        {
		/* Blade Add-on Product Name VPD - Blade Add-on 4 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.9.4",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "867821Z",
			},
		},
	},
        {
		/* Blade Add-on Product Name VPD - Blade Add-on 5 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.9.5",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "867821Z",
			},
		},
	},
        {
		/* Blade Add-on Product Name VPD - Blade Add-on 6 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.9.6",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "867821Z",
			},
		},
	},
        {
		/* Blade Add-on Product Name VPD - Blade Add-on 7 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.9.7",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "867821Z",
			},
		},
	},
        {
		/* Blade Add-on Product Name VPD - Blade Add-on 8 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.9.8",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "867821Z",
			},
		},
	},
        {
		/* Blade Add-on Product Name VPD - Blade Add-on 9 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.9.9",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "867821Z",
			},
		},
	},
        {
		/* Blade Add-on Product Name VPD - Blade Add-on 10 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.9.10",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "867821Z",
			},
		},
	},
        {
		/* Blade Add-on Product Name VPD - Blade Add-on 11 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.9.11",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "867821Z",
			},
		},
	},
        {
		/* Blade Add-on Product Name VPD - Blade Add-on 12 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.9.12",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "867821Z",
			},
		},
	},
        {
		/* Blade Add-on Product Name VPD - Blade Add-on 13 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.9.13",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "867821Z",
			},
		},
	},
        {
		/* Blade Add-on Product Name VPD - Blade Add-on 14 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.9.14",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "867821Z",
			},
		},
	},
        {
		/* Blade Add-on Product Version VPD - Blade Add-on 1 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.12.1",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 0,
			},
		},
	},
        {
		/* Blade Add-on Product Version VPD - Blade Add-on 2 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.12.2",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 0,
			},
		},
	},
        {
		/* Blade Add-on Product Version VPD - Blade Add-on 3 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.12.3",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 0,
			},
		},
	},
        {
		/* Blade Add-on Product Version VPD - Blade Add-on 4 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.12.4",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 0,
			},
		},
	},
        {
		/* Blade Add-on Product Version VPD - Blade Add-on 5 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.12.5",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 0,
			},
		},
	},
        {
		/* Blade Add-on Product Version VPD - Blade Add-on 6 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.12.6",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 0,
			},
		},
	},
        {
		/* Blade Add-on Product Version VPD - Blade Add-on 7 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.12.7",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 0,
			},
		},
	},
        {
		/* Blade Add-on Product Version VPD - Blade Add-on 8 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.12.8",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 0,
			},
		},
	},
        {
		/* Blade Add-on Product Version VPD - Blade Add-on 9 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.12.9",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 0,
			},
		},
	},
        {
		/* Blade Add-on Product Version VPD - Blade Add-on 10 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.12.10",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 0,
			},
		},
	},
        {
		/* Blade Add-on Product Version VPD - Blade Add-on 11 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.12.11",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 0,
			},
		},
	},
        {
		/* Blade Add-on Product Version VPD - Blade Add-on 12 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.12.12",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 0,
			},
		},
	},
        {
		/* Blade Add-on Product Version VPD - Blade Add-on 13 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.12.13",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 0,
			},
		},
	},
        {
		/* Blade Add-on Product Version VPD - Blade Add-on 14 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.12.14",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 0,
			},
		},
	},
        {
		/* Blade Add-on Part Number VPD - Blade Add-on 1 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.11.1",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "596610",
			},
		},
	},
        {
		/* Blade Add-on Part Number VPD - Blade Add-on 2 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.11.2",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "596610",
			},
		},
	},
        {
		/* Blade Add-on Part Number VPD - Blade Add-on 3 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.11.3",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "596610",
			},
		},
	},
        {
		/* Blade Add-on Part Number VPD - Blade Add-on 4 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.11.4",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "596610",
			},
		},
	},
        {
		/* Blade Add-on Part Number VPD - Blade Add-on 5 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.11.5",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "596610",
			},
		},
	},
        {
		/* Blade Add-on Part Number VPD - Blade Add-on 6 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.11.6",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "596610",
			},
		},
	},
        {
		/* Blade Add-on Part Number VPD - Blade Add-on 7 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.11.7",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "596610",
			},
		},
	},
        {
		/* Blade Add-on Part Number VPD - Blade Add-on 8 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.11.8",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "596610",
			},
		},
	},
        {
		/* Blade Add-on Part Number VPD - Blade Add-on 9 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.11.9",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "596610",
			},
		},
	},
        {
		/* Blade Add-on Part Number VPD - Blade Add-on 10 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.11.10",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "596610",
			},
		},
	},
        {
		/* Blade Add-on Part Number VPD - Blade Add-on 11 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.11.11",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "596610",
			},
		},
	},
        {
		/* Blade Add-on Part Number VPD - Blade Add-on 12 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.11.12",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "596610",
			},
		},
	},
        {
		/* Blade Add-on Part Number VPD - Blade Add-on 13 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.11.13",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "596610",
			},
		},
	},
        {
		/* Blade Add-on Part Number VPD - Blade Add-on 14 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.11.14",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "596610",
			},
		},
	},
        {
		/* Media Tray Manufacturer VPD */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.9.3.0",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "SLRM",
			},
		},
	},
        {
		/* Media Tray Product Version VPD */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.9.5.0",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 5,
			},
		},
	},
        {
		/* Media Tray Part Number VPD */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.9.4.0",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "59P6609",
			},
		},
	},
        {
		/* Power Module Manufacturer VPD - PM 1 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.8.1.1.3.1",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "SLRM",
			},
		},
	},
        {
		/* Power Module Manufacturer VPD - PM 2 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.8.1.1.3.2",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "SLRM",
			},
		},
	},
        {
		/* Power Module Manufacturer VPD - PM 3 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.8.1.1.3.3",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "SLRM",
			},
		},
	},
        {
		/* Power Module Manufacturer VPD - PM 4 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.8.1.1.3.4",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "SLRM",
			},
		},
	},
        {
		/* Power Module Product Version VPD - PM 1 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.8.1.1.5.1",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 5,
			},
		},
	},
        {
		/* Power Module Product Version VPD - PM 2 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.8.1.1.5.2",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 5,
			},
		},
	},
        {
		/* Power Module Product Version VPD - PM 3 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.8.1.1.5.3",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 5,
			},
		},
	},
        {
		/* Power Module Product Version VPD - PM 4 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.8.1.1.5.4",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 5,
			},
		},
	},
        {
		/* Power Module Part Number VPD - PM 1 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.8.1.1.4.1",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "59P6609",
			},
		},
	},
        {
		/* Power Module Part Number VPD - PM 2 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.8.1.1.4.2",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "59P6609",
			},
		},
	},
        {
		/* Power Module Part Number VPD - PM 3 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.8.1.1.4.3",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "59P6609",
			},
		},
	},
        {
		/* Power Module Part Number VPD - PM 4 */	
		.oid = ".1.3.6.1.4.1.2.3.51.2.2.21.8.1.1.4.4",
		.mib = {
			.type = ASN_OCTET_STR,
			.value = {
				.string = "59P6609",
			},
		},
	},

	{} /* Terminate array with a null element */
};
