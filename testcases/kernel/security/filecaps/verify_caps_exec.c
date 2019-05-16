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
/* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA    */
/*                                                                            */
/******************************************************************************/
/*
 * File: verify_caps_exec.c
 * Author: Serge Hallyn
 * Purpose: perform several tests of file capabilities:
 *  1. try setting caps without privilege
 *  2. test proper calculation of pI', pE', and pP'.
 *     Try setting valid caps, drop rights, and run the executable,
 *     make sure we get the rights
 */

#include <stdio.h>
#include <unistd.h>
#include <endian.h>
#include <byteswap.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <errno.h>
#include <fcntl.h>
#include "config.h"
#if HAVE_SYS_CAPABILITY_H
#include <linux/types.h>
#include <sys/capability.h>
#endif
#include <sys/prctl.h>
#include "test.h"
#include "filecaps_common.h"

#define TSTPATH "print_caps"
char *TCID = "filecaps";
int TST_TOTAL = 1;

int errno;

static void usage(const char *me)
{
	tst_resm(TFAIL, "Usage: %s <0|1> [arg]\n", me);
	tst_resm(TINFO, "  0: set file caps without privilege\n");
	tst_resm(TINFO, "  1: test that file caps are set correctly on exec\n");
	tst_exit();
}

#define DROP_PERMS 0
#define KEEP_PERMS 1

#ifdef HAVE_LIBCAP
static void print_my_caps(void)
{
	cap_t cap = cap_get_proc();
	char *txt = cap_to_text(cap, NULL);
	tst_resm(TINFO, "\ncaps are %s\n", txt);
	cap_free(cap);
	cap_free(txt);
}

static void drop_root(int keep_perms)
{
	int ret;

	if (keep_perms)
		prctl(PR_SET_KEEPCAPS, 1);
	ret = setresuid(1000, 1000, 1000);
	if (ret) {
		tst_brkm(TFAIL | TERRNO, NULL, "Error dropping root privs\n");
		tst_exit();
	}
	if (keep_perms) {
		cap_t cap = cap_from_text("=eip");
		int ret;
		if (!cap)
			tst_brkm(TBROK | TERRNO, NULL,
				 "cap_from_text failed\n");
		ret = cap_set_proc(cap);
		if (ret < 0)
			tst_brkm(TBROK | TERRNO, NULL, "cap_set_proc failed\n");
		cap_free(cap);
	}
}

static int perms_test(void)
{
	int ret;
	cap_t cap;

	drop_root(DROP_PERMS);
	cap = cap_from_text("all=eip");
	if (!cap) {
		tst_resm(TFAIL, "could not get cap from text for perms test\n");
		return 1;
	}
	ret = cap_set_file(TSTPATH, cap);
	if (ret) {
		tst_resm(TPASS, "could not set capabilities as non-root\n");
		ret = 0;
	} else {
		tst_resm(TFAIL, "could set capabilities as non-root\n");
		ret = 1;
	}

	cap_free(cap);
	return ret;
}

static void create_fifo(void)
{
	int ret;

	ret = mkfifo(get_caps_fifo(), S_IRWXU | S_IRWXG | S_IRWXO);
	if (ret == -1 && errno != EEXIST)
		tst_brkm(TFAIL | TERRNO, NULL, "failed creating %s\n",
			 get_caps_fifo());
}

static void write_to_fifo(const char *buf)
{
	int fd;

	fd = open(get_caps_fifo(), O_WRONLY);
	write(fd, buf, strlen(buf));
	close(fd);
}

static void read_from_fifo(char *buf)
{
	int fd;

	memset(buf, 0, 200);
	fd = open(get_caps_fifo(), O_RDONLY);
	if (fd < 0)
		tst_brkm(TFAIL | TERRNO, NULL, "Failed opening fifo\n");
	read(fd, buf, 199);
	close(fd);
}

