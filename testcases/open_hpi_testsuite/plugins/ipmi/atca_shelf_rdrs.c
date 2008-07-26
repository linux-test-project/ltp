
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
#include <oh_utils.h>


#define uchar  unsigned char
int ipmicmd_send(ipmi_domain_t *domain,
                 uchar netfn, uchar cmd, uchar lun, uchar chan,
		 uchar *pdata, uchar sdata,
		 ipmi_addr_response_handler_t handler,
		 void *handler_data);


static void init_power_on_sequence_data_cb(ipmi_entity_t *entity,
                                           void *cb_data)
{
	struct ohoi_handler *ipmi_handler = cb_data;
	ipmi_fru_t   *fru = ipmi_entity_get_fru(entity);
	unsigned int num, r_num;
	unsigned int len;
	int broken = 0;
	unsigned char data[256];
	unsigned char type, ver;
	ohoi_atca_pwonseq_rec_t *recp = NULL;
	ohoi_atca_pwonseq_dsk_t *dscp = NULL;
	int i;
	int rv;

	g_slist_foreach(ipmi_handler->atca_pwonseq_recs, (GFunc)g_free, NULL);
	g_slist_free(ipmi_handler->atca_pwonseq_recs);
	g_slist_foreach(ipmi_handler->atca_pwonseq_desk, (GFunc)g_free, NULL);
	g_slist_free(ipmi_handler->atca_pwonseq_desk);
	ipmi_handler->atca_pwonseq_recs = NULL;
	ipmi_handler->atca_pwonseq_desk = NULL;
	ipmi_handler->atca_pwonseq_updated = 0;

	r_num = ipmi_entity_get_num_multi_records(entity);
	for (num = 0; num < r_num; num++) {
		len = 256;
		rv = ipmi_fru_get_multi_record_data(fru, num, data, &len);
		if (rv != 0) {
			err("ipmi_fru_get_multi_record_data("
				"fru, %d, data, 0x%x) = 0x%x",
				num, len, rv);
			broken = 1;
			break;
		}
		rv = ipmi_fru_get_multi_record_type(fru, num, &type);
		if (rv) {
			err("ipmi_entity_get_multi_record_type = %d", rv);
			broken = 1;
			break;
		}
		if (type != 0xc0) {
			// record type. Must be OEM
			err("Record type = 0x%x", data[0]);
			continue;
		}
		rv = ipmi_fru_get_multi_record_format_version(fru, num, &ver);
		if (rv) {
			err("ipmi_entity_get_multi_record_format_version = %d", rv);
			broken = 1;
			break;
		}

		if ((ver & 0x0f) != 0x2) {
			// must be 2 for PICMG 3.0 ATCA vD1.0
			continue;
		}
		if (len < 5) {
			continue;
		}
		if ((data[0] | (data[1] << 8) | (data[2] << 16)) !=
						ATCAHPI_PICMG_MID) {
			err("MId = 0x%x", data[0] | (data[1] << 8) | (data[2] << 16));
			continue;
		}

		if (data[3] != 0x12) {
			continue;
		}

		if (len < 7) {
			err("Record #%d too short(%d)", num, len);
			broken = 1;
			break;
		}
		if (len < 7 + data[6] * 5) {
			err("Record #%d length(0x%x) mismatches with expected(0x%x)",
				num, len, 7 + data[6] * 5);
			broken = 1;
			break;
		}
		recp = malloc(sizeof (ohoi_atca_pwonseq_rec_t));
		if (recp) {
			memcpy(&recp->head, data, 7);
			recp->updated = 0;
			recp->rec_num = num;
			ipmi_handler->atca_pwonseq_recs =
				g_slist_append(ipmi_handler->atca_pwonseq_recs,
				               recp);
		} else {
			err("Out of memory");
			broken = 1;
			break;
		}
		for (i = 0; i < data[6]; i++) {
			dscp = malloc(sizeof (ohoi_atca_pwonseq_dsk_t));
			if (dscp) {
				memcpy(&dscp->body, &data[7 + 5 * i], 5);
				dscp->slotid = 0;
				ipmi_handler->atca_pwonseq_desk =
					g_slist_append(
						ipmi_handler->atca_pwonseq_desk,
						dscp);
			} else {
				err("Out of memory");
				broken = 1;
				break;
			}
		}
	}

	if (broken) {
		// XXX delete all alloced memory
	}
}



	/*
         *      Chassis status control
	 */


struct atca_chassis_status_control_s {
	SaHpiCtrlStateOemT *state;
	int done;
	SaErrorT rv;
};

SaErrorT get_atca_chassis_status_control_state(
                                      struct oh_handler_state *hnd,
                                      struct ohoi_control_info *c,
                                      SaHpiRdrT * rdr,
                                      SaHpiCtrlModeT *mode,
                                      SaHpiCtrlStateT *state);

SaErrorT set_atca_chassis_status_control_state(
                                      struct oh_handler_state *hnd,
                                      struct ohoi_control_info *c,
                                      SaHpiRdrT * rdr,
                                      SaHpiCtrlModeT mode,
                                      SaHpiCtrlStateT *state);

static int get_atca_chassis_status_control_states_cb(
			ipmi_domain_t *domain,
			ipmi_msgi_t   *rspi)
{
	struct atca_chassis_status_control_s *info = rspi->data1;
	ipmi_msg_t *msg = &rspi->msg;
	int rv = msg->data[0];


	dbg("get chassis response(%d): %02x %02x %02x %02x %02x\n",
		msg->data_len, msg->data[0], msg->data[1], msg->data[2],
		msg->data[3], msg->data[4]);

	if (domain == NULL) {
		info->rv = SA_ERR_HPI_INVALID_PARAMS;
		info->done = 1;
		return IPMI_MSG_ITEM_NOT_USED;
	}
	if (rv == 0xc1) {
		info->rv = SA_ERR_HPI_INVALID_CMD;
	} else if (rv == 0xc3) {
		info->rv = SA_ERR_HPI_NO_RESPONSE;
	} else if (rv != 0) {
		info->rv = SA_ERR_HPI_INVALID_PARAMS;
	} else {
		info->state->Body[0] = msg->data[1];
		info->state->Body[1] = msg->data[2];
		info->state->Body[2] = msg->data[3];
		if (msg->data_len > 3) {
			info->state->Body[3] = msg->data[4];
		} else {
			info->state->Body[3] = 0;
		}
		info->state->BodyLength = 4;
		info->state->MId = ATCAHPI_PICMG_MID;
	}
	info->done = 1;
	return IPMI_MSG_ITEM_NOT_USED;
};

static void get_atca_chassis_status_control_states(
			ipmi_domain_t *domain,
			void *cb_data)
{
	struct atca_chassis_status_control_s *info = cb_data;
	unsigned char data[16];
	int rv;


	memset(data, 0, 16);
	rv = ipmicmd_send(domain,
		0x00, 0x01, 0, IPMI_BMC_CHANNEL, data, 0,
		get_atca_chassis_status_control_states_cb,
		cb_data);
	if (rv != 0) {
		err("ipmicmd_send = 0x%x", rv);
		OHOI_MAP_ERROR(info->rv, rv);
		return;
	}
	return;
}





static SaHpiRdrT *create_atca_chassis_status_control(
			struct ohoi_handler *ipmi_handler,
			SaHpiRptEntryT *rpt,
			struct ohoi_control_info **ctrl_info
			)
{
	SaHpiRdrT *rdr;
	struct ohoi_control_info *c_info;
	SaHpiCtrlStateOemT states;
	SaErrorT rv;
	struct atca_chassis_status_control_s info;

	info.state = &states;
	info.done = 0;
	info.rv = SA_OK;
	rv = ipmi_domain_pointer_cb(ipmi_handler->domain_id,
				get_atca_chassis_status_control_states,
				&info);
	if (rv != 0) {
		err("ipmi_domain_pointer_cb = 0x%x", rv);
		return NULL;
	}
	rv = ohoi_loop(&info.done, ipmi_handler);
	if (rv != SA_OK) {
		err("ohoi_loop = 0x%x", rv);
		return NULL;
	}
	if (info.rv != SA_OK) {
		err("info.rv = 0x%x", info.rv);
		return NULL;
	}

	rdr = malloc(sizeof (*rdr));
	if (rdr == NULL) {
		err("Out of memory");
		return NULL;
	}
	c_info = malloc(sizeof (*c_info));
	if (rdr == NULL) {
		err("Out of memory");
		free(rdr);
		return NULL;
	}

	memset(rdr, 0, sizeof (*rdr));
	memset(c_info, 0, sizeof (*c_info));

	rdr->RdrType = SAHPI_CTRL_RDR;
	rdr->IsFru = SAHPI_FALSE;
	rdr->Entity = rpt->ResourceEntity;
	rdr->RdrTypeUnion.CtrlRec.Num = ATCAHPI_CTRL_NUM_SHELF_STATUS;
	rdr->RdrTypeUnion.CtrlRec.OutputType = SAHPI_CTRL_GENERIC;
	rdr->RdrTypeUnion.CtrlRec.Type = SAHPI_CTRL_TYPE_OEM;
	rdr->RdrTypeUnion.CtrlRec.TypeUnion.Oem.Default.MId = ATCAHPI_PICMG_MID;
	rdr->RdrTypeUnion.CtrlRec.TypeUnion.Oem.Default.BodyLength = 4;
	rdr->RdrTypeUnion.CtrlRec.DefaultMode.Mode = SAHPI_CTRL_MODE_AUTO;
	rdr->RdrTypeUnion.CtrlRec.DefaultMode.ReadOnly = SAHPI_TRUE;
	rdr->RdrTypeUnion.CtrlRec.WriteOnly = SAHPI_FALSE;
	rdr->RdrTypeUnion.CtrlRec.Oem = ATCAHPI_PICMG_CT_CHASSIS_STATUS;
	oh_init_textbuffer(&rdr->IdString);
	oh_append_textbuffer(&rdr->IdString, "Chassis Status");

	rdr->RdrTypeUnion.CtrlRec.TypeUnion.Oem.Default.Body[0] =
			states.Body[0];
	rdr->RdrTypeUnion.CtrlRec.TypeUnion.Oem.Default.Body[1] =
			states.Body[1];
	rdr->RdrTypeUnion.CtrlRec.TypeUnion.Oem.Default.Body[2] =
			states.Body[2];
	rdr->RdrTypeUnion.CtrlRec.TypeUnion.Oem.Default.Body[3] =
			states.Body[3];
	rdr->RdrTypeUnion.CtrlRec.TypeUnion.Oem.Default.MId =
						ATCAHPI_PICMG_MID;
	rdr->RdrTypeUnion.CtrlRec.TypeUnion.Oem.MId = ATCAHPI_PICMG_MID;

	c_info->ohoii.get_control_state = get_atca_chassis_status_control_state;
	c_info->ohoii.set_control_state = set_atca_chassis_status_control_state;
	c_info->mode = SAHPI_CTRL_MODE_AUTO;
	*ctrl_info = c_info;

	return rdr;
}


