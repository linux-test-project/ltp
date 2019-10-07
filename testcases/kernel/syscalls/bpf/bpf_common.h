/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (c) 2019 Linux Test Project
 */

#ifndef LTP_BPF_COMMON_H
#define LTP_BPF_COMMON_H

#define BPF_MEMLOCK_ADD (256*1024)

void rlimit_bump_memlock(void)
{
	struct rlimit memlock_r;

	SAFE_GETRLIMIT(RLIMIT_MEMLOCK, &memlock_r);
	memlock_r.rlim_cur += BPF_MEMLOCK_ADD;
	tst_res(TINFO, "Raising RLIMIT_MEMLOCK to %ld",
		(long)memlock_r.rlim_cur);

	if (memlock_r.rlim_cur <= memlock_r.rlim_max) {
		SAFE_SETRLIMIT(RLIMIT_MEMLOCK, &memlock_r);
	} else if ((geteuid() == 0)) {
		memlock_r.rlim_max += BPF_MEMLOCK_ADD;
		SAFE_SETRLIMIT(RLIMIT_MEMLOCK, &memlock_r);
	} else {
		tst_res(TINFO, "Can't raise RLIMIT_MEMLOCK, test may fail "
			"due to lack of max locked memory");
	}
}

int bpf_map_create(union bpf_attr *attr)
{
	TEST(bpf(BPF_MAP_CREATE, attr, sizeof(*attr)));
	if (TST_RET == -1) {
		if (TST_ERR == EPERM) {
			tst_brk(TCONF | TTERRNO,
				"bpf() requires CAP_SYS_ADMIN on this system");
		} else {
			tst_brk(TBROK | TTERRNO, "Failed to create array map");
		}
	}

	return TST_RET;
}

#endif
