/*      -*- linux-c -*-
 *
 * (C) Copyright IBM Corp. 2003, 2006
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. This
 * file and program are licensed under a BSD style license. See
 * the Copying file included with the OpenHPI distribution for
 * full licensing terms.
 *
 * Author(s):
 *      Steve Sherman <stevees@us.ibm.com>
 */

/**************************************************************************
 * This source file defines the resource arrays declared in sim_resources.h
 *************************************************************************/

#include <glib.h>
#include <snmp_utils.h>
#include <sim_resources.h>

struct snmp_bc_data sim_resource_array[] = {
        {
                /* TIMEOUT
		 * This OID is used to force a SNMP Access timeout condition.
		 * It is used to test Device Busy/Device Not Respond  
		 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.4.4.1.7777",
                .mib = {
                        .type = ASN_INTEGER,
                        .value = {
                                .integer = SNMP_FORCE_TIMEOUT,
                        },
                },
        },
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
        {
                /* Chassis Type 
                 *
                 * This OID is used to determine if the chassis type. Only available on
                 * newer levels of BladeCenter code.
                 * If integer == 97 system is a BC; if integer = 98 system in BCT
                 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.4.38.0",
                .mib = {
                        .type = ASN_INTEGER,
                        .value = {
                                .integer = 98,
                        },
                },
        },
        {
                /* Chassis Subtype 
                 *
                 * This OID is used to determine if the chassis subtype. Only available on
                 * newer levels of BladeCenter code.
                 * If integer == 0, its the orignal system; 2 its the H models 
                 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.4.39.0",
                .mib = {
                        .type = ASN_INTEGER,
                        .value = {
                                .integer = 0,
                        },
                },
        },
        {
                /* RSA Health
                 *
                 * This OID is used to determine if the system is a RSA or not
                 * If integer == SA_ERR_SNMP_NOSUCHOBJECT, platform is not an RSA.
		 * Code checks for RSA platform before it checks for BC platform
                 * type.
                 */
                .oid = ".1.3.6.1.4.1.2.3.51.1.2.7.1.0",
                .mib = {
                        .type = ASN_INTEGER,
                        .value = {
				.integer = SA_ERR_SNMP_NOSUCHOBJECT,  /* 255 = RSA */
                        },
                },
        },
        {
                /* BCT System Health Status
                 *
                 * This OID is used to determine if the system is a BCT or not.
                 * If integer == 255 system is a BCT; if integer = SA_ERR_SNMP_NOSUCHOBJECT
		 * system is not a BCT; Need to coordinate with BCI Health value below.
                 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.2.9.1.0",
                .mib = {
                        .type = ASN_INTEGER,
                        .value = {
                                .integer = SA_ERR_SNMP_NOSUCHOBJECT,
                                /* .integer = 255, BCT */
                        },
                },
        },
        {
                /* TimeZone - DayLight Savings Time */
                .oid = ".1.3.6.1.4.1.2.3.51.2.4.4.2.0",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+0:00,no",
                        },
                },
        },
	
		/* 
		 * OIDs definitions for Blade Center Chassis Topology 
		 */
	{
		/* SNMP_BC_NOS_FP_SUPPORTED mmblade.mib - chassisNoOfFPsSupported, FanPack */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.4.18.0",
                .mib = {
                        .type = ASN_INTEGER,
                        .value = {
                                .integer = 2,
                        },
                },
        },
	{ 
		/* SNMP_BC_NOS_PB_SUPPORTED mmblade.mib - chassisNoOfPBsSupported, ProcessorBlade */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.4.19.0",
                .mib = {
                        .type = ASN_INTEGER,
                        .value = {
                                .integer = 14,
                        },
                },
        },
	{
		/* SNMP_BC_NOS_SM_SUPPORTED mmblade.mib - chassisNoOfSMsSupported, SwitchModule */ 
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.4.20.0",
                .mib = {
                        .type = ASN_INTEGER,
                        .value = {
                                .integer = 4,
                        },
                },
        },
	{
		/* SNMP_BC_NOS_MM_SUPPORTED mmblade.mib - chassisNoOfMMsSupported, ManagementModule */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.4.21.0",
                .mib = {
                        .type = ASN_INTEGER,
                        .value = {
                                .integer = 2,
                        },
                },
        },
	{
		/* SNMP_BC_NOS_PM_SUPPORTED mmblade.mib - chassisNoOfPMsSupported, PowerModule */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.4.22.0",
                .mib = {
                        .type = ASN_INTEGER,
                        .value = {
                                .integer = 4,
                        },
                },
        },
	{
		/* SNMP_BC_NOS_MT_SUPPORTED mmblade.mib - chassisNoOfMTsSupported, MediaTray */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.4.23.0", 
                .mib = {
                        .type = ASN_INTEGER,
                        .value = {
                                .integer = 1,
                        },
                },
        },
	{
		/* SNMP_BC_NOS_BLOWER_SUPPORTED mmblade.mib - chassisNoOfBlowersSupported, Blower */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.4.24.0",
                .mib = {
                        .type = ASN_INTEGER,
                        .value = {
                                .integer = 4,
                        },
                },
        },
	{
		/* SNMP_BC_PB_INSTALLED mmblade.mib - chassisPBsInstalled, ProcessorBlade */ 	
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.4.25.0",	
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "10101010101010",
                        },
                },
        },
	{
		/* SNMP_BC_SM_INSTALLED mmblade.mib - chassisSMsInstalled, SwitchModule */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.4.29.0",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "1111",
                        },
                },
        },
	{
		/* SNMP_BC_MM_INSTALLED mmblade.mib - chassisMMsInstalled, ManagementModule */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.4.30.0",	
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "11",
                        },
                },
        },
	{
		/* SNMP_BC_PM_INSTALLED mmblade.mib - chassisPMsInstalled, PowerModule */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.4.31.0",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "1111",
                        },
                },
        },
	{
		/* SNMP_BC_MT_INSTALLED mmblade.mib - chassisMTsInstalled, MediaTray */	
		.oid = ",1.3.6.1.4.1.2.3.51.2.22.4.32.0",
                .mib = {
                        .type = ASN_INTEGER,
                        .value = {
                                .integer = 1,
                        },
                },
        }, 
	{
		/* SNMP_BC_BLOWER_INSTALLED mmblade.mib - chassisBlowersInstalled, Blower */
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.4.33.0",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "1111",
                        },
                },
        },
	{
		/* SNMP_BC_FP_INSTALLED  mmblade.mib - chassisFPsinstalled, FanPack */ 
		.oid = ".1.3.6.1.4.1.2.3.51.2.22.4.37.0",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "11",
                        },
                },
        },	
        /* NOTE:: Must have one plus the END of Log Entry for each
           event in the simulator's event log */
        {
                /* Event Log Index Number for Event 1 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.3.4.2.1.1.1",
                .mib = {
                        .type = ASN_INTEGER,
                        .value = {
                                .integer = 1,
                        },
                },
        },
	{
                /* Event Log Index Number for Event 2 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.3.4.2.1.1.2",
                .mib = {
                        .type = ASN_INTEGER,
                        .value = {
                                .integer = 1,
                        },
                },
        },
        {
                /* Event Log Index Number for Event 3 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.3.4.2.1.1.3",
                .mib = {
                        .type = ASN_INTEGER,
                        .value = {
                                .integer = 1,
                        },
                },
        },
        {
                /* Event Log Index Number for Event 4 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.3.4.2.1.1.4",
                .mib = {
                        .type = ASN_INTEGER,
                        .value = {
                                .integer = 1,
                        },
                },
        },
	/* Event Log Index Number for Event 5 */
	{
		.oid = ".1.3.6.1.4.1.2.3.51.2.3.4.2.1.1.5",
		.mib = {
			.type = ASN_INTEGER,
			.value = {
				.integer = 1,
			},
		},
        },
	/*
	 * Special End of Log Entry - Simulator ONLY
	 * Code always reads one SNMP OID past end of log. When
	 * snmp_get returns a negative value, the code knows its read
	 * the entire error log. This entry allows the simulator to
	 * force the snmp_get to return a negative value
	 */
        {
                .oid = ".1.3.6.1.4.1.2.3.51.2.3.4.2.1.1.6",
                .mib = {
                        .type = ASN_INTEGER,
                        .value = {
                                .integer = SNMP_FORCE_ERROR, /* Force negative return */
                        },
                },
        },
        {
                /* Event Log Message */
                .oid = ".1.3.6.1.4.1.2.3.51.2.3.4.2.1.2.1",
                .mib = {
                        .type = ASN_OCTET_STR,
			.value = {
				.string = "Severity:ERR  Source:BLADE_01  Name:SN#ZJ1R6G5932JX  Date:11/19/05  Time:14:13:15  Text:CPU 3 ",
			},
		},
        },
        {
                /* Event Log Message */
                .oid = ".1.3.6.1.4.1.2.3.51.2.3.4.2.1.2.2",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
				.string = "Severity:WARN  Source:BLADE_01  Name:SN#ZJ1R6G5932JX  Date:11/19/05  Time:16:49:42  Text:System shutoff due to VRM 1 over voltage.  Read value 247.01. Threshold value. 0.",
                       },
                },
        },
        {
                /* Event Log Message */
                .oid = ".1.3.6.1.4.1.2.3.51.2.3.4.2.1.2.3",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
 				.string = "Severity:WARN  Source:BLADE_01  Name:SN#ZJ1R6G5932JX  Date:11/19/05  Time:16:49:42  Text:System shutoff due to VRM 1 over voltage.  Read value 247.01. Threshold value. 0.",
                     },
                },
	},
        {
                /* Event Log Message */
                .oid = ".1.3.6.1.4.1.2.3.51.2.3.4.2.1.2.4",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
				.string = "Severity:WARN  Source:BLADE_01  Name:SN#ZJ1R6G5932JX  Date:11/19/05  Time:16:49:42  Text:System shutoff due to VRM 1 over voltage.  Read value 247.01. Threshold value. 0.",
                      },
                },
	},
        {
                /* Event Log Message */
                .oid = ".1.3.6.1.4.1.2.3.51.2.3.4.2.1.2.5",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
				.string = "Severity:WARN  Source:BLADE_01  Name:SN#ZJ1R6G5932JX  Date:11/19/05  Time:16:49:42  Text:System shutoff due to VRM 1 over voltage.  Read value 247.01. Threshold value. 0.",
                      },
                },
	},
        {
                .oid = ".1.3.6.1.4.1.2.3.51.2.3.4.2.1.2.6",
                .mib = {
                        .type = ASN_INTEGER,
                        .value = {
                                .integer = SNMP_FORCE_ERROR, /* Force negative return */
                        },
                },
        },

#if 0
	.string = "Severity:ERR  Source:BLADE_02  Name:SN#ZJ1R6G5931XY  Date:11/19/05  Time:17:26:32  Text:Critical Interrupt - Front panel NMI",

	.string = "Severity:WARN  Source:BLADE_01  Name:SN#ZJ1R6G5932JX  Date:11/19/05  Time:16:49:42  Text:System shutoff due to VRM 1 over voltage.  Read value 247.01. Threshold value. 0.",

	.string = "Severity:INFO  Source:SERVPROC  Name:SN#              Date:11/19/05  Time:15:34:05  Text:Management Module 2 was removed.",

	.string = "Severity:ERR  Source:BLADE_01  Name:SN#ZJ1R6G5932JX  Date:11/19/05  Time:14:13:15  Text:CPU 3 shut off due to over temperature ",

	.string = "Severity:ERR  Source:SERVPROC  Name:SN#              Date:11/19/05  Time:12:27:43  Text:Blower 1 Fault Single blower failure",

	.string = "Severity:INFO  Source:SERVPROC  Name:SN#              Date:11/19/05  Time:12:17:51  Text:Management Module 2 was removed.",

	.string "Severity:WARN  Source:SERVPROC  Name:SN#              Date:11/19/05  Time:10:56:35  Text:Management Module network uplink loss.",
.
	.string = "Severity:ERR  Source:SERVPROC  Name:SN#              Date:11/18/05  Time:21:46:49  Text:Blower 1 Failure Single blower failure",

	.string = "Severity:WARN  Source:BLADE_01  Name:SN#ZJ1R6G5932JX  Date:11/18/05  Time:18:15:47  Text:System over temperature for CPU 4.   Read value. 0. Threshold value. 0.",

	.string = "Severity:INFO  Source:SERVPROC  Name:SN#              Date:11/18/05  Time:18:15:47  Text:TAM MNR alert for Event (ID = 0x0421d504) System over temperature for CPU 4.",

	.string = "Severity:ERR  Source:BLADE_01  Name:SN#ZJ1R6G5932JX  Date:11/18/05  Time:18:14:37  Text:System shutoff due to CPU 3 over temperature.   Read value 247.01. Threshold value. 0.",

	.string = "Severity:ERR  Source:SERVPROC  Name:SN#              Date:11/18/05  Time:17:44:41  Text:CPU 1 shut off due to over temperature ",
