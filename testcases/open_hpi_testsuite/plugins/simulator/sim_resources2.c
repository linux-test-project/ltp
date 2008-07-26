/*      -*- linux-c -*-
 *
 * (C) Copyright IBM Corp. 2005
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  This
 * file and program are licensed under a BSD style license.  See
 * the Copying file included with the OpenHPI distribution for
 * full licensing terms.
 *
 * Author(s):
 *        W. David Ashley <dashley@us.ibm.com>
 */

#include <sim_init.h>


/**************************************************************************
 *                        Resource Definitions
 *
 * These are patterned after an RSA type machine
 **************************************************************************/

/*-------------------------------------------------------------------------
  NOTE!!!!!!!!!
  The order is important here! Changing the order of these resources or
  adding additional resources means the code in the function sim_discovery()
  also needs to change!
  ------------------------------------------------------------------------*/

struct sim_rpt sim_rpt_array[] = {
	/* Chassis */
        {
                .rpt = {
			.ResourceInfo = {
                                .ManufacturerId = IBM_MANUFACTURING_ID,
                        },
                        .ResourceEntity = {
                                .Entry[0] =
                                {
                                        .EntityType = SAHPI_ENT_ROOT,
                                        .EntityLocation = 0
                                }
                        },

                        .ResourceCapabilities = SAHPI_CAPABILITY_EVENT_LOG |
			                        SAHPI_CAPABILITY_EVT_DEASSERTS |
			                        SAHPI_CAPABILITY_INVENTORY_DATA |
                                                SAHPI_CAPABILITY_RDR |
			                        SAHPI_CAPABILITY_RESOURCE |
			                        SAHPI_CAPABILITY_SENSOR |
                                                SAHPI_CAPABILITY_ANNUNCIATOR |
                                                SAHPI_CAPABILITY_POWER |
                                                SAHPI_CAPABILITY_RESET |
                                                SAHPI_CAPABILITY_WATCHDOG |
                                                SAHPI_CAPABILITY_CONTROL |
                                                SAHPI_CAPABILITY_DIMI |
                                                SAHPI_CAPABILITY_FUMI,
                        .ResourceSeverity = SAHPI_CRITICAL,
			.ResourceFailed = SAHPI_FALSE,
                },
                .comment = "Chassis"
        },
        /* CPUs */
        {
                .rpt = {
                        .ResourceInfo = {
                                .ManufacturerId = IBM_MANUFACTURING_ID,
                        },
                        .ResourceEntity = {
                                .Entry[0] =
				{
                                        .EntityType = SAHPI_ENT_PROCESSOR,
                                        .EntityLocation = SIM_HPI_LOCATION_BASE
                                },
                                {
                                        .EntityType = SAHPI_ENT_ROOT,
                                        .EntityLocation = 0
                                }
			},
                        .ResourceCapabilities = SAHPI_CAPABILITY_EVT_DEASSERTS |
			                        SAHPI_CAPABILITY_RDR |
			                        SAHPI_CAPABILITY_RESOURCE |
                                                SAHPI_CAPABILITY_SENSOR,
                        .ResourceSeverity = SAHPI_MAJOR,
			.ResourceFailed = SAHPI_FALSE,
                 },
                .comment = "CPU"
        },
        /* DASD */
        {
                .rpt = {
                        .ResourceInfo = {
                                .ManufacturerId = IBM_MANUFACTURING_ID,
                        },
                        .ResourceEntity = {
                                .Entry[0] =
				{
                                        .EntityType = SAHPI_ENT_DISK_DRIVE,
                                        .EntityLocation = SIM_HPI_LOCATION_BASE
                                },
                                {
                                        .EntityType = SAHPI_ENT_ROOT,
                                        .EntityLocation = 0
                                }
			},
                        .ResourceCapabilities = SAHPI_CAPABILITY_EVT_DEASSERTS |
			                        SAHPI_CAPABILITY_RDR |
    			                        SAHPI_CAPABILITY_RESOURCE |
                                                SAHPI_CAPABILITY_SENSOR,
                        .ResourceSeverity = SAHPI_MAJOR,
 			.ResourceFailed = SAHPI_FALSE,
                },
                .comment = "DASD 1"
        },
        /* HS DASD */
        {
                .rpt = {
                        .ResourceInfo = {
                                .ManufacturerId = IBM_MANUFACTURING_ID,
                        },
                        .ResourceEntity = {
                                .Entry[0] =
				{
                                        .EntityType = SAHPI_ENT_DISK_DRIVE,
                                        .EntityLocation = SIM_HPI_LOCATION_BASE + 1

                                },
                                {
                                        .EntityType = SAHPI_ENT_ROOT,
                                        .EntityLocation = 0
                                }
			},
                        .ResourceCapabilities = SAHPI_CAPABILITY_EVT_DEASSERTS |
			                        SAHPI_CAPABILITY_RDR |
			                        SAHPI_CAPABILITY_RESOURCE |
                                                SAHPI_CAPABILITY_FRU |
                                                SAHPI_CAPABILITY_MANAGED_HOTSWAP |
                                                SAHPI_HS_CAPABILITY_AUTOEXTRACT_READ_ONLY |
                                                SAHPI_CAPABILITY_WATCHDOG |
                                                SAHPI_CAPABILITY_CONTROL |
                                                SAHPI_CAPABILITY_ANNUNCIATOR |
                                                SAHPI_CAPABILITY_POWER |
                                                SAHPI_CAPABILITY_RESET |
                                                SAHPI_CAPABILITY_INVENTORY_DATA |
                                                SAHPI_CAPABILITY_EVENT_LOG |
                                                SAHPI_CAPABILITY_SENSOR,
                        .ResourceSeverity = SAHPI_MAJOR,
 			.ResourceFailed = SAHPI_FALSE,
                },
                .comment = "HS DASD 1"
        },
        /* Fans */
        {
                .rpt = {
                        .ResourceInfo = {
                                .ManufacturerId = IBM_MANUFACTURING_ID,
                        },
                        .ResourceEntity = {
                                .Entry[0] =
                                {
				        .EntityType = SAHPI_ENT_FAN,
			                .EntityLocation = SIM_HPI_LOCATION_BASE
				},
                                {
                                        .EntityType = SAHPI_ENT_ROOT,
                                        .EntityLocation = 0,
                                }
			},
                        .ResourceCapabilities = SAHPI_CAPABILITY_EVT_DEASSERTS |
			                        SAHPI_CAPABILITY_RESOURCE |
			                        SAHPI_CAPABILITY_RDR |
			                        SAHPI_CAPABILITY_SENSOR |
						SAHPI_CAPABILITY_CONTROL,
                        .ResourceSeverity = SAHPI_MAJOR,
 			.ResourceFailed = SAHPI_FALSE,
                },
                .comment = "Fan"
        },

        {} /* Terminate array with a null element */
};


/******************************************************************************
 *                      Sensor Definitions
 *
 * These are patterned after an RSA type machine
 ******************************************************************************/

/*****************
 * Chassis Sensors
 *****************/

