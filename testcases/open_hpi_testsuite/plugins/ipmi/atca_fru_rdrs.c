
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
#define uchar  unsigned char


int ipmicmd_send(ipmi_domain_t *domain,
                 uchar netfn, uchar cmd, uchar lun, uchar chan,
		 uchar *pdata, uchar sdata,
		 ipmi_addr_response_handler_t handler,
		 void *handler_data);

int ipmicmd_mc_send(ipmi_mc_t *mc,
                 uchar netfn, uchar cmd, uchar lun,
		 uchar *pdata, uchar sdata,
		 ipmi_mc_response_handler_t handler,
		 void *handler_data);
		 
		 
		/*
		 *   IPMB-0 Sensor
		 */


struct atca_ipmb0_link_num_s {
	int l_num;
	int s_num;
	ipmi_sensor_t	*sensor;
	int done;
	SaErrorT rv;
}; 
 
 


static int get_ipmb0_sensor_num_done(
			ipmi_domain_t *domain,
			ipmi_msgi_t   *rspi)
{
	struct atca_ipmb0_link_num_s *info = rspi->data1;
	ipmi_msg_t *msg = &rspi->msg;
	int rv = msg->data[0];
	
	
	dbg("get ipmb link info(%d) for sensor 0x%x: %02x %02x %02x %02x",
		msg->data_len, info->s_num, msg->data[0], msg->data[1],
		msg->data[2], msg->data[3]);
		
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
		info->l_num = msg->data[2];
	}
	info->done = 1;
	return IPMI_MSG_ITEM_NOT_USED;
};

static void get_ipmb0_sensor_num_cb(
			ipmi_domain_t *domain,
			void *cb_data)
{
	struct atca_ipmb0_link_num_s *info = cb_data;
	unsigned char data[16];
	int rv;
	ipmi_sensor_id_t sid = ipmi_sensor_convert_to_id(info->sensor);
	info->s_num = sid.sensor_num;

	data[0] = 0; // PICMB Identifier;
	data[1] = 0x01; // next byte contains sensor number
	data[2] = sid.sensor_num;

	rv = ipmicmd_send(domain,
		0x2c, 0x18, sid.lun, sid.mcid.channel, data, 3,
		get_ipmb0_sensor_num_done,
		cb_data);
	if (rv != 0) {
		err("ipmicmd_send = 0x%x", rv);
		OHOI_MAP_ERROR(info->rv, rv);
		return;
	} else {
		dbg("get ipmb link info send(0x%x 0x%x): %02x %02x %02x",
			sid.lun, sid.mcid.channel, data[0], data[1], data[2]);
	} 
	return;
}



static int get_ipmb0_sensor_num(struct oh_handler_state *handler,
					  ipmi_sensor_t	*sensor,
					  SaHpiSensorNumT *snump)
{
	struct ohoi_handler *ipmi_handler = handler->data;
	struct atca_ipmb0_link_num_s info;
	ipmi_entity_t *entity = ipmi_sensor_get_entity(sensor);
	int rv;
	
	if (ipmi_handler->d_type != IPMI_DOMAIN_TYPE_ATCA) {
		return 1;
	}
	
	if (ipmi_entity_get_entity_id(entity) != 0xf0) {
		// it is not shelf manager
		*snump = ATCAHPI_SENSOR_NUM_IPMB0;
		return 0;
	}
	
	info.done = 0;
	info.sensor = sensor;
	info.rv = SA_OK;
	rv = ipmi_domain_pointer_cb(ipmi_handler->domain_id,
				get_ipmb0_sensor_num_cb,
				&info);
	if (rv != 0) {
		err("ipmi_domain_pointer_cb = 0x%x", rv);
		return 1;
	}
	rv = ohoi_loop(&info.done, ipmi_handler);
	if (rv != SA_OK) {
		err("ohoi_loop = 0x%x", rv);
		return 1;
	}
	if (info.rv != SA_OK) {
		err("info.rv = 0x%x", info.rv);
		return 1;
	}
	*snump = ATCAHPI_SENSOR_NUM_IPMB0 + info.l_num;
	return 0;
}



static SaHpiEventStateT ipmb0_sensor_events_to_hpi(SaHpiEventStateT ev)
{
	SaHpiEventStateT newev = 0;
	if (ev & 0x01) {
		newev |= SAHPI_ES_NON_REDUNDANT_INSUFFICIENT_RESOURCES;
	}
	if ((ev & 0x02) || (ev & 0x04)) {
		newev |= SAHPI_ES_NON_REDUNDANT_SUFFICIENT_RESOURCES;
	}
	if (ev & 0x08) {
		newev |= SAHPI_ES_FULLY_REDUNDANT;
	}
	return newev;
}



static SaHpiEventStateT ipmb0_sensor_events_to_ipmi(SaHpiEventStateT ev)
{
	SaHpiEventStateT newev = 0;

	if (ev & SAHPI_ES_NON_REDUNDANT_INSUFFICIENT_RESOURCES) {
		newev |= 0x01;
	}
	if (ev & SAHPI_ES_NON_REDUNDANT_SUFFICIENT_RESOURCES) {
		newev |= 0x06;
	}
	if (ev & SAHPI_ES_FULLY_REDUNDANT) {
		newev |= 0x08;
	}
	return newev;
}




static SaErrorT get_ipmb0_sensor_reading(
				 struct oh_handler_state *handler,
				 struct ohoi_sensor_info *sinfo, 
				 SaHpiSensorReadingT * reading,
				 SaHpiEventStateT * ev_state)
{
	SaHpiEventStateT tmp_state;
	SaErrorT rv;
	
	rv = orig_get_sensor_reading(handler, sinfo, reading, &tmp_state);
	if (rv != SA_OK) {
		return rv;
	}
	*ev_state = ipmb0_sensor_events_to_hpi(tmp_state);
	return SA_OK;
}



static SaErrorT get_ipmb0_sensor_event_enable(
				      struct oh_handler_state *handler,
				      struct ohoi_sensor_info *sinfo,
				      SaHpiBoolT *enable,
				      SaHpiEventStateT *assert,
				      SaHpiEventStateT *deassert)
{
	SaHpiEventStateT tmp_assert;
	SaHpiEventStateT tmp_deassert;
	SaErrorT rv;
	
	rv = orig_get_sensor_event_enable(handler, sinfo, enable,
					&tmp_assert, &tmp_deassert);
	if (rv != SA_OK) {
		return rv;
	}
	*assert = ipmb0_sensor_events_to_hpi(tmp_assert);
	*deassert = ipmb0_sensor_events_to_hpi(tmp_deassert);
	
	return SA_OK;
}



static SaErrorT set_ipmb0_sensor_event_enable(
				      struct oh_handler_state *handler,
				      struct ohoi_sensor_info *sinfo,
				      SaHpiBoolT enable,
				      SaHpiEventStateT assert,
				      SaHpiEventStateT deassert,
				      unsigned int a_supported,
				      unsigned int d_supported)
{
	return orig_set_sensor_event_enable(handler, sinfo, enable,
			ipmb0_sensor_events_to_ipmi(assert),
			ipmb0_sensor_events_to_ipmi(deassert),
			a_supported, d_supported);
}

void adjust_sensor_to_atcahpi_spec(struct oh_handler_state *handler,
				   SaHpiRptEntryT	*rpt,
				   SaHpiRdrT		*rdr,
				   struct ohoi_sensor_info *sensor_info,
				   ipmi_sensor_t	*sensor)
{
	struct ohoi_handler *ipmi_handler = handler->data;
	SaHpiSensorRecT	*rec = &rdr->RdrTypeUnion.SensorRec;
	
	if (!IS_ATCA(ipmi_handler->d_type)) {
		return;
	}
	if (ipmi_sensor_get_sensor_type(sensor) == 0xf0) {
		// ATCA hot swap sensor
		rec->Type = SAHPI_OEM_SENSOR;
		rec->Category = SAHPI_EC_GENERIC;
		return;
	}
	if (ipmi_sensor_get_sensor_type(sensor) != 0xf1) {
		// not IPMB-0 sensor
		return;
	}

	if (get_ipmb0_sensor_num(handler, sensor, &rec->Num) != 0) {
		err("Couldn't get IPMB-0 sensor link. #%d for resource %d",
			rec->Num, rpt->ResourceId);
		return;
	}
	rec->Type = SAHPI_OEM_SENSOR;
	rec->Category = SAHPI_EC_REDUNDANCY;
	rec->Events = SAHPI_ES_FULLY_REDUNDANT |
			SAHPI_ES_NON_REDUNDANT_SUFFICIENT_RESOURCES |
			SAHPI_ES_NON_REDUNDANT_INSUFFICIENT_RESOURCES;
	sensor_info->ohoii.get_sensor_event_enable =
					get_ipmb0_sensor_event_enable;
	sensor_info->ohoii.set_sensor_event_enable =
					set_ipmb0_sensor_event_enable;
	sensor_info->ohoii.get_sensor_reading = get_ipmb0_sensor_reading;
}








		/*
		 *    IPMB-0 State Control
		 */



	 
