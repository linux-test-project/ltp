/*      -*- linux-c -*-
 *
 * (C) Copyright IBM Corp. 2003
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  This
 * file and program are licensed under a BSD style license.  See
 * the Copying file included with the OpenHPI distribution for
 * full licensing terms.
 *
 * Author(s):
 *      Chris Chia <chia@austin.ibm.com>
 */

#ifndef SNMP_BC_UTILS_H
#define SNMP_BC_UTILS_H

gchar * snmp_derive_objid(SaHpiEntityPathT ep, const gchar *oid);

int get_interpreted_value(gchar *string, 
			  SaHpiSensorInterpretedTypeT type, 
			  SaHpiSensorInterpretedUnionT *value); 

#endif /* SNMP_BC_UTILS_H */