struct sim_sensor sim_chassis_sensors[] = {
        /* Thermal sensor on planar */
        {
		.index = 1,
                .sensor = {
                        .Num = 1,
                        .Type = SAHPI_TEMPERATURE,
                        .Category = SAHPI_EC_THRESHOLD,
			.EnableCtrl = SAHPI_FALSE,
                        .EventCtrl = SAHPI_SEC_READ_ONLY,
                        .Events = SAHPI_ES_UPPER_MINOR | SAHPI_ES_UPPER_MAJOR | SAHPI_ES_UPPER_CRIT,
                        .DataFormat = {
                                .IsSupported = SAHPI_TRUE,
                                .ReadingType = SAHPI_SENSOR_READING_TYPE_FLOAT64,
                                .BaseUnits = SAHPI_SU_DEGREES_F,
                                .ModifierUnits = SAHPI_SU_UNSPECIFIED,
                                .ModifierUse = SAHPI_SMUU_NONE,
                                .Percentage = SAHPI_FALSE,
                                .Range = {
                                        .Flags = SAHPI_SRF_MAX | SAHPI_SRF_MIN |
                                                 SAHPI_SRF_NORMAL_MAX | SAHPI_SRF_NORMAL_MIN |
                                                 SAHPI_SRF_NOMINAL,
                                        .Max = {
						.IsSupported = SAHPI_TRUE,
                                                .Type = SAHPI_SENSOR_READING_TYPE_FLOAT64,
						.Value = {
							.SensorFloat64 = 125,
						},
					},
                                        .Min = {
						.IsSupported = SAHPI_TRUE,
                                                .Type = SAHPI_SENSOR_READING_TYPE_FLOAT64,
						.Value = {
							.SensorFloat64 = 40,
						},
                                        },
                                        .NormalMax = {
						.IsSupported = SAHPI_TRUE,
                                                .Type = SAHPI_SENSOR_READING_TYPE_FLOAT64,
						.Value = {
							.SensorFloat64 = 110,
						},
					},
                                        .Nominal = {
						.IsSupported = SAHPI_TRUE,
                                                .Type = SAHPI_SENSOR_READING_TYPE_FLOAT64,
						.Value = {
							.SensorFloat64 = 100,
						},
					},
                                        .NormalMin = {
						.IsSupported = SAHPI_TRUE,
                                                .Type = SAHPI_SENSOR_READING_TYPE_FLOAT64,
						.Value = {
							.SensorFloat64 = 90,
						},
                                        },
                                },
                        },
                        .ThresholdDefn = {
				.IsAccessible = SAHPI_TRUE,
                                .ReadThold = SAHPI_STM_UP_MAJOR | SAHPI_STM_UP_CRIT |
                                             SAHPI_STM_LOW_MAJOR | SAHPI_STM_LOW_CRIT |
				             SAHPI_STM_UP_CRIT | SAHPI_STM_UP_HYSTERESIS,
                                .WriteThold = SAHPI_STM_UP_MAJOR | SAHPI_STM_UP_CRIT |
                                             SAHPI_STM_LOW_MAJOR | SAHPI_STM_LOW_CRIT |
				             SAHPI_STM_UP_CRIT | SAHPI_STM_UP_HYSTERESIS,
				.Nonlinear = SAHPI_FALSE,
                        },
                        .Oem = 0,
                },
		.sensor_info = {
			.cur_state = SAHPI_ES_UPPER_MINOR,
                        .sensor_enabled = SAHPI_TRUE,
                        .events_enabled = SAHPI_TRUE,
                        .assert_mask   = SAHPI_ES_UPPER_MINOR | SAHPI_ES_UPPER_MAJOR | SAHPI_ES_UPPER_CRIT,
                        .deassert_mask = SAHPI_ES_UPPER_MINOR | SAHPI_ES_UPPER_MAJOR | SAHPI_ES_UPPER_CRIT,
			.event_array = {
				{
                                        .event = "0501C480", /* EN_CUTOFF_HI_OVER_TEMP_PLANAR */
					.event_assertion = SAHPI_TRUE,
					.event_res_failure = SAHPI_TRUE,
					.event_res_failure_unexpected = SAHPI_TRUE,
                                        .event_state = SAHPI_ES_UPPER_CRIT | SAHPI_ES_UPPER_MAJOR | SAHPI_ES_UPPER_MINOR,
                                        .recovery_state = SAHPI_ES_UPPER_MAJOR | SAHPI_ES_UPPER_MINOR,
                                },
				{
                                        .event = "0501C080", /* EN_OVER_TEMP_PLANAR */
					.event_assertion = SAHPI_TRUE,
					.event_res_failure = SAHPI_TRUE,
					.event_res_failure_unexpected = SAHPI_TRUE,
                                        .event_state = SAHPI_ES_UPPER_CRIT | SAHPI_ES_UPPER_MAJOR | SAHPI_ES_UPPER_MINOR,
                                        .recovery_state = SAHPI_ES_UPPER_MAJOR | SAHPI_ES_UPPER_MINOR,
                                },
                                {
                                        .event = "0501D500", /* EN_PFA_HI_OVER_TEMP_PLANAR */
 					.event_assertion = SAHPI_TRUE,
					.event_res_failure = SAHPI_FALSE,
					.event_res_failure_unexpected = SAHPI_FALSE,
					.event_state = SAHPI_ES_UPPER_MAJOR | SAHPI_ES_UPPER_MINOR,
                                        .recovery_state = SAHPI_ES_UNSPECIFIED,
                                },
				{},
			},
            .reading = {
                                .IsSupported = SAHPI_TRUE,
                                .Type = SAHPI_SENSOR_READING_TYPE_FLOAT64,
                                .Value = {
				                .SensorFloat64 = 35,
				         },
                       },
            .thres = {
                        .LowCritical = {
                                                .IsSupported = SAHPI_TRUE,
                                                .Type = SAHPI_SENSOR_READING_TYPE_FLOAT64,
                                                .Value = {
                                                          .SensorFloat64 = 40,
                                                         },
                                       },
                        .LowMajor =    {
                                                .IsSupported = SAHPI_TRUE,
                                                .Type = SAHPI_SENSOR_READING_TYPE_FLOAT64,
                                                .Value = {
                                                          .SensorFloat64 = 50,
                                                         },
                                       },
                        .LowMinor =    {
                                                .IsSupported = SAHPI_TRUE,
                                                .Type = SAHPI_SENSOR_READING_TYPE_FLOAT64,
                                                .Value = {
                                                          .SensorFloat64 = 60,
                                                          },
                                       },
                        .UpCritical =  {
                                                .IsSupported = SAHPI_TRUE,
                                                .Type = SAHPI_SENSOR_READING_TYPE_FLOAT64,
                                                .Value = {
		      			                  .SensorFloat64 = 125,
		      		                         },
                                       },
                        .UpMajor =     {
                                                .IsSupported = SAHPI_TRUE,
                                                .Type = SAHPI_SENSOR_READING_TYPE_FLOAT64,
                                                .Value = {
      	      			                          .SensorFloat64 = 120,
		      		                         },
                                       },
                        .UpMinor =     {
                                                .IsSupported = SAHPI_TRUE,
                                                .Type = SAHPI_SENSOR_READING_TYPE_FLOAT64,
                                                .Value = {
                                                          .SensorFloat64 = 110,
                                                         },
                                       },
                        .PosThdHysteresis = {
                                                .IsSupported = SAHPI_TRUE,
                                                .Type = SAHPI_SENSOR_READING_TYPE_FLOAT64,
                                                .Value = {
       	      			                          .SensorFloat64 = 2,
		      	 	                         },
                                            },
                        .NegThdHysteresis = {
                                                .IsSupported = SAHPI_TRUE,
                                                .Type = SAHPI_SENSOR_READING_TYPE_FLOAT64,
                                                .Value = {
                                                          .SensorFloat64 = 2,
                                                         },
                                            },
                     },
	    },
            .comment = "Planar temperature sensor"
        },
        /* CPU area thermal sensor on planar */
        {
		.index = 2,
                .sensor = {
                        .Num = 2,
                        .Type = SAHPI_TEMPERATURE,
                        .Category = SAHPI_EC_THRESHOLD,
			.EnableCtrl = SAHPI_FALSE,
                        .EventCtrl = SAHPI_SEC_READ_ONLY,
                        .Events = SAHPI_ES_UPPER_MINOR | SAHPI_ES_UPPER_MAJOR | SAHPI_ES_UPPER_CRIT,
                        .DataFormat = {
                                .IsSupported = SAHPI_TRUE,
                                .ReadingType = SAHPI_SENSOR_READING_TYPE_FLOAT64,
                                .BaseUnits = SAHPI_SU_DEGREES_F,
                                .ModifierUnits = SAHPI_SU_UNSPECIFIED,
                                .ModifierUse = SAHPI_SMUU_NONE,
                                .Percentage = SAHPI_FALSE,
                                .Range = {
                                        .Flags = SAHPI_SRF_MAX | SAHPI_SRF_MIN |
                                                 SAHPI_SRF_NORMAL_MAX | SAHPI_SRF_NORMAL_MIN |
                                                 SAHPI_SRF_NOMINAL,
                                        .Max = {
						.IsSupported = SAHPI_TRUE,
                                                .Type = SAHPI_SENSOR_READING_TYPE_FLOAT64,
						.Value = {
							.SensorFloat64 = 125,
						},
					},
                                        .Min = {
						.IsSupported = SAHPI_TRUE,
                                                .Type = SAHPI_SENSOR_READING_TYPE_FLOAT64,
						.Value = {
							.SensorFloat64 = 40,
						},
                                        },
                                        .NormalMax = {
						.IsSupported = SAHPI_TRUE,
                                                .Type = SAHPI_SENSOR_READING_TYPE_FLOAT64,
						.Value = {
							.SensorFloat64 = 110,
						},
					},
                                        .Nominal = {
						.IsSupported = SAHPI_TRUE,
                                                .Type = SAHPI_SENSOR_READING_TYPE_FLOAT64,
						.Value = {
							.SensorFloat64 = 100,
						},
					},
                                        .NormalMin = {
						.IsSupported = SAHPI_TRUE,
                                                .Type = SAHPI_SENSOR_READING_TYPE_FLOAT64,
						.Value = {
							.SensorFloat64 = 90,
						},
                                        },
                                },
                        },
                        .ThresholdDefn = {
				.IsAccessible = SAHPI_FALSE,
                        },
                        .Oem = 0,
                },
		.sensor_info = {
			.cur_state = SAHPI_ES_UPPER_MINOR,
                        .sensor_enabled = SAHPI_TRUE,
                        .events_enabled = SAHPI_TRUE,
                        .assert_mask   = SAHPI_ES_UPPER_MINOR | SAHPI_ES_UPPER_MAJOR | SAHPI_ES_UPPER_CRIT,
                        .deassert_mask = SAHPI_ES_UPPER_MINOR | SAHPI_ES_UPPER_MAJOR | SAHPI_ES_UPPER_CRIT,
			.event_array = {
				{
                                        .event = "0601C480", /* EN_CUTOFF_HI_OVER_TEMP_PLANAR */
					.event_assertion = SAHPI_TRUE,
					.event_res_failure = SAHPI_TRUE,
					.event_res_failure_unexpected = SAHPI_TRUE,
                                        .event_state = SAHPI_ES_UPPER_CRIT | SAHPI_ES_UPPER_MAJOR | SAHPI_ES_UPPER_MINOR,
                                        .recovery_state = SAHPI_ES_UPPER_MAJOR | SAHPI_ES_UPPER_MINOR,
                                },
				{
                                        .event = "0601C080", /* EN_OVER_TEMP_PLANAR */
					.event_assertion = SAHPI_TRUE,
					.event_res_failure = SAHPI_TRUE,
					.event_res_failure_unexpected = SAHPI_TRUE,
                                        .event_state = SAHPI_ES_UPPER_CRIT | SAHPI_ES_UPPER_MAJOR | SAHPI_ES_UPPER_MINOR,
                                        .recovery_state = SAHPI_ES_UPPER_MAJOR | SAHPI_ES_UPPER_MINOR,
                                },
                                {
                                        .event = "0601D500", /* EN_PFA_HI_OVER_TEMP_PLANAR */
 					.event_assertion = SAHPI_TRUE,
					.event_res_failure = SAHPI_FALSE,
					.event_res_failure_unexpected = SAHPI_FALSE,
					.event_state = SAHPI_ES_UPPER_MAJOR | SAHPI_ES_UPPER_MINOR,
                                        .recovery_state = SAHPI_ES_UNSPECIFIED,
                                },
				{},
			},
            .reading = {
                                .IsSupported = SAHPI_TRUE,
                                .Type = SAHPI_SENSOR_READING_TYPE_FLOAT64,
                                .Value = {
				                .SensorFloat64 = 35,
				         },
                       },
            .thres = {
                        .LowCritical = {
                                                .IsSupported = SAHPI_FALSE,
                                       },
                        .LowMajor =    {
                                                .IsSupported = SAHPI_FALSE,
                                       },
                        .LowMinor =    {
                                                .IsSupported = SAHPI_FALSE,
                                       },
                        .UpCritical =  {
                                                .IsSupported = SAHPI_FALSE,
                                       },
                        .UpMajor =     {
                                                .IsSupported = SAHPI_FALSE,
                                       },
                        .UpMinor =     {
                                                .IsSupported = SAHPI_FALSE,
                                       },
                        .PosThdHysteresis = {
                                                .IsSupported = SAHPI_FALSE,
                                            },
                        .NegThdHysteresis = {
                                                .IsSupported = SAHPI_FALSE,
                                            },
                     },
	    },
            .comment = "Planar CPU area temperature sensor"
        },

