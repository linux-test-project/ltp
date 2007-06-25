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
 *     Renier Morales <renier@openhpi.org>
 */

#include <glib.h>

#include <SaHpi.h>
#include <oh_utils.h>

/**
 * main: Calls oh_entity_path_lookup using a NULL entity path.
 * Passes if it returns -1,
 * otherwise fails.
 *
 * Return value: 0 on success, 1 on failure
 **/
int main(int argc, char **argv)
{
        SaHpiEntityPathT ep;
        guint id;

        oh_init_ep(&ep);

	if (oh_uid_initialize())
		return 1;

        id = oh_uid_from_entity_path(&ep);
        if (!id)
                return 1;

        if (!oh_entity_path_lookup(id, NULL))
                return 1;

        return 0;
}
