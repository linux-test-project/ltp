// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2015 Cyril Hrubis <chrubis@suse.cz>
 * Copyright (C) 2021 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

#ifndef FUTEX_UTILS_H__
#define FUTEX_UTILS_H__

#include <stdio.h>
#include <stdlib.h>

#if __NR_futex != __LTP__NR_INVALID_SYSCALL && __NR_futex_time64 != __LTP__NR_INVALID_SYSCALL
# define FUTEX_VARIANTS 2
#else
# define FUTEX_VARIANTS 1
#endif

static inline struct futex_test_variants futex_variant(void)
{
	struct futex_test_variants variants[] = {
	#if (__NR_futex != __LTP__NR_INVALID_SYSCALL)
		{ .fntype = FUTEX_FN_FUTEX, .desc = "syscall with old kernel spec" },
	#endif

	#if (__NR_futex_time64 != __LTP__NR_INVALID_SYSCALL)
		{ .fntype = FUTEX_FN_FUTEX64, .desc = "syscall time64 with kernel spec" },
	#endif
	};

	return variants[tst_variant];
}

/*
 * Wait for nr_threads to be sleeping
 */
static inline int wait_for_threads(unsigned int nr_threads)
{
	char thread_state, name[1024];
	DIR *dir;
	struct dirent *dent;
	unsigned int cnt = 0;

	snprintf(name, sizeof(name), "/proc/%i/task/", getpid());

	dir = SAFE_OPENDIR(name);

	while ((dent = SAFE_READDIR(dir))) {
		/* skip ".", ".." and the main thread */
		if (atoi(dent->d_name) == getpid() || atoi(dent->d_name) == 0)
			continue;

		snprintf(name, sizeof(name), "/proc/%i/task/%s/stat",
			 getpid(), dent->d_name);

		SAFE_FILE_SCANF(name, "%*i %*s %c", &thread_state);

		if (thread_state != 'S') {
			tst_res(TINFO, "Thread %s not sleeping yet", dent->d_name);
			SAFE_CLOSEDIR(dir);
			return 1;
		}
		cnt++;
	}

	SAFE_CLOSEDIR(dir);

	if (cnt != nr_threads) {
		tst_res(TINFO, "%u threads sleeping, expected %u", cnt,
			nr_threads);
	}

	return 0;
}

#endif /* FUTEX_UTILS_H__ */
