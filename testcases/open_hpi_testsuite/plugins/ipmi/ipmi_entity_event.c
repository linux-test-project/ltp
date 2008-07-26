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
#include<oh_domain.h>
#include <oh_utils.h>
#include <string.h>




static void trace_ipmi_entity(char *str, int inst, ipmi_entity_t *entity)
{
	if (!getenv("OHOI_TRACE_ENTITY") && !IHOI_TRACE_ALL) {
		return;
	}

	char *type;
	char logical[24];

	logical[0] = 0;
	switch (ipmi_entity_get_type(entity)) {
	case IPMI_ENTITY_UNKNOWN:
		type = "UNKNOWN"; break;
	case IPMI_ENTITY_MC:
		type = "MC"; break;
	case IPMI_ENTITY_FRU:
		type = "FRU";
		if (ipmi_entity_get_is_logical_fru(entity)) {
			snprintf(logical, 24, " Logical (%d) ",
			   ipmi_entity_get_fru_device_id(entity));
		} else {
			snprintf(logical, 24, " NonLogic(%d) ",
			   ipmi_entity_get_fru_device_id(entity));
		}
		break;
	case IPMI_ENTITY_GENERIC:
		type = "GENERIC"; break;
	case IPMI_ENTITY_EAR:
		type = "EAR"; break;
	case IPMI_ENTITY_DREAR:
		type = "DREAR"; break;
	default :
		type = "INVALID"; break;
	}

	fprintf(stderr, "*** Entity %s %s %s: %d (%d.%d.%d.%d) (%s)   entity = %p\n",
		type, logical, str,
		inst,
		ipmi_entity_get_entity_id(entity),
		ipmi_entity_get_entity_instance(entity),
		ipmi_entity_get_device_channel(entity),
		ipmi_entity_get_device_address(entity),
		ipmi_entity_get_entity_id_string(entity),
		entity);
}


void entity_rpt_set_updated(struct ohoi_resource_info *res_info,
		struct ohoi_handler *ipmi_handler)
{
	g_static_rec_mutex_lock(&ipmi_handler->ohoih_lock);
	if (!res_info->presence) {
		g_static_rec_mutex_unlock(&ipmi_handler->ohoih_lock);
		return;
	}
	res_info->updated = 1;
	ipmi_handler->updated = 1;
	g_static_rec_mutex_unlock(&ipmi_handler->ohoih_lock);
}

void entity_rpt_set_presence(struct ohoi_resource_info *res_info,
		struct ohoi_handler *ipmi_handler, int present)
{
	g_static_rec_mutex_lock(&ipmi_handler->ohoih_lock);
	trace_ipmi("res_info %p: old presence %d, new presence %d",
		res_info, res_info->presence, present);
	if (present == res_info->presence) {
		g_static_rec_mutex_unlock(&ipmi_handler->ohoih_lock);
		return;
	}
	res_info->presence =  present;
	res_info->updated = 1;
	ipmi_handler->updated = 1;
	g_static_rec_mutex_unlock(&ipmi_handler->ohoih_lock);
}