static int fork_drop_and_exec(int keepperms, cap_t expected_caps)
{

	int pid;
	int ret = 0;
	char buf[200], *p;
	char *capstxt;
	cap_t actual_caps;
	static int seqno;

	pid = fork();
	if (pid < 0)
		tst_brkm(TFAIL | TERRNO, NULL, "%s: failed fork\n", __func__);
	if (pid == 0) {
		drop_root(keepperms);
		print_my_caps();
		sprintf(buf, "%d", seqno);
		ret = execlp(TSTPATH, TSTPATH, buf, NULL);
		capstxt = cap_to_text(expected_caps, NULL);
		snprintf(buf, 200, "failed to run as %s\n", capstxt);
		cap_free(capstxt);
		write_to_fifo(buf);
		tst_brkm(TFAIL, NULL, "%s: exec failed\n", __func__);
	} else {
		p = buf;
		while (1) {
			int c, s;
			read_from_fifo(buf);
			c = sscanf(buf, "%d", &s);
			if (c == 1 && s == seqno)
				break;
			tst_resm(TINFO,
				 "got a bad seqno (c=%d, s=%d, seqno=%d)", c, s,
				 seqno);
		}
		p = strchr(buf, '.');
		if (!p)
			tst_brkm(TFAIL, NULL,
				 "got a bad message from print_caps\n");
		p += 1;
		actual_caps = cap_from_text(p);
		if (cap_compare(actual_caps, expected_caps) != 0) {
			capstxt = cap_to_text(expected_caps, NULL);
			tst_resm(TINFO,
				 "Expected to run as .%s., ran as .%s..\n",
				 capstxt, p);
			tst_resm(TINFO, "those are not the same\n");
			cap_free(capstxt);
			ret = -1;
		}
		cap_free(actual_caps);
		seqno++;
	}
	return ret;
}

