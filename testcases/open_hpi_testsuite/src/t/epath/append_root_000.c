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
 * main: append_root test.
 *
 * This will call append_root with NULL as the first parameter.
 * Should get a return of -1 from the call exiting gracefully.
 *
 * Return value: 0 on success, 1 on failure
 **/
int main(int argc, char **argv)
{
        if(!append_root(NULL))
                return 1;

        return 0;
}