        {} /* Terminate array with a null element */
};

/*************
 * CPU Sensors
 *************/

struct sim_sensor sim_cpu_sensors[] = {
        /* CPU thermal sensor */
        {
		.index = 1,
                .sensor = {
                        .Num = 1,
                        .Type = SAHPI_TEMPERATURE,
                        .Category = SAHPI_EC_THRESHOLD,
  			.EnableCtrl = SAHPI_FALSE,
                        .EventCtrl = SAHPI_SEC_READ_ONLY,
                        .Events = SAHPI_ES_UPPER_MINOR | SAHPI_ES_UPPER_MAJOR | SAHPI_ES_UPPER_CRIT,
                        .DataFormat = {
				.IsSupported = SAHPI_TRUE,
                                .ReadingType = SAHPI_SENSOR_READING_TYPE_FLOAT64,
                                .BaseUnits = SAHPI_SU_DEGREES_C,
                                .ModifierUnits = SAHPI_SU_UNSPECIFIED,
                                .ModifierUse = SAHPI_SMUU_NONE,
                                .Percentage = SAHPI_FALSE,
                                .Range = {
                                        .Flags = SAHPI_SRF_MAX | SAHPI_SRF_MIN,
                                        .Max = {
						.IsSupported = SAHPI_TRUE,
                                                .Type = SAHPI_SENSOR_READING_TYPE_FLOAT64,
						.Value = {
							.SensorFloat64 = 125,
						},
					},
                                        .Min = {
						.IsSupported = SAHPI_TRUE,
                                                .Type = SAHPI_SENSOR_READING_TYPE_FLOAT64,
						.Value = {
							.SensorFloat64 = 0,
						},
                                        },
                                },
			},
                        .ThresholdDefn = {
				.IsAccessible = SAHPI_TRUE,
                                .ReadThold = SAHPI_STM_UP_MAJOR | SAHPI_STM_UP_CRIT |
				             SAHPI_STM_UP_HYSTERESIS,
				.WriteThold = 0,
                        },
                        .Oem = 0,
                },
		.sensor_info = {
			.cur_state = SAHPI_ES_UPPER_MINOR,
                        .sensor_enabled = SAHPI_TRUE,
                        .events_enabled = SAHPI_TRUE,
                        .assert_mask   = SAHPI_ES_UPPER_MINOR | SAHPI_ES_UPPER_MAJOR | SAHPI_ES_UPPER_CRIT,
                        .deassert_mask = SAHPI_ES_UPPER_MINOR | SAHPI_ES_UPPER_MAJOR | SAHPI_ES_UPPER_CRIT,
			.event_array = {
				{
					.event = "0421C40x", /* EN_PROC_HOT_CPUx */
					.event_assertion = SAHPI_TRUE,
					.event_res_failure = SAHPI_TRUE,
					.event_res_failure_unexpected = SAHPI_TRUE,
					.event_state = SAHPI_ES_UPPER_CRIT | SAHPI_ES_UPPER_MAJOR |
					               SAHPI_ES_UPPER_MINOR,
					.recovery_state = SAHPI_ES_UPPER_MAJOR | SAHPI_ES_UPPER_MINOR,
				},
				{
					.event = "0421C48x", /* EN_CUTOFF_HI_OVER_TEMP_CPUx */
					.event_assertion = SAHPI_TRUE,
					.event_res_failure = SAHPI_TRUE,
					.event_res_failure_unexpected = SAHPI_TRUE,
					.event_state = SAHPI_ES_UPPER_CRIT | SAHPI_ES_UPPER_MAJOR |
					               SAHPI_ES_UPPER_MINOR,
					.recovery_state = SAHPI_ES_UPPER_MAJOR | SAHPI_ES_UPPER_MINOR,
				},
				{
					.event = "0421D08x", /* EN_THERM_TRIP_CPUx */
					.event_assertion = SAHPI_TRUE,
					.event_res_failure = SAHPI_TRUE,
					.event_res_failure_unexpected = SAHPI_TRUE,
					.event_state = SAHPI_ES_UPPER_CRIT | SAHPI_ES_UPPER_MAJOR |
					               SAHPI_ES_UPPER_MINOR,
					.recovery_state = SAHPI_ES_UPPER_MAJOR | SAHPI_ES_UPPER_MINOR,
				},
				{
                                        .event = "0421D50x", /* EN_PFA_HI_OVER_TEMP_CPUx */
 					.event_assertion = SAHPI_TRUE,
					.event_res_failure = SAHPI_FALSE,
					.event_res_failure_unexpected = SAHPI_FALSE,
					.event_state = SAHPI_ES_UPPER_MAJOR | SAHPI_ES_UPPER_MINOR,
					.recovery_state = SAHPI_ES_UNSPECIFIED,
				},
				{},
			},
            .reading = { .IsSupported = SAHPI_TRUE,
                                              .Type = SAHPI_SENSOR_READING_TYPE_FLOAT64,
                                              .Value = {
							                        .SensorFloat64 = 50,
						                      },
                                            },
            .thres = {
                        .LowCritical = { .IsSupported = SAHPI_FALSE, },
                        .LowMajor =    { .IsSupported = SAHPI_FALSE, },
                        .LowMinor =    { .IsSupported = SAHPI_FALSE, },
                        .UpCritical =  { .IsSupported = SAHPI_TRUE,
                                              .Type = SAHPI_SENSOR_READING_TYPE_FLOAT64,
                                              .Value = {
							                        .SensorFloat64 = 80,
						                      },
                                            },
                        .UpMajor =     { .IsSupported =  SAHPI_TRUE,
                                              .Type = SAHPI_SENSOR_READING_TYPE_FLOAT64,
                                              .Value = {
							                        .SensorFloat64 = 60,
						                      },
                                            },
                        .UpMinor =     { .IsSupported = SAHPI_FALSE, },
                        .PosThdHysteresis = { .IsSupported = SAHPI_TRUE,
                                              .Type = SAHPI_SENSOR_READING_TYPE_FLOAT64,
                                              .Value = {
							                        .SensorFloat64 = 2,
						                      },
                                            },
                        .NegThdHysteresis = { .IsSupported = SAHPI_FALSE, },
             },
		},
                .comment = "CPU temperature sensor"
        },

        {} /* Terminate array with a null element */
};

/**************
 * DASD Sensors
 **************/

