/******************************************************************************/
/*                                                                            */
/* Copyright (c) International Business Machines  Corp., 2007, 2008           */
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
/*
 * File: verify_caps_exec.c
 * Author: Serge Hallyn
 * Purpose: perform several tests of file capabilities:
 *  1. try setting caps without CAP_SYS_ADMIN
 *  2. test proper calculation of pI', pE', and pP'.
 *     Try setting valid caps, drop rights, and run the executable,
 *     make sure we get the rights
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <endian.h>
#include <byteswap.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <attr/xattr.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/capability.h>
#include <sys/prctl.h>
#include <test.h>

#define TSTPATH "./print_caps"
char *TCID = "filecaps";
int TST_TOTAL=1;

int errno;

void usage(char *me)
{
	tst_resm(TFAIL, "Usage: %s <0|1> [arg]\n", me);
	tst_resm(TINFO, "  0: set file caps without CAP_SYS_ADMIN\n");
	tst_resm(TINFO, "  1: test that file caps are set correctly on exec\n");
	tst_exit(1);
}

#define DROP_PERMS 0
#define KEEP_PERMS 1

void print_my_caps()
{
	cap_t cap = cap_get_proc();
	tst_resm(TINFO, "\ncaps are %s\n", cap_to_text(cap, NULL));
}

int drop_root(int keep_perms)
{
	int ret;

	if (keep_perms)
		prctl(PR_SET_KEEPCAPS, 1);
	ret = setresuid(1000, 1000, 1000);
	if (ret) {
		perror("setresuid");
		tst_resm(TFAIL, "Error dropping root privs\n");
		tst_exit(4);
	}
	if (keep_perms) {
		cap_t cap = cap_from_text("=eip cap_setpcap-eip");
		cap_set_proc(cap);
	}

	return 1;
}

#if BYTE_ORDER == LITTLE_ENDIAN
#define cpu_to_le32(x)  x
#else
#define cpu_to_le32(x)  bswap_32(x)
#endif

/*
 * TODO: find a better way to do this.  Emulate libcap's
 * way, or just take it from linux/capability.h
 */
/*
 * TODO: accomodate 64-bit capabilities
 */
#define CAPNAME "security.capability"
#ifndef __CAP_BITS
#define __CAP_BITS 31
#endif

#define XATTR_CAPS_SZ (3*sizeof(__le32))
#define VFS_CAP_REVISION_MASK   0xFF000000
#define VFS_CAP_REVISION        0x01000000

#define VFS_CAP_FLAGS_MASK      ~VFS_CAP_REVISION_MASK
#define VFS_CAP_FLAGS_EFFECTIVE 0x000001

int perms_test(void)
{
	int ret;
	unsigned int value[3];
	unsigned int v;

	drop_root(DROP_PERMS);
	v = VFS_CAP_REVISION | VFS_CAP_FLAGS_EFFECTIVE;
	value[0] = cpu_to_le32(v);
	value[1] = 1;
	value[2] = 1;
	ret = setxattr(TSTPATH, CAPNAME, value, 3*sizeof(unsigned int), 0);
	if (ret) {
		perror("setxattr");
		tst_resm(TPASS, "could not set capabilities as non-root\n");
		ret = 0;
	} else {
		tst_resm(TFAIL, "could set capabilities as non-root\n");
		ret = 1;
	}

	return ret;
}

#define FIFOFILE "caps_fifo"
void create_fifo(void)
{
	int ret;

	ret = mkfifo(FIFOFILE, S_IRWXU | S_IRWXG | S_IRWXO);
	if (ret == -1 && errno != EEXIST) {
		perror("mkfifo");
		tst_resm(TFAIL, "failed creating %s\n", FIFOFILE);
		tst_exit(1);
	}
}

void write_to_fifo(char *buf)
{
	int fd;

	fd = open(FIFOFILE, O_WRONLY);
	write(fd, buf, strlen(buf));
	close(fd);
}

void read_from_fifo(char *buf)
{
	int fd;

	memset(buf, 0, 200);
	fd = open(FIFOFILE, O_RDONLY);
	if (fd < 0) {
		perror("open");
		tst_resm(TFAIL, "Failed opening fifo\n");
		tst_exit(1);
	}
	read(fd, buf, 199);
	close(fd);
}

int compare_caps(char *buf1, char *buf2)
{
	int res;

	res = strcmp(buf1, buf2) == 0;
	return res;
}

