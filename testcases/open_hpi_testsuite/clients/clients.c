/* -*- linux-c -*-
 *
 * Copyright (C) 2007-2008, Hewlett-Packard Development Company, LLP
 * (C) Copyright IBM Corp. 2007
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  This
 * file and program are licensed under a BSD style license.  See
 * the Copying file included with the OpenHPI distribution for
 * full licensing terms.
 *
 * Author(s):
 *      Shuah Khan <shuah.khan@hp.com>
 *	Renier Morales <renier@openhpi.org>
 */

#include <string.h>

#include <SaHpi.h>
#include <oHpi.h>
#include <oh_clients.h>

void oh_prog_version(const char *prog_name, const char *svn_rev_str)
{
	SaHpiUint32T ohpi_major = oHpiVersionGet() >> 48;
        SaHpiUint32T ohpi_minor = (oHpiVersionGet() << 16) >> 48;
        SaHpiUint32T ohpi_patch = (oHpiVersionGet() << 32) >> 48;
        SaHpiVersionT hpiver;
        char svn_rev[SAHPI_MAX_TEXT_BUFFER_LENGTH];

        strncpy(svn_rev, svn_rev_str, SAHPI_MAX_TEXT_BUFFER_LENGTH);
        svn_rev[strlen(svn_rev_str)-2] = '\0';

        printf("%s (rev %s) - This program came with OpenHPI %u.%u.%u\n",
               prog_name, svn_rev+11, ohpi_major, ohpi_minor, ohpi_patch);
        hpiver = saHpiVersionGet();
        printf("SAF HPI Version %c.0%d.0%d\n\n",
               (hpiver >> 16) + ('A' - 1),
               (hpiver & 0x0000FF00) >> 8,
               hpiver & 0x000000FF);
}

