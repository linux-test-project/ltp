/*      -*- linux-c -*-
 *
 * Copyright (c) 2004 by Intel Corp.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  This
 * file and program are licensed under a BSD style license.  See
 * the Copying file included with the OpenHPI distribution for
 * full licensing terms.
 *
 * Author(s):
 *	Kouzmich	< Mikhail.V.Kouzmich@intel.com >
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <hpi_ui.h>

//	function numbers for lookup_proc

#define LANG_PROC	1
#define TAGTYPE_PROC	2
#define RDRTYPE_PROC	3
#define SENREADT_PROC	4
#define SENUNIT_PROC	5
#define SENMODU_PROC	6
#define SENTYPE_PROC	7
#define CATEGORY_PROC	8
#define CTRLTYPE_PROC	9
#define CTRLMODE_PROC	10
#define CTRLOUTPUT_PROC	11
#define CTRLDIGIT_PROC	12
#define SEVERITY_PROC	13
#define EVENTCTRL_PROC	14

//	function numbers for decode_proc

#define EPATH_PROC	1

//	function numbers for decode1_proc

#define CAPAB_PROC	1
#define HSCAPAB_PROC	2
#define THDMASK_PROC	3
#define RANGEMASK_PROC	4

extern char	*lookup_proc(int num, int val);
extern SaErrorT	decode_proc(int num, void *val, char *buf, int bufsize);
extern SaErrorT	decode1_proc(int num, int val, char *buf, int bufsize);
extern SaErrorT	thres_value(SaHpiSensorReadingT *item, char *buf, int size);

#define RPT_ATTRS_NUM	9

attr_t	Def_rpt[] = {
	{ "EntryId",		INT_TYPE,	0, { .d = 0} },	//  0
	{ "ResourceId",		INT_TYPE,	0, { .d = 0} },	//  1
	{ "ResourceInfo",	STRUCT_TYPE,	0, { .d = 0} },	//  2
	{ "ResourceEntity",	DECODE_TYPE,	EPATH_PROC, { .d = 0} },	//  3
	{ "Capabilities",	DECODE1_TYPE,	CAPAB_PROC, { .d = 0} },	//  4
	{ "HotSwapCapabilities",DECODE1_TYPE,	HSCAPAB_PROC, { .d = 0} },	//  5
	{ "ResourceSeverity",	LOOKUP_TYPE,	SEVERITY_PROC, { .d = 0} },	//  6
	{ "ResourceFailed",	BOOL_TYPE,	0, { .d = 0} },	//  7
	{ "Tag",		TEXT_BUFF_TYPE,	0, { .d = 0} }	//  8
};

#define RESINFO_ATTRS_NUM	9

attr_t	Def_resinfo[] = {
	{ "ResourceRev",	INT_TYPE,	0, { .d = 0} },	//  0
	{ "SpecificVer",	INT_TYPE,	0, { .d = 0} },	//  1
	{ "DeviceSupport",	INT_TYPE,	0, { .d = 0} },	//  2
	{ "ManufacturerId",	INT_TYPE,	0, { .d = 0} },	//  3
	{ "ProductId",		INT_TYPE,	0, { .d = 0} },	//  4
	{ "FirmwareMajorRev",	INT_TYPE,	0, { .d = 0} },	//  5
	{ "FirmwareMinorRev",	INT_TYPE,	0, { .d = 0} },	//  6
	{ "AuxFirmwareRev",	INT_TYPE,	0, { .d = 0} },	//  7
	{ "Guid",		STR_TYPE,	0, { .d = 0} }	//  8
};

char *lookup_proc(int num, int val)
{
	char	*string = (char *)NULL;

	switch (num) {
		case LANG_PROC:
			string = oh_lookup_language(val); break;
		case TAGTYPE_PROC:
			string = oh_lookup_texttype(val); break;
		case RDRTYPE_PROC:
			string = oh_lookup_rdrtype(val); break;
		case SENREADT_PROC:
			string = oh_lookup_sensorreadingtype(val); break;
		case SENUNIT_PROC:
			string = oh_lookup_sensorunits(val); break;
		case SENMODU_PROC:
			string = oh_lookup_sensormodunituse(val); break;
		case SENTYPE_PROC:
			string = oh_lookup_sensortype(val); break;
		case CATEGORY_PROC:
			string = oh_lookup_eventcategory(val); break;
		case CTRLTYPE_PROC:
			string = oh_lookup_ctrltype(val); break;
		case CTRLMODE_PROC:
			string = oh_lookup_ctrlmode(val); break;
		case CTRLOUTPUT_PROC:
			string = oh_lookup_ctrloutputtype(val); break;
		case CTRLDIGIT_PROC:
			string = oh_lookup_ctrlstatedigital(val); break;
		case SEVERITY_PROC:
			string = oh_lookup_severity(val); break;
		case EVENTCTRL_PROC:
			string = oh_lookup_sensoreventctrl(val); break;
	};
	if (string == (char *)NULL)
		return("");
	return(string);
}

SaErrorT decode_proc(int num, void *val, char *buf, int bufsize)
{
	oh_big_textbuffer	tmpbuf;
	SaErrorT		rv;

	oh_init_bigtext(&tmpbuf);
	memset(buf, 0, bufsize);
	switch (num) {
		case EPATH_PROC:
			rv = oh_decode_entitypath((SaHpiEntityPathT *)val, &tmpbuf);
			if (rv != SA_OK) return(-1);
			break;
	};
	strncpy(buf, (char *)(tmpbuf.Data), bufsize);
	return(SA_OK);
}

static void oh_threshold_mask(SaHpiSensorThdMaskT mask, char *buf, int bufsize)
{
	char	tmp[256];
	int	ind;

	memset(buf, 0, 256);
	if (mask == 0) return;
	strcpy(tmp, "{ ");
	if (mask & SAHPI_STM_LOW_MINOR)
		strcat(tmp, "LOW_MINOR | ");
	if (mask & SAHPI_STM_LOW_MAJOR)
		strcat(tmp, "LOW_MAJOR | ");
	if (mask & SAHPI_STM_LOW_CRIT)
		strcat(tmp, "LOW_CRIT | ");
	if (mask & SAHPI_STM_LOW_HYSTERESIS)
		strcat(tmp, "LOW_HYSTERESIS | ");
	if (mask & SAHPI_STM_UP_MINOR)
		strcat(tmp, "UP_MINOR | ");
	if (mask & SAHPI_STM_UP_MAJOR)
		strcat(tmp, "UP_MAJOR | ");
	if (mask & SAHPI_STM_UP_CRIT)
		strcat(tmp, "UP_CRIT | ");
	if (mask & SAHPI_STM_UP_HYSTERESIS)
		strcat(tmp, "UP_HYSTERESIS | ");

	ind = strlen(tmp);
	/* Remove last delimiter */
	if (tmp[ind - 2] == '{')
		// null mask
		return;
	tmp[ind - 2] = '}';
	tmp[ind - 1] = 0;
	strncpy(buf, tmp, bufsize);
	return;
}

