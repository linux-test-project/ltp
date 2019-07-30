/*
 *
 *   Copyright (c) Red Hat Inc., 2008
 *
 *   This program is free software;  you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY;  without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 *   the GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program;  if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/*
 * NAME
 *	signalfd01.c
 *
 * DESCRIPTION
 *	Check signalfd can receive signals
 *
 * USAGE
 * 	signalfd01
 *
 * HISTORY
 *	9/2008 Initial version by Masatake YAMATO <yamato@redhat.com>
 *
 * RESTRICTIONS
 * 	None
 */
#define _GNU_SOURCE

#include "config.h"

#include "test.h"

#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <inttypes.h>
#include "ltp_signal.h"

TCID_DEFINE(signalfd01);
int TST_TOTAL = 1;

#ifndef HAVE_SIGNALFD
#define  USE_STUB
#endif

#if defined HAVE_SYS_SIGNALFD_H
#include <sys/signalfd.h>
#elif defined HAVE_LINUX_SIGNALFD_H
#include <linux/signalfd.h>
#define USE_OWNIMPL
#else
#define  USE_STUB
#endif

#ifndef HAVE_STRUCT_SIGNALFD_SIGINFO_SSI_SIGNO
#define USE_STUB
#endif

#ifdef USE_STUB
int main(void)
{
	tst_brkm(TCONF, NULL, "System doesn't support execution of the test");
}

#else
#if defined USE_OWNIMPL
#include "lapi/syscalls.h"
int signalfd(int fd, const sigset_t * mask, int flags)
{
	/* Taken from GLIBC. */
	return ltp_syscall(__NR_signalfd, fd, mask, SIGSETSIZE);
}
#endif

void cleanup(void);
void setup(void);

int do_test1(uint32_t sig)
{
	int sfd_for_next;
	int sfd;
	sigset_t mask;
	pid_t pid;
	struct signalfd_siginfo fdsi;
	ssize_t s;

	sigemptyset(&mask);
	sigaddset(&mask, sig);
	if (sigprocmask(SIG_BLOCK, &mask, NULL) < 0) {
		tst_brkm(TBROK, cleanup,
			 "sigprocmask() Failed: errno=%d : %s",
			 errno, strerror(errno));
	}

	TEST(signalfd(-1, &mask, 0));

	if ((sfd = TEST_RETURN) == -1) {
		tst_resm(TFAIL,
			 "signalfd() Failed, errno=%d : %s",
			 TEST_ERRNO, strerror(TEST_ERRNO));
		sfd_for_next = -1;
		return sfd_for_next;

	} else {
		tst_resm(TPASS, "signalfd is created successfully");
		sfd_for_next = sfd;
		goto out;
	}

	if (fcntl(sfd, F_SETFL, O_NONBLOCK) == -1) {
		close(sfd);
		tst_brkm(TBROK, cleanup,
			 "setting signalfd nonblocking mode failed: errno=%d : %s",
			 errno, strerror(errno));
	}

	pid = getpid();
	if (kill(pid, sig) == -1) {
		close(sfd);
		tst_brkm(TBROK, cleanup,
			 "kill(self, %s) failed: errno=%d : %s",
			 strsignal(sig), errno, strerror(errno));
	}

	s = read(sfd, &fdsi, sizeof(struct signalfd_siginfo));
	if ((s > 0) && (s != sizeof(struct signalfd_siginfo))) {
		tst_resm(TFAIL,
			 "getting incomplete signalfd_siginfo data: "
			 "actual-size=%zd, expected-size=%zu",
			 s, sizeof(struct signalfd_siginfo));
		sfd_for_next = -1;
		close(sfd);
		goto out;
	} else if (s < 0) {
		if (errno == EAGAIN) {
			tst_resm(TFAIL,
				 "signalfd_siginfo data is not delivered yet");
			sfd_for_next = -1;
			close(sfd);
			goto out;
		} else {
			close(sfd);
			tst_brkm(TBROK, cleanup,
				 "read signalfd_siginfo data failed: errno=%d : %s",
				 errno, strerror(errno));
		}
	} else if (s == 0) {
		tst_resm(TFAIL, "got EOF unexpectedly");
		sfd_for_next = -1;
		close(sfd);
		goto out;
	}

	if (fdsi.ssi_signo == sig) {
		tst_resm(TPASS, "got expected signal");
		sfd_for_next = sfd;
		goto out;
	} else {
		tst_resm(TFAIL, "got unexpected signal: signal=%d : %s",
			 fdsi.ssi_signo, strsignal(fdsi.ssi_signo));
		sfd_for_next = -1;
		close(sfd);
		goto out;
	}

out:
	return sfd_for_next;
}

