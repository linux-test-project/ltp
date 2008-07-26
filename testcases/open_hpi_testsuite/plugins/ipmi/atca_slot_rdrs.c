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
 *     Denis Sadykov <sadden@mail.ru>
 */



#include "ipmi.h"

#define uchar  unsigned char
int ipmicmd_mc_send(ipmi_mc_t *mc,
                 uchar netfn, uchar cmd, uchar lun,
		 uchar *pdata, uchar sdata,
		 ipmi_mc_response_handler_t handler,
		 void *handler_data);


static void atca_shelf_record_get(ipmi_entity_t *entity,
				  unsigned char record_type,
				  unsigned int *record_ver,
				  unsigned int *record_num,
				  unsigned char **record,
				  unsigned int *record_len)
{
	ipmi_fru_t			*fru;
	unsigned char			data[256];
	unsigned int			num;
	unsigned int			len;
	unsigned char			ver,	type;
	unsigned int			r_num;
	int				rv;

	fru = ipmi_entity_get_fru(entity);
	r_num = ipmi_fru_get_num_multi_records(fru);

	*record = NULL;
	for (num = 0; num < r_num; num++) {
		len = 256;
		rv = ipmi_fru_get_multi_record_data(fru, num, data, &len);
		if (rv != 0) {
			continue;
		}
		rv = ipmi_fru_get_multi_record_type(fru, num, &type);
		if (rv) {
			continue;
		}
		if (type != 0xc0) {
			continue;
		}
		rv = ipmi_fru_get_multi_record_format_version(fru, num, &ver);
		if (rv) {
			continue;
		}

		if ((ver & 0x0f) != 0x2) {
			continue;
		}
		if (len < 5) {
			continue;
		}
		if ((data[0] | (data[1] << 8) | (data[2] << 16)) !=
						ATCAHPI_PICMG_MID) {
			continue;
		}
		if (data[3] != record_type) {
			continue;
		}
		*record = malloc(len);
		memcpy(*record, data, len);
		*record_len = len;
		*record_ver = ver;
		*record_num = num;
		break;
	}

}

static void get_atca_fru_pwr_desc_cb(ipmi_entity_t *entity,
				     void *cb_data)
{
	atca_common_info_t		*info = cb_data;
	unsigned char			*data;
	unsigned int			len,	offset,	ver,	num_rec;
	unsigned int			i;

	info->len = offset = 0;

	atca_shelf_record_get(entity, 0x12, &ver, &num_rec, &data, &len);
	if (data == NULL) {
		info->done = 1;
		return;
	}

	for (i = 0; i < data[6]; i++) {
		if (data[7 + i*5] << 1 == info->addr) {
		memcpy(info->data + offset, data + 7 + i*5, 5);
		offset += 5;
		}
	}

	free(data);

	info->len = offset;
	info->done = 1;
}


/*--------------- Slot State Sensor --------------------------------------*/