int fork_drop_and_exec(int keepperms, char *capstxt)
{
	int pid;
	int ret = 0;
	char buf[200], *p;
	static int seqno = 0;

	pid = fork();
	if (pid < 0) {
		perror("fork");
		tst_resm(TFAIL, "%s: failed fork\n", __FUNCTION__);
		tst_exit(1);
	}
	if (pid == 0) {
		drop_root(keepperms);
		print_my_caps();
		sprintf(buf, "%d", seqno);
		ret = execlp(TSTPATH, TSTPATH, buf, NULL);
		perror("execl");
		tst_resm(TFAIL, "%s: exec failed\n", __FUNCTION__);
		snprintf(buf, 200, "failed to run as %s\n", capstxt);
		write_to_fifo(buf);
		tst_exit(1);
	} else {
		p = buf;
		while (1) {
			int c, s;
			read_from_fifo(buf);
			c = sscanf(buf, "%d", &s);
			if (c==1 && s==seqno)
				break;
			tst_resm(TINFO, "got a bad seqno (c=%d, s=%d, seqno=%d)",
				c, s, seqno);
		}
		p = index(buf, '.')+1;
		if (p==(char *)1) {
			tst_resm(TFAIL, "got a bad message from print_caps\n");
			tst_exit(1);
		}
		tst_resm(TINFO, "Expected to run as .%s., ran as .%s..\n",
			capstxt, p);
		if (strcmp(p, capstxt) != 0) {
			tst_resm(TINFO, "those are not the same\n");
			ret = -1;
		}
		seqno++;
	}
	return ret;
}

