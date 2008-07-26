/*      -*- linux-c -*-
 *
 * Copyright (c) 2003 by Intel Corp.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  This
 * file and program are licensed under a BSD style license.  See
 * the Copying file included with the OpenHPI distribution for
 * full licensing terms.
 *
 * Authors:
 *     Louis Zhuang <louis.zhuang@linux.intel.com>
 */

#include "ipmi.h"
#include <oh_utils.h>
#include <string.h>

static inline int domain_id_is_equal(const ipmi_domain_id_t id1,
                                     const ipmi_domain_id_t id2)
{
        return (id1.domain == id2.domain);
}

static inline int entity_id_is_equal(const ipmi_entity_id_t id1, 
                                     const ipmi_entity_id_t id2)
{
	return (domain_id_is_equal(id1.domain_id, id2.domain_id)
                && (id1.entity_id == id2.entity_id)
                && (id1.entity_instance == id2.entity_instance)
                && (id1.channel == id2.channel)
                && (id1.address == id2.address)
                && (id1.seq == id2.seq));
}

static inline int mc_id_is_equal(const ipmi_mcid_t id1,
                                 const ipmi_mcid_t id2)
{
//printf("compare MC: (%d,%d,%ld) and (%d,%d,%ld)\n", id1.mc_num, id1.channel, id1.seq, id2.mc_num, id2.channel, id2.seq);
        return (domain_id_is_equal(id1.domain_id, id2.domain_id)
                && (id1.mc_num == id2.mc_num)
                && (id1.channel== id2.channel)
                && (id1.seq == id2.seq));
}

static inline int ohoi_resource_info_is_equal(
                const struct ohoi_resource_info info1,
                const struct ohoi_resource_info info2)
{
     /* We don't compare rdr_count */
     
	if (info1.type & OHOI_RESOURCE_MC) {
		if (!(info2.type & OHOI_RESOURCE_MC)) {
			return 0;
		}
		return mc_id_is_equal(info1.u.entity.mc_id,
				info2.u.entity.mc_id);
	}

        if (info1.type & OHOI_RESOURCE_ENTITY) {
		return (entity_id_is_equal(info1.u.entity.entity_id, 
                                                   info2.u.entity.entity_id));
	} else if (info1.type & OHOI_RESOURCE_SLOT) {
		return (entity_id_is_equal(info1.u.slot.entity_id, 
                                                   info2.u.slot.entity_id));
	} else {
               err("UNKNOWN OHOI RESOURCE TYPE!");
               return 0;
        }
}

SaHpiResourceIdT ohoi_get_parent_id(SaHpiRptEntryT *child)
{
	SaHpiEntityPathT	ep;
	int			i;

	if (child == NULL)
		return 0;

	for (i = 1; i < SAHPI_MAX_ENTITY_PATH; i ++) {
		ep.Entry[i - 1].EntityLocation =
				child->ResourceEntity.Entry[i].EntityLocation;
		ep.Entry[i - 1].EntityType =
				child->ResourceEntity.Entry[i].EntityType;
		if (child->ResourceEntity.Entry[i].EntityType ==
							SAHPI_ENT_ROOT) {
			break;
		}
	}
	return oh_uid_lookup(&ep);
}

/*XXX algorithm here is so ulgy! */
SaHpiRptEntryT *ohoi_get_resource_by_entityid(RPTable                *table,
                                              const ipmi_entity_id_t *entity_id)
{
        struct ohoi_resource_info res_info1;
        SaHpiRptEntryT *rpt_entry;
        
        res_info1.type            = OHOI_RESOURCE_ENTITY;
        res_info1.u.entity.entity_id     = *entity_id;
        
        rpt_entry = oh_get_resource_next(table, SAHPI_FIRST_ENTRY);
        while (rpt_entry) {
                struct ohoi_resource_info *ohoi_res_info;
                ohoi_res_info = oh_get_resource_data(table, rpt_entry->ResourceId);
                if (ohoi_resource_info_is_equal(res_info1, *ohoi_res_info)) {
                        return rpt_entry;
		}
                rpt_entry = oh_get_resource_next(table, 
                                                 rpt_entry->ResourceId);            
        }

	err("Not found resource by entity_id");

        return NULL;
}


