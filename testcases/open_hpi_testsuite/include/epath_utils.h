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
 *      Steve Sherman <stevees@us.ibm.com>
 *      Renier Morales <renierm@users.sf.net>
 */

#ifndef EPATH_UTILS_H
#define EPATH_UTILS_H

#include <glib.h>

/* Max number of digits an enitity instance has */
#define MAX_INSTANCE_DIGITS 6

/* Defines to manipulate eshort_names, which is a mirrors SaHpiEntityTypeT */
#define ESHORTNAMES_ARRAY_SIZE 61
#define ELEMENTS_IN_SaHpiEntityT 2
#define EPATHSTRING_START_DELIMITER "{"
#define EPATHSTRING_START_DELIMITER_CHAR '{'
#define EPATHSTRING_END_DELIMITER "}"
#define EPATHSTRING_VALUE_DELIMITER ","
#define ESHORTNAMES_BEFORE_JUMP 40
#define ESHORTNAMES_FIRST_JUMP 41
#define ESHORTNAMES_SECOND_JUMP 42
#define ESHORTNAMES_THIRD_JUMP 43
#define ESHORTNAMES_LAST_JUMP 44

int string2entitypath(const gchar *epathstr,
		      SaHpiEntityPathT *epathptr);

int entitypath2string(const SaHpiEntityPathT *epathptr,
		      gchar *epathstr,
		      gint strsize);

int ep_concat(SaHpiEntityPathT *dest, const SaHpiEntityPathT *append);

int set_epath_instance(SaHpiEntityPathT *ep, SaHpiEntityTypeT et, SaHpiEntityInstanceT ei);

int append_root(SaHpiEntityPathT *ep);

#endif
