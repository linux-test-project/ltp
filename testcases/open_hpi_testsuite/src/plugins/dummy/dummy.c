/*      -*- linux-c -*- 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <uuid/uuid.h>

#include <SaHpi.h>
#include <openhpi.h>
#include <epath_utils.h>
#include <uid_utils.h>

#define info(f, ...) printf(__FILE__": " f "\n", ## __VA_ARGS__)
#define error(f, ...) perror("ERROR: " f, ## __VA_ARGS__)
#define trace(f, ...) printf(__FILE__":%s(" f ")\n", __FUNCTION__, ## __VA_ARGS__)
#define ELEMENT_NUM(x) (sizeof(x)/sizeof(x[0]))


#define NUM_RESOURCES 	3
#define NUM_RDRS	8

#define VIRTUAL_NODES 2

//static int remove_resource(struct oh_handler_state *inst);


/* dummy resource status */
static struct {
	SaHpiHsPowerStateT 	power;
	SaHpiHsStateT		hotswap;
	SaHpiHsIndicatorStateT	indicator;
	SaHpiResetActionT	reset;
	SaHpiResourceIdT    	ResourceId;
} dummy_resource_status[NUM_RESOURCES] = {
	{
		.power = SAHPI_HS_POWER_ON,
		.hotswap = SAHPI_HS_STATE_ACTIVE_HEALTHY,
		.indicator =  SAHPI_HS_INDICATOR_ON,
		.reset = SAHPI_COLD_RESET,
	},
	{
		.power = SAHPI_HS_POWER_ON,
		.hotswap = SAHPI_HS_STATE_ACTIVE_HEALTHY,
		.indicator =  SAHPI_HS_INDICATOR_ON,
		.reset = SAHPI_COLD_RESET,
	},
	{
		.power = SAHPI_HS_POWER_ON,
		.hotswap = SAHPI_HS_STATE_ACTIVE_HEALTHY,
		.indicator =  SAHPI_HS_INDICATOR_ON,
		.reset = SAHPI_COLD_RESET,
	},
};

/* dummy entity array */
static SaHpiRptEntryT dummy_resources[NUM_RESOURCES] = {

/*  SaHpiEntryIdT        EntryId;
    SaHpiResourceIdT     ResourceId;
    SaHpiResourceInfoT   ResourceInfo;
    SaHpiEntityPathT     ResourceEntity; 
    SaHpiCapabilitiesT   ResourceCapabilities;  
    SaHpiSeverityT       ResourceSeverity; 
    SaHpiDomainIdT       DomainId;  
    SaHpiTextBufferT     ResourceTag; */
	/* resource one */
	{
		.DomainId = SAHPI_DEFAULT_DOMAIN_ID,
		.EntryId = 0, 
		.ResourceCapabilities =
			SAHPI_CAPABILITY_INVENTORY_DATA |
			SAHPI_CAPABILITY_CONTROL |
			SAHPI_CAPABILITY_RESOURCE |
			SAHPI_CAPABILITY_RDR |
			SAHPI_CAPABILITY_CONFIGURATION |
                        SAHPI_CAPABILITY_SENSOR |
			SAHPI_CAPABILITY_SEL,
		.ResourceEntity = {	
			.Entry = {
				{SAHPI_ENT_SYSTEM_BOARD, 1},
				{0, 0}
			},
		},
		.ResourceId = 0,
		.ResourceInfo = {0},
		.ResourceSeverity= SAHPI_MAJOR, 
		.ResourceTag	= {
			.DataType	= SAHPI_TL_TYPE_ASCII6,
			.Language	= SAHPI_LANG_ENGLISH,
			.DataLength 	= 18,
			.Data		= "Dummy-System-Board"
		},	
	},
	/* resource two */
	{
		.DomainId = SAHPI_DEFAULT_DOMAIN_ID,
		.EntryId = 0, 
		.ResourceCapabilities = 
			SAHPI_CAPABILITY_RESOURCE 
			| SAHPI_CAPABILITY_RDR 
                        | SAHPI_CAPABILITY_SENSOR 
			| SAHPI_CAPABILITY_FRU 
			| SAHPI_CAPABILITY_MANAGED_HOTSWAP,
		.ResourceEntity = {	
			.Entry = {
				{SAHPI_ENT_POWER_SUPPLY, 1},
				{0, 0}
			},
		},
		.ResourceId = 0,
		.ResourceInfo = {0},
		.ResourceSeverity= SAHPI_MAJOR, 
		.ResourceTag	= {
			.DataType	= SAHPI_TL_TYPE_ASCII6,
			.Language	= SAHPI_LANG_ENGLISH,
			.DataLength 	= 18,
			.Data		= "Dummy-System-Board"
		},	
	},
	/* resource third */
	{
		.DomainId = SAHPI_DEFAULT_DOMAIN_ID,
		.EntryId = 0, 
		.ResourceCapabilities = SAHPI_CAPABILITY_RESOURCE,
		.ResourceEntity = {	
			.Entry = {
				{SAHPI_ENT_POWER_SUPPLY, 2},
				{0, 0}
			},
		},
		.ResourceId = 0,
		.ResourceInfo = {0},
		.ResourceSeverity= SAHPI_MAJOR, 
		.ResourceTag	= {
			.DataType	= SAHPI_TL_TYPE_ASCII6,
			.Language	= SAHPI_LANG_ENGLISH,
			.DataLength 	= 18,
			.Data		= "Dummy-System-Board"
		},	
	}
};

static struct oh_event hotswap_event[] = {
	/* This is an hotswap event for insertion pending */
	{
		.type = OH_ET_HPI,
		.u = {
			.hpi_event = {
				.parent = 0, /* this needs to be the resourceId */
				.id = 1, /* ????, unquie amoung the hotswap events */
				.event = {
					.Source = 0,
					.EventType = SAHPI_ET_HOTSWAP,
					.Timestamp = 0,
					.Severity = SAHPI_CRITICAL,
					.EventDataUnion = {
						.HotSwapEvent = {
							.HotSwapState = SAHPI_HS_STATE_INSERTION_PENDING,
							.PreviousHotSwapState = SAHPI_HS_STATE_NOT_PRESENT,
						},
					},
				},
			},
		},
	},
	/* This is an hotswap event for insertion pending */
	{
		.type = OH_ET_HPI,
		.u = {
			.hpi_event = {
				.parent = 0,  /* this needs to be the resourceId */
				.id = 2,      /* ????, unquie amoung the hotswap events */
				.event = {
					.Source = 0,
					.EventType = SAHPI_ET_HOTSWAP,
					.Timestamp = 0,
					.Severity = SAHPI_CRITICAL,
					.EventDataUnion = {
						.HotSwapEvent = {
							.HotSwapState = SAHPI_HS_STATE_EXTRACTION_PENDING,
							.PreviousHotSwapState = SAHPI_HS_STATE_ACTIVE_HEALTHY,
						},
					},
				},
			},
		},
	},
};