struct set_ipmb0_state_control_s {
	unsigned char a;
	unsigned char b;
	unsigned char chan;
	int done;
	SaErrorT rv;
};




static void _set_ipmb0_state_control_cb(
			ipmi_mc_t *mc,
			ipmi_msg_t *msg,
			void       *rsp_data)
{
	struct set_ipmb0_state_control_s *info = rsp_data;
	int rv = msg->data[0];
	
	
	dbg("set IPMB state response(%d): %02x\n", msg->data_len, rv);
		
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
	}
	info->done = 1;
	return;
};

static void set_ipmb0_state_control_cb(
			ipmi_mc_t *mc,
			void *cb_data)
{
	struct set_ipmb0_state_control_s *info = cb_data;
	unsigned char data[16];
	int rv;


	data[0] = 0;
	data[1] = info->a;
	data[2] = info->b;
	dbg("set IPMB state to MC (%d, %d) : %02x %02x %02x",
		ipmi_mc_get_channel(mc), ipmi_mc_get_address(mc),
		data[0], data[1], data[2]);
	rv = ipmicmd_mc_send(mc,
		0x2c, 0x09, 0, data, 3,
		_set_ipmb0_state_control_cb,
		cb_data);
	if (rv != 0) {
		err("ipmicmd_send = 0x%x", rv);
		OHOI_MAP_ERROR(info->rv, rv);
		info->done =1;
		return;
	}
	return;
}

