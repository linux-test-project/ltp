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
 * This test tests creates an SEL and adds five events.
 * It then save the SEL to a file.
 * 
 * Return value: 0 on success, 1 on failure
 **/
int main(int argc, char **argv) 
{
        oh_sel *sel;
        int x;
        SaErrorT retc;
        
        /* create the SEL */
        sel = oh_sel_create();
        
        if(sel == NULL) {
                dbg("ERROR: sel == NULL.");
                return 1;
        }

        /* add a multiple events */
        for (x = 0; x < 5; x++) {
                retc = add_event(sel, x);
                if (retc != SA_OK) {
                        dbg("ERROR: add_event failed.");
                        return 1;
                }
        }

        /* save the SEL */
        retc = oh_sel_map_to_file(sel, "./selTest.data");
        if (retc != SA_OK) {
                dbg("ERROR: oh_sel_map_to_file failed.");
                return 1;
        }

        /* close the sel */
        retc = oh_sel_close(sel);
        if (retc != SA_OK) {
                dbg("ERROR: oh_sel_close failed.");
                return 1;
        }

        
        return 0;
}