int entity_presence(ipmi_entity_t		*entity,
			    int			present,
			    void		*cb_data,
			    ipmi_event_t	*event)
{
	struct oh_handler_state *handler = (struct oh_handler_state *)cb_data;
	struct ohoi_handler *ipmi_handler = handler->data;

	SaHpiRptEntryT  *rpt;
	SaHpiResourceIdT rid;
	ipmi_entity_id_t ent_id;
  	struct ohoi_resource_info	*res_info;

	ent_id = ipmi_entity_convert_to_id(entity);

	g_static_rec_mutex_lock(&ipmi_handler->ohoih_lock);
	rpt = ohoi_get_resource_by_entityid(handler->rptcache, &ent_id);
	if (!rpt) {
		trace_ipmi_entity("SET PRESENCE. NO RPT", present, entity);
		err("No rpt");
		g_static_rec_mutex_unlock(&ipmi_handler->ohoih_lock);
		return SA_ERR_HPI_NOT_PRESENT;
	}
	rid = rpt->ResourceId;
	if (!(rpt->ResourceCapabilities & SAHPI_CAPABILITY_FRU) && !present) {
		// This is a workaround
		trace_ipmi_entity("PRESENCE HANDLER CALLED FOR NOT FRU ENTITY",
					present, entity);
		g_static_rec_mutex_unlock(&ipmi_handler->ohoih_lock);
		return SA_ERR_HPI_NOT_PRESENT;
	}
	res_info = oh_get_resource_data(handler->rptcache, rid);
	if (res_info->presence == present) {
		g_static_rec_mutex_unlock(&ipmi_handler->ohoih_lock);
		return SA_OK;
	}
	trace_ipmi_entity(present ? "PRESENT" : "NOT PRESENT",
						present, entity);

	if (present && res_info->deleted) {
		// became not present and present again.
		res_info->deleted = 0;
		rpt->ResourceFailed = SAHPI_FALSE;
	}

	if (IS_ATCA(ipmi_handler->d_type)) {
		switch (ipmi_entity_get_entity_id(entity)) {
		case 0xa0: // Blade
			atca_slot_state_sensor_event_send(handler,
							  rpt,
							  present);
			break;
		case 0xf0: // Shelf Manager
			if ((ipmi_entity_get_device_channel(entity) == 0) &&
				(ipmi_entity_get_device_address(entity) == 32)) {
				// Virtual ShM. Do nothing. It cannot be.
				break;
			}
			if (present) {
				ipmi_handler->shmc_present_num++;
			} else {
				if (rpt->ResourceFailed) {
					// it's already marked
					break;
				}
				ipmi_handler->shmc_present_num--;
			}
			if (ipmi_handler->fully_up) {
				ohoi_send_vshmgr_redundancy_sensor_event(
						handler, present);
			}
			break;
		case 0x1e:	// Fan Tray
			if (present) {
				ohoi_create_fan_control(handler, rpt->ResourceId);
			}
			break;
		default:
			break;
		}
	}


	entity_rpt_set_presence(res_info, handler->data,  present);

	if (!present) {
		res_info->deleted = 1;
		// send event to infrastructure but don't
		// touch our local structures while
                struct oh_event *e = malloc(sizeof(*e));
		if (e != NULL) {
			SaHpiEventUnionT *u = &e->event.EventDataUnion;
                	memset(e, 0, sizeof(*e));
                	e->resource = *rpt;
			e->event.Source = rpt->ResourceId;
			e->event.Severity = rpt->ResourceSeverity;
			oh_gettimeofday(&e->event.Timestamp);
                	if (rpt->ResourceCapabilities & SAHPI_CAPABILITY_FRU) {
                		e->event.EventType = SAHPI_ET_HOTSWAP;
                		u->HotSwapEvent.HotSwapState =
                			SAHPI_HS_STATE_NOT_PRESENT;
				u->HotSwapEvent.PreviousHotSwapState =
                			SAHPI_HS_STATE_ACTIVE;
			} else {
				e->event.EventType = SAHPI_ET_RESOURCE;
				u->ResourceEvent.ResourceEventType =
					SAHPI_RESE_RESOURCE_FAILURE;
			}

                	e->hid = handler->hid;
                        oh_evt_queue_push(handler->eventq, e);
		} else {
			err("Out of memory");
		}
#if 0
        	while (SA_OK == oh_remove_rdr(handler->rptcache, rid,
							SAHPI_FIRST_ENTRY));
		ohoi_delete_rpt_fru(res_info);
		res_info->type = OHOI_RESOURCE_ENTITY;
		// XXX free inventory area memory
#endif
	}

	g_static_rec_mutex_unlock(&ipmi_handler->ohoih_lock);

	return SA_OK;
}


static void append_parent_epath(SaHpiRptEntryT	*entry, SaHpiRptEntryT	*parent)
{
	const SaHpiEntityPathT *ep = &(parent->ResourceEntity);
	oh_concat_ep(&entry->ResourceEntity, ep);
}


static void init_rpt(SaHpiRptEntryT	*entry)
{
	int i;

	entry->ResourceInfo.ResourceRev = 0;
	entry->ResourceInfo.SpecificVer = 0;
	entry->ResourceInfo.DeviceSupport = 0;
	entry->ResourceInfo.ManufacturerId = 0;
	entry->ResourceInfo.ProductId = 0;
	entry->ResourceInfo.FirmwareMajorRev = 0;
	entry->ResourceInfo.FirmwareMinorRev = 0;
	entry->ResourceInfo.AuxFirmwareRev = 0;

	entry->EntryId = 0;
	entry->ResourceCapabilities = SAHPI_CAPABILITY_RESOURCE;
	entry->HotSwapCapabilities = 0;

	for (i=0;i<15;i++) {
		entry->ResourceInfo.Guid[i] = 0;
	}
	entry->ResourceSeverity = SAHPI_MAJOR;	/* Default Value -- not related to IPMI */
	entry->ResourceFailed = SAHPI_FALSE;
	oh_init_textbuffer(&entry->ResourceTag);
}