static void oh_range_mask(SaHpiSensorRangeFlagsT mask, char *buf, int bufsize)
{
	char	tmp[256];
	int	ind;

	memset(buf, 0, 256);
	if (mask == 0) return;
	strcpy(tmp, "{ ");
	if (mask & SAHPI_SRF_MIN)
		strcat(tmp, "MIN | ");
	if (mask & SAHPI_SRF_MAX)
		strcat(tmp, "MAX | ");
	if (mask & SAHPI_SRF_NORMAL_MIN)
		strcat(tmp, "NORMAL MIN | ");
	if (mask & SAHPI_SRF_NORMAL_MAX)
		strcat(tmp, "NORMAL MAX | ");
	if (mask & SAHPI_SRF_NOMINAL)
		strcat(tmp, "NOMINAL | ");

	ind = strlen(tmp);
	/* Remove last delimiter */
	if (tmp[ind - 2] == '{')
		// null mask
		return;
	tmp[ind - 2] = '}';
	tmp[ind - 1] = 0;
	strncpy(buf, tmp, bufsize);
	return;
}

SaErrorT decode1_proc(int num, int val, char *buf, int bufsize)
{
	SaHpiTextBufferT	tbuf;
	SaErrorT		rv;

	memset(buf, 0, bufsize);
	switch (num) {
		case CAPAB_PROC:
			rv = oh_decode_capabilities(val, &tbuf);
			if (rv != SA_OK) return(-1);
			break;
		case HSCAPAB_PROC:
			rv = oh_decode_hscapabilities(val, &tbuf);
			if (rv != SA_OK) return(-1);
			break;
		case THDMASK_PROC:
			oh_threshold_mask(val, buf, bufsize);
			return(SA_OK);
		case RANGEMASK_PROC:
			oh_range_mask(val, buf, bufsize);
			return(SA_OK);
	};
	strncpy(buf, (char *)(tbuf.Data), bufsize);
	return(SA_OK);
}

SaErrorT thres_value(SaHpiSensorReadingT *item, char *buf, int size)
{
	char	*val;
    int i, n;

	memset(buf, 0, size);
	if (item->IsSupported != SAHPI_TRUE)
		return(-1);
	switch (item->Type) {
		case SAHPI_SENSOR_READING_TYPE_INT64:
			snprintf(buf, size, "%lld", item->Value.SensorInt64);
			break;
		case SAHPI_SENSOR_READING_TYPE_UINT64:
			snprintf(buf, size, "%llu", item->Value.SensorUint64);
			break;
		case SAHPI_SENSOR_READING_TYPE_FLOAT64:
			snprintf(buf, size, "%10.3f", item->Value.SensorFloat64);
			break;
		case SAHPI_SENSOR_READING_TYPE_BUFFER:
			val = (char *)(item->Value.SensorBuffer);
            if (val == NULL ) {
                return(-1);
            }
            n = ( SAHPI_SENSOR_BUFFER_LENGTH > ( size / 2 ) ) ? ( size / 2 ) : SAHPI_SENSOR_BUFFER_LENGTH;
            buf[0] = '\0';
            for (i = 0; i < n; ++i) {
                sprintf( buf + 2 * i, "%02x", val[i] );
            }
	};
	return(SA_OK);
}

static int find_attr(Attributes_t *attrs, char *name)
{
	int	i;

	for (i = 0; i < attrs->n_attrs; i++) {
		if (strcmp(attrs->Attrs[i].name, name) == 0)
			return(i);
	};
	return(-1);
}

void make_attrs_rpt(Rpt_t *Rpt, SaHpiRptEntryT *rptentry)
{
	attr_t			*att, *att1;
	int			i = 0;
	Attributes_t		*at;
	SaHpiRptEntryT		*obj;

	Rpt->Table = *rptentry;
	obj = &(Rpt->Table);
	Rpt->Attrutes.n_attrs = RPT_ATTRS_NUM;
	Rpt->Attrutes.Attrs = (attr_t *)malloc(sizeof(attr_t) * RPT_ATTRS_NUM);
	memcpy(Rpt->Attrutes.Attrs, Def_rpt, sizeof(attr_t) * RPT_ATTRS_NUM);
	att = Rpt->Attrutes.Attrs;
	att[i++].value.i = obj->EntryId;
	att[i++].value.i = obj->ResourceId;
	at = (Attributes_t *)malloc(sizeof(Attributes_t));
	at->n_attrs = RESINFO_ATTRS_NUM;
	att1 = (attr_t *)malloc(sizeof(attr_t) * RESINFO_ATTRS_NUM);
	memcpy(att1, Def_resinfo, sizeof(attr_t) * RESINFO_ATTRS_NUM);
	at->Attrs = att1;
	att[i++].value.a = at;
	att1[0].value.i = obj->ResourceInfo.ResourceRev;
	att1[1].value.i = obj->ResourceInfo.SpecificVer;
	att1[2].value.i = obj->ResourceInfo.DeviceSupport;
	att1[3].value.i = obj->ResourceInfo.ManufacturerId;
	att1[4].value.i = obj->ResourceInfo.ProductId;
	att1[5].value.i = obj->ResourceInfo.FirmwareMajorRev;
	att1[6].value.i = obj->ResourceInfo.FirmwareMinorRev;
	att1[7].value.i = obj->ResourceInfo.AuxFirmwareRev;
	att1[8].value.s = (char *)(obj->ResourceInfo.Guid);
	att[i++].value.a = &(obj->ResourceEntity);
	att[i++].value.i = obj->ResourceCapabilities;
	att[i++].value.i = obj->HotSwapCapabilities;
	att[i++].value.i = obj->ResourceSeverity;
	att[i++].value.i = obj->ResourceFailed;
	att[i++].value.a = &(obj->ResourceTag);
}

#define RDR_ATTRS_COMMON_NUM	6

