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
 *      Kouzmich        < Mikhail.V.Kouzmich@intel.com >
 *
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <hpi_ui.h>

#define SHOW_BUF_SZ     1024

#define HPIBOOL2STR( x ) ( ( x == SAHPI_TRUE ) ? "TRUE" : "FALSE" )

extern char     *lookup_proc(int num, int val);
extern SaErrorT decode_proc(int num, void *val, char *buf, int bufsize);
extern SaErrorT decode1_proc(int num, int val, char *buf, int bufsize);
extern SaErrorT thres_value(SaHpiSensorReadingT *item, char *buf, int size);

static int is_ATCA(SaHpiSessionIdT sessionid, SaHpiResourceIdT resourceid)
{
        SaHpiRptEntryT          rpt_entry;
        SaHpiEntityPathT        *path;
        int                     i;

        if (saHpiRptEntryGetByResourceId(sessionid, resourceid, &rpt_entry) != SA_OK)
                return(0);
        path = &(rpt_entry.ResourceEntity);
        for (i = 0; i < SAHPI_MAX_ENTITY_PATH; i++)
                if (path->Entry[i].EntityType == SAHPI_ENT_ADVANCEDTCA_CHASSIS)
                        return(1);
        return(0);
}

SaErrorT sensor_list(SaHpiSessionIdT sessionid, hpi_ui_print_cb_t proc)
{
        SaErrorT                rv = SA_OK;
        Pr_ret_t                ret;
        SaHpiRptEntryT          rptentry;
        SaHpiEntryIdT           rptentryid;
        SaHpiEntryIdT           nextrptentryid;

        /* walk the RPT list */
        rptentryid = SAHPI_FIRST_ENTRY;
        while (rptentryid != SAHPI_LAST_ENTRY) {
                rv = saHpiRptEntryGet(sessionid, rptentryid, &nextrptentryid,
                        &rptentry);
                if (rv != SA_OK)
                        break;
                ret = show_sensor_list(sessionid, rptentry.ResourceId, proc);
                if (ret == HPI_UI_END)
                        return(SA_OK);
                rptentryid = nextrptentryid;
        };
        return(rv);
}

Pr_ret_t print_thres_value(SaHpiSensorReadingT *item, char *info,
        SaHpiSensorThdDefnT *def, int num, hpi_ui_print_cb_t proc)
{
        char    mes[256];
        char    buf[SHOW_BUF_SZ];
	int     i, j = 0;

        if (item->IsSupported != SAHPI_TRUE) {
                snprintf(buf, SHOW_BUF_SZ, "%s     not supported.\n", info);
                return(proc(buf));
        };
        strcpy(mes, info);
        if (def != (SaHpiSensorThdDefnT *)NULL) {
                if (def->ReadThold & num) {
                        if (def->WriteThold & num) strcat(mes, "(RW)");
                        else strcat(mes, "(RO)");
                } else {
                        if (def->WriteThold & num) strcat(mes, "(WO)");
                        else strcat(mes, "(NA)");
                }
        };
        switch (item->Type) {
                case SAHPI_SENSOR_READING_TYPE_INT64:
                        snprintf(buf, SHOW_BUF_SZ, "%s %lld\n", mes,
                                item->Value.SensorInt64);
                        break;
                case SAHPI_SENSOR_READING_TYPE_UINT64:
                        snprintf(buf, SHOW_BUF_SZ, "%s %llu\n", mes,
                                item->Value.SensorUint64);
                        break;
                case SAHPI_SENSOR_READING_TYPE_FLOAT64:
                        snprintf(buf, SHOW_BUF_SZ, "%s %10.3f\n", mes,
                                item->Value.SensorFloat64);
                        break;
                case SAHPI_SENSOR_READING_TYPE_BUFFER:
                        j = snprintf(buf, SHOW_BUF_SZ, "%s ", mes);
			for (i = 0; i < SAHPI_SENSOR_BUFFER_LENGTH; i++) {
				j = j + snprintf(buf + j, SHOW_BUF_SZ-j,"%02x", item->Value.SensorBuffer[i]);
				if (j >= SHOW_BUF_SZ)
					break;
			}
			if (j < SHOW_BUF_SZ)
				sprintf(buf + j, "\n");
                        break;
        };
        return(proc(buf));
}

int show_threshold(SaHpiSessionIdT sessionid, SaHpiResourceIdT resourceid,
        SaHpiSensorNumT sensornum, SaHpiSensorRecT *sen, hpi_ui_print_cb_t proc)
{
        SaErrorT                rv;
        SaHpiSensorThresholdsT  senstbuff;
        SaHpiSensorThdDefnT     *sendef;
        SaHpiSensorTypeT        type;
        SaHpiEventCategoryT     categ;
        char                    buf[SHOW_BUF_SZ];
        Pr_ret_t                res;

        sendef = &(sen->ThresholdDefn);
        rv = saHpiSensorTypeGet(sessionid, resourceid, sensornum, &type, &categ);
        if (rv != SA_OK) {
                snprintf(buf, SHOW_BUF_SZ, "ERROR: saHpiSensorTypeGet error = %s\n",
                        oh_lookup_error(rv));
                proc(buf);
                return -1;
        };
        if (categ != SAHPI_EC_THRESHOLD)
                return(SA_OK);
        if (sendef->IsAccessible == SAHPI_FALSE) {
                proc("Thresholds are not accessible.\n");
                return(SA_OK);
        }
        if (sendef->ReadThold == 0) {
                proc("Thresholds are not readable.\n");
                return(SA_OK);
        }

        memset(&senstbuff, 0, sizeof(SaHpiSensorThresholdsT));
        rv = saHpiSensorThresholdsGet(sessionid, resourceid, sensornum, &senstbuff);
        if (rv != SA_OK) {
                snprintf(buf, SHOW_BUF_SZ,
                        "ERROR: saHpiSensorThresholdsGet error = %s\n",
                        oh_lookup_error(rv));
                proc(buf);
                return -1;
        };
        res = print_thres_value(&(senstbuff.LowMinor), "Lower Minor Threshold",
                sendef, SAHPI_ES_LOWER_MINOR, proc);
        if (res == HPI_UI_END) return(HPI_UI_END);
        res = print_thres_value(&(senstbuff.LowMajor), "Lower Major Threshold",
                sendef, SAHPI_ES_LOWER_MAJOR, proc);
        if (res == HPI_UI_END) return(HPI_UI_END);
        res = print_thres_value(&(senstbuff.LowCritical), "Lower Critical Threshold",
                sendef, SAHPI_ES_LOWER_CRIT, proc);
        if (res == HPI_UI_END) return(HPI_UI_END);
        res = print_thres_value(&(senstbuff.UpMinor), "Upper Minor Threshold",
                sendef, SAHPI_ES_UPPER_MINOR, proc);
        if (res == HPI_UI_END) return(HPI_UI_END);
        res = print_thres_value(&(senstbuff.UpMajor), "Upper Major Threshold",
                sendef, SAHPI_ES_UPPER_MAJOR, proc);
        if (res == HPI_UI_END) return(HPI_UI_END);
        res = print_thres_value(&(senstbuff.UpCritical), "Upper Critical Threshold",
                sendef, SAHPI_ES_UPPER_CRIT, proc);
        if (res == HPI_UI_END) return(HPI_UI_END);
        res = print_thres_value(&(senstbuff.PosThdHysteresis),
                "Positive Threshold Hysteresis", NULL, 0, proc);
        if (res == HPI_UI_END) return(HPI_UI_END);
        res = print_thres_value(&(senstbuff.NegThdHysteresis),
                "Negative Threshold Hysteresis", NULL, 0, proc);
        return SA_OK;
}