SaErrorT get_atca_chassis_status_control_state(
                                      struct oh_handler_state *hnd,
                                      struct ohoi_control_info *c,
                                      SaHpiRdrT * rdr,
                                      SaHpiCtrlModeT *mode,
                                      SaHpiCtrlStateT *state)
{
	struct atca_chassis_status_control_s info;
	struct ohoi_handler *ipmi_handler = (struct ohoi_handler *)hnd->data;
	SaErrorT rv;

	if (state == NULL) {
		goto no_state;
	}
	info.state = &state->StateUnion.Oem;
	info.done = 0;
	info.rv = SA_OK;
	rv = ipmi_domain_pointer_cb(ipmi_handler->domain_id,
				get_atca_chassis_status_control_states,
				&info);
	if (rv != 0) {
		err("ipmi_domain_pointer_cb = 0x%x", rv);
		return SA_ERR_HPI_INVALID_CMD;
	}
	rv = ohoi_loop(&info.done, ipmi_handler);
	if (rv != SA_OK) {
		err("ohoi_loop = 0x%x", rv);
		return rv;
	}
	if (info.rv != SA_OK) {
		err("info.rv = 0x%x", info.rv);
		return rv;
	}
	state->Type = SAHPI_CTRL_TYPE_OEM;
no_state:
	if (mode) {
		*mode = SAHPI_CTRL_MODE_AUTO;
	}

	return SA_OK;
}

SaErrorT set_atca_chassis_status_control_state(
                                      struct oh_handler_state *hnd,
                                      struct ohoi_control_info *c,
                                      SaHpiRdrT * rdr,
                                      SaHpiCtrlModeT mode,
                                      SaHpiCtrlStateT *state)
{
	return SA_ERR_HPI_READ_ONLY;
}





		/*
		 * Shelf Address control
		 */


struct atca_shelf_address_control_s {
	SaHpiCtrlStateTextT     *text;
	SaHpiTextTypeT type;
	int done;
	SaErrorT rv;
};

static int set_shelf_address_control_msg_data(SaHpiTextTypeT type,
					      SaHpiTextBufferT *text,
					      unsigned char *data)
{
	int i, j, shift;

	if (type == text->DataType) {
		memcpy(data + 2, &text->Data, text->DataLength);
		data[1] = (type << 6);
		switch (type) {
		case SAHPI_TL_TYPE_UNICODE:
			data[1] |= ((text->DataLength & 0x1f) >> 1);
			break;
		case SAHPI_TL_TYPE_BCDPLUS:
			data[1] |= ((text->DataLength * 2) & 0x1f);
			break;
		case SAHPI_TL_TYPE_ASCII6:
			data[1] |= (((text->DataLength * 8 + 5) / 6) & 0x1f);
			break;
		case SAHPI_TL_TYPE_TEXT:
		case SAHPI_TL_TYPE_BINARY:
			data[1] |= (text->DataLength & 0x1f);
			break;
		}
		return 0;
	}
	if ((type != SAHPI_TL_TYPE_ASCII6) || (text->DataType != SAHPI_TL_TYPE_TEXT)) {
		err("Datatype dismaych : %d & %d", type, text->DataType);
		return 1;
	}
	for (i = 0; i < text->DataLength; i++) {
		if ((text->Data[i] < 0x20) || (text->Data[i] > 0x5F)) {
			err("cannot convert TEXT to ASCII6");
			return 1;
		}
	}
	shift = 0;
	j = 0;
	for (i = 0; i < text->DataLength; i++) {
		unsigned char a = text->Data[i] - 0x20;
		switch (shift) {
		case 0:
			data[j] |= (a << 2);
			shift = 6;
			break;
		case 2:
			data[j] |= a;
			j++;
			shift = 0;
			break;
		case 4:
			data[j] |= (a >> 2);
			j++;
			data[j] = (a & 0x03) << 6;
			shift = 2;
			break;
		case 6:
			data[j] |= ((a & 0x30) >> 6);
			j++;
			data[j] = (a & 0x0f) << 4;
			shift = 4;
		}
	}

	data[1] = (unsigned char)((SAHPI_TL_TYPE_ASCII6 << 6) |
					(text->DataLength & 0x0f));
	return 0;
}


static int set_atca_shelf_address_control_states_cb(
			ipmi_domain_t *domain,
			ipmi_msgi_t   *rspi)
{
	struct atca_shelf_address_control_s *info = rspi->data1;
	ipmi_msg_t *msg = &rspi->msg;
	int rv = msg->data[0];


	dbg("set shelf address response(%d): %02x %02x\n",
		msg->data_len, msg->data[0], msg->data[1]);

	if (domain == NULL) {
		info->rv = SA_ERR_HPI_INVALID_PARAMS;
		info->done = 1;
		return IPMI_MSG_ITEM_NOT_USED;
	}
	if (rv == 0xc1) {
		info->rv = SA_ERR_HPI_INVALID_CMD;
	} else if (rv == 0xc3) {
		info->rv = SA_ERR_HPI_NO_RESPONSE;
	} else if (rv != 0) {
		info->rv = SA_ERR_HPI_INVALID_PARAMS;
	}
	info->done = 1;
	return IPMI_MSG_ITEM_NOT_USED;
};


static void set_atca_shelf_address_control_states(
			ipmi_domain_t *domain,
			void *cb_data)
{
	struct atca_shelf_address_control_s *info = cb_data;
	unsigned char data[32];
	int rv;


	memset(data, 0, 32);
	if (set_shelf_address_control_msg_data(info->type,
					&info->text->Text, data)) {
		info->rv = SA_ERR_HPI_INVALID_DATA;
		info->done = 1;
		return;
	}
	dbg("set addr control: 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x",
		data[0], data[1], data[2], data[3], data[4], data[5]);
	rv = ipmicmd_send(domain,
		0x2c, 0x03, 0, IPMI_BMC_CHANNEL, data, 32,
		set_atca_shelf_address_control_states_cb,
		cb_data);
	if (rv != 0) {
		err("ipmicmd_send = 0x%x", rv);
		OHOI_MAP_ERROR(info->rv, rv);
		info->done = 1;
		return;
	}
	return;
}

static SaErrorT set_atca_shelf_address_control_state(
                                      struct oh_handler_state *hnd,
                                      struct ohoi_control_info *c,
                                      SaHpiRdrT * rdr,
                                      SaHpiCtrlModeT mode,
                                      SaHpiCtrlStateT *state)
{
	struct atca_shelf_address_control_s info;
	struct ohoi_handler *ipmi_handler = (struct ohoi_handler *)hnd->data;
	SaErrorT rv;
	SaHpiCtrlStateTextT *text;

	if (mode == SAHPI_CTRL_MODE_AUTO) {
//		|| (state == NULL)) {
//		c->mode = mode;
		return SA_ERR_HPI_READ_ONLY;
	}
	if (state->Type != SAHPI_CTRL_TYPE_TEXT) {
		err("state->Type != SAHPI_CTRL_TYPE_TEXT");
		return SA_ERR_HPI_INVALID_DATA;
	}
	text = &state->StateUnion.Text;
	if ((text->Line != 1) &&
			(text->Line != SAHPI_TLN_ALL_LINES)) {
		err("text->Line != 1 or SAHPI_TLN_ALL_LINES");
		return SA_ERR_HPI_INVALID_DATA;
	}

	info.text = text;
	info.done = 0;
	info.type = rdr->RdrTypeUnion.CtrlRec.TypeUnion.Text.DataType;
	info.rv = SA_OK;
	rv = ipmi_domain_pointer_cb(ipmi_handler->domain_id,
				set_atca_shelf_address_control_states,
				&info);
	if (rv != 0) {
		err("ipmi_domain_pointer_cb = 0x%x", rv);
		return SA_ERR_HPI_INVALID_CMD;
	}
	rv = ohoi_loop(&info.done, ipmi_handler);
	if (rv != SA_OK) {
		err("ohoi_loop = 0x%x", rv);
		return rv;
	}
	if (info.rv != SA_OK) {
		err("info.rv = 0x%x", info.rv);
		return info.rv;
	}
//	c->mode = mode;
	state->Type = SAHPI_CTRL_TYPE_TEXT;
	return SA_OK;
}


