/*      -*- linux-c -*-
 *
 * (C) Copyright IBM Corp. 2004,2006
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  This
 * file and program are licensed under a BSD style license.  See
 * the Copying file included with the OpenHPI distribution for
 * full licensing terms.
 *
 * Author(s):
 *      Steve Sherman <stevees@us.ibm.com>
 *	Renier Morales <renier@openhpi.org>
 */

#ifndef __SAHPI_STRUCT_UTILS_H
#define __SAHPI_STRUCT_UTILS_H

#ifndef __OH_UTILS_H
#warning *** Include oh_utils.h instead of individual utility header files ***
#endif

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

/***********************
 * Text buffer utilities
 ***********************/
SaErrorT oh_init_textbuffer(SaHpiTextBufferT *buffer);
SaErrorT oh_append_textbuffer(SaHpiTextBufferT *buffer, const char *from);
SaErrorT oh_copy_textbuffer(SaHpiTextBufferT *dest, const SaHpiTextBufferT *from);

/* Print just the Data portions of the text structures */
#define oh_print_text(buf_ptr)  oh_fprint_text(stdout, buf_ptr)
SaErrorT oh_fprint_text(FILE *stream, const SaHpiTextBufferT *buffer);

/* Same as SaHpiTextBufferT, only more Data */
#define OH_MAX_TEXT_BUFFER_LENGTH 2048
typedef struct {
        SaHpiTextTypeT  DataType;
        SaHpiLanguageT  Language;
        SaHpiUint16T    DataLength;
        SaHpiUint8T     Data[OH_MAX_TEXT_BUFFER_LENGTH];
} oh_big_textbuffer;

SaErrorT oh_init_bigtext(oh_big_textbuffer *big_buffer);
SaErrorT oh_append_bigtext(oh_big_textbuffer *big_buffer, const char *from);
SaErrorT oh_copy_bigtext(oh_big_textbuffer *dest, const oh_big_textbuffer *from);

/* Print just the Data portions of the text structures */
#define oh_print_bigtext(bigbuf_ptr) oh_fprint_bigtext(stdout, bigbuf_ptr)
SaErrorT oh_fprint_bigtext(FILE *stream, const oh_big_textbuffer *big_buffer);

/************************************
 * HPI structure to string conversion
 ************************************/
SaErrorT oh_decode_manufacturerid(SaHpiManufacturerIdT value,
                                  SaHpiTextBufferT *buffer);

SaErrorT oh_decode_sensorreading(SaHpiSensorReadingT reading,
                                 SaHpiSensorDataFormatT format,
                                 SaHpiTextBufferT *buffer);

SaErrorT oh_encode_sensorreading(SaHpiTextBufferT *buffer,
                                 SaHpiSensorReadingTypeT type,
                                 SaHpiSensorReadingT *reading);

SaErrorT oh_decode_capabilities(SaHpiCapabilitiesT ResourceCapabilities,
                                SaHpiTextBufferT *buffer);

SaErrorT oh_decode_hscapabilities(SaHpiHsCapabilitiesT HsCapabilities,
                                  SaHpiTextBufferT *buffer);

SaErrorT oh_decode_sensoroptionaldata(SaHpiSensorOptionalDataT sensor_opt_data,
                                      SaHpiTextBufferT *buffer);

SaErrorT oh_decode_sensorenableoptdata(SaHpiSensorEnableOptDataT sensor_enable_opt_data,
                                       SaHpiTextBufferT *buffer);
SaErrorT oh_decode_dimitestcapabilities(SaHpiDimiTestCapabilityT capabilities,
                                        SaHpiTextBufferT *buffer);

SaErrorT oh_decode_fumiprotocols(SaHpiFumiProtocolT protocols,
                                 SaHpiTextBufferT *buffer);

SaErrorT oh_decode_fumicapabilities(SaHpiFumiCapabilityT capabilities,
                                  SaHpiTextBufferT *buffer);

/*************************
 * Validate HPI structures
 *************************/
SaHpiBoolT oh_valid_textbuffer(SaHpiTextBufferT *buffer);

SaErrorT oh_valid_thresholds(SaHpiSensorThresholdsT *thds,
                             SaHpiRdrT *rdr);

SaErrorT oh_valid_ctrl_state_mode(SaHpiCtrlRecT *ctrl_rdr,
                                  SaHpiCtrlModeT mode,
                                  SaHpiCtrlStateT *state);

/************************
 * Compare HPI structures
 ************************/
int oh_compare_sensorreading(SaHpiSensorReadingTypeT type,
                             SaHpiSensorReadingT *reading1,
                             SaHpiSensorReadingT *reading2);