SaErrorT show_control(SaHpiSessionIdT sessionid, SaHpiResourceIdT resourceid,
        SaHpiCtrlNumT num, hpi_ui_print_cb_t proc)
{
        SaErrorT                rv;
        SaHpiRdrT               rdr;
        SaHpiCtrlRecT           *ctrl;
        char                    *str, *str1;
        char                    buf[SHOW_BUF_SZ];
        char                    errbuf[SHOW_BUF_SZ];
        SaHpiCtrlTypeT          type;
        SaHpiCtrlRecDigitalT    *digit;
        SaHpiCtrlRecDiscreteT   *discr;
        SaHpiCtrlRecAnalogT     *analog;
        SaHpiCtrlRecStreamT     *stream;
        SaHpiCtrlRecTextT       *text;
        SaHpiCtrlRecOemT        *oem;
        int                     i;

        rv = saHpiRdrGetByInstrumentId(sessionid, resourceid, SAHPI_CTRL_RDR,
                num, &rdr);
        if (rv != SA_OK) {
                snprintf(errbuf, SHOW_BUF_SZ,
                        "\nERROR: saHpiRdrGetByInstrumentId: error: %s\n",
                        oh_lookup_error(rv));
                proc(errbuf);
                return(rv);
        };
        ctrl = &(rdr.RdrTypeUnion.CtrlRec);
        type = ctrl->Type;
        if (ctrl->WriteOnly) str = "(Write Only)";
        else str = " ";
        snprintf(buf, SHOW_BUF_SZ, "Control(%d/%d) Type: %s  %s  Output: %s\n",
                resourceid, num, oh_lookup_ctrltype(type), str,
                oh_lookup_ctrloutputtype(ctrl->OutputType));
        if (proc(buf) != HPI_UI_OK) return(SA_OK);
        if (ctrl->DefaultMode.ReadOnly) str = "(Read Only)";
        else str = " ";
        snprintf(buf, SHOW_BUF_SZ, "  Mode: %s  %s\n",
                oh_lookup_ctrlmode(ctrl->DefaultMode.Mode), str);
        if (proc(buf) != HPI_UI_OK) return(SA_OK);

        if (proc("Data:\n") != HPI_UI_OK) return(SA_OK);
        switch (type) {
                case SAHPI_CTRL_TYPE_DIGITAL:
                        digit = &(ctrl->TypeUnion.Digital);
                        str = oh_lookup_ctrlstatedigital(digit->Default);
                        if (str == (char *)NULL) {
                                snprintf(errbuf, SHOW_BUF_SZ, "Invalid value (0x%x)",
                                        digit->Default);
                                str = errbuf;
                        };
                        snprintf(buf, SHOW_BUF_SZ, "\tDefault: %s\n", str);
                        break;
                case SAHPI_CTRL_TYPE_DISCRETE:
                        discr = &(ctrl->TypeUnion.Discrete);
                        snprintf(buf, SHOW_BUF_SZ, "\tDefault: %d\n", discr->Default);
                        break;
                case SAHPI_CTRL_TYPE_ANALOG:
                        analog = &(ctrl->TypeUnion.Analog);
                        snprintf(buf, SHOW_BUF_SZ,
                                "\tDefault: %d  (min = %d  max = %d)\n",
                                analog->Default, analog->Min, analog->Max);
                        break;
                case SAHPI_CTRL_TYPE_STREAM:
                        stream = &(ctrl->TypeUnion.Stream);
                        snprintf(buf, SHOW_BUF_SZ,
                                "\tDefault: Repeat = %d  lendth = %d  stream = %s\n",
                                stream->Default.Repeat, stream->Default.StreamLength,
                                stream->Default.Stream);
                        break;
                case SAHPI_CTRL_TYPE_TEXT:
                        text = &(ctrl->TypeUnion.Text);
                        snprintf(buf, SHOW_BUF_SZ, "\tMaxChars = %d  MaxLines = %d\n",
                                text->MaxChars, text->MaxLines);
                        if (proc(buf) != HPI_UI_OK) return(SA_OK);
                        str = oh_lookup_language(text->Language);
                        if (str == (char *)NULL) str = "UNKNOWN";
                        str1 = oh_lookup_texttype(text->DataType);
                        if (str1 == (char *)NULL) str1 = "UNKNOWN";
                        snprintf(buf, SHOW_BUF_SZ,
                                "\tLanguage = %s  DataType = %s\n", str, str1);
                        if (proc(buf) != HPI_UI_OK) return(SA_OK);
                        snprintf(buf, SHOW_BUF_SZ, "\tDefault: Line # = %d",
                                text->Default.Line);
                        if (proc(buf) != HPI_UI_OK) return(SA_OK);
                        print_text_buffer_text("  Text = ", &(text->Default.Text),
                                "\n", proc);
                        return SA_OK;
                case SAHPI_CTRL_TYPE_OEM:
                        oem = &(ctrl->TypeUnion.Oem);
                        snprintf(buf, SHOW_BUF_SZ, "\tMId = %d  Config data = ",
                                oem->MId);
                        proc(buf);
                        str = (char *)(oem->ConfigData);
                        for (i = 0; i < SAHPI_CTRL_OEM_CONFIG_LENGTH; i++)
                                sprintf(buf + i * 3, "%2.2X ", (unsigned char)(str[i]));
                        strcat(buf, "\n\t");
                        if (proc(buf) != HPI_UI_OK) return(SA_OK);
                        sprintf(buf, "Default: MId = %d  Body = ", oem->MId);
                        str = (char *)(oem->Default.Body);
                        for (i = 0; i < oem->Default.BodyLength; i++)
                                sprintf(buf + i * 3, "%2.2X ", (unsigned char)(str[i]));
                        strcat(buf, "\n");
                        break;
                default: strcpy(buf, "Unknown control type\n");
        };
        proc(buf);
        return SA_OK;
}