attr_t	Def_common_rdr[] = {
	{ "RecordId",		INT_TYPE,	0, { .d = 0} },	//  0
	{ "RdrType",		LOOKUP_TYPE,	RDRTYPE_PROC, { .d = 0} },	//  1
	{ "EntityPath",		DECODE_TYPE,	EPATH_PROC, { .d = 0} },	//  2
	{ "IsFru",		BOOL_TYPE,	0, { .d = 0} },	//  3
	{ "Record",		STRUCT_TYPE,	0, { .d = 0} },	//  4
	{ "IdString",		TEXT_BUFF_TYPE,	0, { .d = 0} }	//  5
};

#define RDR_ATTRS_SENSOR_NUM	9

attr_t	Def_sensor_rdr[] = {
	{ "Num",		INT_TYPE,	0, { .d = 0} },	//  0
	{ "Type",		LOOKUP_TYPE,	SENTYPE_PROC, { .d = 0} },  //  1
	{ "Category",		LOOKUP_TYPE,	CATEGORY_PROC, { .d = 0} }, //  2
	{ "EnableCtrl",		BOOL_TYPE,	0, { .d = 0} },	//  3
	{ "EventCtrl",		LOOKUP_TYPE,	EVENTCTRL_PROC, { .d = 0} }, //  4
	{ "Events",		HEX_TYPE,	0, { .d = 0} },	//  5
	{ "DataFormat",		STRUCT_TYPE,	0, { .d = 0} },	//  6
	{ "ThresholdDefn",	STRUCT_TYPE,	0, { .d = 0} },	//  7
	{ "Oem",		INT_TYPE,	0, { .d = 0} }	//  8
};

#define ATTRS_SENSOR_DATAFORMAT	8

attr_t	DataForm_rdr[] = {
	{ "IsSupported",	BOOL_TYPE,	0, { .d = 0} },	//  0
	{ "ReadingType",	LOOKUP_TYPE,	SENREADT_PROC, { .d = 0} }, //  1
	{ "BaseUnits",		LOOKUP_TYPE,	SENUNIT_PROC, { .d = 0} },  //  2
	{ "ModifierUnits",	LOOKUP_TYPE,	SENUNIT_PROC, { .d = 0} },  //  3
	{ "ModifierUse",	LOOKUP_TYPE,	SENMODU_PROC, { .d = 0} },  //  4
	{ "Percentage",		BOOL_TYPE,	0, { .d = 0} },	//  5
	{ "Range",		STRUCT_TYPE,	0, { .d = 0} },	//  6
	{ "AccuracyFactor",	FLOAT_TYPE,	0, { .d = 0} }	//  7
};

#define ATTRS_SENSOR_THDDEF	4

attr_t	ThresDef_rdr[] = {
	{ "IsAccessible",	BOOL_TYPE,	0, { .d = 0} },	//  0
	{ "ReadMask",		DECODE1_TYPE,	THDMASK_PROC, { .d = 0} },	//  1
	{ "WriteMask",		DECODE1_TYPE,	THDMASK_PROC, { .d = 0} }, 	//  2
	{ "Nonlinear",		BOOL_TYPE,	0, { .d = 0} }	//  3
};

#define ATTRS_SENSOR_RANGE	6

attr_t	Range_rdr[] = {
	{ "Flags",		DECODE1_TYPE,	RANGEMASK_PROC, { .d = 0} },	//  0
	{ "Max",		READING_TYPE,	0, { .d = 0} },	//  1
	{ "Min",		READING_TYPE,	0, { .d = 0} }, 	//  2
	{ "Nominal",		READING_TYPE,	0, { .d = 0} },	//  3
	{ "NormalMax",		READING_TYPE,	0, { .d = 0} }, 	//  4
	{ "NormalMin",		READING_TYPE,	0, { .d = 0} }	//  5
};

static Attributes_t *make_attrs_sensor(SaHpiSensorRecT *sensor)
{
	attr_t		*att1, *att2, *att3;
	Attributes_t	*at, *at2, *at3;
	
	at = (Attributes_t *)malloc(sizeof(Attributes_t));
	at->n_attrs = RDR_ATTRS_SENSOR_NUM;
	att1 = (attr_t *)malloc(sizeof(attr_t) * RDR_ATTRS_SENSOR_NUM);
	memcpy(att1, Def_sensor_rdr, sizeof(attr_t) * RDR_ATTRS_SENSOR_NUM);
	at->Attrs = att1;
	att1[0].value.i = sensor->Num;
	att1[1].value.i = sensor->Type;
	att1[2].value.i = sensor->Category;
	att1[3].value.i = sensor->EnableCtrl;
	att1[4].value.i = sensor->EventCtrl;
	att1[5].value.i = sensor->Events;

	at2 = (Attributes_t *)malloc(sizeof(Attributes_t));
	at2->n_attrs = ATTRS_SENSOR_DATAFORMAT;
	att2 = (attr_t *)malloc(sizeof(attr_t) * ATTRS_SENSOR_DATAFORMAT);
	memcpy(att2, DataForm_rdr, sizeof(attr_t) * ATTRS_SENSOR_DATAFORMAT);
	at2->Attrs = att2;
	att2[0].value.i = sensor->DataFormat.IsSupported;
	att2[1].value.i = sensor->DataFormat.ReadingType;
	att2[2].value.i = sensor->DataFormat.BaseUnits;
	att2[3].value.i = sensor->DataFormat.ModifierUnits;
	att2[4].value.i = sensor->DataFormat.ModifierUse;
	att2[5].value.i = sensor->DataFormat.Percentage;

	at3 = (Attributes_t *)malloc(sizeof(Attributes_t));
	at3->n_attrs = ATTRS_SENSOR_RANGE;
	att3 = (attr_t *)malloc(sizeof(attr_t) * ATTRS_SENSOR_RANGE);
	memcpy(att3, Range_rdr, sizeof(attr_t) * ATTRS_SENSOR_RANGE);
	at3->Attrs = att3;
	att3[0].value.i = sensor->DataFormat.Range.Flags;
	att3[1].value.a = &(sensor->DataFormat.Range.Max);
	att3[2].value.a = &(sensor->DataFormat.Range.Min);
	att3[3].value.a = &(sensor->DataFormat.Range.Nominal);
	att3[4].value.a = &(sensor->DataFormat.Range.NormalMax);
	att3[5].value.a = &(sensor->DataFormat.Range.NormalMin);

	att2[6].value.a = at3;
	att2[7].value.d = sensor->DataFormat.AccuracyFactor;

	att1[6].value.a = at2;

	at2 = (Attributes_t *)malloc(sizeof(Attributes_t));
	at2->n_attrs = ATTRS_SENSOR_THDDEF;
	att2 = (attr_t *)malloc(sizeof(attr_t) * ATTRS_SENSOR_THDDEF);
	memcpy(att2, ThresDef_rdr, sizeof(attr_t) * ATTRS_SENSOR_THDDEF);
	att2[0].value.i = sensor->ThresholdDefn.IsAccessible;
	att2[1].value.i = sensor->ThresholdDefn.ReadThold;
	att2[2].value.i = sensor->ThresholdDefn.WriteThold;
	at2->Attrs = att2;
	att1[7].value.a = at2;

	att1[8].value.i = sensor->Oem;
	return(at);
}

