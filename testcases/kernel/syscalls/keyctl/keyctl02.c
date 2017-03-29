/*
 * Copyright (c) 2017 Fujitsu Ltd.
 *  Ported: Guangwen Feng <fenggw-fnst@cn.fujitsu.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program, if not, see <http://www.gnu.org/licenses/>.
 */

/*
 * This is a regression test for the race between keyctl_read() and
 * keyctl_revoke(), if the revoke happens between keyctl_read()
 * checking the validity of a key and the key's semaphore being taken,
 * then the key type read method will see a revoked key.
 *
 * This causes a problem for the user-defined key type because it
 * assumes in its read method that there will always be a payload
 * in a non-revoked key and doesn't check for a NULL pointer.
 *
 * This test can crash the buggy kernel, and the bug was fixed in:
 *
 *  commit b4a1b4f5047e4f54e194681125c74c0aa64d637d
 *  Author: David Howells <dhowells@redhat.com>
 *  Date:   Fri Dec 18 01:34:26 2015 +0000
 *
 *  KEYS: Fix race between read and revoke
 */

#include "config.h"
#include <errno.h>
#include <pthread.h>
#include <sys/types.h>
#ifdef HAVE_KEYUTILS_H
# include <keyutils.h>
#endif
#include "tst_safe_pthread.h"
#include "tst_test.h"

#ifdef HAVE_KEYUTILS_H

#define LOOPS	20000
#define PATH_KEY_COUNT_QUOTA	"/proc/sys/kernel/keys/root_maxkeys"

static int orig_maxkeys;

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
	int i;
	key_serial_t key;
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
	}

	tst_res(TPASS, "Bug not reproduced");
}

static void setup(void)
{
	SAFE_FILE_SCANF(PATH_KEY_COUNT_QUOTA, "%d", &orig_maxkeys);
	SAFE_FILE_PRINTF(PATH_KEY_COUNT_QUOTA, "%d", orig_maxkeys + LOOPS);
}

static void cleanup(void)
{
	if (orig_maxkeys > 0)
		SAFE_FILE_PRINTF(PATH_KEY_COUNT_QUOTA, "%d", orig_maxkeys);
}

static struct tst_test test = {
	.tid = "keyctl02",
	.needs_root = 1,
	.setup = setup,
	.cleanup = cleanup,
	.test_all = do_test,
};

#else
	TST_TEST_TCONF("keyutils.h does not exist");
#endif /* HAVE_KEYUTILS_H */