SaErrorT show_control_state(SaHpiSessionIdT sessionid, SaHpiResourceIdT resourceid,
        SaHpiCtrlNumT num, hpi_ui_print_cb_t proc,
        get_int_param_t get_int_param)
{
        SaErrorT                rv;
        int                     i;
        int                     res;
        char                    *str;
        char                    buf[SHOW_BUF_SZ];
        char                    errbuf[SHOW_BUF_SZ];
        SaHpiCtrlModeT          mode;
        SaHpiCtrlStateT         state;
        SaHpiCtrlTypeT          type;
        SaHpiCtrlStateDigitalT  digit;
        SaHpiCtrlStateDiscreteT discr;
        SaHpiCtrlStateAnalogT   analog;
        SaHpiCtrlStateStreamT   *stream;
        SaHpiCtrlStateTextT     *text;
        SaHpiCtrlStateOemT      *oem;

        rv = saHpiControlTypeGet(sessionid, resourceid, num, &type);
        if (rv != SA_OK) {
                snprintf(errbuf, SHOW_BUF_SZ,
                        "\nERROR: saHpiControlTypeGet: error: %s\n", oh_lookup_error(rv));
                proc(errbuf);
                return(rv);
        };

        state.Type = type;
        if (type == SAHPI_CTRL_TYPE_TEXT) {
                i = get_int_param("Line #(0 == all): ", &res);
                if (i != 1) {
                        printf("Invalid value\n");
                        return SA_ERR_HPI_ERROR;
                };
                state.StateUnion.Text.Line = res;
        }
        rv = saHpiControlGet(sessionid, resourceid, num, &mode, &state);
        if (rv != SA_OK) {
                snprintf(errbuf, SHOW_BUF_SZ,
                        "\nERROR: saHpiControlGet: error: %s\n", oh_lookup_error(rv));
                proc(errbuf);
                return(rv);
        };
        type = state.Type;
        snprintf(buf, SHOW_BUF_SZ, "Control(%d/%d) %s State: ",
                resourceid, num, oh_lookup_ctrlmode(mode));
        if (proc(buf) != HPI_UI_OK) return(SA_OK);

        switch (type) {
                case SAHPI_CTRL_TYPE_DIGITAL:
                        digit = state.StateUnion.Digital;
                        str = oh_lookup_ctrlstatedigital(digit);
                        if (str == (char *)NULL) {
                                snprintf(errbuf, SHOW_BUF_SZ, "Invalid value (0x%x)",
                                        digit);
                                str = errbuf;
                        };
                        snprintf(buf, SHOW_BUF_SZ, "%s\n", str);
                        break;
                case SAHPI_CTRL_TYPE_DISCRETE:
                        discr = state.StateUnion.Discrete;
                        snprintf(buf, SHOW_BUF_SZ, "%d\n", discr);
                        break;
                case SAHPI_CTRL_TYPE_ANALOG:
                        analog = state.StateUnion.Analog;
                        snprintf(buf, SHOW_BUF_SZ, "%d\n", analog);
                        break;
                case SAHPI_CTRL_TYPE_STREAM:
                        stream = &(state.StateUnion.Stream);
                        snprintf(buf, SHOW_BUF_SZ,
                                "Repeat = %d  lendth = %d  stream = %s\n",
                                stream->Repeat, stream->StreamLength, stream->Stream);
                        break;
                case SAHPI_CTRL_TYPE_TEXT:
                        text = &(state.StateUnion.Text);
                        snprintf(buf, SHOW_BUF_SZ, "Line # = %d", text->Line);
                        if (proc(buf) != HPI_UI_OK) return(SA_OK);
                        print_text_buffer_text("  Text = ", &(text->Text), "\n", proc);
                        return SA_OK;
                case SAHPI_CTRL_TYPE_OEM:
                        oem = &(state.StateUnion.Oem);
                        str = (char *)(oem->Body);
                        if (is_ATCA(sessionid, resourceid) &&
                                (oem->MId == ATCAHPI_PICMG_MID))
                                snprintf(buf, SHOW_BUF_SZ,
                                        "MId = %d  Color = %s  Body = ",
                                        oem->MId, oh_lookup_atcahpiledcolor(str[2]));
                        else
                                snprintf(buf, SHOW_BUF_SZ, "MId = %d  Body = ", oem->MId);
                        proc(buf);
                        for (i = 0; i < oem->BodyLength; i++)
                                sprintf(buf + i * 3, "%2.2X ", (unsigned char)(str[i]));
                        strcat(buf, "\n");
                        break;
                default: strcpy(buf, "Unknown control type\n");
        };
        proc(buf);
        return SA_OK;
}

