/*      -*- linux-c -*-
 *
 * (C) Copyright IBM Corp. 2003, 2004
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
 */

#ifndef SNMP_BC_DISCOVER_H
#define SNMP_BC_DISCOVER_H

/* Resource discovery prototypes */
struct oh_event * snmp_bc_discover_blade(char *blade_vector, SaHpiEntityPathT *ep, int bladenum);
struct oh_event * snmp_bc_discover_blade_addin(struct snmp_session *ss, char *blade_vector, SaHpiEntityPathT *ep, int bladenum);
struct oh_event * snmp_bc_discover_chassis(char *blade_vector, SaHpiEntityPathT *ep);
struct oh_event * snmp_bc_discover_fan(char *fan_vector, SaHpiEntityPathT *ep, int fannum);
struct oh_event * snmp_bc_discover_mediatray(long exists, SaHpiEntityPathT *ep, int mtnum);
struct oh_event * snmp_bc_discover_mgmnt(char *mm_vector, SaHpiEntityPathT *ep, int mmnum);
struct oh_event * snmp_bc_discover_power(char *power_vector, SaHpiEntityPathT *ep, int powernum);
struct oh_event * snmp_bc_discover_subchassis(char *blade_vector, SaHpiEntityPathT *ep, int scnum);
struct oh_event * snmp_bc_discover_switch(char *switch_vector, SaHpiEntityPathT *ep, int switchnum);

/* RDR discovery prototypes */
struct oh_event * snmp_bc_discover_controls(struct snmp_session *ss,
					    SaHpiEntityPathT parent_ep,
					    const struct snmp_bc_control *control);

struct oh_event * snmp_bc_discover_sensors(struct snmp_session *ss,
                                           SaHpiEntityPathT parent_ep,
                                           const struct snmp_bc_sensor *sensor);

struct oh_event * snmp_bc_discover_inventories(struct snmp_session *ss,
                                           SaHpiEntityPathT parent_ep,
                                           const struct snmp_bc_inventory *inventory);

#endif /* SNMP_BC_DISCOVER_H */
