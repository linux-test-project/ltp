/*      -*- linux-c -*-
 *
 * (C) Copyright IBM Corp. 2004-2006
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  This
 * file and program are licensed under a BSD style license.  See
 * the Copying file included with the OpenHPI distribution for
 * full licensing terms.
 *
 * Author(s):
 *      Sean Dague <sdague@users.sf.net>
 *      Steve Sherman <stevees@us.ibm.com>
 *      Racing Guo <racing.guo@intel.com>
 *      Renier Morales <renier@openhpi.org>
 */

#include <glib.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <uuid/uuid.h>

#include <SaHpi.h>
#include <oh_utils.h>
#include <oh_error.h>

#define U_IS_SURROGATE(c) (((c)&0xfffff800)==0xd800)
#define U_IS_LEAD(c) (((c)&0xfffffc00)==0xd800)
#define U_IS_TRAIL(c) (((c)&0xfffffc00)==0xdc00)

static inline SaErrorT oh_append_data(oh_big_textbuffer *big_buffer, const SaHpiUint8T *from, SaHpiUint8T len);

static SaErrorT oh_build_resourceinfo(oh_big_textbuffer *buffer, const SaHpiResourceInfoT *ResourceInfo, int offsets);
static SaErrorT oh_build_sensorrec(oh_big_textbuffer *buffer, const SaHpiSensorRecT *sensor, int offsets);
static SaErrorT oh_build_sensordataformat(oh_big_textbuffer *buffer, const SaHpiSensorDataFormatT *format, int offsets);
static SaErrorT oh_build_sensorthddefn(oh_big_textbuffer *buffer, const SaHpiSensorThdDefnT *tdef, int offsets);
static SaErrorT oh_build_textbuffer(oh_big_textbuffer *buffer, const SaHpiTextBufferT *textbuffer, int offsets);

static SaErrorT oh_build_ctrlrec(oh_big_textbuffer *textbuf, const SaHpiCtrlRecT *ctrlrec, int offsets);
static SaErrorT oh_build_invrec(oh_big_textbuffer *textbuff, const SaHpiInventoryRecT *invrec, int offsets);
static SaErrorT oh_build_wdogrec(oh_big_textbuffer *textbuff, const SaHpiWatchdogRecT *wdogrec, int offsets);
static SaErrorT oh_build_annrec(oh_big_textbuffer *textbuff, const SaHpiAnnunciatorRecT *annrec, int offsets);
static SaErrorT oh_build_dimirec(oh_big_textbuffer *textbuff, const SaHpiDimiRecT *dimirec, int offsets);
static SaErrorT oh_build_fumirec(oh_big_textbuffer *textbuff, const SaHpiFumiRecT *fumirec, int offsets);

static SaErrorT oh_build_event_resource(oh_big_textbuffer *buffer, const SaHpiEventT *event, int offsets);
static SaErrorT oh_build_event_domain(oh_big_textbuffer *buffer, const SaHpiEventT *event, int offsets);
static SaErrorT oh_build_event_sensor(oh_big_textbuffer *buffer, const SaHpiEventT *event, int offsets);
static SaErrorT oh_build_event_sensor_enable_change(oh_big_textbuffer *buffer, const SaHpiEventT *event, int offsets);
static SaErrorT oh_build_event_hotswap(oh_big_textbuffer *buffer, const SaHpiEventT *event, int offsets);
static SaErrorT oh_build_event_watchdog(oh_big_textbuffer *buffer, const SaHpiEventT *event, int offsets);
static SaErrorT oh_build_event_hpi_sw(oh_big_textbuffer *buffer, const SaHpiEventT *event, int offsets);
static SaErrorT oh_build_event_oem(oh_big_textbuffer *buffer, const SaHpiEventT *event, int offsets);
static SaErrorT oh_build_event_user(oh_big_textbuffer *buffer, const SaHpiEventT *event, int offsets);
static SaErrorT oh_build_event_dimi(oh_big_textbuffer *buffer, const SaHpiEventT *event, int offsets);
static SaErrorT oh_build_event_dimi_update(oh_big_textbuffer *buffer, const SaHpiEventT *event, int offsets);
static SaErrorT oh_build_event_fumi(oh_big_textbuffer *buffer, const SaHpiEventT *event, int offsets);

/************************************************************************
 * NOTES!
 *
 * - Several error checks can be removed if valid_xxx routines are defined
 *   for input structures. If this happens, several of the default switch
 *   statements should also return SA_ERR_HPI_INTERNAL_ERROR instead
 *   of SA_ERR_HPI_INVALID_PARMS.
 ************************************************************************/

/**
 * oh_lookup_manufacturerid:
 * @value: enum value of type SaHpiManufacturerIdT.
 * @buffer:  Location to store the string.
 *
 * Converts @value into a string based on @value's enum definition
 * in http://www.iana.org/assignments/enterprise-numbers.
 * String is stored in an SaHpiTextBufferT data structure.
 *
 * Only a few of the manufacturers in that list have been defined.
 * For all others, this routine returns "Unknown Manufacturer".
 * Feel free to add your own favorite manufacturer to this routine.
 *
 * Returns:
 * SA_OK - Normal operation.
 * SA_ERR_HPI_INVALID_PARAMS - @buffer is  NULL.
 **/
SaErrorT oh_decode_manufacturerid(SaHpiManufacturerIdT value, SaHpiTextBufferT *buffer)
{
        SaErrorT err;
        SaHpiTextBufferT working;

        if (!buffer) {
                err("Invalid parameters.");
                return(SA_ERR_HPI_INVALID_PARAMS);
        }

        err = oh_init_textbuffer(&working);
        if (err) { return(err); }

        switch(value) {
        case SAHPI_MANUFACTURER_ID_UNSPECIFIED:
                err = oh_append_textbuffer(&working, "Unspecified");
                if (err) { return(err); }
                break;
        case 2:		/* 2 is IANA number */
        case 20944:     /* 20944 is IANA code for Modular Blade Server */
                err = oh_append_textbuffer(&working,"IBM");
                if (err) { return(err); }
                break;
        case ATCAHPI_PICMG_MID: /* ATCAHPI_PICMG_MID is IANA code for PICMG manufacturer identifier */
                err = oh_append_textbuffer(&working,"PICMG");
                if (err) { return(err); }
                break;
        case 11:
                err = oh_append_textbuffer(&working, "Hewlett-Packard");
                if (err) { return(err); }
                break;
        case 42:
                err = oh_append_textbuffer(&working, "Sun Microsystems");
                if (err) { return(err); }
                break;
        case 161:
                err = oh_append_textbuffer(&working, "Motorola");
                if (err) { return(err); }
                break;
        case 185:
                err = oh_append_textbuffer(&working, "Interphase");
                if (err) { return(err); }
                break;
        case 343:
                err = oh_append_textbuffer(&working, "Intel Corporation");
                if (err) { return(err); }
                break;
        case 398:
                err = oh_append_textbuffer(&working, "Tyco Electronics");
                if (err) { return(err); }
                break;
        case 688:
                err = oh_append_textbuffer(&working, "Znyx Advanced Systems Division, Inc.");
                if (err) { return(err); }
                break;
        case 912:
                err = oh_append_textbuffer(&working, "Adax Inc.");
                if (err) { return(err); }
                break;
        case 1458:
                err = oh_append_textbuffer(&working, "AMCC");
                if (err) { return(err); }
                break;
        case 1556:
                err = oh_append_textbuffer(&working, "Performance Technologies, Inc.");
                if (err) { return(err); }
                break;
        case 2012:
                err = oh_append_textbuffer(&working, "Schroff GmbH");
                if (err) { return(err); }
                break;
        case 2537:
                err = oh_append_textbuffer(&working, "Diversified Technology, Inc.");
                if (err) { return(err); }
                break;
        case 2606:
                err = oh_append_textbuffer(&working, "Rittal-Werk Rudolf Loh GmbH & Co.KG");
                if (err) { return(err); }
                break;
        case 2628:
                err = oh_append_textbuffer(&working, "Natural MicroSystems");
                if (err) { return(err); }
                break;
        case 3028:
                err = oh_append_textbuffer(&working, "Dialogic Corporation");
                if (err) { return(err); }
                break;
        case 3442:
                err = oh_append_textbuffer(&working, "Advantech Inc.");
                if (err) { return(err); }
                break;
        case 4127:
                err = oh_append_textbuffer(&working, "Mercury Computer Systems");
                if (err) { return(err); }
                break;
        case 4337:
                err = oh_append_textbuffer(&working, "RadiSys Corporation");
                if (err) { return(err); }
                break;
        case 5380:
                err = oh_append_textbuffer(&working, "CDOT");
                if (err) { return(err); }
                break;
        case 6629:
                err = oh_append_textbuffer(&working, "Ulticom");
                if (err) { return(err); }
                break;
        case 7994:
                err = oh_append_textbuffer(&working, "Continuous Computing Corp.");
                if (err) { return(err); }
                break;
        case 8337:
                err = oh_append_textbuffer(&working, "Hybricon Corp");
                if (err) { return(err); }
                break;
        case 10297:
                err = oh_append_textbuffer(&working, "Advantech Co., Ltd.");
                if (err) { return(err); }
                break;
        case 10520:
                err = oh_append_textbuffer(&working, "Tyco Electronics Power Systems");
                if (err) { return(err); }
                break;
        case 10728:
                err = oh_append_textbuffer(&working, "Redline Communications Inc.");
                if (err) { return(err); }
                break;
        case 13400:
                err = oh_append_textbuffer(&working, "Emerson Network Power");
                if (err) { return(err); }
                break;
        case 13427:
                err = oh_append_textbuffer(&working, "Artesyn Technologies");
                if (err) { return(err); }
                break;
        case 15000:
                err = oh_append_textbuffer(&working, "Kontron Canada Inc");
                if (err) { return(err); }
                break;
        case 15563:
                err = oh_append_textbuffer(&working, "Adtron");
                if (err) { return(err); }
                break;
        case 16394:
                err = oh_append_textbuffer(&working, "Pigeon Point Systems");
                if (err) { return(err); }
                break;
        case 16446:
                err = oh_append_textbuffer(&working, "Adlink");
                if (err) { return(err); }
                break;
        case 18765:
                err = oh_append_textbuffer(&working, "Comtel Electronics GmbH");
                if (err) { return(err); }
                break;
        case 19911:
                err = oh_append_textbuffer(&working, "Global Velocity Inc.");
                if (err) { return(err); }
                break;
        case 20974:
                err = oh_append_textbuffer(&working, "American Megatrends, Inc");
                if (err) { return(err); }
                break;
        case 22341:
                err = oh_append_textbuffer(&working, "ESO Technologies");
                if (err) { return(err); }
                break;
        case 23858:
                err = oh_append_textbuffer(&working, "VadaTech Inc.");
                if (err) { return(err); }
                break;
        case 24632:
                err = oh_append_textbuffer(&working, "CorEdge Networks");
                if (err) { return(err); }
                break;
        case 25635:
                err = oh_append_textbuffer(&working, "Carlo Gavazzi Computing Solutions");
                if (err) { return(err); }
                break;
        case 26609:
                err = oh_append_textbuffer(&working, "Pentair Electronic Packaging");
                if (err) { return(err); }
                break;
        case 24893:
                err = oh_append_textbuffer(&working, "GE Fanuc Embedded Systems");
                if (err) { return(err); }
                break;
        case 26061:
                err = oh_append_textbuffer(&working, "Artesyn Communication Products Ltd");
                if (err) { return(err); }
                break;
        case 26655:
                err = oh_append_textbuffer(&working, "Advantech Co., Ltd");
                if (err) { return(err); }
                break;
        case 27317:
                err = oh_append_textbuffer(&working, "Extreme Engineering Solutions, Inc");
                if (err) { return(err); }
                break;
        case 27768:
                err = oh_append_textbuffer(&working, "Gesellschaft für Netzwerk- und Automatisierungs-Technologie GmbH");
                if (err) { return(err); }
                break;
        case 29333:
                err = oh_append_textbuffer(&working, "JBlade LLC");
                if (err) { return(err); }
                break;
        case 186:
                err = oh_append_textbuffer(&working, "Toshiba");
                if (err) { return(err); }
                break;
        case 116:
                err = oh_append_textbuffer(&working, "Hitachi");
                if (err) { return(err); }
                break;
        case 399:
                err = oh_append_textbuffer(&working, "Hitachi");
                if (err) { return(err); }
                break;
        case 119:
                err = oh_append_textbuffer(&working, "NEC");
                if (err) { return(err); }
                break;
        case 373:
                err = oh_append_textbuffer(&working, "Tatung");
                if (err) { return(err); }
                break;
        case 802:
                err = oh_append_textbuffer(&working, "National Semiconductor");
                if (err) { return(err); }
                break;
        case 674:
                err = oh_append_textbuffer(&working, "Dell");
                if (err) { return(err); }
                break;
        case 2168:
                err = oh_append_textbuffer(&working, "LMC");
                if (err) { return(err); }
                break;
        case 6653:
                err = oh_append_textbuffer(&working, "Tyan");
                if (err) { return(err); }
                break;
        case 10368:
                err = oh_append_textbuffer(&working, "Fujitsu-Siemens");
                if (err) { return(err); }
                break;
        case 10876:
                err = oh_append_textbuffer(&working, "SuperMicro");
                if (err) { return(err); }
                break;
        case 13742:
                err = oh_append_textbuffer(&working, "Raritan");
                if (err) { return(err); }
                break;
        case 10437:
                err = oh_append_textbuffer(&working, "Peppercon");
                if (err) { return(err); }
                break;
        case 10418:
                err = oh_append_textbuffer(&working, "Avocent");
                if (err) { return(err); }
                break;
        case 11102:
                err = oh_append_textbuffer(&working, "OSA");
                if (err) { return(err); }
                break;
        default:
                err = oh_append_textbuffer(&working,  "Unknown");
                if (err) { return(err); }
        }

        oh_copy_textbuffer(buffer, &working);

        return(SA_OK);
}

/**
 * oh_decode_sensorreading:
 * @reading: SaHpiSensorReadingT to convert.
 * @format: SaHpiDataFormatT for the sensor reading.
 * @buffer: Location to store the converted string.
 *
 * Converts an HPI sensor reading and format into a string.
 * String is stored in an SaHpiTextBufferT data structure.
 *
 * Returns:
 * SA_OK - Normal operation.
 * SA_ERR_HPI_INVALID_CMD - @format or @reading have IsSupported == FALSE.
 * SA_ERR_HPI_INVALID_DATA - @format and @reading types don't match.
 * SA_ERR_HPI_INVALID_PARAMS - Pointer parameter(s) are NULL; @reading type != @format type.
 * SA_ERR_HPI_OUT_OF_SPACE - @buffer not big enough to accomodate appended string
 **/
SaErrorT oh_decode_sensorreading(SaHpiSensorReadingT reading,
                                 SaHpiSensorDataFormatT format,
                                 SaHpiTextBufferT *buffer)
{
        char text[SAHPI_MAX_TEXT_BUFFER_LENGTH];
        SaErrorT err;
        SaHpiTextBufferT working;
        char str[SAHPI_SENSOR_BUFFER_LENGTH + 1];

        if (!buffer) {
                err("Invalid parameter.");
                return(SA_ERR_HPI_INVALID_PARAMS);
        }
        if (!reading.IsSupported || !format.IsSupported) {
                err("Invalid Command.");
                return(SA_ERR_HPI_INVALID_CMD);
        }
        if (reading.Type != format.ReadingType) {
                err("Invalid Data.");
                return(SA_ERR_HPI_INVALID_DATA);
        }

        oh_init_textbuffer(&working);
        memset(text, 0, SAHPI_MAX_TEXT_BUFFER_LENGTH);

        switch(reading.Type) {
        case SAHPI_SENSOR_READING_TYPE_INT64:
                snprintf(text, SAHPI_MAX_TEXT_BUFFER_LENGTH,
                         "%lld", reading.Value.SensorInt64);
                err = oh_append_textbuffer(&working, text);
                if (err) { return(err); }
                break;
        case SAHPI_SENSOR_READING_TYPE_UINT64:
                snprintf(text, SAHPI_MAX_TEXT_BUFFER_LENGTH,
                         "%llu", reading.Value.SensorUint64);
                err = oh_append_textbuffer(&working, text);
                if (err) { return(err); }
                break;
        case SAHPI_SENSOR_READING_TYPE_FLOAT64:
                snprintf(text, SAHPI_MAX_TEXT_BUFFER_LENGTH,
                         "%5.3lf", reading.Value.SensorFloat64);
                err = oh_append_textbuffer(&working, text);
                if (err) { return(err); }
                break;
        case SAHPI_SENSOR_READING_TYPE_BUFFER:
                /* In case Sensor Buffer contains no end of string deliminter */
                memset(str, 0, SAHPI_SENSOR_BUFFER_LENGTH + 1);
                strncpy(str, (char *)reading.Value.SensorBuffer, SAHPI_SENSOR_BUFFER_LENGTH);
                err = oh_append_textbuffer(&working, str);
                if (err) { return(err); }
                break;
        default:
                return(SA_ERR_HPI_INVALID_PARAMS);
        }

        if (format.Percentage) {
                err = oh_append_textbuffer(&working, "%");
                if (err) { return(err); }
        }
        else {
                /* Add units */
                if (format.BaseUnits != SAHPI_SU_UNSPECIFIED) {
                        char *str;

                        err = oh_append_textbuffer(&working, " ");
                        if (err) { return(err); }
                        str = oh_lookup_sensorunits(format.BaseUnits);
                        if (str == NULL) { return(SA_ERR_HPI_INVALID_PARAMS); }
                        err = oh_append_textbuffer(&working, str);
                        if (err) { return(err); }
                }

                /* Add modifier units, if appropriate */
                if (format.BaseUnits != SAHPI_SU_UNSPECIFIED &&
                    format.ModifierUse != SAHPI_SMUU_NONE) {
                        char *str;

                        switch(format.ModifierUse) {
                        case SAHPI_SMUU_BASIC_OVER_MODIFIER:
                                err = oh_append_textbuffer(&working, " / ");
                                if (err) { return(err); }
                                break;
                        case SAHPI_SMUU_BASIC_TIMES_MODIFIER:
                                err = oh_append_textbuffer(&working, " * ");
                                if (err) { return(err); }
                                break;
                        default:
                                return(SA_ERR_HPI_INVALID_PARAMS);
                        }
                        str = oh_lookup_sensorunits(format.ModifierUnits);
                        if (str == NULL) { return(SA_ERR_HPI_INVALID_PARAMS); }
                        err = oh_append_textbuffer(&working, str);
                        if (err) { return(err); }
                }
        }

        oh_copy_textbuffer(buffer, &working);

        return(SA_OK);
}

/**
 * oh_encode_sensorreading:
 * @buffer: Location of SaHpiTextBufferT containing the string to
 *          convert into a SaHpiSensorReadingT.
 * @type: SaHpiSensorReadingTypeT of converted reading.
 * @reading: SaHpiSensorReadingT location to store converted string.
 *
 * Converts @buffer->Data string to an HPI SaHpiSensorReadingT structure.
 * Generally @buffer->Data is created by oh_decode_sensorreading() or has
 * been built by a plugin, which gets string values for sensor readings (e.g.
 * through SNMP OID commands). Any non-numeric portion of the string is
 * discarded. For example, the string "-1.43 Volts" is converted to -1.43
 * of type @type.
 *
 * If type = SAHPI_SENSOR_READING_TYPE_BUFFER, and @buffer->Data >
 * SAHPI_SENSOR_BUFFER_LENGTH, data is truncated to fit into the reading
 * buffer.
 *
 * Notes!
 * - Numerical strings can contain commas but it is assummed that strings follow.
 *   US format (e.g. "1,000,000" = 1 million; not "1.000.000").
 * - Decimal points are always preceded by at least one number (e.g. "0.9").
 * - Numerical percentage strings like "89%" are stripped of their percent sign.
 * - Hex notation is not supported (e.g. "0x23").
 * - Scientific notation is not supported (e.g. "e+02").
 *
 * Returns:
 * SA_OK - Normal operation.
 * SA_ERR_HPI_INVALID_PARAMS - Pointer parameter(s) are NULL; Invalid @type.
 * SA_ERR_HPI_INVALID_DATA - Converted @buffer->Data too large for @type; cannot
 *                           convert string into valid number; @type incorrect
 *                           for resulting number.
 **/