SaErrorT show_sensor(SaHpiSessionIdT sessionid, SaHpiResourceIdT resourceid,
        SaHpiSensorNumT sensornum, hpi_ui_print_cb_t proc)
{
        SaHpiSensorReadingT     reading;
        SaHpiEventStateT        status, assert, deassert;
        SaHpiRdrT               rdr;
        SaErrorT                rv;
        SaHpiBoolT              val;
        char                    buf[SHOW_BUF_SZ];
        char                    errbuf[SHOW_BUF_SZ];
        Pr_ret_t                res;

        rv = saHpiRdrGetByInstrumentId(sessionid, resourceid, SAHPI_SENSOR_RDR,
                sensornum, &rdr);
        if (rv != SA_OK) {
                snprintf(errbuf, SHOW_BUF_SZ,
                        "\nERROR: saHpiRdrGetByInstrumentId: error: %s\n",
                        oh_lookup_error(rv));
                proc(errbuf);
                return(rv);
        };
        snprintf(buf, SHOW_BUF_SZ, "Sensor(%d/%d) %s", resourceid, sensornum,
                oh_lookup_sensortype(rdr.RdrTypeUnion.SensorRec.Type));
        proc(buf);
        res = print_text_buffer_text("  ", &(rdr.IdString), "\n", proc);
        if (res != HPI_UI_OK) return(SA_OK);
        rv = saHpiSensorEnableGet(sessionid, resourceid, sensornum, &val);
        if (rv != SA_OK) {
                snprintf(errbuf, SHOW_BUF_SZ,
                        "\nERROR: saHpiSensorEnableGet: error: %s\n",
                        oh_lookup_error(rv));
                if (proc(errbuf) != HPI_UI_OK) return(rv);
        } else {
                if (val) res = proc("Enable ");
                else res = proc("Disable ");
                if (res != HPI_UI_OK) return(SA_OK);
        };
        rv = saHpiSensorEventEnableGet(sessionid, resourceid, sensornum, &val);
        if (rv != SA_OK) {
                snprintf(errbuf, SHOW_BUF_SZ,
                        "\nERROR: saHpiSensorEventEnableGet: error: %s\n",
                        oh_lookup_error(rv));
                if (proc(errbuf) != HPI_UI_OK) return(rv);
        } else {
                if (proc("    event : ") != HPI_UI_OK) return(SA_OK);
                if (val) res = proc("Enable");
                else res = proc("Disable");
                if (res != HPI_UI_OK) return(SA_OK);
        };
        rv = saHpiSensorEventMasksGet(sessionid, resourceid, sensornum,
                &assert, &deassert);
        if (rv != SA_OK) {
                snprintf(errbuf, SHOW_BUF_SZ,
                        "\nERROR: saHpiSensorEventMasksGet: error: %s\n",
                        oh_lookup_error(rv));
                if (proc(errbuf) != HPI_UI_OK) return(rv);
        } else {
                snprintf(buf, SHOW_BUF_SZ,
                        "   supported: 0x%4.4x  masks: assert = 0x%4.4x"
                        "   deassert = 0x%4.4x\n",
                        rdr.RdrTypeUnion.SensorRec.Events, assert, deassert);
                if (proc(buf) != HPI_UI_OK) return(rv);
        };
        rv = saHpiSensorReadingGet(sessionid, resourceid, sensornum,
                &reading, &status);
        if (rv != SA_OK) {
                snprintf(errbuf, SHOW_BUF_SZ,
                        "\nERROR: saHpiSensorReadingGet: error: %s\n",
                        oh_lookup_error(rv));
                proc(errbuf);
                return(rv);
        };
        snprintf(buf, SHOW_BUF_SZ, "\tEvent states = 0x%x\n", status);
        if (proc(buf) != HPI_UI_OK) return(SA_OK);
        if (reading.IsSupported) {
                res = print_thres_value(&reading, "\tReading Value =",
                        NULL, 0, proc);
                if (res == HPI_UI_END) return(SA_OK);
        } else {
                if (proc("\tReading not supported\n") != HPI_UI_OK) return(SA_OK);
        }

        show_threshold(sessionid, resourceid, sensornum,
                &(rdr.RdrTypeUnion.SensorRec), proc);

        return SA_OK;
}

SaErrorT show_event_log(SaHpiSessionIdT sessionid, SaHpiResourceIdT resourceid,
        int show_short, hpi_ui_print_cb_t proc)
{
        SaErrorT                rv = SA_OK;
        SaHpiRptEntryT          rptentry;
        SaHpiEventLogInfoT      info;
        SaHpiEventLogEntryIdT   entryid;
        SaHpiEventLogEntryIdT   nextentryid;
        SaHpiEventLogEntryIdT   preventryid;
        SaHpiEventLogEntryT     sel;
        SaHpiRdrT               rdr;
        SaHpiRptEntryT          rpt;
        char                    buf[SHOW_BUF_SZ];
        char                    date[30], date1[30];

        if (resourceid != SAHPI_UNSPECIFIED_RESOURCE_ID) {
                rv = saHpiRptEntryGetByResourceId(sessionid, resourceid, &rptentry);
                if (rv != SA_OK) {
                        snprintf(buf, SHOW_BUF_SZ,
                                "ERROR: saHpiRptEntryGetByResourceId error = %s\n",
                                oh_lookup_error(rv));
                        proc(buf);
                        return rv;
                };
                if (!(rptentry.ResourceCapabilities & SAHPI_CAPABILITY_EVENT_LOG)) {
                        proc("ERROR: The designated resource hasn't SEL.\n");
                        return SA_OK;
                }
        };

        rv = saHpiEventLogInfoGet(sessionid, resourceid, &info);
        if (rv != SA_OK) {
                snprintf(buf, SHOW_BUF_SZ, "ERROR: saHpiEventLogInfoGet error = %s\n",
                        oh_lookup_error(rv));
                proc(buf);
                return rv;
        }
        snprintf(buf, SHOW_BUF_SZ, "EventLog: entries = %d, size = %d, enabled = %d\n",
                info.Entries, info.Size, info.Enabled);
        if (proc(buf) != HPI_UI_OK) return(SA_OK);
        time2str(info.UpdateTimestamp, date, 30);
        time2str(info.CurrentTime, date1, 30);
        snprintf(buf, SHOW_BUF_SZ, "UpdateTime = %s  CurrentTime = %s  Overflow = %d\n",
                date, date1, info.OverflowFlag);
        if (proc(buf) != HPI_UI_OK) return(SA_OK);

        if (info.Entries != 0){
                entryid = SAHPI_OLDEST_ENTRY;
                while (entryid != SAHPI_NO_MORE_ENTRIES)
                {
                        rv = saHpiEventLogEntryGet(sessionid, resourceid,
                                        entryid, &preventryid, &nextentryid,
                                        &sel, &rdr, &rpt);
                        if (rv != SA_OK) {
                                snprintf(buf, SHOW_BUF_SZ,
                                        "ERROR: saHpiEventLogEntryGet error = %s\n",
                                        oh_lookup_error(rv));
                                proc(buf);
                                return -1;
                        };
                        if (show_short) {
                                if (show_short_event(&(sel.Event), proc) != HPI_UI_OK)
                                        return(SA_OK);
                        } else {
                                if (rpt.ResourceCapabilities != 0) {
                                        oh_print_eventlogentry(&sel, &rpt.ResourceEntity, 1);
                                } else if (rdr.RdrType != SAHPI_NO_RECORD) {
                                        oh_print_eventlogentry(&sel, &rdr.Entity, 1);
                                } else {
                                        oh_print_eventlogentry(&sel, NULL, 1);
                                }
                        }


                        preventryid = entryid;
                        entryid = nextentryid;
                }
        } else {
                proc("SEL is empty\n");
        };
        return SA_OK;
}

Pr_ret_t show_sensor_list(SaHpiSessionIdT sessionid, SaHpiResourceIdT resourceid,
                hpi_ui_print_cb_t proc)
{
        SaErrorT        rv = SA_OK;
        SaHpiEntryIdT   entryid;
        SaHpiEntryIdT   nextentryid;
        SaHpiRdrT       rdr;
        char            buf[SHOW_BUF_SZ];

        entryid = SAHPI_FIRST_ENTRY;
        while (entryid != SAHPI_LAST_ENTRY) {
                rv = saHpiRdrGet(sessionid, resourceid, entryid, &nextentryid, &rdr);
                if (rv != SA_OK)
                        break;
                if (rdr.RdrType == SAHPI_SENSOR_RDR) {
                        snprintf(buf, 256, "Resource Id: %d, Sensor Num: %d",
                                resourceid, rdr.RdrTypeUnion.SensorRec.Num);
                        if (proc(buf) != 0) return(HPI_UI_END);
                        if (print_text_buffer_text(", Tag: ", &(rdr.IdString),
                                NULL, proc) != 0)
                                return(-1);
                        if (proc("\n") != 0) return(HPI_UI_END);
                };
                entryid = nextentryid;
        };
        return(HPI_UI_OK);
}

