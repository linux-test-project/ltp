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
 *      Renier Morales <renierm@users.sf.net>
 *      Sean Dague <http://dague.net/sean>
 *      Steve Sherman <stevees@us.ibm.com>
 */

/**************************************************************************
 * This source file defines the resource arrays declared in bc_resources.h
 *************************************************************************/

#include <SaHpi.h>
#include <bc_resources.h>

/*************************************************************************
 * RESTRICTIONS!!!
 *
 * - If IsThreshold=SAHPI_TRUE for an interpreted sensor, 
 *   Range.Max.Interpreted.Type must be defined for snmp_bc.c 
 *   get_interpreted_thresholds to work.
 * - Digital controls must be integers and depend on SaHpiStateDigitalT
 *************************************************************************/

/*************************************************************************
 *                   Resource Definitions
 *************************************************************************/
struct snmp_rpt snmp_rpt_array[] = {
	/* Chassis */
        {
                .rpt = {
			.ResourceInfo = {
                                .ManufacturerId = 2,
                        },
                        .ResourceEntity = {
                                .Entry = {}
                        },
                        .ResourceCapabilities = SAHPI_CAPABILITY_CONTROL |
			                        SAHPI_CAPABILITY_INVENTORY_DATA |
                                                SAHPI_CAPABILITY_RDR |
			                        SAHPI_CAPABILITY_RESOURCE |
			                        SAHPI_CAPABILITY_SEL |
			                        SAHPI_CAPABILITY_SENSOR,
                        .ResourceSeverity = SAHPI_CRITICAL,
                },
		.mib = {
			.OidHealth = ".1.3.6.1.4.1.2.3.51.2.2.7.1.0",
			.HealthyValue = 255,
			.OidReset = '\0',
			.OidPowerState = '\0',
			.OidPowerOnOff = '\0',
		},
                .comment = "Top-level blade chassis"
        },
        /* Sub-chassis */
        {
                .rpt = {
                        .ResourceInfo = {
                                .ManufacturerId = 2,
                        },
                        .ResourceEntity = {
                                .Entry[0] =
                                {
                                        .EntityType = SAHPI_ENT_SUB_CHASSIS,
                                        .EntityInstance = BC_HPI_INSTANCE_BASE
                                }
			},
                        .ResourceCapabilities = SAHPI_CAPABILITY_RDR |
			                        SAHPI_CAPABILITY_RESOURCE,
                        .ResourceSeverity = SAHPI_CRITICAL,
                },
		.mib = {
			.OidHealth = '\0',
			.HealthyValue = 0,
			.OidReset = '\0',
			.OidPowerState = '\0',
			.OidPowerOnOff = '\0',
		},
                .comment = "Power distribution sub-system"
        },
        /* Management module */
        {
                .rpt = {
                        .ResourceInfo = {
                                .ManufacturerId = 2,
                        },
                        .ResourceEntity = {
                                .Entry[0] = 
				{
                                        .EntityType = SAHPI_ENT_SYS_MGMNT_MODULE,
                                        .EntityInstance = BC_HPI_INSTANCE_BASE
                                },
				{
                                        .EntityType = SAHPI_ENT_SUB_CHASSIS,
                                        .EntityInstance = BC_HPI_INSTANCE_BASE
                                }
			},	
                        .ResourceCapabilities = SAHPI_CAPABILITY_FRU |
			                        SAHPI_CAPABILITY_INVENTORY_DATA |
			                        SAHPI_CAPABILITY_RDR |
			                        SAHPI_CAPABILITY_RESOURCE,
                        .ResourceSeverity = SAHPI_MAJOR,
		},
		.mib = {
			.OidHealth = '\0',
			.HealthyValue = 0,
			.OidReset = ".1.3.6.1.4.1.2.3.51.2.7.4.0",
			.OidPowerState = '\0',
			.OidPowerOnOff = '\0',
		},
                .comment = "Management module"
        },
        /* Switch module */
        {
                .rpt = {
                        .ResourceInfo = {
                                .ManufacturerId = 2,
                        },
                        .ResourceEntity = {
                                .Entry[0] =
                                {
				        .EntityType = SAHPI_ENT_INTERCONNECT,
			                .EntityInstance = BC_HPI_INSTANCE_BASE
				},
				{
				        .EntityType = SAHPI_ENT_SUB_CHASSIS,
			                .EntityInstance = BC_HPI_INSTANCE_BASE
				}
			},
                        .ResourceCapabilities = SAHPI_CAPABILITY_FRU |
			                        SAHPI_CAPABILITY_INVENTORY_DATA |
			                        SAHPI_CAPABILITY_RDR |
			                        SAHPI_CAPABILITY_RESOURCE,
                        .ResourceSeverity = SAHPI_MAJOR,
                },
		.mib = {
			.OidHealth = '\0',
			.HealthyValue = 0,
			.OidReset = ".1.3.6.1.4.1.2.3.51.2.22.3.1.1.1.8.x",
			.OidPowerState = ".1.3.6.1.4.1.2.3.51.2.22.3.1.1.1.7.x",
			.OidPowerOnOff = ".1.3.6.1.4.1.2.3.51.2.22.3.1.1.1.7.x",
		},
                .comment = "Network switch module"
        },
        /* Blade */
        {
                .rpt = {
                        .ResourceInfo = {
                                .ManufacturerId = 2,
                        },
                        .ResourceEntity = {
                                .Entry[0] = 
                                {
                                        .EntityType = SAHPI_ENT_SBC_BLADE,
                                        .EntityInstance = BC_HPI_INSTANCE_BASE
                                },
                                {
                                        .EntityType = SAHPI_ENT_SUB_CHASSIS,
			                .EntityInstance = BC_HPI_INSTANCE_BASE
                                }
			},
                        .ResourceCapabilities = SAHPI_CAPABILITY_CONTROL |
			                        SAHPI_CAPABILITY_FRU |
			                        SAHPI_CAPABILITY_INVENTORY_DATA |
			                        SAHPI_CAPABILITY_RDR |
			                        SAHPI_CAPABILITY_RESOURCE |
			                        SAHPI_CAPABILITY_SENSOR,
                        .ResourceSeverity = SAHPI_MAJOR,
                },
		.mib = {
			.OidHealth = ".1.3.6.1.4.1.2.3.51.2.22.1.5.1.1.5.x",
			.HealthyValue = 1,
			.OidReset = ".1.3.6.1.4.1.2.3.51.2.22.1.6.1.1.8.x",
			.OidPowerState = ".1.3.6.1.4.1.2.3.51.2.22.1.6.1.1.4.x",
			.OidPowerOnOff = ".1.3.6.1.4.1.2.3.51.2.22.1.6.1.1.7.x",
		},
                .comment = "Blade"
        },
        /* Blade daughter card */
        {
                .rpt = {
                        .ResourceInfo = {
                                .ManufacturerId = 2,
                        },
                        .ResourceEntity = {
                                .Entry[0] = 
                                {
                                        .EntityType = SAHPI_ENT_ADD_IN_CARD,
                                        .EntityInstance = BC_HPI_INSTANCE_BASE
                                },
                                {
                                        .EntityType = SAHPI_ENT_SBC_BLADE,
                                        .EntityInstance = BC_HPI_INSTANCE_BASE
                                },
                                {
                                        .EntityType = SAHPI_ENT_SUB_CHASSIS,
			                .EntityInstance = BC_HPI_INSTANCE_BASE
                                }
			},
                        .ResourceCapabilities = SAHPI_CAPABILITY_INVENTORY_DATA |
			                        SAHPI_CAPABILITY_RDR |
			                        SAHPI_CAPABILITY_RESOURCE,
                        .ResourceSeverity = SAHPI_MAJOR,
                },
		.mib = {
			.OidHealth = '\0',
			.HealthyValue = 0,
			.OidReset = '\0',
			.OidPowerState = '\0',
			.OidPowerOnOff = '\0',
		},
                .comment = "Blade daughter card"
        },
        /* Media Tray */
        {
                .rpt = {
                        .ResourceInfo = {
                                .ManufacturerId = 2,
                        },
                        .ResourceEntity = {
                                .Entry[0] =
                                {
				        .EntityType = SAHPI_ENT_PERIPHERAL_BAY,
			                .EntityInstance = BC_HPI_INSTANCE_BASE
				},
				{
				        .EntityType = SAHPI_ENT_SUB_CHASSIS,
			                .EntityInstance = BC_HPI_INSTANCE_BASE
				}
			},
                        .ResourceCapabilities = SAHPI_CAPABILITY_INVENTORY_DATA |
			                        SAHPI_CAPABILITY_RDR |
			                        SAHPI_CAPABILITY_RESOURCE,
                        .ResourceSeverity = SAHPI_MAJOR,
                },
		.mib = {
			.OidHealth = '\0',
			.HealthyValue = 0,
			.OidReset = '\0',
			.OidPowerState = '\0',
			.OidPowerOnOff = '\0',
		},
                .comment = "Control panel/Media tray"
        },
        /* Blower module */
        {
                .rpt = {
                        .ResourceInfo = {
                                .ManufacturerId = 2,
                        },
                        .ResourceEntity = {
                                .Entry[0] =
                                {
				        .EntityType = SAHPI_ENT_FAN,
			                .EntityInstance = BC_HPI_INSTANCE_BASE
				},
				{
				        .EntityType = SAHPI_ENT_SUB_CHASSIS,
			                .EntityInstance = BC_HPI_INSTANCE_BASE
				}
			},
                        .ResourceCapabilities = SAHPI_CAPABILITY_FRU |
			                        SAHPI_CAPABILITY_RESOURCE |
			                        SAHPI_CAPABILITY_RDR |
			                        SAHPI_CAPABILITY_SENSOR,
                        .ResourceSeverity = SAHPI_MAJOR,
                 },
		.mib = {
			.OidHealth = '\0',
			.HealthyValue = 0,
			.OidReset = '\0',
			.OidPowerState = '\0',
			.OidPowerOnOff = '\0',
		},
                .comment = "Blower module"
        },
        /* Power module */
        {
                .rpt = {
                        .ResourceInfo = {
                                .ManufacturerId = 2,
                        },
                        .ResourceEntity = {
                                .Entry[0] =
                                {
				        .EntityType = SAHPI_ENT_POWER_SUPPLY,
			                .EntityInstance = BC_HPI_INSTANCE_BASE
				},
				{
				        .EntityType = SAHPI_ENT_SUB_CHASSIS,
			                .EntityInstance = BC_HPI_INSTANCE_BASE
				}
			},
                        .ResourceCapabilities = SAHPI_CAPABILITY_FRU |
			                        SAHPI_CAPABILITY_INVENTORY_DATA |
			                        SAHPI_CAPABILITY_RESOURCE |
			                        SAHPI_CAPABILITY_RDR,
                        .ResourceSeverity = SAHPI_MAJOR,
                 },
		.mib = {
			.OidHealth = '\0',
			.HealthyValue = 0,
			.OidReset = '\0',
			.OidPowerState = '\0',
			.OidPowerOnOff = '\0',
		},
                .comment = "Power module"
        },

        {} /* Terminate array with a null element */
};

/******************************************************************************
 *                      Sensor Definitions
 ******************************************************************************/

/*****************
 * Chassis Sensors
 *****************/

struct snmp_bc_sensor snmp_bc_chassis_sensors[] = {

