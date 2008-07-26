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
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#include <SaHpi.h>
#include <openhpi.h>
#include <oh_utils.h>


/**
 * main: EL test
 *
 * This test tests the creation of an EL.
 *
 * Return value: 0 on success, 1 on failure
 **/
int main(int argc, char **argv)
{
        oh_el *el;

        el = oh_el_create(5);

        if(el == NULL) {
                err("ERROR: el pointer == NULL.");
                return 1;
        }

        if(el->info.Enabled != TRUE) {
                err("ERROR: el->info.Enabled invalid.");
                return 1;
        }

        if(el->info.OverflowFlag != FALSE) {
                err("ERROR: el->info.OverflowFlag invalid.");
                return 1;
        }

        if(el->info.UpdateTimestamp != SAHPI_TIME_UNSPECIFIED) {
                err("ERROR: el->info.UpdateTimestamp invalid.");
                return 1;
        }

        if(el->basetime != 0) {
                err("ERROR: el->basetime invalid.");
                return 1;
        }

        if(el->nextid != SAHPI_OLDEST_ENTRY + 1) {
                err("ERROR: el->nextid invalid.");
                return 1;
        }

        if(el->info.Size != 5) {
                err("ERROR: el->info.Size invalid.");
                return 1;
        }

        if(el->list != NULL) {
                err("ERROR: el->list invalid.");
                return 1;
        }

        return 0;
}