int show_rdr_list(Domain_t *domain, SaHpiResourceIdT rptid, SaHpiRdrTypeT passed_type,
        hpi_ui_print_cb_t proc)
//  return: list size
{
        SaHpiRdrT               rdr;
        SaHpiEntryIdT           entryid;
        SaHpiEntryIdT           nextentryid;
        char                    buf[SHOW_BUF_SZ];
        SaHpiRdrTypeT           type;
        char                    ar[256];
        SaHpiSensorRecT         *sensor;
        SaErrorT                ret;
        int                     res_num = 0;

        entryid = SAHPI_FIRST_ENTRY;
        while (entryid !=SAHPI_LAST_ENTRY) {
		memset(buf, 0, SHOW_BUF_SZ);
		memset(ar, 0, 256);
                ret = saHpiRdrGet(domain->sessionId, rptid, entryid,
                        &nextentryid, &rdr);
                if (ret != SA_OK)
                        return(res_num);
                type = rdr.RdrType;
                if ((passed_type != SAHPI_NO_RECORD) && (type != passed_type)) {
                        entryid = nextentryid;
                        continue;
                };
                snprintf(buf, SHOW_BUF_SZ, "(%3.3d): %s ID=%u",
		         oh_get_rdr_num(rdr.RecordId),
			 oh_lookup_rdrtype(type), rdr.RecordId);
                switch (type) {
                        case SAHPI_SENSOR_RDR:
                                sensor = &(rdr.RdrTypeUnion.SensorRec);
                                snprintf(ar, 256, ", Ctrl=%d, EvtCtrl=",
                                         sensor->EnableCtrl);
                                switch (sensor->EventCtrl) {
                                        case SAHPI_SEC_PER_EVENT:
                                                strcat(ar, "WR"); break;
                                        case SAHPI_SEC_READ_ONLY_MASKS:
                                                strcat(ar, "RM"); break;
                                        default:
                                                strcat(ar, "RO"); break;
                                };
                                break;
                        case SAHPI_CTRL_RDR:
                                break;
                        case SAHPI_INVENTORY_RDR:
                                break;
                        case SAHPI_WATCHDOG_RDR:
                                break;
                        case SAHPI_ANNUNCIATOR_RDR:
                                break;
                        case SAHPI_DIMI_RDR:
                                break;
                        case SAHPI_FUMI_RDR:
                                break;
                        default:
                                snprintf(ar, 256, ", Unrecognized RDR Type");
                };
                strcat(buf, ar);
                res_num++;
                if (proc(buf) != HPI_UI_OK) return(res_num);
                if (print_text_buffer_text(", Tag=", &(rdr.IdString),
                        "\n", proc) != HPI_UI_OK)
                        return(res_num);
                entryid = nextentryid;
        };
        return(res_num);
}

typedef struct {
        int                     len_buf;
        char                    outbuf[SHOW_BUF_SZ];
        SaHpiEntityPathT        path;
} rpt_outbuf_t;

static int lsres_sort(void *st1, void *st2)
{
        int             i, n1, n2;
        rpt_outbuf_t    *ar1 = (rpt_outbuf_t *)st1;
        rpt_outbuf_t    *ar2 = (rpt_outbuf_t *)st2;

        for (i = 0; i < SAHPI_MAX_ENTITY_PATH; i++)
                if (ar1->path.Entry[i].EntityType == SAHPI_ENT_ROOT) break;
        n1 = i;
        for (i = 0; i < SAHPI_MAX_ENTITY_PATH; i++)
                if (ar2->path.Entry[i].EntityType == SAHPI_ENT_ROOT) break;
        n2 = i;
        while ((n1 >= 0) && (n2 >= 0)) {
                if (ar1->path.Entry[n1].EntityType > ar2->path.Entry[n2].EntityType)
                        return(1);
                if (ar1->path.Entry[n1].EntityType < ar2->path.Entry[n2].EntityType)
                        return(-1);
                if (ar1->path.Entry[n1].EntityLocation >
                        ar2->path.Entry[n2].EntityLocation)
                        return(1);
                if (ar1->path.Entry[n1].EntityLocation <
                        ar2->path.Entry[n2].EntityLocation)
                        return(-1);
                n1--;
                n2--;
        };
        if (n1 >= 0) return(1);
        if (n2 >= 0) return(-1);
        return(0);
}

static void print_rpt_paths(rpt_outbuf_t *ar, int len, hpi_ui_print_cb_t proc)
{
        int                     i, max_len = 0;
        char                    buf[SHOW_BUF_SZ];
        oh_big_textbuffer       tmpbuf;
        SaErrorT                rv;

        for (i = 0; i < len; i++) {
                if (ar[i].len_buf > max_len)
                        max_len = ar[i].len_buf;
        };
        qsort(ar, len, sizeof(rpt_outbuf_t),
                (int(*)(const void *, const void *))lsres_sort);
        for (i = 0; i < len; i++) {
                if (ar[i].len_buf == 0) continue;
                memset(buf, ' ', SHOW_BUF_SZ);
                strncpy(buf, ar[i].outbuf, ar[i].len_buf);
                buf[max_len + 1] = 0;
                strcat(buf, ": ");
                rv = oh_decode_entitypath(&(ar[i].path), &tmpbuf);
                if (rv == SA_OK) {
                        strcat(buf, (char *)(tmpbuf.Data));
                };
                strcat(buf, "\n");
                if (proc(buf) != HPI_UI_OK) return;
        }
}

int show_rpt_list(Domain_t *domain, int as, SaHpiResourceIdT rptid,
        int addedfields, hpi_ui_print_cb_t proc)
