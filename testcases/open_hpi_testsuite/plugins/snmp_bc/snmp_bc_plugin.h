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

#ifndef __SNMP_BC_PLUGIN_H
#define __SNMP_BC_PLUGIN_H

/* Order is important */
#include <openhpi.h>
#include <SaHpiBladeCenter.h>
#include <snmp_utils.h>

#include <snmp_bc.h>
#include <snmp_bc_resources.h>
#include <snmp_bc_annunciator.h>
#include <snmp_bc_control.h>
#include <snmp_bc_event.h> /* has to be before discover.h */
#include <snmp_bc_discover.h>
#include <snmp_bc_discover_bc.h>
#include <snmp_bc_el.h>
#include <snmp_bc_el2event.h>
#include <snmp_bc_hotswap.h>
#include <snmp_bc_inventory.h>
#include <snmp_bc_power.h>
#include <snmp_bc_reset.h>
#include <snmp_bc_sel.h>
#include <snmp_bc_sensor.h>
#include <snmp_bc_session.h>
#include <snmp_bc_time.h>
#include <snmp_bc_watchdog.h>
#include <snmp_bc_utils.h>
#include <snmp_bc_lock.h>

#endif
