// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2019 SUSE LLC <mdoucha@suse.cz>
 */

/*\
 * CVE-2018-1000204
 *
 * Test ioctl(SG_IO) and check that kernel doesn't leak data. Requires
 * a read-accessible generic SCSI device (e.g. a DVD drive).
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
#include "tst_memutils.h"

#define SYSDIR "/sys/block"
#define BLOCKDIR "/sys/block/%s/device/generic"

#define BUF_SIZE (128 * 4096)
#define CMD_SIZE 6

static int devfd = -1;
static char buffer[BUF_SIZE + 1];
static unsigned char command[CMD_SIZE];
static struct sg_io_hdr query;

/* TODO: split this off to a separate SCSI library? */
static const char *find_generic_scsi_device(int access_flags)
{
	DIR *sysdir;
	struct dirent *ent;
	int tmpfd;
	ssize_t length;
	char *filename;
	static char devpath[PATH_MAX], genpath[PATH_MAX];

	sysdir = opendir(SYSDIR);

	if (!sysdir)
		return NULL;

	/* Scan block devices */
	while ((ent = SAFE_READDIR(sysdir))) {
		if (ent->d_name[0] == '.')
			continue;

		snprintf(devpath, PATH_MAX, BLOCKDIR, ent->d_name);
		devpath[PATH_MAX - 1] = '\0';
		length = readlink(devpath, genpath, PATH_MAX - 1);

		if (length < 0)
			continue;

		genpath[length] = '\0';
		filename = basename(genpath);

		snprintf(devpath, PATH_MAX, "/dev/%s", filename);
		/* access() makes incorrect assumptions about block devices */
		tmpfd = open(devpath, access_flags);

		if (tmpfd >= 0) {
			SAFE_CLOSE(tmpfd);
			SAFE_CLOSEDIR(sysdir);
			return devpath;
		}

		tst_res(TINFO | TERRNO, "Cannot open device %s", devpath);
	}

	SAFE_CLOSEDIR(sysdir);
	return NULL;
}

static void dump_hex(const char *str, size_t size)
{
	size_t i;

	for (; size && !str[size - 1]; size--)
		;

	for (i = 0; i < size; i++) {
		if (i && (i % 32) == 0)
			printf("\n");
		else if (i && (i % 4) == 0)
			printf(" ");

		printf("%02x", (unsigned int)str[i]);
	}

	printf("\n");
}

static void setup(void)
{
	const char *devpath = find_generic_scsi_device(O_RDONLY);

	if (!devpath)
		tst_brk(TCONF, "Could not find any usable SCSI device");

	tst_res(TINFO, "Found SCSI device %s", devpath);

	/* Pollute some memory to avoid false negatives */
	tst_pollute_memory(0, 0x42);

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
	size_t i, j;

	memset(buffer, 0, BUF_SIZE);

	for (i = 0; i < 100; i++) {
		TEST(ioctl(devfd, SG_IO, &query));
		buffer[BUF_SIZE] = '\0';

		if (TST_RET != 0 && TST_RET != -1)
			tst_brk(TBROK|TTERRNO, "Invalid ioctl() return value");

		/* Check the buffer even if ioctl() failed, just in case. */
		for (j = 0; j < BUF_SIZE; j++) {
			if (buffer[j]) {
				tst_res(TFAIL, "Kernel memory leaked");
				tst_res(TINFO, "Buffer contents: %s", buffer);
				dump_hex(buffer, BUF_SIZE);
				return;
			}
		}
	}

	tst_res(TPASS, "Output buffer is empty, no data leaked");
}

static struct tst_test test = {
	.test_all = run,
	.setup = setup,
	.cleanup = cleanup,
	.timeout = 3600,
	.tags = (const struct tst_tag[]) {
		{"linux-git", "a45b599ad808"},
		{"CVE", "2018-1000204"},
		{}
	}
};