/***************************
 * Print HPI data structures
 ***************************/
#define OH_PRINT_OFFSET "  "  /* Offset string */

SaErrorT oh_append_offset(oh_big_textbuffer *buffer, int offsets);

#define oh_print_event(event_ptr, ep, offsets) oh_fprint_event(stdout, event_ptr, ep, offsets)
SaErrorT oh_fprint_event(FILE *stream,
                         const SaHpiEventT *event,
                         const SaHpiEntityPathT *entitypath,
                         int offsets);

#define oh_print_idrfield(thisfield, offsets) oh_fprint_idrfield(stdout, thisfield, offsets)
SaErrorT oh_fprint_idrfield(FILE *stream, const SaHpiIdrFieldT *thisfield, int offsets);

#define oh_print_idrinfo(idrInfo, offsets) oh_fprint_idrinfo(stdout, idrInfo, offsets)
SaErrorT oh_fprint_idrinfo(FILE *stream, const SaHpiIdrInfoT *idrInfo, int offsets);

#define oh_print_idrareaheader(areaHeader, offsets) oh_fprint_idrareaheader(stdout, areaHeader, offsets)
SaErrorT oh_fprint_idrareaheader(FILE *stream, const SaHpiIdrAreaHeaderT *areaHeader, int offsets);

#define oh_print_rptentry(rptEntry, offsets) oh_fprint_rptentry(stdout, rptEntry, offsets)
SaErrorT oh_fprint_rptentry(FILE *stream, const SaHpiRptEntryT *rptEntry, int offsets);

#define oh_print_sensorrec(sensor_ptr, offsets) oh_fprint_sensorrec(stdout, sensor_ptr, offsets)
SaErrorT oh_fprint_sensorrec(FILE *stream, const SaHpiSensorRecT *sensor, int offsets);

#define oh_print_rdr(rdr, offsets) oh_fprint_rdr(stdout, rdr, offsets)
SaErrorT oh_fprint_rdr(FILE *stream, const SaHpiRdrT *rdrEntry, int offsets);

#define oh_print_textbuffer(buf_ptr, offsets)  oh_fprint_textbuffer(stdout, buf_ptr, offsets)
SaErrorT oh_fprint_textbuffer(FILE *stream, const SaHpiTextBufferT *textbuffer, int offsets);

#define oh_print_ctrlrec(ctrl_ptr, offsets) oh_fprint_ctrlrec(stdout, ctrl_ptr, offsets)
SaErrorT oh_fprint_ctrlrec(FILE *stream, const SaHpiCtrlRecT *control, int offsets);

#define oh_print_watchdogrec(watchdog_ptr, offsets) oh_fprint_watchdogrec(stdout, watchdog_ptr, offsets)
SaErrorT oh_fprint_watchdogrec(FILE *stream, const SaHpiWatchdogRecT *watchdog, int offsets);

#define oh_print_eventloginfo(elinfo_ptr, offsets) oh_fprint_eventloginfo(stdout, elinfo_ptr, offsets)
SaErrorT oh_fprint_eventloginfo(FILE *stream, const SaHpiEventLogInfoT *thiselinfo, int offsets);

#define oh_print_eventlogentry(eventlog_ptr, ep, offsets) oh_fprint_eventlogentry(stdout, eventlog_ptr, ep, offsets)
SaErrorT oh_fprint_eventlogentry(FILE *stream,
                                 const SaHpiEventLogEntryT *thiseventlog,
                                 const SaHpiEntityPathT *entitypath,
                                 int offsets);

#define oh_print_ctrlstate(ctrlstate_ptr, offsets) oh_fprint_ctrlstate(stdout, ctrlstate_ptr, offsets)
SaErrorT oh_fprint_ctrlstate(FILE *stream, const SaHpiCtrlStateT *thisctrlstate, int offsets);

#define oh_print_thresholds(thresholds, format, offsets) oh_fprint_thresholds(stdout, thresholds, format, offsets)
SaErrorT oh_fprint_thresholds(FILE *stream,
			      const SaHpiSensorThresholdsT *thresholds,
			      const SaHpiSensorDataFormatT *format,
			      int offsets);

SaErrorT oh_build_event(oh_big_textbuffer *buffer, const SaHpiEventT *event, const SaHpiEntityPathT *entitypath, int offsets);
SaErrorT oh_build_threshold_mask(oh_big_textbuffer *buffer, const SaHpiSensorThdMaskT tmask, int offsets);


#ifdef __cplusplus
}
#endif

#endif
