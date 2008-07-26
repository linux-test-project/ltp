

/*      -*- linux-c -*-
 *
 * Copyright (c) 2005 by Intel Corp.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  This
 * file and program are licensed under a BSD style license.  See
 * the Copying file included with the OpenHPI distribution for
 * full licensing terms.
 *
 * Authors:
 *     Vadim Revyakin <vadim.a.revyakin@intel.com>
 */
 
  
#include "ipmi.h"
#include "ekeyfru.h"
#include <oh_utils.h>
#include <string.h>



typedef struct atca_oem_field_info {
	SaHpiEntryIdT	field;
	unsigned int	rec_num;
	unsigned int	pad_len;
	unsigned int	off;
	unsigned int	len;
} atca_oem_field_info_t;

typedef struct atca_oem_area_info {
	SaHpiEntryIdT	area;
	unsigned int	record;
	int		field_num;
	atca_oem_field_info_t *fields;
} atca_oem_area_info_t;

typedef struct ohoi_atca_pw_on_seq_s {
	unsigned char head[7];
	unsigned char updated;
	int dsk_num;
} ohoi_atca_pw_on_seq_t;



	/* Structures to create E-Keying sensors */
	
typedef unsigned char GUID_t[16];

typedef struct {
	unsigned char link_grouping_id;
	unsigned char link_type;
	unsigned char link_type_extension;
	unsigned char interface_type;
	unsigned char channels[16];
} ekey_descriptor_t;

typedef struct {
	GUID_t	guids[15];
	int	guid_number;
	GSList	*descriptors;
} ekey_sensors_info_t;


static SaHpiEntryIdT init_ftrst_fields(atca_oem_field_info_t *fields,
                              unsigned char *data)
{
	// Manufacture Id
	fields[0].field = OHOI_FIELD_FIRST_ID;
	fields[0].off   = 0;
        fields[0].len   = 3;
	// Record Id
	fields[1].field = OHOI_FIELD_FIRST_ID + 1;
	fields[1].off   = 3;
        fields[1].len   = 1;
	// Format version
	fields[2].field = OHOI_FIELD_FIRST_ID + 2;
	fields[2].off   = 4;
        fields[2].len   = 1;
	
	return OHOI_FIELD_FIRST_ID + 3;
}



static atca_oem_area_info_t *create_address_table_oem_area(
					unsigned char *data,
					unsigned int length)
{
	atca_oem_field_info_t *fields;
	atca_oem_area_info_t *area;
	unsigned int len, off;
	SaHpiEntryIdT cur_field;
	unsigned int i;


	if (length < 27 + data[26] * 3) {
		err("Record length(0x%x) mismatches with expected(0x%x)",
			length, 27 + data[26] * 3);
		return NULL;
	}

	len = 6 + data[26];
	fields = malloc(sizeof (atca_oem_field_info_t) * len);
	if (fields == NULL) {
		err("Out of memory");
		return NULL;
	}
	memset(fields, 0, sizeof (atca_oem_field_info_t) * len); 

	cur_field = init_ftrst_fields(fields, data);
	// Type/Length of Address
	fields[cur_field - OHOI_FIELD_FIRST_ID].field = cur_field;
	fields[cur_field - OHOI_FIELD_FIRST_ID].off   = 5;
        fields[cur_field - OHOI_FIELD_FIRST_ID].len   = 1;
	cur_field++;
	// Shelf Address
	fields[cur_field - OHOI_FIELD_FIRST_ID].field = cur_field;
	fields[cur_field - OHOI_FIELD_FIRST_ID].off   = 6;
        fields[cur_field - OHOI_FIELD_FIRST_ID].len   = 6;
	cur_field++;
	// Entries Count
	fields[cur_field - OHOI_FIELD_FIRST_ID].field = cur_field;
	fields[cur_field - OHOI_FIELD_FIRST_ID].off   = 26;
        fields[cur_field - OHOI_FIELD_FIRST_ID].len   = 1;
	cur_field++;

	off = 27;
	for (i = 0; i < data[26]; i++) {
		// Address Mapping Tables
		fields[cur_field - OHOI_FIELD_FIRST_ID].field = cur_field;
		fields[cur_field - OHOI_FIELD_FIRST_ID].off = off + 3 * i;
        	fields[cur_field - OHOI_FIELD_FIRST_ID].len = 3;
		cur_field++;
	}

	area = malloc(sizeof (atca_oem_area_info_t));
	if (area == NULL) {
		err("Out of memory");
		free(fields);
		return NULL;
	}
	area->field_num = cur_field - OHOI_FIELD_FIRST_ID;
	area->fields = fields;
	return area;
}


static atca_oem_area_info_t *create_shm_ip_connection_oem_area(
                                        unsigned char *data,
					unsigned int length)
{
	atca_oem_field_info_t *fields;
	atca_oem_area_info_t *area;
	int len;
	SaHpiEntryIdT cur_field;


	len = 6;
	fields = malloc(sizeof (atca_oem_field_info_t) * len);
	if (fields == NULL) {
		err("Out of memory");
		return NULL;
	}
	memset(fields, 0,sizeof (atca_oem_field_info_t) * len);

	cur_field = init_ftrst_fields(fields, data);
	// Shelf Manager IP Address
	fields[cur_field - OHOI_FIELD_FIRST_ID].field = cur_field;
	fields[cur_field - OHOI_FIELD_FIRST_ID].off   = 5;
        fields[cur_field - OHOI_FIELD_FIRST_ID].len   = 4;
        fields[cur_field - OHOI_FIELD_FIRST_ID].pad_len   = 12;
	cur_field++;
	// Default Gateway Address
	fields[cur_field - OHOI_FIELD_FIRST_ID].field = cur_field;
	fields[cur_field - OHOI_FIELD_FIRST_ID].off   = 9;
        fields[cur_field - OHOI_FIELD_FIRST_ID].len   = 4;
        fields[cur_field - OHOI_FIELD_FIRST_ID].pad_len   = 12;
	cur_field++;
	// Subnet Mask
	fields[cur_field - OHOI_FIELD_FIRST_ID].field = cur_field;
	fields[cur_field - OHOI_FIELD_FIRST_ID].off   = 13;
        fields[cur_field - OHOI_FIELD_FIRST_ID].len   = 4;
        fields[cur_field - OHOI_FIELD_FIRST_ID].pad_len   = 12;
	cur_field++;

	area = malloc(sizeof (atca_oem_area_info_t));
	if (area == NULL) {
		err("Out of memory");
		free(fields);
		return NULL;
	}
	area->field_num = cur_field - OHOI_FIELD_FIRST_ID;
	area->fields = fields;

	return area;
}