static SaHpiRdrT dummy_rdrs[] = {
	/* first rdr */
	/* This is the one temperature sensor on System Board */
	{
		.RecordId = 0, /*no use, should be rewritten by upper layer */
		.RdrType  = SAHPI_SENSOR_RDR,
		.Entity   = {
			.Entry = {
				{SAHPI_ENT_SYSTEM_BOARD, 1},
				{0, 0}
			},
		},
		.RdrTypeUnion = {
			.SensorRec = {
				.Num  = 1, /*no use! should be rewritten by upper layer */
				.Type = SAHPI_TEMPERATURE,
				.Category = SAHPI_EC_THRESHOLD,
				.EventCtrl = SAHPI_SEC_PER_EVENT,
				.Events	= 
					SAHPI_ES_UPPER_MINOR
					| SAHPI_ES_UPPER_MAJOR
					| SAHPI_ES_UPPER_CRIT,
				.Ignore = SAHPI_FALSE,
				.DataFormat = {
					.ReadingFormats = SAHPI_SRF_RAW,
					.IsNumeric = SAHPI_TRUE,
					.SignFormat = SAHPI_SDF_UNSIGNED,
					.BaseUnits = SAHPI_SU_DEGREES_C,
					.ModifierUnits = SAHPI_SU_UNSPECIFIED,
					.ModifierUse = SAHPI_SMUU_NONE,
					.FactorsStatic = SAHPI_TRUE,
					.Factors = { 
						.M_Factor = 1, 
						.B_Factor = 0, 
						.AccuracyFactor = 0,
						.ToleranceFactor = 0,
						.ExpA = 0,
						.ExpR = 0,
						.ExpB = 0,
						.Linearization = SAHPI_SL_LINEAR,
					},
					.Percentage = SAHPI_FALSE,
					.Range = {
						.Flags = SAHPI_SRF_MIN | SAHPI_SRF_MAX,
						.Max = {
							.ValuesPresent = SAHPI_SRF_RAW,
							.Raw = 200,
						},
						.Min = {
							.ValuesPresent = SAHPI_SRF_RAW,
							.Raw = 0,
						}
					},
				},
				.ThresholdDefn = {
					.IsThreshold = SAHPI_TRUE,
					.TholdCapabilities=SAHPI_STC_RAW,
					.ReadThold = SAHPI_STM_LOW_CRIT,
					.WriteThold = SAHPI_STM_LOW_CRIT,
				},
				.Oem = 0,
			},
		},
		.IdString = {
			.DataType	= SAHPI_TL_TYPE_ASCII6,
			.Language	= SAHPI_LANG_ENGLISH,
			.DataLength 	= 24,
			.Data		= "Dummy-System-Temperature"
		},
	},
	/* second rdr */
	/* This is the one temperature sensor on Power Supply */
	{
		.RecordId = 0, /*no use, should be rewritten by upper layer */
		.RdrType  = SAHPI_SENSOR_RDR,
		.Entity   = {
			.Entry = {
				{SAHPI_ENT_POWER_SUPPLY, 1},
				{0, 0}
			},
		},
		.RdrTypeUnion = {
			.SensorRec = {
				.Num  = 2, /*no use! should be rewritten by upper layer */
				.Type = SAHPI_TEMPERATURE,
				.Category = SAHPI_EC_THRESHOLD,
				.EventCtrl = SAHPI_SEC_PER_EVENT,
				.Events	= SAHPI_ES_UPPER_MINOR | SAHPI_ES_UPPER_MAJOR | SAHPI_ES_UPPER_CRIT,
				.Ignore = SAHPI_FALSE,
				.DataFormat = {
					.ReadingFormats = SAHPI_SRF_RAW,
					.IsNumeric = SAHPI_TRUE,
					.SignFormat = SAHPI_SDF_UNSIGNED,
					.BaseUnits = SAHPI_SU_DEGREES_C,
					.ModifierUnits = SAHPI_SU_UNSPECIFIED,
					.ModifierUse = SAHPI_SMUU_NONE,
					.FactorsStatic = SAHPI_TRUE,
					.Factors = { 
						.M_Factor = 1, 
						.B_Factor = 0, 
						.AccuracyFactor = 0,
						.ToleranceFactor = 0,
						.ExpA = 0,
						.ExpR = 0,
						.ExpB = 0,
						.Linearization = SAHPI_SL_LINEAR,
					},
					.Percentage = SAHPI_FALSE,
					.Range = {
						.Flags = SAHPI_SRF_MIN | SAHPI_SRF_MAX,
						.Max = {
							.ValuesPresent = SAHPI_SRF_RAW,
							.Raw = 200,
						},
						.Min = {
							.ValuesPresent = SAHPI_SRF_RAW,
							.Raw = 0,
						}
					},
				},
				.ThresholdDefn = {
					.IsThreshold = SAHPI_TRUE,
					.TholdCapabilities=SAHPI_STC_RAW,
					.ReadThold = SAHPI_STM_LOW_CRIT,
					.WriteThold = SAHPI_STM_LOW_CRIT,
				},
				.Oem = 0,
			},
		},
		.IdString = {
			.DataType	= SAHPI_TL_TYPE_ASCII6,
			.Language	= SAHPI_LANG_ENGLISH,
			.DataLength 	= 23,
			.Data		= "Dummy-Power-Temperature"
		},
	},
	
	/* third rdr */
	/* on the first system board */
	{
		.RdrType  = SAHPI_CTRL_RDR,
		.Entity   = {
			.Entry = {
				{SAHPI_ENT_SYSTEM_BOARD, 1},
				{0, 0}
			},
		},
		.RdrTypeUnion = {
			.CtrlRec = {
				.Num = 5,
				.Ignore = SAHPI_FALSE,
				.OutputType = SAHPI_CTRL_LED,
				.Type = SAHPI_CTRL_TYPE_DIGITAL,
				.TypeUnion = {
					.Digital = {
						.Default = SAHPI_CTRL_STATE_OFF,
					},
					.Oem = {
						.MId = 0xff,
						.ConfigData = "BOGUS CFG",
						.Default = {
							.MId = 0xff,
							.BodyLength = 56,
							.Body = "Bogus Data for CTRL on {SAHPI_ENT_SYSTEM_BOARD, 1}{,}{,}",
						},
					},
				},
			},
		},
		.IdString = {
			.DataType	= SAHPI_TL_TYPE_ASCII6,
			.Language	= SAHPI_LANG_ENGLISH,
			.DataLength 	= 17,
			.Data		= "Digital-Control-1"
		},
	},
	/* fourth rdr */
	/* This is an RDR representing a inventory */
	/* on the first system board */
	{
		.RdrType  = SAHPI_INVENTORY_RDR,
		.Entity   = {
			.Entry = {
				{SAHPI_ENT_SYSTEM_BOARD, 1},
				{0, 0}
			},
		},
		.RdrTypeUnion = {
			.InventoryRec = {
				.EirId	= 6,
				.Oem	= 0x12344321,
			},
		},
		.IdString = {
			.DataType	= SAHPI_TL_TYPE_ASCII6,
			.Language	= SAHPI_LANG_ENGLISH,
			.DataLength 	= 18,
			.Data		= "System-Inventory-1"
		},
	},
	/* fifth rdr */
	/* This is the one temperature sensor on System Board */
	{
		.RecordId = 0, /*no use, should be rewritten by upper layer */
		.RdrType  = SAHPI_SENSOR_RDR,
		.Entity   = {
			.Entry = {
				{SAHPI_ENT_SYSTEM_BOARD, 1},
				{0, 0}
			},
		},
		.RdrTypeUnion = {
			.SensorRec = {
				.Num  = 3, 
				.Type = SAHPI_TEMPERATURE,
				.Category = SAHPI_EC_THRESHOLD,
				.EventCtrl = SAHPI_SEC_PER_EVENT,
				.Events	= SAHPI_ES_UPPER_MINOR | SAHPI_ES_UPPER_MAJOR | SAHPI_ES_UPPER_CRIT,
				.Ignore = SAHPI_FALSE,
				.DataFormat = {
					.ReadingFormats = SAHPI_SRF_RAW,
					.IsNumeric = SAHPI_TRUE,
					.SignFormat = SAHPI_SDF_UNSIGNED,
					.BaseUnits = SAHPI_SU_DEGREES_C,
					.ModifierUnits = SAHPI_SU_UNSPECIFIED,
					.ModifierUse = SAHPI_SMUU_NONE,
					.FactorsStatic = SAHPI_TRUE,
					.Factors = { 
						.M_Factor = 1, 
						.B_Factor = 0, 
						.AccuracyFactor = 0,
						.ToleranceFactor = 0,
						.ExpA = 0,
						.ExpR = 0,
						.ExpB = 0,
						.Linearization = SAHPI_SL_LINEAR,
					},
					.Percentage = SAHPI_FALSE,
					.Range = {
						.Flags = SAHPI_SRF_MIN
						| SAHPI_SRF_MAX,
						.Max = {
							.ValuesPresent = SAHPI_SRF_RAW,
							.Raw = 200,
						},
						.Min = {
							.ValuesPresent = SAHPI_SRF_RAW,
							.Raw = 0,
						}
					},
				},
				.ThresholdDefn = {
					.IsThreshold = SAHPI_TRUE,
					.TholdCapabilities=SAHPI_STC_RAW,
					.ReadThold = SAHPI_STM_LOW_CRIT,
					.WriteThold = SAHPI_STM_LOW_CRIT,
				},
				.Oem = 0,
			},
		},
		.IdString = {
			.DataType	= SAHPI_TL_TYPE_ASCII6,
			.Language	= SAHPI_LANG_ENGLISH,
			.DataLength 	= 24,
			.Data		= "Dummy-System-Temperature"
		},
	},
	/* sixth rdr */
	/* This is the one temperature sensor on Power Supply */
	{
		.RecordId = 0, /*no use,
				 should be rewritten by upper layer */
		.RdrType  = SAHPI_SENSOR_RDR,
		.Entity   = {
			.Entry = {
				{SAHPI_ENT_POWER_SUPPLY, 1},
				{0, 0}
			},
		},
		.RdrTypeUnion = {
			.SensorRec = {
				.Num  = 4, /*no use!
					     should be rewritten by upper layer */
				.Type = SAHPI_TEMPERATURE,
				.Category = SAHPI_EC_THRESHOLD,
				.EventCtrl = SAHPI_SEC_PER_EVENT,
				.Events	= SAHPI_ES_UPPER_MINOR
				| SAHPI_ES_UPPER_MAJOR
				| SAHPI_ES_UPPER_CRIT,
				.Ignore = SAHPI_FALSE,
				.DataFormat = {
					.ReadingFormats = SAHPI_SRF_RAW,
					.IsNumeric = SAHPI_TRUE,
					.SignFormat = SAHPI_SDF_UNSIGNED,
					.BaseUnits = SAHPI_SU_DEGREES_C,
					.ModifierUnits = SAHPI_SU_UNSPECIFIED,
					.ModifierUse = SAHPI_SMUU_NONE,
					.FactorsStatic = SAHPI_TRUE,
					.Factors = { 
						.M_Factor = 1, 
						.B_Factor = 0, 
						.AccuracyFactor = 0,
						.ToleranceFactor = 0,
						.ExpA = 0,
						.ExpR = 0,
						.ExpB = 0,
						.Linearization = SAHPI_SL_LINEAR,
					},
					.Percentage = SAHPI_FALSE,
					.Range = {
						.Flags = SAHPI_SRF_MIN
						| SAHPI_SRF_MAX,
						.Max = {
							.ValuesPresent = SAHPI_SRF_RAW,
							.Raw = 200,
						},
						.Min = {
							.ValuesPresent = SAHPI_SRF_RAW,
							.Raw = 0,
						}
					},
				},
				.ThresholdDefn = {
					.IsThreshold = SAHPI_TRUE,
					.TholdCapabilities=SAHPI_STC_RAW,
					.ReadThold = SAHPI_STM_LOW_CRIT,
					.WriteThold = SAHPI_STM_LOW_CRIT,
				},
				.Oem = 0,
			},
		},
		.IdString = {
			.DataType	= SAHPI_TL_TYPE_ASCII6,
			.Language	= SAHPI_LANG_ENGLISH,
			.DataLength 	= 23,
			.Data		= "Dummy-Power-Temperature"
		},
	},
	/* seventh rdr */
	/* This is an RDR representing a digital control */
	/* on the first system board */
	{
		.RdrType  = SAHPI_CTRL_RDR,
		.Entity   = {
			.Entry = {
				{SAHPI_ENT_SYSTEM_BOARD, 1},
				{0, 0}
			},
		},
		.RdrTypeUnion = {
			.CtrlRec = {
				.Num = 7,
				.Ignore = SAHPI_FALSE,
				.OutputType = SAHPI_CTRL_LED,
				.Type = SAHPI_CTRL_TYPE_DIGITAL,
				.TypeUnion = {
					.Digital = {
						.Default = SAHPI_CTRL_STATE_OFF,
					},
				},
			},
		},
		.IdString = {
			.DataType	= SAHPI_TL_TYPE_ASCII6,
			.Language	= SAHPI_LANG_ENGLISH,
			.DataLength 	= 17,
			.Data		= "Digital-Control-1"
		},
	},
	/* eigth rdr */
	/* This is an RDR representing a inventory */
	/* on the first system board */
	{
		.RdrType  = SAHPI_INVENTORY_RDR,
		.Entity   = {
			.Entry = {
				{SAHPI_ENT_SYSTEM_BOARD, 1},
				{0, 0}
			},
		},
		.RdrTypeUnion = {
			.InventoryRec = {
				.EirId	= 8,
				.Oem	= 0x12344321,
			},
		},
		.IdString = {
			.DataType	= SAHPI_TL_TYPE_ASCII6,
			.Language	= SAHPI_LANG_ENGLISH,
			.DataLength 	= 18,
			.Data		= "System-Inventory-1"
		},
	},

};