static int get_atca_shelf_address_control_states_cb(
			ipmi_domain_t *domain,
			ipmi_msgi_t   *rspi)
{
	struct atca_shelf_address_control_s *info = rspi->data1;
	ipmi_msg_t *msg = &rspi->msg;
	int rv = msg->data[0];
	int len;
	int i;


	dbg("get shelf address response(%d): %02x %02x %02x %02x %02x\n",
		msg->data_len, msg->data[0], msg->data[1], msg->data[2],
		msg->data[3], msg->data[4]);

	if (domain == NULL) {
		info->rv = SA_ERR_HPI_INVALID_PARAMS;
		info->done = 1;
		return IPMI_MSG_ITEM_NOT_USED;
	}
	if (rv == 0xc1) {
		info->rv = SA_ERR_HPI_INVALID_CMD;
	} else if (rv == 0xc3) {
		info->rv = SA_ERR_HPI_NO_RESPONSE;
	} else if (rv != 0) {
		info->rv = SA_ERR_HPI_INVALID_PARAMS;
	}
	if (rv != 0) {
		info->done = 1;
		return IPMI_MSG_ITEM_NOT_USED;
	}

	info->text->Line = 1;
	info->text->Text.DataType = (msg->data[2] & 0xc0) >> 6;
	len = (msg->data[2] & 0x0f);
	memset(&info->text->Text.Data, 0, SAHPI_MAX_TEXT_BUFFER_LENGTH);
	switch (info->text->Text.DataType) {
	case SAHPI_TL_TYPE_UNICODE:
		len = len * 2;
		memcpy(&info->text->Text.Data, msg->data + 3, len);
		break;
	case SAHPI_TL_TYPE_BCDPLUS:
		len = (len + 1) / 2;
		memcpy(&info->text->Text.Data, msg->data + 3, len);
		break;
	case SAHPI_TL_TYPE_ASCII6:
		for (i = 0; i < len; i++) {
			int b, off;
			SaHpiUint8T a = 0;
			b = (6 * i) / 8;
			off = (6 * i) % 8;
			switch (off) {
			case 0: a = (msg->data[3 + b] & 0xFC) >> 2;
				break;
			case 2: a = (msg->data[3 + b] & 0x3F);
				break;
				break;
			case 4: a = ((msg->data[3 + b] & 0x0F) << 2) |
				    ((msg->data[3 + b + 1] & 0xC0) >> 6);
				break;
			case 6: a = ((msg->data[3 + b] & 0x03) << 4) |
				    ((msg->data[3 + b + 1] & 0xF0) >> 4);
			}
			info->text->Text.Data[i] = a + 0x20;
		}
		break;
	case SAHPI_TL_TYPE_TEXT:
		info->text->Text.Language = SAHPI_LANG_ENGLISH;
	case SAHPI_TL_TYPE_BINARY:
		memcpy(&info->text->Text.Data, msg->data + 3, len);
		break;
	}
	info->text->Text.DataLength = len;

	info->done = 1;
	return IPMI_MSG_ITEM_NOT_USED;
};

static void get_atca_shelf_address_control_states(
			ipmi_domain_t *domain,
			void *cb_data)
{
	struct atca_shelf_address_control_s *info = cb_data;
	unsigned char data[32];
	int rv;

	memset(data, 0, 32);
	rv = ipmicmd_send(domain,
		0x2c, 0x02, 0, IPMI_BMC_CHANNEL, data, 1,
		get_atca_shelf_address_control_states_cb,
		cb_data);
	if (rv != 0) {
		err("ipmicmd_send = 0x%x", rv);
		OHOI_MAP_ERROR(info->rv, rv);
		return;
	}
	return;
}

static SaErrorT get_atca_shelf_address_control_state(
                                      struct oh_handler_state *hnd,
                                      struct ohoi_control_info *c,
                                      SaHpiRdrT * rdr,
                                      SaHpiCtrlModeT *mode,
                                      SaHpiCtrlStateT *state)
{
	struct atca_shelf_address_control_s info;
	struct ohoi_handler *ipmi_handler = (struct ohoi_handler *)hnd->data;
	SaErrorT rv;

	if (state == NULL) {
		goto no_state;
	}
#if 0
	if (state->Type != SAHPI_CTRL_TYPE_TEXT) {
		err("state->Type != SAHPI_CTRL_TYPE_TEXT");
		return SA_ERR_HPI_INVALID_DATA;
	}
#endif
	if ((state->StateUnion.Text.Line != 1) &&
			(state->StateUnion.Text.Line != SAHPI_TLN_ALL_LINES)) {
		err("text->Line != 1 or SAHPI_TLN_ALL_LINES");
		return SA_ERR_HPI_INVALID_DATA;
	}

	info.text = &state->StateUnion.Text;
	info.done = 0;
	info.rv = SA_OK;
	rv = ipmi_domain_pointer_cb(ipmi_handler->domain_id,
				get_atca_shelf_address_control_states,
				&info);
	if (rv != 0) {
		err("ipmi_domain_pointer_cb = 0x%x", rv);
		return SA_ERR_HPI_INVALID_CMD;
	}
	rv = ohoi_loop(&info.done, ipmi_handler);
	if (rv != SA_OK) {
		err("ohoi_loop = 0x%x", rv);
		return rv;
	}
	if (info.rv != SA_OK) {
		err("info.rv = 0x%x", info.rv);
		return rv;
	}
	state->Type = SAHPI_CTRL_TYPE_TEXT;
no_state:
	if (mode) {
		*mode = c->mode;
	}
	return SA_OK;
}






static SaHpiRdrT *create_atca_shelf_address_control(
			struct ohoi_handler *ipmi_handler,
			SaHpiRptEntryT *rpt,
			struct ohoi_control_info **ctrl_info)
{
	SaHpiRdrT *rdr;
	struct ohoi_control_info *c_info;
	SaHpiCtrlStateTextT text;
	SaErrorT rv;
	struct atca_shelf_address_control_s info;


	info.text = &text;
	info.done = 0;
	info.rv = SA_OK;
	rv = ipmi_domain_pointer_cb(ipmi_handler->domain_id,
				get_atca_shelf_address_control_states,
				&info);
	if (rv != 0) {
		err("ipmi_domain_pointer_cb = 0x%x", rv);
		return NULL;
	}
	rv = ohoi_loop(&info.done, ipmi_handler);
	if (rv != SA_OK) {
		err("ohoi_loop = 0x%x", rv);
		return NULL;
	}
	if (info.rv != SA_OK) {
		err("info.rv = 0x%x", info.rv);
		return NULL;
	}

	rdr = malloc(sizeof (*rdr));
	if (rdr == NULL) {
		err("Out of memory");
		return NULL;
	}
	c_info = malloc(sizeof (*c_info));
	if (rdr == NULL) {
		err("Out of memory");
		free(rdr);
		return NULL;
	}

	memset(rdr, 0, sizeof (*rdr));
	memset(c_info, 0, sizeof (*c_info));

	rdr->RdrType = SAHPI_CTRL_RDR;
	rdr->IsFru = SAHPI_FALSE;
	rdr->Entity = rpt->ResourceEntity;
	rdr->RdrTypeUnion.CtrlRec.Num = ATCAHPI_CTRL_NUM_SHELF_ADDRESS;
	rdr->RdrTypeUnion.CtrlRec.OutputType = SAHPI_CTRL_GENERIC;
	rdr->RdrTypeUnion.CtrlRec.Type = SAHPI_CTRL_TYPE_TEXT;
	rdr->RdrTypeUnion.CtrlRec.TypeUnion.Text.MaxChars = 25;
	rdr->RdrTypeUnion.CtrlRec.TypeUnion.Text.MaxLines = 1;
	rdr->RdrTypeUnion.CtrlRec.TypeUnion.Text.Language = text.Text.Language;
	rdr->RdrTypeUnion.CtrlRec.TypeUnion.Text.DataType = text.Text.DataType;
	rdr->RdrTypeUnion.CtrlRec.TypeUnion.Text.Default.Line = 1;
	rdr->RdrTypeUnion.CtrlRec.TypeUnion.Text.Default.Text.DataType =
							text.Text.DataType;
	rdr->RdrTypeUnion.CtrlRec.TypeUnion.Text.Default.Text.DataLength =
							text.Text.DataLength;
	rdr->RdrTypeUnion.CtrlRec.TypeUnion.Text.Default.Text.Language =
							text.Text.Language;
	memcpy(&rdr->RdrTypeUnion.CtrlRec.TypeUnion.Text.Default.Text.Data,
		&text.Text.Data, text.Text.DataLength);
	rdr->RdrTypeUnion.CtrlRec.DefaultMode.Mode = SAHPI_CTRL_MODE_MANUAL;
	rdr->RdrTypeUnion.CtrlRec.DefaultMode.ReadOnly = SAHPI_TRUE;
	rdr->RdrTypeUnion.CtrlRec.WriteOnly = SAHPI_FALSE;
	oh_init_textbuffer(&rdr->IdString);
	oh_append_textbuffer(&rdr->IdString, "Shelf Address");

	c_info->ohoii.get_control_state = get_atca_shelf_address_control_state;
	c_info->ohoii.set_control_state = set_atca_shelf_address_control_state;
	c_info->mode = SAHPI_CTRL_MODE_MANUAL;
	*ctrl_info = c_info;

	return rdr;
}



	/*
         *      Shelf IP Address Control
	 */

