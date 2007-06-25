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

#include <openhpi.h>
#include <snmp_utils.h>
#include <snmp_bc_plugin.h>
#include <sim_init.h>

SaHpiBoolT is_simulator()
{
  return(SAHPI_FALSE);
}

SaErrorT sim_banner(struct snmp_bc_hnd *custom_handle) 
{
  return(SA_OK);  
}

SaErrorT sim_init() 
{
  return(SA_OK);  
}

SaErrorT sim_close()
{
  return(SA_OK);
}
