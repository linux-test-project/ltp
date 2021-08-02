// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2017 Richard Palethorpe <rpalethorpe@suse.com>
 * Based on repro-compatReleaseEntry.c by NCC group
 */
/*
 * Test for CVE-2016-4997
 *
 * For a full explanation of how the vulnerability works see:
 * https://github.com/nccgroup/TriforceLinuxSyscallFuzzer/tree/master/crash_reports/report_compatIpt
 *
 * The original vulnerability was present in the 32-bit compatibility system
 * call, so the test should be compiled with -m32 and run on a 64-bit kernel.
 * For simplicities sake the test requests root privliges instead of creating
 * a user namespace.
 */

#include <stdint.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if.h>
#include <limits.h>
#include <linux/netfilter_ipv4/ip_tables.h>

#include "tst_test.h"
#include "tst_safe_net.h"
#include "tst_kernel.h"

#define TOO_SMALL_OFFSET 74
#define OFFSET_OVERWRITE 0xFFFF
#define NEXT_OFFSET (sizeof(struct ipt_entry)		\
		     + sizeof(struct xt_entry_match)	\
		     + sizeof(struct xt_entry_target))
#define PADDING (OFFSET_OVERWRITE - NEXT_OFFSET)

#ifndef HAVE_STRUCT_XT_ENTRY_MATCH
struct xt_entry_match {
	union {
		struct {
			uint16_t match_size;
			char name[29];
			uint8_t revision;
		} user;
		struct {
			uint16_t match_size;
			void *match;
		} kernel;
		uint16_t match_size;
	} u;
	unsigned char data[0];
};
#endif

#ifndef HAVE_STRUCT_XT_ENTRY_TARGET
struct xt_entry_target {
	union {
		struct {
			uint16_t target_size;
			char name[29];
			uint8_t revision;
		} user;
		struct {
			uint16_t target_size;
			void *target;
		} kernel;
		uint16_t target_size;
	} u;
	unsigned char data[0];
};
#endif

struct payload {
	struct ipt_replace repl;
	struct ipt_entry ent;
	struct xt_entry_match match;
	struct xt_entry_target targ;
	char padding[PADDING];
	struct xt_entry_target targ2;
};

static void setup(void)
{
	if (tst_kernel_bits() == 32 || sizeof(long) > 4)
		tst_res(TCONF,
			"The vulnerability was only present in 32-bit compat mode");
}

static void run(void)
{
	int ret, sock_fd;
	struct payload p = { 0 };

	sock_fd = SAFE_SOCKET(AF_INET, SOCK_DGRAM, 0);

	strncpy(p.match.u.user.name, "icmp", sizeof(p.match.u.user.name));
	p.match.u.match_size = OFFSET_OVERWRITE;

	p.ent.next_offset = NEXT_OFFSET;
	p.ent.target_offset = TOO_SMALL_OFFSET;

	p.repl.num_entries = 2;
	p.repl.num_counters = 1;
	p.repl.size = sizeof(struct payload);
	p.repl.valid_hooks = 0;

	ret = setsockopt(sock_fd, SOL_IP, IPT_SO_SET_REPLACE,
			 &p, sizeof(struct payload));
	tst_res(TPASS | TERRNO, "We didn't cause a crash, setsockopt returned %d", ret);
}

static struct tst_test test = {
	.min_kver = "2.6.32",
	.setup = setup,
	.test_all = run,
	.needs_root = 1,
	.tags = (const struct tst_tag[]){
		{"linux-git", "ce683e5f9d04"},
		{"CVE", "CVE-2016-4997"},
	}
};
