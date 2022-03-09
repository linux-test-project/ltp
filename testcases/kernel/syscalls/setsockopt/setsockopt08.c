// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2021 SUSE LLC <rpalethorpe@suse.com>
 * Based on reproducer by Nicolai Stange based on PoC Andy Nguyen
 */
/*\
 * [Description]
 *
 * This will reproduce the bug on x86_64 in 32bit compatibility
 * mode. It is most reliable with KASAN enabled. Otherwise it relies
 * on the out-of-bounds write corrupting something which leads to a
 * crash. It will run in other scenarious, but is not a test for the
 * CVE.
 *
 * See https://google.github.io/security-research/pocs/linux/cve-2021-22555/writeup.html
 *
 * Also below is Nicolai's detailed description of the bug itself.
 *
 * The problem underlying CVE-2021-22555 fixed by upstream commit
 * b29c457a6511 ("netfilter: x_tables: fix compat match/target pad
 * out-of-bound write") is that the (now removed) padding zeroing code
 * in xt_compat_target_from_user() had been based on the premise that
 * the user specified ->u.user.target_size, which will be considered
 * for the target buffer allocation size, is greater or equal than
 * what's needed to fit the corresponding xt_target instance's
 * ->targetsize: if OTOH the user specified ->u.user.target_size is
 * too small, then the memset() destination address calculated by
 * adding ->targetsize to the payload start will not point at, but
 * into or even past the padding. For the table's last entry's target
 * record, this will result in an out-of-bounds write past the
 * destination buffer allocated for the converted table. The code
 * below will create a (compat) table such that the converted table's
 * calculated size will fit exactly into a slab size of 1024 bytes and
 * that the memset() in xt_compat_target_from_user() will write past
 * this slab.
 *
 * The table will consist of
 *  - the mandatory struct compat_ipt_replace header,
 *  - a single entry consisting of
 *    - the mandatory compat_ipt_entry header
 *    - a single 'state' match entry of appropriate size for
 *      controlling the out-of-bounds write when converting
 *      the target entry following next,
 *    - a single 'REJECT' target entry.
 * The kernel will transform this into a buffer containing (in
 * this order)
 * - a xt_table_info
 * - a single entry consisting of
 *   - its ipt_entry header
 *   - a single 'state' match entry
 *   - followed by a single 'REJECT' target entry.
 *
 * The expected sizes for the 'state' match entries as well as the
 * 'REJECT' target are the size of the base header struct (32 bytes)
 * plus the size of an unsigned int (4 bytes) each. In the course of
 * the compat => non-compat conversion, the kernel will insert four
 * bytes of padding after the unsigned int payload (c.f.  'off'
 * adjustments via xt_compat_match_offset() and
 * xt_compat_target_offset() in xt_compat_match_from_user() and
 * xt_compat_target_from_user() resp.). This code is based on the
 * premise that the user sets the given ->u.user.match_size or
 * ->u.user.target_size consistent to the COMPAT_XT_ALIGN()ed payload
 * size as specified by the corresponding xt_match instance's
 * ->matchsize or xt_target instance's ->targetsize. That is, the
 * padding gets inserted unconditionally during the transformation,
 * independent of the actual values of ->u.user.match_size or
 * ->u.user.target_size and the result ends up getting layed out with
 * proper alignment only if said values match the expectations. That's
 * not a problem in itself, but this unconditional insertion of
 * padding must be taken into account in the match_size calculation
 * below.
 *
 * For the match_size calculation below, note that the chosen
 * target slab size is 1024 and that
 *  - sizeof(xt_table_info) = 64
 *  - sizeof(ipt_entry) = 112
 *  - the kernel will insert four bytes of padding
 *    after the match and target entries each.
 *  - sizeof(struct xt_entry_target) = 32
 */

#include <netinet/in.h>

#include "tst_test.h"
#include "tst_safe_net.h"
#include "lapi/ip_tables.h"
#include "lapi/namespaces_constants.h"

static void *buffer;

void setup(void)
{
	if (tst_kernel_bits() == 32 || sizeof(long) > 4) {
		tst_res(TINFO,
			"The vulnerability was only present in 32-bit compat mode");
	}

	SAFE_TRY_FILE_PRINTF("/proc/sys/user/max_user_namespaces", "%d", 10);

	SAFE_UNSHARE(CLONE_NEWUSER);
	SAFE_UNSHARE(CLONE_NEWNET);
}

void run(void)
{
	const char *const res_fmt_str =
		"setsockopt(%d, IPPROTO_IP, IPT_SO_SET_REPLACE, %p, 1)";
	struct ipt_replace *ipt_replace = buffer;
	struct ipt_entry *ipt_entry = &ipt_replace->entries[0];
	struct xt_entry_match *xt_entry_match =
		(struct xt_entry_match *)&ipt_entry->elems[0];
	const size_t tgt_size = 32;
	const size_t match_size = 1024 - 64 - 112 - 4 - tgt_size - 4;
	struct xt_entry_target *xt_entry_tgt =
		((struct xt_entry_target *) (&ipt_entry->elems[0] + match_size));
	int fd = SAFE_SOCKET(AF_INET, SOCK_DGRAM, 0);
	int result;

	xt_entry_match->u.user.match_size = (u_int16_t)match_size;
	strcpy(xt_entry_match->u.user.name, "state");

	xt_entry_tgt->u.user.target_size = (u_int16_t)tgt_size;
	strcpy(xt_entry_tgt->u.user.name, "REJECT");

	ipt_entry->target_offset =
		(__builtin_offsetof(struct ipt_entry, elems) + match_size);
	ipt_entry->next_offset = ipt_entry->target_offset + tgt_size;

	strcpy(ipt_replace->name, "filter");
	ipt_replace->num_entries = 1;
	ipt_replace->num_counters = 1;
	ipt_replace->size = ipt_entry->next_offset;

	TEST(setsockopt(fd, IPPROTO_IP, IPT_SO_SET_REPLACE, buffer, 1));

	if (TST_RET == -1 && TST_ERR == ENOPROTOOPT)
		tst_brk(TCONF | TTERRNO, res_fmt_str, fd, buffer);

	result = (TST_RET == -1 && TST_ERR == EINVAL) ? TPASS : TFAIL;
	tst_res(result | TTERRNO, res_fmt_str, fd, buffer);

	SAFE_CLOSE(fd);
}

static struct tst_test test = {
	.setup = setup,
	.test_all = run,
	.taint_check = TST_TAINT_W | TST_TAINT_D,
	.forks_child = 1,
	.bufs = (struct tst_buffers []) {
		{&buffer, .size = 2048},
		{},
	},
	.needs_kconfigs = (const char *[]) {
		"CONFIG_NETFILTER_XT_MATCH_STATE",
		"CONFIG_IP_NF_TARGET_REJECT",
		"CONFIG_USER_NS=y",
		"CONFIG_NET_NS=y",
		NULL
	},
	.save_restore = (const struct tst_path_val const[]) {
		{"?/proc/sys/user/max_user_namespaces", NULL},
		NULL,
	},
	.tags = (const struct tst_tag[]) {
		{"linux-git", "b29c457a6511"},
		{"CVE", "2021-22555"},
		{}
	}
};