	/* System Error LED on chassis */
        {
                .sensor = {
                        .Num = 1,
                        .Type = SAHPI_PLATFORM_VIOLATION,
                        .Category = SAHPI_EC_SEVERITY,
                        .EventCtrl = SAHPI_SEC_ENTIRE_SENSOR,
                        .Events = SAHPI_ES_OK | SAHPI_ES_CRITICAL,
                        .Ignore = SAHPI_FALSE,
                        .DataFormat = {
                                .ReadingFormats = SAHPI_SRF_EVENT_STATE | SAHPI_SRF_RAW,
                                .IsNumeric = SAHPI_TRUE, 
                                .SignFormat = SAHPI_SDF_UNSIGNED, 
                                .BaseUnits = SAHPI_SU_UNSPECIFIED,
                                .ModifierUnits = SAHPI_SU_UNSPECIFIED,
                                .ModifierUse = SAHPI_SMUU_NONE,
                                .FactorsStatic = SAHPI_TRUE,
                                .Factors = {
                                        .Linearization = SAHPI_SL_LINEAR,
                                },
                                .Percentage = SAHPI_FALSE,
                                .Range = {
                                        .Flags = SAHPI_SRF_MIN | SAHPI_SRF_MAX,
                                        .Max = {
                                                .ValuesPresent = SAHPI_SRF_EVENT_STATE | SAHPI_SRF_RAW,
                                                .Raw = 1,
                                                .EventStatus = {
                                                        .SensorStatus = SAHPI_SENSTAT_EVENTS_ENABLED,
                                                        .EventStatus = SAHPI_ES_CRITICAL
                                                }
                                        },
                                        .Min = {
                                                .ValuesPresent = SAHPI_SRF_EVENT_STATE | SAHPI_SRF_RAW,
                                                .Raw = 0,
                                                .EventStatus = {
                                                        .SensorStatus = SAHPI_SENSTAT_EVENTS_ENABLED,
                                                        .EventStatus = SAHPI_ES_OK
                                                }
                                        }
                                }
                        },
                        .ThresholdDefn = {
                                .IsThreshold = SAHPI_FALSE
                        },
                        .Oem = 0
                },
		.mib = {
			.not_avail_indicator_num = 0,
			.write_only = 0,
			.convert_snmpstr = -1,
			.oid = ".1.3.6.1.4.1.2.3.51.2.2.8.1.1.0",
		},
                .comment = "Front Panel LED - System Error"
        },
        /* Temperature LED on the chassis */
        {
                .sensor = {
                        .Num = 2,
                        .Type = SAHPI_TEMPERATURE,
                        .Category = SAHPI_EC_SEVERITY,
                        .EventCtrl = SAHPI_SEC_ENTIRE_SENSOR,
                        .Events = SAHPI_ES_OK | SAHPI_ES_CRITICAL,
                        .Ignore = SAHPI_FALSE,
                        .DataFormat = {
                                .ReadingFormats = SAHPI_SRF_EVENT_STATE | SAHPI_SRF_RAW,
                                .IsNumeric = SAHPI_TRUE,
                                .SignFormat = SAHPI_SDF_UNSIGNED,
                                .BaseUnits = SAHPI_SU_UNSPECIFIED,
                                .ModifierUnits = SAHPI_SU_UNSPECIFIED,
                                .ModifierUse = SAHPI_SMUU_NONE,
                                .FactorsStatic = SAHPI_TRUE,
                                .Factors = {
                                        .Linearization = SAHPI_SL_LINEAR,
                                },
                                .Percentage = SAHPI_FALSE,
                                .Range = {
                                        .Flags = SAHPI_SRF_MIN | SAHPI_SRF_MAX,
                                        .Max = {
                                                .ValuesPresent = SAHPI_SRF_EVENT_STATE | SAHPI_SRF_RAW,
                                                .Raw = 1,
                                                .EventStatus = {
                                                        .SensorStatus = SAHPI_SENSTAT_EVENTS_ENABLED,
                                                        .EventStatus = SAHPI_ES_CRITICAL
                                                }
                                        },
                                        .Min = {
                                                .ValuesPresent = SAHPI_SRF_EVENT_STATE | SAHPI_SRF_RAW,
                                                .Raw = 0,
                                                .EventStatus = {
                                                        .SensorStatus = SAHPI_SENSTAT_EVENTS_ENABLED,
                                                        .EventStatus = SAHPI_ES_OK
                                                }
                                        }
                                }
                        },
                        .ThresholdDefn = {
                                .IsThreshold = SAHPI_FALSE
                        },
                        .Oem = 0
                },
		.mib = {
			.not_avail_indicator_num = 0,
			.write_only = 0,
			.convert_snmpstr = -1,
			.oid = ".1.3.6.1.4.1.2.3.51.2.2.8.1.3.0",
		},
                .comment = "Front Panel LED - Temperature"
        },
        /* Ambient air thermal sensor on Control Panel/Media Tray */
        {
                .sensor = {
                        .Num = 3,
                        .Type = SAHPI_TEMPERATURE,
                        .Category = SAHPI_EC_THRESHOLD,
                        .EventCtrl = SAHPI_SEC_GLOBAL_DISABLE,
			/* FIXME:: Change when SNMP adds thresholds */
                        .Events = SAHPI_ES_UPPER_MINOR | SAHPI_ES_UPPER_CRIT,
                        .Ignore = SAHPI_FALSE,
                        .DataFormat = {
                                .ReadingFormats = SAHPI_SRF_INTERPRETED,
                                .IsNumeric = SAHPI_TRUE,
                                .SignFormat = SAHPI_SDF_UNSIGNED,
                                .BaseUnits = SAHPI_SU_DEGREES_C,
                                .ModifierUnits = SAHPI_SU_UNSPECIFIED,
                                .ModifierUse = SAHPI_SMUU_NONE,
                                .FactorsStatic = SAHPI_TRUE,
                                .Factors = {
                                        .Linearization = SAHPI_SL_LINEAR,
                                },
                                .Percentage = SAHPI_FALSE,
                                .Range = {
					.Flags = SAHPI_SRF_MAX | SAHPI_SRF_MIN,
					.Max = {
						.ValuesPresent = SAHPI_SRF_INTERPRETED,
						.Interpreted = {
							.Type = SAHPI_SENSOR_INTERPRETED_TYPE_FLOAT32,
							.Value = {
								.SensorFloat32 =125,
							}	
						},
					},
					.Min = {
						.ValuesPresent = SAHPI_SRF_INTERPRETED,
						.Interpreted = {
							.Type = SAHPI_SENSOR_INTERPRETED_TYPE_FLOAT32,
							.Value = {
								.SensorFloat32 = 0,
							}	
						},
					},
				},
                        },
                        .ThresholdDefn = {
				/* FIXME:: SNMP is supposed to add thresholds soon */
                                .IsThreshold = SAHPI_FALSE,
				.TholdCapabilities = SAHPI_STC_INTERPRETED,
				/* FIXME:: SNMP is supposed to add thresholds soon */
				.ReadThold = SAHPI_STM_UP_MINOR | SAHPI_STM_UP_CRIT,
				.FixedThold = SAHPI_STM_UP_MINOR | SAHPI_STM_UP_CRIT,
                        },
                        .Oem = 0
                },
		.mib = {
			.not_avail_indicator_num = 0,
			.write_only = 0,
			.convert_snmpstr = SAHPI_SENSOR_INTERPRETED_TYPE_FLOAT32,
			.oid = ".1.3.6.1.4.1.2.3.51.2.2.1.5.1.0",
			/* FIXME:: SNMP is supposed to add thresholds soon */
			.threshold_oids = {
				.InterpretedThresholds = {
					.OidUpMinor      = "\0",
					.OidUpCrit       = "\0",
				},
			},
		},
                .comment = "Ambient temperature in degrees centigrade(C)."
        },
        /* Thermal sensor on Management Module */
        {
                .sensor = {
                        .Num = 4,
                        .Type = SAHPI_TEMPERATURE,
                        .Category = SAHPI_EC_THRESHOLD,
                        .EventCtrl = SAHPI_SEC_GLOBAL_DISABLE,
                        /* FIXME:: Change when SNMP adds thresholds */
                        .Events = SAHPI_ES_UPPER_MINOR | SAHPI_ES_UPPER_CRIT,
                        .Ignore = SAHPI_FALSE,
                        .DataFormat = {
                                .ReadingFormats = SAHPI_SRF_INTERPRETED,
                                .IsNumeric = SAHPI_TRUE,
                                .SignFormat = SAHPI_SDF_UNSIGNED,
                                .BaseUnits = SAHPI_SU_DEGREES_C,
                                .ModifierUnits = SAHPI_SU_UNSPECIFIED,
                                .ModifierUse = SAHPI_SMUU_NONE,
                                .FactorsStatic = SAHPI_TRUE,
                                .Factors = {
                                        .Linearization = SAHPI_SL_LINEAR,
                                },
                                .Percentage = SAHPI_FALSE,
                                .Range = {
					.Flags = SAHPI_SRF_MAX | SAHPI_SRF_MIN,
					.Max = {
						.ValuesPresent = SAHPI_SRF_INTERPRETED,
						.Interpreted = {
							.Type = SAHPI_SENSOR_INTERPRETED_TYPE_FLOAT32,
							.Value = {
								.SensorFloat32 = 125,
							}	
						},
					},
					.Min = {
						.ValuesPresent = SAHPI_SRF_INTERPRETED,
						.Interpreted = {
							.Type = SAHPI_SENSOR_INTERPRETED_TYPE_FLOAT32,
							.Value = {
								.SensorFloat32 = 0,
							}	
						},
					},
				},
                        },
                        .ThresholdDefn = {
                                 /* FIXME:: SNMP is supposed to add thresholds soon */
                                .IsThreshold = SAHPI_FALSE,
				.TholdCapabilities = SAHPI_STC_INTERPRETED,
                                 /* FIXME:: SNMP is supposed to add thresholds soon */
				.ReadThold = SAHPI_STM_UP_MINOR | SAHPI_STM_UP_CRIT,
				.FixedThold = SAHPI_STM_UP_MINOR | SAHPI_STM_UP_CRIT,
                        },
                        .Oem = 0
                },
		.mib = {
			.not_avail_indicator_num = 0,
			.write_only = 0,
			.convert_snmpstr = SAHPI_SENSOR_INTERPRETED_TYPE_FLOAT32,
			.oid = ".1.3.6.1.4.1.2.3.51.2.2.1.1.2.0",
			/* FIXME:: SNMP is supposed to add thresholds soon */
			.threshold_oids = {
				.InterpretedThresholds = {
					.OidUpMinor      = "\0",
					.OidUpCrit       = "\0",
				},
			},
		},
                .comment = "Management module temperature in degrees centigrade(C)."
        },
        /* 1.8V voltage sensor on Management Module */
        {
                .sensor = {
                        .Num = 5,
                        .Type = SAHPI_VOLTAGE,
                        .Category = SAHPI_EC_THRESHOLD,
                        .EventCtrl = SAHPI_SEC_GLOBAL_DISABLE,
                        .Events = SAHPI_ES_LOWER_MINOR | SAHPI_ES_UPPER_MINOR,
                        .Ignore = SAHPI_FALSE,
                        .DataFormat = {
                                .ReadingFormats = SAHPI_SRF_INTERPRETED,
                                .IsNumeric = SAHPI_TRUE,
                                .SignFormat = SAHPI_SDF_UNSIGNED,
                                .BaseUnits = SAHPI_SU_VOLTS,
                                .ModifierUnits = SAHPI_SU_UNSPECIFIED,
                                .ModifierUse = SAHPI_SMUU_NONE,
                                .FactorsStatic = SAHPI_TRUE,
                                .Factors = {
                                        .Linearization = SAHPI_SL_LINEAR,
                                },
                                .Percentage = SAHPI_FALSE,
                                .Range = {
					.Flags = SAHPI_SRF_MAX | SAHPI_SRF_MIN,
					.Max = {
						.ValuesPresent = SAHPI_SRF_INTERPRETED,
						.Interpreted = {
							.Type = SAHPI_SENSOR_INTERPRETED_TYPE_FLOAT32,
							.Value = {
								.SensorFloat32 = 4.4,
							}	
						},
					},
					.Min = {
						.ValuesPresent = SAHPI_SRF_INTERPRETED,
						.Interpreted = {
							.Type = SAHPI_SENSOR_INTERPRETED_TYPE_FLOAT32,
							.Value = {
								.SensorFloat32 = 0,
							}	
						},
					},
				},
                        },
                        .ThresholdDefn = {
                                .IsThreshold = SAHPI_TRUE,
				.TholdCapabilities = SAHPI_STC_INTERPRETED,
				.ReadThold = SAHPI_STM_LOW_MINOR | SAHPI_STM_UP_MINOR |
 				             SAHPI_STM_LOW_HYSTERESIS | SAHPI_STM_UP_HYSTERESIS,
				.FixedThold = SAHPI_STM_LOW_MINOR | SAHPI_STM_UP_MINOR |
				              SAHPI_STM_LOW_HYSTERESIS | SAHPI_STM_UP_HYSTERESIS,
                         },
                        .Oem = 0
                },
		.mib = {
			.not_avail_indicator_num = 0,
			.write_only = 0,
			.convert_snmpstr = SAHPI_SENSOR_INTERPRETED_TYPE_FLOAT32,
			.oid = ".1.3.6.1.4.1.2.3.51.2.2.2.1.8.0",
			.threshold_oids = {
				.InterpretedThresholds = {
					.OidLowMinor     = ".1.3.6.1.4.1.2.3.51.2.2.20.2.1.1.10.6",
					.OidUpMinor      = ".1.3.6.1.4.1.2.3.51.2.2.20.2.1.1.6.6",
					/*  FIXME:: Hysteresis correct? */
					.OidLowHysteresis = ".1.3.6.1.4.1.2.3.51.2.2.20.2.1.1.11.6",
					.OidUpHysteresis  = ".1.3.6.1.4.1.2.3.51.2.2.20.2.1.1.7.6"
				},
			},
		},
                .comment = "Plus 1.8 Volt power supply voltage reading expressed in volts(V)"
        },
        /* 2.5V voltage sensor on Management Module */
        {
                .sensor = {
                        .Num = 6,
                        .Type = SAHPI_VOLTAGE,
                        .Category = SAHPI_EC_THRESHOLD,
                        .EventCtrl = SAHPI_SEC_GLOBAL_DISABLE,
                        .Events = SAHPI_ES_LOWER_MINOR | SAHPI_ES_UPPER_MINOR,
                        .Ignore = SAHPI_FALSE,
                        .DataFormat = {
                                .ReadingFormats = SAHPI_SRF_INTERPRETED,
                                .IsNumeric = SAHPI_TRUE,
                                .SignFormat = SAHPI_SDF_UNSIGNED,
                                .BaseUnits = SAHPI_SU_VOLTS,
                                .ModifierUnits = SAHPI_SU_UNSPECIFIED,
                                .ModifierUse = SAHPI_SMUU_NONE,
                                .FactorsStatic = SAHPI_TRUE,
                                .Factors = {
                                        .Linearization = SAHPI_SL_LINEAR,
                                },
                                .Percentage = SAHPI_FALSE,
                                .Range = {
					.Flags = SAHPI_SRF_MAX | SAHPI_SRF_MIN,
					.Max = {
						.ValuesPresent = SAHPI_SRF_INTERPRETED,
						.Interpreted = {
							.Type = SAHPI_SENSOR_INTERPRETED_TYPE_FLOAT32,
							.Value = {
								.SensorFloat32 = 4.4,
							}	
						},
					},
					.Min = {
						.ValuesPresent = SAHPI_SRF_INTERPRETED,
						.Interpreted = {
							.Type = SAHPI_SENSOR_INTERPRETED_TYPE_FLOAT32,
							.Value = {
								.SensorFloat32 = 0,
							}	
						},
					},
				},
                        },
                        .ThresholdDefn = {
                                .IsThreshold = SAHPI_TRUE,
				.TholdCapabilities = SAHPI_STC_INTERPRETED,
				.ReadThold = SAHPI_STM_LOW_MINOR | SAHPI_STM_UP_MINOR | 
 				             SAHPI_STM_LOW_HYSTERESIS | SAHPI_STM_UP_HYSTERESIS,
				.FixedThold = SAHPI_STM_LOW_MINOR | SAHPI_STM_UP_MINOR | 
 				              SAHPI_STM_LOW_HYSTERESIS | SAHPI_STM_UP_HYSTERESIS,
                        },
                        .Oem = 0
                },
		.mib = {
			.not_avail_indicator_num = 0,
			.write_only = 0,
                        .convert_snmpstr = SAHPI_SENSOR_INTERPRETED_TYPE_FLOAT32,
			.oid = ".1.3.6.1.4.1.2.3.51.2.2.2.1.6.0",
			.threshold_oids = {
				.InterpretedThresholds = {
					.OidLowMinor     = ".1.3.6.1.4.1.2.3.51.2.2.20.2.1.1.10.5",
					.OidUpMinor      = ".1.3.6.1.4.1.2.3.51.2.2.20.2.1.1.6.5",
					/*  FIXME:: Hysteresis correct? */
					.OidLowHysteresis = ".1.3.6.1.4.1.2.3.51.2.2.20.2.1.1.11.5",
					.OidUpHysteresis  = ".1.3.6.1.4.1.2.3.51.2.2.20.2.1.1.7.5"
				},
			},
		},
                .comment = "Plus 2.5 Volt power supply voltage reading expressed in volts(V)"
        },
        /* 3.3V voltage sensor on Management Module */
        {
                .sensor = {
                        .Num = 7,
                        .Type = SAHPI_VOLTAGE,
                        .Category = SAHPI_EC_THRESHOLD,
                        .EventCtrl = SAHPI_SEC_GLOBAL_DISABLE,
                        .Events = SAHPI_ES_LOWER_MINOR | SAHPI_ES_UPPER_MINOR,
                        .Ignore = SAHPI_FALSE,
                        .DataFormat = {
                                .ReadingFormats = SAHPI_SRF_INTERPRETED,
                                .IsNumeric = SAHPI_TRUE,
                                .SignFormat = SAHPI_SDF_UNSIGNED,
                                .BaseUnits = SAHPI_SU_VOLTS,
                                .ModifierUnits = SAHPI_SU_UNSPECIFIED,
                                .ModifierUse = SAHPI_SMUU_NONE,
                                .FactorsStatic = SAHPI_TRUE,
                                .Factors = {
                                        .Linearization = SAHPI_SL_LINEAR,
                                },
                                .Percentage = SAHPI_FALSE,
                                .Range = {
					.Flags = SAHPI_SRF_MAX | SAHPI_SRF_MIN,
					.Max = {
						.ValuesPresent = SAHPI_SRF_INTERPRETED,
						.Interpreted = {
							.Type = SAHPI_SENSOR_INTERPRETED_TYPE_FLOAT32,
							.Value = {
								.SensorFloat32 = 3.6,
							}	
						},
					},
					.Min = {
						.ValuesPresent = SAHPI_SRF_INTERPRETED,
						.Interpreted = {
							.Type = SAHPI_SENSOR_INTERPRETED_TYPE_FLOAT32,
							.Value = {
								.SensorFloat32 = 0,
							}	
						},
					},
				},
                        },
                        .ThresholdDefn = {
                                .IsThreshold = SAHPI_TRUE,
				.TholdCapabilities = SAHPI_STC_INTERPRETED,
				.ReadThold = SAHPI_STM_LOW_MINOR | SAHPI_STM_UP_MINOR |  
				             SAHPI_STM_LOW_HYSTERESIS | SAHPI_STM_UP_HYSTERESIS,
				.FixedThold = SAHPI_STM_LOW_MINOR | SAHPI_STM_UP_MINOR | 
  				              SAHPI_STM_LOW_HYSTERESIS | SAHPI_STM_UP_HYSTERESIS,
                         },
                        .Oem = 0
                },
		.mib = {
			.not_avail_indicator_num = 0,
			.write_only = 0,
                        .convert_snmpstr = SAHPI_SENSOR_INTERPRETED_TYPE_FLOAT32,
			.oid = ".1.3.6.1.4.1.2.3.51.2.2.2.1.2.0",
			.threshold_oids = {
				.InterpretedThresholds = {
					.OidLowMinor     = ".1.3.6.1.4.1.2.3.51.2.2.20.2.1.1.10.2",
					.OidUpMinor      = ".1.3.6.1.4.1.2.3.51.2.2.20.2.1.1.6.2",
					/*  FIXME:: Hysteresis correct? */
					.OidLowHysteresis = ".1.3.6.1.4.1.2.3.51.2.2.20.2.1.1.11.2",
					.OidUpHysteresis  = ".1.3.6.1.4.1.2.3.51.2.2.20.2.1.1.7.2"
				},
			},
		},
                .comment = "Plus 3.3 Volt power supply voltage reading expressed in volts(V)"
        },
        /* 5V voltage sensor on Management Module */
        {
                .sensor = {
                        .Num = 8,
                        .Type = SAHPI_VOLTAGE,
                        .Category = SAHPI_EC_THRESHOLD,
                        .EventCtrl = SAHPI_SEC_GLOBAL_DISABLE,
                        .Events = SAHPI_ES_LOWER_MINOR | SAHPI_ES_UPPER_MINOR,
                        .Ignore = SAHPI_FALSE,
                        .DataFormat = {
                                .ReadingFormats = SAHPI_SRF_INTERPRETED,
                                .IsNumeric = SAHPI_TRUE,
                                .SignFormat = SAHPI_SDF_UNSIGNED,
                                .BaseUnits = SAHPI_SU_VOLTS,
                                .ModifierUnits = SAHPI_SU_UNSPECIFIED,
                                .ModifierUse = SAHPI_SMUU_NONE,
                                .FactorsStatic = SAHPI_TRUE,
                                .Factors = {
                                        .Linearization = SAHPI_SL_LINEAR,
                                },
                                .Percentage = SAHPI_FALSE,
                                .Range = {
					.Flags = SAHPI_SRF_MAX | SAHPI_SRF_MIN,
					.Max = {
						.ValuesPresent = SAHPI_SRF_INTERPRETED,
						.Interpreted = {
							.Type = SAHPI_SENSOR_INTERPRETED_TYPE_FLOAT32,
							.Value = {
								.SensorFloat32 = 6.7,
							}	
						},
					},
					.Min = {
						.ValuesPresent = SAHPI_SRF_INTERPRETED,
						.Interpreted = {
							.Type = SAHPI_SENSOR_INTERPRETED_TYPE_FLOAT32,
							.Value = {
								.SensorFloat32 = 0,
							}	
						},
					},
				},
                        },
                        .ThresholdDefn = {
                                .IsThreshold = SAHPI_TRUE,
				.TholdCapabilities = SAHPI_STC_INTERPRETED,
				.ReadThold = SAHPI_STM_LOW_MINOR | SAHPI_STM_UP_MINOR |
  				             SAHPI_STM_LOW_HYSTERESIS | SAHPI_STM_UP_HYSTERESIS,
				.FixedThold = SAHPI_STM_LOW_MINOR | SAHPI_STM_UP_MINOR |
				              SAHPI_STM_LOW_HYSTERESIS | SAHPI_STM_UP_HYSTERESIS,
                         },
                        .Oem = 0
                },
		.mib = {
			.not_avail_indicator_num = 0,
			.write_only = 0,
                        .convert_snmpstr = SAHPI_SENSOR_INTERPRETED_TYPE_FLOAT32,
			.oid = ".1.3.6.1.4.1.2.3.51.2.2.2.1.1.0",
			.threshold_oids = {
				.InterpretedThresholds = {
					.OidLowMinor     = ".1.3.6.1.4.1.2.3.51.2.2.20.2.1.1.10.1",
					.OidUpMinor      = ".1.3.6.1.4.1.2.3.51.2.2.20.2.1.1.6.1",
					/*  FIXME:: Hysteresis correct? */
					.OidLowHysteresis = ".1.3.6.1.4.1.2.3.51.2.2.20.2.1.1.11.1",
					.OidUpHysteresis  = ".1.3.6.1.4.1.2.3.51.2.2.20.2.1.1.7.1"
				},
			},
		},
                .comment = "Plus 5 Volt power supply voltage reading expressed in volts(V)"
        },
        /* -5V voltage sensor on Management Module */
        {
                .sensor = {
                        .Num = 9,
                        .Type = SAHPI_VOLTAGE,
                        .Category = SAHPI_EC_THRESHOLD,
                        .EventCtrl = SAHPI_SEC_GLOBAL_DISABLE,
                        .Events = SAHPI_ES_LOWER_MINOR | SAHPI_ES_UPPER_MINOR,
                        .Ignore = SAHPI_FALSE,
                        .DataFormat = {
                                .ReadingFormats = SAHPI_SRF_INTERPRETED,
                                .IsNumeric = SAHPI_TRUE,
                                .SignFormat = SAHPI_SDF_UNSIGNED,
                                .BaseUnits = SAHPI_SU_VOLTS,
                                .ModifierUnits = SAHPI_SU_UNSPECIFIED,
                                .ModifierUse = SAHPI_SMUU_NONE,
                                .FactorsStatic = SAHPI_TRUE,
                                .Factors = {
                                        .Linearization = SAHPI_SL_LINEAR,
                                },
                                .Percentage = SAHPI_FALSE,
                                .Range = {
					.Flags = SAHPI_SRF_MAX | SAHPI_SRF_MIN,
					.Max = {
						.ValuesPresent = SAHPI_SRF_INTERPRETED,
						.Interpreted = {
							.Type = SAHPI_SENSOR_INTERPRETED_TYPE_FLOAT32,
							.Value = {
								.SensorFloat32 = -6.7,
							}	
						},
					},
					.Min = {
						.ValuesPresent = SAHPI_SRF_INTERPRETED,
						.Interpreted = {
							.Type = SAHPI_SENSOR_INTERPRETED_TYPE_FLOAT32,
							.Value = {
								.SensorFloat32 = 0,
							}	
						},
					},
				},
                        },
                        .ThresholdDefn = {
                                .IsThreshold = SAHPI_TRUE,
				.TholdCapabilities = SAHPI_STC_INTERPRETED,
				.ReadThold = SAHPI_STM_LOW_MINOR | SAHPI_STM_UP_MINOR |
   				             SAHPI_STM_LOW_HYSTERESIS | SAHPI_STM_UP_HYSTERESIS,
				.FixedThold = SAHPI_STM_LOW_MINOR | SAHPI_STM_UP_MINOR |
   				              SAHPI_STM_LOW_HYSTERESIS | SAHPI_STM_UP_HYSTERESIS,
                         },
                        .Oem = 0
                },
		.mib = {
			.not_avail_indicator_num = 0,
			.write_only = 0,
                        .convert_snmpstr = SAHPI_SENSOR_INTERPRETED_TYPE_FLOAT32,
			.oid = ".1.3.6.1.4.1.2.3.51.2.2.2.1.5.0",
			.threshold_oids = {
				.InterpretedThresholds = {
					.OidLowMinor     = ".1.3.6.1.4.1.2.3.51.2.2.20.2.1.1.10.4",
					.OidUpMinor      = ".1.3.6.1.4.1.2.3.51.2.2.20.2.1.1.6.4",
					/*  FIXME:: Hysteresis correct? */
					.OidLowHysteresis = ".1.3.6.1.4.1.2.3.51.2.2.20.2.1.1.11.4",
					.OidUpHysteresis  = ".1.3.6.1.4.1.2.3.51.2.2.20.2.1.1.7.4"
				},
			},
		},
                .comment = "Negative 5 Volt power supply voltage reading expressed in volts(V)"
        },
        /* 12V voltage sensor on Management Module */
        {
                .sensor = {
                        .Num = 10,
                        .Type = SAHPI_VOLTAGE,
                        .Category = SAHPI_EC_THRESHOLD,
                        .EventCtrl = SAHPI_SEC_GLOBAL_DISABLE,
                        .Events = SAHPI_ES_LOWER_MINOR | SAHPI_ES_UPPER_MINOR,
                        .Ignore = SAHPI_FALSE,
                        .DataFormat = {
                                .ReadingFormats = SAHPI_SRF_INTERPRETED,
                                .IsNumeric = SAHPI_TRUE,
                                .SignFormat = SAHPI_SDF_UNSIGNED,
                                .BaseUnits = SAHPI_SU_VOLTS,
                                .ModifierUnits = SAHPI_SU_UNSPECIFIED,
                                .ModifierUse = SAHPI_SMUU_NONE,
                                .FactorsStatic = SAHPI_TRUE,
                                .Factors = {
                                        .Linearization = SAHPI_SL_LINEAR,
                                },
                                .Percentage = SAHPI_FALSE,
                                .Range = {
					.Flags = SAHPI_SRF_MAX | SAHPI_SRF_MIN,
					.Max = {
						.ValuesPresent = SAHPI_SRF_INTERPRETED,
						.Interpreted = {
							.Type = SAHPI_SENSOR_INTERPRETED_TYPE_FLOAT32,
							.Value = {
								.SensorFloat32 = 16,
							}	
						},
					},
					.Min = {
						.ValuesPresent = SAHPI_SRF_INTERPRETED,
						.Interpreted = {
							.Type = SAHPI_SENSOR_INTERPRETED_TYPE_FLOAT32,
							.Value = {
								.SensorFloat32 = 0,
							}	
						},
					},
				},
                        },
                        .ThresholdDefn = {
                                .IsThreshold = SAHPI_TRUE,
				.TholdCapabilities = SAHPI_STC_INTERPRETED,
				.ReadThold = SAHPI_STM_LOW_MINOR | SAHPI_STM_UP_MINOR |
				             SAHPI_STM_LOW_HYSTERESIS | SAHPI_STM_UP_HYSTERESIS,
				.FixedThold = SAHPI_STM_LOW_MINOR | SAHPI_STM_UP_MINOR |
   				              SAHPI_STM_LOW_HYSTERESIS | SAHPI_STM_UP_HYSTERESIS,
                        },
                        .Oem = 0
                },
		.mib = {
			.not_avail_indicator_num = 0,
			.write_only = 0,
                        .convert_snmpstr = SAHPI_SENSOR_INTERPRETED_TYPE_FLOAT32,
			.oid = ".1.3.6.1.4.1.2.3.51.2.2.2.1.3.0",
			.threshold_oids = {
				.InterpretedThresholds = {
					.OidLowMinor     = ".1.3.6.1.4.1.2.3.51.2.2.20.2.1.1.10.3",
					.OidUpMinor      = ".1.3.6.1.4.1.2.3.51.2.2.20.2.1.1.6.3",
					/*  FIXME:: Hysteresis correct? */
					.OidLowHysteresis = ".1.3.6.1.4.1.2.3.51.2.2.20.2.1.1.11.3",
					.OidUpHysteresis  = ".1.3.6.1.4.1.2.3.51.2.2.20.2.1.1.7.3"
				},
			},
		},
                .comment = "Plus 12 Volt power supply voltage reading expressed in volts(V)"
        },