void do_test2(int fd, uint32_t sig)
{
	int sfd;
	sigset_t mask;
	pid_t pid;
	struct signalfd_siginfo fdsi;
	ssize_t s;

	sigemptyset(&mask);
	sigaddset(&mask, sig);
	if (sigprocmask(SIG_BLOCK, &mask, NULL) < 0) {
		close(fd);
		tst_brkm(TBROK, cleanup,
			 "sigprocmask() Failed: errno=%d : %s",
			 errno, strerror(errno));
	}

	TEST(signalfd(fd, &mask, 0));

	if ((sfd = TEST_RETURN) == -1) {
		tst_resm(TFAIL,
			 "reassignment the file descriptor by signalfd() failed, errno=%d : %s",
			 TEST_ERRNO, strerror(TEST_ERRNO));
		return;
	} else if (sfd != fd) {
		tst_resm(TFAIL,
			 "different fd is returned in reassignment: expected-fd=%d, actual-fd=%d",
			 fd, sfd);
		close(sfd);
		return;

	} else {
		tst_resm(TPASS, "signalfd is successfully reassigned");
		goto out;
	}

	pid = getpid();
	if (kill(pid, sig) == -1) {
		close(sfd);
		tst_brkm(TBROK, cleanup,
			 "kill(self, %s) failed: errno=%d : %s",
			 strsignal(sig), errno, strerror(errno));
	}

	s = read(sfd, &fdsi, sizeof(struct signalfd_siginfo));
	if ((s > 0) && (s != sizeof(struct signalfd_siginfo))) {
		tst_resm(TFAIL,
			 "getting incomplete signalfd_siginfo data: "
			 "actual-size=%zd, expected-size= %zu",
			 s, sizeof(struct signalfd_siginfo));
		goto out;
	} else if (s < 0) {
		if (errno == EAGAIN) {
			tst_resm(TFAIL,
				 "signalfd_siginfo data is not delivered yet");
			goto out;
		} else {
			close(sfd);
			tst_brkm(TBROK, cleanup,
				 "read signalfd_siginfo data failed: errno=%d : %s",
				 errno, strerror(errno));
		}
	} else if (s == 0) {
		tst_resm(TFAIL, "got EOF unexpectedly");
		goto out;
	}

	if (fdsi.ssi_signo == sig) {
		tst_resm(TPASS, "got expected signal");
		goto out;
	} else {
		tst_resm(TFAIL, "got unexpected signal: signal=%d : %s",
			 fdsi.ssi_signo),
			 strsignal(fdsi.ssi_signo);
		goto out;
	}

out:
	return;
}

int main(int argc, char **argv)
{
	int lc;
	int sfd;

	if ((tst_kvercmp(2, 6, 22)) < 0) {
		tst_resm(TWARN,
			 "This test can only run on kernels that are 2.6.22 and higher");
		exit(0);
	}

	tst_parse_opts(argc, argv, NULL, NULL);

	setup();
	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		sfd = do_test1(SIGUSR1);
		if (sfd < 0)
			continue;

		do_test2(sfd, SIGUSR2);
		close(sfd);
	}

	cleanup();

	tst_exit();
}

/*
 * setup() - performs all the ONE TIME setup for this test.
 */
void setup(void)
{

	TEST_PAUSE;
}

/*
 * cleanup() - performs all the ONE TIME cleanup for this test at completion
 * 	       or premature exit.
 */
void cleanup(void)
{

}

#endif
