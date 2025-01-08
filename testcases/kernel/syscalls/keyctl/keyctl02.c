// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2017 Fujitsu Ltd.
 * Copyright (c) Linux Test Project, 2017-2024
 * Ported: Guangwen Feng <fenggw-fnst@cn.fujitsu.com>
 */

/*\
 * [Description]
 *
 * Regression test for the race between keyctl_read() and
 * keyctl_revoke(), if the revoke happens between keyctl_read()
 * checking the validity of a key and the key's semaphore being taken,
 * then the key type read method will see a revoked key.
 *
 * This causes a problem for the user-defined key type because it
 * assumes in its read method that there will always be a payload
 * in a non-revoked key and doesn't check for a NULL pointer.
 *
 * Bug was fixed in commit
 * b4a1b4f5047e ("KEYS: Fix race between read and revoke")
 */

#include <errno.h>
#include <pthread.h>
#include <sys/types.h>

#include "tst_safe_pthread.h"
#include "tst_test.h"
#include "tst_kconfig.h"
#include "lapi/keyctl.h"

#define LOOPS	20000
#define MAX_WAIT_FOR_GC_MS 5000
#define PATH_KEY_COUNT_QUOTA	"/proc/sys/kernel/keys/root_maxkeys"

static int orig_maxkeys;
static int realtime_kernel;

static void *do_read(void *arg)
{
	key_serial_t key = (unsigned long)arg;
	char buffer[4] = { 0 };

	keyctl(KEYCTL_READ, key, buffer, 4);

	return NULL;
}

static void *do_revoke(void *arg)
{
	key_serial_t key = (unsigned long)arg;

	keyctl(KEYCTL_REVOKE, key);

	return NULL;
}

static void do_test(void)
{
	int i, ret;
	key_serial_t key, key_inv;
	pthread_t pth[4];

	for (i = 0; i < LOOPS; i++) {
		key = add_key("user", "ltptestkey", "foo", 3,
			KEY_SPEC_PROCESS_KEYRING);
		if (key == -1)
			tst_brk(TBROK | TERRNO, "Failed to add key");

		SAFE_PTHREAD_CREATE(&pth[0], NULL, do_read,
			(void *)(unsigned long)key);
		SAFE_PTHREAD_CREATE(&pth[1], NULL, do_revoke,
			(void *)(unsigned long)key);
		SAFE_PTHREAD_CREATE(&pth[2], NULL, do_read,
			(void *)(unsigned long)key);
		SAFE_PTHREAD_CREATE(&pth[3], NULL, do_revoke,
			(void *)(unsigned long)key);

		SAFE_PTHREAD_JOIN(pth[0], NULL);
		SAFE_PTHREAD_JOIN(pth[1], NULL);
		SAFE_PTHREAD_JOIN(pth[2], NULL);
		SAFE_PTHREAD_JOIN(pth[3], NULL);

		if (!tst_remaining_runtime()) {
			tst_res(TINFO, "Runtime exhausted, exiting after %d loops", i);
			break;
		}

		/*
		 * Realtime kernel has deferred post-join thread cleanup which
		 * may result in exhaustion of cgroup thread limit. Add delay
		 * to limit the maximum number of stale threads to 4000
		 * even with CONFIG_HZ=100.
		 */
		if (realtime_kernel)
			usleep(100);
	}

	/*
	 * Kernel should start garbage collect when last reference to key
	 * is removed (see key_put()). Since we are adding keys with identical
	 * description and type, each replacement should schedule a gc run,
	 * see comment at __key_link().
	 *
	 * We create extra key here, to remove reference to last revoked key.
	 */
	key_inv = add_key("user", "ltptestkey", "foo", 3,
		KEY_SPEC_PROCESS_KEYRING);
	if (key_inv == -1)
		tst_brk(TBROK | TERRNO, "Failed to add key");

	/*
	 * If we have invalidate, we can drop extra key immediately as well,
	 * which also schedules gc.
	 */
	if (keyctl(KEYCTL_INVALIDATE, key_inv) == -1 && errno != EOPNOTSUPP)
		tst_brk(TBROK | TERRNO, "Failed to invalidate key");

	/*
	 * At this point we are quite confident that gc has been scheduled,
	 * so we wait and periodically check for last test key to be removed.
	 */
	for (i = 0; i < MAX_WAIT_FOR_GC_MS; i += 100) {
		ret = keyctl(KEYCTL_REVOKE, key);
		if (ret == -1 && errno == ENOKEY)
			break;
		usleep(100*1000);
	}

	if (i)
		tst_res(TINFO, "waiting for key gc took: %d ms", i);
	tst_res(TPASS, "Bug not reproduced");
}

static void setup(void)
{
	unsigned int i;
	struct tst_kconfig_var rt_kconfigs[] = {
		TST_KCONFIG_INIT("CONFIG_PREEMPT_RT"),
		TST_KCONFIG_INIT("CONFIG_PREEMPT_RT_FULL")
	};

	SAFE_FILE_SCANF(PATH_KEY_COUNT_QUOTA, "%d", &orig_maxkeys);
	SAFE_FILE_PRINTF(PATH_KEY_COUNT_QUOTA, "%d", orig_maxkeys + LOOPS + 1);

	tst_kconfig_read(rt_kconfigs, ARRAY_SIZE(rt_kconfigs));

	for (i = 0; i < ARRAY_SIZE(rt_kconfigs); i++)
		realtime_kernel |= rt_kconfigs[i].choice == 'y';
}

static void cleanup(void)
{
	if (orig_maxkeys > 0)
		SAFE_FILE_PRINTF(PATH_KEY_COUNT_QUOTA, "%d", orig_maxkeys);
}

static struct tst_test test = {
	.needs_root = 1,
	.setup = setup,
	.cleanup = cleanup,
	.runtime = 60,
	.test_all = do_test,
	.tags = (const struct tst_tag[]) {
		{"linux-git", "b4a1b4f5047e"},
		{"CVE", "2015-7550"},
		{}
	}
};