        {} /* Terminate array with a null element */
};

/*********************
 * Sub-chassis Sensors
 *********************/

struct snmp_bc_sensor snmp_bc_subchassis_sensors[] = {

        {} /* Terminate array with a null element */
};

/***************
 * Blade Sensors
 ***************/

struct snmp_bc_sensor snmp_bc_blade_sensors[] = {

        /* Blade Error LED */
        {
                .sensor = {
                        .Num = 1,
                        .Type = SAHPI_PLATFORM_VIOLATION,
                        .Category = SAHPI_EC_SEVERITY,
                        .EventCtrl = SAHPI_SEC_ENTIRE_SENSOR,
                        .Events = SAHPI_ES_OK | SAHPI_ES_CRITICAL,
                        .Ignore = SAHPI_FALSE,
                        .DataFormat = {
                                .ReadingFormats = SAHPI_SRF_EVENT_STATE | SAHPI_SRF_RAW,
                                .IsNumeric = SAHPI_TRUE, 
                                .SignFormat = SAHPI_SDF_UNSIGNED, 
                                .BaseUnits = SAHPI_SU_UNSPECIFIED,
                                .ModifierUnits = SAHPI_SU_UNSPECIFIED,
                                .ModifierUse = SAHPI_SMUU_NONE,
                                .FactorsStatic = SAHPI_TRUE,
                                .Factors = {
                                        .Linearization = SAHPI_SL_LINEAR,
                                },
                                .Percentage = SAHPI_FALSE,
                                .Range = {
                                        .Flags = SAHPI_SRF_MIN | SAHPI_SRF_MAX,
                                        .Max = {
                                                .ValuesPresent = SAHPI_SRF_EVENT_STATE | SAHPI_SRF_RAW,
                                                .Raw = 1,
                                                .EventStatus = {
                                                        .SensorStatus = SAHPI_SENSTAT_EVENTS_ENABLED,
                                                        .EventStatus = SAHPI_ES_CRITICAL
                                                }
                                        },
                                        .Min = {
                                                .ValuesPresent = SAHPI_SRF_EVENT_STATE | SAHPI_SRF_RAW,
                                                .Raw = 0,
                                                .EventStatus = {
                                                        .SensorStatus = SAHPI_SENSTAT_EVENTS_ENABLED,
                                                        .EventStatus = SAHPI_ES_OK
                                                }
                                        }
                                }
                        },
                        .ThresholdDefn = {
                                .IsThreshold = SAHPI_FALSE
                        },
                        .Oem = 0
                },
		.mib = {
			.not_avail_indicator_num = 0,
			.write_only = 0,
			.convert_snmpstr = -1,
			.oid = ".1.3.6.1.4.1.2.3.51.2.2.8.2.1.1.7.x",
		},
                .comment = "Blade LED - Error"
        },
        /*  Blade KVM Usage LED */
        {
                .sensor = {
                        .Num = 2,
                        .Type = SAHPI_BUTTON,
                        .Category = SAHPI_EC_USAGE,
                        .EventCtrl = SAHPI_SEC_ENTIRE_SENSOR,
                        .Events = SAHPI_ES_IDLE | SAHPI_ES_ACTIVE | SAHPI_ES_BUSY,
                        .Ignore = SAHPI_FALSE,
                        .DataFormat = {
                                .ReadingFormats = SAHPI_SRF_EVENT_STATE | SAHPI_SRF_RAW,
                                .IsNumeric = SAHPI_TRUE, 
                                .SignFormat = SAHPI_SDF_UNSIGNED, 
                                .BaseUnits = SAHPI_SU_UNSPECIFIED,
                                .ModifierUnits = SAHPI_SU_UNSPECIFIED,
                                .ModifierUse = SAHPI_SMUU_NONE,
                                .FactorsStatic = SAHPI_TRUE,
                                .Factors = {
                                        .Linearization = SAHPI_SL_LINEAR,
                                },
                                .Percentage = SAHPI_FALSE,
                                .Range = {
                                        .Flags = SAHPI_SRF_MIN | SAHPI_SRF_NOMINAL |SAHPI_SRF_MAX,
                                        .Max = {
                                                .ValuesPresent = SAHPI_SRF_EVENT_STATE | SAHPI_SRF_RAW,
                                                .Raw = 2,
                                                .EventStatus = {
                                                        .SensorStatus = SAHPI_SENSTAT_EVENTS_ENABLED,
                                                        .EventStatus = SAHPI_ES_BUSY
                                                }
                                        },
                                        .Nominal = {
                                                .ValuesPresent = SAHPI_SRF_EVENT_STATE | SAHPI_SRF_RAW,
                                                .Raw = 1,
                                                .EventStatus = {
                                                        .SensorStatus = SAHPI_SENSTAT_EVENTS_ENABLED,
                                                        .EventStatus = SAHPI_ES_ACTIVE
                                                }
                                        },
                                        .Min = {
                                                .ValuesPresent = SAHPI_SRF_EVENT_STATE | SAHPI_SRF_RAW,
                                                .Raw = 0,
                                                .EventStatus = {
                                                        .SensorStatus = SAHPI_SENSTAT_EVENTS_ENABLED,
                                                        .EventStatus = SAHPI_ES_IDLE
                                                }
                                        }
                                }
                        },
                        .ThresholdDefn = {
                                .IsThreshold = SAHPI_FALSE
                        },
                        .Oem = 0
                },
		.mib = {
			.not_avail_indicator_num = 0,
			.write_only = 0,
			.convert_snmpstr = -1,
			.oid = ".1.3.6.1.4.1.2.3.51.2.2.8.2.1.1.9.x",
		},
                .comment = "Blade LED - KVM usage"
        },
        /*  Blade Media Tray Usage LED */
        {
                .sensor = {
                        .Num = 3,
                        .Type = SAHPI_BUTTON,
                        .Category = SAHPI_EC_USAGE,
                        .EventCtrl = SAHPI_SEC_ENTIRE_SENSOR,
                        .Events = SAHPI_ES_IDLE | SAHPI_ES_ACTIVE | SAHPI_ES_BUSY,
                        .Ignore = SAHPI_FALSE,
                        .DataFormat = {
                                .ReadingFormats = SAHPI_SRF_EVENT_STATE | SAHPI_SRF_RAW,
                                .IsNumeric = SAHPI_TRUE, 
                                .SignFormat = SAHPI_SDF_UNSIGNED, 
                                .BaseUnits = SAHPI_SU_UNSPECIFIED,
                                .ModifierUnits = SAHPI_SU_UNSPECIFIED,
                                .ModifierUse = SAHPI_SMUU_NONE,
                                .FactorsStatic = SAHPI_TRUE,
                                .Factors = {
                                        .Linearization = SAHPI_SL_LINEAR,
                                },
                                .Percentage = SAHPI_FALSE,
                                .Range = {
                                        .Flags = SAHPI_SRF_MIN | SAHPI_SRF_NOMINAL |SAHPI_SRF_MAX,
                                        .Max = {
                                                .ValuesPresent = SAHPI_SRF_EVENT_STATE | SAHPI_SRF_RAW,
                                                .Raw = 2,
                                                .EventStatus = {
                                                        .SensorStatus = SAHPI_SENSTAT_EVENTS_ENABLED,
                                                        .EventStatus = SAHPI_ES_BUSY
                                                }
                                        },
                                        .Nominal = {
                                                .ValuesPresent = SAHPI_SRF_EVENT_STATE | SAHPI_SRF_RAW,
                                                .Raw = 1,
                                                .EventStatus = {
                                                        .SensorStatus = SAHPI_SENSTAT_EVENTS_ENABLED,
                                                        .EventStatus = SAHPI_ES_ACTIVE
                                                }
                                        },
                                        .Min = {
                                                .ValuesPresent = SAHPI_SRF_EVENT_STATE | SAHPI_SRF_RAW,
                                                .Raw = 0,
                                                .EventStatus = {
                                                        .SensorStatus = SAHPI_SENSTAT_EVENTS_ENABLED,
                                                        .EventStatus = SAHPI_ES_IDLE
                                                }
                                        }
                                }
                        },
                        .ThresholdDefn = {
                                .IsThreshold = SAHPI_FALSE
                        },
                        .Oem = 0
                },
		.mib = {
			.not_avail_indicator_num = 0,
			.write_only = 0,
			.convert_snmpstr = -1,
			.oid = ".1.3.6.1.4.1.2.3.51.2.2.8.2.1.1.10.x",
		},
                .comment = "Blade LED - Media Tray usage"
        },
        /* CPU 1 thermal sensor */
        {
                .sensor = {
                        .Num = 4,
                        .Type = SAHPI_TEMPERATURE,
                        .Category = SAHPI_EC_THRESHOLD,
                        .EventCtrl = SAHPI_SEC_GLOBAL_DISABLE,
                        .Events = SAHPI_ES_UPPER_MINOR | SAHPI_ES_UPPER_CRIT,
                        .Ignore = SAHPI_FALSE,
                        .DataFormat = {
                                .ReadingFormats = SAHPI_SRF_INTERPRETED,
                                .IsNumeric = SAHPI_TRUE,
                                .SignFormat = SAHPI_SDF_UNSIGNED,
                                .BaseUnits = SAHPI_SU_DEGREES_C,
                                .ModifierUnits = SAHPI_SU_UNSPECIFIED,
                                .ModifierUse = SAHPI_SMUU_NONE,
                                .FactorsStatic = SAHPI_TRUE,
                                .Factors = {
                                        .Linearization = SAHPI_SL_LINEAR,
                                },
                                .Percentage = SAHPI_FALSE,
                                .Range = {
					.Flags = SAHPI_SRF_MAX | SAHPI_SRF_MIN,
					.Max = {
						.ValuesPresent = SAHPI_SRF_INTERPRETED,
						.Interpreted = {
							.Type = SAHPI_SENSOR_INTERPRETED_TYPE_FLOAT32,
							.Value = {
								.SensorFloat32 =125,
							}	
						},
					},
					.Min = {
						.ValuesPresent = SAHPI_SRF_INTERPRETED,
						.Interpreted = {
							.Type = SAHPI_SENSOR_INTERPRETED_TYPE_FLOAT32,
							.Value = {
								.SensorFloat32 = 0,
							}	
						},
					},
				},
                        },
                        .ThresholdDefn = {
                                .IsThreshold = SAHPI_TRUE,
				.TholdCapabilities = SAHPI_STC_INTERPRETED,
				.ReadThold = SAHPI_STM_UP_MINOR | SAHPI_STM_UP_CRIT | 
				             SAHPI_STM_UP_HYSTERESIS,
				.FixedThold = SAHPI_STM_UP_MINOR | SAHPI_STM_UP_CRIT |
				              SAHPI_STM_UP_HYSTERESIS,
                        },
                        .Oem = 0
                },
		.mib = {
			.not_avail_indicator_num = 0,
			.write_only = 0,
			.convert_snmpstr = SAHPI_SENSOR_INTERPRETED_TYPE_FLOAT32,
			.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.3.1.6.x",
			.threshold_oids = {
				.InterpretedThresholds = {
					.OidUpCrit       = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.6.x",
					.OidUpMinor      = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.7.x",
					/* FIXME:: Hysteresis correct? */
					.OidUpHysteresis = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.8.x",
				},
			},
		},
                .comment = "Blade CPU 1 temperature in degrees centigrade(C)."
        },
        /* CPU 2 thermal sensor */
        {
                .sensor = {
                        .Num = 5,
                        .Type = SAHPI_TEMPERATURE,
                        .Category = SAHPI_EC_THRESHOLD,
                        .EventCtrl = SAHPI_SEC_GLOBAL_DISABLE,
                        .Events = SAHPI_ES_UPPER_MINOR | SAHPI_ES_UPPER_CRIT,
                        .Ignore = SAHPI_FALSE,
                        .DataFormat = {
                                .ReadingFormats = SAHPI_SRF_INTERPRETED,
                                .IsNumeric = SAHPI_TRUE,
                                .SignFormat = SAHPI_SDF_UNSIGNED,
                                .BaseUnits = SAHPI_SU_DEGREES_C,
                                .ModifierUnits = SAHPI_SU_UNSPECIFIED,
                                .ModifierUse = SAHPI_SMUU_NONE,
                                .FactorsStatic = SAHPI_TRUE,
                                .Factors = {
                                        .Linearization = SAHPI_SL_LINEAR,
                                },
                                .Percentage = SAHPI_FALSE,
                                .Range = {
					.Flags = SAHPI_SRF_MAX | SAHPI_SRF_MIN,
					.Max = {
						.ValuesPresent = SAHPI_SRF_INTERPRETED,
						.Interpreted = {
							.Type = SAHPI_SENSOR_INTERPRETED_TYPE_FLOAT32,
							.Value = {
								.SensorFloat32 =125,
							}	
						},
					},
					.Min = {
						.ValuesPresent = SAHPI_SRF_INTERPRETED,
						.Interpreted = {
							.Type = SAHPI_SENSOR_INTERPRETED_TYPE_FLOAT32,
							.Value = {
								.SensorFloat32 = 0,
							}	
						},
					},
				},
                        },
                        .ThresholdDefn = {
                                .IsThreshold = SAHPI_TRUE,
				.TholdCapabilities = SAHPI_STC_INTERPRETED,
				.ReadThold = SAHPI_STM_UP_MINOR | SAHPI_STM_UP_CRIT |
				             SAHPI_STM_UP_HYSTERESIS,
				.FixedThold = SAHPI_STM_UP_MINOR | SAHPI_STM_UP_CRIT |
				              SAHPI_STM_UP_HYSTERESIS,
                        },
                        .Oem = 0
                },
		.mib = {
			.not_avail_indicator_num = 0,
			.write_only = 0,
			.convert_snmpstr = SAHPI_SENSOR_INTERPRETED_TYPE_FLOAT32,
			.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.3.1.7.x",
			.threshold_oids = {
				.InterpretedThresholds = {
					.OidUpCrit       = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.9.x",
					.OidUpMinor      = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.10.x",
					/* FIXME:: Hysteresis correct? */
					.OidUpHysteresis = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.11.x",
				},
			},
		},
                .comment = "Blade CPU 2 temperature in degrees centigrade(C)."
        },
        /* DASD 1 thermal sensor */
        {
                .sensor = {
                        .Num = 6,
                        .Type = SAHPI_TEMPERATURE,
                        .Category = SAHPI_EC_THRESHOLD,
                        .EventCtrl = SAHPI_SEC_GLOBAL_DISABLE,
                        .Events = SAHPI_ES_UPPER_MINOR | SAHPI_ES_UPPER_CRIT,
                        .Ignore = SAHPI_FALSE,
                        .DataFormat = {
                                .ReadingFormats = SAHPI_SRF_INTERPRETED,
                                .IsNumeric = SAHPI_TRUE,
                                .SignFormat = SAHPI_SDF_UNSIGNED,
                                .BaseUnits = SAHPI_SU_DEGREES_C,
                                .ModifierUnits = SAHPI_SU_UNSPECIFIED,
                                .ModifierUse = SAHPI_SMUU_NONE,
                                .FactorsStatic = SAHPI_TRUE,
                                .Factors = {
                                        .Linearization = SAHPI_SL_LINEAR,
                                },
                                .Percentage = SAHPI_FALSE,
                                .Range = {
					.Flags = SAHPI_SRF_MAX | SAHPI_SRF_MIN,
					.Max = {
						.ValuesPresent = SAHPI_SRF_INTERPRETED,
						.Interpreted = {
							.Type = SAHPI_SENSOR_INTERPRETED_TYPE_FLOAT32,
							.Value = {
								.SensorFloat32 =125,
							}	
						},
					},
					.Min = {
						.ValuesPresent = SAHPI_SRF_INTERPRETED,
						.Interpreted = {
							.Type = SAHPI_SENSOR_INTERPRETED_TYPE_FLOAT32,
							.Value = {
								.SensorFloat32 = 0,
							}	
						},
					},
				},
                        },
                        .ThresholdDefn = {
                                .IsThreshold = SAHPI_TRUE,
				.TholdCapabilities = SAHPI_STC_INTERPRETED,
				.ReadThold = SAHPI_STM_UP_MINOR | SAHPI_STM_UP_CRIT |
				             SAHPI_STM_UP_HYSTERESIS,
				.FixedThold = SAHPI_STM_UP_MINOR | SAHPI_STM_UP_CRIT |
				              SAHPI_STM_UP_HYSTERESIS,
                        },
                        .Oem = 0
                },
		.mib = {
			.not_avail_indicator_num = 0,
			.write_only = 0,
			.convert_snmpstr = SAHPI_SENSOR_INTERPRETED_TYPE_FLOAT32,
			.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.3.1.8.x",
			.threshold_oids = {
				.InterpretedThresholds = {
					.OidUpCrit       = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.12.x",
					.OidUpMinor      = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.13.x",
					/* FIXME:: Hysteresis correct? */
					.OidUpHysteresis = ".1.3.6.1.4.1.2.3.51.2.22.1.5.4.1.14.x",
				},
			},
		},
                .comment = "Blade DASD 1 temperature in degrees centigrade(C)."
        },
        /* Blade's 5V voltage sensor */
        {
                .sensor = {
                        .Num = 7,
                        .Type = SAHPI_VOLTAGE,
                        .Category = SAHPI_EC_THRESHOLD,
                        .EventCtrl = SAHPI_SEC_GLOBAL_DISABLE,
                        .Events = SAHPI_ES_LOWER_MINOR | SAHPI_ES_UPPER_MINOR,
                        .Ignore = SAHPI_FALSE,
                        .DataFormat = {
                                .ReadingFormats = SAHPI_SRF_INTERPRETED,
                                .IsNumeric = SAHPI_TRUE,
                                .SignFormat = SAHPI_SDF_UNSIGNED,
                                .BaseUnits = SAHPI_SU_VOLTS,
                                .ModifierUnits = SAHPI_SU_UNSPECIFIED,
                                .ModifierUse = SAHPI_SMUU_NONE,
                                .FactorsStatic = SAHPI_TRUE,
                                .Factors = {
                                        .Linearization = SAHPI_SL_LINEAR,
                                },
                                .Percentage = SAHPI_FALSE,
                                .Range = {
					.Flags = SAHPI_SRF_MAX | SAHPI_SRF_MIN,
					.Max = {
						.ValuesPresent = SAHPI_SRF_INTERPRETED,
						.Interpreted = {
							.Type = SAHPI_SENSOR_INTERPRETED_TYPE_FLOAT32,
							.Value = {
								.SensorFloat32 = 6.7,
							}	
						},
					},
					.Min = {
						.ValuesPresent = SAHPI_SRF_INTERPRETED,
						.Interpreted = {
							.Type = SAHPI_SENSOR_INTERPRETED_TYPE_FLOAT32,
							.Value = {
								.SensorFloat32 = 0,
							}	
						},
					},
				},
                        },
                        .ThresholdDefn = {
                                .IsThreshold = SAHPI_TRUE,
				.TholdCapabilities = SAHPI_STC_INTERPRETED,
				.ReadThold = SAHPI_STM_LOW_MINOR | SAHPI_STM_UP_MINOR,
				.FixedThold = SAHPI_STM_LOW_MINOR | SAHPI_STM_UP_MINOR,
                        },
                        .Oem = 0
                },
		.mib = {
			.not_avail_indicator_num = 0,
			.write_only = 0,
                        .convert_snmpstr = SAHPI_SENSOR_INTERPRETED_TYPE_FLOAT32,
			.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.6.x",
			.threshold_oids = {
				.InterpretedThresholds = {
					.OidLowMinor     = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.7.x",
					.OidUpMinor      = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.6.x",
				},
			},
		},
                .comment = "Blade Plus 5 Volt power supply voltage reading expressed in volts(V)"
        },
        /* Blade's 3.3V voltage sensor */
        {
                .sensor = {
                        .Num = 8,
                        .Type = SAHPI_VOLTAGE,
                        .Category = SAHPI_EC_THRESHOLD,
                        .EventCtrl = SAHPI_SEC_GLOBAL_DISABLE,
                        .Events = SAHPI_ES_LOWER_MINOR | SAHPI_ES_UPPER_MINOR,
                        .Ignore = SAHPI_FALSE,
                        .DataFormat = {
                                .ReadingFormats = SAHPI_SRF_INTERPRETED,
                                .IsNumeric = SAHPI_TRUE,
                                .SignFormat = SAHPI_SDF_UNSIGNED,
                                .BaseUnits = SAHPI_SU_VOLTS,
                                .ModifierUnits = SAHPI_SU_UNSPECIFIED,
                                .ModifierUse = SAHPI_SMUU_NONE,
                                .FactorsStatic = SAHPI_TRUE,
                                .Factors = {
                                        .Linearization = SAHPI_SL_LINEAR,
                                },
                                .Percentage = SAHPI_FALSE,
                                .Range = {
					.Flags = SAHPI_SRF_MAX | SAHPI_SRF_MIN,
					.Max = {
						.ValuesPresent = SAHPI_SRF_INTERPRETED,
						.Interpreted = {
							.Type = SAHPI_SENSOR_INTERPRETED_TYPE_FLOAT32,
							.Value = {
								.SensorFloat32 = 3.6,
							}	
						},
					},
					.Min = {
						.ValuesPresent = SAHPI_SRF_INTERPRETED,
						.Interpreted = {
							.Type = SAHPI_SENSOR_INTERPRETED_TYPE_FLOAT32,
							.Value = {
								.SensorFloat32 = 0,
							}	
						},
					},
				},
                        },
                        .ThresholdDefn = {
                                .IsThreshold = SAHPI_TRUE,
				.TholdCapabilities = SAHPI_STC_INTERPRETED,
				.ReadThold = SAHPI_STM_UP_MINOR | SAHPI_STM_UP_MINOR,
				.FixedThold = SAHPI_STM_UP_MINOR | SAHPI_STM_UP_MINOR,
                        },
                        .Oem = 0
                },
		.mib = {
			.not_avail_indicator_num = 0,
			.write_only = 0,
                        .convert_snmpstr = SAHPI_SENSOR_INTERPRETED_TYPE_FLOAT32,
			.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.7.x",
			.threshold_oids = {
				.InterpretedThresholds = {
					.OidLowMinor     = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.9.x",
					.OidUpMinor      = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.8.x",
				},
			},
		},
                /* Warning Under voltage - ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.9.x ", */
                .comment = "Blade Plus 3.3 Volt power supply voltage reading expressed in volts(V)"
        },
        /* Blade's 12V voltage sensor */
        {
                .sensor = {
                        .Num = 9,
                        .Type = SAHPI_VOLTAGE,
                        .Category = SAHPI_EC_THRESHOLD,
                        .EventCtrl = SAHPI_SEC_GLOBAL_DISABLE,
                        .Events = SAHPI_ES_LOWER_MINOR | SAHPI_ES_UPPER_MINOR,
                        .Ignore = SAHPI_FALSE,
                        .DataFormat = {
                                .ReadingFormats = SAHPI_SRF_INTERPRETED,
                                .IsNumeric = SAHPI_TRUE,
                                .SignFormat = SAHPI_SDF_UNSIGNED,
                                .BaseUnits = SAHPI_SU_VOLTS,
                                .ModifierUnits = SAHPI_SU_UNSPECIFIED,
                                .ModifierUse = SAHPI_SMUU_NONE,
                                .FactorsStatic = SAHPI_TRUE,
                                .Factors = {
                                        .Linearization = SAHPI_SL_LINEAR,
                                },
                                .Percentage = SAHPI_FALSE,
                                .Range = {
					.Flags = SAHPI_SRF_MAX | SAHPI_SRF_MIN,
					.Max = {
						.ValuesPresent = SAHPI_SRF_INTERPRETED,
						.Interpreted = {
							.Type = SAHPI_SENSOR_INTERPRETED_TYPE_FLOAT32,
							.Value = {
								.SensorFloat32 = 16,
							}	
						},
					},
					.Min = {
						.ValuesPresent = SAHPI_SRF_INTERPRETED,
						.Interpreted = {
							.Type = SAHPI_SENSOR_INTERPRETED_TYPE_FLOAT32,
							.Value = {
								.SensorFloat32 = 0,
							}	
						},
					},
				},
                        },
                        .ThresholdDefn = {
                                .IsThreshold = SAHPI_TRUE,
				.TholdCapabilities = SAHPI_STC_INTERPRETED,
				.ReadThold = SAHPI_STM_LOW_MINOR | SAHPI_STM_UP_MINOR,
				.FixedThold = SAHPI_STM_LOW_MINOR | SAHPI_STM_UP_MINOR,
                        },
                        .Oem = 0
                },
		.mib = {
			.not_avail_indicator_num = 0,
			.write_only = 0,
                        .convert_snmpstr = SAHPI_SENSOR_INTERPRETED_TYPE_FLOAT32,
			.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.8.x",
			.threshold_oids = {
				.InterpretedThresholds = {
					.OidLowMinor     = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.11.x",
					.OidUpMinor      = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.10.x",
				},
			},
		},
		/* Warning Under voltage - ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.11.x ", */
                .comment = "Blade Plus 12 Volt power supply voltage reading expressed in volts(V)"
        },
        /* Blade's 2.5V voltage sensor */
        {
                .sensor = {
                        .Num = 10,
                        .Type = SAHPI_VOLTAGE,
                        .Category = SAHPI_EC_THRESHOLD,
                        .EventCtrl = SAHPI_SEC_GLOBAL_DISABLE,
                        .Events = SAHPI_ES_LOWER_MINOR | SAHPI_ES_UPPER_MINOR,
                        .Ignore = SAHPI_FALSE,
                        .DataFormat = {
                                .ReadingFormats = SAHPI_SRF_INTERPRETED,
                                .IsNumeric = SAHPI_TRUE,
                                .SignFormat = SAHPI_SDF_UNSIGNED,
                                .BaseUnits = SAHPI_SU_VOLTS,
                                .ModifierUnits = SAHPI_SU_UNSPECIFIED,
                                .ModifierUse = SAHPI_SMUU_NONE,
                                .FactorsStatic = SAHPI_TRUE,
                                .Factors = {
                                        .Linearization = SAHPI_SL_LINEAR,
                                },
                                .Percentage = SAHPI_FALSE,
                                .Range = {
					.Flags = SAHPI_SRF_MAX | SAHPI_SRF_MIN,
					.Max = {
						.ValuesPresent = SAHPI_SRF_INTERPRETED,
						.Interpreted = {
							.Type = SAHPI_SENSOR_INTERPRETED_TYPE_FLOAT32,
							.Value = {
								.SensorFloat32 = 4.4,
							}	
						},
					},
					.Min = {
						.ValuesPresent = SAHPI_SRF_INTERPRETED,
						.Interpreted = {
							.Type = SAHPI_SENSOR_INTERPRETED_TYPE_FLOAT32,
							.Value = {
								.SensorFloat32 = 0,
							}	
						},
					},
				},
                        },
                        .ThresholdDefn = {
                                .IsThreshold = SAHPI_TRUE,
				.TholdCapabilities = SAHPI_STC_INTERPRETED,
				.ReadThold = SAHPI_STM_LOW_MINOR | SAHPI_STM_UP_MINOR,
				.FixedThold = SAHPI_STM_LOW_MINOR | SAHPI_STM_UP_MINOR,
                        },
                        .Oem = 0
                },
		.mib = {
			.not_avail_indicator_num = 0,
			.write_only = 0,
                        .convert_snmpstr = SAHPI_SENSOR_INTERPRETED_TYPE_FLOAT32,
			.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.10.x",
			.threshold_oids = {
				.InterpretedThresholds = {
					.OidLowMinor     = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.15.x",
					.OidUpMinor      = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.14.x",
				},
			},
		},
                /* Warning Under voltage - ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.15.x ", */
                .comment = "Blade Plus 2.5 Volt power supply voltage reading expressed in volts(V)"
        },
        /* Blade's 1.5V voltage sensor */
        {
                .sensor = {
                        .Num = 11,
                        .Type = SAHPI_VOLTAGE,
                        .Category = SAHPI_EC_THRESHOLD,
                        .EventCtrl = SAHPI_SEC_GLOBAL_DISABLE,
                        .Events = SAHPI_ES_LOWER_MINOR | SAHPI_ES_UPPER_MINOR,
                        .Ignore = SAHPI_FALSE,
                        .DataFormat = {
                                .ReadingFormats = SAHPI_SRF_INTERPRETED,
                                .IsNumeric = SAHPI_TRUE,
                                .SignFormat = SAHPI_SDF_UNSIGNED,
                                .BaseUnits = SAHPI_SU_VOLTS,
                                .ModifierUnits = SAHPI_SU_UNSPECIFIED,
                                .ModifierUse = SAHPI_SMUU_NONE,
                                .FactorsStatic = SAHPI_TRUE,
                                .Factors = {
                                        .Linearization = SAHPI_SL_LINEAR,
                                },
                                .Percentage = SAHPI_FALSE,
                                .Range = {
					.Flags = SAHPI_SRF_MAX | SAHPI_SRF_MIN,
					.Max = {
						.ValuesPresent = SAHPI_SRF_INTERPRETED,
						.Interpreted = {
							.Type = SAHPI_SENSOR_INTERPRETED_TYPE_FLOAT32,
							.Value = {
								.SensorFloat32 = 4.4,
							}	
						},
					},
					.Min = {
						.ValuesPresent = SAHPI_SRF_INTERPRETED,
						.Interpreted = {
							.Type = SAHPI_SENSOR_INTERPRETED_TYPE_FLOAT32,
							.Value = {
								.SensorFloat32 = 0,
							}	
						},
					},
				},
                        },
                        .ThresholdDefn = {
                                .IsThreshold = SAHPI_TRUE,
				.TholdCapabilities = SAHPI_STC_INTERPRETED,
				.ReadThold = SAHPI_STM_LOW_MINOR | SAHPI_STM_UP_MINOR,
				.FixedThold = SAHPI_STM_LOW_MINOR | SAHPI_STM_UP_MINOR,
                        },
                        .Oem = 0
                },
		.mib = {
			.not_avail_indicator_num = 0,
			.write_only = 0,
                        .convert_snmpstr = SAHPI_SENSOR_INTERPRETED_TYPE_FLOAT32,
			.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.11.x",
			.threshold_oids = {
				.InterpretedThresholds = {
					.OidLowMinor     = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.17.x",
					.OidUpMinor      = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.16.x",
				},
			}
		},
                /* Warning Under voltage - ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.17.x ", */
                .comment = "Blade Plus 1.5 Volt power supply voltage reading expressed in volts(V)"
        },
        /* Blade's 1.25V voltage sensor */
        {
                .sensor = {
                        .Num = 12,
                        .Type = SAHPI_VOLTAGE,
                        .Category = SAHPI_EC_THRESHOLD,
                        .EventCtrl = SAHPI_SEC_GLOBAL_DISABLE,
                        .Events = SAHPI_ES_LOWER_MINOR | SAHPI_ES_UPPER_MINOR,
                        .Ignore = SAHPI_FALSE,
                        .DataFormat = {
                                .ReadingFormats = SAHPI_SRF_INTERPRETED,
                                .IsNumeric = SAHPI_TRUE,
                                .SignFormat = SAHPI_SDF_UNSIGNED,
                                .BaseUnits = SAHPI_SU_VOLTS,
                                .ModifierUnits = SAHPI_SU_UNSPECIFIED,
                                .ModifierUse = SAHPI_SMUU_NONE,
                                .FactorsStatic = SAHPI_TRUE,
                                .Factors = {
                                        .Linearization = SAHPI_SL_LINEAR,
                                },
                                .Percentage = SAHPI_FALSE,
                                .Range = {
					.Flags = SAHPI_SRF_MAX | SAHPI_SRF_MIN,
					.Max = {
						.ValuesPresent = SAHPI_SRF_INTERPRETED,
						.Interpreted = {
							.Type = SAHPI_SENSOR_INTERPRETED_TYPE_FLOAT32,
							.Value = {
								.SensorFloat32 = 3.3,
							}	
						},
					},
					.Min = {
						.ValuesPresent = SAHPI_SRF_INTERPRETED,
						.Interpreted = {
							.Type = SAHPI_SENSOR_INTERPRETED_TYPE_FLOAT32,
							.Value = {
								.SensorFloat32 = 0,
							}	
						},
					},
				},
                        },
                        .ThresholdDefn = {
                                .IsThreshold = SAHPI_TRUE,
				.TholdCapabilities = SAHPI_STC_INTERPRETED,
				.ReadThold = SAHPI_STM_UP_MINOR | SAHPI_STM_UP_MINOR,
				.FixedThold = SAHPI_STM_UP_MINOR | SAHPI_STM_UP_MINOR,
                        },
                        .Oem = 0
                },
		.mib = {
			.not_avail_indicator_num = 0,
			.write_only = 0,
                        .convert_snmpstr = SAHPI_SENSOR_INTERPRETED_TYPE_FLOAT32,
			.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.12.x",
			.threshold_oids = {
				.InterpretedThresholds = {
					.OidLowMinor     = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.18.x",
					.OidUpMinor      = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.17.x",
				},
			},
		},
                /* Warning Under voltage - ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.18.x ", */
                .comment = "Blade Plus 1.25 Volt power supply voltage reading expressed in volts(V)"
        },
        /* Blade's VRM 1 voltage sensor */
        {
                .sensor = {
                        .Num = 13,
                        .Type = SAHPI_VOLTAGE,
                        .Category = SAHPI_EC_THRESHOLD,
                        .EventCtrl = SAHPI_SEC_GLOBAL_DISABLE,
			/* FIXME:: No thresholds defined - Change catagory type - new events */
                        .Events = SAHPI_ES_LOWER_MINOR | SAHPI_ES_UPPER_MINOR,
                        .Ignore = SAHPI_FALSE,
                        .DataFormat = {
                                .ReadingFormats = SAHPI_SRF_INTERPRETED,
                                .IsNumeric = SAHPI_TRUE,
                                .SignFormat = SAHPI_SDF_UNSIGNED,
                                .BaseUnits = SAHPI_SU_VOLTS,
                                .ModifierUnits = SAHPI_SU_UNSPECIFIED,
                                .ModifierUse = SAHPI_SMUU_NONE,
                                .FactorsStatic = SAHPI_TRUE,
                                .Factors = {
                                        .Linearization = SAHPI_SL_LINEAR,
                                },
                                .Percentage = SAHPI_FALSE,
                                .Range = {
					.Flags = SAHPI_SRF_MAX | SAHPI_SRF_MIN,
					.Max = {
						.ValuesPresent = SAHPI_SRF_INTERPRETED,
						.Interpreted = {
							.Type = SAHPI_SENSOR_INTERPRETED_TYPE_FLOAT32,
							.Value = {
								.SensorFloat32 = 3.6,
							}	
						},
					},
					.Min = {
						.ValuesPresent = SAHPI_SRF_INTERPRETED,
						.Interpreted = {
							.Type = SAHPI_SENSOR_INTERPRETED_TYPE_FLOAT32,
							.Value = {
								.SensorFloat32 = 0,
							}	
						},
					},
				},
                        },
                        .ThresholdDefn = {
				/* Threshold commented out in MIB */
                                .IsThreshold = SAHPI_FALSE,
                        },
                        .Oem = 0
                },
		.mib = {
			.not_avail_indicator_num = 0,
			.write_only = 0,
                        .convert_snmpstr = SAHPI_SENSOR_INTERPRETED_TYPE_FLOAT32,
			.oid = ".1.3.6.1.4.1.2.3.51.2.22.1.5.5.1.13.x",
			/* .OidUpMinor  = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.20.x ", */
			/* .OidLowMinor = ".1.3.6.1.4.1.2.3.51.2.22.1.5.6.1.21.x ", */
		},
                .comment = "Blade VRM 1 Volt power supply voltage reading expressed in volts(V)"
        },