SaErrorT oh_encode_sensorreading(SaHpiTextBufferT *buffer,
                                 SaHpiSensorReadingTypeT type,
                                 SaHpiSensorReadingT *reading)
{
        char *endptr;
        char  numstr[SAHPI_MAX_TEXT_BUFFER_LENGTH];
        int   i, j, skip;
        int   found_sign, found_number, found_float, in_number;
        int   is_percent = 0;
        SaHpiFloat64T num_float64 = 0.0;
        SaHpiInt64T   num_int64 = 0;
        SaHpiUint64T  num_uint64 = 0;
        SaHpiSensorReadingT working;

        if (!buffer || !reading ||
            buffer->Data == NULL || buffer->Data[0] == '\0' ||
            !oh_lookup_sensorreadingtype(type)) {
                err("Invalid parameter");
                return(SA_ERR_HPI_INVALID_PARAMS);
        }

        if (type == SAHPI_SENSOR_READING_TYPE_BUFFER) {
                reading->IsSupported = SAHPI_TRUE;
                reading->Type = type;
                strncpy((char *)reading->Value.SensorBuffer, (char *)buffer->Data, SAHPI_SENSOR_BUFFER_LENGTH);
                return(SA_OK);
        }

        memset(numstr, 0, SAHPI_MAX_TEXT_BUFFER_LENGTH);
        memset(&working, 0, sizeof(SaHpiSensorReadingT));
        working.IsSupported = SAHPI_TRUE;
        working.Type = type;

        /* Search string and pull out first numeric string. The strtol type
         * functions below are pretty good at handling non-numeric junk in
         * string, but they don't handle spaces after a sign or commas within
         * a number. So we normalize the string a bit first.
         */

        /* Skip any characters before an '=' sign */
        char *skipstr = strchr((char *)buffer->Data, '=');
        if (skipstr) skip = (long int)skipstr - (long int)(buffer->Data) + 1;
        else skip = 0;

        j = found_sign = in_number = found_number = found_float = 0;
        for (i=skip; i<buffer->DataLength && !found_number; i++) {
                if (buffer->Data[i] == '+' || buffer->Data[i] == '-') {
                        if (found_sign) {
                                err("Cannot parse multiple sign values");
                                return(SA_ERR_HPI_INVALID_DATA);
                        }
                        found_sign = 1;
                        numstr[j] = buffer->Data[i];
                        j++;
                }
                if (isdigit(buffer->Data[i])) {
                        if (!found_number) {
                                in_number = 1;
                                numstr[j] = buffer->Data[i];
                                j++;
                        }
                }
                else { /* Strip non-numerics */
                        if (buffer->Data[i] == '.') { /* Unless its a decimal point */
                                if (in_number) {
                                        if (found_float) {
                                                err("Cannot parse multiple decimal points");
                                                return(SA_ERR_HPI_INVALID_DATA);
                                        }
                                        found_float = 1;
                                        numstr[j] = buffer->Data[i];
                                        j++;
                                }
                        }
                        else {
                                /* Delete commas but don't end search for more numbers */
                                if (in_number && buffer->Data[i] != ',') {
                                        found_number = 1;
                                }
                        }
                }
        }

        if (found_number || in_number) { /* in_number means string ended in a digit character */
                for (j=i-1; j<buffer->DataLength; j++) {
                        if (buffer->Data[j] == '%') {
                                is_percent = 1;
                                break;
                        }
                }
                found_number = 1;
        }

        if (found_float && type != SAHPI_SENSOR_READING_TYPE_FLOAT64) {
                err("Number and type incompatible");
                return(SA_ERR_HPI_INVALID_DATA);
        }

        /* Convert string to number */
        switch (type) {
        case SAHPI_SENSOR_READING_TYPE_INT64:
                if (found_number) {
                        errno = 0;
                        num_int64 = strtoll(numstr, &endptr, 10);
                        if (errno) {
                                err("strtoll failed, errno=%d", errno);
                                return(SA_ERR_HPI_INVALID_DATA);
                        }
                        if (*endptr != '\0') {
                                err("strtoll failed: End Pointer=%s", endptr);
                                return(SA_ERR_HPI_INVALID_DATA);
                        }
                }
                else { /* No number in string */
                        num_int64 = 0;
                }

                working.Value.SensorInt64 = num_int64;
                break;

        case SAHPI_SENSOR_READING_TYPE_UINT64:
                if (found_number) {
                        errno = 0;
                        num_uint64 = strtoull(numstr, &endptr, 10);
                        if (errno) {
                                err("strtoull failed, errno=%d", errno);
                                return(SA_ERR_HPI_INVALID_DATA);
                        }
                        if (*endptr != '\0') {
                                err("strtoull failed: End Pointer=%s", endptr);
                                return(SA_ERR_HPI_INVALID_DATA);
                        }
                }
                else { /* No number in string */
                        num_uint64 = 0;
                }

                working.Value.SensorUint64 = num_uint64;
                break;

        case SAHPI_SENSOR_READING_TYPE_FLOAT64:
                if (found_number) {
                        errno = 0;
                        num_float64 = strtold(numstr, &endptr);
                        if (errno) {
                                err("strtold failed, errno=%d", errno);
                                return(SA_ERR_HPI_INVALID_DATA);
                        }
                        if (*endptr != '\0') {
                                err("strtold failed: End Pointer=%s", endptr);
                                return(SA_ERR_HPI_INVALID_DATA);
                        }

                        working.Value.SensorFloat64 = num_float64;
                }
                else { /* No number in string */
                        num_float64 = 0;
                }
                break;

        default: /* Should never get here */
                err("Invalid type=%d", type);
                return(SA_ERR_HPI_INTERNAL_ERROR);
        }

        *reading = working;

        return(SA_OK);
}

/**
 * oh_fprint_text:
 * @stream: File handle.
 * @buffer: Pointer to SaHpiTextBufferT to be printed.
 *
 * Prints the text data contained in SaHpiTextBufferT to a file. Data must
 * be of type SAHPI_TL_TYPE_TEXT. @buffer->DataLength is ignored.
 * The MACRO oh_print_text(), uses this function to print to STDOUT.
 *
 * Returns:
 * SA_OK - Normal operation.
 * SA_ERR_HPI_INVALID_DATA - @buffer->DataType not SAHPI_TL_TYPE_TEXT.
 **/
SaErrorT oh_fprint_text(FILE *stream, const SaHpiTextBufferT *buffer)
{
        SaErrorT err;

        if (buffer->DataType == SAHPI_TL_TYPE_TEXT) {
                err = fwrite( buffer->Data, buffer->DataLength, 1, stream);
                if (err < 0) {
                        err("Invalid parameter.");
                        return(SA_ERR_HPI_INVALID_PARAMS);
                }
        }
        else {
                err("Invalid Data.");
                return(SA_ERR_HPI_INVALID_DATA);
        }

        return(SA_OK);
}

/**
 * oh_fprint_bigtext:
 * @stream: File handle.
 * @big_buffer: Pointer to oh_big_textbuffer to be printed.
 *
 * Prints the text data contained in oh_big_textbuffer to a file. Data must
 * be of type SAHPI_TL_TYPE_TEXT. @big_buffer->DataLength is ignored.
 * The MACRO oh_print_bigtext(), uses this function to print to STDOUT.
 *
 * Returns:
 * SA_OK - Normal operation.
 * SA_ERR_HPI_INVALID_DATA - @big_buffer->DataType not SAHPI_TL_TYPE_TEXT.
 **/
SaErrorT oh_fprint_bigtext(FILE *stream, const oh_big_textbuffer *big_buffer)
{
        SaErrorT err;

        if (big_buffer->DataType == SAHPI_TL_TYPE_TEXT) {
                err = fprintf(stream, "%s\n", big_buffer->Data);
                if (err < 0) {
                        err("Invalid parameter.");
                        return(SA_ERR_HPI_INVALID_PARAMS);
                }
        }
        else {
                err("Invalid Data.");
                return(SA_ERR_HPI_INVALID_DATA);
        }

        return(SA_OK);
}

/**
 * oh_init_textbuffer:
 * @buffer: Pointer to an SaHpiTextBufferT.
 *
 * Initializes an SaHpiTextBufferT. Assumes an English language set.
 *
 * Returns:
 * SA_OK - Normal operation.
 * SA_ERR_HPI_INVALID_PARAMS - @buffer is NULL.
 **/
SaErrorT oh_init_textbuffer(SaHpiTextBufferT *buffer)
{
        if (!buffer) {
                err("Invalid parameter.");
                return(SA_ERR_HPI_INVALID_PARAMS);
        }

        memset(buffer, 0, sizeof(*buffer));
        buffer->DataType = SAHPI_TL_TYPE_TEXT;
        buffer->Language = SAHPI_LANG_ENGLISH;
        buffer->DataLength = 0;
        return(SA_OK);
}

SaErrorT oh_init_bigtext(oh_big_textbuffer *big_buffer)
{
        if (!big_buffer) {
                err("Invalid parameter.");
                return(SA_ERR_HPI_INVALID_PARAMS);
        }

        memset(big_buffer, 0, sizeof(*big_buffer));
        big_buffer->DataType = SAHPI_TL_TYPE_TEXT;
        big_buffer->Language = SAHPI_LANG_ENGLISH;
        big_buffer->DataLength = 0;
        return(SA_OK);
}

/**
 * oh_copy_textbuffer:
 * @dest: SaHpiTextBufferT to copy into.
 * @from:SaHpiTextBufferT to copy from.
 *
 * Copies one SaHpiTextBufferT structure to another.
 *
 * Returns:
 * SA_OK - Normal operation.
 * SA_ERR_HPI_INVALID_PARAMS - Pointer parameter(s) are NULL.
 **/
SaErrorT oh_copy_textbuffer(SaHpiTextBufferT *dest, const SaHpiTextBufferT *from)
{
        if (!dest || !from) {
                err("Invalid parameter.");
                return(SA_ERR_HPI_INVALID_PARAMS);
        }

        dest->DataType = from->DataType;
        dest->Language = from->Language;
        dest->DataLength = from->DataLength;
        memcpy(dest->Data, from->Data, SAHPI_MAX_TEXT_BUFFER_LENGTH);
        return(SA_OK);
}

SaErrorT oh_copy_bigtext(oh_big_textbuffer *dest, const oh_big_textbuffer *from)
{
        if (!dest || !from) {
                err("Invalid parameter.");
                return(SA_ERR_HPI_INVALID_PARAMS);
        }

        dest->DataType = from->DataType;
        dest->Language = from->Language;
        dest->DataLength = from->DataLength;
        memcpy(dest->Data, from->Data, OH_MAX_TEXT_BUFFER_LENGTH);
        return(SA_OK);
}

/**
 * oh_append_textbuffer:
 * @buffer: SaHpiTextBufferT to append to.
 * @from: String to be appended.
 *
 * Appends a string to @buffer->Data.
 *
 * Returns:
 * SA_OK - Normal operation.
 * SA_ERR_HPI_INVALID_PARAMS - Pointer parameter(s) are NULL.
 * SA_ERR_HPI_OUT_OF_SPACE - @buffer not big enough to accomodate appended string.
 **/
SaErrorT oh_append_textbuffer(SaHpiTextBufferT *buffer, const char *from)
{
        char *p;
        uint size;

        if (!buffer || !from) {
                err("Invalid parameter.");
                return(SA_ERR_HPI_INVALID_PARAMS);
        }
        size = strlen(from);
        if ((size + buffer->DataLength) >= SAHPI_MAX_TEXT_BUFFER_LENGTH) {
                err("Cannot append to text buffer. Bufsize=%d, size=%u",
                    buffer->DataLength, size);
                return(SA_ERR_HPI_OUT_OF_SPACE);
        }

        /* Can't trust NULLs to be right, so use a targeted strncpy instead */
        p = (char *)buffer->Data;
        p += buffer->DataLength;
        strncpy(p, from, size);
        buffer->DataLength += size;

        return(SA_OK);
}

SaErrorT oh_append_bigtext(oh_big_textbuffer *big_buffer, const char *from)
{
        char *p;
        uint size;

        if (!big_buffer || !from) {
                err("Invalid parameters");
                return(SA_ERR_HPI_INVALID_PARAMS);
        }
        size = strlen(from);
        if ((size + big_buffer->DataLength) >= OH_MAX_TEXT_BUFFER_LENGTH) {
                err("Cannot append to buffer. Bufsize=%d, size=%u",
                    big_buffer->DataLength, size);
                return(SA_ERR_HPI_INTERNAL_ERROR);
        }

        /* Can't trust NULLs to be right, so use a targeted strncpy instead */
        p = (char *)big_buffer->Data;
        p += big_buffer->DataLength;
        strncpy(p, from, size);
        big_buffer->DataLength += size;

        return(SA_OK);
}

static inline SaErrorT oh_append_data(oh_big_textbuffer *big_buffer, const SaHpiUint8T *from, SaHpiUint8T len)
{
        SaHpiUint8T i;

        if (!big_buffer || !from || len == 0) {
                err("Invalid parameters");
                return(SA_ERR_HPI_INVALID_PARAMS);
        }

        for (i=0; i<len; i++) {
                char *p;
                char buff[10];
                int slen;

                memset(buff, 0 ,sizeof(buff));
                snprintf(buff, 10, "%d ", *(from + i));

                slen = strlen(buff);

                if ((slen + big_buffer->DataLength) >= OH_MAX_TEXT_BUFFER_LENGTH) {
                        err("Cannot append to buffer. Bufsize=%d, len=%d",
                            big_buffer->DataLength, len);
                        return(SA_ERR_HPI_INTERNAL_ERROR);
                }

                p = (char *)big_buffer->Data;
                p += big_buffer->DataLength;
                strncpy(p, buff, slen);
                big_buffer->DataLength += slen;
        }

        return(SA_OK);
}

/* Append an arbitrary number of fixed offset strings to a big text buffer */
SaErrorT oh_append_offset(oh_big_textbuffer *buffer, int offsets)
{
        int i;

        for (i=0; i < offsets; i++) {
                oh_append_bigtext(buffer, OH_PRINT_OFFSET);
        }

        return(SA_OK);
}

/**
 * oh_fprint_ctrlrec:
 * @stream: File handle.
 * @control: Pointer to SaHpiCtrlRecT to be printed.
 * @offsets: Number of offsets to start printing structure.
 *
 * Prints a control's SaHpiCtrlRecT data to a file.
 * The MACRO oh_print_ctrlrec(), uses this function to print to STDOUT.
 *
 * Returns:
 * SA_OK - Normal operation.
 * SA_ERR_HPI_INVALID_PARAMS - Pointer parameter(s) are NULL.
 **/
SaErrorT oh_fprint_ctrlrec(FILE *stream, const SaHpiCtrlRecT *control, int offsets)
{
        SaErrorT err;
        oh_big_textbuffer buffer;

        if (!stream || !control) {
                err("Invalid parameter.");
                return(SA_ERR_HPI_INVALID_PARAMS);
        }

        oh_init_bigtext(&buffer);
        err = oh_build_ctrlrec(&buffer, control, offsets);
        if (err) { return(err); }

        err = oh_fprint_bigtext(stream, &buffer);
        if (err) { return(err); }

        return(SA_OK);
}

/**
 * oh_fprint_watchdogrec:
 * @stream: File handle.
 * @watchdog: Pointer to SaHpiWatchdogRecT to be printed.
 * @offsets: Number of offsets to start printing structure.
 *
 * Prints a watchdog's SaHpiWatchdogRecT data to a file.
 * The MACRO oh_print_watchdogrec(), uses this function to print to STDOUT.
 *
 * Returns:
 * SA_OK - Normal operation.
 * SA_ERR_HPI_INVALID_PARAMS - Pointer parameter(s) are NULL.
 **/
SaErrorT oh_fprint_watchdogrec(FILE *stream, const SaHpiWatchdogRecT *watchdog, int offsets)
{
        SaErrorT err;
        oh_big_textbuffer buffer;

        if (!stream || !watchdog) {
                err("Invalid parameter.");
                return(SA_ERR_HPI_INVALID_PARAMS);
        }

        oh_init_bigtext(&buffer);
        err = oh_build_wdogrec(&buffer, watchdog, offsets);
        if (err) { return(err); }

        err = oh_fprint_bigtext(stream, &buffer);
        if (err) { return(err); }

        return(SA_OK);
}

/**
 * oh_fprint_sensorrec:
 * @stream: File handle.
 * @sensor: Pointer to SaHpiSensorRecT to be printed.
 * @offsets: Number of offsets to start printing structure.
 *
 * Prints a sensor's SaHpiSensorRecT data to a file.
 * The MACRO oh_print_sensorrec(), uses this function to print to STDOUT.
 *
 * Returns:
 * SA_OK - Normal operation.
 * SA_ERR_HPI_INVALID_PARAMS - Pointer parameter(s) are NULL.
 **/
SaErrorT oh_fprint_sensorrec(FILE *stream, const SaHpiSensorRecT *sensor, int offsets)
{
        SaErrorT err;
        oh_big_textbuffer buffer;

        if (!stream || !sensor) {
                err("Invalid parameter.");
                return(SA_ERR_HPI_INVALID_PARAMS);
        }

        oh_init_bigtext(&buffer);
        err = oh_build_sensorrec(&buffer, sensor, offsets);
        if (err) { return(err); }

        err = oh_fprint_bigtext(stream, &buffer);
        if (err) { return(err); }

        return(SA_OK);
}

static SaErrorT oh_build_resourceinfo(oh_big_textbuffer *buffer, const SaHpiResourceInfoT *ResourceInfo, int offsets)
{
        char str[SAHPI_MAX_TEXT_BUFFER_LENGTH];
        char tempstr[SAHPI_MAX_TEXT_BUFFER_LENGTH];
        int found;
        oh_big_textbuffer working;
        SaHpiTextBufferT tmpbuffer;
        SaErrorT err;

        if (!buffer || !ResourceInfo) {
                err("Invalid parameter.");
                return(SA_ERR_HPI_INVALID_PARAMS);
        }

        oh_append_offset(buffer, offsets);
        oh_append_bigtext(buffer, "Resource Information: ");

        /* Initial temp buffer - assumming we find something to print */
        err = oh_init_bigtext(&working);
        if (err) { return(err); }
        err = oh_append_bigtext(&working, "\n");
        if (err) { return(err); }

        offsets++;
        found = 0;
        if (ResourceInfo->ResourceRev) {
                oh_append_offset(&working, offsets);
                snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "Resource Revision: %d\n", ResourceInfo->ResourceRev);
                oh_append_bigtext(&working, str);
                found++;
        }
        if (ResourceInfo->SpecificVer) {
                oh_append_offset(&working, offsets);
                snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "Specific Version: %d\n", ResourceInfo->SpecificVer);
                oh_append_bigtext(&working, str);
                found++;
        }
        if (ResourceInfo->DeviceSupport) {
                oh_append_offset(&working, offsets);
                snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "Device Support: %x\n", ResourceInfo->DeviceSupport);
                oh_append_bigtext(&working, str);
                found++;
        }
        if (ResourceInfo->ManufacturerId) {
                oh_append_offset(&working, offsets);
                snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "Manufacturer ID: ");
                oh_append_bigtext(&working, str);
                oh_decode_manufacturerid(ResourceInfo->ManufacturerId, &tmpbuffer);
                oh_append_bigtext(&working, (char *)tmpbuffer.Data);
                oh_append_bigtext(&working, "\n");
                found++;
        }
        if (ResourceInfo->ProductId) {
                oh_append_offset(&working, offsets);
                snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "Product ID: %d\n", ResourceInfo->ProductId);
                oh_append_bigtext(&working, str);
                found++;
        }
        if (ResourceInfo->FirmwareMajorRev) {
                oh_append_offset(&working, offsets);
                snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "Firmware Major Revision: %d\n",
                         ResourceInfo->FirmwareMajorRev);
                oh_append_bigtext(&working, str);
                found++;
        }
        if (ResourceInfo->FirmwareMinorRev) {
                oh_append_offset(&working, offsets);
                snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "Firmware Minor Revision: %d\n",
                         ResourceInfo->FirmwareMinorRev);
                oh_append_bigtext(&working, str);
                found++;
        }
        if (ResourceInfo->AuxFirmwareRev) {
                oh_append_offset(&working, offsets);
                snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "Aux Firmware Revision: %d\n",
                         ResourceInfo->AuxFirmwareRev);
                oh_append_bigtext(&working, str);
                found++;
        }
        {
                SaHpiGuidT empty_guid;
                memset(empty_guid, 0, sizeof(SaHpiGuidT));

                if (memcmp(empty_guid, ResourceInfo->Guid, sizeof(SaHpiGuidT))) {
#if defined(__sun) && defined(__SVR4)
                        uuid_unparse((unsigned char *)ResourceInfo->Guid, tempstr);
#else
                        uuid_unparse(ResourceInfo->Guid, tempstr);
#endif
                        oh_append_offset(&working, offsets);
                        snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "GUID: %s\n",
                                        tempstr);
                        oh_append_bigtext(&working, str);
                        found++;
                }
        }

        if (!found) {
                oh_init_bigtext(&working);
                oh_append_bigtext(&working, "None\n");
        }

        oh_append_bigtext(buffer, (char *)working.Data);

        return(SA_OK);
}

static SaErrorT oh_build_sensorrec(oh_big_textbuffer *buffer, const SaHpiSensorRecT *sensor, int offsets)
{
        char str[SAHPI_MAX_TEXT_BUFFER_LENGTH];
        SaErrorT err;
        SaHpiTextBufferT tmpbuffer;

        /* Sensor Num */
        oh_append_offset(buffer, offsets);
        snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "Sensor Num: %d (%x hex)\n",
                 sensor->Num, sensor->Num);
        oh_append_bigtext(buffer, str);
        offsets++;

        /* Sensor Type */
        oh_append_offset(buffer, offsets);
        snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "Type: %s\n",
                 oh_lookup_sensortype(sensor->Type));
        oh_append_bigtext(buffer, str);

        /* Sensor Category */
        oh_append_offset(buffer, offsets);
        snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "Category: %s\n",
                 oh_lookup_eventcategory(sensor->Category));
        oh_append_bigtext(buffer, str);

        /* Sensor Enable Control */
        oh_append_offset(buffer, offsets);
        snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "EnableCtrl: %s\n",
                (sensor->EnableCtrl == SAHPI_TRUE) ? "TRUE" : "FALSE");
        oh_append_bigtext(buffer, str);

        /* Sensor Event Control */
        oh_append_offset(buffer, offsets);
        snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "EventCtrl: %s\n",
                 oh_lookup_sensoreventctrl(sensor->EventCtrl));
        oh_append_bigtext(buffer, str);

        /* Sensor Supported Events */
        oh_append_offset(buffer, offsets);
        oh_append_bigtext(buffer, "Events: ");
        err = oh_decode_eventstate(sensor->Events, sensor->Category, &tmpbuffer);
        if (err) {oh_append_bigtext(buffer, "\n"); return(err); }
        oh_append_bigtext(buffer, (char *)tmpbuffer.Data);
        oh_append_bigtext(buffer, "\n");

        /* Sensor Data Format */
        err = oh_build_sensordataformat(buffer, &(sensor->DataFormat), offsets);
        if (err) { return(err); }

        /* Sensor Threshold Definition */
        err = oh_build_sensorthddefn(buffer, &(sensor->ThresholdDefn), offsets);
        if (err) { return(err); }

        /* Sensor OEM Data */
        oh_append_offset(buffer, offsets);
        snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "OEM: %x\n", sensor->Oem);
        oh_append_bigtext(buffer, str);

        /* printf("SENSOR LENGTH = %d\n", strlen(buffer->Data)); */
        return(SA_OK);
}