static void handle_ekey_dsk(ekey_sensors_info_t *eks_info,
                          unsigned char *bytes)
{
	int ch_num = CHANNEL_NUM(bytes);
	unsigned char link_grouping_id = LINK_GROUPING_ID(bytes);
	unsigned char link_type_extension = LINK_TYPE_EXTENSION(bytes);
	unsigned char link_type = LINK_TYPE(bytes);
	unsigned char interface_type = INTERFACE_TYPE(bytes);
	unsigned char ports = PORTS(bytes);



	ekey_descriptor_t *dsk = NULL;
	GSList *node;
#if DEBUG_EKEY
printf("EKEY %02X%02X%02X%02X ", bytes[0], bytes[1], bytes[2], bytes[3]);
#endif
	if (ch_num == 0 || ch_num >= 16) {
		err("channel number too big = %d", ch_num);
#if DEBUG_EKEY
printf("channel number too big = %d\n", ch_num);
#endif
		return;
	}

	for (node = eks_info->descriptors; node; node = g_slist_next(node)) {
		dsk = g_slist_nth_data(node, 0);
		if (link_grouping_id == 0) {
			break;
		}
		if (dsk->link_grouping_id != link_grouping_id) {
			continue;
		}
		if (dsk->link_type_extension != link_type_extension) {
			continue;
		}
		if (dsk->link_type != link_type) {
			continue;
		}
		if (dsk->interface_type != interface_type) {
			continue;
		}
		dsk->channels[ch_num - 1] |= ports;
#if DEBUG_EKEY
printf("FOUND: (%d,%d,%d,%d) add ports 0x%x to channel %d\n",
link_grouping_id, link_type, link_type_extension, interface_type,
dsk->channels[ch_num], ch_num);
#endif
		return;
	}
	
	dsk = malloc(sizeof (ekey_descriptor_t));
	if (dsk == NULL) {
		err("No Memory");
		return;
	}
	memset(dsk, 0, sizeof (ekey_descriptor_t));
	dsk->link_grouping_id = link_grouping_id;
	dsk->link_type_extension = link_type_extension;
	dsk->link_type = link_type;
	dsk->interface_type = interface_type;
	dsk->channels[ch_num - 1] = ports;
#if DEBUG_EKEY
printf("NEW:   (%d,%d,%d,%d)     ports 0x%x to channel %d\n",
link_grouping_id, link_type, link_type_extension, interface_type,
dsk->channels[ch_num - 1], ch_num);
#endif
	eks_info->descriptors = g_slist_append(eks_info->descriptors,
	                                         dsk);
}

static atca_oem_area_info_t *create_board_p2p_connectivity_oem_area(
                                        unsigned char *data,
					unsigned int length,
					ekey_sensors_info_t *eks_info)
{
	atca_oem_field_info_t *fields;
	atca_oem_area_info_t *area;
	int len;
	int num_desk;
	int i;
	int off;
	SaHpiEntryIdT cur_field;

	if (length < 6 + data[5] * 16) {
		err("Record length(0x%x) mismatches with expected(0x%x)",
			length, 6 + data[5] * 16);
		return NULL;
	}
	if ((length - (6 + data[5] * 16)) % 4) {
		err("The rest of record is not divisible by 4: %d - (%d)",
			length, 6 + data[5] * 16);
		return NULL;
	}
	num_desk = (length - (6 + data[5] * 16)) / 4;
	len = 4 + data[5] + num_desk + 1;
	fields = malloc(sizeof (atca_oem_field_info_t) * len);
	if (fields == NULL) {
		err("Out of memory");
		return NULL;
	}
	memset(fields, 0,sizeof (atca_oem_field_info_t) * len);

	cur_field = init_ftrst_fields(fields, data);
	// OEM GUID count
	fields[cur_field - OHOI_FIELD_FIRST_ID].field = cur_field;
	fields[cur_field - OHOI_FIELD_FIRST_ID].off   = 5;
        fields[cur_field - OHOI_FIELD_FIRST_ID].len   = 1;
	cur_field++;
	
	for (i = 0; i < data[5]; i++) {
		// OEM GUIDs
		fields[cur_field - OHOI_FIELD_FIRST_ID].field = cur_field;
		fields[cur_field - OHOI_FIELD_FIRST_ID].off   = 6 + 16 * i;
        	fields[cur_field - OHOI_FIELD_FIRST_ID].len   = 16;
		cur_field++;
		if (eks_info->guid_number < 15) {
			memcpy(eks_info->guids[eks_info->guid_number],
							&data[6 + 16 * i], 16);
			eks_info->guid_number++;
		} else {
			err("Too many GUIDS");
		}
	}
#if 0
	// Number of link descriptors (virtual field)
	fields[cur_field - OHOI_FIELD_FIRST_ID].field = cur_field;
	fields[cur_field - OHOI_FIELD_FIRST_ID].off   = num_desk;
        fields[cur_field - OHOI_FIELD_FIRST_ID].len   = 0;
	cur_field++;
#endif
	off = 6 + 16 * data[5];
	for (i = 0; i < num_desk; i++) {
		// Link Descriptors
		fields[cur_field - OHOI_FIELD_FIRST_ID].field = cur_field;
		fields[cur_field - OHOI_FIELD_FIRST_ID].off   = off + 4 * i;
        	fields[cur_field - OHOI_FIELD_FIRST_ID].len   = 4;
		cur_field++;
		handle_ekey_dsk(eks_info, &data[off + 4 * i]);
	}

	area = malloc(sizeof (atca_oem_area_info_t));
	if (area == NULL) {
		err("Out of memory");
		free(fields);
		return NULL;
	}
	area->field_num = cur_field - OHOI_FIELD_FIRST_ID;
	area->fields = fields;

	return area;
}