/*  as : SHOW_ALL_RPT  -  show all rpt entry only
 *       SHOW_ALL_RDR  -  show all rdr for all rpt
 *       SHOW_RPT_RDR  -  show all rdr for rptid
 *  addedfields : SHORT_LSRES - traditional resource list
 *                STATE_LSRES - show resource status
 *                PATH_LSRES  - show entity path
 *  return: list size
 */
{
        SaHpiRptEntryT          rpt_entry;
        SaHpiEntryIdT           rptentryid, nextrptentryid;
        int                     ind = 0, show_path;
        char                    buf[SHOW_BUF_SZ];
        SaErrorT                rv;
        SaHpiCapabilitiesT      cap;
        SaHpiHsCapabilitiesT    hscap;
        SaHpiHsStateT           state;
        rpt_outbuf_t            *rpt_out = NULL, *tmp;
        int                     res_num = 0, n_rpt = 0, max_rpt = 0;

        if (as != SHOW_ALL_RPT) show_path = 0;
        else show_path = addedfields & PATH_LSRES;
        rptentryid = SAHPI_FIRST_ENTRY;
        while (rptentryid != SAHPI_LAST_ENTRY) {
                rv = saHpiRptEntryGet(domain->sessionId, rptentryid, &nextrptentryid,
                        &rpt_entry);
                if (rv != SA_OK) break;
                if ((as == SHOW_RPT_RDR) && (rpt_entry.ResourceId != rptid)) {
                        rptentryid = nextrptentryid;
                        continue;
                };
                res_num++;
                snprintf(buf, SHOW_BUF_SZ, "(%3.3d):", rpt_entry.ResourceId);
                get_text_buffer_text(NULL, &(rpt_entry.ResourceTag),
                        ":", buf + strlen(buf));
                strcat(buf, "{");
                cap = rpt_entry.ResourceCapabilities;
                if (cap & SAHPI_CAPABILITY_SENSOR) strcat(buf, "S|");
                if (cap & SAHPI_CAPABILITY_RDR) strcat(buf, "RDR|");
                if (cap & SAHPI_CAPABILITY_EVENT_LOG) strcat(buf, "ELOG|");
                if (cap & SAHPI_CAPABILITY_INVENTORY_DATA) strcat(buf, "INV|");
                if (cap & SAHPI_CAPABILITY_RESET) strcat(buf, "RST|");
                if (cap & SAHPI_CAPABILITY_POWER) strcat(buf, "PWR|");
                if (cap & SAHPI_CAPABILITY_ANNUNCIATOR) strcat(buf, "AN|");
                if (cap & SAHPI_CAPABILITY_FRU) strcat(buf, "FRU|");
                if (cap & SAHPI_CAPABILITY_CONTROL) strcat(buf, "CNT|");
                if (cap & SAHPI_CAPABILITY_WATCHDOG) strcat(buf, "WTD|");
                if (cap & SAHPI_CAPABILITY_MANAGED_HOTSWAP) strcat(buf, "HS|");
                if (cap & SAHPI_CAPABILITY_CONFIGURATION) strcat(buf, "CF |");
                if (cap & SAHPI_CAPABILITY_AGGREGATE_STATUS) strcat(buf, "AG|");
                if (cap & SAHPI_CAPABILITY_EVT_DEASSERTS) strcat(buf, "DS|");
                if (cap & SAHPI_CAPABILITY_RESOURCE) strcat(buf, "RES|");
                if (cap & SAHPI_CAPABILITY_DIMI) strcat(buf, "DIMI|");
                if (cap & SAHPI_CAPABILITY_FUMI) strcat(buf, "FUMI|");
                ind  = strlen(buf);
                if (buf[ind - 1] == '|')
                        buf[ind - 1] = 0;
                strcat(buf, "}");
                if (addedfields & STATE_LSRES) {
                        rv = saHpiHotSwapStateGet(domain->sessionId,
                                rpt_entry.ResourceId, &state);
                        hscap = rpt_entry.HotSwapCapabilities;
                        if ((rv == SA_OK) || (hscap != 0)) {
                                strcat(buf, "  HS={");
                                if (rv == SA_OK)
                                        strcat(buf, oh_lookup_hsstate(state));
                                if (hscap & SAHPI_HS_CAPABILITY_AUTOEXTRACT_READ_ONLY)
                                        strcat(buf, " RO|");
                                if (hscap & SAHPI_HS_CAPABILITY_INDICATOR_SUPPORTED)
                                        strcat(buf, " IND|");
                                ind  = strlen(buf);
                                if (buf[ind - 1] == '|')
                                        buf[ind - 1] = 0;
                                strcat(buf, "}");
                        }
                };
                if (show_path) {
                        if (n_rpt >= max_rpt) {
                                max_rpt += 10;
                                tmp = (rpt_outbuf_t *)malloc(sizeof(rpt_outbuf_t) *
                                                max_rpt);
                                memset(tmp, 0, sizeof(rpt_outbuf_t) * max_rpt);
                                if (n_rpt > 0) {
                                        memcpy(tmp, rpt_out,
                                                sizeof(rpt_outbuf_t) * n_rpt);
                                        free(rpt_out);
                                };
                                rpt_out = tmp;
                        };
                        tmp = rpt_out + n_rpt;
                        tmp->len_buf = strlen(buf);
                        if (tmp->len_buf > 0) {
                                strcpy(tmp->outbuf, buf);
                                tmp->path = rpt_entry.ResourceEntity;
                                n_rpt++;
                        }
                } else {
                        strcat(buf, "\n");
                        if (proc(buf) != HPI_UI_OK) return(res_num);
                        if (as == SHOW_ALL_RDR)
                                show_rdr_list(domain, rpt_entry.ResourceId,
                                        SAHPI_NO_RECORD, proc);
                };
                rptentryid = nextrptentryid;
        };
        if (show_path) {
                print_rpt_paths(rpt_out, n_rpt, proc);
                free(rpt_out);
        };
        return(res_num);
}

static Pr_ret_t show_attrs(Attributes_t *Attrs, int delta, hpi_ui_print_cb_t proc)
{
        int             i, type, len, del;
        char            tmp[256], *name;
        char            buf[SHOW_BUF_SZ];
        union_type_t    val;
        SaErrorT        rv;
        Pr_ret_t        ret;

        memset(buf, ' ', SHOW_BUF_SZ);
        del = delta << 1;
        len = SHOW_BUF_SZ - del;
        for (i = 0; i < Attrs->n_attrs; i++) {
                name = get_attr_name(Attrs, i);
                if (name == (char *)NULL)
                        break;
                rv = get_value(Attrs, i, &val);
                if (rv != SA_OK) continue;
                type = get_attr_type(Attrs, i);
                switch (type) {
                        case NO_TYPE:   continue;
                        case STRUCT_TYPE:
                                snprintf(buf + del, len, "%s:\n", name);
                                if (proc(buf) != 0) return(-1);
                                rv = get_value(Attrs, i, &val);
                                if (rv != SA_OK) continue;
                                ret = show_attrs((Attributes_t *)(val.a),
                                        delta + 1, proc);
                                if (ret != HPI_UI_OK) return(HPI_UI_END);
                                continue;
                        case LOOKUP_TYPE:
                                strncpy(tmp, lookup_proc(Attrs->Attrs[i].lunum,
                                        val.i), 256);
                                break;
                        case DECODE_TYPE:
                                rv = decode_proc(Attrs->Attrs[i].lunum, val.a,
                                        tmp, 256);
                                if (rv != SA_OK) continue;
                                break;
                        case DECODE1_TYPE:
                                rv = decode1_proc(Attrs->Attrs[i].lunum, val.i,
                                        tmp, 256);
                                if (rv != SA_OK) continue;
                                break;
                        case READING_TYPE:
                                if (thres_value(val.a, tmp, 256) != SA_OK)
                                        continue;
                                break;
                        case TEXT_BUFF_TYPE:
                                snprintf(buf + del, len, "%s: ", name);
                                if (proc(buf) != HPI_UI_OK) return(HPI_UI_END);
                                ret = print_text_buffer(NULL, val.a, "\n", proc);
                                if (ret != HPI_UI_OK) return(HPI_UI_END);
                                continue;
                        default:
                                rv = get_value_as_string(Attrs, i, tmp, 256);
                                if (rv != SA_OK) continue;
                };
                snprintf(buf + del, len, "%s: %s\n", name, tmp);
                if (proc(buf) != HPI_UI_OK) return(HPI_UI_END);
        };
        return(0);
}