        {} /* Terminate array with a null element */
};

/**********************
 * Blade Add In Sensors
 **********************/

struct snmp_bc_sensor snmp_bc_blade_addin_sensors[] = {

        {} /* Terminate array with a null element */
};

/***************************
 * Management Module Sensors
 ***************************/

struct snmp_bc_sensor snmp_bc_mgmnt_sensors[] = {

        {} /* Terminate array with a null element */
};

/********************
 * Media Tray Sensors
 ********************/

struct snmp_bc_sensor snmp_bc_mediatray_sensors[] = {

        {} /* Terminate array with a null element */
};

/***********************
 * Blower Module Sensors
 ***********************/

struct snmp_bc_sensor snmp_bc_fan_sensors[] = {

        {
		/* Blower fan speed */
                .sensor = {
                        .Num = 1,
                        .Type = SAHPI_FAN,
                        .Category = SAHPI_EC_UNSPECIFIED,
                        .EventCtrl = SAHPI_SEC_GLOBAL_DISABLE,
                        .Events = SAHPI_ES_UNSPECIFIED,
                        .Ignore = SAHPI_FALSE,
                        .DataFormat = {
                                .ReadingFormats = SAHPI_SRF_INTERPRETED,
                                .IsNumeric = SAHPI_TRUE,
                                .SignFormat = SAHPI_SDF_UNSIGNED,
                                .BaseUnits = SAHPI_SU_RPM,
                                .ModifierUnits = SAHPI_SU_UNSPECIFIED,
                                .ModifierUse = SAHPI_SMUU_NONE,
                                .FactorsStatic = SAHPI_TRUE,
                                .Factors = {
                                        .Linearization = SAHPI_SL_LINEAR,
                                },
                                .Percentage = SAHPI_TRUE,
                        },
                        .ThresholdDefn = {
                                .IsThreshold = SAHPI_FALSE
                        },
                        .Oem = 0
                },
		.mib = {
			.not_avail_indicator_num = 0,
			.write_only = 0,
			.convert_snmpstr = SAHPI_SENSOR_INTERPRETED_TYPE_FLOAT32,
			.oid = ".1.3.6.1.4.1.2.3.51.2.2.3.x.0",
		},
                .comment = "Blower fan speed expressed in percent(%) of maximum RPM."
        },
        {} /* Terminate array with a null element */
};