static SaErrorT oh_build_sensordataformat(oh_big_textbuffer *buffer,
                                          const SaHpiSensorDataFormatT *format,
                                          int offsets)
{
        char str[SAHPI_MAX_TEXT_BUFFER_LENGTH];
        SaErrorT err;
        SaHpiTextBufferT reading_buffer;

        /* Sensor Data Format Title */
        oh_append_offset(buffer, offsets);
        oh_append_bigtext(buffer, "Data Format:\n");
        offsets++;

        /* Sensor Data Format IsSupported */
        oh_append_offset(buffer, offsets);
        snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "IsSupported: %s\n",
                 (format->IsSupported == SAHPI_TRUE) ? "TRUE" : "FALSE");
        oh_append_bigtext(buffer, str);

        if (format->IsSupported) {

                /* Sensor Data Format Reading Type */
                oh_append_offset(buffer, offsets);
                snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "Reading Type: %s\n",
                         oh_lookup_sensorreadingtype(format->ReadingType));
                oh_append_bigtext(buffer, str);

                if (format->ReadingType != SAHPI_SENSOR_READING_TYPE_BUFFER) {

                        /* Sensor Data Format Base Units */
                        oh_append_offset(buffer, offsets);
                        snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "Base Unit: %s\n",
                                 oh_lookup_sensorunits(format->BaseUnits));
                        oh_append_bigtext(buffer, str);

                        /* Sensor Data Format Modifier Units */
                        if (format->ModifierUnits) {
                                oh_append_offset(buffer, offsets);
                                snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "Modifier Unit: %s\n",
                                         oh_lookup_sensorunits(format->ModifierUnits));
                                oh_append_bigtext(buffer, str);
                                /* Sensor Data Format Modifier Use */
                                oh_append_offset(buffer, offsets);
                                snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "Modifier Use: %s\n",
                                         oh_lookup_sensormodunituse(format->ModifierUse));
                                oh_append_bigtext(buffer, str);
                        }

                        /* Sensor Data Format Percentage */
                        oh_append_offset(buffer, offsets);
                        snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "Percentage: %s\n",
                                 (format->Percentage == SAHPI_TRUE) ? "TRUE" : "FALSE");
                        oh_append_bigtext(buffer, str);

                        /* Sensor Data Format Max Range */
                        if (format->Range.Flags & SAHPI_SRF_MAX &&
                            format->Range.Max.IsSupported) {
                                oh_append_offset(buffer, offsets);
                                oh_append_bigtext(buffer, "Range Max: ");

                                err = oh_decode_sensorreading(format->Range.Max,
                                                              *format,
                                                              &reading_buffer);
                                if (err) { return(err); }
                                oh_append_bigtext(buffer, (char *)reading_buffer.Data);
                                oh_append_bigtext(buffer, "\n");
                        }

                        /* Sensor Data Format Min Range */
                        if (format->Range.Flags & SAHPI_SRF_MIN &&
                            format->Range.Min.IsSupported) {
                                oh_append_offset(buffer, offsets);
                                oh_append_bigtext(buffer, "Range Min: ");

                                err = oh_decode_sensorreading(format->Range.Min,
                                                              *format,
                                                              &reading_buffer);
                                if (err) { return(err); }
                                oh_append_bigtext(buffer, (char *)reading_buffer.Data);
                                oh_append_bigtext(buffer, "\n");
                        }

                        /* Sensor Data Format Nominal Range */
                        if (format->Range.Flags & SAHPI_SRF_NOMINAL &&
                            format->Range.Nominal.IsSupported) {
                                oh_append_offset(buffer, offsets);
                                oh_append_bigtext(buffer, "Range Nominal: ");

                                err = oh_decode_sensorreading(format->Range.Nominal,
                                                              *format,
                                                              &reading_buffer);
                                if (err) { return(err); }
                                oh_append_bigtext(buffer, (char *)reading_buffer.Data);
                                oh_append_bigtext(buffer, "\n");
                        }

                        /* Sensor Data Format Normal Max Range */
                        if (format->Range.Flags & SAHPI_SRF_NORMAL_MAX &&
                            format->Range.NormalMax.IsSupported) {
                                oh_append_offset(buffer, offsets);
                                oh_append_bigtext(buffer, "Range Normal Max: ");

                                err = oh_decode_sensorreading(format->Range.NormalMax,
                                                              *format,
                                                              &reading_buffer);
                                if (err) { return(err); }
                                oh_append_bigtext(buffer, (char *)reading_buffer.Data);
                                oh_append_bigtext(buffer, "\n");
                        }

                        /* Sensor Data Format Normal Min Range */
                        if (format->Range.Flags & SAHPI_SRF_NORMAL_MIN &&
                            format->Range.NormalMin.IsSupported) {
                                oh_append_offset(buffer, offsets);
                                oh_append_bigtext(buffer, "Range Normal Min: ");

                                err = oh_decode_sensorreading(format->Range.NormalMin,
                                                              *format,
                                                              &reading_buffer);
                                if (err) { return(err); }
                                oh_append_bigtext(buffer, (char *)reading_buffer.Data);
                                oh_append_bigtext(buffer, "\n");
                        }

                        /* Sensor Data Format Accuracy Factor */
                        oh_append_offset(buffer, offsets);
                        snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "Accuracy: %lf\n", format->AccuracyFactor);
                        oh_append_bigtext(buffer, str);
                }
        }

        return(SA_OK);
}

static SaErrorT oh_build_sensorthddefn(oh_big_textbuffer *buffer,
                                       const SaHpiSensorThdDefnT *tdef,
                                       int offsets)
{
        char str[SAHPI_MAX_TEXT_BUFFER_LENGTH];
        SaErrorT err;

        /* Sensor Threshold Definition Title */
        oh_append_offset(buffer, offsets);
        oh_append_bigtext(buffer, "Threshold Definitions:\n");
        offsets++;

        /* Sensor Threshold Definition IsAccessible */
        oh_append_offset(buffer, offsets);
        snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "IsAccessible: %s\n",
                 (tdef->IsAccessible == SAHPI_TRUE) ? "TRUE" : "FALSE");
        oh_append_bigtext(buffer, str);

        if (tdef->IsAccessible) {

                /* Sensor Threshold Read Threshold */
                if (tdef->ReadThold) {
                        oh_append_offset(buffer, offsets);
                        oh_append_bigtext(buffer, "Readable Thresholds:\n");

                        err = oh_build_threshold_mask(buffer, tdef->ReadThold, offsets + 1);
                        if (err) { return(err); }
                }

                /* Sensor Threshold Write Threshold */
                if (tdef->WriteThold) {
                        oh_append_offset(buffer, offsets);
                        oh_append_bigtext(buffer, "Writeable Thresholds:\n");

                        err = oh_build_threshold_mask(buffer, tdef->WriteThold, offsets + 1);
                        if (err) { return(err); }
                }

                /* Sensor Threshold Nonlinear */
                oh_append_offset(buffer, offsets);
                snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "Nonlinear: %s\n",
                         (tdef->Nonlinear == SAHPI_TRUE) ? "TRUE" : "FALSE");
                oh_append_bigtext(buffer, str);
        }

        return(SA_OK);
}

SaErrorT oh_build_threshold_mask(oh_big_textbuffer *buffer,
                                 const SaHpiSensorThdMaskT tmask,
                                 int offsets)
{
        oh_append_offset(buffer, offsets);

        if (tmask == 0) {
                oh_append_bigtext(buffer, "None");
                oh_append_bigtext(buffer, "\n");
                return SA_OK;
        }

        if (tmask & SAHPI_STM_LOW_MINOR) {
                oh_append_bigtext(buffer, "LOW_MINOR");
                oh_append_bigtext(buffer, OH_ENCODE_DELIMITER);
        }
        if (tmask & SAHPI_STM_LOW_MAJOR) {
                oh_append_bigtext(buffer, "LOW_MAJOR");
                oh_append_bigtext(buffer, OH_ENCODE_DELIMITER);
        }
        if (tmask & SAHPI_STM_LOW_CRIT) {
                oh_append_bigtext(buffer, "LOW_CRIT");
                oh_append_bigtext(buffer, OH_ENCODE_DELIMITER);
        }
        if (tmask & SAHPI_STM_LOW_HYSTERESIS) {
                oh_append_bigtext(buffer, "LOW_HYSTERESIS");
                oh_append_bigtext(buffer, OH_ENCODE_DELIMITER);
        }
        if (tmask & SAHPI_STM_UP_MINOR) {
                oh_append_bigtext(buffer, "UP_MINOR");
                oh_append_bigtext(buffer, OH_ENCODE_DELIMITER);
        }
        if (tmask & SAHPI_STM_UP_MAJOR) {
                oh_append_bigtext(buffer, "UP_MAJOR");
                oh_append_bigtext(buffer, OH_ENCODE_DELIMITER);
        }
        if (tmask & SAHPI_STM_UP_CRIT) {
                oh_append_bigtext(buffer, "UP_CRIT");
                oh_append_bigtext(buffer, OH_ENCODE_DELIMITER);
        }
        if (tmask & SAHPI_STM_UP_HYSTERESIS) {
                oh_append_bigtext(buffer, "UP_HYSTERESIS");
                oh_append_bigtext(buffer, OH_ENCODE_DELIMITER);
        }

        /* Remove last delimiter; add NL */
        buffer->Data[buffer->DataLength-OH_ENCODE_DELIMITER_LENGTH] = '\0';
        buffer->DataLength = buffer->DataLength - OH_ENCODE_DELIMITER_LENGTH;

        oh_append_bigtext(buffer, "\n");

        return(SA_OK);
}

/**
 * oh_fprint_idrfield:
 * @stream: File handle.
 * @thisfield: Pointer to SaHpiIdrFieldT to be printed.
 * @offsets: Number of offsets to start printing structure.
 *
 * Prints the member data contained in SaHpiIdrFieldT struct to a file.
 * The MACRO oh_print_idrfield(), uses this function to print to STDOUT.
 *
 * Returns:
 * SA_OK - Normal operation.
 **/
SaErrorT oh_fprint_idrfield(FILE *stream, const SaHpiIdrFieldT *thisfield, int offsets)
{
        char str[SAHPI_MAX_TEXT_BUFFER_LENGTH+1];
        oh_big_textbuffer mybuf;
        SaErrorT err;

        if (!stream || !thisfield) return(SA_ERR_HPI_INVALID_PARAMS);

        oh_init_bigtext(&mybuf);
        oh_append_offset(&mybuf, offsets);
        snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "Field Id: %d\n", thisfield->FieldId);
        oh_append_bigtext(&mybuf, str);

        oh_append_offset(&mybuf, offsets);
        snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "Field Type: %s\n",
                 oh_lookup_idrfieldtype(thisfield->Type));
        oh_append_bigtext(&mybuf, str);

        oh_append_offset(&mybuf, offsets);
        snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "ReadOnly: %s\n",
                 (thisfield->ReadOnly == SAHPI_TRUE) ? "TRUE" : "FALSE");
        oh_append_bigtext(&mybuf, str);

        oh_append_offset(&mybuf, offsets);
        snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "Data Type: %s\n",
                 oh_lookup_texttype(thisfield->Field.DataType));
        oh_append_bigtext(&mybuf, str);

        oh_append_offset(&mybuf, offsets);
        snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "Language: %s\n",
                 oh_lookup_language(thisfield->Field.Language));
        oh_append_bigtext(&mybuf, str);

        oh_append_offset(&mybuf, offsets);
        oh_append_bigtext(&mybuf, "Content: ");
        if (thisfield->Field.DataLength == 0)
                oh_append_bigtext(&mybuf, "NULL\n");
        else {
                if (thisfield->Field.DataType == SAHPI_TL_TYPE_BINARY)
                        oh_append_data(&mybuf, thisfield->Field.Data, thisfield->Field.DataLength);
                else {
                        memcpy( str, thisfield->Field.Data, thisfield->Field.DataLength );
                        str[thisfield->Field.DataLength] = '\0';
                        oh_append_bigtext(&mybuf, str);
                }
        }

        err = oh_fprint_bigtext(stream, &mybuf);

        return(err);
}

/**
 * oh_fprint_idrareaheader:
 * @stream: File handle.
 * @areaheader: Pointer to SaHpiIdrAreaHeaderT to be printed.
 * @offsets: Number of offsets to start printing structure.
 *
 * Prints the member data contained in SaHpiIdrAreaHeaderT struct to a file.
 * The MACRO oh_print_idrareaheader(), uses this function to print to STDOUT.
 *
 * Returns:
 * SA_OK - Normal operation.
 **/
SaErrorT oh_fprint_idrareaheader(FILE *stream, const SaHpiIdrAreaHeaderT *areaheader, int offsets)
{
        char str[SAHPI_MAX_TEXT_BUFFER_LENGTH];
        oh_big_textbuffer mybuf;
        SaErrorT err;

        if (!stream || !areaheader) return(SA_ERR_HPI_INVALID_PARAMS);

        oh_init_bigtext(&mybuf);
        oh_append_offset(&mybuf, offsets);
        snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "AreaId: %d\n", areaheader->AreaId);
        oh_append_bigtext(&mybuf, str);

        oh_append_offset(&mybuf, offsets);
        snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "AreaType: %s\n",
                 oh_lookup_idrareatype(areaheader->Type));
        oh_append_bigtext(&mybuf, str);

        oh_append_offset(&mybuf, offsets);
        snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "ReadOnly: %s\n",
                 (areaheader->ReadOnly == SAHPI_TRUE) ? "TRUE" : "FALSE" );
        oh_append_bigtext(&mybuf, str);

        oh_append_offset(&mybuf, offsets);
        snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "NumFields: %d\n", areaheader->NumFields);
        oh_append_bigtext(&mybuf, str);

        err = oh_fprint_bigtext(stream, &mybuf);
        return(err);
}

/**
 * oh_fprint_idrinfo:
 * @stream: File handle.
 * @idrinfo: Pointer to SaHpiIdrInfoT to be printed.
 * @offsets: Number of offsets to start printing structure.
 *
 * Prints the member data contained in SaHpiIdrInfoT struct to a file.
 * The MACRO oh_print_idrinfo(), uses this function to print to STDOUT.
 *
 * Returns:
 * SA_OK - Normal operation.
 * SA_ERR_HPI_INVALID_PARAMS - Pointer parameter(s) are NULL.
 **/
SaErrorT oh_fprint_idrinfo(FILE *stream, const SaHpiIdrInfoT *idrinfo, int offsets)
{
        SaErrorT err;
        char str[SAHPI_MAX_TEXT_BUFFER_LENGTH];
        oh_big_textbuffer mybuf;

        if (!stream || !idrinfo) {
                err("Invalid parameter.");
                return(SA_ERR_HPI_INVALID_PARAMS);
        }

        oh_init_bigtext(&mybuf);
        oh_append_offset(&mybuf, offsets);
        snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "IdrId: %d\n", idrinfo->IdrId);
        oh_append_bigtext(&mybuf, str);

        oh_append_offset(&mybuf, offsets);
        snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "UpdateCount: %d\n", idrinfo->UpdateCount);
        oh_append_bigtext(&mybuf, str);

        oh_append_offset(&mybuf, offsets);
        snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "ReadOnly: %s\n",
                                (idrinfo->ReadOnly == SAHPI_TRUE) ? "TRUE" : "FALSE" );
        oh_append_bigtext(&mybuf, str);

        oh_append_offset(&mybuf, offsets);
        snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "NumAreas: %d\n", idrinfo->NumAreas);
        oh_append_bigtext(&mybuf, str);

        err = oh_fprint_bigtext(stream, &mybuf);
        return(err);
}

SaErrorT oh_fprint_textbuffer(FILE *stream, const SaHpiTextBufferT *textbuffer, int offsets) {

        SaErrorT err;
        oh_big_textbuffer buffer;

        if (!stream || !textbuffer) {
                err("Invalid parameter.");
                return(SA_ERR_HPI_INVALID_PARAMS);
        }

        oh_init_bigtext(&buffer);
        err = oh_build_textbuffer(&buffer, textbuffer, offsets);
        if (err) { return(err); }

        err = oh_fprint_bigtext(stream, &buffer);

        return(err);
}

static SaErrorT oh_build_textbuffer(oh_big_textbuffer *buffer, const SaHpiTextBufferT *textbuffer, int offsets)
{
        char str[SAHPI_MAX_TEXT_BUFFER_LENGTH];
        SaHpiTextBufferT working_textbuffer;

        memset(&working_textbuffer, 0, sizeof(working_textbuffer));
        oh_copy_textbuffer(&working_textbuffer, textbuffer);

        /* Text Buffer Data Type */
        oh_append_offset(buffer, offsets);
        snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "Data Type: %s\n",
                 oh_lookup_texttype(textbuffer->DataType));
        oh_append_bigtext(buffer, str);

        /* Text Buffer Language */
        oh_append_offset(buffer, offsets);
        snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "Language: %s\n",
                 oh_lookup_language(textbuffer->Language));
        oh_append_bigtext(buffer, str);

        /* Text Buffer Data Length */
        oh_append_offset(buffer, offsets);
        snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "Data Length: %d\n",
                 textbuffer->DataLength);
        oh_append_bigtext(buffer, str);

        /* Text Buffer Data */
        if (textbuffer->DataLength) {
                oh_append_offset(buffer, offsets);
                memset(str, 0, SAHPI_MAX_TEXT_BUFFER_LENGTH);
                oh_append_bigtext(buffer, "Data: ");
                if (textbuffer->DataType == SAHPI_TL_TYPE_BINARY)
                        oh_append_data(buffer, textbuffer->Data, textbuffer->DataLength);
                else {
                        working_textbuffer.Data[working_textbuffer.DataLength] = '\0';
                        oh_append_bigtext(buffer, (const char *)&working_textbuffer.Data);
                }
                oh_append_bigtext(buffer, "\n");
        }

        return(SA_OK);
}


/**
 * oh_decode_capabilities:
 * @ResourceCapabilities: enum value of type SaHpiCapabilitiesT.
 * @buffer:  Location to store the string.
 *
 * Converts @ResourceCapabilities type into a string based on
 * @ResourceCapabilities HPI definition. For example:
 * @ResourceCapabilities = SAHPI_CAPABILITY_RESOURCE | SAHPI_CAPABILITY_EVENT_LOG
 * returns a string "RESOURCE | EVENT_LOG".
 *
 * Returns:
 * SA_OK - Normal operation.
 * SA_ERR_HPI_INVALID_PARAMS - @buffer is NULL
 **/
SaErrorT oh_decode_capabilities(SaHpiCapabilitiesT ResourceCapabilities,
                                SaHpiTextBufferT *buffer)
{
        int found;
        SaErrorT err;
        SaHpiTextBufferT working;

        if (!buffer) {
                err("Invalid parameter.");
                return(SA_ERR_HPI_INVALID_PARAMS);
        }

        err = oh_init_textbuffer(&working);
        if (err) { return(err); }

        found = 0;
        if (ResourceCapabilities & SAHPI_CAPABILITY_AGGREGATE_STATUS) {
                found++;
                err = oh_append_textbuffer(&working, "AGGREGATE_STATUS | ");
                if (err) { return(err); }
        }
        if (ResourceCapabilities & SAHPI_CAPABILITY_ANNUNCIATOR) {
                found++;
                err = oh_append_textbuffer(&working, "ANNUNCIATOR | ");
                if (err) { return(err); }
        }
        if (ResourceCapabilities & SAHPI_CAPABILITY_CONFIGURATION) {
                found++;
                err = oh_append_textbuffer(&working, "CONFIGURATION | ");
                if (err) { return(err); }
        }
        if (ResourceCapabilities & SAHPI_CAPABILITY_CONTROL) {
                found++;
                err = oh_append_textbuffer(&working, "CONTROL | ");
                if (err) { return(err); }
        }
        if (ResourceCapabilities & SAHPI_CAPABILITY_DIMI) {
                found++;
                err = oh_append_textbuffer(&working, "DIMI | ");
                if (err) { return(err); }
        }
        if (ResourceCapabilities & SAHPI_CAPABILITY_EVENT_LOG) {
                found++;
                err = oh_append_textbuffer(&working, "EVENT_LOG | ");
                if (err) { return(err); }
        }
        if (ResourceCapabilities & SAHPI_CAPABILITY_EVT_DEASSERTS) {
                found++;
                err = oh_append_textbuffer(&working, "EVT_DEASSERTS | ");
                if (err) { return(err); }
        }
        if (ResourceCapabilities & SAHPI_CAPABILITY_FRU) {
                found++;
                err = oh_append_textbuffer(&working, "FRU | ");
                if (err) { return(err); }
        }
        if (ResourceCapabilities & SAHPI_CAPABILITY_FUMI) {
                found++;
                err = oh_append_textbuffer(&working, "FUMI | ");
                if (err) { return(err); }
        }
        if (ResourceCapabilities & SAHPI_CAPABILITY_INVENTORY_DATA) {
                found++;
                err = oh_append_textbuffer(&working, "INVENTORY_DATA | ");
                if (err) { return(err); }
        }
        if (ResourceCapabilities & SAHPI_CAPABILITY_MANAGED_HOTSWAP) {
                found++;
                err = oh_append_textbuffer(&working, "MANAGED_HOTSWAP | ");
                if (err) { return(err); }
        }
        if (ResourceCapabilities & SAHPI_CAPABILITY_POWER) {
                found++;
                err = oh_append_textbuffer(&working, "POWER | ");
                if (err) { return(err); }
        }
        if (ResourceCapabilities & SAHPI_CAPABILITY_RDR) {
                found++;
                err = oh_append_textbuffer(&working, "RDR | ");
                if (err) { return(err); }
        }
        if (ResourceCapabilities & SAHPI_CAPABILITY_RESET) {
                found++;
                err = oh_append_textbuffer(&working, "RESET | ");
                if (err) { return(err); }
        }
        if (ResourceCapabilities & SAHPI_CAPABILITY_RESOURCE) {
                found++;
                err = oh_append_textbuffer(&working, "RESOURCE | ");
                if (err) { return(err); }
        }
        if (ResourceCapabilities & SAHPI_CAPABILITY_SENSOR) {
                found++;
                err = oh_append_textbuffer(&working, "SENSOR | ");
                if (err) { return(err); }
        }
        if (ResourceCapabilities & SAHPI_CAPABILITY_WATCHDOG) {
                found++;
                err = oh_append_textbuffer(&working, "WATCHDOG | ");
                if (err) { return(err); }
        }

        if (found) {
                working.DataLength -= OH_ENCODE_DELIMITER_LENGTH;
                working.Data[working.DataLength] = 0;
        }

        oh_copy_textbuffer(buffer, &working);

        return(SA_OK);
}