static atca_oem_area_info_t *create_p2p_connectivity_oem_area(
					unsigned char *data,
					int desk_num,
					unsigned int length)
{
	atca_oem_field_info_t *fields;
	atca_oem_area_info_t *area;
	int len;
	SaHpiEntryIdT cur_field;
	int off = 5;
	int i;

	for (i = 0; i < desk_num; i++) {
		if (off + 3 + data[off + 2] * 3 >=  length) {
			err("dismatch datalen(0x%x) and record struct(0x%x)"
				"  desk_num = %d",
				length, off + 3 + data[off + 2] * 3, desk_num);
			return NULL;
		}
		off = off + 3 + data[off + 2] * 3;
	}
	//printf("   ****  create_p2p_connectivity_oem_area: %d: off = 0x%x, num = 0x%x\n",
	// feed_num, off, data[off + 2]);
	len = 6 + data[off + 2];

	fields = malloc(sizeof (atca_oem_field_info_t) * len);
	if (fields == NULL) {
		err("Out of memory");
		return NULL;
	}
	memset(fields, 0,sizeof (atca_oem_field_info_t) * len);
 
	cur_field = init_ftrst_fields(fields, data);
	// Point-to-Point Channel Type
	fields[cur_field - OHOI_FIELD_FIRST_ID].field = cur_field;
	fields[cur_field - OHOI_FIELD_FIRST_ID].off   = off;
        fields[cur_field - OHOI_FIELD_FIRST_ID].len   = 1;
	cur_field++;
	// Slot Address
	fields[cur_field - OHOI_FIELD_FIRST_ID].field = cur_field;
	fields[cur_field - OHOI_FIELD_FIRST_ID].off   = off + 1;
        fields[cur_field - OHOI_FIELD_FIRST_ID].len   = 1;
	cur_field++;
	// Channel Count
	fields[cur_field - OHOI_FIELD_FIRST_ID].field = cur_field;
	fields[cur_field - OHOI_FIELD_FIRST_ID].off   = off + 2;
        fields[cur_field - OHOI_FIELD_FIRST_ID].len   = 1;
	cur_field++;


	for (i = 0; i < data[off + 2]; i++) {
		fields[cur_field - OHOI_FIELD_FIRST_ID].field = cur_field;
		fields[cur_field - OHOI_FIELD_FIRST_ID].off   = off +3 + 3 * i;
        	fields[cur_field - OHOI_FIELD_FIRST_ID].len   = 3;
		cur_field++;
	}

	area = malloc(sizeof (atca_oem_area_info_t));
	if (area == NULL) {
		err("Out of memory");
		free(fields);
		return NULL;
	}
	area->field_num = cur_field - OHOI_FIELD_FIRST_ID;
	area->fields = fields;
	return area;
}






								
static atca_oem_area_info_t *create_power_distribution_oem_area(
					unsigned char *data,
					int feed_num,
					unsigned int length)
{
	atca_oem_field_info_t *fields;
	atca_oem_area_info_t *area;
	int len;
	SaHpiEntryIdT cur_field;
	int off = 6;
	int i;

	for (i = 0; i < feed_num; i++) {
		if (off + 6 + data[off + 5] * 2 >= length) {
			err("dismatch datalen(0x%x) and record struct("
				"0x%x + 6 + 0x%x * 2)"
				" feed_num = %d",
				length, off, data[off + 5], feed_num);
			return NULL;
		}
		off += 6 + data[off + 5] * 2;

	}
	//printf("   ****  create_power_distribution_oem_area: %d: off = 0x%x, num = 0x%x\n",
	//feed_num, off, data[off + 5]);

	len = 7 + data[off + 5];

	fields = malloc(sizeof (atca_oem_field_info_t) * len);
	if (fields == NULL) {
		err("Out of memory");
		return NULL;
	}
	memset(fields, 0,sizeof (atca_oem_field_info_t) * len);
 
	cur_field = init_ftrst_fields(fields, data);
	// Maximum External Available Current
	fields[cur_field - OHOI_FIELD_FIRST_ID].field = cur_field;
	fields[cur_field - OHOI_FIELD_FIRST_ID].off   = off;
        fields[cur_field - OHOI_FIELD_FIRST_ID].len   = 2;
	cur_field++;
	// Maximum Internal Current
	fields[cur_field - OHOI_FIELD_FIRST_ID].field = cur_field;
	fields[cur_field - OHOI_FIELD_FIRST_ID].off   = off + 2;
        fields[cur_field - OHOI_FIELD_FIRST_ID].len   = 2;
	cur_field++;
	// Maximum Expected Operating Voltage
	fields[cur_field - OHOI_FIELD_FIRST_ID].field = cur_field;
	fields[cur_field - OHOI_FIELD_FIRST_ID].off   = off + 4;
        fields[cur_field - OHOI_FIELD_FIRST_ID].len   = 1;
	cur_field++;
	// FRU to Feed Mapping Entries Count	
	fields[cur_field - OHOI_FIELD_FIRST_ID].field = cur_field;
	fields[cur_field - OHOI_FIELD_FIRST_ID].off   = off + 5;
        fields[cur_field - OHOI_FIELD_FIRST_ID].len   = 1;
	cur_field++;
	for (i = 0; i < data[off + 5]; i++) {
		fields[cur_field - OHOI_FIELD_FIRST_ID].field = cur_field;
		fields[cur_field - OHOI_FIELD_FIRST_ID].off   = off + 6 + i * 2;
        	fields[cur_field - OHOI_FIELD_FIRST_ID].len   = 2;
		cur_field++;
	}

	area = malloc(sizeof (atca_oem_area_info_t));
	if (area == NULL) {
		err("Out of memory");
		free(fields);
		return NULL;
	}
	area->field_num = cur_field - OHOI_FIELD_FIRST_ID;
	area->fields = fields;
	return area;
}



