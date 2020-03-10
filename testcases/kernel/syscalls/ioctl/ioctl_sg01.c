// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2019 SUSE LLC <mdoucha@suse.cz>
 */

/*
 * CVE-2018-1000204
 *
 * Test ioctl(SG_IO) and check that kernel doesn't leak data. Requires
 * a read-accessible SCSI-compatible device (e.g. SATA disk). Running mem01
 * test program before this one may increase the chance of successfully
 * reproducing the bug.
 *
 * Leak fixed in:
 *
 *  commit a45b599ad808c3c982fdcdc12b0b8611c2f92824
 *  Author: Alexander Potapenko <glider@google.com>
 *  Date:   Fri May 18 16:23:18 2018 +0200
 *
 *  scsi: sg: allocate with __GFP_ZERO in sg_build_indirect()
 */

#include <sys/types.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>
#include <scsi/sg.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include "tst_test.h"

#define BUF_SIZE 128 * 4096
#define CMD_SIZE 6

static int devfd = -1;
static char buffer[BUF_SIZE];
static unsigned char command[CMD_SIZE];
static struct sg_io_hdr query;

/* TODO: split this off to a separate SCSI library? */
static const char *find_generic_scsi_device(int access_flags)
{
	DIR *devdir;
	struct dirent *ent;
	int tmpfd;
	static char devpath[PATH_MAX];

	errno = 0;
	devdir = opendir("/dev");

	if (!devdir)
		return NULL;

	while ((ent = SAFE_READDIR(devdir))) {
		/* The bug is most likely reproducible only on /dev/sg* */
		if (strncmp(ent->d_name, "sg", 2) || !isdigit(ent->d_name[2]))
			continue;

		snprintf(devpath, PATH_MAX, "/dev/%s", ent->d_name);
		/* access() makes incorrect assumptions about block devices */
		tmpfd = open(devpath, access_flags);

		if (tmpfd >= 0) {
			SAFE_CLOSE(tmpfd);
			SAFE_CLOSEDIR(devdir);
			return devpath;
		}
	}

	SAFE_CLOSEDIR(devdir);
	return NULL;
}

static void setup(void)
{
	const char *devpath = find_generic_scsi_device(O_RDONLY);

	if (!devpath)
		tst_brk(TCONF, "Could not find any usable SCSI device");

	tst_res(TINFO, "Found SCSI device %s", devpath);
	devfd = SAFE_OPEN(devpath, O_RDONLY);
	query.interface_id = 'S';
	query.dxfer_direction = SG_DXFER_FROM_DEV;
	query.cmd_len = CMD_SIZE;
	query.dxfer_len = BUF_SIZE;
	query.dxferp = buffer;
	query.cmdp = command;
}

static void cleanup(void)
{
	if (devfd >= 0)
		SAFE_CLOSE(devfd);
}

static void run(void)
{
	size_t i;

	memset(buffer, 0, BUF_SIZE);
	TEST(ioctl(devfd, SG_IO, &query));

	if (TST_RET != 0 && TST_RET != -1)
		tst_brk(TBROK | TTERRNO, "Invalid ioctl() return value");

	/* Check the output buffer even if ioctl() failed, just in case. */
	for (i = 0; i < BUF_SIZE; i++) {
		if (buffer[i]) {
			tst_res(TFAIL, "Kernel memory leaked");
			return;
		}
	}

	tst_res(TPASS, "Output buffer is empty, no data leaked");
}

static struct tst_test test = {
	.test_all = run,
	.setup = setup,
	.cleanup = cleanup,
	.tags = (const struct tst_tag[]) {
		{"linux-git", "e093c4be760e"},
		{"CVE", "2018-1000204"},
		{}
	}
};
