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
 */

#ifndef SNMP_BC_H
#define SNMP_BC_H

/**
 * This handle will be unique per instance of this plugin. 
 * SNMP session data is stored in the handle along with config file data.
 **/

struct snmp_bc_hnd {
        struct snmp_session session;
        struct snmp_session *ss; /* SNMP Session pointer */
};

#endif
