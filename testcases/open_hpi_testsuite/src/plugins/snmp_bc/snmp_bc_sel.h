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
 *      Renier Morales <renierm@users.sf.net>
 *      Sean Dague <http://dague.net/sean>
 */

#ifndef SNMP_BC_SEL_H
#define SNMP_BC_SEL_H

#define BC_DATETIME_OID ".1.3.6.1.4.1.2.3.51.2.4.4.1.0"
#define BC_SEL_INDEX_OID ".1.3.6.1.4.1.2.3.51.2.3.4.2.1.1"
#define BC_SEL_ENTRY_OID ".1.3.6.1.4.1.2.3.51.2.3.4.2.1.2"

#define BC_SEL_ID_STRING 20
#define BC_SEL_ENTRY_STRING 256

typedef struct {
        struct tm time;
        SaHpiSeverityT sev;
        char source[BC_SEL_ID_STRING];
        char sname[BC_SEL_ID_STRING];
        char text[BC_SEL_ENTRY_STRING];
} bc_sel_entry;


/* 
 * Function Prototyping
 */
int snmp_bc_parse_sel_entry(struct snmp_session *,char * text, bc_sel_entry * sel);
int snmp_bc_get_sel_info(void *hnd, SaHpiResourceIdT id, SaHpiSelInfoT *info);
int snmp_bc_get_sel_entry(void *hnd, SaHpiResourceIdT id, SaHpiSelEntryIdT current,
                          SaHpiSelEntryIdT *prev, SaHpiSelEntryIdT *next,
                          SaHpiSelEntryT *entry);
int snmp_bc_set_sel_time(void *hnd, SaHpiResourceIdT id, SaHpiTimeT time);
int snmp_bc_add_sel_entry(void *hnd, SaHpiResourceIdT id, const SaHpiSelEntryT *Event);
int snmp_bc_del_sel_entry(void *hnd, SaHpiResourceIdT id, SaHpiSelEntryIdT sid);
int set_bc_sp_time(struct snmp_session *, struct tm *);
int get_bc_sp_time(struct snmp_session *, struct tm *);

#endif