/**
 * oh_decode_hscapabilities:
 * @HsCapabilities: enum value of type SaHpiHsCapabilitiesT.
 * @buffer:  Location to store the string.
 *
 * Converts @HsCapabilities type into a string based on HPI definition. For example:
 * @HsCapabilities = SAHPI_HS_CAPABILITY_AUTOEXTRACT_READ_ONLY | SAHPI_HS_CAPABILITY_INDICATOR_SUPPORTED
 * returns a string "AUTOEXTRACT_READ_ONLY | INDICATOR_SUPPORTED".
 *
 * Returns:
 * SA_OK - Normal operation.
 * SA_ERR_HPI_INVALID_PARAMS - @buffer is NULL
 **/
SaErrorT oh_decode_hscapabilities(SaHpiHsCapabilitiesT HsCapabilities,
                                  SaHpiTextBufferT *buffer)
{
        int found;
        SaErrorT err;
        SaHpiTextBufferT working;

        if (!buffer) {
                err("Invalid parameter.");
                return(SA_ERR_HPI_INVALID_PARAMS);
        }

        err = oh_init_textbuffer(&working);
        if (err) { return(err); }

        found = 0;
        if (HsCapabilities & SAHPI_HS_CAPABILITY_AUTOEXTRACT_READ_ONLY) {
                found++;
                err = oh_append_textbuffer(&working, "AUTOEXTRACT_READ_ONLY | ");
                if (err) { return(err); }
        }
        if (HsCapabilities & SAHPI_HS_CAPABILITY_INDICATOR_SUPPORTED) {
                found++;
                err = oh_append_textbuffer(&working, "INDICATOR_SUPPORTED | ");
                if (err) { return(err); }
        }

        if (found) {
                working.DataLength -= OH_ENCODE_DELIMITER_LENGTH;
                working.Data[working.DataLength] = 0;
        }
        else {
                oh_append_textbuffer(&working, "None");
        }

        oh_copy_textbuffer(buffer, &working);

        return(SA_OK);
}

/**
 * oh_fprint_rptentry:
 * @stream: File handle.
 * @rptentry: Pointer to SaHpiRptEntryT to be printed.
 * @offsets:  Number of offsets to start printing structure.
 *
 * Prints the member data contained in SaHpiRptEntryT struct to a file.
 * The MACRO oh_print_rptentry(), uses this function to print to STDOUT.
 *
 * Returns:
 * SA_OK - Normal operation.
 **/
SaErrorT oh_fprint_rptentry(FILE *stream, const SaHpiRptEntryT *rptentry, int offsets)
{
        SaErrorT err = SA_OK;
        oh_big_textbuffer mybuf;
        SaHpiTextBufferT tmpbuffer;
        char* str = (char *)tmpbuffer.Data;

        if (!stream || !rptentry) {
                err("Invalid parameter.");
                return(SA_ERR_HPI_INVALID_PARAMS);
        }

        oh_init_bigtext(&mybuf);
        offsets++;

        oh_append_offset(&mybuf, offsets);
        snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "EntryId: %d\n", rptentry->EntryId);
        oh_append_bigtext(&mybuf, str);

        oh_append_offset(&mybuf, offsets);
        snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "ResourceId: %d\n", rptentry->ResourceId);
        oh_append_bigtext(&mybuf, str);

        err = oh_build_resourceinfo(&mybuf, &(rptentry->ResourceInfo), offsets);
        if (err) { return(err); }

        oh_append_offset(&mybuf, offsets);
        snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "Entity Path: ");
        oh_append_bigtext(&mybuf, str);
        {
                oh_big_textbuffer tmpbuf;

                oh_init_bigtext(&tmpbuf);
                oh_decode_entitypath(&rptentry->ResourceEntity, &tmpbuf);
                oh_append_bigtext(&mybuf, (char *)tmpbuf.Data);
        }
        oh_append_bigtext(&mybuf, "\n");

        oh_append_offset(&mybuf, offsets);
        snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "Capabilities: \n");
        oh_append_bigtext(&mybuf, str);
        oh_append_offset(&mybuf, offsets + 1);
        oh_decode_capabilities(rptentry->ResourceCapabilities, &tmpbuffer);
        oh_append_bigtext(&mybuf, (char *)tmpbuffer.Data);
        oh_append_bigtext(&mybuf, "\n");

        oh_append_offset(&mybuf, offsets);
        snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "HotSwap Capabilities: ");
        oh_append_bigtext(&mybuf, str);
        oh_decode_hscapabilities(rptentry->HotSwapCapabilities, &tmpbuffer);
        oh_append_bigtext(&mybuf, (char *)tmpbuffer.Data);
        oh_append_bigtext(&mybuf, "\n");

        oh_append_offset(&mybuf, offsets);
        snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "Resource Severity: %s\n",
                 oh_lookup_severity(rptentry->ResourceSeverity));
        oh_append_bigtext(&mybuf, str);

        oh_append_offset(&mybuf, offsets);
        snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "ResourceFailed: %s\n",
                 (rptentry->ResourceFailed == SAHPI_TRUE) ? "TRUE" : "FALSE" );
        oh_append_bigtext(&mybuf, str);

        oh_append_offset(&mybuf, offsets);
        snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "ResourceTag:\n");
        oh_append_bigtext(&mybuf, str);
        oh_build_textbuffer(&mybuf, &(rptentry->ResourceTag), offsets + 1);

        err = oh_fprint_bigtext(stream, &mybuf);
        return(err);
}

/**
 * oh_fprint_rdr:
 * @stream: File handle.
 * @thisrdr: Pointer to SaHpiRdrT to be printed.
 * @offsets: Number of offsets to start printing structure.
 *
 * Prints the member data contained in SaHpiRdrT struct to a file.
 * The MACRO oh_print_rdr(), uses this function to print to STDOUT.
 *
 * Returns:
 * SA_OK - Normal operation.
 * SA_ERR_HPI_INVALID_PARAMS - Pointer parameter(s) are NULL.
 **/
SaErrorT oh_fprint_rdr(FILE *stream, const SaHpiRdrT *thisrdr, int offsets)
{
        SaErrorT err;
        char str[SAHPI_MAX_TEXT_BUFFER_LENGTH];
        oh_big_textbuffer mybuf, mybuf1;

        if (!stream || !thisrdr) {
                err("Invalid parameter.");
                return(SA_ERR_HPI_INVALID_PARAMS);
        }

        oh_init_bigtext(&mybuf);

        oh_append_offset(&mybuf, offsets);
        snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "RecordId: %d\n", thisrdr->RecordId);
        oh_append_bigtext(&mybuf, str);

        oh_append_offset(&mybuf, offsets);
        snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "RdrType: %s\n", oh_lookup_rdrtype(thisrdr->RdrType));
        oh_append_bigtext(&mybuf, str);

        oh_append_offset(&mybuf, offsets);
        snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "Entity Path: ");
        oh_append_bigtext(&mybuf, str);
        {
                oh_big_textbuffer tmpbuf;

                oh_init_bigtext(&tmpbuf);
                oh_decode_entitypath(&thisrdr->Entity, &tmpbuf);
                oh_append_bigtext(&mybuf, (char *)tmpbuf.Data);
        }
        oh_append_bigtext(&mybuf, "\n");

        oh_append_offset(&mybuf, offsets);
        snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "IsFru: %s\n",
                 (thisrdr->IsFru == SAHPI_TRUE) ? "TRUE" : "FALSE" );
        oh_append_bigtext(&mybuf, str);

        oh_init_bigtext(&mybuf1);
        switch(thisrdr->RdrType)
        {
                case SAHPI_CTRL_RDR:
                        err = oh_build_ctrlrec(&mybuf1,
                                (const SaHpiCtrlRecT*) &thisrdr->RdrTypeUnion.CtrlRec, offsets);
                        break;
                case SAHPI_SENSOR_RDR:
                        err = oh_build_sensorrec(&mybuf1,
                                (const SaHpiSensorRecT*) &thisrdr->RdrTypeUnion.SensorRec, offsets);
                        break;
                case SAHPI_INVENTORY_RDR:
                        err = oh_build_invrec(&mybuf1,
                                (const SaHpiInventoryRecT*) &thisrdr->RdrTypeUnion.InventoryRec, offsets);
                        break;
                case SAHPI_WATCHDOG_RDR:
                        err = oh_build_wdogrec(&mybuf1,
                                (const SaHpiWatchdogRecT*) &thisrdr->RdrTypeUnion.WatchdogRec, offsets);
                        break;
                case SAHPI_ANNUNCIATOR_RDR:
                        err = oh_build_annrec(&mybuf1,
                                (const SaHpiAnnunciatorRecT*) &thisrdr->RdrTypeUnion.AnnunciatorRec, offsets);
                        break;
                case SAHPI_DIMI_RDR:
                        err = oh_build_dimirec(&mybuf1,
                                (const SaHpiDimiRecT*) &thisrdr->RdrTypeUnion.DimiRec, offsets);
                        break;
                case SAHPI_FUMI_RDR:
                        err = oh_build_fumirec(&mybuf1,
                                (const SaHpiFumiRecT*) &thisrdr->RdrTypeUnion.FumiRec, offsets);
                        break;
                case SAHPI_NO_RECORD:
                        oh_append_offset(&mybuf1, offsets);
                        oh_append_bigtext(&mybuf1, oh_lookup_rdrtype(thisrdr->RdrType));
                        break;
                default:
                        oh_append_bigtext(&mybuf1, "Invalid/Unknown RDR Type\n");

        }

        oh_append_bigtext(&mybuf, (char *)mybuf1.Data);

        oh_append_offset(&mybuf, offsets);
        snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "IdString:\n");
        oh_append_bigtext(&mybuf, str);
        oh_build_textbuffer(&mybuf, &(thisrdr->IdString), offsets + 1);

        err = oh_fprint_bigtext(stream, &mybuf);
        return(err);
}

/**
 * oh_build_ctrlrec:
 * @textbuff: Buffer into which to store flattened ctrl rec structure.
 * @thisrdrunion: Pointer to SaHpiRdrTypeUnionT to be flattened.
 * @offsets: Number of offsets to start printing structure.
 *
 * Flatten member data contained in SaHpiCtrlRecT struct to a text buffer.
 *
 * Returns:
 * SA_OK - Normal operation.
 * SA_ERR_HPI_INVALID_PARAMS - Pointer parameter(s) are NULL.
 **/
static SaErrorT oh_build_ctrlrec(oh_big_textbuffer *textbuf, const SaHpiCtrlRecT *ctrlrec, int offsets)
{
        SaErrorT err;
        char str[SAHPI_MAX_TEXT_BUFFER_LENGTH];
        oh_big_textbuffer mybuf;
        SaHpiTextBufferT  smallbuf;

        if (!textbuf || !ctrlrec) {
                err("Invalid parameter.");
                return(SA_ERR_HPI_INVALID_PARAMS);
        }

        oh_init_bigtext(&mybuf);

        oh_append_offset(&mybuf, offsets);
        snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "Control Num: %d (%x hex)\n",
                 ctrlrec->Num, ctrlrec->Num);
        oh_append_bigtext(&mybuf, str);

        oh_append_offset(&mybuf, offsets);
        snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "Control Output Type: %s\n",
                 oh_lookup_ctrloutputtype(ctrlrec->OutputType));
        oh_append_bigtext(&mybuf, str);

        oh_append_offset(&mybuf, offsets);
        snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "Control Type: %s\n",
                 oh_lookup_ctrltype(ctrlrec->Type));
        oh_append_bigtext(&mybuf, str);
        oh_append_offset(&mybuf, offsets);

        switch(ctrlrec->Type) {
        case SAHPI_CTRL_TYPE_DIGITAL:
                snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "Digital Default: %s\n",
                         oh_lookup_ctrlstatedigital(ctrlrec->TypeUnion.Digital.Default));
                oh_append_bigtext(&mybuf, str);
                break;
        case SAHPI_CTRL_TYPE_DISCRETE:
                snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "Discrete Default: %d\n",
                         ctrlrec->TypeUnion.Discrete.Default);
                oh_append_bigtext(&mybuf, str);
                break;
        case SAHPI_CTRL_TYPE_ANALOG:
                snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "Analog Min: %d\n",
                         ctrlrec->TypeUnion.Analog.Min);
                oh_append_bigtext(&mybuf, str);
                oh_append_offset(&mybuf, offsets);
                snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "Analog Max: %d\n",
                         ctrlrec->TypeUnion.Analog.Max);
                oh_append_bigtext(&mybuf, str);
                oh_append_offset(&mybuf, offsets);
                snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "Analog Default: %d\n",
                         ctrlrec->TypeUnion.Analog.Default);
                oh_append_bigtext(&mybuf, str);
                break;
        case SAHPI_CTRL_TYPE_STREAM:
                snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "Stream Default:\n");
                oh_append_bigtext(&mybuf, str);
                oh_append_offset(&mybuf, offsets + 1);
                snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "Repeat: %s\n",
                         (ctrlrec->TypeUnion.Stream.Default.Repeat == SAHPI_TRUE) ? "TRUE" : "FALSE");
                oh_append_bigtext(&mybuf, str);
                oh_append_offset(&mybuf, offsets + 1);
                snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "StreamLength: %d\n",
                         ctrlrec->TypeUnion.Stream.Default.StreamLength);
                oh_append_bigtext(&mybuf, str);
                oh_append_offset(&mybuf, offsets + 1);
                snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "Stream Default: %s\n",
                         ctrlrec->TypeUnion.Stream.Default.Stream);
                oh_append_bigtext(&mybuf, str);
                break;
        case SAHPI_CTRL_TYPE_TEXT:
                snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "Max Chars: %d\n",
                         ctrlrec->TypeUnion.Text.MaxChars);
                oh_append_bigtext(&mybuf, str);
                oh_append_offset(&mybuf, offsets);
                snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "Max Lines: %d\n",
                         ctrlrec->TypeUnion.Text.MaxLines);
                oh_append_bigtext(&mybuf, str);
                oh_append_offset(&mybuf, offsets);
                snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "Language: %s\n",
                         oh_lookup_language(ctrlrec->TypeUnion.Text.Language));
                oh_append_bigtext(&mybuf, str);
                oh_append_offset(&mybuf, offsets);
                snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "Data Type: %s\n",
                         oh_lookup_texttype(ctrlrec->TypeUnion.Text.DataType));
                oh_append_bigtext(&mybuf, str);
                oh_append_offset(&mybuf, offsets);
                snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "Default:\n");
                oh_append_bigtext(&mybuf, str);
                oh_append_offset(&mybuf, offsets + 1);
                snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "Line: %d\n",
                         ctrlrec->TypeUnion.Text.Default.Line);
                oh_append_bigtext(&mybuf, str);
                oh_build_textbuffer(&mybuf, &(ctrlrec->TypeUnion.Text.Default.Text), offsets + 1);
                break;
        case SAHPI_CTRL_TYPE_OEM:
                err = oh_decode_manufacturerid(ctrlrec->TypeUnion.Oem.MId, &smallbuf);
                snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "ManufacturerId: %s\n",
                         smallbuf.Data);
                oh_append_bigtext(&mybuf, str);
                oh_append_offset(&mybuf, offsets);
                snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "ConfigData: %s\n",
                         ctrlrec->TypeUnion.Oem.ConfigData);
                oh_append_bigtext(&mybuf, str);
                oh_append_offset(&mybuf, offsets);
                snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "Default:\n");
                oh_append_bigtext(&mybuf, str);
                oh_append_offset(&mybuf, offsets + 1);
                err = oh_decode_manufacturerid(ctrlrec->TypeUnion.Oem.Default.MId, &smallbuf);
                snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "ManufacturerId: %s\n",
                         smallbuf.Data);
                oh_append_bigtext(&mybuf, str);
                oh_append_offset(&mybuf, offsets + 1);
                snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "BodyLength: %d\n",
                         ctrlrec->TypeUnion.Oem.Default.BodyLength);
                oh_append_bigtext(&mybuf, str);
                oh_append_offset(&mybuf, offsets + 1);
                snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "Body: %s\n",
                         ctrlrec->TypeUnion.Oem.Default.Body);
                oh_append_bigtext(&mybuf, str);
                break;
        default:
                snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "Invalid ControlType Detected\n");
                oh_append_bigtext(&mybuf, str);
        }

        oh_append_offset(&mybuf, offsets);
        snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "DefaultMode:\n");
        oh_append_bigtext(&mybuf, str);

        oh_append_offset(&mybuf, offsets + 1);
        snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "Mode: %s\n",
                 oh_lookup_ctrlmode(ctrlrec->DefaultMode.Mode));
        oh_append_bigtext(&mybuf, str);

        oh_append_offset(&mybuf, offsets + 1);
        snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "ReadOnly: %s\n",
                        (ctrlrec->DefaultMode.ReadOnly == SAHPI_TRUE) ? "TRUE" : "FALSE");
        oh_append_bigtext(&mybuf, str);

        oh_append_offset(&mybuf, offsets);
        snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "WriteOnly: %s\n",
                        (ctrlrec->WriteOnly == SAHPI_TRUE) ? "TRUE" : "FALSE");
        oh_append_bigtext(&mybuf, str);

        oh_append_offset(&mybuf, offsets);
        snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "OEM: %d\n", ctrlrec->Oem);
        oh_append_bigtext(&mybuf, str);

        err = oh_copy_bigtext(textbuf, &mybuf);

        return(err);
}

/**
 * oh_build_invrec:
 * @textbuff: Buffer into which to store flattened ctrl rdr structure.
 * @invrec: Pointer to SaHpiInventoryRecT to be flattened.
 * @offsets: Number of offsets to start printing structure.
 *
 * Flatten member data contained in SaHpiInventoryRecT struct to a text buffer.
 *
 * Returns:
 * SA_OK - Normal operation.
 * SA_ERR_HPI_INVALID_PARAMS - Pointer parameter(s) are NULL.
 **/
static SaErrorT oh_build_invrec(oh_big_textbuffer *textbuff,const SaHpiInventoryRecT *invrec, int offsets)
{
        SaErrorT err;
        char str[SAHPI_MAX_TEXT_BUFFER_LENGTH];
        oh_big_textbuffer mybuf;

        if (!textbuff || !invrec) {
                err("Invalid parameter.");
                return(SA_ERR_HPI_INVALID_PARAMS);
        }

        oh_init_bigtext(&mybuf);

        oh_append_offset(&mybuf, offsets);
        snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "IdrId: %d\n",invrec->IdrId);
        oh_append_bigtext(&mybuf, str);

        oh_append_offset(&mybuf, offsets);
        snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "Persistent: %s\n",
                 (invrec->Persistent == SAHPI_TRUE) ? "TRUE" : "FALSE" );
        oh_append_bigtext(&mybuf, str);

        oh_append_offset(&mybuf, offsets);
        snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "Oem: %d\n",invrec->Oem);
        oh_append_bigtext(&mybuf, str);

        err = oh_copy_bigtext(textbuff, &mybuf);
        return(err);
}

/**
 * oh_build_wdogrec:
 * @textbuff: Buffer into which to store flattened watchdog rec structure.
 * @wdogrec: Pointer to SaHpiWatchdogRecT to be flattened.
 * @offsets: Number of offsets to start printing structure.
 *
 * Flatten member data contained in SaHpiWatchdogRecT struct to a text buffer.
 *
 * Returns:
 * SA_OK - Normal operation.
 * SA_ERR_HPI_INVALID_PARAMS - Pointer parameter(s) are NULL.
 **/
static SaErrorT oh_build_wdogrec(oh_big_textbuffer *textbuff,const SaHpiWatchdogRecT *wdogrec, int offsets)
{

        SaErrorT err;
        char str[SAHPI_MAX_TEXT_BUFFER_LENGTH];
        oh_big_textbuffer mybuf;

        if (!textbuff || !wdogrec) {
                err("Invalid parameter.");
                return(SA_ERR_HPI_INVALID_PARAMS);
        }

        oh_init_bigtext(&mybuf);
        oh_append_offset(&mybuf, offsets);
        snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "Watchdog Num: %d (%x hex)\n",
                 wdogrec->WatchdogNum, wdogrec->WatchdogNum);
        oh_append_bigtext(&mybuf, str);

        oh_append_offset(&mybuf, offsets);
        snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "Oem: %d\n",wdogrec->Oem);
        oh_append_bigtext(&mybuf, str);

        err = oh_copy_bigtext(textbuff, &mybuf);
        return(err);
}