#if 0
static void _get_is_sel(ipmi_mc_t *mc, void *cb_data)
{
	int *is_selp = cb_data;
	*is_selp = ipmi_mc_sel_device_support(mc);
printf("~~~~~~  MC (%d,%d). sel_support = %d\n", ipmi_mc_get_channel(mc),
                 ipmi_mc_get_address(mc), *is_selp);
}
#endif

static void update_resource_capabilities(ipmi_entity_t	*entity,
					 SaHpiRptEntryT	*entry,
					 struct ohoi_resource_info *res_info)
{
	if (ipmi_entity_supports_managed_hot_swap(entity)) {
		trace_ipmi("Entity is hot swapable");
		entry->ResourceCapabilities |= SAHPI_CAPABILITY_MANAGED_HOTSWAP;
		/* if entity supports managed hot swap
		 * check if it has indicator */

		/* we need only return value from function */
		if (!ipmi_entity_get_hot_swap_indicator(entity, NULL, NULL)) {
			trace_ipmi("setting SAHPI_HS_CAPABILITY_INDICATOR_SUPPORTED");
			entry->HotSwapCapabilities |= SAHPI_HS_CAPABILITY_INDICATOR_SUPPORTED;
		}
	} else {
		entry->ResourceCapabilities &= ~SAHPI_CAPABILITY_MANAGED_HOTSWAP;
		entry->HotSwapCapabilities &= ~SAHPI_HS_CAPABILITY_INDICATOR_SUPPORTED;
	}

	/* OpenIPMI used ipmi_entity_hot_swappable to indicate it's FRU
	 * do not use ipmi_entity_get_is_fru()
	 * it's used only for checking if entity has FRU data
	 */
	if ((ipmi_entity_get_entity_id(entity) != 0xf0) ||
			(ipmi_entity_get_device_channel(entity) != 0) ||
			(ipmi_entity_get_device_address(entity) != 32)) {
		// this is not virtual shelf manager
		if(ipmi_entity_hot_swappable(entity)) {
			trace_ipmi("Entity supports simplified hotswap");
			entry->ResourceCapabilities |= SAHPI_CAPABILITY_FRU;
		} else {
			entry->ResourceCapabilities &= ~SAHPI_CAPABILITY_FRU;
		}

	}

	ipmi_mcid_t mc_id;
	if (ipmi_entity_get_mc_id(entity, &mc_id) == 0) {
		res_info->u.entity.mc_id = mc_id;
		res_info->type |= OHOI_RESOURCE_MC;
	} else {
		res_info->type &= ~OHOI_RESOURCE_MC;
	}
}

struct add_parent_ep_s {
	struct oh_handler_state *handler;
	SaHpiRptEntryT	*entry;
};

static void add_parent_ep(ipmi_entity_t *ent, ipmi_entity_t *parent, void *cb_data)
{
	struct add_parent_ep_s *info = cb_data;
	ipmi_entity_id_t parent_id = ipmi_entity_convert_to_id(parent);
	SaHpiRptEntryT *pr_rpt;


	pr_rpt = ohoi_get_resource_by_entityid(info->handler->rptcache, &parent_id);
	if (pr_rpt == NULL) {
		err("       Couldn't find out res-info for parent: %d.%d.%d.%d %s",
			ipmi_entity_get_entity_id(parent),
			ipmi_entity_get_entity_instance(parent),
			ipmi_entity_get_device_channel(parent),
			ipmi_entity_get_device_address(parent),
			ipmi_entity_get_entity_id_string(parent));
		trace_ipmi_entity("CAN NOT FIND OUT PARENT. NO RES_INFO",
			0, parent);

		return;
	}
	append_parent_epath(info->entry, pr_rpt);
	return;
}