#define RDR_ATTRS_CTRL_NUM	7

attr_t	Def_ctrl_rdr[] = {
	{ "Num",		INT_TYPE,	0, { .d = 0} },	//  0
	{ "Type",		LOOKUP_TYPE,	CTRLTYPE_PROC, { .d = 0} }, //  1
	{ "OutputType",		LOOKUP_TYPE,	CTRLOUTPUT_PROC, { .d = 0} }, //  2
	{ "TypeUnion",		STRUCT_TYPE,	0, { .d = 0} },	//  3
	{ "DefaultMode",	STRUCT_TYPE,	0, { .d = 0} },	//  4
	{ "WriteOnly",		BOOL_TYPE,	0, { .d = 0} },	//  5
	{ "Oem",		INT_TYPE,	0, { .d = 0} }	//  6
};

#define ATTRS_CTRL_MODE		2

attr_t	Def_ctrl_mode[] = {
	{ "Mode",		LOOKUP_TYPE,	CTRLMODE_PROC, { .d = 0} },	//  0
	{ "ReadOnly",		BOOL_TYPE,	0, { .d = 0} } //  1
};

#define ATTRS_CTRL_DIGITAL	1

attr_t	Def_ctrl_digital[] = {
	{ "Default",		LOOKUP_TYPE,	CTRLDIGIT_PROC, { .d = 0} }	//  0
};

#define ATTRS_CTRL_DISCRETE	1

attr_t	Def_ctrl_discrete[] = {
	{ "Default",		INT_TYPE,	0, { .d = 0} }	//  0
};

#define ATTRS_CTRL_ANALOG	3

attr_t	Def_ctrl_analog[] = {
	{ "Min",		INT_TYPE,	0, { .d = 0} },	//  0
	{ "Max",		INT_TYPE,	0, { .d = 0} },	//  1
	{ "Default",		INT_TYPE,	0, { .d = 0} }  //  2
};

#define ATTRS_CTRL_STREAM	3

attr_t	Def_ctrl_stream[] = {
	{ "Repeat",		BOOL_TYPE,	0, { .d = 0} },	//  0
	{ "Length",		INT_TYPE,	0, { .d = 0} },	//  1
	{ "Stream",		STR_TYPE,	0, { .d = 0} }  //  2
};

#define ATTRS_CTRL_TEXT		5

attr_t	Def_ctrl_text[] = {
	{ "MaxChars",		INT_TYPE,	0, { .d = 0} },	//  0
	{ "MaxLines",		INT_TYPE,	0, { .d = 0} },	//  1
	{ "Language",		LOOKUP_TYPE,	LANG_PROC, { .d = 0} },	//  2
	{ "DataType",		LOOKUP_TYPE,	TAGTYPE_PROC, { .d = 0} },	//  3
	{ "Default",		STR_TYPE,	0, { .d = 0} }  //  4
};

#define ATTRS_CTRL_TEXT_DEFAULT	2

attr_t	Def_ctrl_text_def[] = {
	{ "Line",		INT_TYPE,	0, { .d = 0} },	//  0
	{ "Text",		TEXT_BUFF_TYPE,	0, { .d = 0} }  //  4
};