static SaErrorT get_atca_shelf_ip_address_control_state(
                                      struct oh_handler_state *hnd,
                                      struct ohoi_control_info *c,
                                      SaHpiRdrT * rdr,
                                      SaHpiCtrlModeT *mode,
                                      SaHpiCtrlStateT *state);

static SaErrorT set_atca_shelf_ip_address_control_state(
                                      struct oh_handler_state *hnd,
                                      struct ohoi_control_info *c,
                                      SaHpiRdrT * rdr,
                                      SaHpiCtrlModeT mode,
                                      SaHpiCtrlStateT *state);



static SaHpiRdrT *create_atca_shelf_ip_address_control(
			struct oh_handler_state *handler,
			SaHpiRptEntryT *rpt,
			struct ohoi_control_info **ctrl_info)
{
//	struct ohoi_handler *ipmi_handler = handler->data;
	SaHpiRdrT *rdr;
	struct ohoi_control_info *c_info;



	rdr = malloc(sizeof (*rdr));
	if (rdr == NULL) {
		err("Out of memory");
		return NULL;
	}
	c_info = malloc(sizeof (*c_info));
	if (rdr == NULL) {
		err("Out of memory");
		free(rdr);
		return NULL;
	}

	memset(rdr, 0, sizeof (*rdr));
	memset(c_info, 0, sizeof (*c_info));

	rdr->RdrType = SAHPI_CTRL_RDR;
	rdr->IsFru = SAHPI_FALSE;
	rdr->Entity = rpt->ResourceEntity;
	rdr->RdrTypeUnion.CtrlRec.Num = ATCAHPI_CTRL_NUM_SHELF_IP_ADDRESS;
	rdr->RdrTypeUnion.CtrlRec.OutputType = SAHPI_CTRL_GENERIC;
	rdr->RdrTypeUnion.CtrlRec.Type = SAHPI_CTRL_TYPE_TEXT;
	rdr->RdrTypeUnion.CtrlRec.TypeUnion.Text.MaxChars = 4;
	rdr->RdrTypeUnion.CtrlRec.TypeUnion.Text.MaxLines = 3;
	rdr->RdrTypeUnion.CtrlRec.TypeUnion.Text.Language = SAHPI_LANG_UNDEF;
	rdr->RdrTypeUnion.CtrlRec.TypeUnion.Text.DataType = SAHPI_TL_TYPE_BINARY;
	rdr->RdrTypeUnion.CtrlRec.TypeUnion.Text.Default.Line =
						SAHPI_TLN_ALL_LINES;
	rdr->RdrTypeUnion.CtrlRec.TypeUnion.Text.Default.Text.DataType =
							SAHPI_TL_TYPE_BINARY;
	rdr->RdrTypeUnion.CtrlRec.TypeUnion.Text.Default.Text.DataLength = 12;
	rdr->RdrTypeUnion.CtrlRec.TypeUnion.Text.Default.Text.Language =
							     SAHPI_LANG_UNDEF;
	memset(&rdr->RdrTypeUnion.CtrlRec.TypeUnion.Text.Default.Text.Data,
					0, SAHPI_MAX_TEXT_BUFFER_LENGTH);
	rdr->RdrTypeUnion.CtrlRec.DefaultMode.Mode = SAHPI_CTRL_MODE_MANUAL;
	rdr->RdrTypeUnion.CtrlRec.DefaultMode.ReadOnly = SAHPI_TRUE;
	rdr->RdrTypeUnion.CtrlRec.WriteOnly = SAHPI_FALSE;
	oh_init_textbuffer(&rdr->IdString);
	oh_append_textbuffer(&rdr->IdString, "Shelf IP Address");

	c_info->info.atcamap_ctrl_info.data = NULL;
	c_info->type = OHOI_CTRL_ATCA_MAPPED;
	c_info->mode = SAHPI_CTRL_MODE_MANUAL;

	c_info->ohoii.get_control_state = get_atca_shelf_ip_address_control_state;
	c_info->ohoii.set_control_state = set_atca_shelf_ip_address_control_state;
	*ctrl_info = c_info;

	return rdr;
}

static SaErrorT get_shelf_ip_address_record(ipmi_entity_t *ent,
                                   unsigned char *buf,
				   unsigned int *len,
				   unsigned char *ver,
				   unsigned int *num)
{
	unsigned char type, vr;
	unsigned int i, num_rec;
	unsigned int orig_len = *len;
	int rv;

	num_rec = ipmi_entity_get_num_multi_records(ent);
	for (i = 0; i < num_rec; i++) {
		*len = orig_len;
		rv = ipmi_entity_get_multi_record_data(ent, i, buf, len);
		if (rv != 0) {
			err("ipmi_entity_get_multi_record_data(%d) = 0x%x",
						i, rv);
			return SA_ERR_HPI_INVALID_DATA;
		}
		if (*len < 17) {
			continue;
		}
		rv = ipmi_entity_get_multi_record_type(ent, i, &type);
		if (rv) {
			err("ipmi_entity_get_multi_record_type = %d", rv);
			continue;
		}
		if (type != 0xc0) {
			// record type. Must be OEM
			continue;
		}
		rv = ipmi_entity_get_multi_record_format_version(ent, i, &vr);
		if (rv) {
			err("ipmi_entity_get_multi_record_format_version = %d", rv);
			continue;
		}

		if ((vr & 0x0f) != 0x2) {
			// must be 2 for PICMG 3.0 ATCA vD1.0
			continue;
		}
		// checksums are checked by OpenIPMI
		if ((buf[0] | (buf[1] << 8) | (buf[2] << 16)) !=
						ATCAHPI_PICMG_MID) {
			continue;
		}
		if (buf[3] != 0x13) {
			continue;
		}
		if (buf[4] != 0x01) {
			continue;
		}
		break;
	}
	if (i == num_rec) {
		err("No record for shelf IP address");
		return SA_ERR_HPI_INTERNAL_ERROR;
	}
	*num = i;
	*ver = vr;
	return SA_OK;
}


struct atca_shelf_ip_address_control_state {
	struct oh_handler_state *hnd;
	SaErrorT rv;
	SaHpiCtrlStateTextT		*text;
	int done;
};

static void get_atca_shelf_ip_address_control_state_cb(ipmi_entity_t *ent,
                                                void *cb_data)
{
	struct atca_shelf_ip_address_control_state *info = cb_data;
	unsigned char buf[256];
	unsigned int len = 256;
	unsigned int num_rec;
	unsigned char ver;



	info->rv = get_shelf_ip_address_record(ent, buf, &len, &ver, &num_rec);
	if (info->rv != SA_OK) {
		return;
	}


	info->text->Text.DataType = SAHPI_TL_TYPE_BINARY;
	info->text->Text.Language = SAHPI_LANG_UNDEF;
	switch (info->text->Line) {
	case 1:		// Shelf Manager IP Address
		info->text->Text.DataLength = 4;
		memcpy(info->text->Text.Data, buf + 5, 4);
		break;
	case 2:		// Default Gateway IP Address
		info->text->Text.DataLength = 4;
		memcpy(info->text->Text.Data, buf + 9, 4);
		break;
	case 3:		// Subnet Mask
		info->text->Text.DataLength = 4;
		memcpy(info->text->Text.Data, buf + 13, 4);
		break;
	case SAHPI_TLN_ALL_LINES:		// all
		info->text->Text.DataLength = 12;
		memcpy(info->text->Text.Data, buf + 5, 12);
		break;
	default :
		err("wrong text->Line = %d", info->text->Line);
		info->rv = SA_ERR_HPI_INVALID_DATA;
		break;
	}
}




SaErrorT get_atca_shelf_ip_address_control_state(
                                      struct oh_handler_state *handler,
                                      struct ohoi_control_info *c,
                                      SaHpiRdrT * rdr,
                                      SaHpiCtrlModeT *mode,
                                      SaHpiCtrlStateT *state)
{
	struct ohoi_resource_info *res_info;
	struct ohoi_handler *ohoi_handler = handler->data;
	int rv;
	struct atca_shelf_ip_address_control_state info;

	if (state == NULL) {
		goto no_state;
	}
#if 0
	if (state->Type != SAHPI_CTRL_TYPE_TEXT) {
		err("wrong state type %d", state->Type);
		return SA_ERR_HPI_INVALID_DATA;
	}
#endif
	res_info = oh_get_resource_data(handler->rptcache,
					ohoi_handler->atca_shelf_id);
	if (res_info == NULL) {
		err("No shelf resource info?");
		return SA_ERR_HPI_INTERNAL_ERROR;
	}
	if (res_info->fru == NULL) {
		err("Shelf does not have IDR");
		return SA_ERR_HPI_INVALID_DATA;
	}

	info.rv = SA_OK;
	info.text = &state->StateUnion.Text;

	g_mutex_lock(res_info->fru->mutex);
	rv = ipmi_entity_pointer_cb(res_info->u.entity.entity_id,
		get_atca_shelf_ip_address_control_state_cb, &info);
	if (rv != 0) {
		err("ipmi_entity_pointer_cb = 0x%x", rv);
		g_mutex_unlock(res_info->fru->mutex);
		return SA_ERR_HPI_INTERNAL_ERROR;
	}
	g_mutex_unlock(res_info->fru->mutex);
	if (info.rv != SA_OK) {
		return info.rv;
	}
no_state:
	if (mode) {
		*mode = c->mode;
	}
	return SA_OK;
}




