// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2025 Petr Vorel <pvorel@suse.cz>
 */

/* update also tst_test.sh */
#define LTP_USR_UID 65534
#define LTP_USR_GID 65534

#define TST_NO_DEFAULT_MAIN
#include "tst_test.h"

static void print_help(void)
{
	fprintf(stderr, "Usage: %s cmd [args] ...\n", __FILE__);
	fprintf(stderr, "Usage: %s cmd [-h] print help\n\n", __FILE__);

	fprintf(stderr, "Environment Variables\n");
	fprintf(stderr, "LTP_USR_UID: UID of 'nobody' user, defaults %d\n",
			LTP_USR_UID);
	fprintf(stderr, "LTP_USR_GID: GID of 'nobody' user, defaults %d\n",
			LTP_USR_GID);
}

int main(int argc, char *argv[])
{
	if (argc < 2 || !strcmp(argv[1], "-h")) {
		print_help();
		return 1;
	}

	unsigned uid = LTP_USR_UID, gid = LTP_USR_GID;

	char *uid_env = getenv("LTP_USR_UID");
	char *gid_env = getenv("LTP_USR_GID");

	if (uid_env)
		uid = SAFE_STRTOL(uid_env, 1, INT_MAX);

	if (gid_env)
		gid = SAFE_STRTOL(gid_env, 1, INT_MAX);

	tst_res(TINFO, "UID: %d, GID: %d", uid, gid);
	SAFE_SETGROUPS(0, NULL);
	SAFE_SETRESGID(gid, gid, gid);
	SAFE_SETRESUID(uid, uid, uid);

	SAFE_CMD((const char * const *)&argv[1], NULL, NULL);

	return 0;
}