static void get_entity_event(ipmi_entity_t	*entity,
			     struct ohoi_resource_info *ohoi_res_info,
			     SaHpiRptEntryT	*entry,
			     void *cb_data)
{
	SaHpiEntityPathT entity_ep;
	struct oh_handler_state *handler = cb_data;
	struct ohoi_handler *ipmi_handler = handler->data;
//	int rv;
	const char *str;
	int entity_id = ipmi_entity_get_entity_id(entity);
	int entity_instance = ipmi_entity_get_entity_instance(entity);
	SaHpiEntityPathT ep;
	unsigned char slot_n_str[8];
	int no_slot = 1;
	unsigned int slot_val = 0;


	init_rpt(entry);

	entry->ResourceEntity.Entry[0].EntityType = entity_id;
	if(entity_instance >= 96) {
		entry->ResourceEntity.Entry[0].EntityLocation =
                				entity_instance- 96;
	} else {
		entry->ResourceEntity.Entry[0].EntityLocation =
                					entity_instance;
	}

	entry->ResourceEntity.Entry[1].EntityType = SAHPI_ENT_ROOT;
	entry->ResourceEntity.Entry[1].EntityLocation = 0;


	update_resource_capabilities(entity, entry, ohoi_res_info);


	if (entry->ResourceEntity.Entry[0].EntityType == SAHPI_ENT_SYSTEM_BOARD) {
		/* This is the BMC entry, so we need to add watchdog. */
		if (!ipmi_handler->islan) {
			// no watchdog commands over lan
			entry->ResourceCapabilities |= SAHPI_CAPABILITY_WATCHDOG;
		}
	}



	if (ipmi_handler->d_type != IPMI_DOMAIN_TYPE_ATCA) {
		goto no_atca;
	}

	if (entry->ResourceEntity.Entry[0].EntityType == SAHPI_ENT_SYSTEM_CHASSIS) {
		entry->ResourceEntity.Entry[0].EntityType = SAHPI_ENT_ROOT;
		entry->ResourceEntity.Entry[0].EntityLocation = 0;
		oh_append_textbuffer(&entry->ResourceTag, "Shelf");
	}

	/* Since OpenIPMI does not hand us a more descriptive
	   tag which is an SDR issue in the chassis really, we'll over-ride
	   it here until things change
	*/

		/*
		 * If entity has a slot try to get it's number
		 */
	no_slot = ipmi_entity_get_physical_slot_num(entity, &slot_val);
	trace_ipmi_entity("  SLOT presence for Entity", no_slot ? 0 : 1, entity);
	if (no_slot) {
		/* will use device address */
		goto end_of_slot;
	}

	{ // create Resource for phisical blade slot if it hasn't already been created
		SaHpiEntityPathT rootep;
		SaHpiRptEntryT *rpt;
		struct ohoi_resource_info *s_r_info;
		char *name;

		switch (entity_id) {
		case 0xa0: // Blade
			ep.Entry[0].EntityType = SAHPI_ENT_PHYSICAL_SLOT;
			name = "Blade Slot ";
			break;
		case 0xf0: // Shelf Manager
			ep.Entry[0].EntityType = ATCAHPI_ENT_SHELF_MANAGER_SLOT;
			name = "Shelf Manager Slot ";
			break;
		case 0xf1: // Filtration Unit
			ep.Entry[0].EntityType = ATCAHPI_ENT_FAN_FILTER_TRAY_SLOT;
			name = "Fan Filter Tray Slot ";
			break;
		case 0x0a: // PEM
			ep.Entry[0].EntityType = ATCAHPI_ENT_POWER_ENTRY_MODULE_SLOT;
			name = "PEM Slot ";
			break;
		case 0xf2: // Shelf FRU
			ep.Entry[0].EntityType = ATCAHPI_ENT_SHELF_FRU_DEVICE_SLOT;
			name = "Shelf FRU Device Slot ";
			break;
		case 0x1e: // Fan Tray
			ep.Entry[0].EntityType = ATCAHPI_ENT_FAN_TRAY_SLOT;
			name = "Fan Tray Slot ";
			break;
		case 0xc0: // RTM
			ep.Entry[0].EntityType = ATCAHPI_ENT_RTM_SLOT;
			name = "RTM Slot ";
			break;
		default:
			no_slot = 1;
			goto end_of_slot;
		}
		ep.Entry[0].EntityLocation = slot_val;
		ep.Entry[1].EntityType = SAHPI_ENT_ROOT;
		ep.Entry[1].EntityLocation = 0;
		oh_encode_entitypath(ipmi_handler->entity_root, &rootep);
		oh_concat_ep(&ep, &rootep);
		rpt = oh_get_resource_by_ep(handler->rptcache, &ep);
		if (rpt == NULL) {
			// create rpt for slot
			SaHpiRptEntryT srpt;
			int i;

			init_rpt(&srpt);
			for (i = 0; i < SAHPI_MAX_ENTITY_PATH; i ++) {
				srpt.ResourceEntity.Entry[i].EntityLocation =
					ep.Entry[i].EntityLocation;
				srpt.ResourceEntity.Entry[i].EntityType =
					ep.Entry[i].EntityType;
				if (ep.Entry[i].EntityType == SAHPI_ENT_ROOT) {
					break;
				}
			}
			oh_append_textbuffer(&srpt.ResourceTag, name);
			snprintf((char *)slot_n_str, 8, "%d", slot_val);
			oh_append_textbuffer(&srpt.ResourceTag, (char *)slot_n_str);
			srpt.ResourceId =
				oh_uid_from_entity_path(&srpt.ResourceEntity);

			s_r_info = malloc(sizeof(*ohoi_res_info));
			if (s_r_info == NULL) {
				err("Out of Memory");
				goto end_of_slot;
			}
			memset(s_r_info, 0, sizeof(*ohoi_res_info));
			s_r_info->type = OHOI_RESOURCE_SLOT;
			s_r_info->u.slot.devid =
				ipmi_entity_get_fru_device_id(entity);
			s_r_info->u.slot.addr =
				ipmi_entity_get_device_address(entity);
			s_r_info->u.slot.entity_id =
				ipmi_entity_convert_to_id(entity);
			if (oh_add_resource(handler->rptcache, &srpt,
								s_r_info, 1)) {
				err("couldn't add resource for slot %d",
						ep.Entry[0].EntityLocation);
				trace_ipmi_entity("COULD NOT CREATE SLOT for ",
					0, entity);
				no_slot = 1;
				g_free(s_r_info);
				goto end_of_slot;
			}
			trace_ipmi_entity("CREATE SLOT for ", 0, entity);
			if (ipmi_entity_get_entity_id(entity) == 0xf0) {
				// this is not virtual shelf manager
				ipmi_handler->shmc_num++;
			}
			entity_rpt_set_presence(s_r_info, ipmi_handler, 1);
			rpt = oh_get_resource_by_ep(handler->rptcache, &ep);
			atca_create_slot_rdrs(handler, srpt.ResourceId);
		} else {
			s_r_info = oh_get_resource_data(handler->rptcache,
					rpt->ResourceId);
			if (s_r_info && (s_r_info->type & OHOI_RESOURCE_SLOT)) {
				s_r_info->u.slot.devid =
					ipmi_entity_get_fru_device_id(entity);
				s_r_info->u.slot.addr =
					ipmi_entity_get_device_address(entity);
				s_r_info->u.slot.entity_id =
					ipmi_entity_convert_to_id(entity);
			} else {
				err("Internal error. s_r_info == %p", s_r_info);
			}
		}
	}
end_of_slot:

	if ((entity_id == 0xa0) && (entity_instance >= 96)) {
		// ATCA Board
		if ((ipmi_entity_get_device_address(entity) == 130)
			|| (ipmi_entity_get_device_address(entity) == 132)) {
			oh_append_textbuffer(&entry->ResourceTag, "Switch ");
			entry->ResourceEntity.Entry[0].EntityType =
							SAHPI_ENT_SWITCH_BLADE;
		} else if (entity_instance == 102) {
			/* this is here for Force-made Storage blades
	 		* until we have access to latest hardware
	 		* DO NOT CHANGE
			*/
			oh_append_textbuffer(&entry->ResourceTag,
							"Storage/Disk Blade ");
			entry->ResourceEntity.Entry[0].EntityType =
							SAHPI_ENT_DISK_BLADE;
		} else {
			oh_append_textbuffer(&entry->ResourceTag, "Blade ");
			entry->ResourceEntity.Entry[0].EntityType =
							SAHPI_ENT_SBC_BLADE;
		}
        }



	if ((entity_id == 0x0a) && (entity_instance >= 96)) {
		// Power Unit
		oh_append_textbuffer(&entry->ResourceTag, "PEM ");

        }

       if ((entity_id == 0xf0) && (entity_instance >= 96))  {
		// Shelf Manager
		if ((ipmi_entity_get_device_channel(entity) != 0) ||
				(ipmi_entity_get_device_address(entity) != 32)) {
			oh_append_textbuffer(&entry->ResourceTag, "Shelf Manager ");
		} else {
			oh_append_textbuffer(&entry->ResourceTag, "Virtual Shelf Manager");
			no_slot = 1;
			// XXXX Temporary. Until SDRs fixed
			ohoi_res_info->type |= OHOI_RESOURCE_MC;
			ohoi_res_info->u.entity.mc_id = ipmi_handler->virt_mcid;
//			entry->ResourceCapabilities |= SAHPI_CAPABILITY_EVENT_LOG;
		}
		entry->ResourceEntity.Entry[0].EntityType = SAHPI_ENT_SHELF_MANAGER;
        }

	if ((entity_id == 0xf2) && (entity_instance >= 96))  {
		// Shelf FRU
		oh_append_textbuffer(&entry->ResourceTag, "Shelf FRU Device ");
		entry->ResourceEntity.Entry[0].EntityType =
					ATCAHPI_ENT_SHELF_FRU_DEVICE;
        }

	if ((entity_id == 0xf1) && (entity_instance >= 96)) {
		// Filtration Unit
		oh_append_textbuffer(&entry->ResourceTag, "Fan Filter Tray ");
		entry->ResourceEntity.Entry[0].EntityType =
					ATCAHPI_ENT_FILTRATION_UNIT;
	}

	if ((entity_id == 0x1e) && (entity_instance >= 96)) {
		// Fan Tray
		oh_append_textbuffer(&entry->ResourceTag, "Fan Tray ");
	}

	if ((entity_id == 0xc0) && (entity_instance >= 96)) {
		// Fan Tray
		oh_append_textbuffer(&entry->ResourceTag, "RTM ");
	}

	/* End AdvancedTCA Fix-ups */

	if (!no_slot) {
		oh_append_textbuffer(&entry->ResourceTag, (char *)slot_n_str);
	}

no_atca:
	if (entry->ResourceTag.DataLength == 0) {
		str = ipmi_entity_get_entity_id_string(entity);
		oh_append_textbuffer(&entry->ResourceTag, str);
	}
	if (!no_slot) {
		oh_concat_ep(&entry->ResourceEntity, &ep);
	} else if (ipmi_entity_get_is_child(entity)) {
		struct add_parent_ep_s info;
		info.handler = handler;
		info.entry = entry;
		ipmi_entity_iterate_parents(entity, add_parent_ep, &info);
	} else {
		oh_encode_entitypath(ipmi_handler->entity_root, &entity_ep);
		oh_concat_ep(&entry->ResourceEntity, &entity_ep);
	}

	entry->ResourceId = oh_uid_from_entity_path(&entry->ResourceEntity);
}