static void set_atca_shelf_ip_address_control_state_cb(ipmi_entity_t *ent,
                                                void *cb_data)
{
	struct atca_shelf_ip_address_control_state *info = cb_data;
	unsigned char buf[256];
	unsigned int len = 256;
	unsigned int num_rec;
	unsigned char ver;
	SaErrorT rv;
	ipmi_fru_t *fru = ipmi_entity_get_fru(ent);
//	struct ohoi_handler *ohoi_handler = info->hnd->data;

	info->rv = get_shelf_ip_address_record(ent, buf, &len, &ver, &num_rec);
	if (info->rv != SA_OK) {
		info->done = 1;
		return;
	}
	switch (info->text->Line) {
	case 1:		// Shelf Manager IP Address
		memcpy(buf + 5, info->text->Text.Data, 4);
		break;
	case 2:		// Default Gateway IP Address
		memcpy(buf + 9, info->text->Text.Data, 4);
		break;
	case 3:		// Subnet Mask
		memcpy(buf + 13, info->text->Text.Data, 4);
		break;
	case SAHPI_TLN_ALL_LINES:		// all
		memcpy(buf + 5, info->text->Text.Data, 12);
		break;
	default :
		err("wrong text->Line = %d", info->text->Line);
		info->rv = SA_ERR_HPI_INVALID_DATA;
		info->done = 1;
		return;
	}

	rv = ipmi_fru_set_multi_record(fru, num_rec, 0xC0, ver, buf, len);
	if (rv != 0) {
		err("ipmi_fru_set_multi_record(fru, %d, 0xC0, 0x%x, buf, 0x%x",
				num_rec,  ver, len);
		info->rv = SA_ERR_HPI_INTERNAL_ERROR;
	}
	info->done = 1;
}

SaErrorT set_atca_shelf_ip_address_control_state(
                                      struct oh_handler_state *handler,
                                      struct ohoi_control_info *c,
                                      SaHpiRdrT * rdr,
                                      SaHpiCtrlModeT mode,
                                      SaHpiCtrlStateT *state)
{
	struct ohoi_resource_info *res_info;
	struct ohoi_handler *ohoi_handler = handler->data;
	SaHpiCtrlStateTextT	*text;
	SaHpiCtrlRecTextT	*ctrl;
	int rv;
	struct atca_shelf_ip_address_control_state info;

	if (mode == SAHPI_CTRL_MODE_AUTO) {
//		c->mode = mode;
		return SA_ERR_HPI_READ_ONLY;
	}
	if (state->Type != SAHPI_CTRL_TYPE_TEXT) {
		err("wrong state type %d", state->Type);
		return SA_ERR_HPI_INVALID_DATA;
	}
	res_info = oh_get_resource_data(handler->rptcache,
					ohoi_handler->atca_shelf_id);
	if (res_info == NULL) {
		err("No shelf resource info?");
		return SA_ERR_HPI_INTERNAL_ERROR;
	}
	if (res_info->fru == NULL) {
		err("Shelf does not have IDR");
		return SA_ERR_HPI_INVALID_DATA;
	}
	ctrl = &rdr->RdrTypeUnion.CtrlRec.TypeUnion.Text;
	text = &state->StateUnion.Text;
	if (text->Text.DataType != SAHPI_TL_TYPE_BINARY) {
		err("wrong DataType = %d", text->Text.DataType);
		return SA_ERR_HPI_INVALID_DATA;
	}
	if (text->Line == SAHPI_TLN_ALL_LINES) {
		if (text->Text.DataLength != 12) {
			err("wrong DataLength = %d", text->Text.DataLength);
			return SA_ERR_HPI_INVALID_DATA;
		}
	} else if (text->Line > ctrl->MaxLines) {
		err("wrong text->Line = %d", text->Line);
		return SA_ERR_HPI_INVALID_DATA;
	} else if (text->Text.DataLength != 4) {
		err("wrong DataLength = %d", text->Text.DataLength);
		return SA_ERR_HPI_INVALID_DATA;
	}

	info.hnd = handler;
	info.text = text;
	info.done = 0;
	info.rv = SA_OK;

	g_mutex_lock(res_info->fru->mutex);
	rv = ipmi_entity_pointer_cb(res_info->u.entity.entity_id,
		set_atca_shelf_ip_address_control_state_cb, &info);
	if (rv != 0) {
		err("ipmi_entity_pointer_cb = 0x%x", rv);
		g_mutex_unlock(res_info->fru->mutex);
		return SA_ERR_HPI_INTERNAL_ERROR;
	}
	rv = ohoi_loop(&info.done, ohoi_handler);
	g_mutex_unlock(res_info->fru->mutex);
	if (rv != SA_OK) {
		return rv;
	}
	if (info.rv != SA_OK) {
		return info.rv;
	}
	rv = ohoi_fru_write(ohoi_handler, res_info->u.entity.entity_id);
//	if (rv == SA_OK) {
//		c->mode = mode;
//	}
	return rv;
}








	/*
         *      FRU Power On Sequence Commit Status Sensor
	 */



static void send_pwronseq_commit_status_sensor_event(
                                      struct oh_handler_state *handler,
				      int updated)
{
	struct ohoi_handler *ipmi_handler = handler->data;
        SaErrorT         rv;
	struct ohoi_sensor_info *s_info = NULL;
        struct oh_event		*e;
        SaHpiSensorEventT	*sen_evt;

	rv = ohoi_get_rdr_data(handler, ipmi_handler->atca_shelf_id,
		SAHPI_SENSOR_RDR, ATCAHPI_SENSOR_NUM_PWRONSEQ_COMMIT_STATUS,
                (void *)&s_info);
	if (rv != SA_OK) {
		err("could not get sensor info");
		return;
	}

	if (s_info == NULL) {
		err("could not get sensor info");
		return;
	}
	if (s_info->sen_enabled == SAHPI_FALSE) {
		err("sensor disabled");
		return;
	}
	if (!s_info->info.atcamap_sensor_info.val) {
		// sensor event disable
		err("sensor event disabled");
		return;
	}
	if (updated && !(s_info->assert & SAHPI_ES_STATE_01)) {
		err("SAHPI_ES_STATE_01 disabled");
		return;
	}
	if (!updated && !(s_info->assert & SAHPI_ES_STATE_00)) {
		err("SAHPI_ES_STATE_00 disabled");
		return;
	}
        e = malloc(sizeof(*e));
        if (!e) {
                err("Out of space");
                return;
        }

        SaHpiRdrT *rdr = oh_get_rdr_by_type(handler->rptcache,
        				    ipmi_handler->atca_shelf_id,
        				    SAHPI_SENSOR_RDR,
        				    ATCAHPI_SENSOR_NUM_PWRONSEQ_COMMIT_STATUS);

        memset(e, 0, sizeof(*e));
        e->event.Source = ipmi_handler->atca_shelf_id;
        e->event.EventType = SAHPI_ET_SENSOR;
        e->event.Severity = SAHPI_INFORMATIONAL;
        oh_gettimeofday(&e->event.Timestamp);

        sen_evt = &(e->event.EventDataUnion.SensorEvent);
        sen_evt->SensorNum = ATCAHPI_SENSOR_NUM_PWRONSEQ_COMMIT_STATUS;
        sen_evt->SensorType = SAHPI_OEM_SENSOR;
        sen_evt->EventCategory = SAHPI_EC_SENSOR_SPECIFIC;
        sen_evt->Assertion = SAHPI_TRUE;
        sen_evt->EventState = updated ? SAHPI_ES_STATE_01 : SAHPI_ES_STATE_00;
        sen_evt->OptionalDataPresent = SAHPI_SOD_PREVIOUS_STATE |
					 SAHPI_SOD_CURRENT_STATE;
        sen_evt->CurrentState = updated ? SAHPI_ES_STATE_01 : SAHPI_ES_STATE_00;
        sen_evt->PreviousState = updated ? SAHPI_ES_STATE_00 : SAHPI_ES_STATE_01;

        if (rdr) e->rdrs = g_slist_append(e->rdrs, g_memdup(rdr, sizeof(SaHpiRdrT)));

        e->hid = handler->hid;
        oh_evt_queue_push(handler->eventq, e);
}

static SaErrorT get_pwronseq_commit_status_sensor_event_enable(
					    struct oh_handler_state *hnd,
					    struct ohoi_sensor_info *sinfo,
					    SaHpiBoolT   *enable,
					    SaHpiEventStateT  *assert,
					    SaHpiEventStateT  *deassert)
{
	*assert = sinfo->assert;
	*deassert = 0;
	*enable = sinfo->info.atcamap_sensor_info.val;
	return SA_OK;
}


static SaErrorT set_pwronseq_commit_status_sensor_event_enable(
					    struct oh_handler_state *hnd,
					    struct ohoi_sensor_info *sinfo,
					    SaHpiBoolT enable,
					    SaHpiEventStateT assert,
					    SaHpiEventStateT deassert,
					    unsigned int a_supported,
					    unsigned int d_supported)
{

	if (deassert != 0) {
		err("deassert(0x%x) != 0", deassert);
		return SA_ERR_HPI_INVALID_DATA;
	}
	if ((assert & ~(SAHPI_ES_STATE_00 | SAHPI_ES_STATE_01))) {
		err("assert(0x%x)", assert);
		return SA_ERR_HPI_INVALID_DATA;
	}
	sinfo->assert = assert;
	sinfo->info.atcamap_sensor_info.val = enable;

	return SA_OK;
}


