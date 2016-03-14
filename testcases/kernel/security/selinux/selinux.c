#include <errno.h>
#include <fcntl.h>

#include "test.h"
#include <selinux/selinux.h>


char *TCID = "selinux";
int TST_TOTAL = 1;

static void cleanup(void);
static void setup(void);

static int old_enforcing;

static void setup(void)
{
	tst_require_root();
	tst_tmpdir();

	if (!is_selinux_enabled())
		tst_brkm(TBROK, cleanup, "selinux not enabled\n");

	old_enforcing = security_getenforce();

	int setenforce_failure = security_setenforce(1);

	if (setenforce_failure)
		tst_brkm(TBROK, cleanup,
			"unable to enforce selinux: %d", errno);

	TEST_PAUSE;
}

static void cleanup(void)
{
	tst_rmdir();
	security_setenforce(old_enforcing);
}

int main(void)
{
	setup();

	int fd;

	fd = creat("file", 0600);
	if (-1 == fd)
		tst_brkm(TBROK, cleanup, "could not create file: %d", errno);
	else
		printf("created file %d\n", fd);

	cleanup();
	tst_exit();
}
