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
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#include <SaHpi.h>
#include <openhpi.h>
#include <sel_utils.h>


/**
 * main: SEL test
 * 
 * This test tests the creation of an SEL.
 * 
 * Return value: 0 on success, 1 on failure
 **/
int main(int argc, char **argv) 
{
        oh_sel *sel;
        
        sel = oh_sel_create();
        
        if(sel == NULL) {
                dbg("ERROR: sel pointer == NULL.");
                return 1;
        }
        
        if(sel->enabled != TRUE) {
                dbg("ERROR: sel->enabled invalid.");
                return 1;
        }
        
        if(sel->overflow != FALSE) {
                dbg("ERROR: sel->overflow invalid.");
                return 1;
        }
        
        if(sel->deletesupported != FALSE) {
                dbg("ERROR: sel->deletesupported invalid.");
                return 1;
        }
        
        if(sel->lastUpdate != SAHPI_TIME_UNSPECIFIED) {
                dbg("ERROR: sel->lastUpdate invalid.");
                return 1;
        }
        
        if(sel->offset != 0) {
                dbg("ERROR: sel->offset invalid.");
                return 1;
        }
        
        if(sel->nextId != 0) {
                dbg("ERROR: sel->nextId invalid.");
                return 1;
        }
        
        if(sel->selentries != NULL) {
                dbg("ERROR: sel->selentries invalid.");
                return 1;
        }
        
        return 0;
}