/* SaHpiCtrlStateT */
static SaHpiCtrlStateT dummy_controls = {
	.Type = SAHPI_CTRL_TYPE_DIGITAL,
	.StateUnion = {
		.Digital = SAHPI_CTRL_STATE_OFF,
	},
};
/************************************************************************/
/* sensor data 								*/
/************************************************************************/
static struct dummy_sensor {
	SaHpiSensorReadingT reading;
	SaHpiSensorThresholdsT thresholds;
	SaHpiSensorEvtEnablesT enables;
} dummy_sensors[]= {
	{
	/*This is temp sensor on system board*/
		.reading = {
			.ValuesPresent = SAHPI_SRF_RAW,
			.Raw = 0x01,
		},
		.thresholds = {
			.LowCritical = {
				.ValuesPresent = SAHPI_SRF_RAW,
			},
			.LowMajor = {
				.ValuesPresent = SAHPI_SRF_RAW,
			},
			.LowMinor = {
				.ValuesPresent = SAHPI_SRF_RAW,
			},
			.UpCritical = {
				.ValuesPresent = SAHPI_SRF_RAW,
			},
			.UpMajor = {
				.ValuesPresent = SAHPI_SRF_RAW,
			},
			.UpMinor = {
				.ValuesPresent = SAHPI_SRF_RAW,
			},

		}
	},
	{
	/*This is temp sensor on system board*/
		.reading = {
			.ValuesPresent = SAHPI_SRF_RAW,
			.Raw = 0x01,
		},
		.thresholds = {
			.LowCritical = {
				.ValuesPresent = SAHPI_SRF_RAW,
			},
			.LowMajor = {
				.ValuesPresent = SAHPI_SRF_RAW,
			},
			.LowMinor = {
				.ValuesPresent = SAHPI_SRF_RAW,
			},
			.UpCritical = {
				.ValuesPresent = SAHPI_SRF_RAW,
			},
			.UpMajor = {
				.ValuesPresent = SAHPI_SRF_RAW,
			},
			.UpMinor = {
				.ValuesPresent = SAHPI_SRF_RAW,
			},
			.PosThdHysteresis = {
				.ValuesPresent = SAHPI_SRF_RAW,
			},
			.NegThdHysteresis = {
				.ValuesPresent = SAHPI_SRF_RAW,
			},

		}
	},
	{
	/*This is temp sensor on system board*/
		.reading = {
			.ValuesPresent = SAHPI_SRF_RAW,
			.Raw = 0x01,
		},
		.thresholds = {
			.LowCritical = {
				.ValuesPresent = SAHPI_SRF_RAW,
			},
			.LowMajor = {
				.ValuesPresent = SAHPI_SRF_RAW,
			},
			.LowMinor = {
				.ValuesPresent = SAHPI_SRF_RAW,
			},
			.UpCritical = {
				.ValuesPresent = SAHPI_SRF_RAW,
			},
			.UpMajor = {
				.ValuesPresent = SAHPI_SRF_RAW,
			},
			.UpMinor = {
				.ValuesPresent = SAHPI_SRF_RAW,
			},
			.PosThdHysteresis = {
				.ValuesPresent = SAHPI_SRF_RAW,
			},
			.NegThdHysteresis = {
				.ValuesPresent = SAHPI_SRF_RAW,
			},


		}
	},
	{
	/*This is temp sensor on system board*/
		.reading = {
			.ValuesPresent = SAHPI_SRF_RAW,
			.Raw = 0x01,
		},
		.thresholds = {
			.LowCritical = {
				.ValuesPresent = SAHPI_SRF_RAW,
			},
			.LowMajor = {
				.ValuesPresent = SAHPI_SRF_RAW,
			},
			.LowMinor = {
				.ValuesPresent = SAHPI_SRF_RAW,
			},
			.UpCritical = {
				.ValuesPresent = SAHPI_SRF_RAW,
			},
			.UpMajor = {
				.ValuesPresent = SAHPI_SRF_RAW,
			},
			.UpMinor = {
				.ValuesPresent = SAHPI_SRF_RAW,
			},
			.PosThdHysteresis = {
				.ValuesPresent = SAHPI_SRF_RAW,
			},
			.NegThdHysteresis = {
				.ValuesPresent = SAHPI_SRF_RAW,
			},

		}
	},     
};