.
	.string = "Severity:ERR  Source:BLADE_01  Name:SN#ZJ1R6G5932JX  Date:11/18/05  Time:17:15:15  Text:Planar voltage fault.  Read value. 0. Threshold value. 0.",

	.string = "Severity:ERR  Source:BLADE_01  Name:SN#ZJ1R6G5932JX  Date:11/18/05  Time:16:43:46  Text:IO Board voltage fault.  Read value. 0. Threshold value. 0.",

	.string =  "Severity:WARN  Source:BLADE_01  Name:SN#ZJ1R6G5932JX  Date:11/18/05  Time:15:26:02  Text:System shutoff due to VRM 1 over voltage.  Read value 247.01. Threshold value. 0.",

	.string = "Severity:ERR  Source:BLADE_01  Name:SN#ZJ1R6G5932JX  Date:11/17/05  Time:14:08:22  Text:BEM +1.5V Fault.  Read value 247.01. Threshold value. 0.",

	.string = "Severity:ERR  Source:BLADE_01  Name:SN#ZJ1R6G5932JX  Date:11/17/05  Time:14:08:10  Text:BEM Option failure ",

	.string = "Severity:ERR  Source:BLADE_01  Name:SN#ZJ1R6G5932JX  Date:11/17/05  Time:14:08:22  Text:BEM +1.5V Fault.  Read value 247.01. Threshold value. 0.",

	.string = "Severity:ERR  Source:BLADE_01  Name:SN#ZJ1R6G5932JX  Date:11/17/05  Time:14:08:10  Text:BEM Option failure ",

	.string = "Severity:INFO  Source:SERVPROC  Name:SN#              Date:11/12/05  Time:21:39:10  Text:Management Module in bay 1 is Active.",

	.string = "Severity:ERR  Source:SERVPROC  Name:bct-33  Date:11/12/05  Time:21:29:08  Text:Blower 1 Fault Single blower failure",

	.string = "Severity:WARN  Source:SERVPROC  Name:SN#              Date:11/13/05  Time:16:19:07  Text:Power modules are nonredundant in domain 2",

	.string = "Severity:ERR  Source:SERVPROC  Name:SN#              Date:11/13/05  Time:16:19:07  Text:Power Supply 4 Removed.",