/***************
 * Power Sensors
 ***************/

struct snmp_bc_sensor snmp_bc_power_sensors[] = {

        {} /* Terminate array with a null element */
};

/****************
 * Switch Sensors
 ****************/

struct snmp_bc_sensor snmp_bc_switch_sensors[] = {

        {} /* Terminate array with a null element */
};

/*************************************************************************
 *                   Control Definitions
 *************************************************************************/

/******************
 * Chassis Controls
 ******************/

struct snmp_bc_control snmp_bc_chassis_controls[] = {

        /* Front Panel Information R/W LED */
        {
                .control = {
                        .Num = 1,
                        .Ignore = SAHPI_FALSE,
                        .OutputType = SAHPI_CTRL_LED,
			.Type = SAHPI_CTRL_TYPE_DIGITAL,
			.TypeUnion.Digital.Default = SAHPI_CTRL_STATE_OFF,
                        .Oem = 0
                },
		.mib = {
			.not_avail_indicator_num = 0,
			.write_only = 0,
			.oid = ".1.3.6.1.4.1.2.3.51.2.2.8.1.2.0",
			.digitalmap[0] = 0, /* Off */
			.digitalmap[1] = 1, /* On */
			.digitalmap[2] = -1, /* Not applicable */
			.digitalmap[3] = -1, /* Not applicable */
			.digitalmap[4] = -1, /* Not applicable */
		},
                .comment = "Front Panel LED - Information."
	},
        /* Front Panel Identify R/W LED */
        {
                .control = {
                        .Num = 2,
                        .Ignore = SAHPI_FALSE,
                        .OutputType = SAHPI_CTRL_LED,
			.Type = SAHPI_CTRL_TYPE_DIGITAL,
			.TypeUnion.Digital.Default = SAHPI_CTRL_STATE_OFF,
                        .Oem = 0
                },
		.mib = {
			.not_avail_indicator_num = 3,
			.write_only = 0,
			.oid = ".1.3.6.1.4.1.2.3.51.2.2.8.1.4.0",
			.digitalmap[0] = 0, /* Off */
			.digitalmap[1] = 1, /* On */
			.digitalmap[2] = -1, /* Not applicable */
			.digitalmap[3] = 2, /* Blinking */
			.digitalmap[4] = -1, /* Not applicable */
		},
                .comment = "Front Panel LED - Identify."
	},
        {} /* Terminate array with a null element */
};