static Attributes_t *make_attrs_ctrl(SaHpiCtrlRecT *ctrl)
{
	attr_t			*att1, *att2, *att3;
	Attributes_t		*at, *at2, *at3;
	SaHpiCtrlRecDigitalT	*digital;
	SaHpiCtrlRecDiscreteT	*discrete;
	SaHpiCtrlRecAnalogT	*analog;
	SaHpiCtrlRecStreamT	*stream;
	SaHpiCtrlRecTextT	*text;

	at = (Attributes_t *)malloc(sizeof(Attributes_t));
	at->n_attrs = RDR_ATTRS_CTRL_NUM;
	att1 = (attr_t *)malloc(sizeof(attr_t) * RDR_ATTRS_CTRL_NUM);
	memcpy(att1, Def_ctrl_rdr, sizeof(attr_t) * RDR_ATTRS_CTRL_NUM);
	at->Attrs = att1;
	att1[0].value.i = ctrl->Num;
	att1[1].value.i = ctrl->Type;
	att1[2].value.i = ctrl->OutputType;

	switch (ctrl->Type) {
		case SAHPI_CTRL_TYPE_DIGITAL:
			digital = &(ctrl->TypeUnion.Digital);
			att1[3].name = "Digital";
			at2 = (Attributes_t *)malloc(sizeof(Attributes_t));
			at2->n_attrs = ATTRS_CTRL_DIGITAL;
			att2 = (attr_t *)malloc(sizeof(attr_t) * ATTRS_CTRL_DIGITAL);
			memcpy(att2, Def_ctrl_digital, sizeof(attr_t) * ATTRS_CTRL_DIGITAL);
			at2->Attrs = att2;
			att2[0].value.i = digital->Default;
			att1[3].value.a = at2;
			break;
		case SAHPI_CTRL_TYPE_DISCRETE:
			discrete = &(ctrl->TypeUnion.Discrete);
			att1[3].name = "Discrete";
			at2 = (Attributes_t *)malloc(sizeof(Attributes_t));
			at2->n_attrs = ATTRS_CTRL_DISCRETE;
			att2 = (attr_t *)malloc(sizeof(attr_t) * ATTRS_CTRL_DISCRETE);
			memcpy(att2, Def_ctrl_discrete,
				sizeof(attr_t) * ATTRS_CTRL_DISCRETE);
			at2->Attrs = att2;
			att2[0].value.i = discrete->Default;
			att1[3].value.a = at2;
			break;
		case SAHPI_CTRL_TYPE_ANALOG:
			analog = &(ctrl->TypeUnion.Analog);
			att1[3].name = "Analog";
			at2 = (Attributes_t *)malloc(sizeof(Attributes_t));
			at2->n_attrs = ATTRS_CTRL_ANALOG;
			att2 = (attr_t *)malloc(sizeof(attr_t) * ATTRS_CTRL_ANALOG);
			memcpy(att2, Def_ctrl_analog,
				sizeof(attr_t) * ATTRS_CTRL_ANALOG);
			at2->Attrs = att2;
			att2[0].value.i = analog->Min;
			att2[1].value.i = analog->Max;
			att2[2].value.i = analog->Default;
			att1[3].value.a = at2;
			break;
		case SAHPI_CTRL_TYPE_STREAM:
			stream = &(ctrl->TypeUnion.Stream);
			att1[3].name = "Stream";
			at2 = (Attributes_t *)malloc(sizeof(Attributes_t));
			at2->n_attrs = ATTRS_CTRL_STREAM;
			att2 = (attr_t *)malloc(sizeof(attr_t) * ATTRS_CTRL_STREAM);
			memcpy(att2, Def_ctrl_stream,
				sizeof(attr_t) * ATTRS_CTRL_STREAM);
			at2->Attrs = att2;
			att2[0].value.i = stream->Default.Repeat;
			att2[1].value.i = stream->Default.StreamLength;
			att2[2].value.s = (char *)(stream->Default.Stream);
			att1[3].value.a = at2;
			break;
		case SAHPI_CTRL_TYPE_TEXT:
			text = &(ctrl->TypeUnion.Text);
			att1[3].name = "Text";
			at2 = (Attributes_t *)malloc(sizeof(Attributes_t));
			at2->n_attrs = ATTRS_CTRL_TEXT;
			att2 = (attr_t *)malloc(sizeof(attr_t) * ATTRS_CTRL_TEXT);
			memcpy(att2, Def_ctrl_text,
				sizeof(attr_t) * ATTRS_CTRL_TEXT);
			at2->Attrs = att2;
			att2[0].value.i = text->MaxChars;
			att2[1].value.i = text->MaxLines;
			att2[2].value.i = text->Language;
			att2[3].value.i = text->DataType;
			at3 = (Attributes_t *)malloc(sizeof(Attributes_t));
			at3->n_attrs = ATTRS_CTRL_TEXT_DEFAULT;
			att3 = (attr_t *)malloc(sizeof(attr_t) * ATTRS_CTRL_TEXT_DEFAULT);
			memcpy(att3, Def_ctrl_text_def,
				sizeof(attr_t) * ATTRS_CTRL_TEXT_DEFAULT);
			at3->Attrs = att3;
			att3[0].value.i = text->Default.Line;
			att3[1].value.a = &(text->Default.Text);
			att2[4].value.a = at3;
			att1[3].value.a = at2;
			break;
		default:
			att1[3].name = "Digital";
			att1[3].type = NO_TYPE;
	};

	at2 = (Attributes_t *)malloc(sizeof(Attributes_t));
	at2->n_attrs = ATTRS_CTRL_MODE;
	att2 = (attr_t *)malloc(sizeof(attr_t) * ATTRS_CTRL_MODE);
	memcpy(att2, Def_ctrl_mode, sizeof(attr_t) * ATTRS_CTRL_MODE);
	at2->Attrs = att2;
	att2[0].value.i = ctrl->DefaultMode.Mode;
	att2[1].value.i = ctrl->DefaultMode.ReadOnly;
	att1[4].value.a = at2;
	
	att1[5].value.i = ctrl->WriteOnly;
	att1[6].value.i = ctrl->Oem;
	return(at);
}

#define RDR_ATTRS_INV_NUM	3

attr_t	Def_inv_rdr[] = {
	{ "IdrId",		INT_TYPE,	0, { .d = 0} },	//  0
	{ "Persistent",		BOOL_TYPE,	0, { .d = 0} }, //  1
	{ "Oem",		INT_TYPE,	0, { .d = 0} }	//  2
};

static Attributes_t *make_attrs_inv(SaHpiInventoryRecT *inv)
{
	attr_t			*att1;
	Attributes_t		*at;

	at = (Attributes_t *)malloc(sizeof(Attributes_t));
	at->n_attrs = RDR_ATTRS_INV_NUM;
	att1 = (attr_t *)malloc(sizeof(attr_t) * RDR_ATTRS_INV_NUM);
	memcpy(att1, Def_inv_rdr, sizeof(attr_t) * RDR_ATTRS_INV_NUM);
	at->Attrs = att1;
	att1[0].value.i = inv->IdrId;
	att1[1].value.i = inv->Persistent;
	att1[2].value.i = inv->Oem;
	return(at);
}

#define RDR_ATTRS_WDOG_NUM	2

attr_t	Def_wdog_rdr[] = {
	{ "Num",		INT_TYPE,	0, { .d = 0} },	//  0
	{ "Oem",		INT_TYPE,	0, { .d = 0} }	//  1
};

static Attributes_t *make_attrs_wdog(SaHpiWatchdogRecT *wdog)
{
	attr_t			*att1;
	Attributes_t		*at;

	at = (Attributes_t *)malloc(sizeof(Attributes_t));
	at->n_attrs = RDR_ATTRS_WDOG_NUM;
	att1 = (attr_t *)malloc(sizeof(attr_t) * RDR_ATTRS_WDOG_NUM);
	memcpy(att1, Def_wdog_rdr, sizeof(attr_t) * RDR_ATTRS_WDOG_NUM);
	at->Attrs = att1;
	att1[0].value.i = wdog->WatchdogNum;
	att1[1].value.i = wdog->Oem;
	return(at);
}

#define RDR_ATTRS_ANNUN_NUM	5

attr_t	Def_annun_rdr[] = {
	{ "Num",		INT_TYPE,	0, { .d = 0} },	//  0
	{ "Type",		INT_TYPE,	0, { .d = 0} },	//  1
	{ "ReadOnly",		INT_TYPE,	0, { .d = 0} },	//  2
	{ "MaxConditions",	INT_TYPE,	0, { .d = 0} },	//  3
	{ "Oem",		INT_TYPE,	0, { .d = 0} }	//  4
};