struct sim_sensor sim_dasd_sensors[] = {
        /* DASD thermal sensor */
        {
		.index = 1,
                .sensor = {
                        .Num = 1,
                        .Type = SAHPI_TEMPERATURE,
                        .Category = SAHPI_EC_THRESHOLD,
  			.EnableCtrl = SAHPI_FALSE,
                        .EventCtrl = SAHPI_SEC_READ_ONLY,
                        .Events = SAHPI_ES_UPPER_MINOR | SAHPI_ES_UPPER_MAJOR | SAHPI_ES_UPPER_CRIT,
                        .DataFormat = {
				.IsSupported = SAHPI_TRUE,
                                .ReadingType = SAHPI_SENSOR_READING_TYPE_FLOAT64,
                                .BaseUnits = SAHPI_SU_DEGREES_C,
                                .ModifierUnits = SAHPI_SU_UNSPECIFIED,
                                .ModifierUse = SAHPI_SMUU_NONE,
                                .Percentage = SAHPI_FALSE,
                                .Range = {
                                        .Flags = SAHPI_SRF_MAX | SAHPI_SRF_MIN,
                                        .Max = {
						.IsSupported = SAHPI_TRUE,
                                                .Type = SAHPI_SENSOR_READING_TYPE_FLOAT64,
						.Value = {
							.SensorFloat64 = 125,
						},
					},
                                        .Min = {
						.IsSupported = SAHPI_TRUE,
                                                .Type = SAHPI_SENSOR_READING_TYPE_FLOAT64,
						.Value = {
							.SensorFloat64 = 0,
						},
                                        },
                                },
                        },
                        .ThresholdDefn = {
				.IsAccessible = SAHPI_TRUE,
                                .ReadThold = SAHPI_STM_UP_MAJOR | SAHPI_STM_UP_CRIT |
				             SAHPI_STM_UP_HYSTERESIS,
				.WriteThold = 0,
                        },
                        .Oem = 0,
                },
		.sensor_info = {
			.cur_state = SAHPI_ES_UPPER_MINOR,
                        .sensor_enabled = SAHPI_TRUE,
                        .events_enabled = SAHPI_TRUE,
                        .assert_mask   = SAHPI_ES_UPPER_MINOR | SAHPI_ES_UPPER_MAJOR | SAHPI_ES_UPPER_CRIT,
                        .deassert_mask = SAHPI_ES_UPPER_MINOR | SAHPI_ES_UPPER_MAJOR | SAHPI_ES_UPPER_CRIT,
			.event_array = {
				{
					.event = "0681C08x", /* EN_CUTOFF_HI_OVER_TEMP_DASD1 */
					.event_assertion = SAHPI_TRUE,
					.event_res_failure = SAHPI_TRUE,
					.event_res_failure_unexpected = SAHPI_TRUE,
					.event_state = SAHPI_ES_UPPER_CRIT | SAHPI_ES_UPPER_MAJOR |
					               SAHPI_ES_UPPER_MINOR,
					.recovery_state = SAHPI_ES_UPPER_MAJOR | SAHPI_ES_UPPER_MINOR,
				},
				{
                                        .event = "0681C40x", /* EN_PFA_HI_OVER_TEMP_DASD1 */
 					.event_assertion = SAHPI_TRUE,
					.event_res_failure = SAHPI_FALSE,
					.event_res_failure_unexpected = SAHPI_FALSE,
					.event_state = SAHPI_ES_UPPER_MAJOR | SAHPI_ES_UPPER_MINOR,
					.recovery_state = SAHPI_ES_UNSPECIFIED,
				},
				{},
			},
            .reading = { .IsSupported = SAHPI_TRUE,
                                              .Type = SAHPI_SENSOR_READING_TYPE_FLOAT64,
                                              .Value = {
							                        .SensorFloat64 = 50,
						                      },
                                            },
            .thres = {
                        .LowCritical = { .IsSupported = SAHPI_FALSE, },
                        .LowMajor =    { .IsSupported = SAHPI_FALSE, },
                        .LowMinor =    { .IsSupported = SAHPI_FALSE, },
                        .UpCritical =  { .IsSupported = SAHPI_TRUE,
                                              .Type = SAHPI_SENSOR_READING_TYPE_FLOAT64,
                                              .Value = {
							                        .SensorFloat64 = 80,
						                      },
                                            },
                        .UpMajor =     { .IsSupported =  SAHPI_TRUE,
                                              .Type = SAHPI_SENSOR_READING_TYPE_FLOAT64,
                                              .Value = {
							                        .SensorFloat64 = 60,
						                      },
                                            },
                        .UpMinor =     { .IsSupported = SAHPI_FALSE, },
                        .PosThdHysteresis = { .IsSupported = SAHPI_TRUE,
                                              .Type = SAHPI_SENSOR_READING_TYPE_FLOAT64,
                                              .Value = {
							                        .SensorFloat64 = 2,
						                      },
                                            },
                        .NegThdHysteresis = { .IsSupported = SAHPI_FALSE, },
             },
		},
                .comment = "DASD temperature sensor"
        },

        {} /* Terminate array with a null element */
};

/***********************
 * Hot Swap DASD Sensors
 **********************/

struct sim_sensor sim_hs_dasd_sensors[] = {
        /* Hot Swap DASD thermal sensor 1 */
        {
		.index = 1,
                .sensor = {
                        .Num = 1,
                        .Type = SAHPI_TEMPERATURE,
                        .Category = SAHPI_EC_THRESHOLD,
			.EnableCtrl = SAHPI_FALSE,
                        .EventCtrl = SAHPI_SEC_READ_ONLY,
                        .Events = SAHPI_ES_UPPER_MINOR | SAHPI_ES_UPPER_MAJOR | SAHPI_ES_UPPER_CRIT,
                        .DataFormat = {
                                .IsSupported = SAHPI_TRUE,
                                .ReadingType = SAHPI_SENSOR_READING_TYPE_FLOAT64,
                                .BaseUnits = SAHPI_SU_DEGREES_F,
                                .ModifierUnits = SAHPI_SU_UNSPECIFIED,
                                .ModifierUse = SAHPI_SMUU_NONE,
                                .Percentage = SAHPI_FALSE,
                                .Range = {
                                        .Flags = SAHPI_SRF_MAX | SAHPI_SRF_MIN |
                                                 SAHPI_SRF_NORMAL_MAX | SAHPI_SRF_NORMAL_MIN |
                                                 SAHPI_SRF_NOMINAL,
                                        .Max = {
						.IsSupported = SAHPI_TRUE,
                                                .Type = SAHPI_SENSOR_READING_TYPE_FLOAT64,
						.Value = {
							.SensorFloat64 = 125,
						},
					},
                                        .Min = {
						.IsSupported = SAHPI_TRUE,
                                                .Type = SAHPI_SENSOR_READING_TYPE_FLOAT64,
						.Value = {
							.SensorFloat64 = 40,
						},
                                        },
                                        .NormalMax = {
						.IsSupported = SAHPI_TRUE,
                                                .Type = SAHPI_SENSOR_READING_TYPE_FLOAT64,
						.Value = {
							.SensorFloat64 = 110,
						},
					},
                                        .Nominal = {
						.IsSupported = SAHPI_TRUE,
                                                .Type = SAHPI_SENSOR_READING_TYPE_FLOAT64,
						.Value = {
							.SensorFloat64 = 100,
						},
					},
                                        .NormalMin = {
						.IsSupported = SAHPI_TRUE,
                                                .Type = SAHPI_SENSOR_READING_TYPE_FLOAT64,
						.Value = {
							.SensorFloat64 = 90,
						},
                                        },
                                },
                        },
                        .ThresholdDefn = {
				.IsAccessible = SAHPI_TRUE,
                                .ReadThold = SAHPI_STM_UP_MAJOR | SAHPI_STM_UP_CRIT |
                                             SAHPI_STM_LOW_MAJOR | SAHPI_STM_LOW_CRIT |
				             SAHPI_STM_UP_CRIT | SAHPI_STM_UP_HYSTERESIS,
                                .WriteThold = SAHPI_STM_UP_MAJOR | SAHPI_STM_UP_CRIT |
                                             SAHPI_STM_LOW_MAJOR | SAHPI_STM_LOW_CRIT |
				             SAHPI_STM_UP_CRIT | SAHPI_STM_UP_HYSTERESIS,
				.Nonlinear = SAHPI_FALSE,
                        },
                        .Oem = 0,
                },
		.sensor_info = {
			.cur_state = SAHPI_ES_UPPER_MINOR,
                        .sensor_enabled = SAHPI_TRUE,
                        .events_enabled = SAHPI_TRUE,
                        .assert_mask   = SAHPI_ES_UPPER_MINOR | SAHPI_ES_UPPER_MAJOR | SAHPI_ES_UPPER_CRIT,
                        .deassert_mask = SAHPI_ES_UPPER_MINOR | SAHPI_ES_UPPER_MAJOR | SAHPI_ES_UPPER_CRIT,
			.event_array = {
				{
                                        .event = "0511C480", /* EN_CUTOFF_HI_OVER_TEMP_PLANAR */
					.event_assertion = SAHPI_TRUE,
					.event_res_failure = SAHPI_TRUE,
					.event_res_failure_unexpected = SAHPI_TRUE,
                                        .event_state = SAHPI_ES_UPPER_CRIT | SAHPI_ES_UPPER_MAJOR | SAHPI_ES_UPPER_MINOR,
                                        .recovery_state = SAHPI_ES_UPPER_MAJOR | SAHPI_ES_UPPER_MINOR,
                                },
				{
                                        .event = "0511C080", /* EN_OVER_TEMP_PLANAR */
					.event_assertion = SAHPI_TRUE,
					.event_res_failure = SAHPI_TRUE,
					.event_res_failure_unexpected = SAHPI_TRUE,
                                        .event_state = SAHPI_ES_UPPER_CRIT | SAHPI_ES_UPPER_MAJOR | SAHPI_ES_UPPER_MINOR,
                                        .recovery_state = SAHPI_ES_UPPER_MAJOR | SAHPI_ES_UPPER_MINOR,
                                },
                                {
                                        .event = "0511D500", /* EN_PFA_HI_OVER_TEMP_PLANAR */
 					.event_assertion = SAHPI_TRUE,
					.event_res_failure = SAHPI_FALSE,
					.event_res_failure_unexpected = SAHPI_FALSE,
					.event_state = SAHPI_ES_UPPER_MAJOR | SAHPI_ES_UPPER_MINOR,
                                        .recovery_state = SAHPI_ES_UNSPECIFIED,
                                },
				{},
			},
            .reading = {
                                .IsSupported = SAHPI_TRUE,
                                .Type = SAHPI_SENSOR_READING_TYPE_FLOAT64,
                                .Value = {
				                .SensorFloat64 = 35,
				         },
                       },
            .thres = {
                        .LowCritical = {
                                                .IsSupported = SAHPI_TRUE,
                                                .Type = SAHPI_SENSOR_READING_TYPE_FLOAT64,
                                                .Value = {
                                                          .SensorFloat64 = 40,
                                                         },
                                       },
                        .LowMajor =    {
                                                .IsSupported = SAHPI_TRUE,
                                                .Type = SAHPI_SENSOR_READING_TYPE_FLOAT64,
                                                .Value = {
                                                          .SensorFloat64 = 50,
                                                         },
                                       },
                        .LowMinor =    {
                                                .IsSupported = SAHPI_TRUE,
                                                .Type = SAHPI_SENSOR_READING_TYPE_FLOAT64,
                                                .Value = {
                                                          .SensorFloat64 = 60,
                                                          },
                                       },
                        .UpCritical =  {
                                                .IsSupported = SAHPI_TRUE,
                                                .Type = SAHPI_SENSOR_READING_TYPE_FLOAT64,
                                                .Value = {
		      			                  .SensorFloat64 = 125,
		      		                         },
                                       },
                        .UpMajor =     {
                                                .IsSupported = SAHPI_TRUE,
                                                .Type = SAHPI_SENSOR_READING_TYPE_FLOAT64,
                                                .Value = {
      	      			                          .SensorFloat64 = 120,
		      		                         },
                                       },
                        .UpMinor =     {
                                                .IsSupported = SAHPI_TRUE,
                                                .Type = SAHPI_SENSOR_READING_TYPE_FLOAT64,
                                                .Value = {
                                                          .SensorFloat64 = 110,
                                                         },
                                       },
                        .PosThdHysteresis = {
                                                .IsSupported = SAHPI_TRUE,
                                                .Type = SAHPI_SENSOR_READING_TYPE_FLOAT64,
                                                .Value = {
       	      			                          .SensorFloat64 = 2,
		      	 	                         },
                                            },
                        .NegThdHysteresis = {
                                                .IsSupported = SAHPI_TRUE,
                                                .Type = SAHPI_SENSOR_READING_TYPE_FLOAT64,
                                                .Value = {
                                                          .SensorFloat64 = 2,
                                                         },
                                            },
                     },
	    },
            .comment = "HS DASD temperature sensor 1"
        },
        /* Hot Swap DASD thermal sensor 2 */
        {
		.index = 2,
                .sensor = {
                        .Num = 2,
                        .Type = SAHPI_TEMPERATURE,
                        .Category = SAHPI_EC_THRESHOLD,
			.EnableCtrl = SAHPI_FALSE,
                        .EventCtrl = SAHPI_SEC_READ_ONLY,
                        .Events = SAHPI_ES_UPPER_MINOR | SAHPI_ES_UPPER_MAJOR | SAHPI_ES_UPPER_CRIT,
                        .DataFormat = {
                                .IsSupported = SAHPI_TRUE,
                                .ReadingType = SAHPI_SENSOR_READING_TYPE_FLOAT64,
                                .BaseUnits = SAHPI_SU_DEGREES_F,
                                .ModifierUnits = SAHPI_SU_UNSPECIFIED,
                                .ModifierUse = SAHPI_SMUU_NONE,
                                .Percentage = SAHPI_FALSE,
                                .Range = {
                                        .Flags = SAHPI_SRF_MAX | SAHPI_SRF_MIN |
                                                 SAHPI_SRF_NORMAL_MAX | SAHPI_SRF_NORMAL_MIN |
                                                 SAHPI_SRF_NOMINAL,
                                        .Max = {
						.IsSupported = SAHPI_TRUE,
                                                .Type = SAHPI_SENSOR_READING_TYPE_FLOAT64,
						.Value = {
							.SensorFloat64 = 125,
						},
					},
                                        .Min = {
						.IsSupported = SAHPI_TRUE,
                                                .Type = SAHPI_SENSOR_READING_TYPE_FLOAT64,
						.Value = {
							.SensorFloat64 = 40,
						},
                                        },
                                        .NormalMax = {
						.IsSupported = SAHPI_TRUE,
                                                .Type = SAHPI_SENSOR_READING_TYPE_FLOAT64,
						.Value = {
							.SensorFloat64 = 110,
						},
					},
                                        .Nominal = {
						.IsSupported = SAHPI_TRUE,
                                                .Type = SAHPI_SENSOR_READING_TYPE_FLOAT64,
						.Value = {
							.SensorFloat64 = 100,
						},
					},
                                        .NormalMin = {
						.IsSupported = SAHPI_TRUE,
                                                .Type = SAHPI_SENSOR_READING_TYPE_FLOAT64,
						.Value = {
							.SensorFloat64 = 90,
						},
                                        },
                                },
                        },
                        .ThresholdDefn = {
				.IsAccessible = SAHPI_FALSE,
                        },
                        .Oem = 0,
                },
		.sensor_info = {
			.cur_state = SAHPI_ES_UPPER_MINOR,
                        .sensor_enabled = SAHPI_TRUE,
                        .events_enabled = SAHPI_TRUE,
                        .assert_mask   = SAHPI_ES_UPPER_MINOR | SAHPI_ES_UPPER_MAJOR | SAHPI_ES_UPPER_CRIT,
                        .deassert_mask = SAHPI_ES_UPPER_MINOR | SAHPI_ES_UPPER_MAJOR | SAHPI_ES_UPPER_CRIT,
			.event_array = {
				{
                                        .event = "0611C480", /* EN_CUTOFF_HI_OVER_TEMP_PLANAR */
					.event_assertion = SAHPI_TRUE,
					.event_res_failure = SAHPI_TRUE,
					.event_res_failure_unexpected = SAHPI_TRUE,
                                        .event_state = SAHPI_ES_UPPER_CRIT | SAHPI_ES_UPPER_MAJOR | SAHPI_ES_UPPER_MINOR,
                                        .recovery_state = SAHPI_ES_UPPER_MAJOR | SAHPI_ES_UPPER_MINOR,
                                },
				{
                                        .event = "0611C080", /* EN_OVER_TEMP_PLANAR */
					.event_assertion = SAHPI_TRUE,
					.event_res_failure = SAHPI_TRUE,
					.event_res_failure_unexpected = SAHPI_TRUE,
                                        .event_state = SAHPI_ES_UPPER_CRIT | SAHPI_ES_UPPER_MAJOR | SAHPI_ES_UPPER_MINOR,
                                        .recovery_state = SAHPI_ES_UPPER_MAJOR | SAHPI_ES_UPPER_MINOR,
                                },
                                {
                                        .event = "0611D500", /* EN_PFA_HI_OVER_TEMP_PLANAR */
 					.event_assertion = SAHPI_TRUE,
					.event_res_failure = SAHPI_FALSE,
					.event_res_failure_unexpected = SAHPI_FALSE,
					.event_state = SAHPI_ES_UPPER_MAJOR | SAHPI_ES_UPPER_MINOR,
                                        .recovery_state = SAHPI_ES_UNSPECIFIED,
                                },
				{},
			},
            .reading = {
                                .IsSupported = SAHPI_TRUE,
                                .Type = SAHPI_SENSOR_READING_TYPE_FLOAT64,
                                .Value = {
				                .SensorFloat64 = 35,
				         },
                       },
            .thres = {
                        .LowCritical = {
                                                .IsSupported = SAHPI_FALSE,
                                       },
                        .LowMajor =    {
                                                .IsSupported = SAHPI_FALSE,
                                       },
                        .LowMinor =    {
                                                .IsSupported = SAHPI_FALSE,
                                       },
                        .UpCritical =  {
                                                .IsSupported = SAHPI_FALSE,
                                       },
                        .UpMajor =     {
                                                .IsSupported = SAHPI_FALSE,
                                       },
                        .UpMinor =     {
                                                .IsSupported = SAHPI_FALSE,
                                       },
                        .PosThdHysteresis = {
                                                .IsSupported = SAHPI_FALSE,
                                            },
                        .NegThdHysteresis = {
                                                .IsSupported = SAHPI_FALSE,
                                            },
                     },
	    },
            .comment = "HS DASD temperature sensor 2"
        },

