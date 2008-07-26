/*      -*- linux-c -*-
 *
 * (C) Copyright IBM Corp. 2004
 * Copyright (c) 2004 by Intel Corp.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  This
 * file and program are licensed under a BSD style license.  See
 * the Copying file included with the OpenHPI distribution for
 * full licensing terms.
 *
 * Authors:
 *      Christina Hernandez<hernanc@us.ibm.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <glib.h>
#include <string.h>

#include <SaHpi.h>
#include <openhpi.h>
#include <oh_utils.h>
#include <el_utils.h>


#include "el_test.h"


/**
 * main: EL test
 *
 * This test verifies the failure of oh_el_info
 * when info == NULL
 *
 * Return value: 0 on success, 1 on failure
 **/


int main(int argc, char **argv)
{
        oh_el *el;
        SaErrorT retc;


	/* test oh_el_info with info == NULL */

	el = oh_el_create(20);

        retc = oh_el_info(el, NULL);
        if (retc == SA_OK) {
                err("ERROR: oh_el_info failed.");
                return 1;
        }

        /* close el without saving to file*/
        retc = oh_el_close(el);
        if (retc != SA_OK) {
                err("ERROR: oh_el_close on el failed.");
                return 1;
        }

        return 0;
}






