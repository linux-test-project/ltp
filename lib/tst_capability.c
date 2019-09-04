// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2019 Richard Palethorpe <rpalethorpe@suse.com>
 */

#include <string.h>

#define TST_NO_DEFAULT_MAIN
#include "tst_test.h"
#include "tst_capability.h"

#include "lapi/syscalls.h"

int tst_capget(struct tst_cap_user_header *hdr,
	       struct tst_cap_user_data *data)
{
	return tst_syscall(__NR_capget, hdr, data);
}

int tst_capset(struct tst_cap_user_header *hdr,
	       const struct tst_cap_user_data *data)
{
	return tst_syscall(__NR_capset, hdr, data);
}

static void do_cap_drop(uint32_t *set, uint32_t mask, const struct tst_cap *cap)
{
	if (*set & mask) {
		tst_res(TINFO, "Dropping %s(%d)", cap->name, cap->id);
		*set &= ~mask;
	}
}

static void do_cap_req(uint32_t *permitted, uint32_t *effective, uint32_t mask,
		       const struct tst_cap *cap)
{
	if (!(*permitted & mask))
		tst_brk(TCONF, "Need %s(%d)", cap->name, cap->id);

	if (!(*effective & mask)) {
		tst_res(TINFO, "Permitting %s(%d)", cap->name, cap->id);
		*effective |= mask;
	}
}

void tst_cap_action(struct tst_cap *cap)
{
	struct tst_cap_user_header hdr = {
		.version = 0x20080522,
		.pid = tst_syscall(__NR_gettid),
	};
	struct tst_cap_user_data cur[2] = { {0} };
	struct tst_cap_user_data new[2] = { {0} };
	uint32_t act = cap->action;
	uint32_t *pE = &new[CAP_TO_INDEX(cap->id)].effective;
	uint32_t *pP = &new[CAP_TO_INDEX(cap->id)].permitted;
	uint32_t mask = CAP_TO_MASK(cap->id);

	if (tst_capget(&hdr, cur))
		tst_brk(TBROK | TTERRNO, "tst_capget()");

	memcpy(new, cur, sizeof(new));

	switch (act) {
	case TST_CAP_DROP:
		do_cap_drop(pE, mask, cap);
		break;
	case TST_CAP_REQ:
		do_cap_req(pP, pE, mask, cap);
		break;
	default:
		tst_brk(TBROK, "Unrecognised action %d", cap->action);
	}

	if (!memcmp(cur, new, sizeof(new)))
		return;

	if (tst_capset(&hdr, new))
		tst_brk(TBROK | TERRNO, "tst_capset(%s)", cap->name);
}

void tst_cap_setup(struct tst_cap *caps, unsigned int action_mask)
{
	struct tst_cap *cap;

	for (cap = caps; cap->action; cap++) {
		if (cap->action & action_mask)
			tst_cap_action(cap);
	}
}