        {} /* Terminate array with a null element */
};

/*************
 * Fan Sensors
 *************/

struct sim_sensor sim_fan_sensors[] = {
	/* Fan speed */
        {
		.index = 1,
                .sensor = {
                        .Num = 1,
                        .Type = SAHPI_FAN,
                        .Category = SAHPI_EC_PRED_FAIL,
  			.EnableCtrl = SAHPI_FALSE,
                        .EventCtrl = SAHPI_SEC_READ_ONLY,
                        .Events = SAHPI_ES_PRED_FAILURE_ASSERT | SAHPI_ES_PRED_FAILURE_DEASSERT,
                        .DataFormat = {
                                .IsSupported = SAHPI_TRUE,
                                .ReadingType = SAHPI_SENSOR_READING_TYPE_FLOAT64,
                                .BaseUnits = SAHPI_SU_RPM,
                                .ModifierUnits = SAHPI_SU_UNSPECIFIED,
                                .ModifierUse = SAHPI_SMUU_NONE,
                                .Percentage = SAHPI_TRUE,
                                .Range = {
                                        .Flags = SAHPI_SRF_MAX | SAHPI_SRF_MIN,
                                        .Max = {
  						.IsSupported = SAHPI_TRUE,
                                                .Type = SAHPI_SENSOR_READING_TYPE_FLOAT64,
						.Value = {
							.SensorFloat64 = 100,
                                                },
                                        },
                                        .Min = {
						.IsSupported = SAHPI_TRUE,
                                                .Type = SAHPI_SENSOR_READING_TYPE_FLOAT64,
						.Value = {
							.SensorFloat64 = 0,
                                                },
                                        },
                                },
                        },
                         .ThresholdDefn = {
                                .IsAccessible = SAHPI_FALSE,
                        },
                        .Oem = 0,
                },
		.sensor_info = {
			.cur_state = SAHPI_ES_PRED_FAILURE_DEASSERT,
                        .sensor_enabled = SAHPI_TRUE,
                        .events_enabled = SAHPI_TRUE,
			.assert_mask   = SAHPI_ES_PRED_FAILURE_ASSERT,
			.deassert_mask = SAHPI_ES_PRED_FAILURE_ASSERT,
			.event_array = {
				{
                                        .event = "000A600x", /* EN_FANx_PFA */
  					.event_assertion = SAHPI_TRUE,
       					.event_res_failure = SAHPI_FALSE,
					.event_res_failure_unexpected = SAHPI_FALSE,
                                        .event_state = SAHPI_ES_PRED_FAILURE_ASSERT,
                                        .recovery_state = SAHPI_ES_PRED_FAILURE_DEASSERT,
				},
				{},
			},
            .reading = { .IsSupported = SAHPI_TRUE,
                                              .Type = SAHPI_SENSOR_READING_TYPE_FLOAT64,
                                              .Value = {
							                        .SensorFloat64 = 60,
						                      },
                                            },
		},
                .comment = "Blower fan speed - percent of maximum RPM"
        },