/*XXX algorithm here is so ulgy! */
SaHpiRptEntryT *ohoi_get_resource_by_mcid(RPTable                *table,
                                          const ipmi_mcid_t *mc_id)
{
        struct ohoi_resource_info res_info1;
        SaHpiRptEntryT *rpt_entry;
        
        res_info1.type            = OHOI_RESOURCE_MC;
        res_info1.u.entity.mc_id     = *mc_id;
        
        rpt_entry = oh_get_resource_next(table, SAHPI_FIRST_ENTRY);
        while (rpt_entry) {
                struct ohoi_resource_info *ohoi_res_info;
                ohoi_res_info = oh_get_resource_data(table, rpt_entry->ResourceId);
                if (ohoi_resource_info_is_equal(res_info1, *ohoi_res_info)) {
                        return rpt_entry;
		}
                rpt_entry = oh_get_resource_next(table, 
                                                 rpt_entry->ResourceId);            
        }

	err("Not found resource by mc_id");

        return NULL;
}

SaErrorT ohoi_get_rdr_data(const struct oh_handler_state *handler,
                           SaHpiResourceIdT              id,
                           SaHpiRdrTypeT                 type,
                           SaHpiSensorNumT               num,
                           void                          **pdata)
{
        SaHpiRdrT * rdr;
        rdr = oh_get_rdr_by_type(handler->rptcache,
                                 id,
                                 type,
                                 num);
        if (!rdr) {
		err("no rdr for Resource %d. type = %d, num = %d", id, type, num);
                /*XXX No err code for invalid rdr?*/
                return SA_ERR_HPI_INVALID_RESOURCE;
        }

        *pdata = oh_get_rdr_data(handler->rptcache,
                                 id,
                                 rdr->RecordId);
        return SA_OK;
}




SaHpiRdrT *ohoi_get_rdr_by_data(RPTable *table,
                                SaHpiResourceIdT rid,
                                SaHpiRdrTypeT  type, 
                                void  *data)
{
	SaHpiRdrT *rdr;
	struct ohoi_sensor_info *s_info;

	if (!data) {
		err("data == NULL");
		return NULL;
	}
	if (type != SAHPI_SENSOR_RDR) {
		err("type != SAHPI_SENSOR_RDR");
		return NULL;
	}
        
	rdr = oh_get_rdr_next(table, rid, SAHPI_FIRST_ENTRY);

	while (rdr) {
		ipmi_sensor_id_t *sidp;
		if (rdr->RdrType != SAHPI_SENSOR_RDR) {
			goto next_rdr;
		}
		s_info = oh_get_rdr_data(table, rid, rdr->RecordId);
		if (!s_info) {
			err("s_info == NULL");
			goto next_rdr;
		}
		if (s_info->type == OHOI_SENSOR_ATCA_MAPPED) {
			if ((struct ohoi_sensor_info *)data == s_info) {
				return rdr;
			}
			goto next_rdr;
		}
		sidp = &s_info->info.orig_sensor_info.sensor_id;
		if (!ipmi_cmp_sensor_id(*(ipmi_sensor_id_t *)data, *sidp)) {
			return rdr;
		}
next_rdr:
		rdr = oh_get_rdr_next(table, rid, rdr->RecordId);
	}
	return NULL;
}

void ohoi_iterate_rptcache(struct oh_handler_state *handler,
			   rpt_loop_handler_cb func, void *cb_data)
{
	RPTable *table = handler->rptcache;
        struct ohoi_resource_info *res_info;
        SaHpiRptEntryT *rpt_entry;

        
        rpt_entry = oh_get_resource_next(table, SAHPI_FIRST_ENTRY);
        while (rpt_entry) {
                res_info = oh_get_resource_data(table, rpt_entry->ResourceId);
                if (func(handler, rpt_entry, res_info, cb_data)) {
                        return;
		}
                rpt_entry = oh_get_resource_next(table, 
                                                 rpt_entry->ResourceId);            
        }
}

void ohoi_iterate_rpt_rdrs(struct oh_handler_state *handler,
			   SaHpiRptEntryT *rpt,
			   rdr_loop_handler_cb func, void *cb_data)
{
	SaHpiRdrT *rdr;
	RPTable *table = handler->rptcache;
        
	rdr = oh_get_rdr_next(table, rpt->ResourceId, SAHPI_FIRST_ENTRY);

	while (rdr) {
		if (func(handler, rpt, rdr, cb_data)) {
			break;
		}
		rdr = oh_get_rdr_next(table, rpt->ResourceId, rdr->RecordId);
	}
	return;
}