static int caps_actually_set_test(void)
{
	int whichcap, finalret = 0, ret;
	cap_t fcap, pcap, cap_fullpi;
	cap_value_t capvalue[1];
	int i;

	fcap = cap_init();
	pcap = cap_init();
	if (!fcap || !pcap) {
		perror("cap_init");
		exit(2);
	}

	create_fifo();

	int num_caps;

	for (num_caps = 0;; num_caps++) {
#if HAVE_DECL_PR_CAPBSET_READ
		ret = prctl(PR_CAPBSET_READ, num_caps);
#else
		tst_resm(TCONF, "System doesn't have CAPBSET prctls");
		ret = -1;
#endif
		/*
		 * Break from the loop in this manner to avoid incrementing,
		 * then having to decrement value.
		 */
		if (ret == -1)
			break;
	}

	/* first, try each bit in fP (forced) with fE on and off. */
	for (whichcap = 0; whichcap < num_caps; whichcap++) {
		/*
		 * fP=whichcap, fE=fI=0
		 * pP'=whichcap, pI'=pE'=0
		 */
		capvalue[0] = whichcap;
		cap_clear(fcap);
		cap_set_flag(fcap, CAP_PERMITTED, 1, capvalue, CAP_SET);
		ret = cap_set_file(TSTPATH, fcap);
		if (ret) {
			tst_resm(TINFO, "%d\n", whichcap);
			continue;
		}
		ret = fork_drop_and_exec(DROP_PERMS, fcap);
		if (ret) {
			tst_resm(TINFO,
				 "Failed CAP_PERMITTED=%d CAP_EFFECTIVE=0\n",
				 whichcap);
			if (!finalret)
				finalret = ret;
		}

/* SERGE here */
		/*
		 * fP = fE = whichcap, fI = 0
		 * pP = pE = whichcap, pI = 0
		 */
		cap_clear(fcap);
		cap_set_flag(fcap, CAP_PERMITTED, 1, capvalue, CAP_SET);
		cap_set_flag(fcap, CAP_EFFECTIVE, 1, capvalue, CAP_SET);
		ret = cap_set_file(TSTPATH, fcap);
		if (ret) {
			tst_resm(TINFO, "%d\n", whichcap);
			continue;
		}
		ret = fork_drop_and_exec(DROP_PERMS, fcap);
		if (ret) {
			tst_resm(TINFO,
				 "Failed CAP_PERMITTED=%d CAP_EFFECTIVE=1\n",
				 whichcap);
			if (!finalret)
				finalret = ret;
		}
	}

	cap_free(pcap);
	cap_free(fcap);
	cap_fullpi = cap_init();
	for (i = 0; i < num_caps; i++) {
		capvalue[0] = i;
		cap_set_flag(cap_fullpi, CAP_INHERITABLE, 1, capvalue, CAP_SET);
	}

	/*
	 * For the inheritable tests, we want to make sure pI starts
	 * filled.
	 */
	ret = cap_set_proc(cap_fullpi);
	if (ret)
		tst_resm(TINFO, "Could not fill pI.  pI tests will fail.\n");

	/*
	 * next try each bit in fI
	 * The first two attemps have the bit which is in fI in pI.
	 *     This should result in the bit being in pP'.
	 *     If fE was set then it should also be in pE'.
	 * The last attempt starts with an empty pI.
	 *     This should result in empty capability, as there were
	 *     no bits to be inherited from the original process.
	 */
	for (whichcap = 0; whichcap < num_caps; whichcap++) {
		cap_t cmpcap;
		capvalue[0] = whichcap;

		/*
		 * fI=whichcap, fP=fE=0
		 * pI=full
		 * pI'=full, pP'=whichcap, pE'=0
		 */
		/* fill pI' */
		pcap = cap_dup(cap_fullpi);
		/* pP' = whichcap */
		cap_set_flag(pcap, CAP_PERMITTED, 1, capvalue, CAP_SET);

		/* fI = whichcap */
		fcap = cap_init();
		cap_set_flag(fcap, CAP_INHERITABLE, 1, capvalue, CAP_SET);
		ret = cap_set_file(TSTPATH, fcap);
		if (ret) {
			tst_resm(TINFO, "%d\n", whichcap);
			continue;
		}
		ret = fork_drop_and_exec(KEEP_PERMS, pcap);
		if (ret) {
			tst_resm(TINFO, "Failed with_perms CAP_INHERITABLE=%d "
				 "CAP_EFFECTIVE=0\n", whichcap);
			if (!finalret)
				finalret = ret;
		}

		/*
		 * fI=fE=whichcap, fP=0
		 * pI=full
		 * pI'=full, pP'=whichcap, pE'=whichcap
		 *
		 * Note that only fE and pE' change, so keep prior
		 * fcap and pcap and set those bits.
		 */

		cap_set_flag(fcap, CAP_EFFECTIVE, 1, capvalue, CAP_SET);
		cap_set_flag(pcap, CAP_EFFECTIVE, 1, capvalue, CAP_SET);
		ret = cap_set_file(TSTPATH, fcap);
		if (ret) {
			tst_resm(TINFO, "%d\n", whichcap);
			continue;
		}
		/* The actual result will be a full pI, with
		 * pE and pP containing just whichcap. */
		cmpcap = cap_dup(cap_fullpi);
		cap_set_flag(cmpcap, CAP_PERMITTED, 1, capvalue, CAP_SET);
		cap_set_flag(cmpcap, CAP_EFFECTIVE, 1, capvalue, CAP_SET);
		ret = fork_drop_and_exec(KEEP_PERMS, cmpcap);
		cap_free(cmpcap);
		if (ret) {
			tst_resm(TINFO, "Failed with_perms CAP_INHERITABLE=%d "
				 "CAP_EFFECTIVE=1\n", whichcap);
			if (!finalret)
				finalret = ret;
		}

		/*
		 * fI=fE=whichcap, fP=0  (so fcap is same as before)
		 * pI=0  (achieved using DROP_PERMS)
		 * pI'=pP'=pE'=0
		 */
		cap_clear(pcap);
		ret = fork_drop_and_exec(DROP_PERMS, pcap);
		if (ret) {
			tst_resm(TINFO,
				 "Failed without_perms CAP_INHERITABLE=%d",
				 whichcap);
			if (!finalret)
				finalret = ret;
		}

		cap_free(fcap);
		cap_free(pcap);
	}

	cap_free(cap_fullpi);

	return finalret;
}
#endif

int main(int argc, char *argv[])
{
#ifdef HAVE_LIBCAP
	if (argc < 2)
		usage(argv[0]);

	int ret = 0;

	switch (atoi(argv[1])) {
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
	default:
		usage(argv[0]);
	}
#else
	tst_resm(TCONF, "System doesn't have POSIX capabilities support.");
#endif

	tst_exit();
}
