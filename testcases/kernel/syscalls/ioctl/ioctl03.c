/*
 *
 * Copyright (c) Rusty Russell <rusty@rustcorp.com.au>
 * Copyright (c) International Business Machines  Corp., 2008
 *
 * This program is free software;  you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY;  without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 * the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program;  if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/*
 *
 *
 * File:        ioctl03.c
 *
 * Description: This program tests whether all the valid IFF flags are
 *              returned properly by implementation of TUNGETFEATURES ioctl
 *              on kernel 2.6.27
 *
 * Total Tests: 1
 *
 * Test Name:   ioctl03
 *
 * Author:      Rusty Russell <rusty@rustcorp.com.au>
 *
 * History:     Created - Nov 28 2008 - Rusty Russell <rusty@rustcorp.com.au>
 *              Ported to LTP
 *                      - Nov 28 2008 - Subrata <subrata@linux.vnet.ibm.com>
 */

#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <linux/if_tun.h>

#include "test.h"

#ifndef TUNGETFEATURES
#define TUNGETFEATURES _IOR('T', 207, unsigned int)
#endif

#ifndef IFF_VNET_HDR
#define IFF_VNET_HDR	0x4000
#endif

#ifndef IFF_MULTI_QUEUE
#define IFF_MULTI_QUEUE	0x0100
#endif

char *TCID = "ioctl03";
int TST_TOTAL = 1;

static void cleanup(void)
{
	tst_rmdir();
}

static void setup(void)
{
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
	IFF_VNET_HDR, "VNET_HDR"}, {
	IFF_MULTI_QUEUE, "MULTI_QUEUE"}
};

int main(void)
{
	unsigned int features, i;

	setup();
	tst_require_root();

	int netfd = open("/dev/net/tun", O_RDWR);
	if (netfd < 0)
		tst_brkm(TBROK | TERRNO, cleanup,
			 "opening /dev/net/tun failed");

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
