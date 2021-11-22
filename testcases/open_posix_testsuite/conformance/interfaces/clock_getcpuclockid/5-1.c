/*
 * Copyright (c) 2010, Ngie Cooper.
 *
 * The clock_getcpuclockid() function shall fail and return EPERM if the
 * requesting process does not have permission to access the CPU-time clock for
 * the process.
 */


#include <sys/types.h>
#include <errno.h>
#include <limits.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include "posixtest.h"

int main(void)
{
#if !defined(_POSIX_CPUTIME) || _POSIX_CPUTIME == -1
	printf("_POSIX_CPUTIME unsupported\n");
	return PTS_UNSUPPORTED;
#else
	clockid_t clockid_1;
	int error;

	/*
	 * Do a best effort at trying to get root demoted to "nobody" for the
	 * duration of the test.
	 */
	if (getuid() == 0) {
		struct passwd *pwd;

		pwd = getpwnam("nobody");
		if (pwd != NULL) {
			if (setgid(pwd->pw_gid)) {
				perror("setgid");
				return PTS_UNRESOLVED;
			}
			if (setuid(pwd->pw_uid)) {
				perror("setuid");
				return PTS_UNRESOLVED;
			}
		}
	}

	/* Hmmm -- above steps failed :(... */
	if (getuid() == 0) {
		printf("Test must be run as non-root user\n");
		return PTS_UNRESOLVED;
	}

	/* Try and get the cpu clock ID for init(1) :)... */
	error = clock_getcpuclockid(1, &clockid_1);
	if (error == 0) {
		printf("clock_getcpuclockid(1, ..) passed\n");
		return PTS_UNTESTED;
	} else if (error != EPERM) {
		printf("clock_getcpuclockid(1, ..) failed with an improper "
		       "error (%d != %d)\n", EPERM, error);
		return PTS_UNRESOLVED;
	}
	printf("Test PASSED\n");
	return PTS_PASS;
#endif
}