/* resource one inventory data */
static struct dummy_inventories {
	SaHpiInventDataValidityT Validity;
	SaHpiInventDataRecordT *DataRecords[2];
	char data[512];
} dummy_inventory = {
		.Validity	= SAHPI_INVENT_DATA_INVALID,
		.DataRecords	= {
			NULL,
			NULL,
		},
 };


static struct dummy_rsel {
	SaHpiSelEntryT entry;
} rsel_entries = {
	.entry = {
		.EntryId = 0,
		.Timestamp = 0,
		.Event = {
			.Source = 0,
			.EventType = SAHPI_ET_HOTSWAP,
			.Timestamp = 0,
			.Severity = SAHPI_CRITICAL,
			.EventDataUnion = {
				.HotSwapEvent = {
					.HotSwapState = SAHPI_HS_STATE_INSERTION_PENDING,
					.PreviousHotSwapState = SAHPI_HS_STATE_NOT_PRESENT,
				},
			},
		},
	},
};



/* The event is used to fill event list */
static struct oh_event dummy_user_event = {
		.type = OH_ET_HPI,
		.u = {
			.hpi_event = {
				.parent = 0,
				.id = 0,
				.event = {
					.Source = 0,
					.EventType = SAHPI_ET_USER,
					.Timestamp = 0,
					.Severity = SAHPI_CRITICAL,
				},
			},
		},
};

#if 0
/* The sensor event is used */
static struct oh_event dummy_user_event = {
		.type = OH_ET_RESOURCE_DEL,
		.u = {
			.res_event = {
				.parent = 0,
				.domain = 0,
				.entry = {
					.Source = 0,
					.EventType = SAHPI_ET_USER,
					.Timestamp = 0,
					.Severity = SAHPI_CRITICAL,
				},
			},
		},
};
#endif
  
static struct oh_event *__eventdup(const struct oh_event *event)
{
	struct oh_event *e;
	e = g_malloc0(sizeof(*e));
	if (!e) {
		dbg("Out of memory!");
		return NULL;
	}
	memcpy(e, event, sizeof(*e));
	return e;
}


static int __build_the_rpt_cache(struct oh_handler_state *oh_hnd)
{
	int i;
	int id;
	SaHpiRptEntryT res;
	SaHpiRdrT res_rdr;

	SaHpiEntityPathT root_ep;
	char *entity_root;

	entity_root = (char *)g_hash_table_lookup(oh_hnd->config,"entity_root");

        string2entitypath(entity_root, &root_ep);
	append_root(&root_ep);

	/* append entity root to resource entity paths */
	for (i=0; i < NUM_RESOURCES; i++) {
		memcpy(&res, &dummy_resources[i], sizeof(SaHpiRptEntryT));

		ep_concat( &res.ResourceEntity, &root_ep);
		
		res.ResourceId = oh_uid_from_entity_path(&res.ResourceEntity);

		/* add the resource */
		if (oh_add_resource(oh_hnd->rptcache, &res, NULL, 0))
			dbg("oh_add_resource failed for resource %d", i);

		/* save the resource id for tracking resource status */
		dummy_resource_status[i].ResourceId = res.ResourceId;

	}
	/* append entity root to rdrs entity paths */
	for (i=0; i < NUM_RDRS; i++) {
		memcpy(&res_rdr, &dummy_rdrs[i], sizeof(SaHpiRdrT));

		ep_concat( &res_rdr.Entity, &root_ep);

		id = oh_uid_lookup(&res_rdr.Entity);

		if( id < 0 ) { dbg("error looking up uid in dummy_open");  return(-1); }

		/* add rdrs */
		if (oh_add_rdr(oh_hnd->rptcache, id, &res_rdr, NULL, 0))
			dbg("oh_add_resource failed for rdr %d", i);
	}

	return(0);
}