static SaErrorT get_pwronseq_commit_status_sensor_reading(
				       struct oh_handler_state *hnd,
				       struct ohoi_sensor_info *sensor_info,
				       SaHpiSensorReadingT *reading,
				       SaHpiEventStateT *ev_state)
{
	struct ohoi_handler *ipmi_handler = hnd->data;
	if (reading != NULL) {
		reading->IsSupported = SAHPI_FALSE;
	}
	if (ev_state) {
		if (ipmi_handler->atca_pwonseq_updated) {
			*ev_state = SAHPI_ES_STATE_01;
		} else {
			*ev_state = SAHPI_ES_STATE_00;
		}
	}

	return SA_OK;
}


static SaErrorT get_pwronseq_commit_status_sensor_thresholds(
					  struct oh_handler_state *hnd,
					  struct ohoi_sensor_info *sinfo,
					  SaHpiSensorThresholdsT *thres)
{
	return SA_ERR_HPI_INVALID_CMD;
}


static SaErrorT set_pwronseq_commit_status_sensor_thresholds(
					  struct oh_handler_state *hnd,
					  struct ohoi_sensor_info *sinfo,
					  const SaHpiSensorThresholdsT *thres)
{
	return SA_ERR_HPI_INVALID_CMD;
}





static SaHpiRdrT *create_fru_power_on_sequence_commit_status_sensor(
			struct oh_handler_state *handler,
			SaHpiRptEntryT *rpt,
			struct ohoi_sensor_info **sensor_info)
{
//	struct ohoi_handler *ipmi_handler = handler->data;
	SaHpiRdrT *rdr;
	struct ohoi_sensor_info *s_info;



	rdr = malloc(sizeof (*rdr));
	if (rdr == NULL) {
		err("Out of memory");
		return NULL;
	}
	s_info = malloc(sizeof (*s_info));
	if (rdr == NULL) {
		err("Out of memory");
		free(rdr);
		return NULL;
	}

	memset(rdr, 0, sizeof (*rdr));
	memset(s_info, 0, sizeof (*s_info));

	rdr->RdrType = SAHPI_SENSOR_RDR;
	rdr->IsFru = SAHPI_FALSE;
	rdr->Entity = rpt->ResourceEntity;
	rdr->RdrTypeUnion.SensorRec.Num =
			ATCAHPI_SENSOR_NUM_PWRONSEQ_COMMIT_STATUS;
	rdr->RdrTypeUnion.SensorRec.Type = SAHPI_OEM_SENSOR;
	rdr->RdrTypeUnion.SensorRec.Category = SAHPI_EC_SENSOR_SPECIFIC;
	rdr->RdrTypeUnion.SensorRec.EnableCtrl = SAHPI_TRUE;
	rdr->RdrTypeUnion.SensorRec.EventCtrl = SAHPI_SEC_PER_EVENT;
	rdr->RdrTypeUnion.SensorRec.Events = SAHPI_ES_STATE_00 |
							SAHPI_ES_STATE_01;
	rdr->RdrTypeUnion.SensorRec.DataFormat.IsSupported = SAHPI_FALSE;
	rdr->RdrTypeUnion.SensorRec.ThresholdDefn.IsAccessible = SAHPI_FALSE;
	oh_init_textbuffer(&rdr->IdString);
	oh_append_textbuffer(&rdr->IdString,
				"FRU Power On Sequence Commit Status");

	s_info->support_assert = SAHPI_ES_STATE_00 | SAHPI_ES_STATE_01;
	s_info->support_deassert = 0;
	s_info->assert = SAHPI_ES_STATE_00 | SAHPI_ES_STATE_01;
	s_info->deassert = 0;
	s_info->sen_enabled = SAHPI_TRUE;
        s_info->enable = SAHPI_TRUE;
	s_info->info.atcamap_sensor_info.data = NULL;
	s_info->info.atcamap_sensor_info.val = SAHPI_TRUE;
	s_info->type = OHOI_SENSOR_ATCA_MAPPED;

	s_info->ohoii.get_sensor_event_enable =
		get_pwronseq_commit_status_sensor_event_enable;
	s_info->ohoii.set_sensor_event_enable =
		set_pwronseq_commit_status_sensor_event_enable;
	s_info->ohoii.get_sensor_reading =
		get_pwronseq_commit_status_sensor_reading;
	s_info->ohoii.get_sensor_thresholds =
		get_pwronseq_commit_status_sensor_thresholds;
	s_info->ohoii.set_sensor_thresholds =
		set_pwronseq_commit_status_sensor_thresholds;

	*sensor_info = s_info;

	return rdr;
}




	/*
         *      FRU Power On Sequence Controls
	 */

static SaErrorT get_atca_fru_pwronseq_control_state(
                                      struct oh_handler_state *hnd,
                                      struct ohoi_control_info *c,
                                      SaHpiRdrT * rdr,
                                      SaHpiCtrlModeT *mode,
                                      SaHpiCtrlStateT *state);


static SaErrorT set_atca_fru_pwronseq_control_state(
                                      struct oh_handler_state *hnd,
                                      struct ohoi_control_info *c,
                                      SaHpiRdrT * rdr,
                                      SaHpiCtrlModeT mode,
                                      SaHpiCtrlStateT *state);





static SaHpiRdrT *create_fru_power_on_sequence_control(
			struct oh_handler_state *handler,
			SaHpiRptEntryT *rpt,
			struct ohoi_control_info **ctrl_info,
			int num,
			SaHpiResourceIdT slotid)
{
//	struct ohoi_handler *ipmi_handler = handler->data;
	SaHpiRdrT *rdr;
	struct ohoi_control_info *c_info;
	unsigned char buf[32];


	rdr = malloc(sizeof (*rdr));
	if (rdr == NULL) {
		err("Out of memory");
		return NULL;
	}
	c_info = malloc(sizeof (*c_info));
	if (rdr == NULL) {
		err("Out of memory");
		free(rdr);
		return NULL;
	}

	memset(rdr, 0, sizeof (*rdr));
	memset(c_info, 0, sizeof (*c_info));

	rdr->RdrType = SAHPI_CTRL_RDR;
	rdr->IsFru = SAHPI_FALSE;
	rdr->Entity = rpt->ResourceEntity;
	rdr->RdrTypeUnion.CtrlRec.Num = ATCAHPI_CTRL_NUM_FRU_POWER_ON_SEQUENCE
						+ num;
	rdr->RdrTypeUnion.CtrlRec.OutputType = SAHPI_CTRL_GENERIC;
	rdr->RdrTypeUnion.CtrlRec.Type = SAHPI_CTRL_TYPE_DISCRETE;
	rdr->RdrTypeUnion.CtrlRec.TypeUnion.Discrete.Default = slotid;
	rdr->RdrTypeUnion.CtrlRec.DefaultMode.Mode = SAHPI_CTRL_MODE_MANUAL;
	rdr->RdrTypeUnion.CtrlRec.DefaultMode.ReadOnly = SAHPI_TRUE;
	rdr->RdrTypeUnion.CtrlRec.WriteOnly = SAHPI_FALSE;
	oh_init_textbuffer(&rdr->IdString);
	snprintf((char *)buf, 32, "FRU Power On Sequence #%d", num);
	oh_append_textbuffer(&rdr->IdString, (void *)buf);

	c_info->info.atcamap_ctrl_info.data = NULL;
	c_info->type = OHOI_CTRL_ATCA_MAPPED;
	c_info->mode = SAHPI_CTRL_MODE_MANUAL;

	c_info->ohoii.get_control_state = get_atca_fru_pwronseq_control_state;
	c_info->ohoii.set_control_state = set_atca_fru_pwronseq_control_state;
	*ctrl_info = c_info;

	return rdr;
}



static SaErrorT get_atca_fru_pwronseq_control_state(
                                      struct oh_handler_state *hnd,
                                      struct ohoi_control_info *c,
                                      SaHpiRdrT * rdr,
                                      SaHpiCtrlModeT *mode,
                                      SaHpiCtrlStateT *state)
{
	struct ohoi_handler *ipmi_handler = hnd->data;
	ohoi_atca_pwonseq_dsk_t *dsk;
	GSList *node;
	int num = rdr->RdrTypeUnion.CtrlRec.Num -
			ATCAHPI_CTRL_NUM_FRU_POWER_ON_SEQUENCE;
	if (state == NULL) {
		goto no_state;
	}
	if ((num < 0) || (num >= g_slist_length(
				ipmi_handler->atca_pwonseq_desk))) {
		err("wrong dsk number %d", num);
		return SA_ERR_HPI_INTERNAL_ERROR;
	}

	state->Type = SAHPI_CTRL_TYPE_DISCRETE;
	g_static_rec_mutex_lock(&ipmi_handler->ohoih_lock);
	node = g_slist_nth(ipmi_handler->atca_pwonseq_desk, num);
	if (node == NULL) {
		g_static_rec_mutex_unlock(&ipmi_handler->ohoih_lock);
		err("No pw on descriptor #%d", num);
		return SA_ERR_HPI_INTERNAL_ERROR;
	}
	dsk = (ohoi_atca_pwonseq_dsk_t *)node->data;
	state->StateUnion.Discrete = dsk->slotid;
	g_static_rec_mutex_unlock(&ipmi_handler->ohoih_lock);
no_state :
	if (mode) {
		*mode = c->mode;
	}
	return SA_OK;
}