static SaErrorT set_ipmb0_state_control_state(
                                      struct oh_handler_state *hnd,
                                      struct ohoi_control_info *c,
                                      SaHpiRdrT * rdr,
                                      SaHpiCtrlModeT mode,
                                      SaHpiCtrlStateT *state)
{
	struct set_ipmb0_state_control_s info;
	struct ohoi_handler *ipmi_handler = (struct ohoi_handler *)hnd->data;
	SaErrorT rv;
	struct ohoi_resource_info *res_info;
	unsigned int link_num;
	unsigned char on;

	if (state && state->Type != SAHPI_CTRL_TYPE_ANALOG) {
		err("wrong state Type : %d", state->Type);
		return SA_ERR_HPI_INVALID_DATA;
	}
	res_info = oh_get_resource_data(hnd->rptcache,
				c->info.atcamap_ctrl_info.val);
	if (res_info == NULL) {
		err("No res_info");
		return SA_ERR_HPI_INTERNAL_ERROR;
	}
	if (!(res_info->type & OHOI_RESOURCE_MC)) {
		err("resource not MC");
		return SA_ERR_HPI_INTERNAL_ERROR;
	}
	link_num = state ? state->StateUnion.Analog : 0;
	if (link_num > rdr->RdrTypeUnion.CtrlRec.TypeUnion.Analog.Max) {
		err("Wrong analog value: %d > %d", link_num,
			rdr->RdrTypeUnion.CtrlRec.TypeUnion.Analog.Max);
		return SA_ERR_HPI_INVALID_DATA;
	}
	on = (mode == SAHPI_CTRL_MODE_AUTO) ? 1 : 0;
	if (rdr->RdrTypeUnion.CtrlRec.Num == ATCAHPI_CTRL_NUM_IPMB_A_STATE) {
		info.b = 0xFF;
		info.a = (link_num << 1) | on;
	} else if (rdr->RdrTypeUnion.CtrlRec.Num ==
					ATCAHPI_CTRL_NUM_IPMB_B_STATE) {
		info.a = 0xFF;
		info.b = (link_num << 1) | on;
	} else {
		err("Not IPMB state control: 0x%x",
					rdr->RdrTypeUnion.CtrlRec.Num);
		return SA_ERR_HPI_INTERNAL_ERROR;
	}
	info.done = 0;
	info.rv = SA_OK;

	rv = ipmi_mc_pointer_cb(res_info->u.entity.mc_id,
				set_ipmb0_state_control_cb,
				&info);
	if (rv != 0) {
		err("ipmi_mc_pointer_cb = 0x%x", rv);
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

	return SA_OK;
}



static SaHpiRdrT *create_ipmb0_state_control(
			struct ohoi_handler *ipmi_handler,
			SaHpiRptEntryT *rpt,
			struct ohoi_control_info **ctrl_info,
			int bus_A,
			SaHpiCtrlStateAnalogT max_link
			)
{
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
	rdr->RdrTypeUnion.CtrlRec.Num = bus_A ? ATCAHPI_CTRL_NUM_IPMB_A_STATE :
						ATCAHPI_CTRL_NUM_IPMB_B_STATE;
	rdr->RdrTypeUnion.CtrlRec.OutputType = SAHPI_CTRL_GENERIC;
	rdr->RdrTypeUnion.CtrlRec.Type = SAHPI_CTRL_TYPE_ANALOG;
	rdr->RdrTypeUnion.CtrlRec.TypeUnion.Analog.Min = 0x00;
	rdr->RdrTypeUnion.CtrlRec.TypeUnion.Analog.Max = max_link;
	rdr->RdrTypeUnion.CtrlRec.TypeUnion.Analog.Default = 0x00;
	rdr->RdrTypeUnion.CtrlRec.DefaultMode.ReadOnly = SAHPI_FALSE;
	rdr->RdrTypeUnion.CtrlRec.DefaultMode.Mode = SAHPI_CTRL_MODE_AUTO;
	rdr->RdrTypeUnion.CtrlRec.WriteOnly = SAHPI_TRUE;
	oh_init_textbuffer(&rdr->IdString);
	if (bus_A) {
		oh_append_textbuffer(&rdr->IdString, "IPMB-A State Control");
	} else {
		oh_append_textbuffer(&rdr->IdString, "IPMB-B State Control");
	}

	c_info->ohoii.get_control_state = NULL;
	c_info->ohoii.set_control_state = set_ipmb0_state_control_state;
	c_info->mode = SAHPI_CTRL_MODE_AUTO;
	c_info->info.atcamap_ctrl_info.val = rpt->ResourceId;
	*ctrl_info = c_info;
		
	return rdr;
}



void ohoi_create_ipmb0_controls(struct oh_handler_state *handler,
				ipmi_entity_t *entity,
				unsigned int max)
{
	ipmi_entity_id_t entity_id = ipmi_entity_convert_to_id(entity);
	SaHpiRptEntryT	*rpt;
	struct ohoi_resource_info *res_info;
	SaHpiRdrT *rdr;
	struct ohoi_control_info *ctrl_info;

	rpt = ohoi_get_resource_by_entityid(handler->rptcache, &entity_id);
	if (rpt == NULL) {
		err("couldn't find out resource");
		return;
	}
	res_info =  oh_get_resource_data(handler->rptcache, rpt->ResourceId);
	if (res_info == NULL) {
		err("couldn't find out res_info");
		return;
	}
//printf("   ################   create ipmb0_controls for Resource %d; num links = %d\n", rpt->ResourceId, max);

	rdr = create_ipmb0_state_control(handler->data, rpt, &ctrl_info,
				1, max);
	if (rdr != NULL && (oh_add_rdr(handler->rptcache,
					rpt->ResourceId,
					rdr, ctrl_info, 1) != SA_OK)) {
		err("couldn't add control rdr");
		free(rdr);
		free(ctrl_info);
	} else {
		rpt->ResourceCapabilities |= SAHPI_CAPABILITY_CONTROL |
     						SAHPI_CAPABILITY_RDR;
	}
	
	rdr = create_ipmb0_state_control(handler->data, rpt, &ctrl_info,
				0, max);
	if (rdr != NULL && (oh_add_rdr(handler->rptcache,
					rpt->ResourceId,
					rdr, ctrl_info, 1) != SA_OK)) {
		err("couldn't add control rdr");
		free(rdr);
		free(ctrl_info);
	} else {
		rpt->ResourceCapabilities |= SAHPI_CAPABILITY_CONTROL |
						SAHPI_CAPABILITY_RDR;
		res_info->type |= OHOI_MC_IPMB0_CONTROL_CREATED;
	}
}




		/*
		 *      FRU Management Controller Reset Control
		 */
 

static SaErrorT get_fru_mc_reset_control_state(
                                      struct oh_handler_state *hnd,
                                      struct ohoi_control_info *c,
                                      SaHpiRdrT * rdr,
                                      SaHpiCtrlModeT *mode,
                                      SaHpiCtrlStateT *state)
{
	return SA_ERR_HPI_INVALID_CMD;
}


struct mc_reset_info {
	int done;
	SaErrorT rv;
	SaHpiCtrlStateAnalogT val;
};

static void reset_mc_done (ipmi_mc_t *mc,
			int err,
			void *cb_data)
{
	struct mc_reset_info *info = cb_data;
	info->done = 1;
	if (err) {
		err("reset_mc_done err = %d", err);
		info->rv = SA_ERR_HPI_INVALID_REQUEST;
	}
}



static void set_mc_reset_state(ipmi_mc_t *mc,
                            void           *cb_data)
{
        struct mc_reset_info *info = cb_data;
	int rv;
	int act;

	if (info->val == 1) {
		act = IPMI_MC_RESET_COLD;
	} else if (info->val == 2) {
		act = IPMI_MC_RESET_WARM;
	} else {
		info->rv = SA_ERR_HPI_INVALID_CMD;
		info->done = 1;
		return;
	}		
        rv = ipmi_mc_reset(mc, act, reset_mc_done, cb_data);
	if (rv) {
		err("ipmi_mc_reset returned err = %d", rv);
		info->rv = SA_ERR_HPI_INVALID_REQUEST;
		info->done = 1;
	}
}

static SaErrorT set_fru_mc_reset_control_state(
                                      struct oh_handler_state *hnd,
                                      struct ohoi_control_info *c,
                                      SaHpiRdrT * rdr,
                                      SaHpiCtrlModeT mode,
                                      SaHpiCtrlStateT *state)
{
	struct mc_reset_info info;
	struct ohoi_handler *ipmi_handler = (struct ohoi_handler *)hnd->data;
	SaErrorT rv;
	struct ohoi_resource_info *res_info;
	SaHpiCtrlStateAnalogT val;

	if (mode == SAHPI_CTRL_MODE_AUTO) {
		return SA_ERR_HPI_READ_ONLY;
	}
	if (state->Type != SAHPI_CTRL_TYPE_ANALOG) {
		err("wrong state Type : %d", state->Type);
		return SA_ERR_HPI_INVALID_DATA;
	}
	res_info = oh_get_resource_data(hnd->rptcache,
				c->info.atcamap_ctrl_info.val);
	if (res_info == NULL) {
		err("No res_info");
		return SA_ERR_HPI_INTERNAL_ERROR;
	}
	if (!(res_info->type & OHOI_RESOURCE_MC)) {
		err("resource not MC");
		return SA_ERR_HPI_INTERNAL_ERROR;
	}
	val = state->StateUnion.Analog;
	if ((val != 1) && (val != 2)) {
		err("wrong state value %d", val);
		return SA_ERR_HPI_INVALID_DATA;
	}
	info.done = 0;
	info.rv = SA_OK;
	info.val = val;
	rv = ipmi_mc_pointer_cb(res_info->u.entity.mc_id, 
			set_mc_reset_state, &info);
	if (rv != 0) {
		err("ipmi_mc_pointer_cb = 0x%x", rv);
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

	return SA_OK;
}





static SaHpiRdrT *create_fru_mc_reset_control(
			struct ohoi_handler *ipmi_handler,
			SaHpiRptEntryT *rpt,
			struct ohoi_control_info **ctrl_info
			)
{
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
	rdr->RdrTypeUnion.CtrlRec.Num = ATCAHPI_CTRL_NUM_FRU_IPMC_RESET;
	rdr->RdrTypeUnion.CtrlRec.OutputType = SAHPI_CTRL_GENERIC;
	rdr->RdrTypeUnion.CtrlRec.Type = SAHPI_CTRL_TYPE_ANALOG;
	rdr->RdrTypeUnion.CtrlRec.TypeUnion.Analog.Min = 0x01;
	rdr->RdrTypeUnion.CtrlRec.TypeUnion.Analog.Max = 0x02;
	rdr->RdrTypeUnion.CtrlRec.TypeUnion.Analog.Default = 0x01;
	rdr->RdrTypeUnion.CtrlRec.DefaultMode.ReadOnly = SAHPI_TRUE;
	rdr->RdrTypeUnion.CtrlRec.DefaultMode.Mode = SAHPI_CTRL_MODE_MANUAL;
	rdr->RdrTypeUnion.CtrlRec.WriteOnly = SAHPI_TRUE;
	oh_init_textbuffer(&rdr->IdString);
	oh_append_textbuffer(&rdr->IdString,
				"FRU Management Controller Reset Control");

	c_info->ohoii.get_control_state = get_fru_mc_reset_control_state;
	c_info->ohoii.set_control_state = set_fru_mc_reset_control_state;
	c_info->mode = SAHPI_CTRL_MODE_MANUAL;
	c_info->info.atcamap_ctrl_info.val = rpt->ResourceId;
	*ctrl_info = c_info;
		
	return rdr;
}



void ohoi_create_fru_mc_reset_control(struct oh_handler_state *handler,
                             SaHpiResourceIdT rid)
{
//	struct ohoi_handler *ipmi_handler = handler->data;
	SaHpiRptEntryT *rpt;
	struct ohoi_resource_info *res_info;
	SaHpiRdrT *rdr;
	struct ohoi_control_info *c_info;
	int rv;

	rpt = oh_get_resource_by_id(handler->rptcache, rid);
	if (rpt == NULL) {
		err("No rpt = %d", rid);
		return;
	}
	res_info =  oh_get_resource_data(handler->rptcache, rpt->ResourceId);
	if (res_info == NULL) {
		err("No res_info for rpt = %d", rid);
		return;
	}
	rdr = create_fru_mc_reset_control(handler->data, rpt, &c_info);
	if (rdr == NULL) {
		err("could not create fan control");
		return;
	}
	rv = oh_add_rdr(handler->rptcache,rpt->ResourceId,
					rdr, c_info, 1);
	if (rv != SA_OK) {
		err("couldn't add control rdr");
		free(rdr);
		free(c_info);
		return;
	}
	rpt->ResourceCapabilities |= SAHPI_CAPABILITY_CONTROL |
     						SAHPI_CAPABILITY_RDR;
	res_info->type |= OHOI_MC_RESET_CONTROL_CREATED;
}







		/*
		 *	FAN Control
		 */
static SaErrorT get_fan_control_state(
                                      struct oh_handler_state *hnd,
                                      struct ohoi_control_info *c,
                                      SaHpiRdrT * rdr,
                                      SaHpiCtrlModeT *mode,
                                      SaHpiCtrlStateT *state);

static SaErrorT set_fan_control_state(
                                      struct oh_handler_state *hnd,
                                      struct ohoi_control_info *c,
                                      SaHpiRdrT * rdr,
                                      SaHpiCtrlModeT mode,
                                      SaHpiCtrlStateT *state);


struct fan_control_s {
	unsigned char min;
	unsigned char max;
	unsigned char normal;
	unsigned char local;
	unsigned char device_id;
	SaErrorT rv;
	int done;
};

static void get_fan_speed_properties_done(
			ipmi_mc_t *mc,
			ipmi_msg_t *msg,
			void       *rsp_data)
{
	struct fan_control_s *info = rsp_data;
	
	
	dbg("get fan speed properties response(%d): %02x %02x %02x "
		"%02x %02x %02x\n",
		msg->data_len, msg->data[0], msg->data[1], msg->data[2],
		msg->data[3], msg->data[4], msg->data[5]);
		
	if (mc == NULL) {
		info->rv = SA_ERR_HPI_INVALID_PARAMS;
		info->done = 1;
		return;
	}
	if (msg->data[0] != 0) {
		info->rv = SA_ERR_HPI_INVALID_PARAMS;
	} else {
		info->min = msg->data[2];
		info->max = msg->data[3];
		info->normal = msg->data[4];
	}
	info->done = 1;
};



static void get_fan_speed_properties(ipmi_mc_t *mc, void *cb_data)
{
	struct fan_control_s *info = cb_data;
	unsigned char data[16];
	int rv;


	info->device_id = 0;
	data[0] = 0;
	data[1] = (unsigned char)info->device_id;

	dbg("get fan properties (%d, %d) : %02x %02x",
		ipmi_mc_get_channel(mc), ipmi_mc_get_address(mc),
		data[0], data[1]);
	rv = ipmicmd_mc_send(mc,
		0x2c, 0x14, 0, data, 2,
		get_fan_speed_properties_done,
		cb_data);
	if (rv != 0) {
		err("ipmicmd_send = 0x%x", rv);
		info->rv = rv;
		info->done = 1;
		return;
	}
	return;
}



static SaHpiRdrT *create_fan_control(
			struct oh_handler_state *handler,
			SaHpiRptEntryT *rpt,
			struct ohoi_control_info **ctrl_info
			)
{
	SaHpiRdrT *rdr;
	struct ohoi_control_info *c_info;
	struct ohoi_resource_info *res_info;
//	struct ohoi_handler *ipmi_handler = handler->data;
	struct fan_control_s info;
	int rv;
	
	res_info = oh_get_resource_data(handler->rptcache, rpt->ResourceId);
	if (res_info == NULL) {
		err("res_info == NULL ?");
		return NULL;
	}
	if (!(res_info->type & OHOI_RESOURCE_MC)) {
		err("only intelligent fru supported now");
		return NULL;
	}
	
	// determine max, min, default values

	info.rv = SA_OK;
	info.done = 0;
	rv = ipmi_mc_pointer_cb(res_info->u.entity.mc_id,
			get_fan_speed_properties, &info);
	if (rv != 0) {
		err("ipmi_pointer_entity_cb = %d", rv);
		return NULL;
	}
	rv = ohoi_loop(&info.done, handler->data);
	if (rv != SA_OK) {
		err("ohoi_loop = %d", rv);
		return NULL;
	}
	if (info.rv != 0) {
		err("info.rv = %d", info.rv);
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
	rdr->RdrTypeUnion.CtrlRec.Num = ATCAHPI_CTRL_NUM_FAN_SPEED;
	rdr->RdrTypeUnion.CtrlRec.OutputType = SAHPI_CTRL_FAN_SPEED;
	rdr->RdrTypeUnion.CtrlRec.Type = SAHPI_CTRL_TYPE_ANALOG;
	rdr->RdrTypeUnion.CtrlRec.TypeUnion.Analog.Min = info.min;
	rdr->RdrTypeUnion.CtrlRec.TypeUnion.Analog.Max = info.max;
	rdr->RdrTypeUnion.CtrlRec.TypeUnion.Analog.Default = info.normal;
	rdr->RdrTypeUnion.CtrlRec.DefaultMode.ReadOnly = SAHPI_TRUE;
	rdr->RdrTypeUnion.CtrlRec.DefaultMode.Mode = SAHPI_CTRL_MODE_AUTO;
	rdr->RdrTypeUnion.CtrlRec.WriteOnly = SAHPI_FALSE;
	oh_init_textbuffer(&rdr->IdString);
	oh_append_textbuffer(&rdr->IdString, "Fan Control");

	c_info->ohoii.get_control_state = get_fan_control_state;
	c_info->ohoii.set_control_state = set_fan_control_state;
	c_info->mode = SAHPI_CTRL_MODE_AUTO;
	c_info->info.atcamap_ctrl_info.val = rpt->ResourceId;
	*ctrl_info = c_info;
		
	return rdr;
}


void ohoi_create_fan_control(struct oh_handler_state *handler,
                             SaHpiResourceIdT rid)
{
//	struct ohoi_handler *ipmi_handler = handler->data;
	SaHpiRptEntryT *rpt;
	SaHpiRdrT *rdr;
	struct ohoi_control_info *c_info;
	int rv;

	rpt = oh_get_resource_by_id(handler->rptcache, rid);
	if (rpt == NULL) {
		err("No rpt = %d", rid);
		return;
	}
	rdr = create_fan_control(handler, rpt, &c_info);
	if (rdr == NULL) {
		err("could not create fan control");
		return;
	}
	rv = oh_add_rdr(handler->rptcache,rpt->ResourceId,
					rdr, c_info, 1);
	if (rv != SA_OK) {
		err("couldn't add control rdr");
		free(rdr);
		free(c_info);
		return;
	}
	rpt->ResourceCapabilities |= SAHPI_CAPABILITY_CONTROL |
     						SAHPI_CAPABILITY_RDR;
}

static void get_fan_control_state_done(
			ipmi_mc_t *mc,
			ipmi_msg_t *msg,
			void       *rsp_data)
{
	struct fan_control_s *info = rsp_data;
	
	
	dbg("get fan level response(%d): %02x %02x %02x %02x\n",
		msg->data_len, msg->data[0], msg->data[1], msg->data[2],
		msg->data[3]);
		
	if (mc == NULL) {
		info->rv = SA_ERR_HPI_INVALID_PARAMS;
		info->done = 1;
		return;
	}
	if (msg->data[0] != 0) {
		info->rv = SA_ERR_HPI_INVALID_PARAMS;
	} else {
		info->local = msg->data[3];
		info->normal = msg->data[2];
	}
	info->done = 1;
};



static void get_fan_control_state_cb(ipmi_mc_t *mc, void *cb_data)
{
	struct fan_control_s *info = cb_data;
	unsigned char data[16];
	int rv;


	info->device_id = 0;
	data[0] = 0;
	data[1] = (unsigned char)info->device_id;

	dbg("get fan level (%d, %d) : %02x %02x",
		ipmi_mc_get_channel(mc), ipmi_mc_get_address(mc),
		data[0], data[1]);
	rv = ipmicmd_mc_send(mc,
		0x2c, 0x16, 0, data, 2,
		get_fan_control_state_done,
		cb_data);
	if (rv != 0) {
		err("ipmicmd_send = 0x%x", rv);
		info->rv = rv;
		info->done = 1;
		return;
	}
	return;
}


static SaErrorT get_fan_control_state(
                                      struct oh_handler_state *handler,
                                      struct ohoi_control_info *c,
                                      SaHpiRdrT * rdr,
                                      SaHpiCtrlModeT *mode,
                                      SaHpiCtrlStateT *state)
{
//	struct ohoi_handler *ipmi_handler = handler->data;
	struct ohoi_resource_info *res_info;
	struct fan_control_s info;
	int rv;

	if (state == NULL) {
		goto no_state;
	}

	
	res_info = oh_get_resource_data(handler->rptcache,
				c->info.atcamap_ctrl_info.val);
	if (res_info == NULL) {
		err("res_info == NULL ?");
		return SA_ERR_HPI_INVALID_RESOURCE;
	}
	if (!(res_info->type & OHOI_RESOURCE_MC)) {
		err("only intelligent fru supported now");
		return SA_ERR_HPI_UNSUPPORTED_API;
	}
	

	info.rv = SA_OK;
	info.done = 0;
	rv = ipmi_mc_pointer_cb(res_info->u.entity.mc_id,
			get_fan_control_state_cb, &info);
	if (rv != 0) {
		err("ipmi_pointer_entity_cb = %d", rv);
		return SA_ERR_HPI_ENTITY_NOT_PRESENT;
	}
	rv = ohoi_loop(&info.done, handler->data);
	if (rv != SA_OK) {
		err("ohoi_loop = %d", rv);
		return SA_ERR_HPI_ENTITY_NOT_PRESENT;
	}
	if (info.rv != 0) {
		err("info.rv = %d", info.rv);
		return SA_ERR_HPI_ENTITY_NOT_PRESENT;
	}
	state->Type = SAHPI_CTRL_TYPE_ANALOG;
	state->StateUnion.Analog = info.normal;

no_state :
	if (mode) {
		*mode = c->mode;
	}
	return SA_OK;
}



static SaErrorT set_fan_control_state(
                                      struct oh_handler_state *hnd,
                                      struct ohoi_control_info *c,
                                      SaHpiRdrT * rdr,
                                      SaHpiCtrlModeT mode,
                                      SaHpiCtrlStateT *state)
{

	if (mode == SAHPI_CTRL_MODE_AUTO) {
		return SA_OK;
	}
	return SA_ERR_HPI_READ_ONLY;
}




		/*
		 *   Ekeying Link State Sensor
		 */
		 

static SaErrorT get_ekeying_link_state_sensor_event_enable(
					    struct oh_handler_state *hnd,
					    struct ohoi_sensor_info *sinfo,
					    SaHpiBoolT   *enable,
					    SaHpiEventStateT  *assert,
					    SaHpiEventStateT  *deassert)
{
	*assert = 0;
	*deassert = 0;
	*enable = 0;
	return SA_OK;
}


typedef struct {
	unsigned char channel;
	SaHpiUint8T *buffer;
	SaHpiEventStateT state;
	int done;
	SaErrorT rv;
} ekey_sen_reading_info_t;





static void get_ekeying_link_state_sensor_reading_done(
			ipmi_mc_t *mc,
			ipmi_msg_t *msg,
			void       *rsp_data)
{
	ekey_sen_reading_info_t *info = rsp_data;
	int rv = msg->data[0];
	unsigned char *data = msg->data;
	SaHpiUint8T *buffer = info->buffer;
	unsigned char link_enabled = 0;
	unsigned char *buf;
	unsigned char chn;
	unsigned char ip;
	unsigned char chns[4] = {0, 0, 0, 0};
	unsigned char order[4] = {0, 0, 0, 0}; 
	
	dbg("get IPMB state response(%d): %02x\n", msg->data_len, rv);
	//printf("get IPMB state response: %02x %02x %02x %02x %02x %02x %02x\n",
	//data[0], data[1], data[2], data[3], data[4], data[05], data[6]);  
		
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
	}
	if (rv != 0) {
		info->done = 1;
		return;
	}
	if (msg->data_len < 7) {
		info->rv = SA_ERR_HPI_NO_RESPONSE;
		goto out;
	}

	memset(buffer, 0, SAHPI_SENSOR_BUFFER_LENGTH);
	// first link
	ip = 2; // index to ports for first channel
	buf = data + 2;
	chn = CHANNEL_NUM(buf);
	if (chn == 0 || chn > 16) {
		err("wrong channel %d for Link 1", chn);
		info->rv = SA_ERR_HPI_NO_RESPONSE;
		goto out;
	}
	if (chn <= 8) {
		buffer[0] = 1 << (chn - 1);
	} else {
		buffer[1] = 1 << (chn - 9);
	}
	order[0] = ip;
	buffer[5] = LINK_GROUPING_ID(buf);
	buffer[4] = LINK_TYPE_EXTENSION(buf);
	buffer[3] = LINK_TYPE(buf);
	buffer[2] = INTERFACE_TYPE(buf);
	link_enabled = data[6];

	
	if (msg->data_len < 12) {
		goto out;
	}
	// second link
	ip = 7; // index to ports for second channel
	buf = data + 7;
	chn = CHANNEL_NUM(buf);
	if (chn == 0 || chn > 16) {
		err("wrong channel %d for Link 2", chn);
		info->rv = SA_ERR_HPI_NO_RESPONSE;
		goto out;
	}
	if (chn > chns[0]) {
		chns[1] = chn;
		order[1] = ip;
	} else {
		chns[1] = chns[0];
		order[1] = order[0];
		chns[0] = chn;
		order[0] = ip;
	}
	if (chn <= 8) {
		buffer[0] |= 1 << (chn - 1);
	} else {
		buffer[1] |= 1 << (chn - 9);
	}
	link_enabled |= data[11];
	
	if (msg->data_len < 17) {
		goto out;
	}
	// third link
	ip = 12; // index to ports for third channel
	buf = data + 12;
	chn = CHANNEL_NUM(buf);
	if (chn == 0 || chn > 16) {
		err("wrong channel %d for Link 2", chn);
		info->rv = SA_ERR_HPI_NO_RESPONSE;
		goto out;
	}
	if (chn > chns[1]) {
		chns[2] = chn;
		order[2] = ip;
	} else {
		chns[2] = chns[1];
		order[2] = order[1];
		if (chn > chns[0]) {
			chns[1] = chn;
			order[1] = ip;
		} else {
			chns[1] = chns[0];
			order[1] = order[0];
			chns[0] = chn;
			order[0] = ip;
		}
	}
	if (chn <= 8) {
		buffer[0] |= 1 << (chn - 1);
	} else {
		buffer[1] |= 1 << (chn - 9);
	}
	link_enabled |= data[16];

	if (msg->data_len < 22) {
		goto out;
	}
	// forth link
	ip = 17; // index to ports for forth cahannel
	buf = data + 17;
	chn = CHANNEL_NUM(buf);
	if (chn == 0 || chn > 16) {
		err("wrong channel %d for Link 2", chn);
		info->rv = SA_ERR_HPI_NO_RESPONSE;
		goto out;
	}
	if (chn > chns[2]) {
		chns[3] = chn;
		order[3] = ip;
	} else {
		chns[3] = chns[2];
		order[3] = order[2];
		if (chn > chns[1]) {
			chns[2] = chn;
			order[2] = ip;
		} else {
			chns[2] = chns[1];
			order[2] = order[1];
			if (chn > chns[0]) {
				chns[1] = chn;
				order[1] = ip;
			} else {
				chns[1] = chns[0];
				order[1] = order[0];
				chns[0] = chn;
				order[0] = ip;
			}
		}
	}
	if (chn <= 8) {
		buffer[0] |= 1 << (chn - 1);
	} else {
		buffer[1] |= 1 << (chn - 9);
	}
	link_enabled |= data[21];
	
	// It will be fine to check all links have the same
	// gpouping_id, link_type, link_type_extension, interface_type
	// and enable bit. XXXX
          
out:
	if (info->rv == 0) {
		// now fill in port map fields
		if (order[0] > 0) {
			buffer[6] = PORTS(data + order[0]);
		}
		if (order[1] > 0) {
			buffer[6] |= (PORTS(data + order[1]) << 4);
		}
		if (order[2] > 0) {
			buffer[7] = PORTS(data + order[2]);
		}
		if (order[3] > 0) {
			buffer[7] |= (PORTS(data + order[3]) << 4);
		}
		info->state = link_enabled ?
			SAHPI_ES_ENABLED : SAHPI_ES_DISABLED;
	}
#if DEBUG_EKEY
printf("E-KEY READING: port map = %02x%02x%02x%02x%02x%02x%02x%02x\n"
       "               Link Grouping Id %02x; Link Type Extension %02x; "
       "Link Type %02x; Intrface Type %02x; Channel Map %02x%02x\n",
       buffer[13], buffer[12], buffer[11], buffer[10], buffer[ 9], buffer[ 8],
       buffer[ 7], buffer[ 6], buffer[ 5], buffer[ 4], buffer[ 3], buffer[ 2],
       buffer[ 1], buffer[ 0]);
#endif   
	info->done = 1;
};

static void get_ekeying_link_state_sensor_reading_cb(ipmi_mc_t *mc,
						     void *cb_data)
{
	ekey_sen_reading_info_t *info = cb_data;
	unsigned char data[16];
	int rv;


	data[0] = 0;
	data[1] = info->channel;
	dbg("Get Port State to MC (%d, %d) : %02x %02x",
		ipmi_mc_get_channel(mc), ipmi_mc_get_address(mc),
		data[0], data[1]);
	rv = ipmicmd_mc_send(mc,
		0x2c, 0x0f, 0, data, 2,
		get_ekeying_link_state_sensor_reading_done,
		cb_data);
	if (rv != 0) {
		err("ipmicmd_send = 0x%x", rv);
		OHOI_MAP_ERROR(info->rv, rv);
		info->done = 1;
		return;
	}
	return;
}

static SaErrorT get_ekeying_link_state_sensor_reading(
				       struct oh_handler_state *handler,
				       struct ohoi_sensor_info *sensor_info,
				       SaHpiSensorReadingT *reading,
				       SaHpiEventStateT *ev_state)
{
	SaHpiRptEntryT *rpt;
	struct ohoi_resource_info *res_info;
	SaHpiRdrT *rdr;
	ekey_sen_reading_info_t info;
	int rv;
	SaHpiUint8T *buf;
	int i;

	rpt = oh_get_resource_by_id(handler->rptcache,
				sensor_info->info.atcamap_sensor_info.val);
	if (rpt == NULL) {
		err("no rpt for resource Id %d",
			sensor_info->info.atcamap_sensor_info.val);
		return SA_ERR_HPI_ENTITY_NOT_PRESENT;
	}
	rdr = ohoi_get_rdr_by_data(handler->rptcache, rpt->ResourceId,
					SAHPI_SENSOR_RDR, sensor_info);
	if (rdr == NULL) {
		err("no rdr for sensor.  Rpt %d, sen_info = %p",
					rpt->ResourceId, sensor_info);
		return SA_ERR_HPI_ENTITY_NOT_PRESENT;
	}
	res_info =  oh_get_resource_data(handler->rptcache, rpt->ResourceId);
	if (res_info == NULL) {
		err("no res_info");
		return SA_ERR_HPI_ENTITY_NOT_PRESENT;
	}
	if (!(res_info->type & OHOI_RESOURCE_MC)) {
		err("resource %d not MC", rpt->ResourceId);
		return SA_ERR_HPI_ENTITY_NOT_PRESENT;
	}
	buf = rdr->RdrTypeUnion.SensorRec.DataFormat.Range.
						Nominal.Value.SensorBuffer;
	// calculate the least channel number
	for (i = 0; i < 8; i++) {
		if (buf[0] & (1 << i)) {
			break;
		}
	}
	if (i == 8) {
		for (i = 0; i < 16; i++) {
			if (buf[1] & (1 << (i - 8))) {
				break;
			}
		}
	}
	if (i == 16) {
		err("No channels for link");
		return SA_ERR_HPI_ERROR;
	}
	info.channel = (i + 1) | (buf[2] << 6);
	info.buffer = reading->Value.SensorBuffer;
	info.state = 0;
	info.done = 0;
	info.rv = SA_OK;
	
	rv = ipmi_mc_pointer_cb(res_info->u.entity.mc_id,
				get_ekeying_link_state_sensor_reading_cb,
				&info);
	if (rv != 0) {
		err("ipmi_mc_pointer_cb = 0x%x", rv);
		return SA_ERR_HPI_INVALID_CMD;
	}
	rv = ohoi_loop(&info.done, handler->data);
	if (rv != SA_OK) {
		err("ohoi_loop = 0x%x", rv);
		return rv;
	}
	if (info.rv != SA_OK) {
		err("info.rv = 0x%x", info.rv);
		return rv;
	}
	if (reading) {
		memcpy(reading->Value.SensorBuffer + 14,
			rdr->RdrTypeUnion.SensorRec.DataFormat.Range.
				Nominal.Value.SensorBuffer + 14, 16); 
		reading->Type = SAHPI_SENSOR_READING_TYPE_BUFFER;
		reading->IsSupported = SAHPI_TRUE;
	}
	if (ev_state) {
		*ev_state = info.state;
	}
	return SA_OK;
}
		 
void ohoi_create_ekeying_link_state_sensor(
			struct oh_handler_state *handler,
			ipmi_entity_t	*entity,
			unsigned int	s_num,
			unsigned char	*guid,
			unsigned char	link_grouping_id,
			unsigned char	link_type,
			unsigned char	link_type_extension,
			unsigned char	interface_type,
			unsigned char	*channels)
{
//printf("Create EKEYING sensor %d for %d. (%d,%d,%d,%d). chan: 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\n", s_num, ipmi_entity_get_device_address(entity), 
//link_grouping_id, link_type, link_type_extension, interface_type,
//channels[0], channels[1], channels[2], channels[3], channels[4], channels[5], channels[6], channels[7], //channels[8], channels[9], channels[10], channels[11], channels[12], channels[13], channels[14], channels[15]);

	SaHpiRptEntryT *rpt;
	struct ohoi_resource_info *res_info;
	ipmi_entity_id_t entity_id = ipmi_entity_convert_to_id(entity);
	SaHpiRdrT *rdr;
	struct ohoi_sensor_info *s_info;
	SaHpiUint8T *buffer;

	rpt = ohoi_get_resource_by_entityid(handler->rptcache, &entity_id);
	if (rpt == NULL) {
		err("Couldn't find out resource by entity %d.%.d.%d.%d  %s",
			ipmi_entity_get_entity_id(entity), 
			ipmi_entity_get_entity_instance(entity),
			ipmi_entity_get_device_channel(entity),
			ipmi_entity_get_device_address(entity),
			ipmi_entity_get_entity_id_string(entity));
		return;
	}
	res_info =  oh_get_resource_data(handler->rptcache, rpt->ResourceId);
	if (res_info == NULL) {
		err("No res_info for resource id = %d", rpt->ResourceId);
		return;
	}

	rdr = malloc(sizeof (*rdr));
	if (rdr == NULL) {
		err("Out of memory");
		return;
	}
	s_info = malloc(sizeof (*s_info));
	if (rdr == NULL) {
		err("Out of memory");
		free(rdr);
		return;
	}

	memset(rdr, 0, sizeof (*rdr));
	memset(s_info, 0, sizeof (*s_info));

	rdr->RdrType = SAHPI_SENSOR_RDR;
	rdr->IsFru = SAHPI_FALSE;
	rdr->Entity = rpt->ResourceEntity;
	rdr->RdrTypeUnion.SensorRec.Num =
				OHOI_FIRST_EKEYING_SENSOR_NUM + s_num;
	rdr->RdrTypeUnion.SensorRec.Type = SAHPI_RESERVED1;
	rdr->RdrTypeUnion.SensorRec.Category = SAHPI_EC_ENABLE;
	rdr->RdrTypeUnion.SensorRec.EnableCtrl = SAHPI_FALSE;
	rdr->RdrTypeUnion.SensorRec.EventCtrl = SAHPI_SEC_PER_EVENT;
	rdr->RdrTypeUnion.SensorRec.Events =
					SAHPI_ES_ENABLED | SAHPI_ES_DISABLED;
	rdr->RdrTypeUnion.SensorRec.ThresholdDefn.IsAccessible = SAHPI_FALSE;
	rdr->RdrTypeUnion.SensorRec.DataFormat.IsSupported = SAHPI_TRUE;
	rdr->RdrTypeUnion.SensorRec.DataFormat.ReadingType =
					SAHPI_SENSOR_READING_TYPE_BUFFER;
	rdr->RdrTypeUnion.SensorRec.DataFormat.BaseUnits =
							SAHPI_SU_UNSPECIFIED;
	rdr->RdrTypeUnion.SensorRec.DataFormat.ModifierUnits =
							SAHPI_SU_UNSPECIFIED;
	rdr->RdrTypeUnion.SensorRec.DataFormat.ModifierUse = SAHPI_SMUU_NONE;
	rdr->RdrTypeUnion.SensorRec.DataFormat.Percentage = SAHPI_FALSE;
	rdr->RdrTypeUnion.SensorRec.DataFormat.Range.Flags = SAHPI_SRF_NOMINAL;
	
	buffer = rdr->RdrTypeUnion.SensorRec.DataFormat.Range.
						Nominal.Value.SensorBuffer;
	
	int i, chn = 0, first_channel = 0;
	for (i = 0; i < 15; i++) {
		if (channels[i] == 0) {
			continue;
		}
		if (chn % 2) {
			buffer[6 + chn / 2] |= (channels[i] << 4);
		} else {
			buffer[6 + chn / 2] = channels[i] & 0x0f;
		}
		if (i >= 8) {
			buffer[1] |= 1 << (i - 8);
		} else {
			buffer[0] |= 1 << i;
		}
		chn++;
		if (first_channel == 0) {
			first_channel = i + 1;
		}
	}
	if (first_channel == 0) {
		err("No channels for sensor");
		free(rdr);
		free(s_info);
		return;
	}
	buffer[2] = interface_type;
	buffer[3] = link_type;
	buffer[4] = link_type_extension;
	buffer[5] = link_grouping_id;
	
	if (guid) {
		memcpy(&buffer[14], guid, 16);
	}
	
	oh_init_textbuffer(&rdr->IdString);
	char name[256], *nm;
	nm = name;
	strncpy(name, "E-Keying Link State: ", 256);
	snprintf(name, 256, "E-Keying Link State: %d Intrface, Link Type %d,"
		" Link Type Ext %d Channel %d", interface_type, link_type,
		link_type_extension, first_channel);
	oh_append_textbuffer(&rdr->IdString, name);

	s_info->support_assert = 0;
	s_info->support_deassert = 0;
	s_info->assert = 0;
	s_info->deassert = 0;
	s_info->sen_enabled = SAHPI_TRUE;
        s_info->enable = SAHPI_FALSE;
	s_info->info.atcamap_sensor_info.data = rpt;
	s_info->info.atcamap_sensor_info.val = rpt->ResourceId;
//			((interface_type & 3) << 6) | (first_channel & 0x3f);
	s_info->type = OHOI_SENSOR_ATCA_MAPPED;

	s_info->ohoii.get_sensor_event_enable =
		get_ekeying_link_state_sensor_event_enable;
	s_info->ohoii.set_sensor_event_enable = NULL;
	s_info->ohoii.get_sensor_reading =
		get_ekeying_link_state_sensor_reading;
	s_info->ohoii.get_sensor_thresholds = NULL;
	s_info->ohoii.set_sensor_thresholds = NULL;
	
	if (oh_add_rdr(handler->rptcache, rpt->ResourceId,
					rdr, s_info, 1) != SA_OK) {
		err("could not add e-keying link state sensor to rpt id = %d",
							      rpt->ResourceId);
		free(rdr);
		free(s_info);
		return;
	}
	rpt->ResourceCapabilities |= SAHPI_CAPABILITY_SENSOR |
     						SAHPI_CAPABILITY_RDR;
	return;
}




		/*
		 *  RPT iterator. It is called after all scannings done.
		 *  Create nessesary RDRs are not created yet
		 */

static int ipmb0_state_control_rdr_iterator(
			     struct oh_handler_state *handler,
			     SaHpiRptEntryT *rpt,
                             SaHpiRdrT      *rdr,
			     void *cb_data)
{
	int *max = cb_data;
	int num;
	
	if (rdr->RdrType != SAHPI_SENSOR_RDR) {
		return 0;
	}
	num = (int)rdr->RdrTypeUnion.SensorRec.Num;
	if ((num < ATCAHPI_SENSOR_NUM_IPMB0) ||
			(num > ATCAHPI_SENSOR_NUM_IPMB0 + 0x5F)) {
		// not IPMB-0 sensor
		return 0;
	}
	if (num > *max + ATCAHPI_SENSOR_NUM_IPMB0) {
		*max = num - ATCAHPI_SENSOR_NUM_IPMB0;
	}
	return 0;
}


/*--------------------- FRU Reset and Diagnostic Control --------------------*/

struct reset_ctrl_state_s {
	SaHpiInt32T	state;
};

static void _set_atca_reset_diagnostic_control_state_cb(
					ipmi_mc_t *mc,
					ipmi_msg_t *msg,
					void *rsp_data)
{
	atca_common_info_t		*info = rsp_data;
	int				rv = msg->data[0];

	if (mc == NULL) {
		info->rv = SA_ERR_HPI_ENTITY_NOT_PRESENT;
		info->done = 1;
		return;
	}
	if (rv == 0xc1) {
		info->rv = SA_ERR_HPI_INVALID_CMD;
	} else if (rv == 0xc3) {
		info->rv = SA_ERR_HPI_NO_RESPONSE;
	} else if (rv == 0xcc) {
		info->rv = SA_ERR_HPI_INVALID_REQUEST;
	} else if (rv != 0) {
		info->rv = SA_ERR_HPI_INVALID_PARAMS;
	} 
	info->done = 1;
	return;
};

static void set_atca_reset_diagnostic_control_state_cb(ipmi_mc_t *mc,
						       void *cb_data)
{
	atca_common_info_t		*info = cb_data;
	struct reset_ctrl_state_s	*ctrl_state = info->info;
	unsigned char			data[25];
	int				rv;

	memset(data, 0, 25);
	data[0] = IPMI_PICMG_GRP_EXT;
	data[1] = info->devid;
	data[2] = ctrl_state->state;

	rv = ipmicmd_mc_send(mc,
		IPMI_GROUP_EXTENSION_NETFN,
		IPMI_PICMG_CMD_FRU_CONTROL,
		0, data, 3,
		_set_atca_reset_diagnostic_control_state_cb,
		cb_data);
	if (rv != 0) {
		err("ipmicmd_send = 0x%x\n", rv);
		OHOI_MAP_ERROR(info->rv, rv);
		return;
	}
	return;
}

static SaErrorT set_atca_reset_diagnostic_control_state(
                                      struct oh_handler_state *hnd,
                                      struct ohoi_control_info *c_info,
                                      SaHpiRdrT *rdr,
                                      SaHpiCtrlModeT mode,
                                      SaHpiCtrlStateT *state)
{
	struct ohoi_handler		*ipmi_handler = (struct ohoi_handler *)
								hnd->data;
	SaHpiRptEntryT			*rpt;
	struct	ohoi_resource_info	*res_info,	*slot_info;
	atca_common_info_t		info;
	struct reset_ctrl_state_s	ctrl_state;
	int				rv;

	if (state == NULL) {
		return SA_ERR_HPI_INVALID_DATA;
	}

	if (state->Type != SAHPI_CTRL_TYPE_ANALOG) {
		return SA_ERR_HPI_INVALID_DATA;
	}

	rpt = oh_get_resource_by_id(hnd->rptcache,
				    c_info->info.atcamap_ctrl_info.rid);
	if (rpt == NULL) {
		return SA_ERR_HPI_INTERNAL_ERROR;
	}

	res_info = oh_get_resource_data(hnd->rptcache,
					c_info->info.atcamap_ctrl_info.rid);
	if (res_info == NULL) {
		return SA_ERR_HPI_INTERNAL_ERROR;
	}
	
	slot_info = oh_get_resource_data(hnd->rptcache, 
					 ohoi_get_parent_id(rpt));
	if (slot_info == NULL) {
		return SA_ERR_HPI_INTERNAL_ERROR;
	}

	info.done = 0;
	info.rv = SA_OK;
	info.addr = slot_info->u.slot.addr;
	info.devid = slot_info->u.slot.devid;
	ctrl_state.state = state->StateUnion.Analog + 1;
	info.info = &ctrl_state;
	rv = ipmi_mc_pointer_cb(res_info->u.entity.mc_id,
				set_atca_reset_diagnostic_control_state_cb,
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

	return info.rv;
}

static SaHpiRdrT *atca_create_reset_diagnostic_control(
				struct oh_handler_state *handler,
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
	rdr->RdrTypeUnion.CtrlRec.Num = ATCAHPI_CTRL_NUM_FRU_CONTROL;
	rdr->RdrTypeUnion.CtrlRec.OutputType = SAHPI_CTRL_GENERIC;
	rdr->RdrTypeUnion.CtrlRec.Type = SAHPI_CTRL_TYPE_ANALOG;
	rdr->RdrTypeUnion.CtrlRec.TypeUnion.Analog.Min = 1;
	rdr->RdrTypeUnion.CtrlRec.TypeUnion.Analog.Max = 2;
	rdr->RdrTypeUnion.CtrlRec.TypeUnion.Analog.Default = 1;
	rdr->RdrTypeUnion.CtrlRec.DefaultMode.Mode = SAHPI_CTRL_MODE_MANUAL;
	rdr->RdrTypeUnion.CtrlRec.DefaultMode.ReadOnly = SAHPI_TRUE;
	rdr->RdrTypeUnion.CtrlRec.WriteOnly = SAHPI_TRUE;
	oh_init_textbuffer(&rdr->IdString);
	oh_append_textbuffer(&rdr->IdString,
				"FRU Reboot and Diagnostic Control");

	l_c_info->ohoii.get_control_state = NULL;
	l_c_info->ohoii.set_control_state =
				set_atca_reset_diagnostic_control_state;
	l_c_info->mode = SAHPI_CTRL_MODE_MANUAL;
	l_c_info->type = OHOI_CTRL_ATCA_MAPPED;

	*c_info = l_c_info;

	return rdr;
}

/*------------------------- Desired Power Control ----------------------------*/

struct desired_pwr_ctrl_s {
	SaHpiInt32T	des_pwr;
};

static void _get_atca_desired_power_control_state_cb(
					ipmi_mc_t *mc,
					ipmi_msg_t *msg,
					void *rsp_data)
{
	atca_common_info_t		*info = rsp_data;
	int				rv = msg->data[0];
	struct desired_pwr_ctrl_s	*des_pwr =
				(struct desired_pwr_ctrl_s *)info->info;

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
		des_pwr->des_pwr = (SaHpiFloat64T)
			(msg->data[msg->data_len - 1] * msg->data[4]);
	}
	info->done = 1;
	return;
};

static void get_atca_desired_power_control_state_cb(ipmi_mc_t *mc,
						    void *cb_data)
{
	atca_common_info_t		*info = cb_data;
	unsigned char			data[25];
	int				rv;

	memset(data, 0, 25);
	data[0] = IPMI_PICMG_GRP_EXT;
	data[1] = info->devid;
	data[2] = 1;	/* Desired steady state draw levels */

	rv = ipmicmd_mc_send(mc,
		IPMI_GROUP_EXTENSION_NETFN,
		IPMI_PICMG_CMD_GET_POWER_LEVEL,
		0, data, 3,
		_get_atca_desired_power_control_state_cb,
		cb_data);
	if (rv != 0) {
		err("ipmicmd_send = 0x%x\n", rv);
		OHOI_MAP_ERROR(info->rv, rv);
		return;
	}
	return;
}

static SaErrorT get_atca_desired_power_control_state(
				struct oh_handler_state *hnd,
				struct ohoi_control_info *c_info,
				SaHpiRdrT * rdr,
				SaHpiCtrlModeT *mode,
				SaHpiCtrlStateT *state)
{
	struct ohoi_handler		*ipmi_handler = (struct ohoi_handler *)
								hnd->data;
	SaHpiRptEntryT			*rpt;
	struct	ohoi_resource_info	*res_info,	*slot_info;
	atca_common_info_t		info;
	struct desired_pwr_ctrl_s	des_pwr;
	int				rv;

	rpt = oh_get_resource_by_id(hnd->rptcache,
				    c_info->info.atcamap_ctrl_info.rid);
	if (rpt == NULL) {
		return SA_ERR_HPI_INTERNAL_ERROR;
	}

	res_info = oh_get_resource_data(hnd->rptcache,
					c_info->info.atcamap_ctrl_info.rid);
	if (res_info == NULL) {
		return SA_ERR_HPI_INTERNAL_ERROR;
	}
	
	slot_info = oh_get_resource_data(hnd->rptcache, 
					 ohoi_get_parent_id(rpt));
	if (slot_info == NULL) {
		return SA_ERR_HPI_INTERNAL_ERROR;
	}

	info.done = 0;
	info.rv = 0;
	info.addr = slot_info->u.slot.addr;
	info.devid = slot_info->u.slot.devid;
	info.info = &des_pwr;
	rv = ipmi_mc_pointer_cb(res_info->u.entity.mc_id,
				get_atca_desired_power_control_state_cb,
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


	if (mode) {
		*mode = c_info->mode = SAHPI_CTRL_MODE_AUTO;
	}
	
	if (state == NULL) {
		return SA_OK;
	}

	state->Type = SAHPI_CTRL_TYPE_ANALOG;
	state->StateUnion.Analog = des_pwr.des_pwr;

	return SA_OK;
}

static SaErrorT set_atca_desired_power_control_state(
                                      struct oh_handler_state *hnd,
                                      struct ohoi_control_info *c_info,
                                      SaHpiRdrT *rdr,
                                      SaHpiCtrlModeT mode,
                                      SaHpiCtrlStateT *state)
{
	return SA_ERR_HPI_READ_ONLY;
}

static SaHpiRdrT *atca_create_desired_pwr_control(
				struct oh_handler_state *handler,
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
	rdr->RdrTypeUnion.CtrlRec.Num = ATCAHPI_CTRL_NUM_DESIRED_PWR;
	rdr->RdrTypeUnion.CtrlRec.OutputType = SAHPI_CTRL_GENERIC;
	rdr->RdrTypeUnion.CtrlRec.Type = SAHPI_CTRL_TYPE_ANALOG;
	rdr->RdrTypeUnion.CtrlRec.TypeUnion.Analog.Min = 0;
	rdr->RdrTypeUnion.CtrlRec.TypeUnion.Analog.Max = 400;
	rdr->RdrTypeUnion.CtrlRec.TypeUnion.Analog.Default = 0;
	rdr->RdrTypeUnion.CtrlRec.DefaultMode.Mode = SAHPI_CTRL_MODE_AUTO;
	rdr->RdrTypeUnion.CtrlRec.DefaultMode.ReadOnly = SAHPI_TRUE;
	rdr->RdrTypeUnion.CtrlRec.WriteOnly = SAHPI_FALSE;
	oh_init_textbuffer(&rdr->IdString);
	oh_append_textbuffer(&rdr->IdString, "FRU Desired Power");

	l_c_info->ohoii.get_control_state =
				get_atca_desired_power_control_state;
	l_c_info->ohoii.set_control_state =
				set_atca_desired_power_control_state;
	l_c_info->mode = SAHPI_CTRL_MODE_AUTO;
	l_c_info->type = OHOI_CTRL_ATCA_MAPPED;

	*c_info = l_c_info;

	return rdr;
}

/*---------------------------------------------------------------------------*/

static int fru_rdrs_rpt_iterator(
			     struct oh_handler_state *handler,
			     SaHpiRptEntryT *rpt,
                             struct ohoi_resource_info *res_info,
			     void *cb_data)
{
	int max = -1;
	SaHpiRdrT *rdr;
	struct ohoi_control_info *ctrl_info;
	
	if (!(res_info->type & OHOI_RESOURCE_MC) ||
		(res_info->type & OHOI_MC_RESET_CONTROL_CREATED)) {
		goto no_reset_control;
	}
	// Create FRU Management Controller Reset Control
	rdr = create_fru_mc_reset_control(handler->data, rpt, &ctrl_info);
	if (rdr != NULL && (oh_add_rdr(handler->rptcache, rpt->ResourceId,
						rdr, ctrl_info, 1) != SA_OK)) {
		err("couldn't add control rdr");
		free(rdr);
		free(ctrl_info);
	} else {
		rpt->ResourceCapabilities |= SAHPI_CAPABILITY_CONTROL |
     						SAHPI_CAPABILITY_RDR;
		res_info->type |= OHOI_MC_RESET_CONTROL_CREATED;
	}
	
no_reset_control:
	if (res_info->type & OHOI_MC_IPMB0_CONTROL_CREATED) {
		goto no_ipmb0_controls;
	}

	// calculate the number of IPMB links 
	ohoi_iterate_rpt_rdrs(handler, rpt,
		ipmb0_state_control_rdr_iterator, &max);
	if (max < 0) {
		err("No ipmb0 sensors for resource %d", rpt->ResourceId);
		res_info->type |= OHOI_MC_IPMB0_CONTROL_CREATED;
		goto no_ipmb0_controls;
	}

	rdr = create_ipmb0_state_control(handler->data, rpt, &ctrl_info,
				1, max);
	if (rdr != NULL && (oh_add_rdr(handler->rptcache,
					rpt->ResourceId,
					rdr, ctrl_info, 1) != SA_OK)) {
		err("couldn't add control rdr");
		free(rdr);
		free(ctrl_info);
	} else {
		rpt->ResourceCapabilities |= SAHPI_CAPABILITY_CONTROL |
     						SAHPI_CAPABILITY_RDR;
	}
	
	rdr = create_ipmb0_state_control(handler->data, rpt, &ctrl_info,
				0, max);
	if (rdr != NULL && (oh_add_rdr(handler->rptcache,
					rpt->ResourceId,
					rdr, ctrl_info, 1) != SA_OK)) {
		err("couldn't add control rdr");
		free(rdr);
		free(ctrl_info);
	} else {
		rpt->ResourceCapabilities |= SAHPI_CAPABILITY_CONTROL |
     						SAHPI_CAPABILITY_RDR;
		res_info->type |= OHOI_MC_RESET_CONTROL_CREATED;
	}
no_ipmb0_controls:

	if (!(rpt->ResourceCapabilities & SAHPI_CAPABILITY_MANAGED_HOTSWAP)) {
		goto no_fru_control;
	}

	rdr = atca_create_desired_pwr_control(handler, &ctrl_info);
	if (rdr) {
		if (oh_add_rdr(handler->rptcache, rpt->ResourceId,
					rdr, ctrl_info, 1) != SA_OK) {
			err("couldn't add control rdr");
			free(rdr);
			free(ctrl_info);
		} else {
			rpt->ResourceCapabilities |= 
					SAHPI_CAPABILITY_CONTROL |
						SAHPI_CAPABILITY_RDR;
			ctrl_info->info.atcamap_ctrl_info.rid =
							rpt->ResourceId;
		}
	}

	rdr = atca_create_reset_diagnostic_control(handler, &ctrl_info);
	if (rdr) {
		if (oh_add_rdr(handler->rptcache, rpt->ResourceId,
					rdr, ctrl_info, 1) != SA_OK) {
			err("couldn't add control rdr");
			free(rdr);
			free(ctrl_info);
		} else {
			rpt->ResourceCapabilities |= 
					SAHPI_CAPABILITY_CONTROL |
						SAHPI_CAPABILITY_RDR;
			ctrl_info->info.atcamap_ctrl_info.rid =
							rpt->ResourceId;
		}
	}

no_fru_control:

	return 0;
}


void ohoi_atca_create_fru_rdrs(struct oh_handler_state *handler)
{
	ohoi_iterate_rptcache(handler, fru_rdrs_rpt_iterator, NULL);
}

void ohoi_atca_delete_fru_rdrs(struct oh_handler_state *handler,
                             ipmi_mcid_t mcid)
{
	SaHpiRptEntryT *rpt;
	struct ohoi_resource_info *res_info;
	SaHpiRdrT *rdr;
	SaHpiSensorNumT num;

	rpt = ohoi_get_resource_by_mcid(handler->rptcache, &mcid);
	if (rpt == NULL) {
		err("Can't delete mc rdrs. rpt == NULL");
		return;
	}
	res_info = oh_get_resource_data(handler->rptcache,
                                             rpt->ResourceId);
	if (res_info == NULL) {
		err("res_info == NULL");
		return;
	}
	
	// Delete FRU Management Controller Reset Control
	if (!(res_info->type & OHOI_MC_RESET_CONTROL_CREATED)) {
		goto no_reset_control;
	}
	rdr = oh_get_rdr_by_type(handler->rptcache, rpt->ResourceId,
			SAHPI_CTRL_RDR, ATCAHPI_CTRL_NUM_FRU_IPMC_RESET);
	if (rdr) {
		oh_remove_rdr(handler->rptcache, rpt->ResourceId,
							rdr->RecordId);
	} else {
		err("No rdr for FRU Management Controller Reset Control");
	}
	res_info ->type &= ~OHOI_MC_RESET_CONTROL_CREATED;
	
no_reset_control:
	if (!(res_info->type & OHOI_MC_IPMB0_CONTROL_CREATED)) {
		goto no_ipmb0_controls;
	}
	rdr = oh_get_rdr_by_type(handler->rptcache, rpt->ResourceId,
			SAHPI_CTRL_RDR, ATCAHPI_CTRL_NUM_IPMB_A_STATE);
	if (rdr) {
		oh_remove_rdr(handler->rptcache, rpt->ResourceId,
							rdr->RecordId);
	} else {
		err("No rdr for ATCAHPI_CTRL_NUM_IPMB_A_STATE");
	}
	rdr = oh_get_rdr_by_type(handler->rptcache, rpt->ResourceId,
			SAHPI_CTRL_RDR, ATCAHPI_CTRL_NUM_IPMB_B_STATE);
	if (rdr) {
		oh_remove_rdr(handler->rptcache, rpt->ResourceId,
							rdr->RecordId);
	} else {
		err("No rdr for ATCAHPI_CTRL_NUM_IPMB_B_STATE");
	}
	for (num = ATCAHPI_SENSOR_NUM_IPMB0;
			num < ATCAHPI_SENSOR_NUM_IPMB0 + 0x5F; num++) {
		rdr = oh_get_rdr_by_type(handler->rptcache, rpt->ResourceId,
						SAHPI_SENSOR_RDR, num);
		if (rdr) {
			oh_remove_rdr(handler->rptcache, rpt->ResourceId,
							rdr->RecordId);
		}
	}
	res_info ->type &= ~OHOI_MC_IPMB0_CONTROL_CREATED;

no_ipmb0_controls:
	if (!(rpt->ResourceCapabilities & SAHPI_CAPABILITY_MANAGED_HOTSWAP)) {
		goto no_fru_control;
	}

	rdr = oh_get_rdr_by_type(handler->rptcache, rpt->ResourceId,
			SAHPI_CTRL_RDR, ATCAHPI_CTRL_NUM_DESIRED_PWR);
	if (rdr) {
		oh_remove_rdr(handler->rptcache, rpt->ResourceId,
							rdr->RecordId);
	} else {
		err("No rdr for ATCAHPI_CTRL_NUM_DESIRED_PWR");
	}

	rdr = oh_get_rdr_by_type(handler->rptcache, rpt->ResourceId,
			SAHPI_CTRL_RDR, ATCAHPI_CTRL_NUM_FRU_CONTROL);
	if (rdr) {
		oh_remove_rdr(handler->rptcache, rpt->ResourceId,
							rdr->RecordId);
	} else {
		err("No rdr for ATCAHPI_CTRL_NUM_FRU_CONTROL");
	}

no_fru_control:
	if (!ohoi_rpt_has_sensors(handler, rpt->ResourceId)) {
		rpt->ResourceCapabilities &= ~SAHPI_CAPABILITY_SENSOR;
	}
	if (!ohoi_rpt_has_controls(handler, rpt->ResourceId)) {
		rpt->ResourceCapabilities &= ~SAHPI_CAPABILITY_CONTROL;
	}
	if ((oh_get_rdr_next(handler->rptcache, rpt->ResourceId,
					 SAHPI_FIRST_ENTRY) == NULL) &&
					 (res_info->fru == NULL)) {
		// no more rdrs for rpt
		rpt->ResourceCapabilities &= ~SAHPI_CAPABILITY_RDR; 
	}
	entity_rpt_set_updated(res_info, handler->data);
}






