/*      -*- linux-c -*-
 *
 * (C) Copyright IBM Corp. 2005, 2006
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  This
 * file and program are licensed under a BSD style license.  See
 * the Copying file included with the OpenHPI distribution for
 * full licensing terms.
 *
 * Author(s):
 *	W. David Ashley <dashley@us.ibm.com>
 *	Renier Morales <renier@openhpi.org>
 */

#ifndef __SIM_INVENTORY_H
#define __SIM_INVENTORY_H

#define SIM_INVENTORY_FIELDS 10
#define SIM_INVENTORY_AREAS 10

/************************************************************************/
/* Resource one inventory data   					*/
/************************************************************************/
struct  sim_idr_area {
        SaHpiEntryIdT       nextfieldid;
        SaHpiIdrAreaHeaderT idrareahead;
        SaHpiIdrFieldT      field[SIM_INVENTORY_FIELDS];
};

struct sim_inventory_info {
        SaHpiEntryIdT       nextareaid;
        SaHpiIdrInfoT       idrinfo;
        struct sim_idr_area area[SIM_INVENTORY_AREAS];
};

struct sim_inventory {
        SaHpiInventoryRecT        invrec;
        struct sim_inventory_info info;
	const char                *comment;
};


SaErrorT sim_discover_chassis_inventory(struct oh_handler_state *state,
                                        struct oh_event *e);
SaErrorT sim_discover_cpu_inventory(struct oh_handler_state *state,
                                    struct oh_event *e);
SaErrorT sim_discover_dasd_inventory(struct oh_handler_state *state,
                                     struct oh_event *e);
SaErrorT sim_discover_hs_dasd_inventory(struct oh_handler_state *state,
                                        struct oh_event *e);
SaErrorT sim_discover_fan_inventory(struct oh_handler_state *state,
                                    struct oh_event *e);

SaErrorT sim_get_idr_info(void *hnd,
		          SaHpiResourceIdT        ResourceId,
		          SaHpiIdrIdT             IdrId,
		          SaHpiIdrInfoT          *IdrInfo);

SaErrorT sim_get_idr_area_header(void *hnd,
		                 SaHpiResourceIdT         ResourceId,
		                 SaHpiIdrIdT              IdrId,
		                 SaHpiIdrAreaTypeT        AreaType,
		                 SaHpiEntryIdT            AreaId,
		                 SaHpiEntryIdT           *NextAreaId,
		                 SaHpiIdrAreaHeaderT     *Header);

SaErrorT sim_add_idr_area(void *hnd,
		          SaHpiResourceIdT         ResourceId,
		          SaHpiIdrIdT              IdrId,
		          SaHpiIdrAreaTypeT        AreaType,
		          SaHpiEntryIdT           *AreaId);

SaErrorT sim_del_idr_area(void *hnd,
		          SaHpiResourceIdT       ResourceId,
		          SaHpiIdrIdT            IdrId,
		          SaHpiEntryIdT          AreaId);

SaErrorT sim_get_idr_field(void *hnd,
		           SaHpiResourceIdT       ResourceId,
		           SaHpiIdrIdT             IdrId,
		           SaHpiEntryIdT           AreaId,
		           SaHpiIdrFieldTypeT      FieldType,
		           SaHpiEntryIdT           FieldId,
		           SaHpiEntryIdT          *NextFieldId,
		           SaHpiIdrFieldT         *Field);

SaErrorT sim_add_idr_field(void *hnd,
		           SaHpiResourceIdT         ResourceId,
		           SaHpiIdrIdT              IdrId,
		           SaHpiIdrFieldT        	*Field);

SaErrorT sim_set_idr_field(void *hnd,
		           SaHpiResourceIdT         ResourceId,
		           SaHpiIdrIdT              IdrId,
		           SaHpiIdrFieldT           *Field);

SaErrorT sim_del_idr_field(void *hnd,
	                   SaHpiResourceIdT         ResourceId,
		           SaHpiIdrIdT              IdrId,
	 	           SaHpiEntryIdT            AreaId,
		           SaHpiEntryIdT            FieldId);

extern struct sim_inventory sim_chassis_inventory[];
extern struct sim_inventory sim_cpu_inventory[];
extern struct sim_inventory sim_dasd_inventory[];
extern struct sim_inventory sim_hs_dasd_inventory[];
extern struct sim_inventory sim_fan_inventory[];


#endif