static SaErrorT set_atca_fru_pwronseq_control_state(
                                      struct oh_handler_state *handler,
                                      struct ohoi_control_info *c,
                                      SaHpiRdrT * rdr,
                                      SaHpiCtrlModeT mode,
                                      SaHpiCtrlStateT *state)
{
	struct ohoi_resource_info *res_info;
	struct ohoi_handler *ipmi_handler = handler->data;
	SaHpiResourceIdT	slotid;
	SaHpiCtrlRecT	*ctrl;
//	GSList *node;
	ohoi_atca_pwonseq_dsk_t *dscp = NULL;
	ohoi_atca_pwonseq_rec_t *recp = NULL;
	int oldid;
	int newid;
	int minid, maxid;
	int beg, len;
	int i;


	if (mode == SAHPI_CTRL_MODE_AUTO) {
		c->mode = mode;
		return SA_OK;
	}
	if (state->Type != SAHPI_CTRL_TYPE_DISCRETE) {
		err("wrong state type %d", state->Type);
		return SA_ERR_HPI_INVALID_DATA;
	}
	slotid = state->StateUnion.Discrete;

	g_static_rec_mutex_lock(&ipmi_handler->ohoih_lock);
	res_info = oh_get_resource_data(handler->rptcache,
					ipmi_handler->atca_shelf_id);
	if (res_info == NULL) {
		err("No shelf resource info?");
		g_static_rec_mutex_unlock(&ipmi_handler->ohoih_lock);
		return SA_ERR_HPI_INTERNAL_ERROR;
	}
	if (res_info->fru == NULL) {
		g_static_rec_mutex_unlock(&ipmi_handler->ohoih_lock);
		err("Shelf does not have IDR");
		return SA_ERR_HPI_INVALID_DATA;
	}
	ctrl = &rdr->RdrTypeUnion.CtrlRec;

	// find out descriptor with required slotid
	oldid = 0;
	for (oldid = 0; oldid < g_slist_length(
				ipmi_handler->atca_pwonseq_desk); oldid++) {
		dscp = (ohoi_atca_pwonseq_dsk_t *)g_slist_nth_data(
					ipmi_handler->atca_pwonseq_desk, oldid);
		if (dscp == NULL) {
			break;
		}
		if (dscp->slotid == slotid) {
			break;
		}
	}
	if (dscp == NULL) {
		g_static_rec_mutex_unlock(&ipmi_handler->ohoih_lock);
		err("No descriptor for slotid %d", slotid);
		return SA_ERR_HPI_INVALID_PARAMS;
	}

	newid = ctrl->Num - ATCAHPI_CTRL_NUM_FRU_POWER_ON_SEQUENCE;
	c->mode = mode;
	if (newid == oldid) {
		// nothing to do
		g_static_rec_mutex_unlock(&ipmi_handler->ohoih_lock);
		return SA_OK;
	}

	// Rearrange descriptor
	ipmi_handler->atca_pwonseq_desk = g_slist_remove(
				ipmi_handler->atca_pwonseq_desk, dscp);
	ipmi_handler->atca_pwonseq_desk = g_slist_insert(
		ipmi_handler->atca_pwonseq_desk, dscp, newid);

	// Which records are affected and mark them as updated
	oldid--;
	maxid = (newid > oldid) ? newid : oldid;
	minid = (newid < oldid) ? newid : oldid;
	beg = 0;
	len = 0;
	for (i = 0; i < g_slist_length(ipmi_handler->atca_pwonseq_recs); i++) {
		recp = g_slist_nth_data(ipmi_handler->atca_pwonseq_recs, i);
		if (recp == NULL) {
			break;
		}
		beg += len;
		len = recp->head[6];
		if ((beg <= maxid) && (beg + len >= minid)) {
			recp->updated = 1;
		}
	}

	if (ipmi_handler->atca_pwonseq_updated) {
		// was updated before. nothing to do more
		g_static_rec_mutex_unlock(&ipmi_handler->ohoih_lock);
		return SA_OK;
	}
	ipmi_handler->atca_pwonseq_updated = 1;

	send_pwronseq_commit_status_sensor_event(handler, 1);

	return SA_OK;
}





	/*
         *      FRU Power On Sequence Commit Control
	 */

static SaErrorT get_atca_fru_pwronseq_commit_control_state(
                                      struct oh_handler_state *hnd,
                                      struct ohoi_control_info *c,
                                      SaHpiRdrT * rdr,
                                      SaHpiCtrlModeT *mode,
                                      SaHpiCtrlStateT *state);

static SaErrorT set_atca_fru_pwronseq_commit_control_state(
                                      struct oh_handler_state *hnd,
                                      struct ohoi_control_info *c,
                                      SaHpiRdrT * rdr,
                                      SaHpiCtrlModeT mode,
                                      SaHpiCtrlStateT *state);



static SaHpiRdrT *create_fru_power_on_sequence_commit_control(
			struct oh_handler_state *handler,
			SaHpiRptEntryT *rpt,
			struct ohoi_control_info **ctrl_info)
{
//	struct ohoi_handler *ipmi_handler = handler->data;
	SaHpiRdrT *rdr;
	struct ohoi_control_info *c_info;



	rdr = malloc(sizeof (*rdr));
	if (rdr == NULL) {
		err("Out of memory");
		return NULL;
	}
	c_info = malloc(sizeof (*c_info));
	if (rdr == NULL) {
		err("Out of memory");
		free(rdr);
		return NULL;
	}

	memset(rdr, 0, sizeof (*rdr));
	memset(c_info, 0, sizeof (*c_info));

	rdr->RdrType = SAHPI_CTRL_RDR;
	rdr->IsFru = SAHPI_FALSE;
	rdr->Entity = rpt->ResourceEntity;
	rdr->RdrTypeUnion.CtrlRec.Num =
			ATCAHPI_CTRL_NUM_FRU_POWER_ON_SEQUENCE_COMMIT;
	rdr->RdrTypeUnion.CtrlRec.OutputType = SAHPI_CTRL_GENERIC;
	rdr->RdrTypeUnion.CtrlRec.Type = SAHPI_CTRL_TYPE_DIGITAL;
	rdr->RdrTypeUnion.CtrlRec.TypeUnion.Digital.Default =
						SAHPI_CTRL_STATE_OFF;
	rdr->RdrTypeUnion.CtrlRec.DefaultMode.Mode = SAHPI_CTRL_MODE_MANUAL;
	rdr->RdrTypeUnion.CtrlRec.DefaultMode.ReadOnly = SAHPI_TRUE;
	rdr->RdrTypeUnion.CtrlRec.WriteOnly = SAHPI_FALSE;
	oh_init_textbuffer(&rdr->IdString);
	oh_append_textbuffer(&rdr->IdString, "FRU Power On Sequence Commit");

	c_info->info.atcamap_ctrl_info.data = NULL;
	c_info->type = OHOI_CTRL_ATCA_MAPPED;
	c_info->mode = SAHPI_CTRL_MODE_MANUAL;

	c_info->ohoii.get_control_state =
				get_atca_fru_pwronseq_commit_control_state;
	c_info->ohoii.set_control_state =
				set_atca_fru_pwronseq_commit_control_state;
	*ctrl_info = c_info;

	return rdr;
}



static SaErrorT get_atca_fru_pwronseq_commit_control_state(
                                      struct oh_handler_state *hnd,
                                      struct ohoi_control_info *c,
                                      SaHpiRdrT * rdr,
                                      SaHpiCtrlModeT *mode,
                                      SaHpiCtrlStateT *state)
{
	if (state) {
		state->Type = SAHPI_CTRL_TYPE_DIGITAL;
		state->StateUnion.Digital = SAHPI_CTRL_STATE_OFF;
	}
	if (mode) {
		*mode = c->mode;
	}
	return SA_OK;
}


struct fru_pwronseq_commit_control_s {
	struct ohoi_handler *ipmi_handler;
	unsigned char *buf;
	unsigned int len;
	unsigned int num;
	SaErrorT rv;
};


static void write_power_on_sequence_data_cb(ipmi_entity_t *ent, void *cb_data)
{
	struct fru_pwronseq_commit_control_s *info = cb_data;
	ipmi_fru_t *fru = ipmi_entity_get_fru(ent);
	int rv;

	rv = ipmi_fru_set_multi_record(fru, info->num, 0xC0, 0x0,
							info->buf, info->len);
	if (rv != 0) {
		err("ipmi_fru_set_multi_record(fru, %d, 0xC0, 0x0, buf, %d)"
			" = %d", info->num, info->len, rv);
		info->rv = SA_ERR_HPI_INTERNAL_ERROR;
	}
}