int caps_actually_set_test(void)
{
	int i, whichset, whichcap, finalret = 0, ret;
	cap_t cap;
	char *capstxt;
	unsigned int value[3];
	cap_value_t capvalue[1];
	unsigned int magic;

	magic = VFS_CAP_REVISION;

	cap = cap_init();
	if (!cap) {
		perror("cap_init");
		exit(2);
	}

	create_fifo();

	/* first, try each bit in fP (forced) with fE on and off. */
	value[1] = value[2] =  cpu_to_le32(0);
	for (whichcap=0; whichcap < __CAP_BITS; whichcap++) {
		if (whichcap == 8)
			continue;
		/* fE = 0, don't gain the perm */
		capvalue[0] = whichcap;
		value[0] = cpu_to_le32(magic);
		value[1] = cpu_to_le32(1 << whichcap);
		ret = setxattr(TSTPATH, CAPNAME, value, 3*sizeof(unsigned int), 0);
		if (ret) {
			tst_resm(TINFO, "%d %d\n", whichset, whichcap);
			perror("setxattr");
			continue;
		}
		/* do a sanity check */
		cap_clear(cap);
		cap_set_flag(cap, CAP_PERMITTED, 1, capvalue, CAP_SET);
		capstxt = cap_to_text(cap, NULL);
		ret = fork_drop_and_exec(DROP_PERMS, capstxt);
		if (ret) {
			tst_resm(TINFO, "Failed CAP_PERMITTED=%d CAP_EFFECTIVE=0\n",
					whichcap);
			if (!finalret)
				finalret = ret;
		}

		/* fE = 1, do gain the perm */
		value[0] = cpu_to_le32(magic | VFS_CAP_FLAGS_EFFECTIVE);
		value[1] = cpu_to_le32(1 << whichcap);
		ret = setxattr(TSTPATH, CAPNAME, value, 3*sizeof(unsigned int), 0);
		if (ret) {
			tst_resm(TINFO, "%d %d\n", whichset, whichcap);
			perror("setxattr");
			continue;
		}
		/* do a sanity check */
		cap_clear(cap);
		cap_set_flag(cap, CAP_PERMITTED, 1, capvalue, CAP_SET);
		cap_set_flag(cap, CAP_EFFECTIVE, 1, capvalue, CAP_SET);
		capstxt = cap_to_text(cap, NULL);
		if (strcmp(capstxt, "=")==0) {
			tst_resm(TINFO, "%s: libcap doesn't know about cap %d, not running\n",
				__FUNCTION__, whichcap);
			ret = 0;
		} else
			ret = fork_drop_and_exec(DROP_PERMS, capstxt);
		if (ret) {
			tst_resm(TINFO, "Failed CAP_PERMITTED=%d CAP_EFFECTIVE=1\n",
				whichcap);
			if (!finalret)
				finalret = ret;
		}
	}


	/*
	 * next try each bit in fI
	 * The first two attemps have the bit which is in fI in pI.
	 *     This should result in the bit being in pP'.
	 *     If fE was set then it should also be in pE'.
	 * The last attempt starts with an empty pI.
	 *     This should result in empty capability, as there were
	 *     no bits to be inherited from the original process.
	 */
	value[1] = value[2] =  cpu_to_le32(0);
	for (whichcap=0; whichcap < __CAP_BITS; whichcap++) {
		if (whichcap == 8)
			continue;
		/*
		 * bit is in fI and pI, so should be in pI'.
		 * but fE=0, so cap is in pP' but not pE'.
		 */
		value[0] = cpu_to_le32(magic);
		value[2] = cpu_to_le32(1 << whichcap);
		ret = setxattr(TSTPATH, CAPNAME, value, 3*sizeof(unsigned int), 0);
		if (ret) {
			tst_resm(TINFO, "%d %d\n", whichset, whichcap);
			perror("setxattr");
			continue;
		}
		/* do a sanity check */
		cap_clear(cap);
		for (i=0; i<32; i++) {
			if (i != 8) {
				capvalue[0] = i;
				cap_set_flag(cap, CAP_INHERITABLE, 1, capvalue, CAP_SET);
			}
		}
		capvalue[0] = whichcap;
		cap_set_flag(cap, CAP_PERMITTED, 1, capvalue, CAP_SET);
		capstxt = cap_to_text(cap, NULL);
		ret = fork_drop_and_exec(KEEP_PERMS,  capstxt);
		if (ret) {
			tst_resm(TINFO, "Failed with_perms CAP_INHERITABLE=%d "
					"CAP_EFFECTIVE=0\n", whichcap);
			if (!finalret)
				finalret = ret;
		}

		/*
		 * bit is in fI and pI, so should be in pI'.
		 * and fE=1, so cap is in pP' and pE'.
		 */

		value[0] = cpu_to_le32(magic | VFS_CAP_FLAGS_EFFECTIVE);
		value[2] = cpu_to_le32(1 << whichcap);
		ret = setxattr(TSTPATH, CAPNAME, value, 3*sizeof(unsigned int), 0);
		if (ret) {
			tst_resm(TINFO, "%d %d\n", whichset, whichcap);
			perror("setxattr");
			continue;
		}
		/* do a sanity check */
		cap_clear(cap);
		for (i=0; i<32; i++) {
			if (i != 8) {
				capvalue[0] = i;
				cap_set_flag(cap, CAP_INHERITABLE, 1, capvalue, CAP_SET);
			}
		}
		capvalue[0] = whichcap;
		cap_set_flag(cap, CAP_PERMITTED, 1, capvalue, CAP_SET);
		cap_set_flag(cap, CAP_EFFECTIVE, 1, capvalue, CAP_SET);
		capstxt = cap_to_text(cap, NULL);
		if (strcmp(capstxt, "=")==0) {
			tst_resm(TINFO, "%s: libcap doesn't know about cap %d, not running\n",
				__FUNCTION__, whichcap);
			ret = 0;
		} else
			ret = fork_drop_and_exec(KEEP_PERMS, capstxt);
		if (ret) {
			tst_resm(TINFO, "Failed with_perms CAP_INHERITABLE=%d "
					"CAP_EFFECTIVE=1\n", whichcap);
			if (!finalret)
				finalret = ret;
		}

		/*
		 * bit is in fI but not in pI
		 * So pP' is empty.
		 * pE' must be empty.
		 */
		value[0] = cpu_to_le32(magic | VFS_CAP_FLAGS_EFFECTIVE);
		value[2] = cpu_to_le32(1 << whichcap);
		ret = setxattr(TSTPATH, CAPNAME, value, 3*sizeof(unsigned int), 0);
		if (ret) {
			tst_resm(TINFO, "%d %d\n", whichset, whichcap);
			perror("setxattr");
			continue;
		}
		/* do a sanity check */
		cap_clear(cap);
		capstxt = cap_to_text(cap, NULL);
		ret = fork_drop_and_exec(DROP_PERMS, capstxt);
		if (ret) {
			tst_resm(TINFO, "Failed without_perms CAP_INHERITABLE=%d",
					whichcap);
			if (!finalret)
				finalret = ret;
		}
	}

	cap_free(cap);
	return finalret;
}

int main(int argc, char *argv[])
{
	int ret = 0;

	if (argc < 2)
		usage(argv[0]);

	switch(atoi(argv[1])) {
		case 0:
			ret = perms_test();
			break;
		case 1:
			ret = caps_actually_set_test();
			if (ret)
				tst_resm(TFAIL, "Some tests failed\n");
			else
				tst_resm(TPASS, "All tests passed\n");
			break;
		default: usage(argv[0]);
	}

	tst_exit(ret);
}