static atca_oem_area_info_t *create_activation_and_pm_oem_area(
					unsigned char *data,
					unsigned int length)
{
	atca_oem_field_info_t *fields;
	atca_oem_area_info_t *area;
	unsigned int len, off;
	unsigned int i;
	SaHpiEntryIdT cur_field;


	if (length < 7 + data[6] * 5) {
		err("Record length(0x%x) mismatches with expected(0x%x)",
			length, 7 + data[6] * 5);
		return NULL;
	}

	len = 5 + data[6];
	fields = malloc(sizeof (atca_oem_field_info_t) * len);
	if (fields == NULL) {
		err("Out of memory");
		return NULL;
	}
	memset(fields, 0,sizeof (atca_oem_field_info_t) * len); 

	cur_field = init_ftrst_fields(fields, data);
	// Allowance For FRU Activation Readiness
	fields[cur_field - OHOI_FIELD_FIRST_ID].field = cur_field;
	fields[cur_field - OHOI_FIELD_FIRST_ID].off   = 5;
        fields[cur_field - OHOI_FIELD_FIRST_ID].len   = 1;
	cur_field++;
	// Descripror Count
	fields[cur_field - OHOI_FIELD_FIRST_ID].field = cur_field;
	fields[cur_field - OHOI_FIELD_FIRST_ID].off   = 6;
        fields[cur_field - OHOI_FIELD_FIRST_ID].len   = 1;
	cur_field++;

	off = 7;
	for (i = 0; i < data[6]; i++) {
		// Address Mapping Tables
		fields[cur_field - OHOI_FIELD_FIRST_ID].field = cur_field;
		fields[cur_field - OHOI_FIELD_FIRST_ID].off   = off + 5 * i;
        	fields[cur_field - OHOI_FIELD_FIRST_ID].len   = 5;
		cur_field++;
	}

	area = malloc(sizeof (atca_oem_area_info_t));
	if (area == NULL) {
		err("Out of memory");
		free(fields);
		return NULL;
	}
	area->field_num = cur_field - OHOI_FIELD_FIRST_ID;
	area->fields = fields;
	return area;
}





static atca_oem_area_info_t *create_radial_ipmb0_link_oem_area(
					unsigned char *data,
					unsigned int length,
					unsigned int *max_link)
{
	atca_oem_field_info_t *fields;
	atca_oem_area_info_t *area;
	unsigned int len, off;
	unsigned int i;
	SaHpiEntryIdT cur_field;


	if (length < 11 + data[10] * 2) {
		err("Record length(0x%x) mismatches with expected(0x%x)",
			length, 11 + data[10] * 2);
		return NULL;
	}

	len = 6 + data[10];
	fields = malloc(sizeof (atca_oem_field_info_t) * len);
	if (fields == NULL) {
		err("Out of memory");
		return NULL;
	}
	memset(fields, 0,sizeof (atca_oem_field_info_t) * len); 

	cur_field = init_ftrst_fields(fields, data);
	// IPMB-0 Connector Definer
	fields[cur_field - OHOI_FIELD_FIRST_ID].field = cur_field;
	fields[cur_field - OHOI_FIELD_FIRST_ID].off   = 5;
        fields[cur_field - OHOI_FIELD_FIRST_ID].len   = 3;
	cur_field++;
	// IPMB-0 Connector Version ID
	fields[cur_field - OHOI_FIELD_FIRST_ID].field = cur_field;
	fields[cur_field - OHOI_FIELD_FIRST_ID].off   = 8;
        fields[cur_field - OHOI_FIELD_FIRST_ID].len   = 2;
	cur_field++;
	// Address Entry Count
	fields[cur_field - OHOI_FIELD_FIRST_ID].field = cur_field;
	fields[cur_field - OHOI_FIELD_FIRST_ID].off   = 10;
        fields[cur_field - OHOI_FIELD_FIRST_ID].len   = 1;
	cur_field++;
	off = 11;
	for (i = 0; i < data[10]; i++) {
		// IPMIB-0 Link Mapping Entries
		fields[cur_field - OHOI_FIELD_FIRST_ID].field = cur_field;
		fields[cur_field - OHOI_FIELD_FIRST_ID].off   = off + 2 * i;
        	fields[cur_field - OHOI_FIELD_FIRST_ID].len   = 2;
		cur_field++;
		if (data[off + 2 * i + 1] > *max_link) {
			// calculating maximum link number for IPMB controls
			*max_link = data[off + 2 * i + 1];
		}
		
	}

	area = malloc(sizeof (atca_oem_area_info_t));
	if (area == NULL) {
		err("Out of memory");
		free(fields);
		return NULL;
	}
	area->field_num = cur_field - OHOI_FIELD_FIRST_ID;
	area->fields = fields;
	return area;
}