SaErrorT show_Rpt(Rpt_t *Rpt, hpi_ui_print_cb_t proc)
{
        show_attrs(&(Rpt->Attrutes), 0, proc);
        return(SA_OK);
}

SaErrorT show_Rdr(Rdr_t *Rdr, hpi_ui_print_cb_t proc)
{
        show_attrs(&(Rdr->Attrutes), 0, proc);
        return(SA_OK);
}

Pr_ret_t show_short_event(SaHpiEventT *event, hpi_ui_print_cb_t proc)
{
        SaHpiTextBufferT        tmbuf;
        SaHpiSensorEventT       *sen;
        SaHpiDomainEventT       *dom;
        SaErrorT                rv;
        char                    buf[SHOW_BUF_SZ], buf1[32];
        char                    *str, *str1;

        rv = oh_decode_time(event->Timestamp, &tmbuf);
        if (rv)
                snprintf(buf, SHOW_BUF_SZ, "%19s ", "TIME UNSPECIFIED");
        else
                snprintf(buf, SHOW_BUF_SZ, "%19s ", tmbuf.Data);
        proc(buf);
        snprintf(buf, SHOW_BUF_SZ, "%s ", oh_lookup_eventtype(event->EventType));
        proc(buf);
        switch (event->EventType) {
                case SAHPI_ET_DOMAIN:
                        dom = &(event->EventDataUnion.DomainEvent);
                        snprintf(buf, SHOW_BUF_SZ, "  Event: %s  DomainId: %d",
                                oh_lookup_domaineventtype(dom->Type), dom->DomainId);
                        proc(buf);
                        break;
                case SAHPI_ET_SENSOR:
                        sen = &(event->EventDataUnion.SensorEvent);
                        if (sen->Assertion == SAHPI_TRUE) str = "ASSERTED";
                        else str = "DEASSERTED";
                        rv = oh_decode_eventstate(sen->EventState,  sen->EventCategory,
                                   &tmbuf);
                        if (rv != SA_OK) {
                                snprintf(buf1, 32, "STATE(%4.4x)", sen->EventState);
                                str1 = buf1;
                        } else
                                str1 = (char *)(tmbuf.Data);
                        snprintf(buf, SHOW_BUF_SZ, "%s %d/%d %s %s %s:%s",
                                oh_lookup_sensortype(sen->SensorType),
                                event->Source, sen->SensorNum,
                                oh_lookup_severity(event->Severity),
                                oh_lookup_eventcategory(sen->EventCategory), str1, str);
                        proc(buf);
                        break;
                case SAHPI_ET_RESOURCE:
                        snprintf(buf, SHOW_BUF_SZ, "%d %s %s",
                                event->Source, oh_lookup_severity(event->Severity),
                                oh_lookup_resourceeventtype(event->EventDataUnion.
                                        ResourceEvent.ResourceEventType));
                        proc(buf);
                        break;
                case SAHPI_ET_HOTSWAP:
                        snprintf(buf, SHOW_BUF_SZ, "%d %s %s -> %s",
                                event->Source, oh_lookup_severity(event->Severity),
                                oh_lookup_hsstate(
                                        event->EventDataUnion.HotSwapEvent.
                                                PreviousHotSwapState),
                                oh_lookup_hsstate(
                                event->EventDataUnion.HotSwapEvent.HotSwapState));
                        if (proc(buf) != HPI_UI_OK) return(HPI_UI_END);
                        break;
                case SAHPI_ET_DIMI:
                        snprintf(buf, SHOW_BUF_SZ, "RESOURCE %d DIMI %d TEST %d : %s",
                                event->Source,
                                event->EventDataUnion.DimiEvent.DimiNum,
                                event->EventDataUnion.DimiEvent.TestNum,
                                oh_lookup_dimitestrunstatus(event->EventDataUnion.DimiEvent.DimiTestRunStatus));
                        proc(buf);
                        break;
                case SAHPI_ET_DIMI_UPDATE:
                        snprintf(buf, SHOW_BUF_SZ, "RESOURCE %d DIMI %d",
                                event->Source,
                                event->EventDataUnion.DimiUpdateEvent.DimiNum);
                        proc(buf);
                        break;
                case SAHPI_ET_FUMI:
                        snprintf(buf, SHOW_BUF_SZ, "RESOURCE %d FUMI %d BANK %d : %s",
                                event->Source,
                                event->EventDataUnion.FumiEvent.FumiNum,
                                event->EventDataUnion.FumiEvent.BankNum,
                                oh_lookup_fumiupgradestatus(event->EventDataUnion.FumiEvent.UpgradeStatus));
                        proc(buf);
                        break;
                default:
                        snprintf(buf, SHOW_BUF_SZ, "%d", event->Source);
                        proc(buf);
                        break;
        };
        return(proc("\n"));
}