        {} /* Terminate array with a null element */
};


/******************************************************************************
 *                      Control Definitions
 *
 * These are patterned after an RSA type machine
 ******************************************************************************/

struct sim_control sim_chassis_controls[] = {
  	/* Digital Control */
	{
                .index = 1,
                .control = {
                        .Num = 1,
                        .OutputType = SAHPI_CTRL_LED,
                        .Type = SAHPI_CTRL_TYPE_DIGITAL,
                        .TypeUnion.Digital.Default = SAHPI_CTRL_STATE_ON,   
			.DefaultMode = {
				.Mode = SAHPI_CTRL_MODE_AUTO,
				.ReadOnly = SAHPI_TRUE,
			},
			.WriteOnly = SAHPI_FALSE,
                        .Oem = 0,
                },
		.mode = SAHPI_CTRL_MODE_AUTO,
                .comment = "Digital Control"
        },
  	/* Discrete Control */
	{
                .index = 2,
                .control = {
                        .Num = 2,
                        .OutputType = SAHPI_CTRL_LED,
                        .Type = SAHPI_CTRL_TYPE_DISCRETE,
                        .TypeUnion.Discrete.Default = 1,
			.DefaultMode = {
				.Mode = SAHPI_CTRL_MODE_AUTO,
				.ReadOnly = SAHPI_TRUE,
			},
			.WriteOnly = SAHPI_FALSE,
                        .Oem = 0,
                },
		.mode = SAHPI_CTRL_MODE_AUTO,
                .comment = "Discrete Control"
        },
  	/* Analog Control */
	{
                .index = 3,
                .control = {
                        .Num = 3,
                        .OutputType = SAHPI_CTRL_AUDIBLE,
                        .Type = SAHPI_CTRL_TYPE_ANALOG,
                        .TypeUnion.Analog.Min = 0,
                        .TypeUnion.Analog.Max = 10,
                        .TypeUnion.Analog.Default = 0,
			.DefaultMode = {
				.Mode = SAHPI_CTRL_MODE_AUTO,
				.ReadOnly = SAHPI_TRUE,
			},
			.WriteOnly = SAHPI_FALSE,
                        .Oem = 0,
                },
		.mode = SAHPI_CTRL_MODE_AUTO,
                .comment = "Analog Control"
        },
  	/* Stream Control */
	{
                .index = 4,
                .control = {
                        .Num = 4,
                        .OutputType = SAHPI_CTRL_LED,
                        .Type = SAHPI_CTRL_TYPE_STREAM,
                        .TypeUnion.Stream.Default = {
                                .Stream[0] = 'O',
                                .Stream[1] = 'k',
                        },
			.DefaultMode = {
				.Mode = SAHPI_CTRL_MODE_AUTO,
				.ReadOnly = SAHPI_TRUE,
			},
			.WriteOnly = SAHPI_FALSE,
                        .Oem = 0,
                },
		.mode = SAHPI_CTRL_MODE_AUTO,
                .comment = "Stream Control"
        },
  	/* Text Control */
	{
                .index = 5,
                .control = {
                        .Num = 5,
                        .OutputType = SAHPI_CTRL_LCD_DISPLAY,
                        .Type = SAHPI_CTRL_TYPE_TEXT,
                        .TypeUnion.Text.MaxChars = 10,
                        .TypeUnion.Text.MaxLines = 2,
                        .TypeUnion.Text.Language = SAHPI_LANG_ENGLISH,
                        .TypeUnion.Text.DataType = SAHPI_TL_TYPE_TEXT,
                        .TypeUnion.Text.Default = {
                                .Line = 0,
                                .Text = {
                                        .DataType = SAHPI_TL_TYPE_TEXT,
                                        .Language = SAHPI_LANG_ENGLISH,
                                        .DataLength = 7,
                                        .Data[0] = 'U',
                                        .Data[1] = 'n',
                                        .Data[2] = 'k',
                                        .Data[3] = 'n',
                                        .Data[4] = 'w',
                                        .Data[5] = 'o',
                                        .Data[6] = 'n',
                                        },
                        },
			.DefaultMode = {
				.Mode = SAHPI_CTRL_MODE_AUTO,
				.ReadOnly = SAHPI_TRUE,
			},
			.WriteOnly = SAHPI_FALSE,
                        .Oem = 0,
                },
		.mode = SAHPI_CTRL_MODE_AUTO,
                .comment = "Text Control"
        },
  	/* Oem Control */
	{
                .index = 6,
                .control = {
                        .Num = 6,
                        .OutputType = SAHPI_CTRL_LED,
                        .Type = SAHPI_CTRL_TYPE_OEM,
                        .TypeUnion.Oem.MId = 123,
                        .TypeUnion.Oem.ConfigData[0] = 0,
                        .TypeUnion.Oem.ConfigData[1] = 0,
                        .TypeUnion.Oem.ConfigData[2] = 0,
                        .TypeUnion.Oem.ConfigData[3] = 0,
                        .TypeUnion.Oem.ConfigData[4] = 0,
                        .TypeUnion.Oem.ConfigData[5] = 0,
                        .TypeUnion.Oem.ConfigData[6] = 0,
                        .TypeUnion.Oem.ConfigData[7] = 0,
                        .TypeUnion.Oem.ConfigData[8] = 0,
                        .TypeUnion.Oem.ConfigData[9] = 0,
                        .TypeUnion.Oem.Default = {
                                .MId = 123,
                                .BodyLength = 2,
                                .Body[0] = 'O',
                                .Body[1] = 'k',
                        },
			.DefaultMode = {
				.Mode = SAHPI_CTRL_MODE_AUTO,
				.ReadOnly = SAHPI_TRUE,
			},
			.WriteOnly = SAHPI_FALSE,
                        .Oem = 0,
                },
		.mode = SAHPI_CTRL_MODE_AUTO,
                .comment = "Oem Control"
        },

        {} /* Terminate array with a null element */
};

struct sim_control sim_cpu_controls[] = {

        {} /* Terminate array with a null element */
};

struct sim_control sim_dasd_controls[] = {

        {} /* Terminate array with a null element */
};

struct sim_control sim_hs_dasd_controls[] = {
  	/* Digital Control */
	{
                .index = 1,
                .control = {
                        .Num = 1,
                        .OutputType = SAHPI_CTRL_LED,
                        .Type = SAHPI_CTRL_TYPE_DIGITAL,
                        .TypeUnion.Digital.Default = SAHPI_CTRL_STATE_ON,
			.DefaultMode = {
				.Mode = SAHPI_CTRL_MODE_AUTO,
				.ReadOnly = SAHPI_TRUE,
			},
			.WriteOnly = SAHPI_FALSE,
                        .Oem = 0,
                },
		.mode = SAHPI_CTRL_MODE_AUTO,
                .comment = "Digital Control"
        },
  	/* Discrete Control */
	{
                .index = 2,
                .control = {
                        .Num = 2,
                        .OutputType = SAHPI_CTRL_LED,
                        .Type = SAHPI_CTRL_TYPE_DISCRETE,
                        .TypeUnion.Discrete.Default = 1,
			.DefaultMode = {
				.Mode = SAHPI_CTRL_MODE_AUTO,
				.ReadOnly = SAHPI_TRUE,
			},
			.WriteOnly = SAHPI_FALSE,
                        .Oem = 0,
                },
		.mode = SAHPI_CTRL_MODE_AUTO,
                .comment = "Discrete Control"
        },
  	/* Analog Control */
	{
                .index = 3,
                .control = {
                        .Num = 3,
                        .OutputType = SAHPI_CTRL_AUDIBLE,
                        .Type = SAHPI_CTRL_TYPE_ANALOG,
                        .TypeUnion.Analog.Min = 0,
                        .TypeUnion.Analog.Max = 10,
                        .TypeUnion.Analog.Default = 0,
			.DefaultMode = {
				.Mode = SAHPI_CTRL_MODE_AUTO,
				.ReadOnly = SAHPI_TRUE,
			},
			.WriteOnly = SAHPI_FALSE,
                        .Oem = 0,
                },
		.mode = SAHPI_CTRL_MODE_AUTO,
                .comment = "Analog Control"
        },
  	/* Stream Control */
	{
                .index = 4,
                .control = {
                        .Num = 4,
                        .OutputType = SAHPI_CTRL_LED,
                        .Type = SAHPI_CTRL_TYPE_STREAM,
                        .TypeUnion.Stream.Default = {
                                .Stream[0] = 'O',
                                .Stream[1] = 'k',
                        },
			.DefaultMode = {
				.Mode = SAHPI_CTRL_MODE_AUTO,
				.ReadOnly = SAHPI_TRUE,
			},
			.WriteOnly = SAHPI_FALSE,
                        .Oem = 0,
                },
		.mode = SAHPI_CTRL_MODE_AUTO,
                .comment = "Stream Control"
        },
  	/* Text Control */
	{
                .index = 5,
                .control = {
                        .Num = 5,
                        .OutputType = SAHPI_CTRL_LCD_DISPLAY,
                        .Type = SAHPI_CTRL_TYPE_TEXT,
                        .TypeUnion.Text.MaxChars = 10,
                        .TypeUnion.Text.MaxLines = 2,
                        .TypeUnion.Text.Language = SAHPI_LANG_ENGLISH,
                        .TypeUnion.Text.DataType = SAHPI_TL_TYPE_TEXT,
                        .TypeUnion.Text.Default = {
                                .Line = 0,
                                .Text = {
                                        .DataType = SAHPI_TL_TYPE_TEXT,
                                        .Language = SAHPI_LANG_ENGLISH,
                                        .DataLength = 7,
                                        .Data[0] = 'U',
                                        .Data[1] = 'n',
                                        .Data[2] = 'k',
                                        .Data[3] = 'n',
                                        .Data[4] = 'w',
                                        .Data[5] = 'o',
                                        .Data[6] = 'n',
                                        },
                        },
			.DefaultMode = {
				.Mode = SAHPI_CTRL_MODE_AUTO,
				.ReadOnly = SAHPI_TRUE,
			},
			.WriteOnly = SAHPI_FALSE,
                        .Oem = 0,
                },
		.mode = SAHPI_CTRL_MODE_AUTO,
                .comment = "Text Control"
        },
  	/* Oem Control */
	{
                .index = 6,
                .control = {
                        .Num = 6,
                        .OutputType = SAHPI_CTRL_LED,
                        .Type = SAHPI_CTRL_TYPE_OEM,
                        .TypeUnion.Oem.MId = 123,
                        .TypeUnion.Oem.ConfigData[0] = 0,
                        .TypeUnion.Oem.ConfigData[1] = 0,
                        .TypeUnion.Oem.ConfigData[2] = 0,
                        .TypeUnion.Oem.ConfigData[3] = 0,
                        .TypeUnion.Oem.ConfigData[4] = 0,
                        .TypeUnion.Oem.ConfigData[5] = 0,
                        .TypeUnion.Oem.ConfigData[6] = 0,
                        .TypeUnion.Oem.ConfigData[7] = 0,
                        .TypeUnion.Oem.ConfigData[8] = 0,
                        .TypeUnion.Oem.ConfigData[9] = 0,
                        .TypeUnion.Oem.Default = {
                                .MId = 123,
                                .BodyLength = 2,
                                .Body[0] = 'O',
                                .Body[1] = 'k',
                        },
			.DefaultMode = {
				.Mode = SAHPI_CTRL_MODE_AUTO,
				.ReadOnly = SAHPI_TRUE,
			},
			.WriteOnly = SAHPI_FALSE,
                        .Oem = 0,
                },
		.mode = SAHPI_CTRL_MODE_AUTO,
                .comment = "Oem Control"
        },