static Attributes_t *make_attrs_annun(SaHpiAnnunciatorRecT *annun)
{
	attr_t			*att1;
	Attributes_t		*at;

	at = (Attributes_t *)malloc(sizeof(Attributes_t));
	at->n_attrs = RDR_ATTRS_ANNUN_NUM;
	att1 = (attr_t *)malloc(sizeof(attr_t) * RDR_ATTRS_ANNUN_NUM);
	memcpy(att1, Def_annun_rdr, sizeof(attr_t) * RDR_ATTRS_ANNUN_NUM);
	at->Attrs = att1;
	att1[0].value.i = annun->AnnunciatorNum;
	att1[1].value.i = annun->AnnunciatorType;
	att1[2].value.i = annun->ModeReadOnly;
	att1[3].value.i = annun->MaxConditions;
	att1[4].value.i = annun->Oem;
	return(at);
}

#define RDR_ATTRS_DIMI_NUM	2

attr_t	Def_dimi_rdr[] = {
	{ "DimiNum",		INT_TYPE,	0, { .d = 0} },	//  0
	{ "Oem",		INT_TYPE,	0, { .d = 0} }	//  1
};

static Attributes_t *make_attrs_dimi(SaHpiDimiRecT *dimi)
{
	attr_t			*att1;
	Attributes_t		*at;

	at = (Attributes_t *)malloc(sizeof(Attributes_t));
	at->n_attrs = RDR_ATTRS_DIMI_NUM;
	att1 = (attr_t *)malloc(sizeof(attr_t) * RDR_ATTRS_DIMI_NUM);
	memcpy(att1, Def_dimi_rdr, sizeof(attr_t) * RDR_ATTRS_DIMI_NUM);
	at->Attrs = att1;
	att1[0].value.i = dimi->DimiNum;
	att1[1].value.i = dimi->Oem;
	return(at);
}

#define RDR_ATTRS_FUMI_NUM	5

attr_t	Def_fumi_rdr[] = {
	{ "Num",		INT_TYPE,	0, { .d = 0} },	//  0
	{ "AccessProt",		INT_TYPE,	0, { .d = 0} },	//  1
	{ "Capability",		INT_TYPE,	0, { .d = 0} },	//  2
	{ "NumBanks",	INT_TYPE,	0, { .d = 0} },	//  3
	{ "Oem",		INT_TYPE,	0, { .d = 0} }	//  4
};

static Attributes_t *make_attrs_fumi(SaHpiFumiRecT *fumi)
{
	attr_t			*att1;
	Attributes_t		*at;

	at = (Attributes_t *)malloc(sizeof(Attributes_t));
	at->n_attrs = RDR_ATTRS_FUMI_NUM;
	att1 = (attr_t *)malloc(sizeof(attr_t) * RDR_ATTRS_FUMI_NUM);
	memcpy(att1, Def_fumi_rdr, sizeof(attr_t) * RDR_ATTRS_FUMI_NUM);
	at->Attrs = att1;
	att1[0].value.i = fumi->Num;
	att1[1].value.i = fumi->AccessProt;
	att1[2].value.i = fumi->Capability;
	att1[3].value.i = fumi->NumBanks;
	att1[4].value.i = fumi->Oem;
	return(at);
}

void make_attrs_rdr(Rdr_t *Rdr, SaHpiRdrT *rdrentry)
{
	attr_t			*att;
	int			i = 0;
	Attributes_t		*at;
	SaHpiRdrT		*obj;
	SaHpiSensorRecT		*sensor;
	SaHpiCtrlRecT		*ctrl;
	SaHpiInventoryRecT	*inv;
	SaHpiWatchdogRecT	*wdog;
	SaHpiAnnunciatorRecT	*annun;
    SaHpiDimiRecT       *dimi;
    SaHpiFumiRecT       *fumi;

	Rdr->Record = *rdrentry;
	obj = &(Rdr->Record);
	Rdr->Attrutes.n_attrs = RDR_ATTRS_COMMON_NUM;
	Rdr->Attrutes.Attrs = (attr_t *)malloc(sizeof(attr_t) * RDR_ATTRS_COMMON_NUM);
	memcpy(Rdr->Attrutes.Attrs, Def_common_rdr, sizeof(attr_t) * RDR_ATTRS_COMMON_NUM);
	att = Rdr->Attrutes.Attrs;
	att[i++].value.i = obj->RecordId;
	att[i++].value.i = obj->RdrType;
	att[i++].value.a = &(obj->Entity);
	att[i++].value.i = obj->IsFru;

	switch (obj->RdrType) {
		case SAHPI_SENSOR_RDR:
			sensor = &(obj->RdrTypeUnion.SensorRec);
			at = make_attrs_sensor(sensor);
			att[i].name = "Sensor";
			att[i++].value.a = at;
			break;
		case SAHPI_CTRL_RDR:
			ctrl = &(obj->RdrTypeUnion.CtrlRec);
			at = make_attrs_ctrl(ctrl);
			att[i].name = "Control";
			att[i++].value.a = at;
			break;
		case SAHPI_INVENTORY_RDR:
			inv = &(obj->RdrTypeUnion.InventoryRec);
			at = make_attrs_inv(inv);
			att[i].name = "Inventory";
			att[i++].value.a = at;
			break;
		case SAHPI_WATCHDOG_RDR:
			wdog = &(obj->RdrTypeUnion.WatchdogRec);
			at = make_attrs_wdog(wdog);
			att[i].name = "Watchdog";
			att[i++].value.a = at;
			break;
		case SAHPI_ANNUNCIATOR_RDR:
			annun = &(obj->RdrTypeUnion.AnnunciatorRec);
			at = make_attrs_annun(annun);
			att[i].name = "Annunciator";
			att[i++].value.a = at;
			break;
       case SAHPI_DIMI_RDR:
           dimi = &(obj->RdrTypeUnion.DimiRec);
           at = make_attrs_dimi(dimi);
           att[i].name = "DIMI";
           att[i++].value.a = at;
           break;
       case SAHPI_FUMI_RDR:
           fumi = &(obj->RdrTypeUnion.FumiRec);
           at = make_attrs_fumi(fumi);
           att[i].name = "FUMI";
           att[i++].value.a = at;
           break;
		default: break;
	};
	att[i++].value.a = &(obj->IdString);
}

void free_attrs(Attributes_t *At)
{
	int		i;
	attr_t		*attr;

	for (i = 0, attr = At->Attrs; i < At->n_attrs; i++, attr++) {
		if (attr->type == STRUCT_TYPE) {
			if (attr->value.a == 0) continue;
			free_attrs((Attributes_t *)(attr->value.a));
			free(attr->value.a);
		}
	};
	free(At->Attrs);
}