static void *dummy_open(GHashTable *handler_config)
{
        struct oh_handler_state *i;
	char *tok;

	if (!handler_config) {
		dbg("GHashTable *handler_config is NULL!");
		return(NULL);
	}

        trace("%s, %s, %s",
	      (char *)g_hash_table_lookup(handler_config, "plugin"),
	      (char *)g_hash_table_lookup(handler_config, "name"),
	      tok = g_hash_table_lookup(handler_config, "entity_root"));        

        if (!tok) {
                dbg("entity_root is needed and not present");
                return(NULL);
        }
              
        i = malloc(sizeof(*i));
	if (!i) {
		dbg("out of memory");
		return( (struct oh_handler_state *)NULL );
	}
	memset(i, 0, sizeof(*i));

	/* save the handler config has table it holds 	*/
	/* the openhpi.conf file config info 		*/
	i->config = handler_config;

	/* initialize hashtable pointer */
	i->rptcache = (RPTable *)g_malloc0(sizeof(RPTable)); 

	/* fill in the local rpt cache */
	__build_the_rpt_cache(i);
	/* associate the static hotswap_event data with a resource */
	hotswap_event[0].u.hpi_event.parent = dummy_resources[1].ResourceId;
	hotswap_event[1].u.hpi_event.parent = dummy_resources[1].ResourceId;
	
	return( (void *)i );
}

static void dummy_close(void *hnd)
{
	struct oh_handler_state *inst = hnd;
	/* TODO: free the rptcache RPTable *rptcache */

	/* free eventq entries GSList *eventq */
	while ( g_slist_length( inst->eventq ) > 0 ) {
		free(inst->eventq->data);
		inst->eventq = g_slist_remove_link(inst->eventq, inst->eventq);
	}

	/* TODO: free the GHashTabel GHashTable *config */

	/* TODO: free *data void *data */
			            
	free(hnd);

	return;
}

static struct oh_event *remove_resource(struct oh_handler_state *inst)
{  
	SaHpiRptEntryT *rpt_e = NULL;
	SaHpiRptEntryT *rpt_e_pre = NULL;

	static struct oh_event e;

	memset(&e, 0, sizeof(e));

	/* get the last resouce in our rptache 	*/
	/* since we know it has no rdrs		*/
	rpt_e = oh_get_resource_next(inst->rptcache, RPT_ENTRY_BEGIN);
	while(rpt_e) {
		rpt_e_pre = rpt_e;
		rpt_e = oh_get_resource_next(inst->rptcache, rpt_e->ResourceId);
	}

	/* generate remove resource event */
	if(rpt_e_pre) {

		e.type = OH_ET_RESOURCE_DEL;
		e.u.res_del_event.resource_id = rpt_e_pre->ResourceId;
//		memcpy(&e.u.res_event.entry, rpt_e_pre, sizeof(SaHpiRptEntryT));       	
	}

dbg("**** ReourceId %d ******", e.u.res_event.entry.ResourceId);

	return(&e);
	
}

static struct oh_event *add_resource(struct oh_handler_state *inst)
{
	SaHpiRptEntryT *rpt_e = NULL;
	SaHpiRptEntryT *rpt_e_pre = NULL;

	static struct oh_event e;

	memset(&e,0,sizeof(e));


	/* get the last resouce in our rptache 	*/
	/* since we know it has no rdrs		*/
	rpt_e = oh_get_resource_next(inst->rptcache, RPT_ENTRY_BEGIN);
	while(rpt_e) {
		rpt_e_pre = rpt_e;
		rpt_e = oh_get_resource_next(inst->rptcache, rpt_e->ResourceId);
	}

	/* generate remove resource event */
	if(rpt_e_pre) {
		e.type = OH_ET_RESOURCE;
		memcpy(&e.u.res_event.entry, rpt_e_pre, sizeof(SaHpiRptEntryT));  
	}

dbg("**** ReourceId %d ******", e.u.res_event.entry.ResourceId);

	return(&e);
	
}


static int dummy_get_event(void *hnd, struct oh_event *event, struct timeval *timeout)
{
	struct oh_handler_state *inst = hnd;

	struct oh_event *e = NULL;
	
	SaHpiRptEntryT *rpt_entry = NULL;

	static unsigned int count = 0;
	static unsigned int toggle = 0;
  
	if (g_slist_length(inst->eventq)>0) {
		memcpy(event, inst->eventq->data, sizeof(*event));
		free(inst->eventq->data);
		inst->eventq = g_slist_remove_link(inst->eventq, inst->eventq);
		return(1);
	} else if (count == 0) {

		count++;

		/* just stick this user event on the first resource */
		rpt_entry = oh_get_resource_next(inst->rptcache, RPT_ENTRY_BEGIN);
		if (!rpt_entry) {
			g_free(e);
			return(-1);
		}

		*event = dummy_user_event;
		event->u.hpi_event.parent = rpt_entry->ResourceId;
  
		return(1);

	} else if (count == 1) {
		count++;
		return(-1);
	}

	toggle++;
	if( (toggle%3) == 0 ) {
		/* once initial reporting of events toggle between 	*/
		/* removing and adding resource, removes resource 3	*/
		/* since it has no rdr's to add back later		*/
		if ( (count%2) == 0 ) {
			count++;
			dbg("\n**** EVEN ****, remove the resource\n");
			if ( (e = remove_resource(inst)) ) {
				*event = *e;
				return(1);
			}
		} else {
			count++;
			dbg("\n**** ODD ****, add the resource\n");
			if ( (e = add_resource(inst)) )	{
				*event = *e;
				return(1);
			}
		}
	}

	return(-1);


}

static int dummy_discover_resources(void *hnd)
{
	static int done_once = FALSE;
	struct oh_handler_state *inst = hnd;
	struct oh_event event;
	SaHpiRptEntryT *rpt_entry;
	SaHpiRdrT      *rdr_entry;

	/* create a counter, on even remove your favorite resource
	   on odd add it back */
	if (!done_once) {

		/* get the first rpt entry */

		rpt_entry = oh_get_resource_next(inst->rptcache, RPT_ENTRY_BEGIN);

		while (rpt_entry) {
//dbg("here resource event id %d", rpt_entry->ResourceId);
			memset(&event, 0, sizeof(event));
			event.type = OH_ET_RESOURCE;
			memcpy(&event.u.res_event.entry, rpt_entry, sizeof(SaHpiRptEntryT));
			inst->eventq = g_slist_append(inst->eventq, __eventdup(&event) );


			/* get every resource rdr's */
			rdr_entry = oh_get_rdr_next(inst->rptcache, 
						    rpt_entry->ResourceId, RDR_BEGIN);
			while (rdr_entry) {
//dbg("here rdr event id %d", rdr_entry->RecordId);
				memset(&event, 0, sizeof(event));
				event.type = OH_ET_RDR;
				memcpy(&event.u.rdr_event.rdr, rdr_entry, sizeof(SaHpiRdrT));
				inst->eventq = g_slist_append(inst->eventq, __eventdup(&event));
				rdr_entry = oh_get_rdr_next(inst->rptcache, 
							    rpt_entry->ResourceId, rdr_entry->RecordId);
			}
			/* get any resource rdr's end */


			rpt_entry = oh_get_resource_next(inst->rptcache, rpt_entry->ResourceId);
		}
		
		done_once = TRUE;
	}
	
	return 0;
}

static int dummy_get_self_id(void *hnd, SaHpiResourceIdT id)
{
	return -1;
}

static int sel_enabled;

