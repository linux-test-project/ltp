/*      -*- linux-c -*-
 *
 * (C) Copyright IBM Corp. 2004, 2006
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. This
 * file and program are licensed under a BSD style license. See
 * the Copying file included with the OpenHPI distribution for
 * full licensing terms.
 *
 * Author(s):
 *      Steve Sherman <stevees@us.ibm.com>
 */


#ifndef __SNMP_BC_DISCOVER_H
#define __SNMP_BC_DISCOVER_H

SaErrorT snmp_bc_discover_resources(void *hnd);

SaErrorT snmp_bc_discover(struct oh_handler_state *handle,
			  SaHpiEntityPathT *ep_root);

SaErrorT snmp_bc_discover_rsa(struct oh_handler_state *handle,
			      SaHpiEntityPathT *ep_root);
			      
SaErrorT snmp_bc_rediscover(struct oh_handler_state *handle,
			    SaHpiEventT *event,
			    LogSource2ResourceT *logsrc2res);

SaErrorT snmp_bc_discover_sensors(struct oh_handler_state *handle,
				  struct snmp_bc_sensor *sensor_array,
				  struct oh_event *res_oh_event);

SaErrorT snmp_bc_discover_controls(struct oh_handler_state *handle,
				   struct snmp_bc_control *control_array,
				   struct oh_event *res_oh_event);

SaErrorT snmp_bc_discover_inventories(struct oh_handler_state *handle,
				      struct snmp_bc_inventory *inventory_array,
				      struct oh_event *res_oh_event);

SaErrorT snmp_bc_create_resourcetag(SaHpiTextBufferT *buffer,
				    const char *str,
				    SaHpiEntityLocationT location);

SaHpiBoolT rdr_exists(struct snmp_bc_hnd *custom_handle,
		      SaHpiEntityPathT *ep,
		      SaHpiEntityLocationT loc_offset,		      
		      const gchar *oidstr,
		      unsigned int na,
		      SaHpiBoolT write_only);
			
SaErrorT snmp_bc_validate_ep(SaHpiEntityPathT *org_ep,
			     SaHpiEntityPathT *val_ep);
				 
SaErrorT snmp_bc_mod_sensor_ep(SaHpiRdrT *rdrptr,
			       void *sensor_array, 
			       int index);
				 
SaErrorT snmp_bc_add_ep(SaHpiRdrT *rdrptr, SaHpiEntityPathT *ep_add);	
		
SaErrorT snmp_bc_discover_media_tray(struct oh_handler_state *handle,
				     SaHpiEntityPathT *ep_root, 
				     int  media_tray_installed);
		
SaErrorT snmp_bc_discover_filter(struct oh_handler_state *handle,
				     SaHpiEntityPathT *ep_root, 
				     int  filter_installed);
				     			  
SaErrorT snmp_bc_discover_chassis(struct oh_handler_state *handle,
				  SaHpiEntityPathT *ep_root);

SaErrorT snmp_bc_discover_blade(struct oh_handler_state *handle,
				SaHpiEntityPathT *ep_root, char *blade_vector);

SaErrorT snmp_bc_discover_blowers(struct oh_handler_state *handle,
			       SaHpiEntityPathT *ep_root, char *blower_vector);

SaErrorT snmp_bc_discover_tap(struct oh_handler_state *handle,
			       SaHpiEntityPathT *ep_root, char *tap_vector);

SaErrorT snmp_bc_discover_nc(struct oh_handler_state *handle,
			       SaHpiEntityPathT *ep_root, char *nc_vector);

SaErrorT snmp_bc_discover_mx(struct oh_handler_state *handle,
			       SaHpiEntityPathT *ep_root, char *mx_vector);

SaErrorT snmp_bc_discover_smi(struct oh_handler_state *handle,
			       SaHpiEntityPathT *ep_root, char *smi_vector);

SaErrorT snmp_bc_discover_mmi(struct oh_handler_state *handle,
			       SaHpiEntityPathT *ep_root, char *mmi_vector);
			       
SaErrorT snmp_bc_discover_power_module(struct oh_handler_state *handle,
				       SaHpiEntityPathT *ep_root, char *power_module_vector);

SaErrorT snmp_bc_discover_switch(struct oh_handler_state *handle,
				 SaHpiEntityPathT *ep_root, char *switch_vector);

SaErrorT snmp_bc_discover_mm(struct oh_handler_state *handle,
			     SaHpiEntityPathT *ep_root, char *mm_vector, SaHpiBoolT global_discovery);

SaErrorT snmp_bc_update_chassis_topo(struct oh_handler_state *handle);

SaErrorT snmp_bc_discover_all_slots(struct oh_handler_state *handle,
					SaHpiEntityPathT *ep_root);

SaErrorT snmp_bc_discover_slot(struct oh_handler_state *handle,
			       SaHpiEntityPathT *ep_root,
			       SaHpiEntityTypeT entitytype,
			       guint entitylocation); 
			       
SaErrorT snmp_bc_discover_blade_expansion(struct oh_handler_state *handle,
			  			SaHpiEntityPathT *ep_root, 
						guint blade_index);
						
SaErrorT snmp_bc_add_blade_expansion_resource(struct oh_handler_state *handle,
			  			SaHpiEntityPathT *ep, 
						guint blade_index,
						BCExpansionTypeT expansionType,
						guint expansionindex);

#endif