/**
 * oh_build_annrec:
 * @textbuff: Buffer into which to store flattened structure.
 * @annrec: Pointer to SaHpiAnnunciatorRecT to be flattened.
 * @offsets:  Number of offsets to start printing structure.
 *
 * Flatten member data contained in SaHpiAnnunciatorRecT struct to a text buffer.
 *
 * Returns:
 * SA_OK - Normal operation.
 * SA_ERR_HPI_INVALID_PARAMS - Pointer parameter(s) are NULL.
 **/
static SaErrorT oh_build_annrec(oh_big_textbuffer *textbuff,const SaHpiAnnunciatorRecT *annrec, int offsets)
{
        SaErrorT err;
        char str[SAHPI_MAX_TEXT_BUFFER_LENGTH];
        oh_big_textbuffer mybuf;

        if (!textbuff || !annrec) {
                err("Invalid parameter.");
                return(SA_ERR_HPI_INVALID_PARAMS);
        }

        oh_init_bigtext(&mybuf);
        oh_append_offset(&mybuf, offsets);
        snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "Annunciator Num: %d (%x hex)\n",
                 annrec->AnnunciatorNum, annrec->AnnunciatorNum);
        oh_append_bigtext(&mybuf, str);

        oh_append_offset(&mybuf, offsets);
        snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "Annunciator Type: %s\n",
                 oh_lookup_annunciatortype(annrec->AnnunciatorType));
        oh_append_bigtext(&mybuf, str);

        oh_append_offset(&mybuf, offsets);
        snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "ModeReadOnly: %s\n",
                 (annrec->ModeReadOnly == SAHPI_TRUE) ? "TRUE" : "FALSE" );
        oh_append_bigtext(&mybuf, str);

        oh_append_offset(&mybuf, offsets);
        snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "MaxCondition: %d\n", annrec->MaxConditions);
        oh_append_bigtext(&mybuf, str);

        oh_append_offset(&mybuf, offsets);
        snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "Oem: %d\n", annrec->Oem);
        oh_append_bigtext(&mybuf, str);

        err = oh_copy_bigtext(textbuff, &mybuf);
        return(err);
}

/**
 * oh_build_dimirec:
 * @textbuff: Buffer into which to store flattened structure.
 * @annrec: Pointer to SaHpiDimiRecT to be flattened.
 * @offsets:  Number of offsets to start printing structure.
 *
 * Flatten member data contained in SaHpiDimiRecT struct to a text buffer.
 *
 * Returns:
 * SA_OK - Normal operation.
 * SA_ERR_HPI_INVALID_PARAMS - Pointer parameter(s) are NULL.
 **/
static SaErrorT oh_build_dimirec(oh_big_textbuffer *textbuff, const SaHpiDimiRecT *dimirec, int offsets)
{
        SaErrorT err;
        char str[SAHPI_MAX_TEXT_BUFFER_LENGTH];
        oh_big_textbuffer mybuf;

        if (!textbuff || !dimirec) {
                dbg("Invalid parameter.");
                return(SA_ERR_HPI_INVALID_PARAMS);
        }

        oh_init_bigtext(&mybuf);
        oh_append_offset(&mybuf, offsets);
        snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "DIMI Num: %d (%x hex)\n",
                 dimirec->DimiNum, dimirec->DimiNum);
        oh_append_bigtext(&mybuf, str);

        oh_append_offset(&mybuf, offsets);
        snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "Oem: %d\n", dimirec->Oem);
        oh_append_bigtext(&mybuf, str);

        err = oh_copy_bigtext(textbuff, &mybuf);
        return(err);
}

/**
 * oh_decode_dimitestcapabilities:
 * @DimiTestCapabilities: enum value of type SaHpiDimiTestCapabilityT.
 * @buffer:  Location to store the string.
 *
 * Converts @DimiTestCapabilities type into a string based on HPI definition.
 *              
 * Returns:
 * SA_OK - Normal operation.
 * SA_ERR_HPI_INVALID_PARAMS - @buffer is NULL
 **/            
SaErrorT oh_decode_dimitestcapabilities(SaHpiDimiTestCapabilityT capabilities,
                                        SaHpiTextBufferT *buffer)
{
        int found;
        SaErrorT err; 
        SaHpiTextBufferT working;
        
        if (!buffer) {
                dbg("Invalid parameter.");
                return(SA_ERR_HPI_INVALID_PARAMS);
        }       
        
        err = oh_init_textbuffer(&working);
        if (err) { return(err); }
                
        found = 0; 
        if (capabilities & SAHPI_DIMITEST_CAPABILITY_RESULTSOUTPUT) {
                found++;
                err = oh_append_textbuffer(&working, "RESULTS_OUTPUT | ");
                if (err) { return(err); }
        }       
        if (capabilities & SAHPI_DIMITEST_CAPABILITY_SERVICEMODE) {
                found++;
                err = oh_append_textbuffer(&working, "SERVICE_MODE | ");
                if (err) { return(err); }
        }       
        if (capabilities & SAHPI_DIMITEST_CAPABILITY_LOOPCOUNT) {
                found++;
                err = oh_append_textbuffer(&working, "LOOP_COUNT | ");
                if (err) { return(err); }
        }       
        if (capabilities & SAHPI_DIMITEST_CAPABILITY_LOOPTIME) {
                found++;
                err = oh_append_textbuffer(&working, "LOOP_TIME | ");
                if (err) { return(err); }
        }       
        if (capabilities & SAHPI_DIMITEST_CAPABILITY_LOGGING) {
                found++;
                err = oh_append_textbuffer(&working, "LOGGING | ");
                if (err) { return(err); }
        }       
        if (capabilities & SAHPI_DIMITEST_CAPABILITY_TESTCANCEL) {
                found++;
                err = oh_append_textbuffer(&working, "TEST_CANCEL | ");
                if (err) { return(err); }
        }       

        if (found) {
                working.DataLength -= OH_ENCODE_DELIMITER_LENGTH;
                working.Data[working.DataLength] = 0;
        }
        else {
                oh_append_textbuffer(&working, "None");
        }

        oh_copy_textbuffer(buffer, &working);

        return(SA_OK);
}


/**
 * oh_decode_fumiprotocols:
 * @FumiProtocol: enum value of type SaHpiFumiProtocolT.
 * @buffer:  Location to store the string.
 *
 * Converts @FumiProtocol type into a string based on HPI definition.
 *              
 * Returns:
 * SA_OK - Normal operation.
 * SA_ERR_HPI_INVALID_PARAMS - @buffer is NULL
 **/            
SaErrorT oh_decode_fumiprotocols(SaHpiFumiProtocolT protocols,
                                  SaHpiTextBufferT *buffer)
{       
        int found;
        SaErrorT err; 
        SaHpiTextBufferT working;
        
        if (!buffer) {
                dbg("Invalid parameter.");
                return(SA_ERR_HPI_INVALID_PARAMS);
        }       
        
        err = oh_init_textbuffer(&working);
        if (err) { return(err); }
                
        found = 0; 
        if (protocols & SAHPI_FUMI_PROT_TFTP) {
                found++;
                err = oh_append_textbuffer(&working, "TFTP | ");
                if (err) { return(err); }
        }       
        if (protocols & SAHPI_FUMI_PROT_FTP) {
                found++;
                err = oh_append_textbuffer(&working, "FTP | ");
                if (err) { return(err); }
        }       
        if (protocols & SAHPI_FUMI_PROT_HTTP) {
                found++;
                err = oh_append_textbuffer(&working, "HTTP | ");
                if (err) { return(err); }
        }       
        if (protocols & SAHPI_FUMI_PROT_LDAP) {
                found++;
                err = oh_append_textbuffer(&working, "LDAP | ");
                if (err) { return(err); }
        }       
        if (protocols & SAHPI_FUMI_PROT_LOCAL) {
                found++;
                err = oh_append_textbuffer(&working, "LOCAL | ");
                if (err) { return(err); }
        }       
        if (protocols & SAHPI_FUMI_PROT_NFS) {
                found++;
                err = oh_append_textbuffer(&working, "NFS | ");
                if (err) { return(err); }
        }       
        if (protocols & SAHPI_FUMI_PROT_DBACCESS) {
                found++;
                err = oh_append_textbuffer(&working, "DBACCESS | ");
                if (err) { return(err); }
        }       
                
        if (found) {
                working.DataLength -= OH_ENCODE_DELIMITER_LENGTH;
                working.Data[working.DataLength] = 0;
        }
        else {
                oh_append_textbuffer(&working, "None");
        }

        oh_copy_textbuffer(buffer, &working);

        return(SA_OK);
}

/**
 * oh_decode_fumicapabilities:
 * @HsCapabilities: enum value of type SaHpiFumiCapabilityT.
 * @buffer:  Location to store the string.
 *
 * Converts @FumiCapability type into a string based on HPI definition.
 *              
 * Returns:
 * SA_OK - Normal operation.
 * SA_ERR_HPI_INVALID_PARAMS - @buffer is NULL
 **/            
SaErrorT oh_decode_fumicapabilities(SaHpiFumiCapabilityT capabilities,
                                  SaHpiTextBufferT *buffer)
{       
        int found;
        SaErrorT err; 
        SaHpiTextBufferT working;
        
        if (!buffer) {
                dbg("Invalid parameter.");
                return(SA_ERR_HPI_INVALID_PARAMS);
        }       
        
        err = oh_init_textbuffer(&working);
        if (err) { return(err); }
                
        found = 0; 
        if (capabilities & SAHPI_FUMI_CAP_ROLLBACK) {
                found++;
                err = oh_append_textbuffer(&working, "ROLLBACK | ");
                if (err) { return(err); }
        }       
        if (capabilities & SAHPI_FUMI_CAP_BANKCOPY) {
                found++;
                err = oh_append_textbuffer(&working, "BANKCOPY | ");
                if (err) { return(err); }
        }       
        if (capabilities & SAHPI_FUMI_CAP_BANKREORDER) {
                found++;
                err = oh_append_textbuffer(&working, "BANKREORDER | ");
                if (err) { return(err); }
        }       
        if (capabilities & SAHPI_FUMI_CAP_BACKUP) {
                found++;
                err = oh_append_textbuffer(&working, "BACKUP | ");
                if (err) { return(err); }
        }       
        if (capabilities & SAHPI_FUMI_CAP_TARGET_VERIFY) {
                found++;
                err = oh_append_textbuffer(&working, "TARGET_VERIFY | ");
                if (err) { return(err); }
        }       
                
        if (found) {
                working.DataLength -= OH_ENCODE_DELIMITER_LENGTH;
                working.Data[working.DataLength] = 0;
        }
        else {
                oh_append_textbuffer(&working, "None");
        }

        oh_copy_textbuffer(buffer, &working);

        return(SA_OK);
}

//==========================

/**
 * oh_build_fumirec:
 * @textbuff: Buffer into which to store flattened structure.
 * @annrec: Pointer to SaHpiFumiRecT to be flattened.
 * @offsets:  Number of offsets to start printing structure.
 *
 * Flatten member data contained in SaHpiFumiRecT struct to a text buffer.
 *
 * Returns:
 * SA_OK - Normal operation.
 * SA_ERR_HPI_INVALID_PARAMS - Pointer parameter(s) are NULL.
 **/
static SaErrorT oh_build_fumirec(oh_big_textbuffer *textbuff, const SaHpiFumiRecT *fumirec, int offsets)
{
        SaErrorT err;
        char str[SAHPI_MAX_TEXT_BUFFER_LENGTH];
        oh_big_textbuffer mybuf;
        SaHpiTextBufferT tmpbuffer;

        if (!textbuff || !fumirec) {
                dbg("Invalid parameter.");
                return(SA_ERR_HPI_INVALID_PARAMS);
        }

        oh_init_bigtext(&mybuf);
        oh_append_offset(&mybuf, offsets);
        snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "FUMI Num: %d (%x hex)\n",
                 fumirec->Num, fumirec->Num);
        oh_append_bigtext(&mybuf, str);

        oh_append_offset(&mybuf, offsets);
        snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "Supported protocols: ");
        oh_append_bigtext(&mybuf, str);
        oh_decode_fumiprotocols(fumirec->AccessProt, &tmpbuffer);
        oh_append_bigtext(&mybuf, (char *)tmpbuffer.Data);
        oh_append_bigtext(&mybuf, "\n");

        oh_append_offset(&mybuf, offsets);
        snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "Optional capabilities: ");
        oh_append_bigtext(&mybuf, str);
        oh_decode_fumicapabilities(fumirec->Capability, &tmpbuffer);
        oh_append_bigtext(&mybuf, (char *)tmpbuffer.Data);
        oh_append_bigtext(&mybuf, "\n");

        oh_append_offset(&mybuf, offsets);
        snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "Number of banks: %d\n", fumirec->NumBanks);
        oh_append_bigtext(&mybuf, str);

        oh_append_offset(&mybuf, offsets);
        snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "Oem: %d\n", fumirec->Oem);
        oh_append_bigtext(&mybuf, str);

        err = oh_copy_bigtext(textbuff, &mybuf);
        return(err);
}

/**
 * oh_fprint_eventloginfo:
 * @stream: File handle.
 * @thiselinfo: Pointer to SaHpiEventLogInfoT to be printed.
 * @offsets: Number of offsets to start printing structure.
 *
 * Prints the member data contained in SaHpiEventLogInfoT struct to a file.
 * The MACRO oh_print_evenloginfo(), uses this function to print to STDOUT.
 *
 * Returns:
 * SA_OK - Normal operation.
 * SA_ERR_HPI_INVALID_PARAMS - Pointer parameter(s) are NULL.
 **/
SaErrorT oh_fprint_eventloginfo(FILE *stream, const SaHpiEventLogInfoT *thiselinfo, int offsets)
{
        SaErrorT err;
        oh_big_textbuffer mybuf;
        char str[SAHPI_MAX_TEXT_BUFFER_LENGTH];
        SaHpiTextBufferT  minibuf;

        if (!stream || !thiselinfo) {
                err("Invalid parameter.");
                return(SA_ERR_HPI_INVALID_PARAMS);
        }

        oh_init_bigtext(&mybuf);

        oh_append_offset(&mybuf, offsets);
        snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "Entries: %d\n", thiselinfo->Entries);
        oh_append_bigtext(&mybuf, str);

        oh_append_offset(&mybuf, offsets);
        snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "Size: %d\n", thiselinfo->Size);
        oh_append_bigtext(&mybuf, str);

        oh_append_offset(&mybuf, offsets);
        snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "UserEventMaxSize: %d\n", thiselinfo->UserEventMaxSize);
        oh_append_bigtext(&mybuf, str);

        oh_append_offset(&mybuf, offsets);
        err = oh_decode_time(thiselinfo->UpdateTimestamp, &minibuf);
        if (err) memcpy(minibuf.Data, "SAHPI_TIME_UNSPECIFIED",
                        sizeof("SAHPI_TIME_UNSPECIFIED"));
        snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "UpdateTimestamp: %s\n", minibuf.Data);
        oh_append_bigtext(&mybuf, str);

        oh_append_offset(&mybuf, offsets);
        err = oh_decode_time(thiselinfo->CurrentTime, &minibuf);
        if (err) memcpy(minibuf.Data, "SAHPI_TIME_UNSPECIFIED",
                        sizeof("SAHPI_TIME_UNSPECIFIED"));
        snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "CurrentTime: %s\n", minibuf.Data);
        oh_append_bigtext(&mybuf, str);

        oh_append_offset(&mybuf, offsets);
        snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "Enabled: %s\n",
                 (thiselinfo->Enabled == SAHPI_TRUE) ? "TRUE" : "FALSE" );
        oh_append_bigtext(&mybuf, str);

        oh_append_offset(&mybuf, offsets);
        snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "OverflowFlag: %s\n",
                 (thiselinfo->OverflowFlag == SAHPI_TRUE) ? "TRUE" : "FALSE" );
        oh_append_bigtext(&mybuf, str);

        oh_append_offset(&mybuf, offsets);
        snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "OverflowResetable: %s\n",
                 (thiselinfo->OverflowResetable == SAHPI_TRUE) ? "TRUE" : "FALSE" );
        oh_append_bigtext(&mybuf, str);

        oh_append_offset(&mybuf, offsets);
        snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "OverflowAction: %s\n",
                 oh_lookup_eventlogoverflowaction(thiselinfo->OverflowAction));
        oh_append_bigtext(&mybuf, str);

        err = oh_fprint_bigtext(stream, &mybuf);
        return(err);
}

/**
 * oh_fprint_eventlogentry:
 * @stream: File handle.
 * @thiseventlog: Pointer to SaHpiEventLogEntryT to be printed.
 * @entitypath: Pointer to entity path.
 * @offsets: Number of offsets to start printing structure.
 *
 * Prints the member data contained in SaHpiEventLogEntryT struct to a file.
 * The MACRO oh_print_evenlogentry(), uses this function to print to STDOUT.
 *
 * Returns:
 * SA_OK - Normal operation.
 * SA_ERR_HPI_INVALID_PARAMS - Pointer parameter(s) are NULL.
 **/
SaErrorT oh_fprint_eventlogentry(FILE *stream,
                                 const SaHpiEventLogEntryT *thiseventlog,
                                 const SaHpiEntityPathT *entitypath,
                                 int offsets)
{
        SaErrorT err;
        oh_big_textbuffer mybuf, mybufX;
        char str[SAHPI_MAX_TEXT_BUFFER_LENGTH];
        SaHpiTextBufferT  minibuf;

        if (!stream || !thiseventlog) {
                err("Invalid parameter.");
                return(SA_ERR_HPI_INVALID_PARAMS);
        }

        oh_init_bigtext(&mybuf);

        oh_append_offset(&mybuf, offsets);
        snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "EntryId: %d\n", thiseventlog->EntryId);
        oh_append_bigtext(&mybuf, str);
        oh_append_offset(&mybuf, offsets);
        err = oh_decode_time(thiseventlog->Timestamp, &minibuf);
        if (err)
                memcpy(minibuf.Data, "SAHPI_TIME_UNSPECIFIED",
                                        sizeof("SAHPI_TIME_UNSPECIFIED"));
        snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "Timestamp: %s\n", minibuf.Data);
        oh_append_bigtext(&mybuf, str);

        oh_init_bigtext(&mybufX);
        err = oh_build_event(&mybufX, &thiseventlog->Event, entitypath, offsets);
        oh_append_bigtext(&mybuf, (char *)mybufX.Data);

        err = oh_fprint_bigtext(stream, &mybuf);
        return(err);
}

/**
 * oh_fprint_event:
 * @stream: File handle.
 * @event: Pointer to SaHpiEventT to be printed.
 * @entitypath: Pointer to entitypath.
 * @offsets: Number of offsets to start printing structure.
 *
 * Prints the member data contained in SaHpiEventT struct to a file.
 * The MACRO oh_print_event(), uses this function to print to STDOUT.
 *
 * Returns:
 * SA_OK - Normal operation.
 * SA_ERR_HPI_INVALID_PARAMS - Pointer parameter(s) are NULL.
 **/
SaErrorT oh_fprint_event(FILE *stream,
                         const SaHpiEventT *event,
                         const SaHpiEntityPathT *entitypath,
                         int offsets)
{
        SaErrorT err;
        oh_big_textbuffer buffer;

        if (!stream || !event) {
                err("Invalid parameter.");
                return(SA_ERR_HPI_INVALID_PARAMS);
        }

        oh_init_bigtext(&buffer);

        err = oh_build_event(&buffer, event, entitypath, offsets);
        if (err) { return(err); }

        err = oh_fprint_bigtext(stream, &buffer);
        if (err) { return(err); }

        return(SA_OK);
}

/**
 * oh_build_event:
 * @buffer: Pointer to space to decipher SaHpiEventT struct
 * @event: Pointer to the event to be deciphered
 * @offset: Number of offsets to start printing structure.
 *
 * Builds SaHpiEventT data into a string buffer.
 *
 * Returns:
 * SA_OK - Normal operation.
 * SA_ERR_HPI_INVALID_PARAMS - Pointer parameter(s) are NULL.
 **/
