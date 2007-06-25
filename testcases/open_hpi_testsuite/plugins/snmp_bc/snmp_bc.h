/*      -*- linux-c -*-
 *
 * (C) Copyright IBM Corp. 2003, 2006
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. This
 * file and program are licensed under a BSD style license. See
 * the Copying file included with the OpenHPI distribution for
 * full licensing terms.
 *
 * Author(s):
 *      Renier Morales <renier@openhpi.org>
 *      Steve Sherman <stevees@us.ibm.com>
 */

#ifndef __SNMP_BC_H
#define __SNMP_BC_H

#define SNMP_BC_MAX_SNMP_RETRY_ATTEMPTED  3
#define SNMP_BC_MAX_RESOURCES_MASK        16 /* 15-char long plus NULL terminated */

#include <stdlib.h>
#include <glib.h>

typedef struct {
        GStaticRecMutex lock;
        guint32 count;
} ohpi_bc_lock;

/* This handle is unique per instance of this plugin. 
 * SNMP session data is stored in the handle along with config file data. */
struct snmp_bc_hnd {
	void   *sessp;			/* Opaque pointer, not a pointer to struct snmp_session */
        struct snmp_session session;
        struct snmp_session *ss; 	/* SNMP Session pointer */
	guint    count_per_getbulk;       /* For performance, GETBULK is used with snmpV3. */
					/* This value indicates max OIDs per GETBULK request */
        GSList *eventq;                 /* Event queue cache */
	GHashTable *event2hpi_hash_ptr; /* Global "Event Number to HPI Event" hash table */
	guint   platform;
	guint   active_mm;                /* Used for duplicate event RID override */
	char   *host;
	char   *host_alternate;
	SaHpiBoolT isFirstDiscovery;
	gchar  handler_timezone[10];
        guint   handler_retries;          /* Number of retries attempted on SNMP target (agent) */
	ohpi_bc_lock snmp_bc_hlock;
	guint max_pb_supported;		/* pb - processor blade    */
	guint max_blower_supported;	/* blower - fan/blower     */
	guint max_pm_supported;		/* pm - power module       */
	guint max_sm_supported;		/* sm - i/o module         */
	guint max_mm_supported;		/* mm - management module  */
	guint max_mt_supported;		/* mt - media tray         */
	guint max_filter_supported;	/* filter - front bezel    */
	guint max_tap_supported;	/* tap - telco alarm panel */
	guint max_nc_supported;		/* nc - network clock      */
	guint max_mx_supported;		/* mx - multiplex card     */
	guint max_mmi_supported;	/* mmi - mm interposer     */	
	guint max_smi_supported;	/* smi - switch interposer */
	gchar installed_pb_mask[SNMP_BC_MAX_RESOURCES_MASK];
	gchar installed_blower_mask[SNMP_BC_MAX_RESOURCES_MASK];
	gchar installed_pm_mask[SNMP_BC_MAX_RESOURCES_MASK];
	gchar installed_sm_mask[SNMP_BC_MAX_RESOURCES_MASK];
	gchar installed_mm_mask[SNMP_BC_MAX_RESOURCES_MASK];
	gchar installed_tap_mask[SNMP_BC_MAX_RESOURCES_MASK];
	gchar installed_nc_mask[SNMP_BC_MAX_RESOURCES_MASK];
	gchar installed_mx_mask[SNMP_BC_MAX_RESOURCES_MASK];
	gchar installed_mmi_mask[SNMP_BC_MAX_RESOURCES_MASK];	
	gchar installed_smi_mask[SNMP_BC_MAX_RESOURCES_MASK];
        gulong installed_mt_mask;
	gulong installed_filter_mask; 
};

SaErrorT snmp_bc_snmp_get(struct snmp_bc_hnd *custom_handle,
                          const char *objid,
                          struct snmp_value *value, 
			  SaHpiBoolT retry);

SaErrorT snmp_bc_oid_snmp_get(struct snmp_bc_hnd *custom_handle,
			      SaHpiEntityPathT *ep,
			      SaHpiEntityLocationT loc_offset,
			      const gchar *oidstr,
			      struct snmp_value *value,
			      SaHpiBoolT retry);

SaErrorT snmp_bc_snmp_set(struct snmp_bc_hnd *custom_handle,
                          char *objid,
                          struct snmp_value value);
			  
SaErrorT snmp_bc_oid_snmp_set(struct snmp_bc_hnd *custom_handle,
			      SaHpiEntityPathT *ep,
			      SaHpiEntityLocationT loc_offset,
			      const gchar *oidstr,
			      struct snmp_value value);
			  			  
SaErrorT snmp_bc_get_event(void *hnd);
			   
SaErrorT snmp_bc_set_resource_tag(void *hnd,
				  SaHpiResourceIdT rid,
				  SaHpiTextBufferT *tag);
				  
SaErrorT snmp_bc_set_resource_severity(void *hnd,
				       SaHpiResourceIdT rid,
				       SaHpiSeverityT sev);
					
SaErrorT snmp_bc_control_parm(void *hnd,
			      SaHpiResourceIdT rid,
			      SaHpiParmActionT act);
#endif

