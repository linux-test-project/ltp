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
 *      David Ashley<dashley@us.ibm.com>
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <glib.h>
#include <string.h>

#include <SaHpi.h>
#include <openhpi.h>
#include <sel_utils.h>


#include "sel_test.h"


/**
 * main: SEL test
 * 
 * This test tests creates an SEL and adds one event.
 * 
 * Return value: 0 on success, 1 on failure
 **/
int main(int argc, char **argv) 
{
        oh_sel *sel;
        SaErrorT retc;

        /* create the SEL */
        sel = oh_sel_create();
        
        if(sel == NULL) {
                dbg("ERROR: sel == NULL.");
                return 1;
        }

        /* add a single event */
        if (add_event(sel, 0)) {
                dbg("ERROR: add_event failed.");
                return 1;
        }

        /* close the SEL */
        retc = oh_sel_close(sel);
        if (retc != SA_OK) {
                dbg("ERROR: oh_sel_close failed.");
                return 1;
        }

        return 0;
}