/**********************
 * Sub-chassis Controls
 **********************/

struct snmp_bc_control snmp_bc_subchassis_controls[] = {

        {} /* Terminate array with a null element */
};

/****************
 * Blade Controls
 ****************/

struct snmp_bc_control snmp_bc_blade_controls[] = {

        /* Blade Information R/W LED */
        {
                .control = {
                        .Num = 1,
                        .Ignore = SAHPI_FALSE,
                        .OutputType = SAHPI_CTRL_LED,
			.Type = SAHPI_CTRL_TYPE_DIGITAL,
			.TypeUnion.Digital.Default = SAHPI_CTRL_STATE_OFF,
                        .Oem = 0
                },
		.mib = {
			.not_avail_indicator_num = 0,
			.write_only = 0,
			.oid = ".1.3.6.1.4.1.2.3.51.2.2.8.2.1.1.8.x",
			.digitalmap[0] = 0, /* Off */
			.digitalmap[1] = 1, /* On */
			.digitalmap[2] = -1, /* Not applicable */
			.digitalmap[3] = -1, /* Not applicable */
			.digitalmap[4] = -1, /* Not applicable */
		},
                .comment = "Blade LED - Information."
	},
        /* Blade Identify R/W LED */
        {
                .control = {
                        .Num = 2,
                        .Ignore = SAHPI_FALSE,
                        .OutputType = SAHPI_CTRL_LED,
			.Type = SAHPI_CTRL_TYPE_DIGITAL,
			.TypeUnion.Digital.Default = SAHPI_CTRL_STATE_OFF,
                        .Oem = 0
                },
		.mib = {
			.not_avail_indicator_num = 0,
			.write_only = 0,
			.oid = ".1.3.6.1.4.1.2.3.51.2.2.8.2.1.1.11.x",
			.digitalmap[0] = 0, /* Off */
			.digitalmap[1] = 1, /* On */
			.digitalmap[2] = -1, /* Not applicable */
			.digitalmap[3] = 2, /* Blinking */
			.digitalmap[4] = -1, /* Not applicable */
		},
                .comment = "Blade LED - Identify."
	},