static SaErrorT atca_slot_state_sensor_event_enable_get(
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

static SaErrorT atca_slot_state_sensor_event_enable_set(
					    struct oh_handler_state *hnd,
					    struct ohoi_sensor_info *sinfo,
					    SaHpiBoolT enable,
					    SaHpiEventStateT assert,
					    SaHpiEventStateT deassert,
					    unsigned int a_supported,
					    unsigned int d_supported)
{

	if (deassert != 0) {
		return SA_ERR_HPI_INVALID_DATA;
	}
	if ((assert & ~(SAHPI_ES_PRESENT | SAHPI_ES_ABSENT))) {
		return SA_ERR_HPI_INVALID_DATA;
	}
	sinfo->enable = enable;
	sinfo->assert = assert;
	sinfo->info.atcamap_sensor_info.val = enable;

	return SA_OK;
}

static SaHpiRptEntryT *atca_get_slot_state(struct oh_handler_state *handler,
					   struct ohoi_resource_info *slot_info)
{
	SaHpiRptEntryT			*rpt;
	struct ohoi_resource_info	*ent_info;

	if (handler == NULL || slot_info == NULL) {
		return NULL;
	}

	rpt = ohoi_get_resource_by_entityid(handler->rptcache,
					&slot_info->u.slot.entity_id);
	if (!rpt) {
		return NULL;
	}

	ent_info = (struct ohoi_resource_info *)
			oh_get_resource_data(handler->rptcache,
						rpt->ResourceId);

	if (ent_info->presence == 0) {
		return NULL;
	} else {
		return rpt;
	}
}

static SaErrorT atca_get_slot_state_reading(struct oh_handler_state *handler,
					   struct ohoi_sensor_info *sensor_info,
					   SaHpiSensorReadingT *reading,
					   SaHpiEventStateT *ev_state)
{
	struct ohoi_resource_info	*res_info;
	SaHpiRptEntryT			*rpt;

	reading->IsSupported = TRUE;
	reading->Type = SAHPI_SENSOR_READING_TYPE_UINT64;

	res_info = (struct ohoi_resource_info *)
			oh_get_resource_data(handler->rptcache,
				sensor_info->info.atcamap_sensor_info.rid);

	rpt = atca_get_slot_state(handler, res_info);

	if (rpt) {
		reading->Value.SensorUint64 = rpt->ResourceId;
	} else {
		reading->Value.SensorUint64 = SAHPI_UNSPECIFIED_RESOURCE_ID;
	}

	if (ev_state) {
		if (reading->Value.SensorUint64 ==
				SAHPI_UNSPECIFIED_RESOURCE_ID) {
			*ev_state = SAHPI_ES_ABSENT;
		} else {
			*ev_state = SAHPI_ES_PRESENT;
		}
	}
	return SA_OK;
}

void atca_slot_state_sensor_event_send(struct oh_handler_state *handler,
				       SaHpiRptEntryT *dev_entry,
				       int present)
{
	SaHpiResourceIdT		rid;
	struct oh_event			*e;
	SaHpiSensorEventT		*sen_evt;
	SaHpiRdrT			*rdr;
	struct ohoi_sensor_info		*sensor_info;

	if (!dev_entry) {
		return;
	}

	rid = ohoi_get_parent_id(dev_entry);
	if (rid == 0) {
		return;
	}
	rdr = oh_get_rdr_by_type(handler->rptcache, rid,
					SAHPI_SENSOR_RDR,
					ATCAHPI_SENSOR_NUM_SLOT_STATE);
	if (!rdr) {
		return;
	}
	sensor_info = (struct ohoi_sensor_info *)
				oh_get_rdr_data(handler->rptcache, rid,
						rdr->RecordId);
	if (!sensor_info) {
		return;
	}
	if (sensor_info->sen_enabled == SAHPI_FALSE ||
		sensor_info->info.atcamap_sensor_info.val == SAHPI_FALSE) {
		return;
	}

	if (present && !(sensor_info->assert & SAHPI_ES_PRESENT)) {
		return;
	}
	if (!present && !(sensor_info->assert & SAHPI_ES_ABSENT)) {
		return;
	}

	e = malloc(sizeof(*e));
	if (!e) {
		return;
	}
	memset(e, 0, sizeof(*e));
	e->resource = *dev_entry;
	e->rdrs = g_slist_append(e->rdrs, g_memdup(rdr, sizeof(SaHpiRdrT)));
	e->event.Source = rid;
	e->event.EventType = SAHPI_ET_SENSOR;
	e->event.Severity = SAHPI_INFORMATIONAL;
	oh_gettimeofday(&e->event.Timestamp);
	sen_evt = &(e->event.EventDataUnion.SensorEvent);
	sen_evt->SensorNum = ATCAHPI_SENSOR_NUM_SLOT_STATE;
	sen_evt->SensorType = SAHPI_OEM_SENSOR;
	sen_evt->EventCategory = SAHPI_EC_PRESENCE;
	sen_evt->Assertion = SAHPI_TRUE;
	sen_evt->EventState = present ? SAHPI_ES_PRESENT : SAHPI_ES_ABSENT;
	sen_evt->OptionalDataPresent = SAHPI_SOD_PREVIOUS_STATE |
					 	SAHPI_SOD_CURRENT_STATE;
	sen_evt->CurrentState = present ? SAHPI_ES_PRESENT : SAHPI_ES_ABSENT;
	sen_evt->PreviousState = present ? SAHPI_ES_ABSENT : SAHPI_ES_PRESENT;

        e->hid = handler->hid;
        oh_evt_queue_push(handler->eventq, e);
}

static SaHpiRdrT *atca_create_slot_state_sensor(
				struct ohoi_handler *ipmi_handler,
				struct ohoi_sensor_info **s_info)
{
	SaHpiRdrT *rdr;
	struct ohoi_sensor_info *l_s_info;

	rdr = malloc(sizeof (SaHpiRdrT));
	if (rdr == NULL) {
		err("Out of memory");
		return NULL;
	}
	l_s_info = malloc(sizeof (struct ohoi_sensor_info));
	if (l_s_info == NULL) {
		err("Out of memory");
		free(rdr);
		return NULL;
	}

	memset(rdr, 0, sizeof (SaHpiRdrT));
	memset(l_s_info, 0, sizeof (struct ohoi_sensor_info));

	rdr->RdrType = SAHPI_SENSOR_RDR;
	rdr->IsFru = SAHPI_FALSE;
	rdr->RdrTypeUnion.SensorRec.Num = ATCAHPI_SENSOR_NUM_SLOT_STATE;
	rdr->RdrTypeUnion.SensorRec.Type = SAHPI_ENTITY_PRESENCE;
	rdr->RdrTypeUnion.SensorRec.Category = SAHPI_EC_PRESENCE;
	rdr->RdrTypeUnion.SensorRec.EnableCtrl = SAHPI_TRUE;
	rdr->RdrTypeUnion.SensorRec.EventCtrl = SAHPI_SEC_PER_EVENT;
	rdr->RdrTypeUnion.SensorRec.Events = SAHPI_ES_PRESENT |
							SAHPI_ES_ABSENT;
	rdr->RdrTypeUnion.SensorRec.DataFormat.IsSupported = SAHPI_TRUE;
	rdr->RdrTypeUnion.SensorRec.DataFormat.ReadingType =
					SAHPI_SENSOR_READING_TYPE_UINT64;
	rdr->RdrTypeUnion.SensorRec.DataFormat.BaseUnits =
						SAHPI_SU_UNSPECIFIED;
	rdr->RdrTypeUnion.SensorRec.DataFormat.ModifierUnits =
						SAHPI_SU_UNSPECIFIED;
	rdr->RdrTypeUnion.SensorRec.DataFormat.ModifierUse =
						SAHPI_SMUU_NONE;
	rdr->RdrTypeUnion.SensorRec.DataFormat.Percentage = SAHPI_FALSE;
	rdr->RdrTypeUnion.SensorRec.DataFormat.Range.Flags = 0x0;
	rdr->RdrTypeUnion.SensorRec.DataFormat.AccuracyFactor = 0.00;
	rdr->RdrTypeUnion.SensorRec.ThresholdDefn.IsAccessible =
						SAHPI_FALSE;
	oh_init_textbuffer(&rdr->IdString);
	oh_append_textbuffer(&rdr->IdString, "Slot State Sensor");

	l_s_info->ohoii.get_sensor_event_enable =
				atca_slot_state_sensor_event_enable_get;
	l_s_info->ohoii.set_sensor_event_enable =
				atca_slot_state_sensor_event_enable_set;
	l_s_info->ohoii.get_sensor_reading = atca_get_slot_state_reading;
	l_s_info->ohoii.get_sensor_thresholds = NULL;
	l_s_info->ohoii.set_sensor_thresholds = NULL;
	l_s_info->support_assert = SAHPI_ES_PRESENT | SAHPI_ES_ABSENT;
	l_s_info->support_deassert = 0;
	l_s_info->assert = SAHPI_ES_PRESENT | SAHPI_ES_ABSENT;
	l_s_info->deassert = 0;
	l_s_info->sen_enabled = SAHPI_TRUE;
        l_s_info->enable = SAHPI_TRUE;
	l_s_info->info.atcamap_sensor_info.data = NULL;
	l_s_info->info.atcamap_sensor_info.val = SAHPI_FALSE;
	l_s_info->type = OHOI_SENSOR_ATCA_MAPPED;

	*s_info = l_s_info;

	return rdr;
}

/* ---------------- FRU activation control ------------------------------ */


struct fru_act_ctrl_state_s {
	SaHpiCtrlModeT	mode;
	SaHpiInt32T	state;
};

static SaErrorT get_atca_fru_activation_control_state(
				struct oh_handler_state *hnd,
				struct ohoi_control_info *c_info,
				SaHpiRdrT * rdr,
				SaHpiCtrlModeT *mode,
				SaHpiCtrlStateT *state)
{
	struct ohoi_handler		*ipmi_handler = (struct ohoi_handler *)
								hnd->data;
	struct	ohoi_resource_info	*shelf_info,	*slot_info;
	atca_common_info_t		info;
	int				rv;


	shelf_info = (struct ohoi_resource_info *)
		oh_get_resource_data(hnd->rptcache,
					ipmi_handler->atca_shelf_id);
	if (shelf_info == NULL) {
		return SA_ERR_HPI_INTERNAL_ERROR;
	}

	slot_info = (struct ohoi_resource_info *)
		oh_get_resource_data(hnd->rptcache,
				c_info->info.atcamap_ctrl_info.rid);
	if (slot_info == NULL) {
		return SA_ERR_HPI_INTERNAL_ERROR;
	}

	info.done = 0;
	info.rv = 0;
	info.addr = slot_info->u.slot.addr;
	info.devid = slot_info->u.slot.devid;

	rv = ipmi_entity_pointer_cb(shelf_info->u.entity.entity_id,
			get_atca_fru_pwr_desc_cb, &info);
	if (rv) {
		return SA_ERR_HPI_INTERNAL_ERROR;
	}
	rv = ohoi_loop(&info.done, ipmi_handler);
	if (rv != SA_OK) {
		return rv;
	}

	if (info.len == 0) {
		return SA_ERR_HPI_INTERNAL_ERROR;
	}


	if (mode) {
		if (info.data[4] >> 6) {
			*mode = c_info->mode = SAHPI_CTRL_MODE_AUTO;
		} else {
			*mode = c_info->mode = SAHPI_CTRL_MODE_MANUAL;
		}
	}

	if (state == NULL) {
		return SA_OK;
	}

	state->Type = SAHPI_CTRL_TYPE_ANALOG;
	state->StateUnion.Analog = (info.data[4] & 0x3F);

	return SA_OK;
}

static void set_atca_fru_activation_control_state_cb(ipmi_entity_t *entity,
						     void *cb_data)
{
	atca_common_info_t		*info = cb_data;
	unsigned char			*data;
	unsigned int			len,	offset,	ver,	num_rec;
	unsigned int			i;
	int				rv;
	struct fru_act_ctrl_state_s	*ctrl_state = info->info;

	info->len = offset = 0;

	atca_shelf_record_get(entity, 0x12, &ver, &num_rec, &data, &len);
	if (data == NULL) {
		info->rv = SA_ERR_HPI_INTERNAL_ERROR;
		info->done = 1;
		return;
	}

	for (i = 0; i < data[6]; i++) {
		if (data[7 + i*5] << 1 == info->addr) {
			if (ctrl_state->mode) {
				data[11 + i*5] = ctrl_state->state |
						(!ctrl_state->mode << 6);
			} else {
				data[11 + i*5] |= (!ctrl_state->mode << 6);
			}
		}
	}

	rv = ipmi_fru_set_multi_record(ipmi_entity_get_fru(entity),
					num_rec, 0xC0, ver,
					data, len);

	free(data);
	if (rv != 0) {
		info->rv = SA_ERR_HPI_INTERNAL_ERROR;
	}
	info->done = 1;
}

static SaErrorT set_atca_fru_activation_control_state(
                                      struct oh_handler_state *hnd,
                                      struct ohoi_control_info *c_info,
                                      SaHpiRdrT *rdr,
                                      SaHpiCtrlModeT mode,
                                      SaHpiCtrlStateT *state)
{
	struct ohoi_handler		*ipmi_handler = (struct ohoi_handler *)
								hnd->data;
	struct	ohoi_resource_info	*shelf_info,	*slot_info;
	atca_common_info_t		info;
	struct fru_act_ctrl_state_s	ctrl_state;
	int				rv;

	if (state == NULL) {
		return SA_ERR_HPI_INVALID_DATA;
	}

	if (state->Type != SAHPI_CTRL_TYPE_ANALOG) {
		return SA_ERR_HPI_INVALID_DATA;
	}

	shelf_info = (struct ohoi_resource_info *)
		oh_get_resource_data(hnd->rptcache,
					ipmi_handler->atca_shelf_id);
	if (shelf_info == NULL) {
		return SA_ERR_HPI_INTERNAL_ERROR;
	}

	slot_info = (struct ohoi_resource_info *)
		oh_get_resource_data(hnd->rptcache,
				c_info->info.atcamap_ctrl_info.rid);
	if (slot_info == NULL) {
		return SA_ERR_HPI_INTERNAL_ERROR;
	}

	info.done = 0;
	info.rv = 0;
	info.addr = slot_info->u.slot.addr;
	info.devid = slot_info->u.slot.devid;
	ctrl_state.mode = mode;
	ctrl_state.state = state->StateUnion.Analog;
	info.info = &ctrl_state;

	rv = ipmi_entity_pointer_cb(shelf_info->u.entity.entity_id,
			set_atca_fru_activation_control_state_cb, &info);
	if (rv) {
		return SA_ERR_HPI_INTERNAL_ERROR;
	}
	rv = ohoi_loop(&info.done, ipmi_handler);
	if (rv != SA_OK) {
		return rv;
	}

	return info.rv;
}

static SaHpiRdrT *atca_create_fru_activation_control(
				struct oh_handler_state *handler,
				SaHpiRptEntryT *slot,
				struct ohoi_control_info **c_info)
{
	SaHpiRdrT			*rdr;
	struct ohoi_control_info	*l_c_info;

	rdr = malloc(sizeof (SaHpiRdrT));
	if (rdr == NULL) {
		err("Out of memory");
		return NULL;
	}
	l_c_info = malloc(sizeof (struct ohoi_control_info));
	if (l_c_info == NULL) {
		err("Out of memory");
		free(rdr);
		return NULL;
	}

	memset(rdr, 0, sizeof (SaHpiRdrT));
	memset(l_c_info, 0, sizeof (struct ohoi_control_info));

	rdr->RdrType = SAHPI_CTRL_RDR;
	rdr->IsFru = SAHPI_FALSE;
	rdr->RdrTypeUnion.CtrlRec.Num = ATCAHPI_CTRL_NUM_FRU_ACTIVATION;
	rdr->RdrTypeUnion.CtrlRec.OutputType = SAHPI_CTRL_GENERIC;
	rdr->RdrTypeUnion.CtrlRec.Type = SAHPI_CTRL_TYPE_ANALOG;
	rdr->RdrTypeUnion.CtrlRec.TypeUnion.Analog.Min = 0;
	rdr->RdrTypeUnion.CtrlRec.TypeUnion.Analog.Max = 0x3f;
	rdr->RdrTypeUnion.CtrlRec.TypeUnion.Analog.Default = 0;
	rdr->RdrTypeUnion.CtrlRec.DefaultMode.Mode = SAHPI_CTRL_MODE_AUTO;
	rdr->RdrTypeUnion.CtrlRec.DefaultMode.ReadOnly = SAHPI_FALSE;
	rdr->RdrTypeUnion.CtrlRec.WriteOnly = SAHPI_FALSE;
	oh_init_textbuffer(&rdr->IdString);
	oh_append_textbuffer(&rdr->IdString, "FRU activation control");

	l_c_info->ohoii.get_control_state =
				get_atca_fru_activation_control_state;
	l_c_info->ohoii.set_control_state =
				set_atca_fru_activation_control_state;
	l_c_info->mode = SAHPI_CTRL_MODE_AUTO;
	l_c_info->type = OHOI_CTRL_ATCA_MAPPED;
	*c_info = l_c_info;

	return rdr;
}

/*----------------------- Assigned Power Sensor -----------------------------*/

struct assigned_pwr_sensor_s {
	SaHpiFloat64T	ass_pwr;
};

static void _atca_get_assigned_pwr_cb(ipmi_mc_t *mc,
				     ipmi_msg_t *msg,
				     void *rsp_data)
{
	atca_common_info_t		*info = rsp_data;
	int				rv = msg->data[0];
	struct assigned_pwr_sensor_s	*ass_pwr =
			(struct assigned_pwr_sensor_s *)info->info;

	if (mc == NULL) {
		info->rv = SA_ERR_HPI_ENTITY_NOT_PRESENT;
		info->done = 1;
		return;
	}
	if (rv == 0xc1) {
		info->rv = SA_ERR_HPI_INVALID_CMD;
	} else if (rv == 0xc3) {
		info->rv = SA_ERR_HPI_NO_RESPONSE;
	} else if (rv != 0) {
		info->rv = SA_ERR_HPI_INVALID_PARAMS;
	} else {
		ass_pwr->ass_pwr = (SaHpiFloat64T)
			(msg->data[msg->data_len - 1] * (msg->data[4] / 10));
	}
	info->done = 1;
	return;
};

static void atca_get_assigned_pwr_cb(ipmi_mc_t *mc,
				  void *cb_data)
{
	atca_common_info_t		*info = cb_data;
	unsigned char			data[25];
	int				rv;

	memset(data, 0, 25);
	data[0] = IPMI_PICMG_GRP_EXT;
	data[1] = info->devid;
	data[2] = 0;

	rv = ipmicmd_mc_send(mc,
		IPMI_GROUP_EXTENSION_NETFN,
		IPMI_PICMG_CMD_GET_POWER_LEVEL,
		0, data, 3,
		_atca_get_assigned_pwr_cb,
		cb_data);
	if (rv != 0) {
		err("ipmicmd_send = 0x%x\n", rv);
		OHOI_MAP_ERROR(info->rv, rv);
		return;
	}
	return;
}

static SaErrorT atca_get_assigned_pwr_reading(struct oh_handler_state *handler,
					      struct ohoi_sensor_info *s_info,
					      SaHpiSensorReadingT *reading,
					      SaHpiEventStateT *ev_state)
{
	struct ohoi_handler		*ipmi_handler =
					(struct ohoi_handler *)handler->data;
	struct	ohoi_resource_info	*slot_info,	*ent_info;
	atca_common_info_t		info;
	struct assigned_pwr_sensor_s	ass_pwr;
	SaHpiRptEntryT			*rpt;
	SaErrorT			rv;

	reading->IsSupported = FALSE;

	if (ev_state) {
		*ev_state = 0;
	}

	slot_info = (struct ohoi_resource_info *)
		oh_get_resource_data(handler->rptcache,
				s_info->info.atcamap_sensor_info.rid);

	rpt = atca_get_slot_state(handler, slot_info);
	if (!rpt) {
		reading->Value.SensorFloat64 = 0;
	} else {
		ent_info = (struct ohoi_resource_info *)
			oh_get_resource_data(handler->rptcache,
						rpt->ResourceId);
		info.done = 0;
		info.rv = SA_OK;
		info.addr = slot_info->u.slot.addr;
		info.devid = slot_info->u.slot.devid;
		info.info = &ass_pwr;
		rv = ipmi_mc_pointer_cb(ent_info->u.entity.mc_id,
				atca_get_assigned_pwr_cb,
				&info);
		if (rv != 0) {
			err("ipmi_domain_pointer_cb = 0x%x", rv);
			return SA_ERR_HPI_INTERNAL_ERROR;
		}
		rv = ohoi_loop(&info.done, ipmi_handler);
		if (rv != SA_OK) {
			err("ohoi_loop = 0x%x", rv);
			return SA_ERR_HPI_INTERNAL_ERROR;
		}
		if (info.rv != SA_OK) {
			err("info.rv = 0x%x\n", info.rv);
			return info.rv;
		}
		reading->Value.SensorFloat64 = ass_pwr.ass_pwr;
	}

	reading->IsSupported = TRUE;
	reading->Type = SAHPI_SENSOR_READING_TYPE_FLOAT64;

	return SA_OK;
}

static SaErrorT atca_assigned_pwr_sensor_event_enable_get(
					struct oh_handler_state *hnd,
					struct ohoi_sensor_info *sinfo,
					SaHpiBoolT   *enable,
					SaHpiEventStateT  *assert,
					SaHpiEventStateT  *deassert)
{
	*assert = sinfo->assert;
	*deassert = sinfo->deassert;
	*enable = sinfo->info.atcamap_sensor_info.val;
	return SA_OK;
}

static SaErrorT atca_get_assigned_pwr_thresholds(struct oh_handler_state *hnd,
						struct ohoi_sensor_info *sinfo,
						SaHpiSensorThresholdsT *thres)
{
	memset(thres, 0, sizeof (SaHpiSensorThresholdsT));
	thres->LowCritical.IsSupported = SAHPI_TRUE;
	thres->LowCritical.Type = SAHPI_SENSOR_READING_TYPE_FLOAT64;
	thres->LowCritical.Value.SensorFloat64 = 0;
	thres->UpCritical.IsSupported = SAHPI_TRUE;
	thres->UpCritical.Type = SAHPI_SENSOR_READING_TYPE_FLOAT64;
	thres->UpCritical.Value.SensorFloat64 = 400;

	return SA_OK;
}

static SaHpiRdrT *atca_create_assigned_pwr_sensor(
				struct ohoi_handler *ipmi_handler,
				struct ohoi_sensor_info **s_info)
{
	SaHpiRdrT *rdr;
	struct ohoi_sensor_info *l_s_info;

	rdr = malloc(sizeof (SaHpiRdrT));
	if (rdr == NULL) {
		err("Out of memory");
		return NULL;
	}
	l_s_info = malloc(sizeof (struct ohoi_sensor_info));
	if (l_s_info == NULL) {
	err("Out of memory");
		free(rdr);
		return NULL;
	}

	memset(rdr, 0, sizeof (SaHpiRdrT));
	memset(l_s_info, 0, sizeof (struct ohoi_sensor_info));

	rdr->RdrType = SAHPI_SENSOR_RDR;
	rdr->IsFru = SAHPI_FALSE;
	rdr->RdrTypeUnion.SensorRec.Num = ATCAHPI_SENSOR_NUM_ASSIGNED_PWR;
	rdr->RdrTypeUnion.SensorRec.Type = SAHPI_OTHER_UNITS_BASED_SENSOR;
	rdr->RdrTypeUnion.SensorRec.Category = SAHPI_EC_THRESHOLD;
	rdr->RdrTypeUnion.SensorRec.EnableCtrl = SAHPI_TRUE;
	rdr->RdrTypeUnion.SensorRec.EventCtrl = SAHPI_SEC_READ_ONLY;
	rdr->RdrTypeUnion.SensorRec.Events = 0;
	rdr->RdrTypeUnion.SensorRec.DataFormat.IsSupported = SAHPI_TRUE;
	rdr->RdrTypeUnion.SensorRec.DataFormat.ReadingType =
					SAHPI_SENSOR_READING_TYPE_FLOAT64;
	rdr->RdrTypeUnion.SensorRec.DataFormat.BaseUnits =
						SAHPI_SU_WATTS;
	rdr->RdrTypeUnion.SensorRec.DataFormat.ModifierUnits =
						SAHPI_SU_UNSPECIFIED;
	rdr->RdrTypeUnion.SensorRec.DataFormat.ModifierUse =
						SAHPI_SMUU_NONE;
	rdr->RdrTypeUnion.SensorRec.DataFormat.Percentage = SAHPI_FALSE;
	rdr->RdrTypeUnion.SensorRec.DataFormat.Range.Flags =
						SAHPI_SRF_MIN | SAHPI_SRF_MAX;
	rdr->RdrTypeUnion.SensorRec.DataFormat.Range.Max.IsSupported =
						SAHPI_TRUE;
	rdr->RdrTypeUnion.SensorRec.DataFormat.Range.Max.Type =
					SAHPI_SENSOR_READING_TYPE_FLOAT64;
	rdr->RdrTypeUnion.SensorRec.DataFormat.Range.Max.Value.SensorFloat64 =
						400;
	rdr->RdrTypeUnion.SensorRec.DataFormat.Range.Min.IsSupported =
						SAHPI_TRUE;
	rdr->RdrTypeUnion.SensorRec.DataFormat.Range.Min.Type =
					SAHPI_SENSOR_READING_TYPE_FLOAT64;
	rdr->RdrTypeUnion.SensorRec.DataFormat.Range.Min.Value.SensorFloat64 =
						0;
	rdr->RdrTypeUnion.SensorRec.DataFormat.AccuracyFactor = 0.1;
	rdr->RdrTypeUnion.SensorRec.ThresholdDefn.IsAccessible = SAHPI_TRUE;
	rdr->RdrTypeUnion.SensorRec.ThresholdDefn.ReadThold =
					SAHPI_STM_LOW_CRIT | SAHPI_STM_UP_CRIT;
	rdr->RdrTypeUnion.SensorRec.ThresholdDefn.WriteThold = 0;
	rdr->RdrTypeUnion.SensorRec.ThresholdDefn.Nonlinear = SAHPI_FALSE;
	oh_init_textbuffer(&rdr->IdString);
	oh_append_textbuffer(&rdr->IdString, "Assigned Power Sensor");

	l_s_info->ohoii.get_sensor_event_enable =
				atca_assigned_pwr_sensor_event_enable_get;
	l_s_info->ohoii.set_sensor_event_enable = NULL;
	l_s_info->ohoii.get_sensor_reading = atca_get_assigned_pwr_reading;
	l_s_info->ohoii.get_sensor_thresholds =
					atca_get_assigned_pwr_thresholds;
	l_s_info->ohoii.set_sensor_thresholds = NULL;
	l_s_info->support_assert = 0;
	l_s_info->support_deassert = 0;
	l_s_info->assert = 0;
	l_s_info->deassert = 0;
	l_s_info->sen_enabled = SAHPI_TRUE;
        l_s_info->enable = SAHPI_TRUE;
	l_s_info->info.atcamap_sensor_info.data = NULL;
	l_s_info->info.atcamap_sensor_info.val = SAHPI_FALSE;
	l_s_info->type = OHOI_SENSOR_ATCA_MAPPED;

	*s_info = l_s_info;

	return rdr;
}

/*-------------- Maximum Power Capability Sensor ----------------------------*/

static SaErrorT atca_get_max_pwr_capability_event_enable(
					struct oh_handler_state *hnd,
					struct ohoi_sensor_info *sinfo,
					SaHpiBoolT   *enable,
					SaHpiEventStateT  *assert,
					SaHpiEventStateT  *deassert)
{
	*assert = 0;
	*deassert = 0;
	*enable = SAHPI_FALSE;
	return SA_OK;
}

static SaErrorT atca_get_max_pwr_capability_reading(
					struct oh_handler_state *hnd,
					struct ohoi_sensor_info *s_info,
					SaHpiSensorReadingT *reading,
					SaHpiEventStateT *ev_state)
{
	struct ohoi_handler		*ipmi_handler = (struct ohoi_handler *)
								hnd->data;
	struct	ohoi_resource_info	*shelf_info,	*slot_info;
	atca_common_info_t		info;
	int				rv;

	reading->IsSupported = FALSE;

	if (ev_state) {
		*ev_state = 0;
	}

	shelf_info = (struct ohoi_resource_info *)
		oh_get_resource_data(hnd->rptcache,
					ipmi_handler->atca_shelf_id);
	if (shelf_info == NULL) {
		return SA_ERR_HPI_INTERNAL_ERROR;
	}

	slot_info = (struct ohoi_resource_info *)
		oh_get_resource_data(hnd->rptcache,
				s_info->info.atcamap_sensor_info.rid);
	if (slot_info == NULL) {
		return SA_ERR_HPI_INTERNAL_ERROR;
	}

	info.done = 0;
	info.rv = 0;
	info.addr = slot_info->u.slot.addr;
	info.devid = slot_info->u.slot.devid;

	rv = ipmi_entity_pointer_cb(shelf_info->u.entity.entity_id,
				    get_atca_fru_pwr_desc_cb, &info);
	if (rv) {
		return SA_ERR_HPI_INTERNAL_ERROR;
	}
	rv = ohoi_loop(&info.done, ipmi_handler);
	if (rv != SA_OK) {
		return rv;
	}

	if (info.len == 0) {
		return SA_ERR_HPI_INTERNAL_ERROR;
	}

	memcpy(&reading->Value.SensorUint64, info.data + 2, 2);

	reading->IsSupported = TRUE;
	reading->Type = SAHPI_SENSOR_READING_TYPE_UINT64;

	return SA_OK;
}

static SaHpiRdrT *atca_create_max_pwr_capability_sensor(
				struct ohoi_handler *ipmi_handler,
				struct ohoi_sensor_info **s_info)
{
	SaHpiRdrT *rdr;
	struct ohoi_sensor_info *l_s_info;

	rdr = malloc(sizeof (SaHpiRdrT));
	if (rdr == NULL) {
		err("Out of memory");
		return NULL;
	}
	l_s_info = malloc(sizeof (struct ohoi_sensor_info));
	if (l_s_info == NULL) {
	err("Out of memory");
		free(rdr);
		return NULL;
	}

	memset(rdr, 0, sizeof (SaHpiRdrT));
	memset(l_s_info, 0, sizeof (struct ohoi_sensor_info));

	rdr->RdrType = SAHPI_SENSOR_RDR;
	rdr->IsFru = SAHPI_FALSE;
	rdr->RdrTypeUnion.SensorRec.Num = ATCAHPI_SENSOR_NUM_MAX_PWR;
	rdr->RdrTypeUnion.SensorRec.Type = SAHPI_OTHER_UNITS_BASED_SENSOR;
	rdr->RdrTypeUnion.SensorRec.Category = SAHPI_EC_THRESHOLD;
	rdr->RdrTypeUnion.SensorRec.EnableCtrl = SAHPI_TRUE;
	rdr->RdrTypeUnion.SensorRec.EventCtrl = SAHPI_SEC_READ_ONLY;
	rdr->RdrTypeUnion.SensorRec.Events = 0;
	rdr->RdrTypeUnion.SensorRec.DataFormat.IsSupported = SAHPI_TRUE;
	rdr->RdrTypeUnion.SensorRec.DataFormat.ReadingType =
					SAHPI_SENSOR_READING_TYPE_UINT64;
	rdr->RdrTypeUnion.SensorRec.DataFormat.BaseUnits =
						SAHPI_SU_WATTS;
	rdr->RdrTypeUnion.SensorRec.DataFormat.ModifierUnits =
						SAHPI_SU_UNSPECIFIED;
	rdr->RdrTypeUnion.SensorRec.DataFormat.ModifierUse =
						SAHPI_SMUU_NONE;
	rdr->RdrTypeUnion.SensorRec.DataFormat.Percentage = SAHPI_FALSE;
	rdr->RdrTypeUnion.SensorRec.DataFormat.Range.Flags =
						SAHPI_SRF_MIN | SAHPI_SRF_MAX;
	rdr->RdrTypeUnion.SensorRec.DataFormat.Range.Max.IsSupported =
						SAHPI_TRUE;
	rdr->RdrTypeUnion.SensorRec.DataFormat.Range.Max.Type =
					SAHPI_SENSOR_READING_TYPE_UINT64;
	rdr->RdrTypeUnion.SensorRec.DataFormat.Range.Max.Value.SensorUint64 =
						800;
	rdr->RdrTypeUnion.SensorRec.DataFormat.Range.Min.IsSupported =
						SAHPI_TRUE;
	rdr->RdrTypeUnion.SensorRec.DataFormat.Range.Min.Type =
					SAHPI_SENSOR_READING_TYPE_UINT64;
	rdr->RdrTypeUnion.SensorRec.DataFormat.Range.Min.Value.SensorUint64 =
						0;
	rdr->RdrTypeUnion.SensorRec.DataFormat.AccuracyFactor = 1;
	rdr->RdrTypeUnion.SensorRec.ThresholdDefn.IsAccessible = SAHPI_FALSE;
	oh_init_textbuffer(&rdr->IdString);
	oh_append_textbuffer(&rdr->IdString,
				"Maximum Power Capability Sensor");

	l_s_info->ohoii.get_sensor_event_enable =
			atca_get_max_pwr_capability_event_enable;
	l_s_info->ohoii.set_sensor_event_enable = NULL;
	l_s_info->ohoii.get_sensor_reading =
			atca_get_max_pwr_capability_reading;
	l_s_info->ohoii.get_sensor_thresholds =NULL;
	l_s_info->ohoii.set_sensor_thresholds = NULL;
	l_s_info->support_assert = 0;
	l_s_info->support_deassert = 0;
	l_s_info->assert = 0;
	l_s_info->deassert = 0;
	l_s_info->sen_enabled = SAHPI_TRUE;
        l_s_info->enable = SAHPI_TRUE;
	l_s_info->info.atcamap_sensor_info.data = NULL;
	l_s_info->info.atcamap_sensor_info.val = SAHPI_FALSE;
	l_s_info->type = OHOI_SENSOR_ATCA_MAPPED;

	*s_info = l_s_info;

	return rdr;
}
/*---------------------------------------------------------------------------*/


void atca_create_slot_rdrs(struct oh_handler_state *handler,
			   SaHpiResourceIdT rid)
{
	struct ohoi_handler		*ipmi_handler = (struct ohoi_handler *)
								handler->data;
	struct ohoi_sensor_info		*s_info;
	struct ohoi_control_info	*c_info;
	SaHpiRdrT			*rdr;
	SaHpiRptEntryT			*rpt;

	rpt = oh_get_resource_by_id(handler->rptcache, rid);
	if (rpt == NULL) {
		err("No rpt for atca chassis?");
		return;
	}

	rdr = atca_create_slot_state_sensor(ipmi_handler, &s_info);

	if (rdr) {
		if (oh_add_rdr(handler->rptcache, rid,
					rdr, s_info, 1) != SA_OK) {
			err("couldn't add sensor rdr");
			free(s_info);
		} else {
			rpt->ResourceCapabilities |=
					SAHPI_CAPABILITY_SENSOR |
						SAHPI_CAPABILITY_RDR;
			s_info->info.atcamap_sensor_info.rid = rid;
		}
	}

	rdr = atca_create_fru_activation_control(handler, rpt, &c_info);

	if (rdr) {
		if (oh_add_rdr(handler->rptcache, rid,
					rdr, c_info, 1) != SA_OK) {
			err("couldn't add control rdr");
			free(c_info);
		} else {
			rpt->ResourceCapabilities |=
					SAHPI_CAPABILITY_CONTROL |
						SAHPI_CAPABILITY_RDR;
			c_info->info.atcamap_ctrl_info.rid = rid;
		}
	}

	rdr = atca_create_assigned_pwr_sensor(ipmi_handler, &s_info);

	if (rdr) {
		if (oh_add_rdr(handler->rptcache, rid,
					rdr, s_info, 1) != SA_OK) {
			err("couldn't add sensor rdr");
			free(s_info);
		} else {
			rpt->ResourceCapabilities |=
					SAHPI_CAPABILITY_SENSOR |
						SAHPI_CAPABILITY_RDR;
			s_info->info.atcamap_sensor_info.rid = rid;
		}
	}

	rdr = atca_create_max_pwr_capability_sensor(ipmi_handler, &s_info);

	if (rdr) {
		if (oh_add_rdr(handler->rptcache, rid,
					rdr, s_info, 1) != SA_OK) {
			err("couldn't add sensor rdr");
			free(s_info);
		} else {
			rpt->ResourceCapabilities |=
					SAHPI_CAPABILITY_SENSOR |
						SAHPI_CAPABILITY_RDR;
			s_info->info.atcamap_sensor_info.rid = rid;
		}
	}

}