SaErrorT show_dat(Domain_t *domain, hpi_ui_print_cb_t proc)
{
        SaHpiAlarmT     alarm;
        SaErrorT        rv = SA_OK;
        char            buf[SHOW_BUF_SZ];
        char            time[256];
        int             ind;
        int             first = 1;

        alarm.AlarmId = SAHPI_FIRST_ENTRY;
        while (rv == SA_OK) {
                rv = saHpiAlarmGetNext(domain->sessionId, SAHPI_ALL_SEVERITIES, FALSE,
                        &alarm);
                if (rv != SA_OK) break;
                first = 0;
                snprintf(buf, SHOW_BUF_SZ, "(%d) ", alarm.AlarmId);
                time2str(alarm.Timestamp, time, 256);
                strcat(buf, time);
                strcat(buf, " ");
                strcat(buf, oh_lookup_severity(alarm.Severity));
                if (alarm.Acknowledged) strcat(buf, " a ");
                else strcat(buf, " - ");
                strcat(buf, oh_lookup_statuscondtype(alarm.AlarmCond.Type));
                ind = strlen(buf);
                if (alarm.AlarmCond.Type == SAHPI_STATUS_COND_TYPE_SENSOR) {
                        snprintf(buf + ind, SHOW_BUF_SZ - ind, " %d/%d 0x%x",
                                alarm.AlarmCond.ResourceId, alarm.AlarmCond.SensorNum,
                                alarm.AlarmCond.EventState);
                } else if (alarm.AlarmCond.Type == SAHPI_STATUS_COND_TYPE_OEM) {
                        snprintf(buf + ind, SHOW_BUF_SZ - ind, " OEM = %d",
                                alarm.AlarmCond.Mid);
                        break;
                };
                strcat(buf, "\n");
                if (proc(buf) != 0)
                        return(-1);
        };
        if ( (rv == SA_ERR_HPI_NOT_PRESENT) && (first == 1) ) {
                proc("No alarms in DAT.\n");
                return(SA_OK);
        };
        return(rv);
}

SaErrorT show_inventory(SaHpiSessionIdT sessionid, SaHpiResourceIdT resourceid,
                        SaHpiIdrIdT IdrId, SaHpiEntryIdT areaid, hpi_ui_print_cb_t proc)
{
        SaHpiIdrInfoT           info;
        SaErrorT                rv;
        SaHpiEntryIdT           entryid, nextentryid;
        SaHpiEntryIdT           fentryid, nextfentryid;
        SaHpiIdrAreaHeaderT     hdr;
        SaHpiIdrFieldT          field;
        char                    buf[SHOW_BUF_SZ];
        char                    *str;
        int                     num;

        rv = saHpiIdrInfoGet(sessionid, resourceid, IdrId, &info);
        if (rv != SA_OK) {
                snprintf(buf, SHOW_BUF_SZ, "ERROR!!! saHpiIdrInfoGet: %s\n",
                        oh_lookup_error(rv));
                proc(buf);
                return(-1);
        };
        num = info.NumAreas;
        snprintf(buf, SHOW_BUF_SZ,
                "Inventory: %d   Update count: %d   Read Only: %s   Areas: %d\n",
                info.IdrId, info.UpdateCount, HPIBOOL2STR( info.ReadOnly ), num);
        if (proc(buf) != 0) return(SA_OK);
        entryid = SAHPI_FIRST_ENTRY;
        while ((entryid != SAHPI_LAST_ENTRY) && (num > 0)) {
                rv = saHpiIdrAreaHeaderGet(sessionid, resourceid, IdrId,
                        SAHPI_IDR_AREATYPE_UNSPECIFIED, entryid,
                        &nextentryid, &hdr);
                if (rv != SA_OK) {
                        proc("ERROR!!! saHpiIdrAreaHeaderGet\n");
                        return(-1);
                };
                if ( areaid != SAHPI_LAST_ENTRY && areaid != hdr.AreaId ) {
                        entryid = nextentryid;
                        continue;
                }
                str = oh_lookup_idrareatype(hdr.Type);
                if (str == NULL) str = "Unknown";
                snprintf(buf, SHOW_BUF_SZ,
                        "    Area: %d   Type: %s   Read Only: %s   Fields: %d\n",
                        hdr.AreaId, str, HPIBOOL2STR( hdr.ReadOnly ), hdr.NumFields);
                if (proc(buf) != 0) return(SA_OK);
                fentryid = SAHPI_FIRST_ENTRY;
                entryid = nextentryid;
                while ((fentryid != SAHPI_LAST_ENTRY) && (hdr.NumFields > 0)) {
                        rv = saHpiIdrFieldGet(sessionid, resourceid, IdrId, hdr.AreaId,
                                SAHPI_IDR_FIELDTYPE_UNSPECIFIED, fentryid,
                                &nextfentryid, &field);
                        if (rv != SA_OK) {
                                proc("ERROR!!! saHpiIdrFieldGet\n");
                                return(-1);
                        };
                        str = oh_lookup_idrfieldtype(field.Type);
                        if (str == NULL) str = "Unknown";
                        snprintf(buf, SHOW_BUF_SZ,
                                "        Field: %d  Type: %s Read Only: %s (",
                                field.FieldId, str, HPIBOOL2STR( field.ReadOnly ));
                        if (proc(buf) != 0) return(SA_OK);
                        if (print_text_buffer(NULL, &(field.Field), NULL, proc) != 0)
                                return(SA_OK);
                        if (proc(")\n") != 0) return(SA_OK);
                        fentryid = nextfentryid;
                }
        };
        return(SA_OK);
}

void show_inv_area_header(SaHpiIdrAreaHeaderT *Header, int del, hpi_ui_print_cb_t proc)
{
        char    buf[SHOW_BUF_SZ], *str;
        int     len;

        del <<= 1;
        len = SHOW_BUF_SZ - del;
        str = buf + del;
        memset(buf, ' ', SHOW_BUF_SZ);
        snprintf(str, len, "AreaId: %d\n", Header->AreaId);
        if (proc(buf) != 0) return;
        snprintf(str, len, "AreaType: %s\n", oh_lookup_idrareatype(Header->Type));
        if (proc(buf) != 0) return;
        snprintf(str, len, "ReadOnly: %s\n", HPIBOOL2STR( Header->ReadOnly ));
        if (proc(buf) != 0) return;
        snprintf(str, len, "NumFields: %d\n", Header->NumFields);
        proc(buf);
}

void show_inv_field(SaHpiIdrFieldT *Field, int del, hpi_ui_print_cb_t proc)
{
        char    buf[SHOW_BUF_SZ], *str;
        int     len;

        del <<= 1;
        len = SHOW_BUF_SZ - del;
        str = buf + del;
        memset(buf, ' ', SHOW_BUF_SZ);
        snprintf(str, len, "Field Id: %d\n", Field->FieldId);
        if (proc(buf) != 0) return;
        snprintf(str, len, "Field Type: %s\n", oh_lookup_idrfieldtype(Field->Type));
        if (proc(buf) != 0) return;
        snprintf(str, len, "ReadOnly: %s\n", HPIBOOL2STR( Field->ReadOnly ));
        if (proc(buf) != 0) return;
        *str = 0;
        proc(buf);
        print_text_buffer("Content: ", &(Field->Field), "\n", proc);
}