int ohoi_delete_orig_sensor_rdr(struct oh_handler_state *handler,
			   SaHpiRptEntryT *rpt,
			   ipmi_sensor_id_t *mysid)
{
	SaHpiRdrT *rdr;
	SaHpiRdrT *rdr_todelete = NULL;
	RPTable *table = handler->rptcache;
	ipmi_sensor_id_t *sid;
	struct ohoi_sensor_info *s_info;
	int more_sensors = 0;
	
	rdr = oh_get_rdr_next(table, rpt->ResourceId, SAHPI_FIRST_ENTRY);
        
	while (rdr) {
		if (rdr_todelete && more_sensors) {
			break;
		}
		if (rdr->RdrType != SAHPI_SENSOR_RDR) {
			goto next_rdr;
		}
		s_info = oh_get_rdr_data(table, rpt->ResourceId,
							rdr->RecordId);
		if (!s_info) {
			err("s_info == NULL");
			goto next_rdr;
		}
		if (rdr_todelete) {
			more_sensors = 1;
			break;
		}
		if (s_info->type == OHOI_SENSOR_ATCA_MAPPED) {
			more_sensors = 1;
			goto next_rdr;
		}

		sid = &s_info->info.orig_sensor_info.sensor_id;
		if (!ipmi_cmp_sensor_id(*mysid, *sid)) {
			rdr_todelete = rdr;
			goto next_rdr;
		}
next_rdr:
		rdr = oh_get_rdr_next(table, rpt->ResourceId, rdr->RecordId);
	}
	if (rdr_todelete) {
		oh_remove_rdr(table, rpt->ResourceId, rdr_todelete->RecordId);
	} else {
		err("Sensor %d for rpt %d not deleted", mysid->sensor_num, rpt->ResourceId);
	}
	return !more_sensors;
}



int ohoi_delete_orig_control_rdr(struct oh_handler_state *handler,
			   SaHpiRptEntryT *rpt,
			   ipmi_control_id_t *mycid)
{
	SaHpiRdrT *rdr;
	SaHpiRdrT *rdr_todelete = NULL;
	RPTable *table = handler->rptcache;
	ipmi_control_id_t *cid;
	struct ohoi_control_info *c_info;
	int more_controls = 0;
	
	rdr = oh_get_rdr_next(table, rpt->ResourceId, SAHPI_FIRST_ENTRY);
        
	while (rdr) {
		if (rdr->RdrType != SAHPI_CTRL_RDR) {
			goto next_rdr;
		}
		c_info = oh_get_rdr_data(table, rpt->ResourceId,
							rdr->RecordId);
		if (!c_info) {
			err("c_info == NULL");
			goto next_rdr;
		}
		if (c_info->type == OHOI_CTRL_ATCA_MAPPED) {
			more_controls = 1;
			goto next_rdr;
		}

		cid = &c_info->info.orig_ctrl_info.ctrl_id;
		if (!ipmi_cmp_control_id(*mycid, *cid)) {
			rdr_todelete = rdr;
			goto next_rdr;
		}
		more_controls = 1;
next_rdr:
	
		rdr = oh_get_rdr_next(table, rpt->ResourceId, rdr->RecordId);
		if (rdr_todelete) {
			oh_remove_rdr(table, rpt->ResourceId, rdr_todelete->RecordId);
			rdr_todelete = NULL;
		}
		// can be more than one control with the same cid. don't break
		
	}
	return !more_controls;
}

int ohoi_rpt_has_sensors(struct oh_handler_state *handler,
                         SaHpiResourceIdT rid)
{
	SaHpiRdrT *rdr;
	RPTable *table = handler->rptcache;
	
	rdr = oh_get_rdr_next(table, rid, SAHPI_FIRST_ENTRY);
        
	while (rdr) {
		if (rdr->RdrType == SAHPI_SENSOR_RDR) {
			return 1;
		}
		rdr = oh_get_rdr_next(table, rid, rdr->RecordId);
	}
	return 0;
}



int ohoi_rpt_has_controls(struct oh_handler_state *handler,
                         SaHpiResourceIdT rid)
{
	SaHpiRdrT *rdr;
	RPTable *table = handler->rptcache;
	
	rdr = oh_get_rdr_next(table, rid, SAHPI_FIRST_ENTRY);
        
	while (rdr) {
		if (rdr->RdrType == SAHPI_CTRL_RDR) {
			return 1;
		}
		rdr = oh_get_rdr_next(table, rid, rdr->RecordId);
	}
	return 0;
}