void time2str(SaHpiTimeT time, char * str, size_t size)
{
	struct tm t;
	time_t tt;
	
	if (!str) return;

        if (time > SAHPI_TIME_MAX_RELATIVE) { /*absolute time*/
                tt = time / 1000000000;
                strftime(str, size, "%F %T", localtime(&tt));
        } else if (time ==  SAHPI_TIME_UNSPECIFIED) { 
                strcpy(str,"SAHPI_TIME_UNSPECIFIED     ");
        } else if (time > SAHPI_TIME_UNSPECIFIED) { /*invalid time*/
                strcpy(str,"invalid time     ");
        } else {   /*relative time*/
		tt = time / 1000000000;
		localtime_r(&tt, &t);
		strftime(str, size, "%b %d, %Y - %H:%M:%S", &t);
	}
}

char *get_attr_name(Attributes_t *Attrs, int num)
{
	if ((num < 0) || (num >= Attrs->n_attrs))
		return((char *)NULL);
	return(Attrs->Attrs[num].name);
}

int get_attr_type(Attributes_t *Attrs, int num)
{
	if ((num < 0) || (num >= Attrs->n_attrs))
		return(-1);
	return(Attrs->Attrs[num].type);
}

SaErrorT get_value_as_string(Attributes_t *Attrs, int num, char *val, int len)
{
	int		type;
	SaHpiBoolT	b;

	if ((num < 0) || (num >= Attrs->n_attrs) || (val == (char *)NULL) || (len == 0))
		return SA_ERR_HPI_INVALID_PARAMS;

	type = Attrs->Attrs[num].type;
	switch (type) {
		case BOOL_TYPE:
			b = Attrs->Attrs[num].value.i;
			if (b) snprintf(val, len, "%s", "TRUE");
			else   snprintf(val, len, "%s", "FALSE");
			break;
		case INT_TYPE:
			snprintf(val, len, "%d", Attrs->Attrs[num].value.i);
			break;
		case HEX_TYPE:
			snprintf(val, len, "0x%x", Attrs->Attrs[num].value.i);
			break;
		case FLOAT_TYPE:
			snprintf(val, len, "%f", Attrs->Attrs[num].value.d);
			break;
		case STR_TYPE:
			if (Attrs->Attrs[num].value.s != (char *)NULL)
				snprintf(val, len, "%s", Attrs->Attrs[num].value.s);
			else *val = 0;
			break;
		default:	return(SA_ERR_HPI_ERROR);
	};
	return(SA_OK);
}

SaErrorT get_rdr_attr_as_string(Rdr_t *rdr, char *attr_name, char *val, int len)
{
	int		i;
	SaErrorT	ret;

	if ((attr_name == (char *)NULL) || (val == (char *)NULL) || (len == 0))
		return(SA_ERR_HPI_INVALID_PARAMS);

	i = find_attr(&(rdr->Attrutes), attr_name);
	if (i < 0)
		return(SA_ERR_HPI_INVALID_PARAMS);
	ret = get_value_as_string(&(rdr->Attrutes), i, val, len);
	return(ret);
}

SaErrorT get_rdr_attr(Rdr_t *rdr, char *attr_name, union_type_t *val)
{
	int	i;

	if ((attr_name == (char *)NULL) || (val == (union_type_t *)NULL))
		return(SA_ERR_HPI_INVALID_PARAMS);

	i = find_attr(&(rdr->Attrutes), attr_name);
	if (i < 0)
		return(SA_ERR_HPI_INVALID_PARAMS);
	*val = rdr->Attrutes.Attrs[i].value;
	return(SA_OK);
}

SaErrorT get_rpt_attr_as_string(Rpt_t *rpt, char *attr_name, char *val, int len)
{
	int		i;
	SaErrorT	ret;

	if ((attr_name == (char *)NULL) || (val == (char *)NULL) || (len == 0))
		return(SA_ERR_HPI_INVALID_PARAMS);

	i = find_attr(&(rpt->Attrutes), attr_name);
	if (i < 0)
		return(SA_ERR_HPI_INVALID_PARAMS);
	ret = get_value_as_string(&(rpt->Attrutes), i, val, len);
	return(ret);
}

SaErrorT get_rpt_attr(Rpt_t *rpt, char *attr_name, union_type_t *val)
{
	int	i;

	if ((attr_name == (char *)NULL) || (val == (union_type_t *)NULL))
		return(SA_ERR_HPI_INVALID_PARAMS);

	i = find_attr(&(rpt->Attrutes), attr_name);
	if (i < 0)
		return(SA_ERR_HPI_INVALID_PARAMS);
	*val = rpt->Attrutes.Attrs[i].value;
	return(SA_OK);
}

SaErrorT get_value(Attributes_t *Attrs, int num, union_type_t *val)
{
	int	type;

	if ((num < 0) || (num >= Attrs->n_attrs) || (val == (union_type_t *)NULL))
		return SA_ERR_HPI_INVALID_PARAMS;
	type = Attrs->Attrs[num].type;
	if ((type == STR_TYPE) || (type == STRUCT_TYPE) || (type == ARRAY_TYPE)) {
		if (Attrs->Attrs[num].value.s == (char *)NULL)
			return(-1);
	};
	*val = Attrs->Attrs[num].value;
	return(SA_OK);
}

SaErrorT find_rdr_by_num(SaHpiSessionIdT sessionid, SaHpiResourceIdT resourceid,
			SaHpiInstrumentIdT num, SaHpiRdrTypeT type, int as,
			SaHpiRdrT *retrdr)
//as : 0 - get rdr by num
//     1 - get first rdr
{
	SaHpiRdrT		rdr;
	SaErrorT		rv;
	SaHpiEntryIdT		entryid;
	SaHpiEntryIdT		nextentryid;
	SaHpiInstrumentIdT	rdrnum;

	entryid = SAHPI_FIRST_ENTRY;
	while (entryid !=SAHPI_LAST_ENTRY) {
		rv = saHpiRdrGet(sessionid, resourceid, entryid, &nextentryid, &rdr);
		if (rv != SA_OK) return(-1);
		if ((type != SAHPI_NO_RECORD) && (rdr.RdrType != type))
			continue;
		switch (rdr.RdrType) {
			case SAHPI_CTRL_RDR:
				rdrnum = rdr.RdrTypeUnion.CtrlRec.Num; break;
			case SAHPI_SENSOR_RDR:
				rdrnum = rdr.RdrTypeUnion.SensorRec.Num; break;
			case SAHPI_INVENTORY_RDR:
				rdrnum = rdr.RdrTypeUnion.InventoryRec.IdrId; break;
			case SAHPI_WATCHDOG_RDR:
				rdrnum = rdr.RdrTypeUnion.WatchdogRec.WatchdogNum; break;
			case SAHPI_ANNUNCIATOR_RDR:
				rdrnum = rdr.RdrTypeUnion.AnnunciatorRec.AnnunciatorNum; break;
            case SAHPI_DIMI_RDR:
                rdrnum = rdr.RdrTypeUnion.DimiRec.DimiNum; break;
            case SAHPI_FUMI_RDR:
                rdrnum = rdr.RdrTypeUnion.FumiRec.Num; break;
			default:
				entryid = nextentryid; continue;
		};
		if ((rdrnum == num) || as) {
			*retrdr = rdr;
			return(SA_OK);
		};
		entryid = nextentryid;
	};
	return(-1);
}