static int dummy_get_sel_info(void *hnd, SaHpiResourceIdT id, SaHpiSelInfoT *info)
{
	info->Entries			= 0;
	info->Size			= 0xFFFFFFFF;
	info->UpdateTimestamp		= 0;
	gettimeofday1(&info->CurrentTime);
	info->Enabled			= sel_enabled;
	info->OverflowFlag		= 0;
	info->OverflowAction		= SAHPI_SEL_OVERFLOW_DROP;
	info->DeleteEntrySupported 	= 1;

	return 0;
}

static int dummy_set_sel_time(void *hnd, SaHpiResourceIdT id, SaHpiTimeT time)
{
	return 0;
}

static int dummy_add_sel_entry(void *hnd, SaHpiResourceIdT id, const SaHpiSelEntryT *Event)
{
dbg("TODO: dummy_add_sel_entry(), need to set res based on id");

#if 0
	struct dummy_instance *inst = hnd;
	struct dummy_rsel *rsel;
	struct oh_event *e;

	e = malloc(sizeof(*e));
	if (!e) {
		dbg("Out of memory");
		return -1;
	}

	rsel = malloc(sizeof(*rsel));
	if (!rsel) {
		dbg("Out of memory");
		return -1;
	}

	memcpy(&rsel->entry, Event, sizeof(*Event));
	memset(e, 0, sizeof(*e));
	e->type = OH_ET_RSEL;
dbg("TODO: need to set res based on id");
//	e->u.rsel_event.rsel.parent = id;

	e->u.rsel_event.rsel.oid.ptr = rsel;
	
	inst->eventq = g_slist_append(inst->eventq, e);
#endif
	return 0;
}

static int dummy_del_sel_entry(void *hnd, SaHpiResourceIdT id, SaHpiSelEntryIdT sid)
{
dbg("TODO: dummy_del_sel_entry() need to set rsel based on id");
//	struct dummy_rsel *rsel = id.ptr;
//	free(rsel);
	return 0;
}

static int dummy_get_sel_entry(void *hnd, 
			       SaHpiResourceIdT id, 
			       SaHpiSelEntryIdT current,
			       SaHpiSelEntryIdT *prev, 
			       SaHpiSelEntryIdT *next, 
			       SaHpiSelEntryT *entry)
{       
dbg("dummy_get_sel_entry(): This is a very bad implementation");
	*prev = SAHPI_NO_MORE_ENTRIES;
	*next = SAHPI_NO_MORE_ENTRIES;
	memcpy(entry, &rsel_entries.entry, sizeof(SaHpiSelEntryT));

	return 0;
}

/************************************************************************/
/* Begin: Sensor functions 						*/
/************************************************************************/
static int dummy_get_sensor_data(void *hnd, SaHpiResourceIdT id, 
                           SaHpiSensorNumT num,
                           SaHpiSensorReadingT *data)
{
	int rval = -1;
	struct oh_handler_state *inst = hnd;
	SaHpiRdrT *rdr;
	
	/* get any resource rdr's */
	rdr = oh_get_rdr_next(inst->rptcache, id, RDR_BEGIN);

	while ( (rdr->RdrTypeUnion.CtrlRec.Num != num) && rdr) {
		rdr = oh_get_rdr_next(inst->rptcache, id, rdr->RecordId);
	}

	if (rdr) {
		memcpy(data, 			
		       &dummy_sensors[rdr->RdrTypeUnion.SensorRec.Num - 1].reading, 
		       sizeof(SaHpiSensorReadingT));
		rval = 0;
	}

	return(rval);
}

static int dummy_get_sensor_thresholds(void *hnd, SaHpiResourceIdT id,
				       SaHpiSensorNumT num,
				       SaHpiSensorThresholdsT *thres)
{
	memcpy(thres, &dummy_sensors[num - 1].thresholds, sizeof(SaHpiSensorThresholdsT));     

	return 0;
}


static int reading_has_values(const SaHpiSensorReadingT *reading)
{
	if ((reading->ValuesPresent & SAHPI_SRF_RAW)
	    || (reading->ValuesPresent & SAHPI_SRF_INTERPRETED))
		return 1;
	return 0;
}

static void reading_cpy(SaHpiSensorReadingT *r1, 
			       const SaHpiSensorReadingT *r2)
{
	if (reading_has_values(r2))
		memcpy(r1, r2, sizeof(*r1));
}

static int dummy_set_sensor_thresholds(void *hnd, SaHpiResourceIdT id,
				       SaHpiSensorNumT num,
				       const SaHpiSensorThresholdsT *thres)
{	
	reading_cpy(&dummy_sensors[num - 1].thresholds.LowCritical, 
		    &thres->LowCritical);

	reading_cpy(&dummy_sensors[num - 1].thresholds.LowMajor, 
		    &thres->LowMajor);
	
	reading_cpy(&dummy_sensors[num - 1].thresholds.LowMinor, 
		    &thres->LowMinor);
	
	reading_cpy(&dummy_sensors[num - 1].thresholds.UpCritical, 
		    &thres->UpCritical);
	
	reading_cpy(&dummy_sensors[num - 1].thresholds.UpMajor, 
		    &thres->UpMajor);
	
	reading_cpy(&dummy_sensors[num - 1].thresholds.UpMinor, 
		    &thres->UpMinor);
	
	reading_cpy(&dummy_sensors[num - 1].thresholds.PosThdHysteresis, 
		    &thres->PosThdHysteresis);
	
	reading_cpy(&dummy_sensors[num - 1].thresholds.NegThdHysteresis, 
		    &thres->NegThdHysteresis);

	return 0;
}

static int dummy_get_sensor_event_enables(void *hnd, SaHpiResourceIdT id,
					  SaHpiSensorNumT num,
					  SaHpiSensorEvtEnablesT *enables)
{
	memcpy(enables, &dummy_sensors[num - 1].enables, sizeof(*enables));

	return 0;
}

static int dummy_set_sensor_event_enables(void *hnd, SaHpiResourceIdT id,
					  SaHpiSensorNumT num,
					  const SaHpiSensorEvtEnablesT *enables)
{			   
	memcpy(&dummy_sensors[num - 1].enables, enables, sizeof(*enables));

	return 0;
}
/************************************************************************/
/* End: Sensor functions 						*/
/************************************************************************/

static int dummy_get_control_state(void *hnd, SaHpiResourceIdT id,
				   SaHpiCtrlNumT num,
				   SaHpiCtrlStateT *state)
{
	struct oh_handler_state *inst = hnd;
	SaHpiRdrT *rdr;

	/* yes, nothing is done with rdr, but 		*/
	/* in the real world you would need to correlate	*/
	/* this rdr with the appropriate data			*/
	rdr = oh_get_rdr_by_type(inst->rptcache, id, SAHPI_CTRL_RDR, num);

	state->StateUnion = dummy_controls.StateUnion;
	state->Type = dummy_controls.Type;

	return 0;
}

static int dummy_set_control_state(void *hnd, SaHpiResourceIdT id,
				   SaHpiCtrlNumT num,
				   SaHpiCtrlStateT *state)
{
dbg("TODO: dummy_set_control_state(), need to set control based on id");
//	struct dummy_control *control = id.ptr;
//	memcpy(&control->state,state,sizeof(SaHpiCtrlStateT));
			
	return 0;
}