SaErrorT oh_build_event(oh_big_textbuffer *buffer,
                        const SaHpiEventT *event,
                        const SaHpiEntityPathT *entitypath,
                        int offsets)
{
        char str[SAHPI_MAX_TEXT_BUFFER_LENGTH];
        guint id;
        SaErrorT err = SA_OK;
        SaHpiEntityPathT ep;
        SaHpiTextBufferT tmpbuffer;
        oh_big_textbuffer bigbuf;

        memset( buffer->Data, 0, OH_MAX_TEXT_BUFFER_LENGTH );
        memset( tmpbuffer.Data, 0, sizeof( tmpbuffer.Data ) );

        id = event->Source;

        if (entitypath) {
                ep = *entitypath;
                err  = oh_decode_entitypath(&ep, &bigbuf);
        } else {
                err = oh_entity_path_lookup(id, &ep);
                if (err) {
                        err("Could not determine entity path.");
                } else {
                        /* Only if we were able to get the entity path */
                        err  = oh_decode_entitypath(&ep, &bigbuf);
                }
        }

        /* Event Type */
        oh_append_offset(buffer, offsets);
        snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "Event Type: %s\n",
                 oh_lookup_eventtype(event->EventType));
        oh_append_bigtext(buffer, str);

        /* Entity Path */
        if (err == SA_OK) {
                /* Skip this if we failed to get/decode entity path earlier */
                oh_append_offset(buffer, offsets);
                snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH,
                         "From Resource: %s\n", bigbuf.Data);
                oh_append_bigtext(buffer, str);
        }
        offsets++;

        /* Event Source */
        oh_append_offset(buffer, offsets);
        snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "Event Resource ID: %d\n", event->Source);
        oh_append_bigtext(buffer, str);

        /* Event Time */
        oh_append_offset(buffer, offsets);
        oh_decode_time(event->Timestamp, &tmpbuffer);
        snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "Event Timestamp: %s\n", tmpbuffer.Data);
        oh_append_bigtext(buffer, str);

        /* Event Severity */
        oh_append_offset(buffer, offsets);
        snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "Event Severity: %s\n",
                 oh_lookup_severity(event->Severity));
        oh_append_bigtext(buffer, str);

        switch (event->EventType) {
        case SAHPI_ET_RESOURCE:
                err = oh_build_event_resource(buffer, event, offsets);
                break;
        case SAHPI_ET_DOMAIN:
                err = oh_build_event_domain(buffer, event, offsets);
                break;
        case SAHPI_ET_SENSOR:
                err = oh_build_event_sensor(buffer, event, offsets);
                break;
        case SAHPI_ET_SENSOR_ENABLE_CHANGE:
                err = oh_build_event_sensor_enable_change(buffer, event, offsets);
                break;
        case SAHPI_ET_HOTSWAP:
                err = oh_build_event_hotswap(buffer, event, offsets);
                break;
        case SAHPI_ET_WATCHDOG:
                err = oh_build_event_watchdog(buffer, event, offsets);
                break;
        case SAHPI_ET_HPI_SW:
                err = oh_build_event_hpi_sw(buffer, event, offsets);
                break;
        case SAHPI_ET_OEM:
                err = oh_build_event_oem(buffer, event, offsets);
                break;
        case SAHPI_ET_USER:
                err = oh_build_event_user(buffer, event, offsets);
                break;
        case SAHPI_ET_DIMI:
                err = oh_build_event_dimi(buffer, event, offsets);
                break;
        case SAHPI_ET_DIMI_UPDATE:
                err = oh_build_event_dimi_update(buffer, event, offsets);
                break;
        case SAHPI_ET_FUMI:
                err = oh_build_event_fumi(buffer, event, offsets);
                break;
        default:
                err("Unrecognized Event Type=%d.", event->EventType);
                return(SA_ERR_HPI_INVALID_PARAMS);
        }

        if (err) { return(err); }
        return(SA_OK);
}

/**
 * oh_build_event_resource:
 * @buffer: Pointer to space to decipher SaHpiEventT struct
 * @event: Pointer to the event to be deciphered
 * @offset: Number of offsets to start printing structure.
 *
 * Builds ResourceEventTypeT data into string buffer.
 *
 * Returns:
 * SA_OK - Normal operation.
 * SA_ERR_HPI_INVALID_PARAMS - Pointer parameter(s) are NULL.
 **/
static SaErrorT oh_build_event_resource(oh_big_textbuffer *buffer, const SaHpiEventT *event, int offsets)
{

        char str[SAHPI_MAX_TEXT_BUFFER_LENGTH];

        if (!buffer || !event) {
                err("Invalid parameter.");
                return(SA_ERR_HPI_INVALID_PARAMS);
        }

        oh_append_offset(buffer, offsets);
        snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "ResourceEventType: %s\n",
                        oh_lookup_resourceeventtype(event->EventDataUnion.ResourceEvent.ResourceEventType));
        oh_append_bigtext(buffer, str);

        return(SA_OK);
}

/**
 * oh_build_event_domain:
 * @buffer: Pointer to space to decipher SaHpiEventT struct
 * @event: Pointer to the event to be deciphered
 * @offset: Number of offsets to start printing structure.
 *
 * Build event domain value structure values into string buffer.
 *
 * Returns:
 * SA_OK - Normal operation.
 * SA_ERR_HPI_INVALID_PARAMS - Pointer parameter(s) are NULL.
 **/
static SaErrorT oh_build_event_domain(oh_big_textbuffer *buffer, const SaHpiEventT *event, int offsets)
{
        char str[SAHPI_MAX_TEXT_BUFFER_LENGTH];

        if (!buffer || !event) {
                err("Invalid parameter.");
                return(SA_ERR_HPI_INVALID_PARAMS);
        }

        oh_append_offset(buffer, offsets);
        snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "DomainEvent:\n");
        oh_append_bigtext(buffer, str);

        oh_append_offset(buffer, offsets + 1);
        snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "Type: %s\n",
                 oh_lookup_domaineventtype(event->EventDataUnion.DomainEvent.Type));
        oh_append_bigtext(buffer, str);

        oh_append_offset(buffer, offsets + 1);
        snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "DomainId: %d\n",
                 event->EventDataUnion.DomainEvent.DomainId);
        oh_append_bigtext(buffer, str);

        return(SA_OK);
}

#define OH_MAX_SENSOROPTIONALDATA 6

typedef struct {
    SaHpiSensorOptionalDataT datadef;
    char *str;
} oh_sensor_opt_data_map;

oh_sensor_opt_data_map sensor_event_optdata_strings[] = {
{SAHPI_SOD_TRIGGER_READING, "TRIGGER_READING"},
{SAHPI_SOD_TRIGGER_THRESHOLD, "TRIGGER_THRESHOLD"},
{SAHPI_SOD_OEM, "OEM"},
{SAHPI_SOD_PREVIOUS_STATE, "PREVIOUS_STATE"},
{SAHPI_SOD_CURRENT_STATE, "CURRENT_STATE"},
{SAHPI_SOD_SENSOR_SPECIFIC, "SENSOR_SPECIFIC"},
};

/**
 * oh_encode_sensoroptionaldata:
 * @buffer: Pointer to space to decipher SaHpiSensorOptionalDataT struct
 * @sensor_opt_data: Sensor's optional data bit mask.
 *
 * Converts a sensor's optional data bit mask field into a string. For example,
 * if the optional data bit mask is SAHPI_SOD_TRIGGER_READING | SAHPI_SOD_CURRENT_STATE,
 * this routine returns the string "TRIGGER_READING | CURRENT_STATE" in a
 * SaHpiTextBufferT structure.
 *
 * Returns:
 * SA_OK - Normal operation.
 * SA_ERR_HPI_INVALID_PARAMS - Pointer parameter(s) are NULL.
 **/
SaErrorT oh_decode_sensoroptionaldata(SaHpiSensorOptionalDataT sensor_opt_data,
                                      SaHpiTextBufferT *buffer)
{
        int i, found;
        SaErrorT err;
        SaHpiTextBufferT working;

        if (!buffer) {
                err("Invalid parameter.");
                return(SA_ERR_HPI_INVALID_PARAMS);
        }

        err = oh_init_textbuffer(&working);
        if (err) { return(err); }

        found = 0;
        /* Look for sensor optional data definitions */
        for (i=0; i<OH_MAX_SENSOROPTIONALDATA; i++) {
                if (sensor_event_optdata_strings[i].datadef & sensor_opt_data) {
                        found++;
                        err = oh_append_textbuffer(&working, sensor_event_optdata_strings[i].str);
                        if (err) { return(err); }
                        err = oh_append_textbuffer(&working, OH_ENCODE_DELIMITER);
                        if (err) { return(err); }
                }
        }

        /* Remove last delimiter */
        if (found) {
                working.DataLength -= OH_ENCODE_DELIMITER_LENGTH;
                working.Data[working.DataLength] = 0;
        }
        else {
                err = oh_append_textbuffer(&working, "None");
                if (err) { return(err); }
        }

        err = oh_copy_textbuffer(buffer, &working);

        return(SA_OK);
}

#define OH_MAX_SENSORENABLEOPTDATA 1

typedef struct {
    SaHpiSensorEnableOptDataT datadef;
    char *str;
} oh_sensor_enable_opt_data_map;

oh_sensor_enable_opt_data_map sensor_enable_optdata_strings[] = {
{SAHPI_SEOD_CURRENT_STATE, "CURRENT_STATE"},
};

/**
 * oh_encode_sensorenableoptdata:
 * @buffer: Pointer to space to decipher SaHpiSensorEnableOptDataT struct
 * @sensor_opt_data: Sensor Enable Event's optional data bit mask.
 *
 * Converts a sensor's enable event's optional data bit mask field into a string. For example,
 * if the optional data bit mask is SAHPI_SEOD_CURRENT_STATE
 * this routine returns the string "CURRENT_STATE" in a
 * SaHpiTextBufferT structure.
 *
 * Returns:
 * SA_OK - Normal operation.
 * SA_ERR_HPI_INVALID_PARAMS - Pointer parameter(s) are NULL.
 **/
SaErrorT oh_decode_sensorenableoptdata(SaHpiSensorEnableOptDataT sensor_enable_opt_data,
                                       SaHpiTextBufferT *buffer)
{
        int i, found;
        SaErrorT err;
        SaHpiTextBufferT working;

        if (!buffer) {
                err("Invalid parameter.");
                return(SA_ERR_HPI_INVALID_PARAMS);
        }

        err = oh_init_textbuffer(&working);
        if (err) { return(err); }

        found = 0;
        /* Look for sensor optional data definitions */
        for (i=0; i<OH_MAX_SENSORENABLEOPTDATA; i++) {
                if (sensor_enable_optdata_strings[i].datadef & sensor_enable_opt_data) {
                        found++;
                        err = oh_append_textbuffer(&working, sensor_enable_optdata_strings[i].str);
                        if (err) { return(err); }
                        err = oh_append_textbuffer(&working, OH_ENCODE_DELIMITER);
                        if (err) { return(err); }
                }
        }

        /* Remove last delimiter */
        if (found) {
                working.DataLength -= OH_ENCODE_DELIMITER_LENGTH;
                working.Data[working.DataLength] = 0;
        }
        else {
                err = oh_append_textbuffer(&working, "None");
                if (err) { return(err); }
        }

        err = oh_copy_textbuffer(buffer, &working);

        return(SA_OK);
}

/**
 * oh_build_event_resource:
 * @buffer: Pointer to space to decipher SaHpiEventT struct
 * @event: Pointer to the event to be deciphered
 * @offset: Number of offsets to start printing structure.
 *
 * Converts a sensor event structure into a string.
 * String is stored in an oh_big_textbuffer data structure.
 *
 * Returns:
 * SA_OK - Normal operation.
 * SA_ERR_HPI_INVALID_PARAMS - Pointer parameter(s) are NULL.
 **/
static SaErrorT oh_build_event_sensor(oh_big_textbuffer *buffer, const SaHpiEventT *event, int offsets)
{
        char str[SAHPI_MAX_TEXT_BUFFER_LENGTH];
        SaErrorT err;
        SaHpiTextBufferT tmpbuffer;

        if ( !buffer || !event) {
                err("Invalid parameter.");
                return(SA_ERR_HPI_INVALID_PARAMS);
        }

        /* Event Sensor Num */
        oh_append_offset(buffer, offsets);
        snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "Event Sensor Num: %d (%x hex)\n",
                 event->EventDataUnion.SensorEvent.SensorNum, event->EventDataUnion.SensorEvent.SensorNum);
        oh_append_bigtext(buffer, str);

        /* Event Sensor Type */
        oh_append_offset(buffer, offsets);
        snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "Event Sensor Type: %s\n",
                 oh_lookup_sensortype(event->EventDataUnion.SensorEvent.SensorType));
        oh_append_bigtext(buffer, str);

        /* Event Sensor Category */
        oh_append_offset(buffer, offsets);
        snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "Event Sensor Category: %s\n",
                 oh_lookup_eventcategory(event->EventDataUnion.SensorEvent.EventCategory));
        oh_append_bigtext(buffer, str);

        /* Event Sensor Assertion */
        oh_append_offset(buffer, offsets);
        snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "Event Sensor Assertion: %s\n",
                (event->EventDataUnion.SensorEvent.Assertion == SAHPI_TRUE) ? "TRUE" : "FALSE");
        oh_append_bigtext(buffer, str);

        /* Event Sensor State */
        oh_append_offset(buffer, offsets);
        oh_append_bigtext(buffer, "Event Sensor State: ");
        err = oh_decode_eventstate(event->EventDataUnion.SensorEvent.EventState,
                                   event->EventDataUnion.SensorEvent.EventCategory,
                                   &tmpbuffer);
        if (err) { return(err); }
        oh_append_bigtext(buffer, (char *)tmpbuffer.Data);
        oh_append_bigtext(buffer, "\n");

        /* Sensor Event - Sensor Optional Data */
        oh_append_offset(buffer, offsets);
        oh_append_bigtext(buffer, "Optional Data: ");

        err = oh_decode_sensoroptionaldata(event->EventDataUnion.SensorEvent.OptionalDataPresent,
                                           &tmpbuffer);
        if (err) { return(err); }
        oh_append_bigtext(buffer, (char *)tmpbuffer.Data);
        oh_append_bigtext(buffer, "\n");

        /* Sensor Event - Sensor Optional Trigger Reading Data */
        if (event->EventDataUnion.SensorEvent.OptionalDataPresent & SAHPI_SOD_TRIGGER_READING) {
                SaHpiSensorDataFormatT format;

                memset(&format, 0, sizeof(SaHpiSensorDataFormatT));
                oh_append_offset(buffer, offsets + 1);
                oh_append_bigtext(buffer, "Trigger Reading: ");

                format.IsSupported = SAHPI_TRUE;
                format.ReadingType = event->EventDataUnion.SensorEvent.TriggerReading.Type;
                format.BaseUnits = SAHPI_SU_UNSPECIFIED;

                err = oh_decode_sensorreading(event->EventDataUnion.SensorEvent.TriggerReading,
                                              format, &tmpbuffer);
                if (err) { return(err); }

                oh_append_bigtext(buffer, (char *)tmpbuffer.Data);
                oh_append_bigtext(buffer, "\n");
        }

        /* Sensor Event - Sensor Optional Trigger Threshold Data */
        if (event->EventDataUnion.SensorEvent.OptionalDataPresent & SAHPI_SOD_TRIGGER_THRESHOLD) {
                SaHpiSensorDataFormatT format;

                memset(&format, 0, sizeof(SaHpiSensorDataFormatT));
                oh_append_offset(buffer, offsets + 1);
                oh_append_bigtext(buffer, "Trigger Threshold: ");

                format.IsSupported = SAHPI_TRUE;
                format.ReadingType = event->EventDataUnion.SensorEvent.TriggerThreshold.Type;
                format.BaseUnits = SAHPI_SU_UNSPECIFIED;

                err = oh_decode_sensorreading(event->EventDataUnion.SensorEvent.TriggerThreshold,
                                              format, &tmpbuffer);
                if (err) { return(err); }

                oh_append_bigtext(buffer, (char *)tmpbuffer.Data);
                oh_append_bigtext(buffer, "\n");
        }

        /* Sensor Event - Sensor Optional Previous State Data */
        if (event->EventDataUnion.SensorEvent.OptionalDataPresent & SAHPI_SOD_PREVIOUS_STATE) {
                oh_append_offset(buffer, offsets + 1);
                oh_append_bigtext(buffer, "Previous Sensor State: ");
                err = oh_decode_eventstate(event->EventDataUnion.SensorEvent.PreviousState,
                                           event->EventDataUnion.SensorEvent.EventCategory,
                                           &tmpbuffer);
                if (err) { return(err); }
                oh_append_bigtext(buffer, (char *)tmpbuffer.Data);
                oh_append_bigtext(buffer, "\n");
        }

        /* Sensor Event - Sensor Optional Current State Data */
        if (event->EventDataUnion.SensorEvent.OptionalDataPresent & SAHPI_SOD_CURRENT_STATE) {
                oh_append_offset(buffer, offsets + 1);
                oh_append_bigtext(buffer, "Current Sensor State: ");
                err = oh_decode_eventstate(event->EventDataUnion.SensorEvent.CurrentState,
                                           event->EventDataUnion.SensorEvent.EventCategory,
                                           &tmpbuffer);
                if (err) { return(err); }
                oh_append_bigtext(buffer, (char *)tmpbuffer.Data);
                oh_append_bigtext(buffer, "\n");
        }
        /* Sensor Event - Sensor Optional OEM Data */
        if (event->EventDataUnion.SensorEvent.OptionalDataPresent & SAHPI_SOD_OEM) {
                oh_append_offset(buffer, offsets + 1);
                snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "OEM: %x\n",
                         event->EventDataUnion.SensorEvent.Oem);
                oh_append_bigtext(buffer, str);
        }

        /* Sensor Event - Sensor Optional Sensor Specific Data */
        if (event->EventDataUnion.SensorEvent.OptionalDataPresent & SAHPI_SOD_SENSOR_SPECIFIC) {
                oh_append_offset(buffer, offsets + 1);
                snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "Sensor Specific: %x\n",
                         event->EventDataUnion.SensorEvent.SensorSpecific );
                oh_append_bigtext(buffer, str);
        }

        return(SA_OK);
}

/**
 * oh_build_event_sensor_enable_change:
 * @buffer: Pointer to space to decipher SaHpiEventT struct
 * @event: Pointer to the event to be deciphered
 * @offset: Number of offsets to start printing structure.
 *
 * Build SaHpiSensorEnableChangeEventT data into a string buffer.
 *
 * Returns:
 * SA_OK - Normal operation.
 * SA_ERR_HPI_INVALID_PARAMS - Pointer parameter(s) are NULL.
 **/
static SaErrorT oh_build_event_sensor_enable_change(oh_big_textbuffer *buffer, const SaHpiEventT *event, int offsets)
{
        char str[SAHPI_MAX_TEXT_BUFFER_LENGTH];
        SaErrorT err;
        SaHpiTextBufferT tmpbuffer;

        if ( !buffer || !event) {
                err("Invalid parameter.");
                return(SA_ERR_HPI_INVALID_PARAMS);
        }

        oh_append_offset(buffer, offsets);
        snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "SensorEnableChangeEvent: \n");
        oh_append_bigtext(buffer, str);

        /* Sensor Enable Change Event - Sensor Number */
        oh_append_offset(buffer, offsets + 1);
        snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "Sensor Num: %d (%x hex)\n",
                 event->EventDataUnion.SensorEnableChangeEvent.SensorNum,
                 event->EventDataUnion.SensorEnableChangeEvent.SensorNum);
        oh_append_bigtext(buffer, str);

        /* Sensor Enable Change Event - Sensor Type */
        oh_append_offset(buffer, offsets + 1);
        snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "Sensor Type: %s\n",
                        oh_lookup_sensortype(event->EventDataUnion.SensorEnableChangeEvent.SensorType));
        oh_append_bigtext(buffer, str);

        /* Sensor Enable Change Event - Sensor Category */
        oh_append_offset(buffer, offsets + 1);
        snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "EventCategory: %s\n",
                        oh_lookup_eventcategory(event->EventDataUnion.SensorEnableChangeEvent.EventCategory));
        oh_append_bigtext(buffer, str);

        /* Sensor Enable Change Event - Sensor Enabled */
        oh_append_offset(buffer, offsets + 1);
        snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "SensorEnable: %s\n",
                (event->EventDataUnion.SensorEnableChangeEvent.SensorEnable == SAHPI_TRUE) ? "TRUE" : "FALSE");
        oh_append_bigtext(buffer, str);

        /* Sensor Enable Change Event - Sensor Events Enabled */
        oh_append_offset(buffer, offsets + 1);
        snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "SensorEventEnable: %s\n",
                (event->EventDataUnion.SensorEnableChangeEvent.SensorEventEnable == SAHPI_TRUE) ? "TRUE" : "FALSE");
        oh_append_bigtext(buffer, str);

        /* Sensor Enable Change Event - Event Assert Mask */
        oh_append_offset(buffer, offsets + 1);
        oh_append_bigtext(buffer, "AssertEventMask: ");
        err = oh_decode_eventstate(event->EventDataUnion.SensorEnableChangeEvent.AssertEventMask,
                                   event->EventDataUnion.SensorEnableChangeEvent.EventCategory,
                                   &tmpbuffer);
        if (err) { return(err); }
        oh_append_bigtext(buffer, (char *)tmpbuffer.Data);
        oh_append_bigtext(buffer, "\n");

        /* Sensor Enable Change Event - Event Deassert Mask */
        oh_append_offset(buffer, offsets + 1);
        oh_append_bigtext(buffer, "DeassertEventMask: ");
        err = oh_decode_eventstate(event->EventDataUnion.SensorEnableChangeEvent.DeassertEventMask,
                                   event->EventDataUnion.SensorEnableChangeEvent.EventCategory,
                                   &tmpbuffer);
        if (err) { return(err); }
        oh_append_bigtext(buffer, (char *)tmpbuffer.Data);
        oh_append_bigtext(buffer, "\n");

        /* Sensor Enable Change Event - Optional Data */
        oh_append_offset(buffer, offsets + 1);
        oh_append_bigtext(buffer, "Optional Data: ");
        err = oh_decode_sensorenableoptdata(event->EventDataUnion.SensorEnableChangeEvent.OptionalDataPresent,
                                            &tmpbuffer);
        if (err) { return(err); }
        oh_append_bigtext(buffer, (char *)tmpbuffer.Data);
        oh_append_bigtext(buffer, "\n");

        /* Sensor Enable Change Event - Optional Data - Current State */
        if (event->EventDataUnion.SensorEnableChangeEvent.OptionalDataPresent & SAHPI_SEOD_CURRENT_STATE) {
                oh_append_offset(buffer, offsets + 2);
                oh_append_bigtext(buffer, "Current State: ");
                err = oh_decode_eventstate(event->EventDataUnion.SensorEnableChangeEvent.CurrentState,
                                           event->EventDataUnion.SensorEnableChangeEvent.EventCategory,
                                           &tmpbuffer);
                if (err) { return(err); }
                oh_append_bigtext(buffer, (char *)tmpbuffer.Data);
                oh_append_bigtext(buffer, "\n");
        }

        return(SA_OK);
}