static atca_oem_area_info_t *create_amc_carrier_information_table_oem_area(
                                        unsigned char *data,
					unsigned int length)
{
	atca_oem_field_info_t *fields;
	atca_oem_area_info_t *area;
	int len;
	SaHpiEntryIdT cur_field;
	int off;
	int i;

	if (length < 7 + data[6]) {
		err("Record length(0x%x) mismatches with expected(0x%x)",
			length, 7 + data[6]);
		return NULL;
	}
	len = 5 + data[6];
	fields = malloc(sizeof (atca_oem_field_info_t) * len);
	if (fields == NULL) {
		err("Out of memory");
		return NULL;
	}
	memset(fields, 0,sizeof (atca_oem_field_info_t) * len);

	cur_field = init_ftrst_fields(fields, data);
	// AMC.0 Extension Version
	fields[cur_field - OHOI_FIELD_FIRST_ID].field = cur_field;
	fields[cur_field - OHOI_FIELD_FIRST_ID].off   = 5;
        fields[cur_field - OHOI_FIELD_FIRST_ID].len   = 1;
	cur_field++;
	// Carrier Site Number Count
	fields[cur_field - OHOI_FIELD_FIRST_ID].field = cur_field;
	fields[cur_field - OHOI_FIELD_FIRST_ID].off   = 6;
        fields[cur_field - OHOI_FIELD_FIRST_ID].len   = 1;
	cur_field++;

	off = 7;
	for (i = 0; i < data[6]; i++) {
		// Carrier Site Number
		fields[cur_field - OHOI_FIELD_FIRST_ID].field = cur_field;
		fields[cur_field - OHOI_FIELD_FIRST_ID].off   = off + i;
        	fields[cur_field - OHOI_FIELD_FIRST_ID].len   = 1;
		cur_field++;
		
	}

	area = malloc(sizeof (atca_oem_area_info_t));
	if (area == NULL) {
		err("Out of memory");
		free(fields);
		return NULL;
	}
	area->field_num = cur_field - OHOI_FIELD_FIRST_ID;
	area->fields = fields;

	return area;
}



static atca_oem_area_info_t *create_carrier_activ_and_curmngmt_oem_area(
                                        unsigned char *data,
					unsigned int length)
{
	atca_oem_field_info_t *fields;
	atca_oem_area_info_t *area;
	int len;
	SaHpiEntryIdT cur_field;
	int off;
	int i;

	if (length < 9 + data[8] * 3) {
		err("Record length(0x%x) mismatches with expected(0x%x)",
			length, 9 + data[9]);
		return NULL;
	}
	len = 6 + data[8];
	fields = malloc(sizeof (atca_oem_field_info_t) * len);
	if (fields == NULL) {
		err("Out of memory");
		return NULL;
	}
	memset(fields, 0,sizeof (atca_oem_field_info_t) * len);

	cur_field = init_ftrst_fields(fields, data);
	// Maximum Internal Current
	fields[cur_field - OHOI_FIELD_FIRST_ID].field = cur_field;
	fields[cur_field - OHOI_FIELD_FIRST_ID].off   = 5;
        fields[cur_field - OHOI_FIELD_FIRST_ID].len   = 2;
	cur_field++;
	// Allowance For Module Activation Readiness
	fields[cur_field - OHOI_FIELD_FIRST_ID].field = cur_field;
	fields[cur_field - OHOI_FIELD_FIRST_ID].off   = 7;
        fields[cur_field - OHOI_FIELD_FIRST_ID].len   = 1;
	cur_field++;
	// Module Activation and Power Descriptor Count
	fields[cur_field - OHOI_FIELD_FIRST_ID].field = cur_field;
	fields[cur_field - OHOI_FIELD_FIRST_ID].off   = 8;
        fields[cur_field - OHOI_FIELD_FIRST_ID].len   = 1;
	cur_field++;

	off = 9;
	for (i = 0; i < data[8]; i++) {
		// Carrier Activation and Power Descriptors
		fields[cur_field - OHOI_FIELD_FIRST_ID].field = cur_field;
		fields[cur_field - OHOI_FIELD_FIRST_ID].off   = off + i * 3;
        	fields[cur_field - OHOI_FIELD_FIRST_ID].len   = 3;
		cur_field++;
		
	}

	area = malloc(sizeof (atca_oem_area_info_t));
	if (area == NULL) {
		err("Out of memory");
		free(fields);
		return NULL;
	}
	area->field_num = cur_field - OHOI_FIELD_FIRST_ID;
	area->fields = fields;

	return area;
}




static atca_oem_area_info_t *create_carrier_p2p_connectivity_oem_area(
                                        unsigned char *data,
					unsigned int length)
{
	atca_oem_field_info_t *fields;
	atca_oem_area_info_t *area;
	int len;
	SaHpiEntryIdT cur_field;
	int off;
	int i;

	if (length < 7 + data[6] * 3) {
		err("Record length(0x%x) mismatches with expected(0x%x)",
			length, 7 + data[6]);
		return NULL;
	}
	len = 5 + data[6];
	fields = malloc(sizeof (atca_oem_field_info_t) * len);
	if (fields == NULL) {
		err("Out of memory");
		return NULL;
	}
	memset(fields, 0,sizeof (atca_oem_field_info_t) * len);

	cur_field = init_ftrst_fields(fields, data);
	// Resource ID
	fields[cur_field - OHOI_FIELD_FIRST_ID].field = cur_field;
	fields[cur_field - OHOI_FIELD_FIRST_ID].off   = 5;
        fields[cur_field - OHOI_FIELD_FIRST_ID].len   = 1;
	cur_field++;
	// Point-to-Point Port Count
	fields[cur_field - OHOI_FIELD_FIRST_ID].field = cur_field;
	fields[cur_field - OHOI_FIELD_FIRST_ID].off   = 6;
        fields[cur_field - OHOI_FIELD_FIRST_ID].len   = 1;
	cur_field++;


	off = 7;
	for (i = 0; i < data[6]; i++) {
		// Carrier Activation and Power Descriptors
		fields[cur_field - OHOI_FIELD_FIRST_ID].field = cur_field;
		fields[cur_field - OHOI_FIELD_FIRST_ID].off   = off + i * 3;
        	fields[cur_field - OHOI_FIELD_FIRST_ID].len   = 3;
		cur_field++;
		
	}

	area = malloc(sizeof (atca_oem_area_info_t));
	if (area == NULL) {
		err("Out of memory");
		free(fields);
		return NULL;
	}
	area->field_num = cur_field - OHOI_FIELD_FIRST_ID;
	area->fields = fields;

	return area;
}