static void add_entity_event(ipmi_domain_t            *domain,
			     ipmi_entity_t	        *entity,
			     struct oh_handler_state    *handler)
{
	struct ohoi_handler *ipmi_handler = handler->data;
	struct ohoi_resource_info *ohoi_res_info;
	SaHpiRptEntryT	entry;
	int rv;
	int inst;

	inst = ipmi_entity_get_entity_instance(entity);
	if (inst >= 96) {
		inst -= 96;
	}

	memset(&entry, 0, sizeof (entry));

	ohoi_res_info = malloc(sizeof(*ohoi_res_info));

	if (!ohoi_res_info) {
	      	err("Out of memory");
		trace_ipmi_entity("CAN NOT ADD ENTITY. OUT OF MEMORY",
			inst, entity);
		return;
	}
	memset(ohoi_res_info, 0, sizeof (*ohoi_res_info));
	ohoi_res_info->max_ipmb0_link = -1;

	ohoi_res_info->type       = OHOI_RESOURCE_ENTITY;
	ohoi_res_info->u.entity.entity_id= ipmi_entity_convert_to_id(entity);

	get_entity_event(entity, ohoi_res_info, &entry, handler);

	rv = oh_add_resource(handler->rptcache, &entry, ohoi_res_info, 1);
	if (rv) {
	      	err("oh_add_resource failed for %d = %s\n",
			entry.ResourceId, oh_lookup_error(rv));
		trace_ipmi_entity("CAN NOT ADD ENTITY. ADD RESOURCE FAILED",
			inst, entity);
		return;
	}
	if (!IS_ATCA(ipmi_domain_get_type(domain))) {
		return;
	}
	if (entry.ResourceEntity.Entry[0].EntityType ==
					SAHPI_ENT_SYSTEM_CHASSIS) {
		ipmi_handler->atca_shelf_id = entry.ResourceId;
	}