        {} /* Terminate array with a null element */
};

/***********************
 * Blade Add In Controls
 ***********************/

struct snmp_bc_control snmp_bc_blade_addin_controls[] = {

        {} /* Terminate array with a null element */
};

/****************************
 * Management Module Controls
 ****************************/

struct snmp_bc_control snmp_bc_mgmnt_controls[] = {

        {} /* Terminate array with a null element */
};

/*********************
 * Media Tray Controls
 *********************/

struct snmp_bc_control snmp_bc_mediatray_controls[] = {

        {} /* Terminate array with a null element */
};

/*****************
 * Blower Controls
 *****************/

struct snmp_bc_control snmp_bc_fan_controls[] = {

        {} /* Terminate array with a null element */
};

/****************
 * Power Controls
 ****************/

struct snmp_bc_control snmp_bc_power_controls[] = {

        {} /* Terminate array with a null element */
};

/************************
 * Switch Module Controls
 ************************/

struct snmp_bc_control snmp_bc_switch_controls[] = {

	{} /* Terminate array with a null element */
};
 
/*************************************************************************
 *                   Inventory Definitions
 *************************************************************************/

/*************
 * Chassis VPD
 *************/

struct snmp_bc_inventory snmp_bc_chassis_inventories[] = {
        {
                .inventory = {
			.EirId = 1,
                        .Oem = 0,
                },
                .mib = {
                        .not_avail_indicator_num = 0,
                        .write_only = 0,
                	.inventory_type = SAHPI_INVENT_RECTYPE_CHASSIS_INFO,
                	.chassis_type = SAHPI_INVENT_CTYP_RACKMOUNT,
			.oid = {
                        	.OidMfgDateTime = '\0',   /* Set to SAHPI_TIME_UNSPECIFIED */
                        	.OidManufacturer = ".1.3.6.1.4.1.2.3.51.2.2.21.1.1.5.0",
                        	.OidProductName = ".1.3.6.1.4.1.2.3.51.2.2.21.1.1.1.0",
                        	.OidProductVersion = ".1.3.6.1.4.1.2.3.51.2.2.21.1.1.6.0",
                        	.OidModelNumber = ".1.3.6.1.4.1.2.3.51.2.2.21.1.1.2.0",
                        	.OidSerialNumber = ".1.3.6.1.4.1.2.3.51.2.2.21.1.1.3.0",
                        	.OidPartNumber = ".1.3.6.1.4.1.2.3.51.2.2.21.1.1.7.0",
                        	.OidFileId = '\0',
                        	.OidAssetTag = '\0',
                        	/* UUID .1.3.6.1.4.1.2.3.51.2.2.21.1.1.4.0  */
                	}
		},
                .comment = "Chassis VPD",
        },