unsigned int ohoi_create_atca_oem_idr_areas(
			struct oh_handler_state *handler,
			ipmi_entity_t *entity,
			struct ohoi_resource_info *res_info,
			struct ohoi_inventory_info *i_info,
			unsigned int r_num)
{
//	struct ohoi_handler *ipmi_handler = handler->data;
	ipmi_fru_t   *fru = ipmi_entity_get_fru(entity);
	atca_oem_area_info_t	*area;
	atca_oem_field_info_t *fields;
	unsigned char		data[256];
	int			rv;
	unsigned int		num;
	unsigned int		len;
	unsigned int		area_num = 0;
	unsigned int		is_atca_field = 0;
	GSList *areas = NULL;
	int i;
	unsigned char ver, type;
	unsigned int max_link = 0;
	ekey_sensors_info_t eks_info;


	memset(&eks_info, 0, sizeof (eks_info));
	for (num = 0; num < r_num; num++) {
		len = 256;
		rv = ipmi_fru_get_multi_record_data(fru, num, data, &len);
		if (rv != 0) {
			err("ipmi_fru_get_multi_record_data("
				"fru, %d, data, 0x%x) = 0x%x",
				num, len, rv);
			continue;
		}
		rv = ipmi_fru_get_multi_record_type(fru, num, &type);
		if (rv) {
			err("ipmi_entity_get_multi_record_type(%d) = %d", num, rv);
			continue;
		}
		if (type != 0xc0) {
			// record type. Must be OEM
			err("Record #%d type = 0x%x", num, data[0]);
			goto orig_record;
		}
		rv = ipmi_fru_get_multi_record_format_version(fru, num, &ver);
		if (rv) {
			err("ipmi_entity_get_multi_record_format_version"
				"(%d) = %d", num, rv);
			continue;
		}

		if ((ver & 0x0f) != 0x2) {
			// must be 2 for PICMG 3.0 ATCA vD1.0
			err("Record #%d format version = 0x%x", num, data[1] & 0x0f);
			goto orig_record;
		}
		if (len < 5) {
			err("Record #%d too short(%d)", num, len);
			goto orig_record;
		}
		if ((data[0] | (data[1] << 8) | (data[2] << 16)) !=
						ATCAHPI_PICMG_MID) {
			err("Record #%d. MId = 0x%x", num,
					data[0] | (data[1] << 8) | (data[2] << 16));
			goto orig_record;
		}
		switch (data[3]) {
		case 0x10 :	// Address Table Record
			if (len < 27) {
				err("Record #%d too short. len = 0x%x", num, len);
				goto orig_record;
			}
			if (data[4] != 0) {
				err("wrong RecFormVersion record #%d = %d",
						num, data[4]);
				goto orig_record;
			}
			area = create_address_table_oem_area(data, len);
			if (area == NULL) {
				goto orig_record;
			}
			break;
		case 0x13 :	// Shelf Manager IP Connection Record
			if (len < 17) {
				err("Record #%d too short. len = 0x%x", num, len);
				goto orig_record;
			}
			if (data[4] != 1) {
				err("wrong RecFormVersion record #%d = %d",
						num, data[4]);
				goto orig_record;
			}
			area = create_shm_ip_connection_oem_area(data, len);
			break;
		case 0x04 :	// Backplane Point-to-Point Connectivity Record
			if (len < 8) {
				err("Record #%d too short. len = 0x%x", num, len);
				goto orig_record;
			}
			if (data[4] != 0) {
				err("wrong RecFormVersion record #%d = %d",
						num, data[4]);
				goto orig_record;
			}
			for (i = 0; ; i++) {			
				area = create_p2p_connectivity_oem_area(
							data, i, len);
				if (area != NULL) {
					area->record = num;
					area->area = FIRST_OEM_AREA_NUM + area_num;
					areas = g_slist_append(areas, area);
					area_num++;
					is_atca_field++;
				} else {
					if (i == 0) {
						goto orig_record;
					}
					break;
				}
			}
			continue;
		case 0x11 :	// Shelf Power Distribution Record
			if (len < 10) {
				err("Record #%d too short. len = 0x%x", num, len);
				goto orig_record;
			}
			if (data[4] != 0) {
				err("wrong RecFormVersion record #%d = %d",
						num, data[4]);
				goto orig_record;
			}
			for (i = 0; i < data[5]; i++) {
				area = create_power_distribution_oem_area(
								data, i, len);
				if (area != NULL) {
					area->record = num;
					area->area = FIRST_OEM_AREA_NUM + area_num;
					areas = g_slist_append(areas, area);
					area_num++;
					is_atca_field++;
				}
			}
			area = NULL;					
			continue;
		case 0x12 :	// Shelf Activation and Power Management Record
			if (len < 7) {
				err("Record #%d too short. len = 0x%x", num, len);
				goto orig_record;
			}
			if (data[4] != 0) {
				err("wrong RecFormVersion record #%d = %d",
						num, data[4]);
				goto orig_record;
			}
			area = create_activation_and_pm_oem_area(data, len);
			break;
		case 0x14 :	// Board Point-to-Point Connectivity Record
			if (len < 6) {
				err("Record #%d too short. len = 0x%x",
								num, len);
				goto orig_record;
			}
			if (data[4] != 0) {
				err("wrong RecFormVersion record #%d = %d",
						num, data[4]);
				goto orig_record;
			}
			area = create_board_p2p_connectivity_oem_area(data,
							     len, &eks_info);
			break;
		case 0x15 :	// Radial IPMB-0 Link Mapping Record
			if (len < 11) {
				err("Record #%d too short. len = 0x%x",
								num, len);
				goto orig_record;
			}
			if (data[4] != 0) {
				err("wrong RecFormVersion record #%d = %d",
						num, data[4]);
				goto orig_record;
			}
			area = create_radial_ipmb0_link_oem_area(
							data, len, &max_link);
			if (area == NULL) {
				goto orig_record;
			}
			break;
		case 0x1a :	// Carrier Information Table
			if (len < 7) {
				err("Record #%d too short. len = 0x%x",
								num, len);
				goto orig_record;
			}
			if (data[4] != 0) {
				err("wrong RecFormVersion record #%d = %d",
						num, data[4]);
				goto orig_record;
			}
			area = create_amc_carrier_information_table_oem_area(
								     data, len);
			if (area == NULL) {
				goto orig_record;
			}
			break;
		case 0x17 : // Carrier Activation and Current Management Record
			if (len < 9) {
				err("Record #%d too short. len = 0x%x",
								num, len);
				goto orig_record;
			}
			if (data[4] != 0) {
				err("wrong RecFormVersion record #%d = %d",
						num, data[4]);
				goto orig_record;
			}
			area = create_carrier_activ_and_curmngmt_oem_area(
								     data, len);
			if (area == NULL) {
				goto orig_record;
			}
			break;
		case 0x18 : // Carrier Point-to-Point Connectivity Record
			if (len < 9) {
				err("Record #%d too short. len = 0x%x",
								num, len);
				goto orig_record;
			}
			if (data[4] != 0) {
				err("wrong RecFormVersion record #%d = %d",
						num, data[4]);
				goto orig_record;
			}
			area = create_carrier_p2p_connectivity_oem_area(
								     data, len);
			if (area == NULL) {
				goto orig_record;
			}
			break;
		default :
			err("Unknown record #%d Id = 0x%x", num, data[3]);
			area = NULL;
			goto orig_record;
		}
		if (area == NULL) {
			continue;
		}
		area->record = num;
		area->area = FIRST_OEM_AREA_NUM + area_num;
		area_num++;
		is_atca_field++;
		areas = g_slist_append(areas, area);
		continue;
orig_record:
		// We met not ATCA specific record. Put it into special area
		fields = malloc(sizeof (atca_oem_field_info_t));
		if (fields == NULL) {
			err("Out of memory");
			continue;
		}
		memset(fields, 0, sizeof (atca_oem_field_info_t));
		area = malloc(sizeof (atca_oem_area_info_t));
		if (area == NULL) {
			err("Out of memory");
			free(fields);
			continue;
		}
		fields->field = OHOI_FIELD_FIRST_ID;
		fields->off = 0;
		fields->len = len;
		area->record = num;
		area->field_num = 1;
		area->area = FIRST_OEM_AREA_NUM + area_num;
		area_num++;
		area->fields = fields;
		areas = g_slist_append(areas, area);
	}
#if 0
	res_info->max_ipmb0_link = max_link;
	if (ipmi_entity_get_type(entity) == IPMI_ENTITY_MC) {
		ohoi_create_ipmb0_controls(handler, entity, max_link);
	}
#endif

	if (!is_atca_field) {
		return 1;
	}
#if 0
	{
		GSList *node;
		atca_oem_area_info_t *ar;
	
		for (node = areas; node; node = node->next) {
			ar = node->data;
			printf("    *******    area %d; num_fields %d\n",
						ar->area, ar->field_num);
		}
	}
#endif


	// create Ekeying link state sensors
	GSList *node;
	unsigned int s_num = 0;
	ekey_descriptor_t *dsk;
	for (node = eks_info.descriptors; node; node = g_slist_next(node)) {
		dsk = g_slist_nth_data(node, 0);
		if (dsk->link_type <0xf0) {
			ohoi_create_ekeying_link_state_sensor(handler,
				entity, s_num, NULL,
				dsk->link_grouping_id, dsk->link_type,
				dsk->link_type_extension, dsk->interface_type,
				dsk->channels);
		} else {
			ohoi_create_ekeying_link_state_sensor(handler,
				entity, s_num,
				eks_info.guids[dsk->link_type - 0xf0],
				dsk->link_grouping_id, dsk->link_type,
				dsk->link_type_extension, dsk->interface_type,
				dsk->channels);
		}
		s_num++;
	}
        g_slist_foreach(eks_info.descriptors, (GFunc)g_free, NULL);
        g_slist_free(eks_info.descriptors);
	
	
	i_info->oem_areas = areas;
	return area_num;
}