	if (ipmi_entity_get_type(entity) == IPMI_ENTITY_MC) {
		ohoi_create_fru_mc_reset_control(handler, entry.ResourceId);
	}

	switch (ipmi_entity_get_entity_id(entity)) {
	case 0xf0:	// Shelf Manager
		if (ipmi_entity_get_device_address(entity) == 0x20) {
			// this is virtual shelf manager
			ipmi_handler->atca_vshm_id = entry.ResourceId;
			create_atca_virt_shmgr_rdrs(handler);
			// virtual shelf manager always present
			entity_rpt_set_presence(ohoi_res_info,
						handler->data, 1);
		}
		break;
	default:
		break;
	}
}

void ohoi_remove_entity(struct oh_handler_state *handler,
			SaHpiResourceIdT res_id)
{
      	struct oh_event *e = NULL;
	struct ohoi_resource_info *res_info = NULL;
	SaHpiRptEntryT *rpte = NULL;

	res_info = oh_get_resource_data(handler->rptcache, res_id);
	rpte = oh_get_resource_by_id(handler->rptcache, res_id);
	if (!rpte) {
		err("Rpt entry not found");
		return;
	}


	/* Now put an event for the resource to DEL */
	e = malloc(sizeof(*e));
	if (e == NULL) {
		err("Out of memory");
		return;
	}
	memset(e, 0, sizeof(*e));

	if (rpte->ResourceCapabilities & SAHPI_CAPABILITY_FRU) {
		SaHpiHotSwapEventT *hse = &e->event.EventDataUnion.HotSwapEvent;
		e->event.EventType = SAHPI_ET_HOTSWAP;
		hse->HotSwapState = SAHPI_HS_STATE_NOT_PRESENT;
		hse->PreviousHotSwapState = SAHPI_HS_STATE_ACTIVE;
	} else {
		SaHpiResourceEventT *re = &e->event.EventDataUnion.ResourceEvent;
		e->event.EventType = SAHPI_ET_RESOURCE;
		re->ResourceEventType = SAHPI_RESE_RESOURCE_FAILURE;
	}
	e->resource = *rpte;
	e->event.Source = rpte->ResourceId;
	e->event.Severity = rpte->ResourceSeverity;
	oh_gettimeofday(&e->event.Timestamp);

	e->hid = handler->hid;
        oh_evt_queue_push(handler->eventq, e);
	entity_rpt_set_updated(res_info, handler->data);
}