#endif
        {
                /* Clear Event Log */
                .oid = ".1.3.6.1.4.1.2.3.51.2.3.4.3.0",
                .mib = {
                        .type = ASN_INTEGER,
                        .value = {
                                .integer = 1, /* write-only */
                        },
                },
        },
        {
                /* SNMP_BC_BLADE_VECTOR */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.4.25.0",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "10101010101010",
                        },
                },
        },
        {
                /* SNMP_BC_FAN_VECTOR */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.4.33.0",
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
                                .integer = 1, /* number of active MM */
                        },
                },
        },
        {
                /* SNMP_BC_MEDIA_TRAY_EXISTS */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.4.32.0",
                .mib = {
                        .type = ASN_INTEGER,
                        .value = {
                                .integer = 1, /* 0=no; 1=yes */
                        },
                },
        },
        {
                /* SNMP_BC_POWER_VECTOR */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.4.31.0",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "1111",
                        },
                },
        },
        {
                /* SNMP_BC_SWITCH_VECTOR */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.4.29.0",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "1111",
                        },
                },
        },
        {
                /*  Management module - Reset */
                .oid = ".1.3.6.1.4.1.2.3.51.2.7.4.0",
                .mib = {
                        .type = ASN_INTEGER,
                        .value = {
                                .integer = 1, /* 1=execute */
                        },
                },
        },
        {
                /*  Management module - Failover */
                .oid = ".1.3.6.1.4.1.2.3.51.2.7.7.0",
                .mib = {
                        .type = ASN_INTEGER,
                        .value = {
                                .integer = 0, /* 1=execute */
                        },
                },
        },
        {
                /* Management module 1 UUID */
                .oid = ".1.3.6.1.4.1.2.3.51.2.2.21.2.1.1.6.1",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "0000 0000 0000 0000 0000 0000 0000 0000"
                        },
                },
        },
        {
                /* Management module 2 UUID */
                .oid = ".1.3.6.1.4.1.2.3.51.2.2.21.2.1.1.6.2",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "Not available"
                        },
                },
        },
        {
                /* Switch Reset - Switch 1 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.3.1.7.1.8.1", /* write-only */
                .mib = {
                        .type = ASN_INTEGER,
                        .value = {
                                .integer = 1, /* 1=execute */
                        },
                },
        },
        {
                /* Switch 1 UUID */
                .oid = ".1.3.6.1.4.1.2.3.51.2.2.21.6.1.1.8.1",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "EC4E 1D7C 704B 11D7 B69E 0005 5D89 A738 "
                        },
                },
        },
        {
                /* Switch Reset - Switch 2 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.3.1.7.1.8.2", /* write-only */
                .mib = {
                        .type = ASN_INTEGER,
                        .value = {
                                .integer = 1, /* 1=execute */
                        },
                },
        },
        {
                /* Switch 2 UUID */
                .oid = ".1.3.6.1.4.1.2.3.51.2.2.21.6.1.1.8.2",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = { // intentional error
                                .string = "No value available"
                        },
                },
        },
        {
                /* Switch Reset - Switch 3 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.3.1.7.1.8.3", /* write-only */
                .mib = {
                        .type = ASN_INTEGER,
                        .value = {
                                .integer = 1, /* 1=execute */
                        },
                },
        },
        {
                /* Switch 3 UUID */
                .oid = ".1.3.6.1.4.1.2.3.51.2.2.21.6.1.1.8.3",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "21DA E982 1B9E 95B8 7821 00C0 DD01 C65A "
                        },
                },
        },
        {
                /* Switch Reset - Switch 4 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.3.1.7.1.8.4", /* write-only */
                .mib = {
                        .type = ASN_INTEGER,
                        .value = {
                                .integer = 1, /* 1=execute */
                        },
                },
        },
        {
                /* Switch 4 UUID */
                .oid = ".1.3.6.1.4.1.2.3.51.2.2.21.6.1.1.8.4",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = { // intentional error
                                .string = "                                        "
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
        {
                /* Blade 1 UUID */
                .oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.8.1",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "D63F A294 1BB4 4A12 9D42 48D0 BE6A 3A20 "
                        },
                },
        },
        {
                /* Blade 2 UUID */
                .oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.8.2",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "Not available"
                        },
                },
        },
        {
                /* Blade 3 UUID */
                .oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.8.3",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {  // intentional error
                                .string = "06DC 3B85 D61D B211 8576 8CF2 8F52 "
                        },
                },
        },
        {
                /* Blade 4 UUID */
                .oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.8.4",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "21DA-E8E3-8C36-22E2-92E1-00C0-DD01-C53C "
                        },
                },
        },
        {
                /* Blade 5 UUID */
                .oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.8.5",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = { // intentional error
                                .string = "  Z4EE EE02 24B4 4A12 9B01 D094 209B 532C  "
                        },
                },
        },
        {
                /* Blade 6 UUID */
                .oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.8.6",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "Not available"
                        },
                },
        },
        {
                /* Blade 7 UUID */
                .oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.8.7",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "64EE EE02 24B4 4A12 9B01 D094 209B 532C "
                        },
                },
        },
        {
                /* Blade 8 UUID */
                .oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.8.8",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "Not available"
                        },
                },
        },
        {
                /* Blade 9 UUID */
                .oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.8.9",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                               .string = "Not available" 
                        },
                },
        },
        {
                /* Blade 10 UUID */
                .oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.8.10",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "Not available"
                        },
                },
        },
        {
                /* Blade 11 UUID */
                .oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.8.11",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "Not available"
                        },
                },
        },
        {
                /* Blade 12 UUID */
                .oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.8.12",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                               .string = "Not available" 
                        },
                },
        },
        {
                /* Blade 13 UUID */
                .oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.8.13",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "Not available" 
                        },
                },
        },
        {
                /* Blade 14 UUID */
                .oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.8.14",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "Not available"
                        },
                },
        },
        {
                /* Media Tray UUID */
                .oid = ".1.3.6.1.4.1.2.3.51.2.2.21.9.8.0",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "0000 0000 0000 0000 0000 0000 0000 0000 "
                        },
                },
        },
        {
                /* Power module 1 UUID */
                .oid = ".1.3.6.1.4.1.2.3.51.2.2.21.8.1.1.8.1",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "22C2 2ADF 51A4 11D7 004B 0090 0005 0047 "
                        },
                },
        },
        {
                /* Power module 2 UUID */
                .oid = ".1.3.6.1.4.1.2.3.51.2.2.21.8.1.1.8.2",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = " B62965A8 6EB2 11D7 00D5 007400B00020 "
                        },
                },
        },
        {
                /* Power module 3 UUID */
                .oid = ".1.3.6.1.4.1.2.3.51.2.2.21.8.1.1.8.3",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "B62965A8-6EB2-11D7-00D5-007400B00020 "
                        },
                },
        },
        {
                /* Power module 4 UUID */
                .oid = ".1.3.6.1.4.1.2.3.51.2.2.21.8.1.1.8.4",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "3D2E 1265 6EB2 11D7 001C 007F 00C7 00A5 "
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
                /* BCT ONLY (BC will add on its next release)
                 *
                 * Blade 1 Number of reboots - bootCountPowerOnTimeBoots
                 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.10.1.1.3.1",
                .mib = {
                        .type = ASN_INTEGER,
                        .value = {
                                .integer = 1,
                        },
                },
        },
        {
                /* BCT ONLY (BC will add on its next release)
                 *
                 * Blade 2 Number of reboots - bootCountPowerOnTimeBoots
                 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.10.1.1.3.2",
                .mib = {
                        .type = ASN_INTEGER,
                        .value = {
                                .integer = 2,
                        },
                },
        },
        {
                /* BCT ONLY (BC will add on its next release)
                 *
                 * Blade 3 Number of reboots - bootCountPowerOnTimeBoots
                 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.10.1.1.3.3",
                .mib = {
                        .type = ASN_INTEGER,
                        .value = {
                                .integer = 3,
                        },
                },
        },
        {
                /* BCT ONLY (BC will add on its next release)
                 *
                 * Blade 4 Number of reboots - bootCountPowerOnTimeBoots
                 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.10.1.1.3.4",
                .mib = {
                        .type = ASN_INTEGER,
                        .value = {
                                .integer = 4,
                        },
                },
        },
        {
                /* BCT ONLY (BC will add on its next release)
                 *
                 * Blade 5 Number of reboots - bootCountPowerOnTimeBoots
                 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.10.1.1.3.5",
                .mib = {
                        .type = ASN_INTEGER,
                        .value = {
                                .integer = 5,
                        },
                },
        },
        {
                /* BCT ONLY (BC will add on its next release)
                 *
                 * Blade 6 Number of reboots - bootCountPowerOnTimeBoots
                 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.10.1.1.3.6",
                .mib = {
                        .type = ASN_INTEGER,
                        .value = {
                                .integer = 6,
                        },
                },
        },
        {
                /* BCT ONLY (BC will add on its next release)
                 *
                 * Blade 7 Number of reboots - bootCountPowerOnTimeBoots
                 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.10.1.1.3.7",
                .mib = {
                        .type = ASN_INTEGER,
                        .value = {
                                .integer = 7,
                        },
                },
        },
        {
                /* BCT ONLY (BC will add on its next release)
                 *
                 * Blade 8 Number of reboots - bootCountPowerOnTimeBoots
                 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.10.1.1.3.8",
                .mib = {
                        .type = ASN_INTEGER,
                        .value = {
                                .integer = 8,
                        },
                },
        },
        {
                /* BCT ONLY (BC will add on its next release)
                 *
                 * Blade 9 Number of reboots - bootCountPowerOnTimeBoots
                 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.10.1.1.3.9",
                .mib = {
                        .type = ASN_INTEGER,
                        .value = {
                                .integer = 9,
                        },
                },
        },
        {
                /* BCT ONLY (BC will add on its next release)
                 *
                 * Blade 10 Number of reboots - bootCountPowerOnTimeBoots
                 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.10.1.1.3.10",
                .mib = {
                        .type = ASN_INTEGER,
                        .value = {
                                .integer = 10,
                        },
                },
        },
        {
                /* BCT ONLY (BC will add on its next release)
                 *
                 * Blade 11 Number of reboots - bootCountPowerOnTimeBoots
                 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.10.1.1.3.11",
                .mib = {
                        .type = ASN_INTEGER,
                        .value = {
                                .integer = 11,
                        },
                },
        },
        {
                /* BCT ONLY (BC will add on its next release)
                 *
                 * Blade 12 Number of reboots - bootCountPowerOnTimeBoots
                 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.10.1.1.3.12",
                .mib = {
                        .type = ASN_INTEGER,
                        .value = {
                                .integer = 12,
                        },
                },
        },
        {
                /* BCT ONLY (BC will add on its next release)
                 *
                 * Blade 13 Number of reboots - bootCountPowerOnTimeBoots
                 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.10.1.1.3.13",
                .mib = {
                        .type = ASN_INTEGER,
                        .value = {
                                .integer = 13,
                        },
                },
        },
        {
                /* BCT ONLY (BC will add on its next release)
                 *
                 * Blade 14 Number of reboots - bootCountPowerOnTimeBoots
                 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.10.1.1.3.14",
                .mib = {
                        .type = ASN_INTEGER,
                        .value = {
                                .integer = 14,
                        },
                },
        },
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
                /* Chassis Plus 1.8 Volt - Low Major */
                .oid = ".1.3.6.1.4.1.2.3.51.2.2.20.2.1.1.10.6",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+ 1.62 Volts",
                        },
                },
        },
        {
                /* Chassis Plus 1.8 Volt - Up Major */
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
                /* Chassis Plus 2.5 Volt - Low Major */
                .oid = ".1.3.6.1.4.1.2.3.51.2.2.20.2.1.1.10.5",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+ 2.25 Volts",
                        },
                },
        },
        {
                /* Chassis Plus 2.5 Volt - Up Major */
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
                /* Chassis Plus 3.3 Volt - Low Major */
                .oid = ".1.3.6.1.4.1.2.3.51.2.2.20.2.1.1.10.2",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+ 3.00 Volts",
                        },
                },
        },
        {
                /* Chassis Plus 3.3 Volt - Up Major */
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
                /* Chassis Plus 5 Volt - Low Major */
                .oid = ".1.3.6.1.4.1.2.3.51.2.2.20.2.1.1.10.1",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+ 4.50 Volts",
                        },
                },
        },
        {
                /* Chassis Plus 5 Volt - Up Major */
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
                /* Chassis Negative 5 Volt - Low Major */
                .oid = ".1.3.6.1.4.1.2.3.51.2.2.20.2.1.1.10.4",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "- 5.50 Volts",
                        },
                },
        },
        {
                /* Chassis Negative 5 Volt - Up Major */
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
                /* Chassis Plus 12 Volt - Low Major */
                .oid = ".1.3.6.1.4.1.2.3.51.2.2.20.2.1.1.10.3",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+10.80 Volts",
                        },
                },
        },
        {
                /* Chassis Plus 12 Volt - Up Major */
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
                /* Chassis UUID */
                .oid = ".1.3.6.1.4.1.2.3.51.2.2.21.1.1.4.0",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "F161 42C1 6593 11D7 8D0E F738 156C AAAC "
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
                /* Blade CPU 1 Up Major temperature - Blade 1 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.7.1",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "  85.00 Centigrade",
                        },
                },
        },
        {
                /* Blade CPU 1 Up Major temperature - Blade 2 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.7.2",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "  85.00 Centigrade",
                        },
                },
        },
        {
                /* Blade CPU 1 Up Major temperature - Blade 3 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.7.3",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "  85.00 Centigrade",
                        },
                },
        },
        {
                /* Blade CPU 1 Up Major temperature - Blade 4 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.7.4",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "  85.00 Centigrade",
                        },
                },
        },
        {
                /* Blade CPU 1 Up Major temperature - Blade 5 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.7.5",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "  85.00 Centigrade",
                        },
                },
        },
        {
                /* Blade CPU 1 Up Major temperature - Blade 6 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.7.6",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "  85.00 Centigrade",
                        },
                },
        },
        {
                /* Blade CPU 1 Up Major temperature - Blade 7 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.7.7",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "  85.00 Centigrade",
                        },
                },
        },
        {
                /* Blade CPU 1 Up Major temperature - Blade 8 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.7.8",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "  85.00 Centigrade",
                        },
                },
        },
        {
                /* Blade CPU 1 Up Major temperature - Blade 9 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.7.9",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "  85.00 Centigrade",
                        },
                },
        },
        {
                /* Blade CPU 1 Up Major temperature - Blade 10 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.7.10",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "  85.00 Centigrade",
                        },
                },
        },
        {
                /* Blade CPU 1 Up Major temperature - Blade 11 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.7.11",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "  85.00 Centigrade",
                        },
                },
        },
        {
                /* Blade CPU 1 Up Major temperature - Blade 12 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.7.12",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "  85.00 Centigrade",
                        },
                },
        },
        {
                /* Blade CPU 1 Up Major temperature - Blade 13 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.7.13",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "  85.00 Centigrade",
                        },
                },
        },
        {
                /* Blade CPU 1 Up Major temperature - Blade 14 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.7.14",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "  85.00 Centigrade",
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
                /* Blade CPU 2 Up Major temperature - Blade 1 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.10.1",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "  85.00 Centigrade",
                        },
                },
        },
        {
                /* Blade CPU 2 Up Major temperature - Blade 2 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.10.2",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "  85.00 Centigrade",
                        },
                },
        },
        {
                /* Blade CPU 2 Up Major temperature - Blade 3 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.10.3",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "  85.00 Centigrade",
                        },
                },
        },
        {
                /* Blade CPU 2 Up Major temperature - Blade 4 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.10.4",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "  85.00 Centigrade",
                        },
                },
        },
        {
                /* Blade CPU 2 Up Major temperature - Blade 5 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.10.5",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "  85.00 Centigrade",
                        },
                },
        },
        {
                /* Blade CPU 2 Up Major temperature - Blade 6 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.10.6",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "  85.00 Centigrade",
                        },
                },
        },
        {
                /* Blade CPU 2 Up Major temperature - Blade 7 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.10.7",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "  85.00 Centigrade",
                        },
                },
        },
        {
                /* Blade CPU 2 Up Major temperature - Blade 8 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.10.8",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "  85.00 Centigrade",
                        },
                },
        },
        {
                /* Blade CPU 2 Up Major temperature - Blade 9 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.10.9",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "  85.00 Centigrade",
                        },
                },
        },
        {
                /* Blade CPU 2 Up Major temperature - Blade 10 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.10.10",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "  85.00 Centigrade",
                        },
                },
        },
        {
                /* Blade CPU 2 Up Major temperature - Blade 11 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.10.11",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "  85.00 Centigrade",
                        },
                },
        },
        {
                /* Blade CPU 2 Up Major temperature - Blade 12 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.10.12",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "  85.00 Centigrade",
                        },
                },
        },
        {
                /* Blade CPU 2 Up Major temperature - Blade 13 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.10.13",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "  85.00 Centigrade",
                        },
                },
        },
        {
                /* Blade CPU 2 Up Major temperature - Blade 14 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.10.14",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "  85.00 Centigrade",
                        },
                },
        },
        {
                /* Blade CPU 3 temperature - Blade 1 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.3.1.8.1",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "  50.00 Centigrade",
                        },
                },
        },
        {
                /* Blade CPU 3 temperature - Blade 2 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.3.1.8.2",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "  50.00 Centigrade",
                        },
                },
        },
        {
                /* Blade CPU 3 temperature - Blade 3 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.3.1.8.3",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "  50.00 Centigrade",
                        },
                },
        },
        {
                /* Blade CPU 3 temperature - Blade 4 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.3.1.8.4",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "  50.00 Centigrade",
                        },
                },
        },
        {
                /* Blade CPU 3 temperature - Blade 5 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.3.1.8.5",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "  50.00 Centigrade",
                        },
                },
        },
        {
                /* Blade CPU 3 temperature - Blade 6 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.3.1.8.6",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "  50.00 Centigrade",
                        },
                },
        },
        {
                /* Blade CPU 3 temperature - Blade 7 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.3.1.8.7",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "  50.00 Centigrade",
                        },
                },
        },
        {
                /* Blade CPU 3 temperature - Blade 8 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.3.1.8.8",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "  50.00 Centigrade",
                        },
                },
        },
        {
                /* Blade CPU 3 temperature - Blade 9 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.3.1.8.9",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "  50.00 Centigrade",
                        },
                },
        },
        {
                /* Blade CPU 3 temperature - Blade 10 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.3.1.8.10",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "  50.00 Centigrade",
                        },
                },
        },
        {
                /* Blade CPU 3 temperature - Blade 11 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.3.1.8.11",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "  50.00 Centigrade",
                        },
                },
        },
        {
                /* Blade CPU 3 temperature - Blade 12 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.3.1.8.12",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "  50.00 Centigrade",
                        },
                },
        },
        {
                /* Blade CPU 3 temperature - Blade 13 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.3.1.8.13",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "  50.00 Centigrade",
                        },
                },
        },
        {
                /* Blade CPU 3 temperature - Blade 14 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.3.1.8.14",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "  50.00 Centigrade",
                        },
                },
        },
        {
                /* Blade CPU 3 Up Critical temperature - Blade 1 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.12.1",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "  95.00 Centigrade",
                        },
                },
        },
        {
                /* Blade CPU 3 Up Critical temperature - Blade 2 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.12.2",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "  95.00 Centigrade",
                        },
                },
        },
        {
                /* Blade CPU 3 Up Critical temperature - Blade 3 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.12.3",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "  95.00 Centigrade",
                        },
                },
        },
        {
                /* Blade CPU 3 Up Critical temperature - Blade 4 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.12.4",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "  95.00 Centigrade",
                        },
                },
        },
        {
                /* Blade CPU 3 Up Critical temperature - Blade 5 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.12.5",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "  95.00 Centigrade",
                        },
                },
        },
        {
                /* Blade CPU 3 Up Critical temperature - Blade 6 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.12.6",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "  95.00 Centigrade",
                        },
                },
        },
        {
                /* Blade CPU 3 Up Critical temperature - Blade 7 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.12.7",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "  95.00 Centigrade",
                        },
                },
        },
        {
                /* Blade CPU 3 Up Critical temperature - Blade 8 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.12.8",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "  95.00 Centigrade",
                        },
                },
        },
        {
                /* Blade CPU 3 Up Critical temperature - Blade 9 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.12.9",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "  95.00 Centigrade",
                        },
                },
        },
        {
                /* Blade CPU 3 Up Critical temperature - Blade 10 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.12.10",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "  95.00 Centigrade",
                        },
                },
        },
        {
                /* Blade CPU 3 Up Critical temperature - Blade 11 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.12.11",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "  95.00 Centigrade",
                        },
                },
        },
        {
                /* Blade CPU 3 Up Critical temperature - Blade 12 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.12.12",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "  95.00 Centigrade",
                        },
                },
        },
        {
                /* Blade CPU 3 Up Critical temperature - Blade 13 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.12.13",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "  95.00 Centigrade",
                        },
                },
        },
        {
                /* Blade CPU 3 Up Critical temperature - Blade 14 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.12.14",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "  95.00 Centigrade",
                        },
                },
        },
        {
                /* Blade CPU 3 Up Major temperature - Blade 1 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.13.1",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "  85.00 Centigrade",
                        },
                },
        },
        {
                /* Blade CPU 3 Up Major temperature - Blade 2 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.13.2",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "  85.00 Centigrade",
                        },
                },
        },
        {
                /* Blade CPU 3 Up Major temperature - Blade 3 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.13.3",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "  85.00 Centigrade",
                        },
                },
        },
        {
                /* Blade CPU 3 Up Major temperature - Blade 4 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.13.4",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "  85.00 Centigrade",
                        },
                },
        },
        {
                /* Blade CPU 3 Up Major temperature - Blade 5 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.13.5",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "  85.00 Centigrade",
                        },
                },
        },
        {
                /* Blade CPU 3 Up Major temperature - Blade 6 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.13.6",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "  85.00 Centigrade",
                        },
                },
        },
        {
                /* Blade CPU 3 Up Major temperature - Blade 7 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.13.7",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "  85.00 Centigrade",
                        },
                },
        },
        {
                /* Blade CPU 3 Up Major temperature - Blade 8 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.13.8",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "  85.00 Centigrade",
                        },
                },
        },
        {
                /* Blade CPU 3 Up Major temperature - Blade 9 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.13.9",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "  85.00 Centigrade",
                        },
                },
        },
        {
                /* Blade CPU 3 Up Major temperature - Blade 10 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.13.10",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "  85.00 Centigrade",
                        },
                },
        },
        {
                /* Blade CPU 3 Up Major temperature - Blade 11 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.13.11",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "  85.00 Centigrade",
                        },
                },
        },
        {
                /* Blade CPU 3 Up Major temperature - Blade 12 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.13.12",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "  85.00 Centigrade",
                        },
                },
        },
        {
                /* Blade CPU 3 Up Major temperature - Blade 13 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.13.13",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "  85.00 Centigrade",
                        },
                },
        },
        {
                /* Blade CPU 3 Up Major temperature - Blade 14 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.13.14",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "  85.00 Centigrade",
                        },
                },
        },
        {
                /* Blade CPU 4 temperature - Blade 1 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.3.1.9.1",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "  50.00 Centigrade",
                        },
                },
        },
        {
                /* Blade CPU 4 temperature - Blade 2 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.3.1.9.2",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "  50.00 Centigrade",
                        },
                },
        },
        {
                /* Blade CPU 4 temperature - Blade 3 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.3.1.9.3",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "  50.00 Centigrade",
                        },
                },
        },
        {
                /* Blade CPU 4 temperature - Blade 4 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.3.1.9.4",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "  50.00 Centigrade",
                        },
                },
        },
        {
                /* Blade CPU 4 temperature - Blade 5 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.3.1.9.5",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "  50.00 Centigrade",
                        },
                },
        },
        {
                /* Blade CPU 4 temperature - Blade 6 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.3.1.9.6",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "  50.00 Centigrade",
                        },
                },
        },
        {
                /* Blade CPU 4 temperature - Blade 7 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.3.1.9.7",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "  50.00 Centigrade",
                        },
                },
        },
        {
                /* Blade CPU 4 temperature - Blade 8 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.3.1.9.8",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "  50.00 Centigrade",
                        },
                },
        },
        {
                /* Blade CPU 4 temperature - Blade 9 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.3.1.9.9",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "  50.00 Centigrade",
                        },
                },
        },
        {
                /* Blade CPU 4 temperature - Blade 10 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.3.1.9.10",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "  50.00 Centigrade",
                        },
                },
        },
        {
                /* Blade CPU 4 temperature - Blade 11 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.3.1.9.11",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "  50.00 Centigrade",
                        },
                },
        },
        {
                /* Blade CPU 4 temperature - Blade 12 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.3.1.9.12",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "  50.00 Centigrade",
                        },
                },
        },
        {
                /* Blade CPU 4 temperature - Blade 13 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.3.1.9.13",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "  50.00 Centigrade",
                        },
                },
        },
        {
                /* Blade CPU 4 temperature - Blade 14 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.3.1.9.14",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "  50.00 Centigrade",
                        },
                },
        },
        {
                /* Blade CPU 4 Up Critical temperature - Blade 1 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.15.1",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "  95.00 Centigrade",
                        },
                },
        },
        {
                /* Blade CPU 4 Up Critical temperature - Blade 2 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.15.2",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "  95.00 Centigrade",
                        },
                },
        },
        {
                /* Blade CPU 4 Up Critical temperature - Blade 3 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.15.3",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "  95.00 Centigrade",
                        },
                },
        },
        {
                /* Blade CPU 4 Up Critical temperature - Blade 4 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.15.4",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "  95.00 Centigrade",
                        },
                },
        },
        {
                /* Blade CPU 4 Up Critical temperature - Blade 5 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.15.5",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "  95.00 Centigrade",
                        },
                },
        },
        {
                /* Blade CPU 4 Up Critical temperature - Blade 6 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.15.6",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "  95.00 Centigrade",
                        },
                },
        },
        {
                /* Blade CPU 4 Up Critical temperature - Blade 7 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.15.7",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "  95.00 Centigrade",
                        },
                },
        },
        {
                /* Blade CPU 4 Up Critical temperature - Blade 8 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.15.8",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "  95.00 Centigrade",
                        },
                },
        },
        {
                /* Blade CPU 4 Up Critical temperature - Blade 9 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.15.9",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "  95.00 Centigrade",
                        },
                },
        },
        {
                /* Blade CPU 4 Up Critical temperature - Blade 10 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.15.10",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "  95.00 Centigrade",
                        },
                },
        },
        {
                /* Blade CPU 4 Up Critical temperature - Blade 11 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.15.11",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "  95.00 Centigrade",
                        },
                },
        },
        {
                /* Blade CPU 4 Up Critical temperature - Blade 12 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.15.12",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "  95.00 Centigrade",
                        },
                },
        },
        {
                /* Blade CPU 4 Up Critical temperature - Blade 13 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.15.13",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "  95.00 Centigrade",
                        },
                },
        },
        {
                /* Blade CPU 4 Up Critical temperature - Blade 14 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.15.14",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "  95.00 Centigrade",
                        },
                },
        },
        {
                /* Blade CPU 4 Up Major temperature - Blade 1 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.16.1",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "  85.00 Centigrade",
                        },
                },
        },
        {
                /* Blade CPU 4 Up Major temperature - Blade 2 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.16.2",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "  85.00 Centigrade",
                        },
                },
        },
        {
                /* Blade CPU 4 Up Major temperature - Blade 3 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.16.3",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "  85.00 Centigrade",
                        },
                },
        },
        {
                /* Blade CPU 4 Up Major temperature - Blade 4 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.16.4",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "  85.00 Centigrade",
                        },
                },
        },
        {
                /* Blade CPU 4 Up Major temperature - Blade 5 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.16.5",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "  85.00 Centigrade",
                        },
                },
        },
        {
                /* Blade CPU 4 Up Major temperature - Blade 6 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.16.6",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "  85.00 Centigrade",
                        },
                },
        },
        {
                /* Blade CPU 4 Up Major temperature - Blade 7 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.16.7",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "  85.00 Centigrade",
                        },
                },
        },
        {
                /* Blade CPU 4 Up Major temperature - Blade 8 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.16.8",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "  85.00 Centigrade",
                        },
                },
        },
        {
                /* Blade CPU 4 Up Major temperature - Blade 9 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.16.9",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "  85.00 Centigrade",
                        },
                },
        },
        {
                /* Blade CPU 4 Up Major temperature - Blade 10 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.16.10",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "  85.00 Centigrade",
                        },
                },
        },
        {
                /* Blade CPU 4 Up Major temperature - Blade 11 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.16.11",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "  85.00 Centigrade",
                        },
                },
        },
        {
                /* Blade CPU 4 Up Major temperature - Blade 12 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.16.12",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "  85.00 Centigrade",
                        },
                },
        },
        {
                /* Blade CPU 4 Up Major temperature - Blade 13 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.16.13",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "  85.00 Centigrade",
                        },
                },
        },
        {
                /* Blade CPU 4 Up Major temperature - Blade 14 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.16.14",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "  85.00 Centigrade",
                        },
                },
        },
	{
                /* Blade DASD1 Temperature - Blade 1 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.3.1.10.1",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = " 199.99 Centigrade",
                        },
                },
        },
        {
                /* Blade DASD1 Temperature - Blade 2 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.3.1.10.2",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "   9.99 Centigrade",
                        },
                },
        },
        {
                /* Blade DASD1 Temperature - Blade 3 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.3.1.10.3",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "-199.99 Centigrade",
                        },
                },
        },
        {
                /* Blade DASD1 Temperature - Blade 4 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.3.1.10.4",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "-  9.99 Centigrade",
                        },
                },
        },
        {
                /* Blade DASD1 Temperature - Blade 5 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.3.1.10.5",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "   0.00 Centigrade",
                        },
                },
        },
        {
                /* Blade DASD1 Temperature - Blade 6 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.3.1.10.6",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "-  0.00 Centigrade",
                        },
                },
        },
        {
                /* Blade DASD1 Temperature - Blade 7 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.3.1.10.7",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "   0.00 Centigrade",
                        },
                },
        },
        {
                /* Blade DASD1 Temperature - Blade 8 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.3.1.10.8",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "   0.00 Centigrade",
                        },
                },
        },
        {
                /* Blade DASD1 Temperature - Blade 9 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.3.1.10.9",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "   0.00 Centigrade",
                        },
                },
        },
        {
                /* Blade DASD1 Temperature - Blade 10 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.3.1.10.10",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "   0.00 Centigrade",
                        },
                },
        },
        {
                /* Blade DASD1 Temperature - Blade 11 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.3.1.10.11",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "   0.00 Centigrade",
                        },
                },
        },
        {
                /* Blade DASD1 Temperature - Blade 12 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.3.1.10.12",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "   0.00 Centigrade",
                        },
                },
        },
        {
                /* Blade DASD1 Temperature - Blade 13 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.3.1.10.13",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "   0.00 Centigrade",
                        },
                },
        },
        {
                /* Blade DASD1 Temperature - Blade 14 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.3.1.10.14",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "   0.00 Centigrade",
                        },
                },
        },
        {
                /* Blade IPMI Temperature Capability  - Blade 1 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.3.1.11.1",
                .mib = {
                        .type = ASN_INTEGER,
                        .value = {
                                .integer = 1,
                        },
                },
        },
        {
                /* Blade IPMI Temperature Capability  - Blade 2 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.3.1.11.2",
                .mib = {
                        .type = ASN_INTEGER,
                        .value = {
                                .integer = 0,
                        },
                },
        },
        {
                /* Blade IPMI Temperature Capability  - Blade 3 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.3.1.11.3",
                .mib = {
                        .type = ASN_INTEGER,
                        .value = {
                                .integer = 1,
                        },
                },
        },
        {
                /* Blade IPMI Temperature Capability  - Blade 4 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.3.1.11.4",
                .mib = {
                        .type = ASN_INTEGER,
                        .value = {
                                .integer = 0,
                        },
                },
        },
        {
                /* Blade IPMI Temperature Capability  - Blade 5 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.3.1.11.5",
                .mib = {
                        .type = ASN_INTEGER,
                        .value = {
                                .integer = 0,
                        },
                },
        },
        {
                /* Blade IPMI Temperature Capability  - Blade 6 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.3.1.11.6",
                .mib = {
                        .type = ASN_INTEGER,
                        .value = {
                                .integer = 0,
                        },
                },
        },
        {
                /* Blade IPMI Temperature Capability  - Blade 7 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.3.1.11.7",
                .mib = {
                        .type = ASN_INTEGER,
                        .value = {
                                .integer = 0,
                        },
                },
        },
        {
                /* Blade IPMI Temperature Capability  - Blade 8 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.3.1.11.8",
                .mib = {
                        .type = ASN_INTEGER,
                        .value = {
                                .integer = 0,
                        },
                },
        },
        {
                /* Blade IPMI Temperature Capability  - Blade 9 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.3.1.11.9",
                .mib = {
                        .type = ASN_INTEGER,
                        .value = {
                                .integer = 0,
                        },
                },
        },
        {
                /* Blade IPMI Temperature Capability  - Blade 10 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.3.1.11.10",
                .mib = {
                        .type = ASN_INTEGER,
                        .value = {
                                .integer = 0,
                        },
                },
        },
        {
                /* Blade IPMI Temperature Capability  - Blade 11 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.3.1.11.11",
                .mib = {
                        .type = ASN_INTEGER,
                        .value = {
                                .integer = 0,
                        },
                },
        },
        {
                /* Blade IPMI Temperature Capability  - Blade 12 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.3.1.11.12",
                .mib = {
                        .type = ASN_INTEGER,
                        .value = {
                                .integer = 0,
                        },
                },
        },
        {
                /* Blade IPMI Temperature Capability  - Blade 13 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.3.1.11.13",
                .mib = {
                        .type = ASN_INTEGER,
                        .value = {
                                .integer = 0,
                        },
                },
        },
        {
                /* Blade IPMI Temperature Capability  - Blade 14 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.3.1.11.14",
                .mib = {
                        .type = ASN_INTEGER,
                        .value = {
                                .integer = 0,
                        },
                },
        },
        {
                /* Blade IPMI Temperature Sensor 1 - Blade 1 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.3.1.12.1",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "CPU 1 TeMP = +37.00 Centigrade",
                        },
                },
        },
        {
                /* Blade IPMI Temperature Sensor 1 - Blade 3 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.3.1.12.3",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "Not Readable!",
                        },
                },
        },
        {
                /* Blade IPMI Temperature Sensor 2 - Blade 1 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.3.1.13.1",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "Not Readable!",
                        },
                },
        },
        {
                /* Blade IPMI Temperature Sensor 2 - Blade 3 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.3.1.13.3",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "Not Readable!",
                        },
                },
        },
        {
                /* Blade IPMI Temperature Sensor 3 - Blade 1 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.3.1.14.1",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = " CPU2 temp = +90.00 Centigrade ",
                        },
                },
        },
        {
                /* Blade IPMI Temperature Sensor 3 - Blade 3 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.3.1.14.3",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "(No temperature)",
                        },
                },
        },
        {
                /* Blade IPMI Temperature Sensor 4 - Blade 1 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.3.1.15.1",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "GARBAGE",
                        },
                },
        },
        {
                /* Blade IPMI Temperature Sensor 4 - Blade 3 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.3.1.15.3",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "GARBAGE",
                        },
                },
        },
        {
                /* Blade IPMI Temperature Sensor 5 - Blade 1 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.3.1.16.1",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "(No temperature)",
                        },
                },
        },
        {
                /* Blade IPMI Temperature Sensor 5 - Blade 3 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.3.1.16.3",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "(No temperature)",
                        },
                },
        },
        {
                /* Blade IPMI Temperature Sensor 6 - Blade 1 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.3.1.17.1",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "(No temperature)",
                        },
                },
        },
        {
                /* Blade IPMI Temperature Sensor 6 - Blade 3 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.3.1.17.3",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "(No temperature)",
                        },
                },
        },
	{
                /* Blade DASD1 Up Critical temperature - Blade 1 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.18.1",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "  95.00 Centigrade",
                        },
                },
        },
        {
                /* Blade DASD1 Up Critical temperature - Blade 2 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.18.2",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "  95.00 Centigrade",
                        },
                },
        },
        {
                /* Blade DASD1 Up Critical temperature - Blade 3 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.18.3",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "  95.00 Centigrade",
                        },
                },
        },
        {
                /* Blade DASD1 Up Critical temperature - Blade 4 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.18.4",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "  95.00 Centigrade",
                        },
                },
        },
        {
                /* Blade DASD1 Up Critical temperature - Blade 5 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.18.5",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "  95.00 Centigrade",
                        },
                },
        },
        {
                /* Blade DASD1 Up Critical temperature - Blade 6 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.18.6",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "  95.00 Centigrade",
                        },
                },
        },
        {
                /* Blade DASD1 Up Critical temperature - Blade 7 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.18.7",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "  95.00 Centigrade",
                        },
                },
        },
        {
                /* Blade DASD1 Up Critical temperature - Blade 8 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.18.8",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "  95.00 Centigrade",
                        },
                },
        },
        {
                /* Blade DASD1 Up Critical temperature - Blade 9 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.18.9",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "  95.00 Centigrade",
                        },
                },
        },
        {
                /* Blade DASD1 Up Critical temperature - Blade 10 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.18.10",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "  95.00 Centigrade",
                        },
                },
        },
        {
                /* Blade DASD1 Up Critical temperature - Blade 11 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.18.11",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "  95.00 Centigrade",
                        },
                },
        },
        {
                /* Blade DASD1 Up Critical temperature - Blade 12 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.18.12",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "  95.00 Centigrade",
                        },
                },
        },
        {
                /* Blade DASD1 Up Critical temperature - Blade 13 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.18.13",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "  95.00 Centigrade",
                        },
                },
        },
        {
                /* Blade DASD1 Up Critical temperature - Blade 14 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.18.14",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "  95.00 Centigrade",
                        },
                },
        },
        {
                /* Blade DASD1 Up Major temperature - Blade 1 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.19.1",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "  85.00 Centigrade",
                        },
                },
        },
        {
                /* Blade DASD1 Up Major temperature - Blade 2 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.19.2",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "  85.00 Centigrade",
                        },
                },
        },
        {
                /* Blade DASD1 Up Major temperature - Blade 3 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.19.3",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "  85.00 Centigrade",
                        },
                },
        },
        {
                /* Blade DASD1 Up Major temperature - Blade 4 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.19.4",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "  85.00 Centigrade",
                        },
                },
        },
        {
                /* Blade DASD1 Up Major temperature - Blade 5 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.19.5",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "  85.00 Centigrade",
                        },
                },
        },
        {
                /* Blade DASD1 Up Major temperature - Blade 6 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.19.6",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "  85.00 Centigrade",
                        },
                },
        },
        {
                /* Blade DASD1 Up Major temperature - Blade 7 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.19.7",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "  85.00 Centigrade",
                        },
                },
        },
        {
                /* Blade DASD1 Up Major temperature - Blade 8 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.19.8",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "  85.00 Centigrade",
                        },
                },
        },
        {
                /* Blade DASD1 Up Major temperature - Blade 9 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.19.9",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "  85.00 Centigrade",
                        },
                },
        },
        {
                /* Blade DASD1 Up Major temperature - Blade 10 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.19.10",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "  85.00 Centigrade",
                        },
                },
        },
        {
                /* Blade DASD1 Up Major temperature - Blade 11 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.19.11",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "  85.00 Centigrade",
                        },
                },
        },
        {
                /* Blade DASD1 Up Major temperature - Blade 12 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.19.12",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "  85.00 Centigrade",
                        },
                },
        },
        {
                /* Blade DASD1 Up Major temperature - Blade 13 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.19.13",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "  85.00 Centigrade",
                        },
                },
        },
        {
                /* Blade DASD1 Up Major temperature - Blade 14 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.19.14",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "  85.00 Centigrade",
                        },
                },
        },
        {
                /* Blade IPMI Temperature Sensor 1 Up Critical - Blade 1 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.22.1",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "  +95.00 Centigrade",
                        },
                },
        },
        {
                /* Blade IPMI Temperature Sensor 1 Up Major - Blade 1 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.23.1",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "  +85.00 Centigrade",
                        },
                },
        },
        {
                /* Blade IPMI Temperature Sensor 3 Up Critical - Blade 1 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.28.1",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "  +95.00 Centigrade",
                        },
                },
        },
        {
                /* Blade IPMI Temperature Sensor 3 Up Major - Blade 1 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.29.1",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "  +85.00 Centigrade",
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
                /* Blade 5V Low Major - Blade 1 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.7.1",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+ 4.40 Volts",
                        },
                },
        },
        {
                /* Blade 5V Low Major - Blade 2 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.7.2",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+ 4.40 Volts",
                        },
                },
        },
        {
                /* Blade 5V Low Major - Blade 3 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.7.3",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+ 4.40 Volts",
                        },
                },
        },
        {
                /* Blade 5V Low Major - Blade 4 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.7.4",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+ 4.40 Volts",
                        },
                },
        },
        {
                /* Blade 5V Low Major - Blade 5 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.7.5",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+ 4.40 Volts",
                        },
                },
        },
        {
                /* Blade 5V Low Major - Blade 6 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.7.6",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+ 4.40 Volts",
                        },
                },
        },
        {
                /* Blade 5V Low Major - Blade 7 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.7.7",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+ 4.40 Volts",
                        },
                },
        },
        {
                /* Blade 5V Low Major - Blade 8 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.7.8",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+ 4.40 Volts",
                        },
                },
        },
        {
                /* Blade 5V Low Major - Blade 9 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.7.9",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+ 4.40 Volts",
                        },
                },
        },
        {
                /* Blade 5V Low Major - Blade 10 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.7.10",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+ 4.40 Volts",
                        },
                },
        },
        {
                /* Blade 5V Low Major - Blade 11 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.7.11",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+ 4.40 Volts",
                        },
                },
        },
        {
                /* Blade 5V Low Major - Blade 12 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.7.12",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+ 4.40 Volts",
                        },
                },
        },
        {
                /* Blade 5V Low Major - Blade 13 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.7.13",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+ 4.40 Volts",
                        },
                },
        },
        {
                /* Blade 5V Low Major - Blade 14 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.7.14",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+ 4.40 Volts",
                        },
                },
        },
        {
                /* Blade 5V Up Major - Blade 1 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.6.1",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+ 5.50 Volts",
                        },
                },
        },
        {
                /* Blade 5V Up Major - Blade 2 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.6.2",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+ 5.50 Volts",
                        },
                },
        },
        {
                /* Blade 5V Up Major - Blade 3 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.6.3",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+ 5.50 Volts",
                        },
                },
        },
        {
                /* Blade 5V Up Major - Blade 4 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.6.4",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+ 5.50 Volts",
                        },
                },
        },
        {
                /* Blade 5V Up Major - Blade 5 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.6.5",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+ 5.50 Volts",
                        },
                },
        },
        {
                /* Blade 5V Up Major - Blade 6 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.6.6",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+ 5.50 Volts",
                        },
                },
        },
        {
                /* Blade 5V Up Major - Blade 7 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.6.7",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+ 5.50 Volts",
                        },
                },
        },
        {
                /* Blade 5V Up Major - Blade 8 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.6.8",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+ 5.50 Volts",
                        },
                },
        },
        {
                /* Blade 5V Up Major - Blade 9 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.6.9",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+ 5.50 Volts",
                        },
                },
        },
        {
                /* Blade 5V Up Major - Blade 10 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.6.10",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+ 5.50 Volts",
                        },
                },
        },
        {
                /* Blade 5V Up Major - Blade 11 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.6.11",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+ 5.50 Volts",
                        },
                },
        },
        {
                /* Blade 5V Up Major - Blade 12 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.6.12",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+ 5.50 Volts",
                        },
                },
        },
        {
                /* Blade 5V Up Major - Blade 13 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.6.13",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+ 5.50 Volts",
                        },
                },
        },
        {
                /* Blade 5V Up Major - Blade 14 */
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
                /* Blade 3.3 Volts Low Major - Blade 1 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.9.1",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+ 2.97 Volts",
                        },
                },
        },
        {
                /* Blade 3.3 Volts Low Major - Blade 2 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.9.2",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+ 2.97 Volts",
                        },
                },
        },
        {
                /* Blade 3.3 Volts Low Major - Blade 3 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.9.3",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+ 2.97 Volts",
                        },
                },
        },
        {
                /* Blade 3.3 Volts Low Major - Blade 4 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.9.4",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+ 2.97 Volts",
                        },
                },
        },
        {
                /* Blade 3.3 Volts Low Major - Blade 5 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.9.5",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+ 2.97 Volts",
                        },
                },
        },
        {
                /* Blade 3.3 Volts Low Major - Blade 6 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.9.6",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+ 2.97 Volts",
                        },
                },
        },
        {
                /* Blade 3.3 Volts Low Major - Blade 7 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.9.7",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+ 2.97 Volts",
                        },
                },
        },
        {
                /* Blade 3.3 Volts Low Major - Blade 8 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.9.8",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+ 2.97 Volts",
                        },
                },
        },
        {
                /* Blade 3.3 Volts Low Major - Blade 9 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.9.9",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+ 2.97 Volts",
                        },
                },
        },
        {
                /* Blade 3.3 Volts Low Major - Blade 10 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.9.10",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+ 2.97 Volts",
                        },
                },
        },
        {
                /* Blade 3.3 Volts Low Major - Blade 11 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.9.11",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+ 2.97 Volts",
                        },
                },
        },
        {
                /* Blade 3.3 Volts Low Major - Blade 12 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.9.12",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+ 2.97 Volts",
                        },
                },
        },
        {
                /* Blade 3.3 Volts Low Major - Blade 13 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.9.13",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+ 2.97 Volts",
                        },
                },
        },
        {
                /* Blade 3.3 Volts Low Major - Blade 14 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.9.14",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+ 2.97 Volts",
                        },
                },
        },
        {
                /* Blade 3.3 Volts Up Major - Blade 1 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.8.1",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+ 3.63 Volts",
                        },
                },
        },
        {
                /* Blade 3.3 Volts Up Major - Blade 2 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.8.2",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+ 3.63 Volts",
                        },
                },
        },
        {
                /* Blade 3.3 Volts Up Major - Blade 3 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.8.3",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+ 3.63 Volts",
                        },
                },
        },
        {
                /* Blade 3.3 Volts Up Major - Blade 4 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.8.4",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+ 3.63 Volts",
                        },
                },
        },
        {
                /* Blade 3.3 Volts Up Major - Blade 5 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.8.5",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+ 3.63 Volts",
                        },
                },
        },
        {
                /* Blade 3.3 Volts Up Major - Blade 6 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.8.6",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+ 3.63 Volts",
                        },
                },
        },
        {
                /* Blade 3.3 Volts Up Major - Blade 7 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.8.7",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+ 3.63 Volts",
                        },
                },
        },
        {
                /* Blade 3.3 Volts Up Major - Blade 8 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.8.8",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+ 3.63 Volts",
                        },
                },
        },
        {
                /* Blade 3.3 Volts Up Major - Blade 9 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.8.9",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+ 3.63 Volts",
                        },
                },
        },
        {
                /* Blade 3.3 Volts Up Major - Blade 10 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.8.10",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+ 3.63 Volts",
                        },
                },
        },
        {
                /* Blade 3.3 Volts Up Major - Blade 11 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.8.11",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+ 3.63 Volts",
                        },
                },
        },
        {
                /* Blade 3.3 Volts Up Major - Blade 12 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.8.12",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+ 3.63 Volts",
                        },
                },
        },
        {
                /* Blade 3.3 Volts Up Major - Blade 13 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.8.13",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+ 3.63 Volts",
                        },
                },
        },
        {
                /* Blade 3.3 Volts Up Major - Blade 14 */
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
                /* Blade 12 Volts Low Major - Blade 1 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.11.1",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+10.80 Volts",
                        },
                },
        },
        {
                /* Blade 12 Volts Low Major - Blade 2 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.11.2",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+10.80 Volts",
                        },
                },
        },
        {
                /* Blade 12 Volts Low Major - Blade 3 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.11.3",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+10.80 Volts",
                        },
                },
        },
        {
                /* Blade 12 Volts Low Major - Blade 4 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.11.4",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+10.80 Volts",
                        },
                },
        },
        {
                /* Blade 12 Volts Low Major - Blade 5 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.11.5",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+10.80 Volts",
                        },
                },
        },
        {
                /* Blade 12 Volts Low Major - Blade 6 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.11.6",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+10.80 Volts",
                        },
                },
        },
        {
                /* Blade 12 Volts Low Major - Blade 7 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.11.7",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+10.80 Volts",
                        },
                },
        },
        {
                /* Blade 12 Volts Low Major - Blade 8 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.11.8",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+10.80 Volts",
                        },
                },
        },
        {
                /* Blade 12 Volts Low Major - Blade 9 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.11.9",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+10.80 Volts",
                        },
                },
        },
        {
                /* Blade 12 Volts Low Major - Blade 10 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.11.10",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+10.80 Volts",
                        },
                },
        },
        {
                /* Blade 12 Volts Low Major - Blade 11 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.11.11",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+10.80 Volts",
                        },
                },
        },
        {
                /* Blade 12 Volts Low Major - Blade 12 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.11.12",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+10.80 Volts",
                        },
                },
        },
        {
                /* Blade 12 Volts Low Major - Blade 13 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.11.13",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+10.80 Volts",
                        },
                },
        },
        {
                /* Blade 12 Volts Low Major - Blade 14 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.11.14",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+10.80 Volts",
                        },
                },
        },
        {
                /* Blade 12 Volts Up Major - Blade 1 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.10.1",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+13.20 Volts",
                        },
                },
        },
        {
                /* Blade 12 Volts Up Major - Blade 2 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.10.2",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+13.20 Volts",
                        },
                },
        },
        {
                /* Blade 12 Volts Up Major - Blade 3 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.10.3",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+13.20 Volts",
                        },
                },
        },
        {
                /* Blade 12 Volts Up Major - Blade 4 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.10.4",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+13.20 Volts",
                        },
                },
        },
        {
                /* Blade 12 Volts Up Major - Blade 5 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.10.5",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+13.20 Volts",
                        },
                },
        },
        {
                /* Blade 12 Volts Up Major - Blade 6 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.10.6",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+13.20 Volts",
                        },
                },
        },
        {
                /* Blade 12 Volts Up Major - Blade 7 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.10.7",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+13.20 Volts",
                        },
                },
        },
        {
                /* Blade 12 Volts Up Major - Blade 8 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.10.8",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+13.20 Volts",
                        },
                },
        },
        {
                /* Blade 12 Volts Up Major - Blade 9 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.10.9",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+13.20 Volts",
                        },
                },
        },
        {
                /* Blade 12 Volts Up Major - Blade 10 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.10.10",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+13.20 Volts",
                        },
                },
        },
        {
                /* Blade 12 Volts Up Major - Blade 11 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.10.11",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+13.20 Volts",
                        },
                },
        },
        {
                /* Blade 12 Volts Up Major - Blade 12 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.10.12",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+13.20 Volts",
                        },
                },
        },
        {
                /* Blade 12 Volts Up Major - Blade 13 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.10.13",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+13.20 Volts",
                        },
                },
        },
        {
                /* Blade 12 Volts Up Major - Blade 14 */
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
                /* Blade 2.5 Volts Low Major - Blade 1 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.15.1",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+ 2.25 Volts",
                        },
                },
        },
        {
                /* Blade 2.5 Volts Low Major - Blade 2 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.15.2",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+ 2.25 Volts",
                        },
                },
        },
        {
                /* Blade 2.5 Volts Low Major - Blade 3 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.15.3",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+ 2.25 Volts",
                        },
                },
        },
        {
                /* Blade 2.5 Volts Low Major - Blade 4 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.15.4",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+ 2.25 Volts",
                        },
                },
        },
        {
                /* Blade 2.5 Volts Low Major - Blade 5 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.15.5",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+ 2.25 Volts",
                        },
                },
        },
        {
                /* Blade 2.5 Volts Low Major - Blade 6 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.15.6",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+ 2.25 Volts",
                        },
                },
        },
        {
                /* Blade 2.5 Volts Low Major - Blade 7 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.15.7",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+ 2.25 Volts",
                        },
                },
        },
        {
                /* Blade 2.5 Volts Low Major - Blade 8 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.15.8",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+ 2.25 Volts",
                        },
                },
        },
        {
                /* Blade 2.5 Volts Low Major - Blade 9 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.15.9",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+ 2.25 Volts",
                        },
                },
        },
        {
                /* Blade 2.5 Volts Low Major - Blade 10 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.15.10",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+ 2.25 Volts",
                        },
                },
        },
        {
                /* Blade 2.5 Volts Low Major - Blade 11 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.15.11",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+ 2.25 Volts",
                        },
                },
        },
        {
                /* Blade 2.5 Volts Low Major - Blade 12 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.15.12",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+ 2.25 Volts",
                        },
                },
        },
        {
                /* Blade 2.5 Volts Low Major - Blade 13 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.15.13",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+ 2.25 Volts",
                        },
                },
        },
        {
                /* Blade 2.5 Volts Low Major - Blade 14 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.15.14",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+ 2.25 Volts",
                        },
                },
        },
        {
                /* Blade 2.5 Volts Up Major - Blade 1 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.14.1",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+ 2.95 Volts",
                        },
                },
        },
        {
                /* Blade 2.5 Volts Up Major - Blade 2 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.14.2",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+ 2.95 Volts",
                        },
                },
        },
        {
                /* Blade 2.5 Volts Up Major - Blade 3 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.14.3",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+ 2.95 Volts",
                        },
                },
        },
        {
                /* Blade 2.5 Volts Up Major - Blade 4 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.14.4",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+ 2.95 Volts",
                        },
                },
        },
        {
                /* Blade 2.5 Volts Up Major - Blade 5 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.14.5",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+ 2.95 Volts",
                        },
                },
        },
        {
                /* Blade 2.5 Volts Up Major - Blade 6 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.14.6",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+ 2.95 Volts",
                        },
                },
        },
        {
                /* Blade 2.5 Volts Up Major - Blade 7 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.14.7",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+ 2.95 Volts",
                        },
                },
        },
        {
                /* Blade 2.5 Volts Up Major - Blade 8 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.14.8",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+ 2.95 Volts",
                        },
                },
        },
        {
                /* Blade 2.5 Volts Up Major - Blade 9 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.14.9",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+ 2.95 Volts",
                        },
                },
        },
        {
                /* Blade 2.5 Volts Up Major - Blade 10 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.14.10",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+ 2.95 Volts",
                        },
                },
        },
        {
                /* Blade 2.5 Volts Up Major - Blade 11 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.14.11",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+ 2.95 Volts",
                        },
                },
        },
        {
                /* Blade 2.5 Volts Up Major - Blade 12 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.14.12",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+ 2.95 Volts",
                        },
                },
        },
        {
                /* Blade 2.5 Volts Up Major - Blade 13 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.14.13",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+ 2.95 Volts",
                        },
                },
        },
        {
                /* Blade 2.5 Volts Up Major - Blade 14 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.14.14",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+ 2.95 Volts",
                        },
                },
        },
        {
                /* Blade IPMI Voltage Sensor 1 Up Major - Blade 1 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.23.1",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+ 2.95 Volts",
                        },
                },
        },
        {
                /* Blade IPMI Voltage Sensor 1 Low Major - Blade 1 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.24.1",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+ 1.6 Volts",
                        },
                },
        },
        {
                /* Blade IPMI Voltage Sensor 25 Up Major - Blade 3 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.71.3",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+ 3.95 Volts",
                        },
                },
        },
        {
                /* Blade IPMI Voltage Sensor 25 Low Major - Blade 3 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.72.3",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+ 2.5 Volts",
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
                /* Blade 1.5 Volts Low Major - Blade 1 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.17.1",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+ 1.32 Volts",
                        },
                },
        },
        {
                /* Blade 1.5 Volts Low Major - Blade 2 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.17.2",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+ 1.32 Volts",
                        },
                },
        },
        {
                /* Blade 1.5 Volts Low Major - Blade 3 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.17.3",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+ 1.32 Volts",
                        },
                },
        },
        {
                /* Blade 1.5 Volts Low Major - Blade 4 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.17.4",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+ 1.32 Volts",
                        },
                },
        },
        {
                /* Blade 1.5 Volts Low Major - Blade 5 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.17.5",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+ 1.32 Volts",
                        },
                },
        },
        {
                /* Blade 1.5 Volts Low Major - Blade 6 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.17.6",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+ 1.32 Volts",
                        },
                },
        },
        {
                /* Blade 1.5 Volts Low Major - Blade 7 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.17.7",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+ 1.32 Volts",
                        },
                },
        },
        {
                /* Blade 1.5 Volts Low Major - Blade 8 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.17.8",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+ 1.32 Volts",
                        },
                },
        },
        {
                /* Blade 1.5 Volts Low Major - Blade 9 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.17.9",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+ 1.32 Volts",
                        },
                },
        },
        {
                /* Blade 1.5 Volts Low Major - Blade 10 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.17.10",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+ 1.32 Volts",
                        },
                },
        },
        {
                /* Blade 1.5 Volts Low Major - Blade 11 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.17.11",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+ 1.32 Volts",
                        },
                },
        },
        {
                /* Blade 1.5 Volts Low Major - Blade 12 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.17.12",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+ 1.32 Volts",
                        },
                },
        },
        {
                /* Blade 1.5 Volts Low Major - Blade 13 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.17.13",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+ 1.32 Volts",
                        },
                },
        },
        {
                /* Blade 1.5 Volts Low Major - Blade 14 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.17.14",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+ 1.32 Volts",
                        },
                },
        },
        {
                /* Blade 1.5 Volts Up Major - Blade 1 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.16.1",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+ 1.68 Volts",
                        },
                },
        },
        {
                /* Blade 1.5 Volts Up Major - Blade 2 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.16.2",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+ 1.68 Volts",
                        },
                },
        },
        {
                /* Blade 1.5 Volts Up Major - Blade 3 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.16.3",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+ 1.68 Volts",
                        },
                },
        },
        {
                /* Blade 1.5 Volts Up Major - Blade 4 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.16.4",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+ 1.68 Volts",
                        },
                },
        },
        {
                /* Blade 1.5 Volts Up Major - Blade 5 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.16.5",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+ 1.68 Volts",
                        },
                },
        },
        {
                /* Blade 1.5 Volts Up Major - Blade 6 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.16.6",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+ 1.68 Volts",
                        },
                },
        },
        {
                /* Blade 1.5 Volts Up Major - Blade 7 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.16.7",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+ 1.68 Volts",
                        },
                },
        },
        {
                /* Blade 1.5 Volts Up Major - Blade 8 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.16.8",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+ 1.68 Volts",
                        },
                },
        },
        {
                /* Blade 1.5 Volts Up Major - Blade 9 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.16.9",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+ 1.68 Volts",
                        },
                },
        },
        {
                /* Blade 1.5 Volts Up Major - Blade 10 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.16.10",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+ 1.68 Volts",
                        },
                },
        },
        {
                /* Blade 1.5 Volts Up Major - Blade 11 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.16.11",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+ 1.68 Volts",
                        },
                },
        },
        {
                /* Blade 1.5 Volts Up Major - Blade 12 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.16.12",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+ 1.68 Volts",
                        },
                },
        },
        {
                /* Blade 1.5 Volts Up Major - Blade 13 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.16.13",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+ 1.68 Volts",
                        },
                },
        },
        {
                /* Blade 1.5 Volts Up Major - Blade 14 */
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
                /* Blade 1.25 Volts Low Major - Blade 1 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.19.1",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+ 1.10 Volts",
                        },
                },
        },
        {
                /* Blade 1.25 Volts Low Major - Blade 2 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.19.2",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+ 1.10 Volts",
                        },
                },
        },
        {
                /* Blade 1.25 Volts Low Major - Blade 3 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.19.3",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+ 1.10 Volts",
                        },
                },
        },
        {
                /* Blade 1.25 Volts Low Major - Blade 4 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.19.4",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+ 1.10 Volts",
                        },
                },
        },
        {
                /* Blade 1.25 Volts Low Major - Blade 5 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.19.5",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+ 1.10 Volts",
                        },
                },
        },
        {
                /* Blade 1.25 Volts Low Major - Blade 6 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.19.6",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+ 1.10 Volts",
                        },
                },
        },
        {
                /* Blade 1.25 Volts Low Major - Blade 7 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.19.7",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+ 1.10 Volts",
                        },
                },
        },
        {
                /* Blade 1.25 Volts Low Major - Blade 8 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.19.8",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+ 1.10 Volts",
                        },
                },
        },
        {
                /* Blade 1.25 Volts Low Major - Blade 9 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.19.9",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+ 1.10 Volts",
                        },
                },
        },
        {
                /* Blade 1.25 Volts Low Major - Blade 10 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.19.10",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+ 1.10 Volts",
                        },
                },
        },
        {
                /* Blade 1.25 Volts Low Major - Blade 11 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.19.11",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+ 1.10 Volts",
                        },
                },
        },
        {
                /* Blade 1.25 Volts Low Major - Blade 12 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.19.12",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+ 1.10 Volts",
                        },
                },
        },
        {
                /* Blade 1.25 Volts Low Major - Blade 13 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.19.13",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+ 1.10 Volts",
                        },
                },
        },
        {
                /* Blade 1.25 Volts Low Major - Blade 14 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.19.14",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+ 1.10 Volts",
                        },
                },
        },
        {
                /* Blade 1.25 Volts Up Major - Blade 1 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.18.1",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+ 1.40 Volts",
                        },
                },
        },
        {
                /* Blade 1.25 Volts Up Major - Blade 2 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.18.2",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+ 1.40 Volts",
                        },
                },
        },
        {
                /* Blade 1.25 Volts Up Major - Blade 3 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.18.3",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+ 1.40 Volts",
                        },
                },
        },
        {
                /* Blade 1.25 Volts Up Major - Blade 4 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.18.4",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+ 1.40 Volts",
                        },
                },
        },
        {
                /* Blade 1.25 Volts Up Major - Blade 5 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.18.5",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+ 1.40 Volts",
                        },
                },
        },
        {
                /* Blade 1.25 Volts Up Major - Blade 6 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.18.6",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+ 1.40 Volts",
                        },
                },
        },
        {
                /* Blade 1.25 Volts Up Major - Blade 7 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.18.7",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+ 1.40 Volts",
                        },
                },
        },
        {
                /* Blade 1.25 Volts Up Major - Blade 8 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.18.8",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+ 1.40 Volts",
                        },
                },
        },
        {
                /* Blade 1.25 Volts Up Major - Blade 9 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.18.9",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+ 1.40 Volts",
                        },
                },
        },
        {
                /* Blade 1.25 Volts Up Major - Blade 10 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.18.10",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+ 1.40 Volts",
                        },
                },
        },
        {
                /* Blade 1.25 Volts Up Major - Blade 11 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.18.11",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+ 1.40 Volts",
                        },
                },
        },
        {
                /* Blade 1.25 Volts Up Major - Blade 12 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.18.12",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+ 1.40 Volts",
                        },
                },
        },
        {
                /* Blade 1.25 Volts Up Major - Blade 13 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.18.13",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "+ 1.40 Volts",
                        },
                },
        },
        {
                /* Blade 1.25 Volts Up Major - Blade 14 */
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
                /* Blade IPMI Voltage Capability - Blade 1 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.14.1",
                .mib = {
                        .type = ASN_INTEGER,
                        .value = {
                                .integer = 0,
                        },
                },
        },
        {
                /* Blade IPMI Voltage Capability - Blade 2 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.14.2",
                .mib = {
                        .type = ASN_INTEGER,
                        .value = {
                                .integer = 0,
                        },
                },
        },
        {
                /* Blade IPMI Voltage Capability - Blade 3 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.14.3",
                .mib = {
                        .type = ASN_INTEGER,
                        .value = {
                                .integer = 1,
                        },
                },
        },
        {
                /* Blade IPMI Voltage Capability - Blade 4 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.14.4",
                .mib = {
                        .type = ASN_INTEGER,
                        .value = {
                                .integer = 0,
                        },
                },
        },
        {
                /* Blade IPMI Voltage Capability - Blade 5 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.14.5",
                .mib = {
                        .type = ASN_INTEGER,
                        .value = {
                                .integer = 0,
                        },
                },
        },
        {
                /* Blade IPMI Voltage Capability - Blade 6 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.14.6",
                .mib = {
                        .type = ASN_INTEGER,
                        .value = {
                                .integer = 0,
                        },
                },
        },
        {
                /* Blade IPMI Voltage Capability - Blade 7 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.14.7",
                .mib = {
                        .type = ASN_INTEGER,
                        .value = {
                                .integer = 0,
                        },
                },
        },
        {
                /* Blade IPMI Voltage Capability - Blade 8 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.14.8",
                .mib = {
                        .type = ASN_INTEGER,
                        .value = {
                                .integer = 0,
                        },
                },
        },
        {
                /* Blade IPMI Voltage Capability - Blade 9 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.14.9",
                .mib = {
                        .type = ASN_INTEGER,
                        .value = {
                                .integer = 0,
                        },
                },
        },
        {
                /* Blade IPMI Voltage Capability - Blade 10 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.14.10",
                .mib = {
                        .type = ASN_INTEGER,
                        .value = {
                                .integer = 0,
                        },
                },
        },
        {
                /* Blade IPMI Voltage Capability - Blade 11 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.14.11",
                .mib = {
                        .type = ASN_INTEGER,
                        .value = {
                                .integer = 0,
                        },
                },
        },
        {
                /* Blade IPMI Voltage Capability - Blade 12 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.14.12",
                .mib = {
                        .type = ASN_INTEGER,
                        .value = {
                                .integer = 0,
                        },
                },
        },
        {
                /* Blade IPMI Voltage Capability - Blade 13 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.14.13",
                .mib = {
                        .type = ASN_INTEGER,
                        .value = {
                                .integer = 0,
                        },
                },
        },
        {
                /* Blade IPMI Voltage Capability - Blade 14 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.14.14",
                .mib = {
                        .type = ASN_INTEGER,
                        .value = {
                                .integer = 0,
                        },
                },
        },
        {
                /* Blade IPMI Voltage Sensor 1 - Blade 1 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.15.1",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "1.8VSB Sense = +3.2 Volts",
                        },
                },
        },
        {
                /* Blade IPMI Voltage Sensor 2 - Blade 1 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.16.1",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "(No voltage)",
                        },
                },
        },
        {
                /* Blade IPMI Voltage Sensor 3 - Blade 1 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.17.1",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "(No voltage)",
                        },
                },
        },
        {
                /* Blade IPMI Voltage Sensor 4 - Blade 1 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.18.1",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "(No voltage)",
                        },
                },
        },
        {
                /* Blade IPMI Voltage Sensor 5 - Blade 1 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.19.1",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "(No voltage)",
                        },
                },
        },
        {
                /* Blade IPMI Voltage Sensor 6 - Blade 1 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.20.1",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "(No voltage)",
                        },
                },
        },
        {
                /* Blade IPMI Voltage Sensor 7 - Blade 1 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.21.1",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "(No voltage)",
                        },
                },
        },
        {
                /* Blade IPMI Voltage Sensor 8 - Blade 1 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.22.1",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "(No voltage)",
                        },
                },
        },
        {
                /* Blade IPMI Voltage Sensor 9 - Blade 1 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.23.1",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "(No voltage)",
                        },
                },
        },
        {
                /* Blade IPMI Voltage Sensor 10 - Blade 1 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.24.1",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "(No voltage)",
                        },
                },
        },
        {
                /* Blade IPMI Voltage Sensor 11 - Blade 1 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.25.1",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "(No voltage)",
                        },
                },
        },
        {
                /* Blade IPMI Voltage Sensor 12 - Blade 1 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.26.1",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "(No voltage)",
                        },
                },
        },
        {
                /* Blade IPMI Voltage Sensor 13 - Blade 1 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.27.1",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "(No voltage)",
                        },
                },
        },
        {
                /* Blade IPMI Voltage Sensor 14 - Blade 1 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.28.1",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "(No voltage)",
                        },
                },
        },
        {
                /* Blade IPMI Voltage Sensor 15 - Blade 1 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.29.1",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "(No voltage)",
                        },
                },
        },
        {
                /* Blade IPMI Voltage Sensor 16 - Blade 1 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.30.1",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "(No voltage)",
                        },
                },
        },
        {
                /* Blade IPMI Voltage Sensor 17 - Blade 1 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.31.1",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "(No voltage)",
                        },
                },
        },
        {
                /* Blade IPMI Voltage Sensor 18 - Blade 1 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.32.1",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "(No voltage)",
                        },
                },
        },
        {
                /* Blade IPMI Voltage Sensor 19 - Blade 1 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.33.1",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "(No voltage)",
                        },
                },
        },
        {
                /* Blade IPMI Voltage Sensor 20 - Blade 1 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.34.1",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "(No voltage)",
                        },
                },
        },
        {
                /* Blade IPMI Voltage Sensor 21 - Blade 1 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.35.1",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "(No voltage)",
                        },
                },
        },
        {
                /* Blade IPMI Voltage Sensor 22 - Blade 1 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.36.1",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "(No voltage)",
                        },
                },
        },
        {
                /* Blade IPMI Voltage Sensor 23 - Blade 1 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.37.1",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "(No voltage)",
                        },
                },
        },
        {
                /* Blade IPMI Voltage Sensor 24 - Blade 1 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.38.1",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "(No voltage)",
                        },
                },
        },
        {
                /* Blade IPMI Voltage Sensor 25 - Blade 1 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.39.1",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "(No voltage)",
                        },
                },
        },
	{
                /* Blade IPMI Voltage Sensor 1 - Blade 3 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.15.3",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "2.5V Sense = +2.50 Volts",
                        },
                },
        },
        {
                /* Blade IPMI Voltage Sensor 2 - Blade 3 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.16.3",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "(No voltage)",
                        },
                },
        },
        {
                /* Blade IPMI Voltage Sensor 3 - Blade 3 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.17.3",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "(No voltage)",
                        },
                },
        },
        {
                /* Blade IPMI Voltage Sensor 4 - Blade 3 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.18.3",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "(No voltage)",
                        },
                },
        },
        {
                /* Blade IPMI Voltage Sensor 5 - Blade 3 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.19.3",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "(No voltage)",
                        },
                },
        },
        {
                /* Blade IPMI Voltage Sensor 6 - Blade 3 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.20.3",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "(No voltage)",
                        },
                },
        },
        {
                /* Blade IPMI Voltage Sensor 7 - Blade 3 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.21.3",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "(No voltage)",
                        },
                },
        },
        {
                /* Blade IPMI Voltage Sensor 8 - Blade 3 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.22.3",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "(No voltage)",
                        },
                },
        },
        {
                /* Blade IPMI Voltage Sensor 9 - Blade 3 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.23.3",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "(No voltage)",
                        },
                },
        },
        {
                /* Blade IPMI Voltage Sensor 10 - Blade 3 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.24.3",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "(No voltage)",
                        },
                },
        },
        {
                /* Blade IPMI Voltage Sensor 11 - Blade 3 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.25.3",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "(No voltage)",
                        },
                },
        },
        {
                /* Blade IPMI Voltage Sensor 12 - Blade 3 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.26.3",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "(No voltage)",
                        },
                },
        },
        {
                /* Blade IPMI Voltage Sensor 13 - Blade 3 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.27.3",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "(No voltage)",
                        },
                },
        },
        {
                /* Blade IPMI Voltage Sensor 14 - Blade 3 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.28.3",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "(No voltage)",
                        },
                },
        },
        {
                /* Blade IPMI Voltage Sensor 15 - Blade 3 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.29.3",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "(No voltage)",
                        },
                },
        },
        {
                /* Blade IPMI Voltage Sensor 16 - Blade 3 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.30.3",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "(No voltage)",
                        },
                },
        },
        {
                /* Blade IPMI Voltage Sensor 17 - Blade 3 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.31.3",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "(No voltage)",
                        },
                },
        },
        {
                /* Blade IPMI Voltage Sensor 18 - Blade 3 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.32.3",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "(No voltage)",
                        },
                },
        },
        {
                /* Blade IPMI Voltage Sensor 19 - Blade 3 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.33.3",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "(No voltage)",
                        },
                },
        },
        {
                /* Blade IPMI Voltage Sensor 20 - Blade 3 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.34.3",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "(No voltage)",
                        },
                },
        },
        {
                /* Blade IPMI Voltage Sensor 21 - Blade 3 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.35.3",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "(No voltage)",
                        },
                },
        },
        {
                /* Blade IPMI Voltage Sensor 22 - Blade 3 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.36.3",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "(No voltage)",
                        },
                },
        },
        {
                /* Blade IPMI Voltage Sensor 23 - Blade 3 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.37.3",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "(No voltage)",
                        },
                },
        },
        {
                /* Blade IPMI Voltage Sensor 24 - Blade 3 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.38.3",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "(No voltage)",
                        },
                },
        },
	{
                /* Blade IPMI Voltage Sensor 25 - Blade 3 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.39.3",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "3.3VSB Sense = + 3.33 Volts",
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
                /* Chassis Identity LED - BC */
                .oid = ".1.3.6.1.4.1.2.3.51.2.2.8.1.4.0",
                .mib = {
                        .type = ASN_INTEGER,
                        .value = {
                                .integer = 0, /* off=0; on=1; blinking=2; NA=3 */
                        },
                },
        },
        {
                /* Chassis Identity LED - BCT */
                .oid = ".1.3.6.1.4.1.2.3.51.2.2.8.3.4.0",
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
                                .string = "8677", /* 8677 (BCE); 8720 (BCT DC); 8730 (BCT AC) */
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
	/* Used to determine if expansion card attached to blade */
        {
                /* BladeServerExpansion - bladeExpBoardVpdBladeBayNumber */
                .oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.3.1.19.1",
                .mib = {
                        .type = ASN_INTEGER,
                        .value = {
                                .integer = SA_ERR_SNMP_NOSUCHOBJECT,
                                /* .integer = 255, BCT */
                        },
                },
        },	
        {
                /* BladeServerExpansion - bladeExpBoardVpdCardType */
                .oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.3.1.20.1",
                .mib = {
                        .type = ASN_INTEGER,
                        .value = {
                                .integer = SA_ERR_SNMP_NOSUCHOBJECT,
                                /* .integer = 255, BCT */
                        },
                },
        },	
        {
                /* BladeServerExpansion - Blade Add-on 2 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.1.1.14.2",
                .mib = {
                        .type = ASN_INTEGER,
                        .value = {
                                .integer = 0,
                        },
                },
        },
        {
                /* BladeServerExpansion - Blade Add-on 3 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.1.1.14.3",
                .mib = {
                        .type = ASN_INTEGER,
                        .value = {
                                .integer = 1,
                        },
                },
        },
        {
                /* BladeServerExpansion - Blade Add-on 4 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.1.1.14.4",
                .mib = {
                        .type = ASN_INTEGER,
                        .value = {
                                .integer = 0,
                        },
                },
        },
        {
                /* BladeServerExpansion - Blade Add-on 5 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.1.1.14.5",
                .mib = {
                        .type = ASN_INTEGER,
                        .value = {
                                .integer = 1,
                        },
                },
        },
        {
                /* BladeServerExpansion - Blade Add-on 6 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.1.1.14.6",
                .mib = {
                        .type = ASN_INTEGER,
                        .value = {
                                .integer = 0,
                        },
                },
        },
        {
                /* BladeServerExpansion - Blade Add-on 7 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.1.1.14.7",
                .mib = {
                        .type = ASN_INTEGER,
                        .value = {
                                .integer = 1,
                        },
                },
        },
        {
                /* BladeServerExpansion - Blade Add-on 8 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.1.1.14.8",
                .mib = {
                        .type = ASN_INTEGER,
                        .value = {
                                .integer = 0,
                        },
                },
        },
        {
                /* BladeServerExpansion - Blade Add-on 9 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.1.1.14.9",
                .mib = {
                        .type = ASN_INTEGER,
                        .value = {
                                .integer = 1,
                        },
                },
        },
        {
                /* BladeServerExpansion - Blade Add-on 10 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.1.1.14.10",
                .mib = {
                        .type = ASN_INTEGER,
                        .value = {
                                .integer = 0,
                        },
                },
        },
        {
                /* BladeServerExpansion - Blade Add-on 11 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.1.1.14.11",
                .mib = {
                        .type = ASN_INTEGER,
                        .value = {
                                .integer = 1,
                        },
                },
        },
        {
                /* BladeServerExpansion - Blade Add-on 12 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.1.1.14.12",
                .mib = {
                        .type = ASN_INTEGER,
                        .value = {
                                .integer = 0,
                        },
                },
        },
        {
                /* BladeServerExpansion - Blade Add-on 13 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.1.1.14.13",
                .mib = {
                        .type = ASN_INTEGER,
                        .value = {
                                .integer = 1,
                        },
                },
        },
        {
                /* BladeServerExpansion - Blade Add-on 14 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.1.1.14.14",
                .mib = {
                        .type = ASN_INTEGER,
                        .value = {
                                .integer = 0,
                        },
                },
        },
        {
                /* Blade Add-on Product Name VPD - Blade Add-on 1 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.25.1",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "867821Z",
                        },
                },
        },
        {
                /* Blade Add-on Product Name VPD - Blade Add-on 2 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.25.2",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "867821Z",
                        },
                },
        },
        {
                /* Blade Add-on Product Name VPD - Blade Add-on 3 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.25.3",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "867821Z",
                        },
                },
        },
        {
                /* Blade Add-on Product Name VPD - Blade Add-on 4 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.25.4",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "867821Z",
                        },
                },
        },
        {
                /* Blade Add-on Product Name VPD - Blade Add-on 5 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.25.5",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "867821Z",
                        },
                },
        },
        {
                /* Blade Add-on Product Name VPD - Blade Add-on 6 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.25.6",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "867821Z",
                        },
                },
        },
        {
                /* Blade Add-on Product Name VPD - Blade Add-on 7 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.25.7",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "867821Z",
                        },
                },
        },
        {
                /* Blade Add-on Product Name VPD - Blade Add-on 8 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.25.8",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "867821Z",
                        },
                },
        },
        {
                /* Blade Add-on Product Name VPD - Blade Add-on 9 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.25.9",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "867821Z",
                        },
                },
        },
        {
                /* Blade Add-on Product Name VPD - Blade Add-on 10 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.25.10",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "867821Z",
                        },
                },
        },
        {
                /* Blade Add-on Product Name VPD - Blade Add-on 11 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.25.11",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "867821Z",
                        },
                },
        },
        {
                /* Blade Add-on Product Name VPD - Blade Add-on 12 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.25.12",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "867821Z",
                        },
                },
        },
        {
                /* Blade Add-on Product Name VPD - Blade Add-on 13 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.25.13",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "867821Z",
                        },
                },
        },
        {
                /* Blade Add-on Product Name VPD - Blade Add-on 14 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.25.14",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "867821Z",
                        },
                },
        },
        {
                /* Blade Add-on Product Version VPD - Blade Add-on 1 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.23.1",
                .mib = {
                        .type = ASN_INTEGER,
                        .value = {
                                .integer = 0,
                        },
                },
        },
        {
                /* Blade Add-on Product Version VPD - Blade Add-on 2 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.23.2",
                .mib = {
                        .type = ASN_INTEGER,
                        .value = {
                                .integer = 0,
                        },
                },
        },
        {
                /* Blade Add-on Product Version VPD - Blade Add-on 3 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.23.3",
                .mib = {
                        .type = ASN_INTEGER,
                        .value = {
                                .integer = 0,
                        },
                },
        },
        {
                /* Blade Add-on Product Version VPD - Blade Add-on 4 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.23.4",
                .mib = {
                        .type = ASN_INTEGER,
                        .value = {
                                .integer = 0,
                        },
                },
        },
        {
                /* Blade Add-on Product Version VPD - Blade Add-on 5 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.23.5",
                .mib = {
                        .type = ASN_INTEGER,
                        .value = {
                                .integer = 0,
                        },
                },
        },
        {
                /* Blade Add-on Product Version VPD - Blade Add-on 6 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.23.6",
                .mib = {
                        .type = ASN_INTEGER,
                        .value = {
                                .integer = 0,
                        },
                },
        },
        {
                /* Blade Add-on Product Version VPD - Blade Add-on 7 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.23.7",
                .mib = {
                        .type = ASN_INTEGER,
                        .value = {
                                .integer = 0,
                        },
                },
        },
        {
                /* Blade Add-on Product Version VPD - Blade Add-on 8 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.23.8",
                .mib = {
                        .type = ASN_INTEGER,
                        .value = {
                                .integer = 0,
                        },
                },
        },
        {
                /* Blade Add-on Product Version VPD - Blade Add-on 9 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.23.9",
                .mib = {
                        .type = ASN_INTEGER,
                        .value = {
                                .integer = 0,
                        },
                },
        },
        {
                /* Blade Add-on Product Version VPD - Blade Add-on 10 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.23.10",
                .mib = {
                        .type = ASN_INTEGER,
                        .value = {
                                .integer = 0,
                        },
                },
        },
        {
                /* Blade Add-on Product Version VPD - Blade Add-on 11 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.23.11",
                .mib = {
                        .type = ASN_INTEGER,
                        .value = {
                                .integer = 0,
                        },
                },
        },
        {
                /* Blade Add-on Product Version VPD - Blade Add-on 12 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.23.12",
                .mib = {
                        .type = ASN_INTEGER,
                        .value = {
                                .integer = 0,
                        },
                },
        },
        {
                /* Blade Add-on Product Version VPD - Blade Add-on 13 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.23.13",
                .mib = {
                        .type = ASN_INTEGER,
                        .value = {
                                .integer = 0,
                        },
                },
        },
        {
                /* Blade Add-on Product Version VPD - Blade Add-on 14 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.23.14",
                .mib = {
                        .type = ASN_INTEGER,
                        .value = {
                                .integer = 0,
                        },
                },
        },
        {
                /* Blade Add-on Part Number VPD - Blade Add-on 1 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.22.1",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "596610",
                        },
                },
        },
        {
                /* Blade Add-on Part Number VPD - Blade Add-on 2 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.22.2",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "596610",
                        },
                },
        },
        {
                /* Blade Add-on Part Number VPD - Blade Add-on 3 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.22.3",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "596610",
                        },
                },
        },
        {
                /* Blade Add-on Part Number VPD - Blade Add-on 4 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.22.4",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "596610",
                        },
                },
        },
        {
                /* Blade Add-on Part Number VPD - Blade Add-on 5 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.22.5",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "596610",
                        },
                },
        },
        {
                /* Blade Add-on Part Number VPD - Blade Add-on 6 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.22.6",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "596610",
                        },
                },
        },
        {
                /* Blade Add-on Part Number VPD - Blade Add-on 7 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.22.7",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "596610",
                        },
                },
        },
        {
                /* Blade Add-on Part Number VPD - Blade Add-on 8 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.22.8",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "596610",
                        },
                },
        },
        {
                /* Blade Add-on Part Number VPD - Blade Add-on 9 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.22.9",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "596610",
                        },
                },
        },
        {
                /* Blade Add-on Part Number VPD - Blade Add-on 10 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.22.10",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "596610",
                        },
                },
        },
        {
                /* Blade Add-on Part Number VPD - Blade Add-on 11 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.22.11",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "596610",
                        },
                },
        },
        {
                /* Blade Add-on Part Number VPD - Blade Add-on 12 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.22.12",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "596610",
                        },
                },
        },
        {
                /* Blade Add-on Part Number VPD - Blade Add-on 13 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.22.13",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "596610",
                        },
                },
        },
        {
                /* Blade Add-on Part Number VPD - Blade Add-on 14 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.22.14",
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
        {
                /* Blade Name - Blade 1 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.2.8.2.1.1.6.1",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "Blade 1",
                        },
                },
        },
        {
                /* Blade Name - Blade 2 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.2.8.2.1.1.6.2",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "Blade 2",
                        },
                },
        },
        {
                /* Blade Name - Blade 3 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.2.8.2.1.1.6.3",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "Blade 3",
                        },
                },
        },
        {
                /* Blade Name - Blade 4 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.2.8.2.1.1.6.4",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "Blade 4",
                        },
                },
        },
        {
                /* Blade Name - Blade 5 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.2.8.2.1.1.6.5",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "Blade 5",
                        },
                },
        },
        {
                /* Blade Name - Blade 6 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.2.8.2.1.1.6.6",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "Blade 6",
                        },
                },
        },
        {
                /* Blade Name - Blade 7 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.2.8.2.1.1.6.7",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "Blade 7",
                        },
                },
        },
        {
                /* Blade Name - Blade 8 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.2.8.2.1.1.6.8",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "Blade 8",
                        },
                },
        },
        {
                /* Blade Name - Blade 9 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.2.8.2.1.1.6.9",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "Blade 9",
                        },
                },
        },
        {
                /* Blade Name - Blade 10 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.2.8.2.1.1.6.10",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "Blade 10",
                        },
                },
        },
        {
                /* Blade Name - Blade 11 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.2.8.2.1.1.6.11",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "Blade 11",
                        },
                },
        },
        {
                /* Blade Name - Blade 12 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.2.8.2.1.1.6.12",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "Blade 12",
                        },
                },
        },
        {
                /* Blade Name - Blade 13 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.2.8.2.1.1.6.13",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "Blade 13",
                        },
                },
        },
        {
                /* Blade Name - Blade 14 */
                .oid = ".1.3.6.1.4.1.2.3.51.2.2.8.2.1.1.6.14",
                .mib = {
                        .type = ASN_OCTET_STR,
                        .value = {
                                .string = "Blade 14",
                        },
                },
        },

        {} /* Terminate array with a null element */
};