static SaErrorT set_atca_fru_pwronseq_commit_control_state(
                                      struct oh_handler_state *hnd,
                                      struct ohoi_control_info *c,
                                      SaHpiRdrT * rdr,
                                      SaHpiCtrlModeT mode,
                                      SaHpiCtrlStateT *state)
{
	SaHpiCtrlStateDiscreteT value;
	struct ohoi_handler *ipmi_handler = hnd->data;
	struct ohoi_resource_info *res_info;
//	GSList *dnode, *rnode;
	ohoi_atca_pwonseq_rec_t *recp;
	ohoi_atca_pwonseq_dsk_t *dscp;
	int i, j, di;
	unsigned int num;
	unsigned char buf[256];
	unsigned int len;
	struct fru_pwronseq_commit_control_s info;

	if (mode == SAHPI_CTRL_MODE_AUTO) {
		return SA_ERR_HPI_READ_ONLY;
	}

	value = state->StateUnion.Digital;

	if (value != SAHPI_CTRL_STATE_PULSE_ON) {
		err("wrong discrete value %d", value);
		return SA_ERR_HPI_INVALID_REQUEST;
	}

	g_static_rec_mutex_lock(&ipmi_handler->ohoih_lock);
	if (!ipmi_handler->atca_pwonseq_updated) {
		g_static_rec_mutex_unlock(&ipmi_handler->ohoih_lock);
		return SA_OK;
	}

	res_info = oh_get_resource_data(hnd->rptcache,
					ipmi_handler->atca_shelf_id);
	if ((value == SAHPI_CTRL_STATE_PULSE_OFF) ||
			(value == SAHPI_CTRL_STATE_OFF)) {
		ipmi_entity_pointer_cb(res_info->u.entity.entity_id,
				init_power_on_sequence_data_cb, ipmi_handler);
		send_pwronseq_commit_status_sensor_event(hnd, 0);
		g_static_rec_mutex_lock(&ipmi_handler->ohoih_lock);
		return SA_OK;
	}

	// SAHPI_CTRL_STATE_PULSE_ON operation

		// at first check the correctness of the lists
	num = 0;
	for (i = 0; ; i++) {
		recp = (ohoi_atca_pwonseq_rec_t *)g_slist_nth_data(
				ipmi_handler->atca_pwonseq_recs, i);
		if (recp == NULL) {
			if (i != 0) break; else continue;
		}
		num += recp->head[6];
	}
	if (num != g_slist_length(ipmi_handler->atca_pwonseq_desk)) {
		err("list length dismatched: %d != %d", num,
				g_slist_length(ipmi_handler->atca_pwonseq_desk));
		g_static_rec_mutex_unlock(&ipmi_handler->ohoih_lock);
		return SA_ERR_HPI_INTERNAL_ERROR;
	}

	di = 0;
	for (i = 0; ; i++) {
		recp = (ohoi_atca_pwonseq_rec_t *)g_slist_nth_data(
				ipmi_handler->atca_pwonseq_recs, i);
		if (recp == NULL) {
			if (i != 0) break; else continue;
		}
		if (!recp->updated) {
			continue;
		}
		memcpy(buf, recp->head, 7);
		len = 7;
		for (j = 0; j < recp->head[6]; j++) {
			dscp = (ohoi_atca_pwonseq_dsk_t *)g_slist_nth_data(
				ipmi_handler->atca_pwonseq_desk, di);
			if (dscp == NULL) {
				err("No descrintor %d for record %d", j, i);
				g_static_rec_mutex_unlock(&ipmi_handler->ohoih_lock);
				return SA_ERR_HPI_INTERNAL_ERROR;
			}
			memcpy(buf + len, dscp->body, 5);
			len += 5;
			di++;
		}
#if 0
printf("RECORD: 0x%02x%02x%02x%02x%02x%02x%02x\n", buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6]);
for (j = 0; j < recp->head[6]; j++)
printf("%02x%02x%02x%02x%02x\n", buf[7 + 5 * j + 0], buf[7 + 5 * j + 1], buf[7 + 5 * j + 2], buf[7 + 5 * j + 3], buf[7 + 5 * j + 4]);
#endif
		info.ipmi_handler = ipmi_handler;
		info.buf = buf;
		info.len = len;
		info.num = recp->rec_num;
		info.rv = SA_OK;
		g_mutex_lock(res_info->fru->mutex);
		ipmi_entity_pointer_cb(res_info->u.entity.entity_id,
				write_power_on_sequence_data_cb, &info);
		g_mutex_unlock(res_info->fru->mutex);
		if (info.rv != SA_OK) {
			ipmi_handler->shelf_fru_corrupted = 1;
			g_static_rec_mutex_unlock(&ipmi_handler->ohoih_lock);
			return info.rv;
		}
		recp->updated = 0;
	}
	ipmi_handler->atca_pwonseq_updated = 0;
	// XXX  Call real write fru
	send_pwronseq_commit_status_sensor_event(hnd, 0);
	g_static_rec_mutex_unlock(&ipmi_handler->ohoih_lock);

	return SA_OK;
}



	/*
	 *       Creating Virtual Shelf RDRs after domain fully up
	 */



struct create_pwonseq {
	unsigned char addr;
	unsigned char devid;
};

static int assign_slotid_to_pwonseq(
			     struct oh_handler_state *handler,
			     SaHpiRptEntryT *rpt,
                             struct ohoi_resource_info *res_info,
			     void *cb_data)
{
	ohoi_atca_pwonseq_dsk_t *dsk = cb_data;
	if (!(res_info->type & OHOI_RESOURCE_SLOT)) {
		return 0;
	}
	if ((dsk->body[0] << 1) != res_info->u.slot.addr) {
		return 0;
	}
	if (dsk->body[1] == res_info->u.slot.devid) {
		dsk->slotid = rpt->ResourceId;
		return 1;
	}
	if (dsk->body[1] == 0xFE) {
		dsk->slotid = rpt->ResourceId;
		return 1;
	}
	return 0;
}

void ohoi_atca_create_shelf_virtual_rdrs(struct oh_handler_state *hnd)
{
	struct ohoi_handler *ipmi_handler = hnd->data;
	ohoi_atca_pwonseq_dsk_t *dsk;
//	GSList *node;
	int i;
	SaHpiRptEntryT *rpt;
	SaHpiRdrT *rdr;
	struct ohoi_control_info *c_info;
	struct ohoi_sensor_info *s_info;
	int num_controls = 0;
	int num_sensors = 0;
	struct ohoi_resource_info *res_info;

	g_static_rec_mutex_lock(&ipmi_handler->ohoih_lock);
	rpt = oh_get_resource_by_id(hnd->rptcache,
				ipmi_handler->atca_shelf_id);
	if (rpt == NULL) {
		err("No rpt for atca chassis?");
		return;
	}
	res_info = oh_get_resource_data(hnd->rptcache,
					ipmi_handler->atca_shelf_id);


	// init data for Power On Sequence RDRs
	ipmi_entity_pointer_cb(res_info->u.entity.entity_id,
				init_power_on_sequence_data_cb, ipmi_handler);


	// Create Shelf Address control
	rdr = create_atca_shelf_address_control(ipmi_handler, rpt, &c_info);
	if (rdr != NULL && (oh_add_rdr(hnd->rptcache,
					ipmi_handler->atca_shelf_id,
					rdr, c_info, 1) != SA_OK)) {
		err("couldn't add control rdr");
		free(rdr);
		free(c_info);
	} else {
		num_controls++;
	}

	// Create Shelf IP Address control
	rdr = create_atca_shelf_ip_address_control(hnd, rpt, &c_info);
	if (rdr != NULL && (oh_add_rdr(hnd->rptcache,
					ipmi_handler->atca_shelf_id,
					rdr, c_info, 1) != SA_OK)) {
		err("couldn't add control rdr");
		free(rdr);
		free(c_info);
	} else {
		num_controls++;
	}

	// Create Chassis Status control
	rdr = create_atca_chassis_status_control(ipmi_handler, rpt, &c_info);
	if (rdr != NULL && (oh_add_rdr(hnd->rptcache,
					ipmi_handler->atca_shelf_id,
					rdr, c_info, 1) != SA_OK)) {
		err("couldn't add control rdr");
		free(rdr);
		free(c_info);
	} else {
		num_controls++;
	}

	// Create Power On Sequence Commit control
	rdr = create_fru_power_on_sequence_commit_control(hnd, rpt, &c_info);
	if (rdr != NULL && (oh_add_rdr(hnd->rptcache,
					ipmi_handler->atca_shelf_id,
					rdr, c_info, 1) != SA_OK)) {
		err("couldn't add control rdr");
		free(rdr);
		free(c_info);
	} else {
		num_controls++;
	}

	// assign Resource slot Ids for FRU activation descriptors
	// and create FRU Power ON Sequence controls
	i = 0;
	for (i = 0; i < g_slist_length(ipmi_handler->atca_pwonseq_desk); i++) {
		dsk = (ohoi_atca_pwonseq_dsk_t *)g_slist_nth_data(
				ipmi_handler->atca_pwonseq_desk, i);
		if (dsk == NULL) {
			err("no descriptor");
			continue;
		}
		ohoi_iterate_rptcache(hnd, assign_slotid_to_pwonseq, dsk);
		rdr = create_fru_power_on_sequence_control(hnd, rpt, &c_info, i,
						dsk->slotid);
		if (rdr != NULL && (oh_add_rdr(hnd->rptcache,
					ipmi_handler->atca_shelf_id,
						    rdr, c_info, 1) != SA_OK)) {
			err("couldn't add control rdr");
			free(rdr);
			free(c_info);
		} else {
			num_controls++;
		}
	}

	// Create Power On Sequence Commit Status sensor
	rdr = create_fru_power_on_sequence_commit_status_sensor(hnd,
							rpt, &s_info);
	if (rdr != NULL && (oh_add_rdr(hnd->rptcache,
					ipmi_handler->atca_shelf_id,
					rdr, s_info, 1) != SA_OK)) {
		err("couldn't add control rdr");
		free(rdr);
		free(s_info);
	} else {
		num_sensors++;
	}
	if (num_controls) {
		rpt->ResourceCapabilities |= SAHPI_CAPABILITY_CONTROL |
     						SAHPI_CAPABILITY_RDR;
	}
	if (num_sensors) {
		rpt->ResourceCapabilities |= SAHPI_CAPABILITY_SENSOR |
     						SAHPI_CAPABILITY_RDR;
	}
	if (!num_sensors && !num_controls) {
		g_static_rec_mutex_unlock(&ipmi_handler->ohoih_lock);
		return;
	}

	entity_rpt_set_updated(res_info, ipmi_handler);
	g_static_rec_mutex_unlock(&ipmi_handler->ohoih_lock);
}