static void change_entity(struct oh_handler_state	*handler,
		          ipmi_entity_t			*entity)
{
	struct ohoi_handler *ipmi_handler = handler->data;
	ipmi_entity_id_t entity_id = ipmi_entity_convert_to_id(entity);
	SaHpiRptEntryT	*rpt;
	SaHpiResourceIdT slot_id;
	struct ohoi_resource_info *s_r_info;
	struct ohoi_resource_info *res_info;
	unsigned int dummy;

	rpt = ohoi_get_resource_by_entityid(handler->rptcache, &entity_id);
	if (rpt == NULL) {
		err("Couldn't find out resource by entity %d.%.d.%d.%d  %s",
			ipmi_entity_get_entity_id(entity),
			ipmi_entity_get_entity_instance(entity),
			ipmi_entity_get_device_channel(entity),
			ipmi_entity_get_device_address(entity),
			ipmi_entity_get_entity_id_string(entity));
		trace_ipmi_entity("CAN NOT CHANGE RESOURCE. NO RPT",
			0, entity);
		return;
	}
	res_info =  oh_get_resource_data(handler->rptcache, rpt->ResourceId);
	update_resource_capabilities(entity, rpt, res_info);
	entity_rpt_set_updated(res_info, ipmi_handler);
	if (ipmi_handler->d_type != IPMI_DOMAIN_TYPE_ATCA) {
		return;
	}
	if (ipmi_entity_get_physical_slot_num(entity, &dummy)) {
		// entity does not have a slot
		return;
	}
#if 0
	if (ipmi_entity_get_type(entity) == IPMI_ENTITY_MC) {
		if (!(res_info->type & OHOI_MC_RESET_CONTROL_CREATED)) {
			ohoi_create_fru_mc_reset_control(handler,
							rpt->ResourceId);
		}
		if (!(res_info->type & OHOI_MC_IPMB0_CONTROL_CREATED) &&
				(res_info->max_ipmb0_link >= 0)) {
			ohoi_create_ipmb0_controls(handler, entity,
			     (SaHpiCtrlStateAnalogT)res_info->max_ipmb0_link);
		}
	}
#endif
	slot_id = ohoi_get_parent_id(rpt);
	s_r_info = oh_get_resource_data(handler->rptcache, slot_id);
	if (s_r_info && (s_r_info->type & OHOI_RESOURCE_SLOT)) {
		s_r_info->u.slot.devid =
					ipmi_entity_get_fru_device_id(entity);
		s_r_info->u.slot.addr =
					ipmi_entity_get_device_address(entity);
	} else {
		err("No res_info(%p) for slot %d", s_r_info, slot_id);
	}
}

