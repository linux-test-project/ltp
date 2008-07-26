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




 		//     Virtual Shelf Manager Redundancy Sensor





void ohoi_send_vshmgr_redundancy_sensor_event(
                                      struct oh_handler_state *handler,
				      int become_present)
{
	struct ohoi_handler *ipmi_handler = handler->data;
        SaErrorT         rv;
	struct ohoi_sensor_info *s_info = NULL;
        struct oh_event		*e;
        SaHpiSensorEventT	*sen_evt;
	int			num;
	SaHpiEventStateT	cur;
	SaHpiEventStateT	prev;

	rv = ohoi_get_rdr_data(handler, ipmi_handler->atca_vshm_id,
		SAHPI_SENSOR_RDR, ATCAHPI_SENSOR_NUM_SHMGR_REDUNDANCY,
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
	num = ipmi_handler->shmc_present_num;

	if (num == 1) {
		if (!(s_info->assert &
			SAHPI_ES_NON_REDUNDANT_SUFFICIENT_RESOURCES)) {
			err("SAHPI_ES_NON_REDUNDANT_SUFFICIENT"
				"_RESOURCES disabled");
			return;
		}
		cur = SAHPI_ES_NON_REDUNDANT_SUFFICIENT_RESOURCES;
		prev = SAHPI_ES_FULLY_REDUNDANT;
	} else if (num == 0) {
		if (!(s_info->assert &
			SAHPI_ES_NON_REDUNDANT_INSUFFICIENT_RESOURCES)) {
			err("SAHPI_ES_NON_REDUNDANT_INSUFFICIENT"
				"_RESOURCES disabled");
			return;
		}
		cur = SAHPI_ES_NON_REDUNDANT_INSUFFICIENT_RESOURCES;
		prev = SAHPI_ES_NON_REDUNDANT_SUFFICIENT_RESOURCES;
	} else if (num >= 2) {
		if (!become_present) {
			err("redunduncy not changed");
			return;
		}
		if (!(s_info->assert & SAHPI_ES_FULLY_REDUNDANT)) {
			err("SAHPI_ES_FULLY_REDUNDANT disabled");
			return;
		}
		cur = SAHPI_ES_FULLY_REDUNDANT;
		prev = SAHPI_ES_NON_REDUNDANT_SUFFICIENT_RESOURCES;
	} else {
		err("Internal error. Negative "
			"ipmi_handler->shmc_present_num = %d", num);
		return;
	}

        e = malloc(sizeof(*e));
        if (!e) {
                err("Out of space");
                return;
        }

        memset(e, 0, sizeof(*e));
        e->event.Source = ipmi_handler->atca_vshm_id;
        e->event.EventType = SAHPI_ET_SENSOR;
        e->event.Severity = SAHPI_MAJOR;
        oh_gettimeofday(&e->event.Timestamp);

        sen_evt = &(e->event.EventDataUnion.SensorEvent);
        sen_evt->SensorNum = ATCAHPI_SENSOR_NUM_SHMGR_REDUNDANCY;
        sen_evt->SensorType = SAHPI_OPERATIONAL;
        sen_evt->EventCategory = SAHPI_EC_REDUNDANCY;
        sen_evt->Assertion = SAHPI_TRUE;
        sen_evt->EventState = cur;
        sen_evt->OptionalDataPresent = SAHPI_SOD_PREVIOUS_STATE |
					 SAHPI_SOD_CURRENT_STATE;
        sen_evt->CurrentState = cur;
        sen_evt->PreviousState = prev;
        SaHpiRdrT *rdr = oh_get_rdr_by_type(handler->rptcache,
        				    ipmi_handler->atca_vshm_id,
        				    SAHPI_SENSOR_RDR,
        				    ATCAHPI_SENSOR_NUM_SHMGR_REDUNDANCY);
	if (rdr) e->rdrs = g_slist_append(e->rdrs, g_memdup(rdr, sizeof(SaHpiRdrT)));
        e->hid = handler->hid;
        oh_evt_queue_push(handler->eventq, e);
}

static SaErrorT get_vshmgr_redundancy_sensor_event_enable(
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


static SaErrorT set_vshmgr_redundancy_sensor_event_enable(
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
	if ((assert & ~(SAHPI_ES_FULLY_REDUNDANT |
			SAHPI_ES_NON_REDUNDANT_SUFFICIENT_RESOURCES |
			 SAHPI_ES_NON_REDUNDANT_INSUFFICIENT_RESOURCES))) {
		err("assert(0x%x)", assert);
		return SA_ERR_HPI_INVALID_DATA;
	}
	sinfo->assert = assert;
	sinfo->info.atcamap_sensor_info.val = enable;

	return SA_OK;
}


static SaErrorT get_vshmgr_redundancy_sensor_reading(
				       struct oh_handler_state *hnd,
				       struct ohoi_sensor_info *sensor_info,
				       SaHpiSensorReadingT *reading,
				       SaHpiEventStateT *ev_state)
{
	struct ohoi_handler *ipmi_handler = hnd->data;
	int num = ipmi_handler->shmc_present_num;

	if (reading != NULL) {
		reading->IsSupported = SAHPI_FALSE;
	}
	if (ev_state) {
		if (num == 0) {
			*ev_state = SAHPI_ES_NON_REDUNDANT_INSUFFICIENT_RESOURCES;
		} else if (num == 1) {
			*ev_state = SAHPI_ES_NON_REDUNDANT_SUFFICIENT_RESOURCES;
		} else {
			*ev_state = SAHPI_ES_FULLY_REDUNDANT;
		}
	}

	return SA_OK;
}


static SaErrorT get_vshmgr_redundancy_sensor_thresholds(
					  struct oh_handler_state *hnd,
					  struct ohoi_sensor_info *sinfo,
					  SaHpiSensorThresholdsT *thres)
{
	return SA_ERR_HPI_INVALID_CMD;
}


static SaErrorT set_vshmgr_redundancy_sensor_thresholds(
					  struct oh_handler_state *hnd,
					  struct ohoi_sensor_info *sinfo,
					  const SaHpiSensorThresholdsT *thres)
{
	return SA_ERR_HPI_INVALID_CMD;
}





static SaHpiRdrT *create_vshmgr_redundancy_sensor(
			struct oh_handler_state *handler,
			SaHpiRptEntryT *rpt,
			struct ohoi_sensor_info **sensor_info)
{
//	struct ohoi_handler *ipmi_handler = handler->data;
	SaHpiRdrT *rdr;
	struct ohoi_sensor_info *s_info;
	SaHpiEventStateT	events;



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
	events = SAHPI_ES_FULLY_REDUNDANT |
		SAHPI_ES_NON_REDUNDANT_SUFFICIENT_RESOURCES |
		SAHPI_ES_NON_REDUNDANT_INSUFFICIENT_RESOURCES;

	rdr->RdrType = SAHPI_SENSOR_RDR;
	rdr->IsFru = SAHPI_FALSE;
	rdr->Entity = rpt->ResourceEntity;
	rdr->RdrTypeUnion.SensorRec.Num = ATCAHPI_SENSOR_NUM_SHMGR_REDUNDANCY;
	rdr->RdrTypeUnion.SensorRec.Type = SAHPI_OPERATIONAL;
	rdr->RdrTypeUnion.SensorRec.Category = SAHPI_EC_REDUNDANCY;
	rdr->RdrTypeUnion.SensorRec.EnableCtrl = SAHPI_TRUE;
	rdr->RdrTypeUnion.SensorRec.EventCtrl = SAHPI_SEC_PER_EVENT;
	rdr->RdrTypeUnion.SensorRec.Events = events;
	rdr->RdrTypeUnion.SensorRec.DataFormat.IsSupported = SAHPI_FALSE;
	rdr->RdrTypeUnion.SensorRec.ThresholdDefn.IsAccessible = SAHPI_FALSE;
	oh_init_textbuffer(&rdr->IdString);
	oh_append_textbuffer(&rdr->IdString,
				"Shelf Manager Redundancy Sensor");

	s_info->support_assert = events;
	s_info->support_deassert = 0;
	s_info->assert = events;
	s_info->deassert = 0;
	s_info->sen_enabled = SAHPI_TRUE;
        s_info->enable = SAHPI_TRUE;
	s_info->info.atcamap_sensor_info.data = NULL;
	s_info->info.atcamap_sensor_info.val = SAHPI_TRUE;
	s_info->type = OHOI_SENSOR_ATCA_MAPPED;

	s_info->ohoii.get_sensor_event_enable =
		get_vshmgr_redundancy_sensor_event_enable;
	s_info->ohoii.set_sensor_event_enable =
		set_vshmgr_redundancy_sensor_event_enable;
	s_info->ohoii.get_sensor_reading =
		get_vshmgr_redundancy_sensor_reading;
	s_info->ohoii.get_sensor_thresholds =
		get_vshmgr_redundancy_sensor_thresholds;
	s_info->ohoii.set_sensor_thresholds =
		set_vshmgr_redundancy_sensor_thresholds;

	*sensor_info = s_info;

	return rdr;
}











void create_atca_virt_shmgr_rdrs(struct oh_handler_state *hnd)
{
	struct ohoi_handler *ipmi_handler = hnd->data;
	SaHpiRptEntryT *rpt;
	SaHpiRdrT *rdr;
//	struct ohoi_control_info *c_info;
	struct ohoi_sensor_info *s_info;
//	int num_controls = 0;
//	int num_sensors = 0;
	struct ohoi_resource_info *res_info;

	g_static_rec_mutex_lock(&ipmi_handler->ohoih_lock);
	rpt = oh_get_resource_by_id(hnd->rptcache,
				ipmi_handler->atca_vshm_id);
	if (rpt == NULL) {
		err("No rpt for atca chassis?");
		return;
	}
	res_info = oh_get_resource_data(hnd->rptcache,
					ipmi_handler->atca_vshm_id);



	// Create Power On Sequence Commit Status sensor
	rdr = create_vshmgr_redundancy_sensor(hnd, rpt, &s_info);
	if (rdr != NULL) {
		if (oh_add_rdr(hnd->rptcache,
					ipmi_handler->atca_vshm_id,
					rdr, s_info, 1) != SA_OK) {
			err("couldn't add control rdr");
			free(rdr);
			free(s_info);
		}
	} else {
		rpt->ResourceCapabilities |= SAHPI_CAPABILITY_SENSOR |
						SAHPI_CAPABILITY_RDR;
	}
	g_static_rec_mutex_unlock(&ipmi_handler->ohoih_lock);
}