extern void ohoi_delete_oem_area(gpointer arg, gpointer u_data);
void ohoi_delete_oem_area(gpointer arg, gpointer u_data)
{
	atca_oem_area_info_t	*area = arg;
	if (area && area->fields)
		free(area->fields);
	if (area)
		free(area);
}



SaHpiUint32T ohoi_atca_oem_area_fields_num(struct ohoi_inventory_info *fru,
			    SaHpiEntryIdT areaid)
{

	GSList *node;
	atca_oem_area_info_t *area;
	
	for (node = fru->oem_areas; node; node = node->next) {
		area = node->data;
		if (area->area == areaid) {
			return area->field_num;
		}
	}
	return 0;
}


struct atca_oem_area_field_s {
	atca_oem_field_info_t *fld;
	SaHpiIdrFieldT *field;
	SaHpiEntryIdT fid;
	int raw_field;
	SaErrorT rv;
	int done;
}; 


static void ohoi_atca_oem_area_field_cb(ipmi_entity_t *ent, void *cb_data)
{
	struct atca_oem_area_field_s *info = cb_data;
	int rv;
	unsigned int len;
	unsigned char type, ver;
	unsigned char buf[SAHPI_MAX_TEXT_BUFFER_LENGTH];
	unsigned int shift = info->fld->pad_len;
	

	rv = ipmi_entity_get_multi_record_data_len(ent, info->fid, &len);
	if (rv) {
		err("ipmi_entity_get_multi_record_data_len = %d", rv);
		info->rv = SA_ERR_HPI_NOT_PRESENT;
		info->done = 1;
		return;
	}
	if (len < info->fld->off + info->fld->len) {
		err("real record too short. %d < %d + %d",
				len, info->fld->off, info->fld->len);
		info->rv = SA_ERR_HPI_NOT_PRESENT;
		info->done = 1;
		return;
	}
	if (info->raw_field) {
		rv = ipmi_entity_get_multi_record_type(ent, info->fid, &type);
		if (rv) {
			err("ipmi_entity_get_multi_record_type = %d", rv);
			info->rv = SA_ERR_HPI_NOT_PRESENT;
			info->done = 1;
			return;
		}
		rv = ipmi_entity_get_multi_record_format_version(ent,
							info->fid, &ver);
		if (rv) {
			err("ipmi_entity_get_multi_record_type = %d", rv);
			info->rv = SA_ERR_HPI_NOT_PRESENT;
			info->done = 1;
			return;
		}
		shift = 2;
	}
/*
	rv = ipmi_entity_get_multi_record_type(ent, f_id, &type);
	if (rv) {
		err("ipmi_entity_get_multi_record_type = %d", rv);
		oif->rv = SA_ERR_HPI_NOT_PRESENT;
		return;
	}
	rv = ipmi_entity_get_multi_record_format_version(ent, f_id, &ver);
	if (rv) {
		err("ipmi_entity_get_multi_record_format_version = %d", rv);
		oif->rv = SA_ERR_HPI_NOT_PRESENT;
		return;
	}
	if (len > SAHPI_MAX_TEXT_BUFFER_LENGTH - 2) {
		len = SAHPI_MAX_TEXT_BUFFER_LENGTH - 2;
	}
*/
	rv = ipmi_entity_get_multi_record_data(ent, info->fid, buf, &len);
	if (rv) {
		err("ipmi_entity_get_multi_record_data = %d", rv);
		info->rv = SA_ERR_HPI_NOT_PRESENT;
		info->done = 1;
		return;
	}
	oh_init_textbuffer(&info->field->Field);
	if (info->raw_field) {
		info->field->Field.Data[0] = type;
		info->field->Field.Data[1] = ver;
	}
	memcpy(info->field->Field.Data + shift,
				&buf[info->fld->off], info->fld->len);
	info->field->Field.Language = SAHPI_LANG_UNDEF;
	info->field->Field.DataType = SAHPI_TL_TYPE_BINARY;
	info->field->Field.DataLength = info->fld->len + shift;
	
	info->done = 1;
}