static void delete_entity(struct oh_handler_state	*handler,
		          ipmi_entity_t			*entity)
{
	ipmi_entity_id_t entity_id = ipmi_entity_convert_to_id(entity);
	SaHpiRptEntryT	*rpt;
	struct ohoi_resource_info *res_info;

	rpt = ohoi_get_resource_by_entityid(handler->rptcache, &entity_id);
	if (rpt == NULL) {
		err("couldn't find out resource");
		return;
	}
	res_info =  oh_get_resource_data(handler->rptcache, rpt->ResourceId);
	// send event to infrastructure and remove rpt entry
	struct oh_event *e = malloc(sizeof(*e));
	if (e != NULL) {
                memset(e, 0, sizeof(*e));
		if (rpt->ResourceCapabilities & SAHPI_CAPABILITY_FRU) {
			e->event.EventType = SAHPI_ET_HOTSWAP;
			e->event.EventDataUnion.HotSwapEvent.HotSwapState =
				SAHPI_HS_STATE_NOT_PRESENT;
		} else {
			e->event.EventType = SAHPI_ET_RESOURCE;
			e->event.EventDataUnion.ResourceEvent.ResourceEventType =
				SAHPI_RESE_RESOURCE_FAILURE;
		}
                e->resource = *rpt;
                e->event.Source = rpt->ResourceId;
                e->event.Severity = rpt->ResourceSeverity;
                oh_gettimeofday(&e->event.Timestamp);
                e->hid = handler->hid;
                oh_evt_queue_push(handler->eventq, e);
	} else {
		err("Out of memory");
	}

	while (SA_OK == oh_remove_rdr(handler->rptcache, rpt->ResourceId,
						SAHPI_FIRST_ENTRY));
	if (res_info) {
		ohoi_delete_rpt_fru(res_info);
	}
	oh_remove_resource(handler->rptcache, rpt->ResourceId);
}

void ohoi_entity_event(enum ipmi_update_e       op,
                       ipmi_domain_t            *domain,
                       ipmi_entity_t            *entity,
                       void                     *cb_data)
{
  	struct oh_handler_state *handler = cb_data;
	struct ohoi_handler *ipmi_handler = handler->data;
	int rv;
	int inst=0;

	inst=ipmi_entity_get_entity_instance(entity);
	if(inst >=96) {
		inst = inst - 96;
	}
	g_static_rec_mutex_lock(&ipmi_handler->ohoih_lock);
	switch (op) {
	  	case IPMI_ADDED:
			add_entity_event(domain, entity, handler);
			trace_ipmi_entity("ADDED", inst, entity);

			/* entity presence overall */
			rv = ipmi_entity_add_presence_handler(entity,
							      entity_presence,
							      handler);
			if (rv)
				err("ipmi_entity_set_presence_handler: %#x", rv);

			/* hotswap handler */
			rv = ipmi_entity_add_hot_swap_handler(entity,
							      ohoi_hot_swap_cb,
							      cb_data);
			if(rv)
			  	err("Failed to set entity hot swap handler");

			/* sensors */
			rv= ipmi_entity_add_sensor_update_handler(entity,
								  ohoi_sensor_event,
								  handler);
			if (rv) {
				err("ipmi_entity_set_sensor_update_handler: %#x", rv);
				break;
			}
			/* controls */
			rv = ipmi_entity_add_control_update_handler(entity,
								    ohoi_control_event,
								    handler);

			if (rv) {
				err("ipmi_entity_set_control_update_handler: %#x", rv);
				return;
			}
			/* inventory (a.k.a FRU) */
			rv = ipmi_entity_add_fru_update_handler(entity,
								ohoi_inventory_event,
								handler);
			if (rv) {
			  	err("ipmi_entity_set_fru_update_handler: %#x", rv);
				break;
			}
			break;

		case IPMI_DELETED:
			delete_entity(handler, entity);
			trace_ipmi_entity("DELETED", inst, entity);
			break;

		case IPMI_CHANGED:
			change_entity(handler, entity);
			trace_ipmi_entity("CHANGED", inst, entity);
			break;
		default:
			err("Entity: Unknow change?!");
	}
	g_static_rec_mutex_unlock(&ipmi_handler->ohoih_lock);
}