char *hex_codes = "0123456789ABCDEF";

Pr_ret_t print_text_buffer_type(char *mes, SaHpiTextBufferT *buf, char *meslast,
	hpi_ui_print_cb_t proc)
{
	char	*str = "";

	if (mes != (char *)NULL) {
		if (proc(mes) == HPI_UI_END) return(HPI_UI_END);
	};
	switch (buf->DataType) {
		case SAHPI_TL_TYPE_UNICODE:	str = "UNICODE"; break;
		case SAHPI_TL_TYPE_BCDPLUS:	str = "BCDPLUS"; break;
		case SAHPI_TL_TYPE_ASCII6:	str = "ASCII6"; break;
		case SAHPI_TL_TYPE_TEXT:	str = "TEXT"; break;
		case SAHPI_TL_TYPE_BINARY:	str = "BIN"; break;
	};
	if (proc(str) == HPI_UI_END) return(HPI_UI_END);
	if (meslast != (char *)NULL) {
		if (proc(meslast) == HPI_UI_END) return(HPI_UI_END);
	};
	return(0);
}

void get_text_buffer_text(char *mes, SaHpiTextBufferT *buf, char *meslast,
	char *outbuf)
{
	int	i, c, tmp_ind, len;
	char	*tmp;

	*outbuf = 0;
	if (mes != (char *)NULL)
		strcpy(outbuf,mes);
	if ((buf->DataLength == 0) && (buf->DataType != SAHPI_TL_TYPE_BINARY)) {
		if (meslast != (char *)NULL)
			strcat(outbuf, meslast);
		return;
	};
	switch (buf->DataType) {
		case SAHPI_TL_TYPE_UNICODE:
			strcat(outbuf, "UNICODE does not implement");
			break;
		case SAHPI_TL_TYPE_BCDPLUS:
		case SAHPI_TL_TYPE_ASCII6:
		case SAHPI_TL_TYPE_TEXT:
			strncat(outbuf, (char *)(buf->Data), buf->DataLength);
			break;
		case SAHPI_TL_TYPE_BINARY:
			len = buf->DataLength * 2 + 1;
			tmp = malloc(len);
			memset(tmp, 0, len);
			tmp_ind = 0;
			memset(tmp, 0, len);
			for (i = 0; i < buf->DataLength; i++) {
				c = (buf->Data[i] & 0xF0) >> 4;
				tmp[tmp_ind++] = hex_codes[c];
				c = buf->Data[i] & 0x0F;
				tmp[tmp_ind++] = hex_codes[c];
			};
			strcat(outbuf, tmp);
			free(tmp);
			break;
	};
	if (meslast != (char *)NULL)
		strcat(outbuf, meslast);
	return;
}

Pr_ret_t print_text_buffer_text(char *mes, SaHpiTextBufferT *buf, char *meslast,
	hpi_ui_print_cb_t proc)
{
	char	outbuf[SHOW_BUF_SZ];

	get_text_buffer_text(mes, buf, meslast, outbuf);
	return(proc(outbuf));
}

Pr_ret_t print_text_buffer_lang(char *mes, SaHpiTextBufferT *buf, char *meslast,
	hpi_ui_print_cb_t proc)
{
	char	*str;

	if ((buf->DataType == SAHPI_TL_TYPE_UNICODE) ||
		(buf->DataType == SAHPI_TL_TYPE_TEXT)) {
		str = oh_lookup_language(buf->Language);
		if (str == (char *)NULL) return(HPI_UI_OK);
		if (strlen(str) == 0) return(HPI_UI_OK);
		if (mes != (char *)NULL) {
			if (proc(mes) == HPI_UI_END) return(HPI_UI_END);
		};
		if (proc(str) != 0) return(1);
		if (meslast != (char *)NULL) {
			if (proc(meslast) == HPI_UI_END) return(HPI_UI_END);
		}
	};
	return(HPI_UI_OK);
}

Pr_ret_t print_text_buffer_length(char *mes, SaHpiTextBufferT *buf, char *meslast,
	hpi_ui_print_cb_t proc)
{
	char	len_buf[32];

	if (mes != (char *)NULL) {
		if (proc(mes) == HPI_UI_END) return(HPI_UI_END);
	};
	snprintf(len_buf, 31, "%d", buf->DataLength);
	if (proc(len_buf) == HPI_UI_END) return(HPI_UI_END);
	if (meslast != (char *)NULL) {
		if (proc(meslast) == HPI_UI_END) return(HPI_UI_END);
	};
	return(HPI_UI_OK);
}

Pr_ret_t print_text_buffer(char *mes, SaHpiTextBufferT *buf, char *meslast,
	hpi_ui_print_cb_t proc)
{
	if (mes != (char *)NULL) {
		if (proc(mes) == HPI_UI_END) return(HPI_UI_END);
	};
	if ((buf->DataLength > 0) || (buf->DataType == SAHPI_TL_TYPE_BINARY)) {
	if (print_text_buffer_type(NULL, buf, ": ", proc) != HPI_UI_OK)
		return(HPI_UI_END);
	if (print_text_buffer_lang(NULL, buf, ": ", proc) != HPI_UI_OK)
		return(HPI_UI_END);
	if (print_text_buffer_text(NULL, buf, NULL, proc) != HPI_UI_OK)
		return(HPI_UI_END);
	if (print_text_buffer_length(" (len=", buf, ")", proc) != HPI_UI_OK)
		return(HPI_UI_END);
	}
	if (meslast != (char *)NULL) {
		if (proc(meslast) == HPI_UI_END) return(HPI_UI_END);
	};
	return(HPI_UI_OK);
}