        {} /* Terminate array with a null element */
};

/*****************
 * Sub-chassis VPD
 ******************/

struct snmp_bc_inventory snmp_bc_subchassis_inventories[] = {

        {} /* Terminate array with a null element */
};

/*********
 * Fan VPD             
 **********/

struct snmp_bc_inventory snmp_bc_fan_inventories[] = {

        {} /* Terminate array with a null element */
};

/***********************
 * Management Module VPD
 ***********************/

struct snmp_bc_inventory snmp_bc_mgmnt_inventories[] = {
        {
                .inventory = {
			.EirId = 4,
                        .Oem = 0,
                },
               	.mib = { 
                        .not_avail_indicator_num = 0,
                        .write_only = 0,
                	.inventory_type = SAHPI_INVENT_RECTYPE_BOARD_INFO,
                	.chassis_type = SAHPI_INVENT_CTYP_RACKMOUNT,
			.oid = {
                        	.OidMfgDateTime = '\0',   /* Set to SAHPI_TIME_UNSPECIFIED */
                        	.OidManufacturer = ".1.3.6.1.4.1.2.3.51.2.2.21.2.1.1.3.x",
                        	.OidProductName = '\0',
                        	.OidProductVersion = ".1.3.6.1.4.1.2.3.51.2.2.21.2.1.1.5.x",
                        	.OidModelNumber = '\0',
                        	.OidSerialNumber = '\0',
                        	.OidPartNumber = ".1.3.6.1.4.1.2.3.51.2.2.21.2.1.1.4.x",
                        	.OidFileId = '\0',
                        	.OidAssetTag = '\0',
                        	/* UUID .1.3.6.1.4.1.2.3.51.2.2.21.2.1.1.6.x */
               		}
		 },
                .comment = "Management Module VPD",
        },

        {} /* Terminate array with a null element */
};

/*******************
 * Switch Module VPD
 *******************/

struct snmp_bc_inventory snmp_bc_switch_inventories[] = {
        {
                .inventory = {
			.EirId = 5,
                        .Oem = 0,
                },
                .mib = {
                        .not_avail_indicator_num = 0,
                        .write_only = 0,
                	.inventory_type = SAHPI_INVENT_RECTYPE_BOARD_INFO,
                	.chassis_type = SAHPI_INVENT_CTYP_RACKMOUNT,
			.oid = {
                        	.OidMfgDateTime = '\0',   /* Set to SAHPI_TIME_UNSPECIFIED */
                        	.OidManufacturer = ".1.3.6.1.4.1.2.3.51.2.2.21.6.1.1.3.x",
                        	.OidProductName = '\0',
                        	.OidProductVersion = ".1.3.6.1.4.1.2.3.51.2.2.21.6.1.1.5.x",
                        	.OidModelNumber = '\0',
                        	.OidSerialNumber = '\0',
                        	.OidPartNumber = ".1.3.6.1.4.1.2.3.51.2.2.21.6.1.1.4.x",
                        	.OidFileId = '\0',
                        	.OidAssetTag = '\0',
                        	/* UUID .1.3.6.1.4.1.2.3.51.2.2.21.6.1.1.8.x */
                	}
		},
                .comment = "Switch Module VPD",
        },

        {} /* Terminate array with a null element */
};

/************
 * Blade VPD
 ************/

struct snmp_bc_inventory snmp_bc_blade_inventories[] = {
        {
                .inventory = {
			.EirId = 6,
                        .Oem = 0,
                },
                .mib = {	
                        .not_avail_indicator_num = 0,
                        .write_only = 0,
                	.inventory_type = SAHPI_INVENT_RECTYPE_BOARD_INFO,
                	.chassis_type = SAHPI_INVENT_CTYP_RACKMOUNT,
			.oid = {
                        	.OidMfgDateTime = '\0',   /* Set to SAHPI_TIME_UNSPECIFIED */
                        	.OidManufacturer = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.3.x",
                        	.OidProductName = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.7.x",
                        	.OidProductVersion = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.5.x",
                        	.OidModelNumber = '\0',
                        	.OidSerialNumber = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.6.x",
                        	.OidPartNumber = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.4.x",
                        	.OidFileId = '\0',
                        	.OidAssetTag = '\0',
                       	 	/* UUID .1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.8.x */
			}
                },
                .comment = "Blade VPD",
        },

        {} /* Terminate array with a null element */
};

/**********************************
 * Blade Add In (Daughter Card) VPD
 **********************************/

struct snmp_bc_inventory snmp_bc_blade_addin_inventories[] = {
        {
                .inventory = {
			.EirId = 7,
                        .Oem = 0,
                },
               	.mib = {
                        .not_avail_indicator_num = 0,
                        .write_only = 0,
                	.inventory_type = SAHPI_INVENT_RECTYPE_BOARD_INFO,
                	.chassis_type = SAHPI_INVENT_CTYP_RACKMOUNT,
			.oid = {
                        	.OidMfgDateTime = '\0',   /* Set to SAHPI_TIME_UNSPECIFIED */
                        	.OidManufacturer = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.10.x",
                        	.OidProductName = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.9.x", /* Type */
                        	.OidProductVersion = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.12.x",
                        	.OidModelNumber = '\0',
                        	.OidSerialNumber = '\0',
                        	.OidPartNumber = ".1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.11.x",
                        	.OidFileId = '\0',
                        	.OidAssetTag = '\0',
                        	/* UUID .1.3.6.1.4.1.2.3.51.2.2.21.4.1.1.13.x*/
			}
                },
                .comment = "Blade Daughter Card VPD",
        },

        {} /* Terminate array with a null element */
};

/****************
 * Media Tray VPD
 *****************/

struct snmp_bc_inventory snmp_bc_mediatray_inventories[] = {
        {
                .inventory = {
			.EirId = 8,
                        .Oem = 0,
                },
                .mib = {
                        .not_avail_indicator_num = 0,
                        .write_only = 0,
                	.inventory_type = SAHPI_INVENT_RECTYPE_BOARD_INFO,
                	.chassis_type = SAHPI_INVENT_CTYP_RACKMOUNT,
			.oid = {
                        	.OidMfgDateTime = '\0',   /* Set to SAHPI_TIME_UNSPECIFIED */
                        	.OidManufacturer = ".1.3.6.1.4.1.2.3.51.2.2.21.9.3.0",
                        	.OidProductName = '\0',
                        	.OidProductVersion = ".1.3.6.1.4.1.2.3.51.2.2.21.9.5.0",
                        	.OidModelNumber = '\0',
                        	.OidSerialNumber = '\0',
                        	.OidPartNumber = ".1.3.6.1.4.1.2.3.51.2.2.21.9.4.0",
                        	.OidFileId = '\0',
                       	 	.OidAssetTag = '\0',
                        	/* UUID .1.3.6.1.4.1.2.3.51.2.2.21.9.8.0 */
			}
                },
                .comment = "Media Tray VPD",
        },

        {} /* Terminate array with a null element */
};

/*******************
 * Power Module VPD
 *******************/

struct snmp_bc_inventory snmp_bc_power_inventories[] = {
        {
                .inventory = {
			.EirId = 9,
                        .Oem = 0,
                },
                .mib = {	
                        .not_avail_indicator_num = 0,
                        .write_only = 0,
                	.inventory_type = SAHPI_INVENT_RECTYPE_BOARD_INFO,
                	.chassis_type = SAHPI_INVENT_CTYP_RACKMOUNT,
			.oid = {
                        	.OidMfgDateTime = '\0',   /* Set to SAHPI_TIME_UNSPECIFIED */
                        	.OidManufacturer = ".1.3.6.1.4.1.2.3.51.2.2.21.8.1.1.3.x",
                        	.OidProductName = '\0',
                        	.OidProductVersion = ".1.3.6.1.4.1.2.3.51.2.2.21.8.1.1.5.x",
                        	.OidModelNumber = '\0',
                        	.OidSerialNumber = '\0',
                        	.OidPartNumber = ".1.3.6.1.4.1.2.3.51.2.2.21.8.1.1.4.x",
                        	.OidFileId = '\0',
                        	.OidAssetTag = '\0',
                        	/* UUID .1.3.6.1.4.1.2.3.51.2.2.21.8.1.1.8.x  */
			}
                },
                .comment = "Power Module VPD",
        },

        {} /* Terminate array with a null element */
};
