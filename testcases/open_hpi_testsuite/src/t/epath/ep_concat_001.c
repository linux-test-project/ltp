/* -*- linux-c -*-
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
 * Authors:
 *     Renier Morales <renierm@users.sf.net>
 *
 */

#include <string.h>
#include <SaHpi.h>
#include <epath_utils.h>

/**
 * main: ep_concat test.
 *
 * This will call ep_concat with NULL as the second parameter.
 * Should get a return of 0 from the call exiting gracefully.
 * Returning 0 is considered ok with a NULL second param.
 *
 * Return value: 0 on success, 1 on failure
 **/
int main(int argc, char **argv)
{
        SaHpiEntityPathT tmp_ep;
        char *entity_root = "{CHASSIS_SPECIFIC,89}{OPERATING_SYSTEM,46}";

        string2entitypath(entity_root, &tmp_ep);

        if(ep_concat(&tmp_ep, NULL))
                return 1;

        return 0;
}