        {} /* Terminate array with a null element */
};

struct sim_control sim_fan_controls[] = {
	{
		.index = 1,
		.control = {
			.Num = 1,
			.OutputType = SAHPI_CTRL_FAN_SPEED,
			.Type = SAHPI_CTRL_TYPE_ANALOG,
			.TypeUnion = {
				.Analog = {
					.Min = 0,
					.Max = 100,
					.Default = 80
				}
			},
			.DefaultMode = {
				.Mode = SAHPI_CTRL_MODE_MANUAL,
				.ReadOnly = SAHPI_FALSE
			},
			.WriteOnly = SAHPI_FALSE,
			.Oem = 0
		},
		.mode = SAHPI_CTRL_MODE_MANUAL,
		.comment = "Fan Analog Control"
	},

        {} /* Terminate array with a null element */
};


/******************************************************************************
 *                      Annunciator Definitions
 *
 * These are completely made up as RSA has no annunciators
 ******************************************************************************/

struct sim_annunciator sim_chassis_annunciators[] = {
        {
                .index = 1,
                .annun = {
                        .AnnunciatorNum = 1,
                        .AnnunciatorType = SAHPI_ANNUNCIATOR_TYPE_AUDIBLE,
                        .ModeReadOnly = SAHPI_FALSE,
                        .MaxConditions = 2,
                        .Oem = 0,
                },
                .announs[0] = {
                        .EntryId = 1,
                        .Timestamp = 0,
                        .AddedByUser = SAHPI_FALSE,
                        .Severity = SAHPI_MAJOR,
                        .Acknowledged = SAHPI_FALSE,
                        .StatusCond = {
                                .Type = SAHPI_STATUS_COND_TYPE_SENSOR,
                                .Entity   = {
                                        .Entry = {
                                                {SAHPI_ENT_SYSTEM_BOARD, 1},
                                                {SAHPI_ENT_ROOT, 0}
                                        },
                                },
                                .DomainId = 1,
                                .ResourceId = 1,
                                .SensorNum = 1,
                                .EventState = SAHPI_ES_UNSPECIFIED,
                                .Name = {
                                        .Length = 5,
                                        .Value = "announ"
                                },
                                .Mid = 123,
                        },
                },
                .announs[1] = {
                        .EntryId = 2,
                        .Timestamp = 0,
                        .AddedByUser = SAHPI_FALSE,
                        .Severity = SAHPI_MINOR,
                        .Acknowledged = SAHPI_FALSE,
                        .StatusCond = {
                                .Type = SAHPI_STATUS_COND_TYPE_SENSOR,
                                .Entity   = {
                                        .Entry = {
                                                {SAHPI_ENT_SYSTEM_BOARD, 1},
                                                {SAHPI_ENT_ROOT, 0}
                                        },
                                },
                                .DomainId = 1,
                                .ResourceId = 1,
                                .SensorNum = 1,
                                .EventState = SAHPI_ES_UNSPECIFIED,
                                .Name = {
                                        .Length = 5,
                                        .Value = "announ"
                                },
                                .Mid = 123,
                        },
                },
                .announs[2] = {
                        .EntryId = 3,
                        .Timestamp = 0,
                        .AddedByUser = SAHPI_FALSE,
                        .Severity = SAHPI_INFORMATIONAL,
                        .Acknowledged = SAHPI_FALSE,
                        .StatusCond = {
                                .Type = SAHPI_STATUS_COND_TYPE_SENSOR,
                                .Entity   = {
                                        .Entry = {
                                                {SAHPI_ENT_SYSTEM_BOARD, 1},
                                                {SAHPI_ENT_ROOT, 0}
                                        },
                                },
                                .DomainId = 1,
                                .ResourceId = 1,
                                .SensorNum = 1,
                                .EventState = SAHPI_ES_UNSPECIFIED,
                                .Name = {
                                        .Length = 5,
                                        .Value = "announ"
                                },
                                .Mid = 123,
                        },
                },
                .comment = "Annunciator 1"
        },

        {} /* Terminate array with a null element */
};

struct sim_annunciator sim_cpu_annunciators[] = {

        {} /* Terminate array with a null element */
};

struct sim_annunciator sim_dasd_annunciators[] = {

        {} /* Terminate array with a null element */
};

struct sim_annunciator sim_hs_dasd_annunciators[] = {
        {
                .index = 1,
                .annun = {
                        .AnnunciatorNum = 1,
                        .AnnunciatorType = SAHPI_ANNUNCIATOR_TYPE_AUDIBLE,
                        .ModeReadOnly = SAHPI_FALSE,
                        .MaxConditions = 2,
                        .Oem = 0,
                },
                .announs[0] = {
                        .EntryId = 1,
                        .Timestamp = 0,
                        .AddedByUser = SAHPI_FALSE,
                        .Severity = SAHPI_MAJOR,
                        .Acknowledged = SAHPI_FALSE,
                        .StatusCond = {
                                .Type = SAHPI_STATUS_COND_TYPE_SENSOR,
                                .Entity   = {
                                        .Entry = {
                                                {SAHPI_ENT_DISK_DRIVE, 2},
                                                {SAHPI_ENT_ROOT, 0}
                                        },
                                },
                                .DomainId = 1,
                                .ResourceId = 1,
                                .SensorNum = 1,
                                .EventState = SAHPI_ES_UNSPECIFIED,
                                .Name = {
                                        .Length = 5,
                                        .Value = "announ"
                                },
                                .Mid = 123,
                        },
                },
                .announs[1] = {
                        .EntryId = 2,
                        .Timestamp = 0,
                        .AddedByUser = SAHPI_FALSE,
                        .Severity = SAHPI_MINOR,
                        .Acknowledged = SAHPI_FALSE,
                        .StatusCond = {
                                .Type = SAHPI_STATUS_COND_TYPE_SENSOR,
                                .Entity   = {
                                        .Entry = {
                                                {SAHPI_ENT_DISK_DRIVE, 2},
                                                {SAHPI_ENT_ROOT, 0}
                                        },
                                },
                                .DomainId = 1,
                                .ResourceId = 1,
                                .SensorNum = 1,
                                .EventState = SAHPI_ES_UNSPECIFIED,
                                .Name = {
                                        .Length = 5,
                                        .Value = "announ"
                                },
                                .Mid = 123,
                        },
                },
                .announs[2] = {
                        .EntryId = 3,
                        .Timestamp = 0,
                        .AddedByUser = SAHPI_FALSE,
                        .Severity = SAHPI_INFORMATIONAL,
                        .Acknowledged = SAHPI_FALSE,
                        .StatusCond = {
                                .Type = SAHPI_STATUS_COND_TYPE_SENSOR,
                                .Entity   = {
                                        .Entry = {
                                                {SAHPI_ENT_DISK_DRIVE, 2},
                                                {SAHPI_ENT_ROOT, 0}
                                        },
                                },
                                .DomainId = 1,
                                .ResourceId = 1,
                                .SensorNum = 1,
                                .EventState = SAHPI_ES_UNSPECIFIED,
                                .Name = {
                                        .Length = 5,
                                        .Value = "announ"
                                },
                                .Mid = 123,
                        },
                },
                .comment = "Annunciator 2"
        },

        {} /* Terminate array with a null element */
};

struct sim_annunciator sim_fan_annunciators[] = {

        {} /* Terminate array with a null element */
};


/******************************************************************************
 *                      Watchdog Definitions
 *
 * These are completely made up as RSA has no watchdogs
 ******************************************************************************/

struct sim_watchdog sim_chassis_watchdogs[] = {
        {
                .watchdogrec = {
                        .WatchdogNum = 1,
                        .Oem = 0,
                },
                .wd = {
                        .Log = SAHPI_TRUE,
                        .Running = SAHPI_FALSE,
                        .TimerUse = SAHPI_WTU_NONE,
                        .TimerAction = SAHPI_WA_NO_ACTION,
                        .PretimerInterrupt = SAHPI_WPI_NONE,
                        .PreTimeoutInterval = 0,
                        .TimerUseExpFlags = SAHPI_WTU_NONE,
                        .InitialCount = 0,
                        .PresentCount = 0,
                },
                .comment = "Watchdog 1"
        },