/************************************************************************/
/* Begin: Inventory functions 						*/
/************************************************************************/
static int dummy_get_inventory_size(void *hnd, SaHpiResourceIdT id,
				    SaHpiEirIdT num,
				    SaHpiUint32T *size)
{
	int rval = -1;
	int i = 0;
	int done = FALSE;

	done = FALSE;
	i = 0;
	do {
		if(dummy_resource_status[i].ResourceId == id) {
			done = TRUE;
		}
		else  {
			i++;
		}

	} while ( (i < NUM_RESOURCES) && !done  );

	if ( (i < NUM_RESOURCES) && 
	     (dummy_resources[i].ResourceCapabilities & SAHPI_CAPABILITY_INVENTORY_DATA) ) {
	       	*size = sizeof(dummy_inventory);
		rval = 0;
	}

	return(rval);
}

static int dummy_get_inventory_info(void *hnd, SaHpiResourceIdT id,
				    SaHpiEirIdT num,
				    SaHpiInventoryDataT *data)
{
	struct dummy_inventories *inventory = &dummy_inventory;
	int i;
	int pos;

	int rval = -1;
	int done = FALSE;

	done = FALSE;
	i = 0;
	do {
		if(dummy_resource_status[i].ResourceId == id) {
			done = TRUE;
		}
		else  {
			i++;
		}

	} while ( (i < NUM_RESOURCES) && !done  );

	if ( (i < NUM_RESOURCES) && 
	     (dummy_resources[i].ResourceCapabilities & SAHPI_CAPABILITY_INVENTORY_DATA) ) {
		rval = 0;

		data->Validity = inventory->Validity;
		memcpy(data, inventory, sizeof(*inventory));

		for (i=0,pos=0; inventory->DataRecords[i]!=NULL; i++) {
			data->DataRecords[i] = (SaHpiInventDataRecordT *)&inventory->data[pos];
			pos+=inventory->DataRecords[i]->DataLength+8;
		}

	}

	return(rval);
}

static int dummy_set_inventory_info(void *hnd, SaHpiResourceIdT id,
				    SaHpiEirIdT num,
				    const SaHpiInventoryDataT *data)
{

	struct dummy_inventories *inventory = &dummy_inventory;
	int i;
	int pos;

	int rval = -1;
	int done = FALSE;

	done = FALSE;
	i = 0;
	do {
		if(dummy_resource_status[i].ResourceId == id) {
			done = TRUE;
		}
		else  {
			i++;
		}

	} while ( (i < NUM_RESOURCES) && !done  );


	if ( (i < NUM_RESOURCES) && 
	     (dummy_resources[i].ResourceCapabilities & SAHPI_CAPABILITY_INVENTORY_DATA) ) {
		rval = 0;
	
		inventory->Validity = data->Validity;
	
		for (i=0, pos=0; data->DataRecords[i] != NULL; i++) {
			inventory->DataRecords[i] = (SaHpiInventDataRecordT *)&inventory->data[pos];
			memcpy(inventory->DataRecords[i], data->DataRecords[i], data->DataRecords[i]->DataLength+8);
			pos+=data->DataRecords[i]->DataLength+8;
		}
		inventory->DataRecords[i] = NULL;
	}

	return(rval);
}
/************************************************************************/
/* End: Inventory functions 						*/
/************************************************************************/


static int dummy_get_watchdog_info(void *hnd, SaHpiResourceIdT id,
				   SaHpiWatchdogNumT num,
				   SaHpiWatchdogT *wdt)
{
	return -1;
}

static int dummy_set_watchdog_info(void *hnd, SaHpiResourceIdT id,
				   SaHpiWatchdogNumT num,
				   SaHpiWatchdogT *wdt)
{
	return -1;
}

static int dummy_reset_watchdog(void *hnd, SaHpiResourceIdT id,
				SaHpiWatchdogNumT num)
{
	return -1;
}

/************************************************************************/
/* Begin: Hotswap functions 						*/
/************************************************************************/
static int dummy_get_hotswap_state(void *hnd, SaHpiResourceIdT id, 
				   SaHpiHsStateT *state)
{
	int rval = -1;
	int i = 0;
	int done = FALSE;

	done = FALSE;
	i = 0;
	do {
		if(dummy_resource_status[i].ResourceId == id) {
			done = TRUE;
		}
		else  {
			i++;
		}

	} while ( (i < NUM_RESOURCES) && !done  );

	if ( i < NUM_RESOURCES ) {
		*state = dummy_resource_status[i].hotswap;  
		rval = 0;
	}

	return(rval);
}

static int dummy_set_hotswap_state(void *hnd, SaHpiResourceIdT id, 
				   SaHpiHsStateT state)
{
	int rval = -1;
	int i = 0;
	int done = FALSE;


	done = FALSE;
	i = 0;
	do {
		if(dummy_resource_status[i].ResourceId == id) {
			done = TRUE;
		}
		else  {
			i++;
		}

	} while ( (i < NUM_RESOURCES) && !done  );
		
	if ( i < NUM_RESOURCES ) {
		dummy_resource_status[i].hotswap = state;   
		rval = 0;
	}


	return(rval);

}

static int dummy_request_hotswap_action(void *hnd, SaHpiResourceIdT id, 
					SaHpiHsActionT act)
{
	struct oh_handler_state *inst = hnd;
//	SaHpiRptEntryT *e = NULL;

	int rval = -1;
	int i = 0;
	int done = FALSE;

	done = FALSE;
	i = 0;
	do {
		if(dummy_resource_status[i].ResourceId == id) {
			done = TRUE;
		}
		else  {
			i++;
		}

	} while ( (i < NUM_RESOURCES) && !done  );

	if ( i < NUM_RESOURCES ) {
		rval = 0;
 	}
	else {
		dbg("The resource does not hotswapable!");
		rval = -1;
	}


	if (!rval && act == SAHPI_HS_ACTION_INSERTION) {

		if (dummy_resource_status[1].hotswap == SAHPI_HS_STATE_INACTIVE) {
			dummy_resource_status[1].hotswap = SAHPI_HS_STATE_INSERTION_PENDING;
			hotswap_event[0].u.hpi_event.parent = id;
			inst->eventq = g_slist_append(inst->eventq, __eventdup(&hotswap_event[0]));
		} else {
			dbg("Cannot instertion");
			rval = -1;
		}

	} else if (!rval && act == SAHPI_HS_ACTION_EXTRACTION) {
		
		if (dummy_resource_status[1].hotswap == SAHPI_HS_STATE_ACTIVE_HEALTHY 
		    || dummy_resource_status[1].hotswap == SAHPI_HS_STATE_ACTIVE_UNHEALTHY) {
			dummy_resource_status[1].hotswap = SAHPI_HS_STATE_EXTRACTION_PENDING;
			hotswap_event[1].u.hpi_event.parent = id;
			inst->eventq = g_slist_append(inst->eventq, __eventdup(&hotswap_event[1]));
		} else {
			dbg("Cannot extraction");
			rval = -1;
		}

	} else 
		rval = -1;
	
	return(rval);

}