/**
 * oh_build_event_hotswap:
 * @buffer: Pointer to space to decipher SaHpiEventT struct
 * @event: Pointer to the event to be deciphered
 * @offset: Number of offsets to start printing structure.
 *
 * Builds SaHpiHotSwapEventT data into a string buffer.
 *
 * Returns:
 * SA_OK - Normal operation.
 * SA_ERR_HPI_INVALID_PARAMS - Pointer parameter(s) are NULL.
 **/
static SaErrorT oh_build_event_hotswap(oh_big_textbuffer *buffer, const SaHpiEventT *event, int offsets)
{
        char str[SAHPI_MAX_TEXT_BUFFER_LENGTH];

        if ( !buffer || !event) {
                err("Invalid parameter.");
                return(SA_ERR_HPI_INVALID_PARAMS);
        }

        oh_append_offset(buffer, offsets);
        snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "HotswapEvent: \n");
        oh_append_bigtext(buffer, str);

        oh_append_offset(buffer, offsets + 1);
        snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "HotSwapState: %s\n",
                        oh_lookup_hsstate(event->EventDataUnion.HotSwapEvent.HotSwapState));
        oh_append_bigtext(buffer, str);

        oh_append_offset(buffer, offsets + 1);
        snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "PreviousHotSwapState: %s\n",
                        oh_lookup_hsstate(event->EventDataUnion.HotSwapEvent.PreviousHotSwapState));
        oh_append_bigtext(buffer, str);

        return(SA_OK);
}

/**
 * oh_build_event_watchdog:
 * @buffer: Pointer to space to decipher SaHpiEventT struct
 * @event: Pointer to the event to be deciphered
 * @offset: Number of offsets to start printing structure.
 *
 * Builds SaHpiWatchdogEventT data into a string buffer.
 *
 * Returns:
 * SA_OK - Normal operation.
 * SA_ERR_HPI_INVALID_PARAMS - Pointer parameter(s) are NULL.
 **/
static SaErrorT oh_build_event_watchdog(oh_big_textbuffer *buffer, const SaHpiEventT *event, int offsets)
{

        char str[SAHPI_MAX_TEXT_BUFFER_LENGTH];
        int i;
        SaHpiBoolT matchFound = SAHPI_FALSE;


        if ( !buffer || !event) {
                err("Invalid parameter.");
                return(SA_ERR_HPI_INVALID_PARAMS);
        }

        oh_append_offset(buffer, offsets);
        snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "Watchdog Event: \n");
        oh_append_bigtext(buffer, str);

        oh_append_offset(buffer, offsets + 1);
        snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "Watchdog Num: %d (%x hex)\n",
                 event->EventDataUnion.WatchdogEvent.WatchdogNum,
                 event->EventDataUnion.WatchdogEvent.WatchdogNum);
        oh_append_bigtext(buffer, str);

        oh_append_offset(buffer, offsets + 1);
        snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "WatchdogActionEvent: %s\n",
                        oh_lookup_watchdogactionevent(event->EventDataUnion.WatchdogEvent.WatchdogAction));
        oh_append_bigtext(buffer, str);

        oh_append_offset(buffer, offsets + 1);
        snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "WatchdogPreTimerAction: %s\n",
                        oh_lookup_watchdogpretimerinterrupt(event->EventDataUnion.WatchdogEvent.WatchdogPreTimerAction));
        oh_append_bigtext(buffer, str);

        oh_append_offset(buffer, offsets + 1);
        for (i = 0; i < OH_MAX_WATCHDOGTIMERUSE; i++) {
                if (event->EventDataUnion.WatchdogEvent.WatchdogUse ==
                    watchdogtimeruse_strings[i].entity_type) {
                        matchFound = SAHPI_TRUE;
                        snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "WatchdogUse: %s\n",
                                 watchdogtimeruse_strings[i].str);
                        break;
                }
        }

        if (!matchFound)
                snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "WatchdogUse: %s\n",
                          "Invalid data");
        oh_append_bigtext(buffer, str);

        return(SA_OK);
}

/**
 * oh_build_event_hpi_sw:
 * @buffer: Pointer to space to decipher SaHpiEventT struct
 * @event: Pointer to the event to be deciphered
 * @offset: Number of offsets to start printing structure.
 *
 * Builds SaHpiHpiSwEventT data into a string buffer.
 *
 * Returns:
 * SA_OK - Normal operation.
 * SA_ERR_HPI_INVALID_PARAMS - Pointer parameter(s) are NULL.
 **/
static SaErrorT oh_build_event_hpi_sw(oh_big_textbuffer *buffer, const SaHpiEventT *event, int offsets)
{
        char str[SAHPI_MAX_TEXT_BUFFER_LENGTH];
        SaHpiTextBufferT tmpbuffer;
        SaErrorT err;

        if ( !buffer || !event) {
                err("Invalid parameter.");
                return(SA_ERR_HPI_INVALID_PARAMS);
        }

        oh_append_offset(buffer, offsets);
        snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "HpiSwEvent: \n");
        oh_append_bigtext(buffer, str);
        offsets++;

        oh_append_offset(buffer, offsets);
        err = oh_decode_manufacturerid(event->EventDataUnion.HpiSwEvent.MId, &tmpbuffer);
        snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "ManufacturerId: %s\n", tmpbuffer.Data);
        oh_append_bigtext(buffer, str);

        oh_append_offset(buffer, offsets);
        snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "EventData: \n");
        oh_append_bigtext(buffer, str);
        offsets++;

        oh_build_textbuffer(buffer, &event->EventDataUnion.HpiSwEvent.EventData, offsets);
        return(SA_OK);
}

/**
 * oh_build_event_oem:
 * @buffer: Pointer to space to decipher SaHpiEventT struct
 * @event: Pointer to the event to be deciphered
 * @offset: Number of offsets to start printing structure.
 *
 * Builds SaHpiOemEventT data into a string buffer.
 *
 * Returns:
 * SA_OK - Normal operation.
 * SA_ERR_HPI_INVALID_PARAMS - Pointer parameter(s) are NULL.
 **/
static SaErrorT oh_build_event_oem(oh_big_textbuffer *buffer, const SaHpiEventT *event, int offsets)
{
        char str[SAHPI_MAX_TEXT_BUFFER_LENGTH];
        SaErrorT err;
        SaHpiTextBufferT tmpbuffer;

        if ( !buffer || !event) {
                err("Invalid parameter.");
                return(SA_ERR_HPI_INVALID_PARAMS);
        }

        oh_append_offset(buffer, offsets);
        snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "OemEvent:\n");
        oh_append_bigtext(buffer, str);
        offsets++;

        oh_append_offset(buffer, offsets);
        err = oh_decode_manufacturerid(event->EventDataUnion.OemEvent.MId, &tmpbuffer);
        snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "ManufacturerId: %s\n", tmpbuffer.Data);
        oh_append_bigtext(buffer, str);

        oh_append_offset(buffer, offsets);
        snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "OemEventData:\n");
        oh_append_bigtext(buffer, str);
        offsets++;

        oh_build_textbuffer(buffer, &event->EventDataUnion.OemEvent.OemEventData, offsets);

        return(SA_OK);
}

/**
 * oh_build_event_user:
 * @buffer: Pointer to space to decipher SaHpiEventT struct
 * @event: Pointer to the event to be deciphered
 * @offset: Number of offsets to start printing structure.
 *
 *
 * Returns:
 * SA_OK - Normal operation.
 * SA_ERR_HPI_INVALID_PARAMS - Pointer parameter(s) are NULL.
 **/
static SaErrorT oh_build_event_user(oh_big_textbuffer *buffer, const SaHpiEventT *event, int offsets)
{
        char str[SAHPI_MAX_TEXT_BUFFER_LENGTH];

        if ( !buffer || !event) {
                err("Invalid parameter.");
                return(SA_ERR_HPI_INVALID_PARAMS);
        }

/*
 *      oh_append_offset(buffer, offsets);
 *      snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "UserEvent:\n");
 *      oh_append_bigtext(buffer, str);
 *      offsets++
 */
        oh_append_offset(buffer, offsets);
        snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "UserEventData:\n");
        oh_append_bigtext(buffer, str);
        offsets++;

        oh_build_textbuffer(buffer, &event->EventDataUnion.UserEvent.UserEventData, offsets);
        return(SA_OK);
}

/**
 * oh_build_event_dimi:
 * @buffer: Pointer to space to decipher SaHpiEventT struct
 * @event: Pointer to the event to be deciphered
 * @offset: Number of offsets to start printing structure.
 *
 *
 * Returns:
 * SA_OK - Normal operation.
 * SA_ERR_HPI_INVALID_PARAMS - Pointer parameter(s) are NULL.
 **/
static SaErrorT oh_build_event_dimi(oh_big_textbuffer *buffer, const SaHpiEventT *event, int offsets)
{
        char str[SAHPI_MAX_TEXT_BUFFER_LENGTH];
        const SaHpiDimiEventT* de = &(event->EventDataUnion.DimiEvent);

        if ( !buffer || !event) {
                dbg("Invalid parameter.");
                return(SA_ERR_HPI_INVALID_PARAMS);
        }

        oh_append_offset(buffer, offsets);
        snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "DimiEventData:\n");
        oh_append_bigtext(buffer, str);
        offsets++;

        oh_append_offset(buffer, offsets);
        snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "DimiNum: %d\n", de->DimiNum);
        oh_append_bigtext(buffer, str);

        oh_append_offset(buffer, offsets);
        snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "TestNum: %d\n", de->TestNum);
        oh_append_bigtext(buffer, str);

        oh_append_offset(buffer, offsets);
        snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "TestRunStatus: %s\n",
                 oh_lookup_dimitestrunstatus(de->DimiTestRunStatus));
        oh_append_bigtext(buffer, str);

        if ( de->DimiTestPercentCompleted != 0xff ) {
            oh_append_offset(buffer, offsets);
            snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "Percent completed: %d\n", de->DimiTestPercentCompleted);
            oh_append_bigtext(buffer, str);
        }

        return(SA_OK);
}

/**
 * oh_build_event_dimi_update:
 * @buffer: Pointer to space to decipher SaHpiEventT struct
 * @event: Pointer to the event to be deciphered
 * @offset: Number of offsets to start printing structure.
 *
 *
 * Returns:
 * SA_OK - Normal operation.
 * SA_ERR_HPI_INVALID_PARAMS - Pointer parameter(s) are NULL.
 **/
static SaErrorT oh_build_event_dimi_update(oh_big_textbuffer *buffer, const SaHpiEventT *event, int offsets)
{
        char str[SAHPI_MAX_TEXT_BUFFER_LENGTH];
        const SaHpiDimiUpdateEventT* de = &(event->EventDataUnion.DimiUpdateEvent);

        if ( !buffer || !event) {
                dbg("Invalid parameter.");
                return(SA_ERR_HPI_INVALID_PARAMS);
        }

        oh_append_offset(buffer, offsets);
        snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "DimiUpdateEventData:\n");
        oh_append_bigtext(buffer, str);
        offsets++;

        oh_append_offset(buffer, offsets);
        snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "DimiNum: %d\n", de->DimiNum);
        oh_append_bigtext(buffer, str);

        return(SA_OK);
}

/**
 * oh_build_event_fumi:
 * @buffer: Pointer to space to decipher SaHpiEventT struct
 * @event: Pointer to the event to be deciphered
 * @offset: Number of offsets to start printing structure.
 *
 *
 * Returns:
 * SA_OK - Normal operation.
 * SA_ERR_HPI_INVALID_PARAMS - Pointer parameter(s) are NULL.
 **/
static SaErrorT oh_build_event_fumi(oh_big_textbuffer *buffer, const SaHpiEventT *event, int offsets)
{
        char str[SAHPI_MAX_TEXT_BUFFER_LENGTH];
        const SaHpiFumiEventT* fe = &(event->EventDataUnion.FumiEvent);

        if ( !buffer || !event) {
                dbg("Invalid parameter.");
                return(SA_ERR_HPI_INVALID_PARAMS);
        }

        oh_append_offset(buffer, offsets);
        snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "FumiEventData:\n");
        oh_append_bigtext(buffer, str);
        offsets++;

        oh_append_offset(buffer, offsets);
        snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "FumiNum: %d\n", fe->FumiNum);
        oh_append_bigtext(buffer, str);

        oh_append_offset(buffer, offsets);
        snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "BankNum: %d\n", fe->BankNum);
        oh_append_bigtext(buffer, str);

        oh_append_offset(buffer, offsets);
        snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "UpgradeStatus: %s\n",
                 oh_lookup_fumiupgradestatus(fe->UpgradeStatus));
        oh_append_bigtext(buffer, str);

        return(SA_OK);
}

/**
 * oh_fprint_ctrlstate:
 * @stream: File handle.
 * @event: Pointer to SaHpitrlStateT to be printed.
 * @offsets: Number of offsets to start printing structure.
 *
 * Prints the member data contained in SaHpiCtrlStateT struct to a file.
 * The MACRO oh_print_ctrlstate(), uses this function to print to STDOUT.
 *
 * Returns:
 * SA_OK - Normal operation.
 * SA_ERR_HPI_INVALID_PARAMS - Pointer parameter(s) are NULL.
 **/
SaErrorT oh_fprint_ctrlstate(FILE *stream, const SaHpiCtrlStateT *thisctrlstate, int offsets)
{

        SaErrorT rv = SA_OK;
        oh_big_textbuffer buffer;
        SaHpiTextBufferT smallbuf;
        char str[SAHPI_MAX_TEXT_BUFFER_LENGTH];

        if (!stream || !thisctrlstate) {
                err("Invalid parameter.");
                return(SA_ERR_HPI_INVALID_PARAMS);
        }

        oh_init_bigtext(&buffer);
        oh_append_offset(&buffer, offsets);
        snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "Type: %s\n",
                                oh_lookup_ctrltype(thisctrlstate->Type));
        oh_append_bigtext(&buffer, str);
        oh_append_offset(&buffer, offsets);

        switch(thisctrlstate->Type) {
                case SAHPI_CTRL_TYPE_DIGITAL:
                        snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "State: %s\n",
                                oh_lookup_ctrlstatedigital(thisctrlstate->StateUnion.Digital));
                        break;
                case SAHPI_CTRL_TYPE_DISCRETE:
                        snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "State: %d\n",
                                                        thisctrlstate->StateUnion.Discrete);
                        break;
                case SAHPI_CTRL_TYPE_ANALOG:
                        snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "Analog: %d\n",
                                                        thisctrlstate->StateUnion.Analog);
                        break;
                case SAHPI_CTRL_TYPE_STREAM:
                        snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "Stream:\n");
                        oh_append_bigtext(&buffer, str);
                        oh_append_offset(&buffer, offsets + 1);
                        snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "Repeat: %s\n",
                                (thisctrlstate->StateUnion.Stream.Repeat == SAHPI_TRUE) ? "TRUE" : "FALSE");
                        oh_append_bigtext(&buffer, str);
                        oh_append_offset(&buffer, offsets + 1);
                        snprintf(str, thisctrlstate->StateUnion.Stream.StreamLength, "%s\n",
                                                        thisctrlstate->StateUnion.Stream.Stream);

                        break;
                case SAHPI_CTRL_TYPE_TEXT:
                        snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "Line: %d\n",
                                                thisctrlstate->StateUnion.Text.Line);
                        oh_append_bigtext(&buffer, str);
                        oh_append_offset(&buffer, offsets);
                        snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "%s\n",
                                                thisctrlstate->StateUnion.Text.Text.Data);
                        break;
                case SAHPI_CTRL_TYPE_OEM:
                        snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "Oem:\n");
                        oh_append_bigtext(&buffer, str);
                        oh_append_offset(&buffer, offsets + 1);
                        rv = oh_decode_manufacturerid(thisctrlstate->StateUnion.Oem.MId, &smallbuf);
                        snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "ManufacturerId: %s\n",
                                                                                 smallbuf.Data);
                        oh_append_bigtext(&buffer, str);
                        oh_append_offset(&buffer, offsets + 1);
                        snprintf(str, thisctrlstate->StateUnion.Oem.BodyLength, "%s\n",
                                                        thisctrlstate->StateUnion.Oem.Body);

                        break;
                default:
                        snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "Invalid Control Type\n");
                        rv = SA_ERR_HPI_INVALID_DATA;
                        break;
        }
        oh_append_bigtext(&buffer, str);
        rv = oh_fprint_bigtext(stream, &buffer);
        return(rv);
}

/**
 * oh_valid_textbuffer:
 * @buffer: Pointer to a SaHpiTextBufferT structure.
 *
 * Verifies @buffer is a valid SaHpiTextBufferT structure.
 * Used in routines where the user can set SaHpiTextBufferT values.
 *
 * Returns:
 * SAHPI_TRUE - if @buffer is valid.
 * SAHPI_FALSE - if @buffer not valid.
 **/
SaHpiBoolT oh_valid_textbuffer(SaHpiTextBufferT *buffer)
{
        int i;
        SaHpiUint16T c1, c2;
        if (!buffer) return SAHPI_FALSE;
        if (oh_lookup_texttype(buffer->DataType) == NULL) return SAHPI_FALSE;
        /* Compiler checks DataLength <= SAHPI_MAX_TEXT_BUFFER_LENGTH */

        switch (buffer->DataType) {
        case SAHPI_TL_TYPE_UNICODE:
                /* Validate language and length */
                if (oh_lookup_language(buffer->Language) == NULL) return SAHPI_FALSE;
                if (buffer->DataLength % 2 != 0) return SAHPI_FALSE;
                /* Validate utf-16 buffers in the case of surrogates */
                for (i = 0; i < buffer->DataLength; i = i + 2) {
                        /* here I am assuming that the least
                         * significant byte is first */
                        c1 = buffer->Data[i];
                        c1 |= buffer->Data[i+1] << 8;
                        if (U_IS_SURROGATE(c1)) {
                                if (buffer->DataLength > i+3) {
                                        c2 = buffer->Data[i+2];
                                        c2 |= buffer->Data[i+3] << 8;
                                        if (U_IS_LEAD(c1) && U_IS_TRAIL(c2)) {
                                                /* already checked for trail ..
                                                 * so increment i by 2 */
                                                i = i + 2;
                                        } else {
                                                /* found a unpaired surrogate */
                                                return SAHPI_FALSE;
                                        }
                                } else {
                                        /* found a unpaired surrogate */
                                        return SAHPI_FALSE;
                                }
                        }
                }
                break;
        case SAHPI_TL_TYPE_BCDPLUS:
                /* 8-bit ASCII, '0'-'9', space, dash, period, colon, comma, or
                   underscore. Always encoded in HPI as 8-bit values */
                for (i = 0;
                     i < buffer->DataLength && i < SAHPI_MAX_TEXT_BUFFER_LENGTH;
                     i++)
                {
                        if ((buffer->Data[i] < 0x30 || buffer->Data[i] > 0x39) &&
                             buffer->Data[i] != 0x20 && buffer->Data[i] != 0x2d &&
                             buffer->Data[i] != 0x2e && buffer->Data[i] != 0x3a &&
                             buffer->Data[i] != 0x2c && buffer->Data[i] != 0x5f)
                                return SAHPI_FALSE;
                }
                break;
        case SAHPI_TL_TYPE_ASCII6:
                /* reduced set, 0x20-0x5f only. Always encoded in HPI as 8-bit values */
                for (i = 0;
                     i < buffer->DataLength && i < SAHPI_MAX_TEXT_BUFFER_LENGTH;
                     i++)
                {
                        if (buffer->Data[i] < 0x20 || buffer->Data[i] > 0x5f)
                                return SAHPI_FALSE;
                }
                break;
        case SAHPI_TL_TYPE_TEXT:
                if (oh_lookup_language(buffer->Language) == NULL) { return(SAHPI_FALSE); }
                /* all character values are good 0x00 - 0xff */
                break;
        case SAHPI_TL_TYPE_BINARY: /* No check possible */
                break;
        default:
                err("Invalid data type");
                return(SAHPI_FALSE);
        }

        return(SAHPI_TRUE);
}

#define validate_threshold(thdname, thdmask) \
do { \
if (thds->thdname.IsSupported) { \
        if (!(writable_thds & thdmask)) return(SA_ERR_HPI_INVALID_CMD); \
        if (format.ReadingType != thds->thdname.Type) return(SA_ERR_HPI_INVALID_DATA); \
        switch (format.ReadingType) { \
        case SAHPI_SENSOR_READING_TYPE_INT64: \
                if (thdmask == SAHPI_STM_UP_HYSTERESIS || thdmask == SAHPI_STM_LOW_HYSTERESIS) { \
                        if (thds->thdname.Value.SensorInt64 < 0) return(SA_ERR_HPI_INVALID_DATA); \
                } else { \
                        if (format.Range.Flags & SAHPI_SRF_MAX) { \
                                if (thds->thdname.Value.SensorInt64 > format.Range.Max.Value.SensorInt64) \
                                        return(SA_ERR_HPI_INVALID_CMD); \
                        } \
                        if (format.Range.Flags & SAHPI_SRF_MIN) { \
                                if (thds->thdname.Value.SensorInt64 < format.Range.Min.Value.SensorInt64) \
                                        return(SA_ERR_HPI_INVALID_CMD); \
                        } \
                } \
                break; \
        case SAHPI_SENSOR_READING_TYPE_FLOAT64: \
                if (thdmask == SAHPI_STM_UP_HYSTERESIS || thdmask == SAHPI_STM_LOW_HYSTERESIS) { \
                        if (thds->thdname.Value.SensorFloat64 < 0) return(SA_ERR_HPI_INVALID_DATA); \
                } else { \
                        if (format.Range.Flags & SAHPI_SRF_MAX) { \
                                if (thds->thdname.Value.SensorFloat64 > format.Range.Max.Value.SensorFloat64) \
                                        return(SA_ERR_HPI_INVALID_CMD); \
                        } \
                        if (format.Range.Flags & SAHPI_SRF_MIN) { \
                                if (thds->thdname.Value.SensorFloat64 < format.Range.Min.Value.SensorFloat64) \
                                        return(SA_ERR_HPI_INVALID_CMD); \
                        } \
                } \
                break; \
        case SAHPI_SENSOR_READING_TYPE_UINT64: \
                if (thdmask == SAHPI_STM_UP_HYSTERESIS || thdmask == SAHPI_STM_LOW_HYSTERESIS) { \
                } else { \
                        if (format.Range.Flags & SAHPI_SRF_MAX) { \
                                if (thds->thdname.Value.SensorUint64 > format.Range.Max.Value.SensorUint64) \
                                        return(SA_ERR_HPI_INVALID_CMD); \
                        } \
                        if (format.Range.Flags & SAHPI_SRF_MIN) { \
                                if (thds->thdname.Value.SensorUint64 < format.Range.Min.Value.SensorUint64) \
                                        return(SA_ERR_HPI_INVALID_CMD); \
                        } \
                } \
                break; \
        case SAHPI_SENSOR_READING_TYPE_BUFFER: \
		break; \
        default: \
                err("Invalid threshold reading type."); \
                return(SA_ERR_HPI_INVALID_CMD); \
        } \
} \
} while(0)

