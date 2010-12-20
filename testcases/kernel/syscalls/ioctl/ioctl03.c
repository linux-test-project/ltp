/******************************************************************************/
/*                                                                            */
/* Copyright (c) Rusty Russell <rusty@rustcorp.com.au>                        */
/* Copyright (c) International Business Machines  Corp., 2008                 */
/*                                                                            */
/* This program is free software;  you can redistribute it and/or modify      */
/* it under the terms of the GNU General Public License as published by       */
/* the Free Software Foundation; either version 2 of the License, or          */
/* (at your option) any later version.                                        */
/*                                                                            */
/* This program is distributed in the hope that it will be useful,            */
/* but WITHOUT ANY WARRANTY;  without even the implied warranty of            */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See                  */
/* the GNU General Public License for more details.                           */
/*                                                                            */
/* You should have received a copy of the GNU General Public License          */
/* along with this program;  if not, write to the Free Software               */
/* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA    */
/*                                                                            */
/******************************************************************************/
/******************************************************************************/
/*                                                                            */
/* File:        ioctl03.c                                                     */
/*                                                                            */
/* Description: This program tests whether all the valid IFF flags are        */
/*              returned properly by implementation of TUNGETFEATURES ioctl   */
/*              on kernel 2.6.27                                              */
/*                                                                            */
/* Total Tests: 1                                                             */
/*                                                                            */
/* Test Name:   ioctl03                                                       */
/*                                                                            */
/* Author:      Rusty Russell <rusty@rustcorp.com.au>                         */
/*                                                                            */
/* History:     Created - Nov 28 2008 - Rusty Russell <rusty@rustcorp.com.au> */
/*              Ported to LTP                                                 */
/*                      - Nov 28 2008 - Subrata <subrata@linux.vnet.ibm.com>  */
/******************************************************************************/

#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <linux/if_tun.h>

/* Harness Specific Include Files. */
#include "test.h"
#include "usctest.h"

#ifndef TUNGETFEATURES
#define TUNGETFEATURES _IOR('T', 207, unsigned int)
#endif

#ifndef define
#define IFF_VNET_HDR	0x4000
#endif

/* Extern Global Variables */

/* Global Variables */
char *TCID = "ioctl03";		/* test program identifier.              */
int TST_TOTAL = 1;		/* total number of tests in this file.   */

/* Extern Global Functions */
/******************************************************************************/
/*                                                                            */
/* Function:    cleanup                                                       */
/*                                                                            */
/* Description: Performs all one time clean up for this test on successful    */
/*              completion,  premature exit or  failure. Closes all temporary */
/*              files, removes all temporary directories exits the test with  */
/*              appropriate return code by calling tst_exit() function.       */
/*                                                                            */
/* Input:       None.                                                         */
/*                                                                            */
/* Output:      None.                                                         */
/*                                                                            */
/* Return:      On failure - Exits calling tst_exit(). Non '0' return code.   */
/*              On success - Exits calling tst_exit(). With '0' return code.  */
/*                                                                            */
/******************************************************************************/
extern void cleanup()
{

	TEST_CLEANUP;
	tst_rmdir();

}

/* Local  Functions */
/******************************************************************************/
/*                                                                            */
/* Function:    setup                                                         */
/*                                                                            */
/* Description: Performs all one time setup for this test. This function is   */
/*              typically used to capture signals, create temporary dirs      */
/*              and temporary files that may be used in the course of this    */
/*              test.                                                         */
/*                                                                            */
/* Input:       None.                                                         */
/*                                                                            */
/* Output:      None.                                                         */
/*                                                                            */
/* Return:      On failure - Exits by calling cleanup().                      */
/*              On success - returns 0.                                       */
/*                                                                            */
/******************************************************************************/
void setup()
{
	/* Capture signals if any */
	/* Create temporary directories */
	TEST_PAUSE;
	tst_tmpdir();
}

static struct {
	unsigned int flag;
	const char *name;
} known_flags[] = {
	{
	IFF_TUN, "TUN"}, {
	IFF_TAP, "TAP"}, {
	IFF_NO_PI, "NO_PI"}, {
	IFF_ONE_QUEUE, "ONE_QUEUE"}, {
	IFF_VNET_HDR, "VNET_HDR"}
};

int main()
{
	unsigned int features, i;

	setup();
	tst_require_root(NULL);

	int netfd = open("/dev/net/tun", O_RDWR);
	if (netfd < 0)
		tst_brkm(TBROK|TERRNO, cleanup, "opening /dev/net/tun failed");

	if (ioctl(netfd, TUNGETFEATURES, &features) != 0)
		tst_brkm(TCONF, cleanup,
		    "Kernel does not support TUNGETFEATURES");
	tst_resm(TINFO, "Available features are: %#x", features);
	for (i = 0; i < sizeof(known_flags) / sizeof(known_flags[0]); i++) {
		if (features & known_flags[i].flag) {
			features &= ~known_flags[i].flag;
			tst_resm(TINFO, "%s %#x", known_flags[i].name,
				 known_flags[i].flag);
		}
	}
	if (features)
		tst_resm(TFAIL, "(UNKNOWN %#x)", features);
	cleanup();
	tst_exit();
}