SaErrorT ohoi_atca_oem_area_field(struct oh_handler_state  *handler,
				struct ohoi_resource_info   *ohoi_res_info,
				SaHpiEntryIdT *nextfieldid,
				SaHpiIdrFieldT *field)
{
	struct ohoi_inventory_info *fru = ohoi_res_info->fru;
	GSList *node = fru->oem_areas;
	atca_oem_area_info_t *area = NULL;
	atca_oem_field_info_t *fields;
	struct atca_oem_area_field_s info;
	int rv;
	int i;
	
	for (node = fru->oem_areas; node; node = g_slist_next(node)) {
		area = g_slist_nth_data(node, 0);
		if (area->area == field->AreaId) {
			break;
		}
	}
	if (area == NULL) {
		err("Area %d not present", field->AreaId);
		return SA_ERR_HPI_NOT_PRESENT;
	}
	
	fields = area->fields;
	for (i = 0; i < area->field_num; i++) {
		if (fields[i].field == field->FieldId) {
			break;
		}
	}
	if (i == area->field_num) {
		err("Field %d for OEM Area %d not present",
				field->FieldId, field->AreaId);
		return SA_ERR_HPI_NOT_PRESENT;
	}
	
	fields += i;
	
	if (fields->len == 0) {
		// this is the virtual field. Compose it
		oh_init_textbuffer(&field->Field);
		field->Field.Data[0] = fields->off;
		field->Field.Language = SAHPI_LANG_UNDEF;
		field->Field.DataType = SAHPI_TL_TYPE_BINARY;
		field->Field.DataLength = 1;
		goto virt_field;
	}

	info.fld = fields;
	info.field = field;
	info.rv = SA_OK;
	info.fid = area->record;
	info.done = 1;
	info.raw_field = (area->field_num == 1);
	rv = ipmi_entity_pointer_cb(ohoi_res_info->u.entity.entity_id,
			ohoi_atca_oem_area_field_cb, &info);
	if (rv) {
		err("ipmi_entity_pointer_cb = 0x%x", rv);
		return SA_ERR_HPI_INTERNAL_ERROR;
	}
	rv = ohoi_loop(&info.done, handler->data);
	if (rv != SA_OK) {
		err("ohoi_loop = %d", rv);
		return rv;
	}
	if (info.rv != SA_OK) {
		err("info.rv = %d", info.rv);
		return info.rv;
	}
virt_field:
	field->ReadOnly = SAHPI_TRUE;
	field->Type = SAHPI_IDR_FIELDTYPE_CUSTOM;
	if (i == area->field_num - 1) {
		*nextfieldid = SAHPI_LAST_ENTRY;
	} else {
		*nextfieldid = fields[1].field;
	}
	return SA_OK;
}