#define validate_threshold_order(thdtype) \
do { \
if (thds->UpCritical.IsSupported == SAHPI_TRUE) { \
        if (thds->UpMajor.IsSupported == SAHPI_TRUE) { \
                if (thds->UpCritical.Value.thdtype < thds->UpMajor.Value.thdtype) \
                        return(SA_ERR_HPI_INVALID_DATA); \
        } \
        if (thds->UpMinor.IsSupported == SAHPI_TRUE) { \
                if (thds->UpCritical.Value.thdtype < thds->UpMinor.Value.thdtype) \
                        return(SA_ERR_HPI_INVALID_DATA); \
        } \
        if (thds->LowMinor.IsSupported == SAHPI_TRUE) { \
                if (thds->UpCritical.Value.thdtype < thds->LowMinor.Value.thdtype) \
                        return(SA_ERR_HPI_INVALID_DATA); \
        } \
        if (thds->LowMajor.IsSupported == SAHPI_TRUE) { \
                if (thds->UpCritical.Value.thdtype < thds->LowMajor.Value.thdtype) \
                        return(SA_ERR_HPI_INVALID_DATA); \
        } \
        if (thds->LowCritical.IsSupported == SAHPI_TRUE) { \
                if (thds->UpCritical.Value.thdtype < thds->LowCritical.Value.thdtype) \
                        return(SA_ERR_HPI_INVALID_DATA); \
        } \
} \
if (thds->UpMajor.IsSupported == SAHPI_TRUE) { \
        if (thds->UpMinor.IsSupported == SAHPI_TRUE) { \
                if (thds->UpMajor.Value.thdtype < thds->UpMinor.Value.thdtype) \
                               return(SA_ERR_HPI_INVALID_DATA); \
        } \
        if (thds->LowMinor.IsSupported == SAHPI_TRUE) { \
                if (thds->UpMajor.Value.thdtype < thds->LowMinor.Value.thdtype) \
                        return(SA_ERR_HPI_INVALID_DATA); \
        } \
        if (thds->LowMajor.IsSupported == SAHPI_TRUE) { \
                if (thds->UpMajor.Value.thdtype < thds->LowMajor.Value.thdtype) \
                        return(SA_ERR_HPI_INVALID_DATA); \
        } \
        if (thds->LowCritical.IsSupported == SAHPI_TRUE) { \
                if (thds->UpMajor.Value.thdtype < thds->LowCritical.Value.thdtype) \
                        return(SA_ERR_HPI_INVALID_DATA); \
        } \
} \
if (thds->UpMinor.IsSupported == SAHPI_TRUE) { \
        if (thds->LowMinor.IsSupported == SAHPI_TRUE) { \
                if (thds->UpMinor.Value.thdtype < thds->LowMinor.Value.thdtype) \
                        return(SA_ERR_HPI_INVALID_DATA); \
        } \
        if (thds->LowMajor.IsSupported == SAHPI_TRUE) { \
                if (thds->UpMinor.Value.thdtype < thds->LowMajor.Value.thdtype) \
                        return(SA_ERR_HPI_INVALID_DATA); \
        } \
        if (thds->LowCritical.IsSupported == SAHPI_TRUE) { \
                if (thds->UpMinor.Value.thdtype < thds->LowCritical.Value.thdtype) \
                        return(SA_ERR_HPI_INVALID_DATA); \
        } \
} \
if (thds->LowMinor.IsSupported == SAHPI_TRUE) { \
        if (thds->LowMajor.IsSupported == SAHPI_TRUE) { \
                if (thds->LowMinor.Value.thdtype < thds->LowMajor.Value.thdtype) \
                        return(SA_ERR_HPI_INVALID_DATA); \
        } \
        if (thds->LowCritical.IsSupported == SAHPI_TRUE) { \
                if (thds->LowMinor.Value.thdtype < thds->LowCritical.Value.thdtype) \
                        return(SA_ERR_HPI_INVALID_DATA); \
        } \
} \
if (thds->LowMajor.IsSupported == SAHPI_TRUE) { \
        if (thds->LowCritical.IsSupported == SAHPI_TRUE) { \
                if (thds->LowMajor.Value.thdtype < thds->LowCritical.Value.thdtype) \
                        return(SA_ERR_HPI_INVALID_DATA); \
        } \
} \
} while(0)

/**
 * oh_valid_thresholds:
 * @thds: Location of threshold definitions to verify.
 * @rdr: Location of sensor's RDR.
 *
 * Validates that the threshold values defined in @thds are valid for a sensor
 * described by @rdr.  The caller may need to read the sensor's existing
 * sensors first, since @thds may only contain a subset of the possible
 * writable thresholds.
 *
 * Return values:
 * SA_OK - normal case.
 * SA_ERR_HPI_INVALID_CMD - Non-writable thresholds, invalid thresholds values, or invalid data type.
 * SA_ERR_HPI_INVALID_DATA - Threshold values out of order; negative hysteresis value.
 * SA_ERR_HPI_INVALID_PARAMS - Pointer parameter(s) are NULL.
 **/
SaErrorT oh_valid_thresholds(SaHpiSensorThresholdsT *thds, SaHpiRdrT *rdr)
{
        SaHpiSensorDataFormatT format;
        SaHpiSensorThdMaskT writable_thds;

        if (!thds || !rdr) {
                err("Invalid parameter.");
                return(SA_ERR_HPI_INVALID_PARAMS);
        }
        if (rdr->RdrType != SAHPI_SENSOR_RDR) {
                err("Invalid parameter");
                return(SA_ERR_HPI_INVALID_PARAMS);
        }

        format = rdr->RdrTypeUnion.SensorRec.DataFormat;
        writable_thds = rdr->RdrTypeUnion.SensorRec.ThresholdDefn.WriteThold;

        if (rdr->RdrTypeUnion.SensorRec.Category != SAHPI_EC_THRESHOLD ||
            rdr->RdrTypeUnion.SensorRec.ThresholdDefn.IsAccessible == SAHPI_FALSE ||
            rdr->RdrTypeUnion.SensorRec.ThresholdDefn.WriteThold == 0) return(SA_ERR_HPI_INVALID_CMD);

        validate_threshold(LowCritical, SAHPI_STM_LOW_CRIT);
        validate_threshold(LowMajor, SAHPI_STM_LOW_MAJOR);
        validate_threshold(LowMinor, SAHPI_STM_LOW_MINOR);
        validate_threshold(UpCritical, SAHPI_STM_UP_CRIT);
        validate_threshold(UpMajor, SAHPI_STM_UP_MAJOR);
        validate_threshold(UpMinor, SAHPI_STM_UP_MINOR);
        validate_threshold(PosThdHysteresis, SAHPI_STM_UP_HYSTERESIS);
        validate_threshold(NegThdHysteresis, SAHPI_STM_LOW_HYSTERESIS);

        /* Validate defined thresholds are in order:
         * upper critical >= upper major >= upper minor >=
         * lower minor >= lower major >= lower critical
         */
        switch (format.ReadingType) {
        case SAHPI_SENSOR_READING_TYPE_INT64:
                validate_threshold_order(SensorInt64);
                break;
        case SAHPI_SENSOR_READING_TYPE_FLOAT64:
                validate_threshold_order(SensorFloat64);
                break;
        case SAHPI_SENSOR_READING_TYPE_UINT64:
                validate_threshold_order(SensorUint64);
                break;
        case SAHPI_SENSOR_READING_TYPE_BUFFER:
		break;
        default:
                err("Invalid threshold reading type.");
                return(SA_ERR_HPI_INVALID_CMD);
        }

        return(SA_OK);
}

/**
 * oh_fprint_thresholds
 * @stream: file handle
 * @thresholds: sensor thresholds to be printed
 * @format: data format for corresponding sensor
 * @offsets: indentation for printing structure
 *
 *
 * Returns:
 * SA_ERR_HPI_INVALID_PARAMS - one or more of the parameters is NULL.
 * SA_ERR_HPI_INVALID_DATA - a value is not one of the enumerated values for the type.
 * SA_OK - Normal operation.
 **/
SaErrorT oh_fprint_thresholds(FILE *stream,
			      const SaHpiSensorThresholdsT *thresholds,
			      const SaHpiSensorDataFormatT *format,
			      int offsets)
{
	oh_big_textbuffer bigbuf;

        if (!stream || !thresholds || !format) {
                err("Invalid parameter.");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        oh_init_bigtext(&bigbuf);
        oh_append_offset(&bigbuf, offsets);        
        
        if (thresholds->LowCritical.IsSupported) {
        	SaHpiTextBufferT smallbuf;
        	memset(&smallbuf, 0, sizeof(SaHpiTextBufferT));
        	oh_append_bigtext(&bigbuf, "Low Critical: ");
        	oh_decode_sensorreading(thresholds->LowCritical, *format, &smallbuf);
        	oh_append_bigtext(&bigbuf, (char *)smallbuf.Data);
        	oh_append_bigtext(&bigbuf, "\n");
        	oh_append_offset(&bigbuf, offsets);
        }
        if (thresholds->LowMajor.IsSupported) {
        	SaHpiTextBufferT smallbuf;
        	memset(&smallbuf, 0, sizeof(SaHpiTextBufferT));
        	oh_append_bigtext(&bigbuf, "Low Major: ");
        	oh_decode_sensorreading(thresholds->LowMajor, *format, &smallbuf);
        	oh_append_bigtext(&bigbuf, (char *)smallbuf.Data);
        	oh_append_bigtext(&bigbuf, "\n");
        	oh_append_offset(&bigbuf, offsets);
        }
        if (thresholds->LowMinor.IsSupported) {
        	SaHpiTextBufferT smallbuf;
        	memset(&smallbuf, 0, sizeof(SaHpiTextBufferT));
        	oh_append_bigtext(&bigbuf, "Low Minor: ");
        	oh_decode_sensorreading(thresholds->LowMinor, *format, &smallbuf);
        	oh_append_bigtext(&bigbuf, (char *)smallbuf.Data);
        	oh_append_bigtext(&bigbuf, "\n");
        	oh_append_offset(&bigbuf, offsets);
        }
        if (thresholds->UpCritical.IsSupported) {
        	SaHpiTextBufferT smallbuf;
        	memset(&smallbuf, 0, sizeof(SaHpiTextBufferT));
        	oh_append_bigtext(&bigbuf, "Up Critical: ");
        	oh_decode_sensorreading(thresholds->UpCritical, *format, &smallbuf);
        	oh_append_bigtext(&bigbuf, (char *)smallbuf.Data);
        	oh_append_bigtext(&bigbuf, "\n");
        	oh_append_offset(&bigbuf, offsets);
        }
        if (thresholds->UpMajor.IsSupported) {
        	SaHpiTextBufferT smallbuf;
        	memset(&smallbuf, 0, sizeof(SaHpiTextBufferT));
        	oh_append_bigtext(&bigbuf, "Up Major: ");
        	oh_decode_sensorreading(thresholds->UpMajor, *format, &smallbuf);
        	oh_append_bigtext(&bigbuf, (char *)smallbuf.Data);
        	oh_append_bigtext(&bigbuf, "\n");
        	oh_append_offset(&bigbuf, offsets);
        }
        if (thresholds->UpMinor.IsSupported) {
        	SaHpiTextBufferT smallbuf;
        	memset(&smallbuf, 0, sizeof(SaHpiTextBufferT));
        	oh_append_bigtext(&bigbuf, "Up Minor: ");
        	oh_decode_sensorreading(thresholds->UpMinor, *format, &smallbuf);
        	oh_append_bigtext(&bigbuf, (char *)smallbuf.Data);
        	oh_append_bigtext(&bigbuf, "\n");
        	oh_append_offset(&bigbuf, offsets);
        }
        if (thresholds->PosThdHysteresis.IsSupported) {
        	SaHpiTextBufferT smallbuf;
        	memset(&smallbuf, 0, sizeof(SaHpiTextBufferT));
        	oh_append_bigtext(&bigbuf, "Positive Hysteresis: ");
        	oh_decode_sensorreading(thresholds->PosThdHysteresis, *format, &smallbuf);
        	oh_append_bigtext(&bigbuf, (char *)smallbuf.Data);
        	oh_append_bigtext(&bigbuf, "\n");
        	oh_append_offset(&bigbuf, offsets);
        }
        if (thresholds->NegThdHysteresis.IsSupported) {
        	SaHpiTextBufferT smallbuf;
        	memset(&smallbuf, 0, sizeof(SaHpiTextBufferT));
        	oh_append_bigtext(&bigbuf, "Negative Hysteresis: ");
        	oh_decode_sensorreading(thresholds->NegThdHysteresis, *format, &smallbuf);
        	oh_append_bigtext(&bigbuf, (char *)smallbuf.Data);
        	oh_append_bigtext(&bigbuf, "\n");
        	oh_append_offset(&bigbuf, offsets);
        }

	return oh_fprint_bigtext(stream, &bigbuf);
}

/**
 * oh_compare_sensorreading:
 * @type: Type of both sensor readings.
 * @reading1: Pointer to sensor reading.
 * @reading1: Pointer to sensor reading.
 *
 * Compares the Value field of two sensor readings. Sensor readings must be of the
 * same Type.
 *
 * Return values:
 * -1 - @reading1 < @reading2
 *  0 - @reading1 = @reading2
 * +1 - @reading1 > @reading2
 **/
int oh_compare_sensorreading(SaHpiSensorReadingTypeT type,
                             SaHpiSensorReadingT *reading1,
                             SaHpiSensorReadingT *reading2)
{
        int res;
        
        switch(type) {
        case SAHPI_SENSOR_READING_TYPE_INT64:
                if (reading1->Value.SensorInt64 < reading2->Value.SensorInt64) { return -1; }
                else {
                        if (reading1->Value.SensorInt64 == reading2->Value.SensorInt64) { return 0; }
                        else { return 1; }
                }
                break;
        case SAHPI_SENSOR_READING_TYPE_UINT64:
                if (reading1->Value.SensorUint64 < reading2->Value.SensorUint64) { return -1; }
                else {
                        if (reading1->Value.SensorUint64 == reading2->Value.SensorUint64) { return 0; }
                        else { return 1; }
                }
                break;
        case SAHPI_SENSOR_READING_TYPE_FLOAT64:
                if (reading1->Value.SensorFloat64 < reading2->Value.SensorFloat64) { return -1; }
                else {
                        if (reading1->Value.SensorFloat64 == reading2->Value.SensorFloat64) { return 0; }
                        else { return 1; }
                }
                break;
        case SAHPI_SENSOR_READING_TYPE_BUFFER:
                res = memcmp(reading1->Value.SensorBuffer, reading2->Value.SensorBuffer,
                              SAHPI_SENSOR_BUFFER_LENGTH);

                if (res < 0) { return -1; }
                else if (res > 0) { return 1; }
                else { return 0; }
                break;
        default:
                err("Invalid sensor reading type.");
                return 0;
        }
}

/**
 * oh_valid_ctrl_state_mode:
 * @ctrl_rdr: Pointer to control's RDR information.
 * @mode: Control's mode.
 * @state: Pointer to contol's state.
 *
 * Verifies that the @mode and @state data are compatible with a control's RDR information.
 * This routine performs all the static checks defined in the HPI spec but the caller must
 * perform the following checks:
 *   - Verify control's resource has SAHPI_CAPABILITY_CONTROL set.
 *   - Check to see if control is on, if SAHPI_STATE_PULSE_ON is supported and set;
 *     Check to see if control is off, if SAHPI_STATE_PULSE_OFF is supported and set.
 *     Caller needs to return SA_ERR_HPI_INVALID_REQUEST in either of these cases.
 *
 * As specified in the HPI spec, if @mode = SAHPI_CTRL_MODE_AUTO, this routine
 * ignores @state.
 *
 * Return values:
 * SA_OK - Normal operation.
 * SA_ERR_HPI_INVALID_PARAMS - See HPI spec.
 * SA_ERR_HPI_INVALID_DATA - See HPI spec.
 * SA_ERR_HPI_READ_ONLY - See HPI spec.
 **/
SaErrorT oh_valid_ctrl_state_mode(SaHpiCtrlRecT *ctrl_rdr,
                                  SaHpiCtrlModeT mode,
                                  SaHpiCtrlStateT *state)
{
        /* Check for valid mode operations */
        if (NULL == oh_lookup_ctrlmode(mode)) return(SA_ERR_HPI_INVALID_PARAMS);
        if (ctrl_rdr->DefaultMode.ReadOnly == SAHPI_TRUE) {
                if (mode != ctrl_rdr->DefaultMode.Mode) return(SA_ERR_HPI_READ_ONLY);
        }
        if (mode != SAHPI_CTRL_MODE_AUTO && !state) return(SA_ERR_HPI_INVALID_PARAMS);

        /* Check for valid state operations */
        if (mode != SAHPI_CTRL_MODE_AUTO) {
                if (ctrl_rdr->Type != state->Type) return(SA_ERR_HPI_INVALID_DATA);
                if (NULL == oh_lookup_ctrltype(state->Type)) return(SA_ERR_HPI_INVALID_DATA);

                switch(state->Type) {
                case SAHPI_CTRL_TYPE_DIGITAL:
                        if (NULL == oh_lookup_ctrlstatedigital(state->StateUnion.Digital))
                                return(SA_ERR_HPI_INVALID_PARAMS);
                        break;
                case SAHPI_CTRL_TYPE_DISCRETE:
                        /* No HPI spec error check - leave to caller, if needed */
                        break;
                case SAHPI_CTRL_TYPE_ANALOG:
                        if (state->StateUnion.Analog < ctrl_rdr->TypeUnion.Analog.Min)
                                return(SA_ERR_HPI_INVALID_DATA);
                        if (state->StateUnion.Analog > ctrl_rdr->TypeUnion.Analog.Max)
                                return(SA_ERR_HPI_INVALID_DATA);
                        break;
                case SAHPI_CTRL_TYPE_STREAM:
                        if (state->StateUnion.Stream.StreamLength > SAHPI_CTRL_MAX_STREAM_LENGTH)
                                return(SA_ERR_HPI_INVALID_PARAMS);
                        break;
                case SAHPI_CTRL_TYPE_TEXT:
                        if (state->StateUnion.Text.Text.DataType !=
                            ctrl_rdr->TypeUnion.Text.DataType)
                                return(SA_ERR_HPI_INVALID_DATA);
                        if (state->StateUnion.Text.Text.DataType == SAHPI_TL_TYPE_UNICODE ||
                            state->StateUnion.Text.Text.DataType == SAHPI_TL_TYPE_TEXT) {
                                if (state->StateUnion.Text.Text.Language !=
                                    ctrl_rdr->TypeUnion.Text.Language)
                                        return(SA_ERR_HPI_INVALID_DATA);
                        }
                        if (!oh_valid_textbuffer(&(state->StateUnion.Text.Text)))
                                return (SA_ERR_HPI_INVALID_PARAMS);
                        {       /* Check for text buffer overflow */
                                int char_num;

                                if (state->StateUnion.Text.Line >
                                    ctrl_rdr->TypeUnion.Text.MaxLines)
                                        return(SA_ERR_HPI_INVALID_DATA);

                                if (state->StateUnion.Text.Text.DataType == SAHPI_TL_TYPE_UNICODE) {
                                        char_num = state->StateUnion.Text.Text.DataLength/2;
                                }
                                else {
                                        char_num = state->StateUnion.Text.Text.DataLength;
                                }

                                if (char_num) {
                                        int max_chars;

                                        if (state->StateUnion.Text.Line == SAHPI_TLN_ALL_LINES) {
                                                max_chars =
                                                        ctrl_rdr->TypeUnion.Text.MaxLines *
                                                        ctrl_rdr->TypeUnion.Text.MaxChars;
                                        }
                                        else {
                                                max_chars =
                                                        (ctrl_rdr->TypeUnion.Text.MaxLines *
                                                         ctrl_rdr->TypeUnion.Text.MaxChars) -
                                                        ((state->StateUnion.Text.Line - 1) *
                                                        ctrl_rdr->TypeUnion.Text.MaxChars);
                                        }

                                        if (char_num > max_chars) return(SA_ERR_HPI_INVALID_DATA);
                                }
                        }
                        break;
                case SAHPI_CTRL_TYPE_OEM:
                        /* No HPI spec error check - leave to caller, if needed */
                        break;
                default:
                        err("Invalid control state");
                        return(SA_ERR_HPI_INTERNAL_ERROR);
                }
        }

        return(SA_OK);
}