static int dummy_get_power_state(void *hnd, SaHpiResourceIdT id, SaHpiHsPowerStateT *state)
{
	int rval = -1;
	int i = 0;
	int done = FALSE;

	done = FALSE;
	i = 0;
	do {
		if(dummy_resource_status[i].ResourceId == id) {
			done = TRUE;
		}
		else  {
			i++;
		}

	} while ( (i < NUM_RESOURCES) && !done  );

	if ( i < NUM_RESOURCES ) {
		*state = dummy_resource_status[i].power;  
		rval = 0;
	}

	return(rval);
}

static int dummy_set_power_state(void *hnd, SaHpiResourceIdT id, 
				 SaHpiHsPowerStateT state)
{
	int rval = -1;
	int i = 0;
	int done = FALSE;


	done = FALSE;
	i = 0;
	do {
		if(dummy_resource_status[i].ResourceId == id) {
			done = TRUE;
		}
		else  {
			i++;
		}

	} while ( (i < NUM_RESOURCES) && !done  );
		
	if ( i < NUM_RESOURCES ) {
		dummy_resource_status[i].power = state;   
		rval = 0;
	}


	return(rval);
}
	
static int dummy_get_indicator_state(void *hnd, SaHpiResourceIdT id, 
				     SaHpiHsIndicatorStateT *state)
{
	int rval = -1;
	int i = 0;
	int done = FALSE;

	done = FALSE;
	i = 0;
	do {
		if(dummy_resource_status[i].ResourceId == id) {
			done = TRUE;
		}
		else  {
			i++;
		}

	} while ( (i < NUM_RESOURCES) && !done  );

	if ( i < NUM_RESOURCES ) {
		*state = dummy_resource_status[i].indicator;  
		rval = 0;
	}

	return(rval);

}

static int dummy_set_indicator_state(void *hnd, SaHpiResourceIdT id, 
				     SaHpiHsIndicatorStateT state)
{
	int rval = -1;
	int i = 0;
	int done = FALSE;


	done = FALSE;
	i = 0;
	do {
		if(dummy_resource_status[i].ResourceId == id) {
			done = TRUE;
		}
		else  {
			i++;
		}

	} while ( (i < NUM_RESOURCES) && !done  );
		
	if ( i < NUM_RESOURCES ) {
		dummy_resource_status[i].indicator = state;   
		rval = 0;
	}


	return(rval);
}
/************************************************************************/
/* End: Hotswap functions 						*/
/************************************************************************/


static int dummy_control_parm(void *hnd, SaHpiResourceIdT id, 
			      SaHpiParmActionT act)
{
	return 0;
}

static int dummy_get_reset_state(void *hnd, SaHpiResourceIdT id, 
				 SaHpiResetActionT *act)
{
	int rval = -1;
	int i = 0;
	int done = FALSE;

	done = FALSE;
	i = 0;
	do {
		if(dummy_resource_status[i].ResourceId == id) {
			done = TRUE;
		}
		else  {
			i++;
		}

	} while ( (i < NUM_RESOURCES) && !done  );

	if ( i < NUM_RESOURCES ) {
		*act = dummy_resource_status[i].reset;  
		rval = 0;
	}

	return(rval);
}

static int dummy_set_reset_state(void *hnd, SaHpiResourceIdT id, 
				 SaHpiResetActionT act)
{
	int rval = -1;
	int i = 0;
	int done = FALSE;


	done = FALSE;
	i = 0;
	do {
		if(dummy_resource_status[i].ResourceId == id) {
			done = TRUE;
		}
		else  {
			i++;
		}

	} while ( (i < NUM_RESOURCES) && !done  );
		
	if ( i < NUM_RESOURCES ) {
		rval = 0;
		switch (act) {
		case SAHPI_COLD_RESET:
		case SAHPI_WARM_RESET:
			dummy_resource_status[i].reset = SAHPI_RESET_DEASSERT;
			break;
		case SAHPI_RESET_ASSERT:
			dummy_resource_status[i].reset = SAHPI_RESET_ASSERT;
			break;
		default:
			rval = -1;
		}
	}

	return(rval);
}

static struct oh_abi_v2 oh_dummy_plugin = {
	.open				= dummy_open,
	.close				= dummy_close,
	.get_event			= dummy_get_event,
	.discover_resources     	= dummy_discover_resources,
	.get_self_id			= dummy_get_self_id,
	.get_sel_info			= dummy_get_sel_info,
	.set_sel_time			= dummy_set_sel_time,
	.add_sel_entry			= dummy_add_sel_entry,
	.del_sel_entry			= dummy_del_sel_entry,
	.get_sel_entry			= dummy_get_sel_entry,
	.get_sensor_data		= dummy_get_sensor_data,
	.get_sensor_thresholds		= dummy_get_sensor_thresholds,
	.set_sensor_thresholds		= dummy_set_sensor_thresholds,
	.get_sensor_event_enables	= dummy_get_sensor_event_enables,
	.set_sensor_event_enables	= dummy_set_sensor_event_enables,
	.get_control_state		= dummy_get_control_state,
	.set_control_state		= dummy_set_control_state,
	.get_inventory_size	= dummy_get_inventory_size,
	.get_inventory_info	= dummy_get_inventory_info,
	.set_inventory_info	= dummy_set_inventory_info,
	.get_watchdog_info	= dummy_get_watchdog_info,
	.set_watchdog_info	= dummy_set_watchdog_info,
	.reset_watchdog		= dummy_reset_watchdog,
	.get_hotswap_state	= dummy_get_hotswap_state,
	.set_hotswap_state	= dummy_set_hotswap_state,
	.request_hotswap_action	= dummy_request_hotswap_action,
	.get_power_state	= dummy_get_power_state,
	.set_power_state	= dummy_set_power_state,
	.get_indicator_state	= dummy_get_indicator_state,
	.set_indicator_state	= dummy_set_indicator_state,
	.control_parm		= dummy_control_parm,
	.get_reset_state	= dummy_get_reset_state,
	.set_reset_state	= dummy_set_reset_state
};

int dummy_get_interface(void **pp, const uuid_t uuid);
int dummy_get_interface(void **pp, const uuid_t uuid)
{
	if (uuid_compare(uuid, UUID_OH_ABI_V2)==0) {
		*(struct oh_abi_v2 **)pp = &oh_dummy_plugin;
		return 0;
	}

	*pp = NULL;
	return -1;
}

int get_interface(void **pp, const uuid_t uuid) __attribute__ ((weak, alias("dummy_get_interface")));


/*******************************************************************/
/*******************************************************************/
/*******************************************************************/
/*******************************************************************/
/*******************************************************************/
/*******************************************************************/

#if 0	



/* The event is used to fill event list */
static struct oh_event user_event[] = {
/*
        {
	.type = OH_ET_HPI,
	.u = {
		.hpi_event = {
			.parent = {
				.ptr  = &dummy_resources[0][1],
			},
			.id = {
				.ptr = NULL,
			},
			.event = {
				.Source = 0,
				.EventType = SAHPI_ET_USER,
				.Timestamp = 0,
				.Severity = SAHPI_CRITICAL,
			},
		},
	},
        },
        {
	.type = OH_ET_HPI,
	.u = {
		.hpi_event = {
			.parent = {
				.ptr  = &dummy_resources[1][1],
			},
			.id = {
				.ptr = NULL,
			},
			.event = {
				.Source = 0,
				.EventType = SAHPI_ET_USER,
				.Timestamp = 0,
				.Severity = SAHPI_CRITICAL,
			},
		},
	},
        },
*/   
};

#endif

