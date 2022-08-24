// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *  Copyright (c) International Business Machines Corp., 2004
 *  Copyright (c) Linux Test Project, 2013-2016
 */

/*
 * This is a test case for madvise(2) system call.
 * It tests madvise(2) with combinations of advice values.
 * No error should be returned.
 */

#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/mount.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "tst_test.h"
#include "lapi/mmap.h"

#define TMP_DIR "tmp_madvise"
#define TEST_FILE TMP_DIR"/testfile"
#define KSM_SYS_DIR "/sys/kernel/mm/ksm"
#define STR "abcdefghijklmnopqrstuvwxyz12345\n"

static char *sfile;
static char *amem;
static struct stat st;

static struct tcase {
	int advice;
	char *name;
	char **addr;
} tcases[] = {
	{MADV_NORMAL,      "MADV_NORMAL",      &sfile},
	{MADV_RANDOM,      "MADV_RANDOM",      &sfile},
	{MADV_SEQUENTIAL,  "MADV_SEQUENTIAL",  &sfile},
	{MADV_WILLNEED,    "MADV_WILLNEED",    &sfile},
	{MADV_DONTNEED,    "MADV_DONTNEED",    &sfile},
	{MADV_REMOVE,      "MADV_REMOVE",      &sfile}, /* since Linux 2.6.16 */
	{MADV_DONTFORK,    "MADV_DONTFORK",    &sfile}, /* since Linux 2.6.16 */
	{MADV_DOFORK,      "MADV_DOFORK",      &sfile}, /* since Linux 2.6.16 */
	{MADV_HWPOISON,    "MADV_HWPOISON",    &sfile}, /* since Linux 2.6.32 */
	{MADV_MERGEABLE,   "MADV_MERGEABLE",   &sfile}, /* since Linux 2.6.32 */
	{MADV_UNMERGEABLE, "MADV_UNMERGEABLE", &sfile}, /* since Linux 2.6.32 */
	{MADV_HUGEPAGE,    "MADV_HUGEPAGE",    &amem},  /* since Linux 2.6.38 */
	{MADV_NOHUGEPAGE,  "MADV_NOHUGEPAGE",  &amem},  /* since Linux 2.6.38 */
	{MADV_DONTDUMP,    "MADV_DONTDUMP",    &sfile}, /* since Linux 3.4 */
	{MADV_DODUMP,      "MADV_DODUMP",      &sfile}, /* since Linux 3.4 */
	{MADV_FREE,        "MADV_FREE",        &amem},  /* since Linux 4.5 */
	{MADV_WIPEONFORK,  "MADV_WIPEONFORK",  &amem},  /* since Linux 4.14 */
	{MADV_KEEPONFORK,  "MADV_KEEPONFORK",  &amem},  /* since Linux 4.14 */
	{MADV_COLD,        "MADV_COLD",        &amem},  /* since Linux 5.4 */
	{MADV_PAGEOUT,     "MADV_PAGEOUT",     &amem},  /* since Linux 5.4 */

};

static void setup(void)
{
	unsigned int i;
	int fd;

	SAFE_MKDIR(TMP_DIR, 0664);
	SAFE_MOUNT(TMP_DIR, TMP_DIR, "tmpfs", 0, NULL);

	fd = SAFE_OPEN(TEST_FILE, O_RDWR | O_CREAT, 0664);

	/* Writing 40 KB of random data into this file [32 * 1280 = 40960] */
	for (i = 0; i < 1280; i++)
		SAFE_WRITE(SAFE_WRITE_ALL, fd, STR, strlen(STR));

	SAFE_FSTAT(fd, &st);

	/* Map the input file into shared memory */
	sfile = SAFE_MMAP(NULL, st.st_size,
			PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

	/* Map the input file into private memory. MADV_HUGEPAGE only works
	 * with private anonymous pages */
	amem = SAFE_MMAP(NULL, st.st_size,
			PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

	SAFE_CLOSE(fd);
}

static void cleanup(void)
{
	SAFE_MUNMAP(sfile, st.st_size);
	SAFE_MUNMAP(amem, st.st_size);
	SAFE_UMOUNT(TMP_DIR);
}

static void verify_madvise(unsigned int i)
{
	struct tcase *tc = &tcases[i];

	TEST(madvise(*(tc->addr), st.st_size, tc->advice));

	if (TST_RET == -1) {
		if (TST_ERR == EINVAL) {
			tst_res(TCONF, "%s is not supported", tc->name);
		} else {
			tst_res(TFAIL, "madvise test for %s failed with "
					"return = %ld, errno = %d : %s",
					tc->name, TST_RET, TST_ERR,
					tst_strerrno(TFAIL | TTERRNO));
		}
	} else {
		tst_res(TPASS, "madvise test for %s PASSED", tc->name);
	}
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(tcases),
	.test = verify_madvise,
	.needs_tmpdir = 1,
	.needs_root = 1,
	.setup = setup,
	.cleanup = cleanup,
};