        {} /* Terminate array with a null element */
};

struct sim_watchdog sim_cpu_watchdogs[] = {

        {} /* Terminate array with a null element */
};

struct sim_watchdog sim_dasd_watchdogs[] = {

        {} /* Terminate array with a null element */
};

struct sim_watchdog sim_hs_dasd_watchdogs[] = {
        {
                .watchdogrec = {
                        .WatchdogNum = 1,
                        .Oem = 0,
                },
                .wd = {
                        .Log = SAHPI_TRUE,
                        .Running = SAHPI_FALSE,
                        .TimerUse = SAHPI_WTU_NONE,
                        .TimerAction = SAHPI_WA_NO_ACTION,
                        .PretimerInterrupt = SAHPI_WPI_NONE,
                        .PreTimeoutInterval = 0,
                        .TimerUseExpFlags = SAHPI_WTU_NONE,
                        .InitialCount = 0,
                        .PresentCount = 0,
                },
                .comment = "Watchdog 2"
        },

        {} /* Terminate array with a null element */
};

struct sim_watchdog sim_fan_watchdogs[] = {

        {} /* Terminate array with a null element */
};



/*************************************************************************
 *                   Inventory Definitions
 *************************************************************************/

struct sim_inventory sim_chassis_inventory[] = {
        {
                .invrec = {
                        .IdrId = 1,
                        .Persistent = SAHPI_FALSE,
                        .Oem = 0,
                },
                .info = {
                        .nextareaid = 2, // change if you add more areas below
                        .idrinfo = {
                                .IdrId = 1,
                                .UpdateCount = 0,
                                .ReadOnly = SAHPI_TRUE,
                                .NumAreas = 1, // change if you want more areas below
                        },
                        .area[0] = {
                                .nextfieldid = 2, // change if you add more fields below
                                .idrareahead = {
                                        .AreaId = 1,
                                        .Type = SAHPI_IDR_AREATYPE_CHASSIS_INFO,
                                        .ReadOnly = SAHPI_TRUE,
                                        .NumFields = 1, //change if you add more fields below
                                },
                                .field[0] = {
                                        .AreaId = 1,
                                        .FieldId = 1,
                                        .Type = SAHPI_IDR_FIELDTYPE_MANUFACTURER,
                                        .ReadOnly = SAHPI_TRUE,
                                        .Field = {
                                                .DataType = SAHPI_TL_TYPE_TEXT,
                                                .Language = SAHPI_LANG_ENGLISH,
                                                .DataLength = 6,
                                                .Data[0] = 'I',
                                                .Data[1] = 'B',
                                                .Data[2] = 'M',
                                                .Data[3] = 'X',
                                                .Data[4] = 'X',
                                                .Data[5] = 'X',
                                                .Data[6] = '\0',
                                        },
                                },
                        },
                },
                .comment = "Simulator Inv 1",
        },

        {} /* Terminate array with a null element */
};

struct sim_inventory sim_cpu_inventory[] = {

        {} /* Terminate array with a null element */
};

struct sim_inventory sim_dasd_inventory[] = {

        {} /* Terminate array with a null element */
};

struct sim_inventory sim_hs_dasd_inventory[] = {
        {
                .invrec = {
                        .IdrId = 1,
                        .Persistent = SAHPI_FALSE,
                        .Oem = 0,
                },
                .info = {
                        .nextareaid = 2, // change if you add more areas below
                        .idrinfo = {
                                .IdrId = 1,
                                .UpdateCount = 0,
                                .ReadOnly = SAHPI_TRUE,
                                .NumAreas = 1, // change if you want more areas below
                        },
                        .area[0] = {
                                .nextfieldid = 2, // change if you add more fields below
                                .idrareahead = {
                                        .AreaId = 1,
                                        .Type = SAHPI_IDR_AREATYPE_CHASSIS_INFO,
                                        .ReadOnly = SAHPI_TRUE,
                                        .NumFields = 1, //change if you add more fields below
                                },
                                .field[0] = {
                                        .AreaId = 1,
                                        .FieldId = 1,
                                        .Type = SAHPI_IDR_FIELDTYPE_MANUFACTURER,
                                        .ReadOnly = SAHPI_TRUE,
                                        .Field = {
                                                .DataType = SAHPI_TL_TYPE_TEXT,
                                                .Language = SAHPI_LANG_ENGLISH,
                                                .DataLength = 6,
                                                .Data[0] = 'I',
                                                .Data[1] = 'B',
                                                .Data[2] = 'M',
                                                .Data[3] = 'X',
                                                .Data[4] = 'X',
                                                .Data[5] = 'X',
                                                .Data[6] = '\0',
                                        },
                                },
                        },
                },
                .comment = "Simulator HS DASD Inv 1",
        },

        {} /* Terminate array with a null element */
};

struct sim_inventory sim_fan_inventory[] = {

        {} /* Terminate array with a null element */
};

struct sim_dimi sim_chassis_dimis[] = {
    {
            .dimirec = {
                    .DimiNum = 1,
                    .Oem = 0,
            },
            
            .info = {
                .NumberOfTests =1,
                .TestNumUpdateCounter=0,
            },
            .test = {
                    .TestName={
                        .DataType = SAHPI_TL_TYPE_TEXT,
                        .Language = SAHPI_LANG_ENGLISH,
                        .DataLength = 7,
                        .Data = "unknown"
                    },
                    .ServiceImpact=SAHPI_DIMITEST_NONDEGRADING,
                    .EntitiesImpacted= {
                        {
                            .EntityImpacted.Entry= {
                                {
                                        .EntityType = SAHPI_ENT_PROCESSOR,
                                        .EntityLocation = 1
                                },
                                {
                                        .EntityType = SAHPI_ENT_POWER_SUPPLY,
                                        .EntityLocation = 1
                                },
                                {
                                        .EntityType = SAHPI_ENT_ADD_IN_CARD,
                                        .EntityLocation = 1
                                },
                                {
                                        .EntityType = SAHPI_ENT_POWER_UNIT,
                                        .EntityLocation = 1
                                },
                                {
                                        .EntityType = SAHPI_ENT_ROOT,
                                        .EntityLocation = 0
                                },
                            },    
                            .ServiceImpact=SAHPI_DIMITEST_NONDEGRADING,
                        }
                    },
                    .NeedServiceOS= SAHPI_FALSE,
                    .ExpectedRunDuration=1000000000,
                    .TestCapabilities=SAHPI_DIMITEST_CAPABILITY_TESTCANCEL,
                    .TestParameters = {
                                {
                                        .ParamName = "Some Test Param Name",
                                        .ParamInfo = {
                                                .DataType = SAHPI_TL_TYPE_TEXT,
                                                .Language = SAHPI_LANG_ENGLISH,
                                                .DataLength = 7,
                                                .Data = "Unknown"
                                        },
                                        .ParamType = SAHPI_DIMITEST_PARAM_TYPE_BOOLEAN,
                                        .DefaultParam.parambool = SAHPI_TRUE
                                }
                    }
            },
            .comment = "Dimi 1 simulator",
    },
    
    {} /* Terminate array with a null element */
};

struct sim_fumi sim_chassis_fumis[] = {
    {
            .fumirec = {
                    .Num = 1,
                    .AccessProt = SAHPI_FUMI_PROT_FTP,
                    .Capability = SAHPI_FUMI_CAP_BANKCOPY,
                    .NumBanks = 1,
                    .Oem = 0,
            },
            
            .srcinfo = {
                    .SourceUri = {
                        .DataType = SAHPI_TL_TYPE_TEXT,
                        .Language = SAHPI_LANG_ENGLISH,
                        .DataLength = 7,
                        .Data = "unknown"
                    },
                    .SourceStatus = SAHPI_FUMI_SRC_VALID,
                    .Identifier = {
                        .DataType = SAHPI_TL_TYPE_TEXT,
                        .Language = SAHPI_LANG_ENGLISH,
                        .DataLength = 3,
                        .Data = "abc"
                    },
                    .Description = {
                        .DataType = SAHPI_TL_TYPE_TEXT,
                        .Language = SAHPI_LANG_ENGLISH,
                        .DataLength = 3,
                        .Data = "xyz"
                    },
                    .DateTime = {
                        .DataType = SAHPI_TL_TYPE_TEXT,
                        .Language = SAHPI_LANG_ENGLISH,
                        .DataLength = 3,
                        .Data = "09/05/2008"
                    },
                    .MajorVersion = 0,
                    .MinorVersion = 0,
                    .AuxVersion = 0
            },
            
            .info = {
                    .BankId = 0,
                    .BankSize = 250,
                    .Position = 1,
                    .BankState = SAHPI_FUMI_BANK_ACTIVE,
                    .Identifier = {
                        .DataType = SAHPI_TL_TYPE_TEXT,
                        .Language = SAHPI_LANG_ENGLISH,
                        .DataLength = 3,
                        .Data = "abc"
                    },
                    .Description = {
                        .DataType = SAHPI_TL_TYPE_TEXT,
                        .Language = SAHPI_LANG_ENGLISH,
                        .DataLength = 3,
                        .Data = "xyz"
                    },
                    .DateTime = {
                        .DataType = SAHPI_TL_TYPE_TEXT,
                        .Language = SAHPI_LANG_ENGLISH,
                        .DataLength = 3,
                        .Data = "09/05/2008"
                    },
                    .MajorVersion = 0,
                    .MinorVersion = 0,
                    .AuxVersion = 0
            },
            .comment = "Fumi 1 simulator",
    },
    
    {} /* Terminate array with a null element */
};
