/*      -*- linux-c -*-
 *
 * (C) Copyright IBM Corp. 2004
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  This
 * file and program are licensed under a BSD style license.  See
 * the Copying file included with the OpenHPI distribution for
 * full licensing terms.
 *
 * Author(s):
 *      Steve Sherman <stevees@us.ibm.com>
 */

#ifndef __SIM_INIT_H
#define __SIM_INIT_H

SaHpiBoolT is_simulator(void);
SaErrorT sim_banner(struct snmp_bc_hnd *custom_handle);
SaErrorT sim_init(void);
SaErrorT sim_close(void);
SaErrorT sim_file(void);

#endif
