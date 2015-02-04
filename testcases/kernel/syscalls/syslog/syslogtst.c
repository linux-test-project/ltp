/*
 *
 *   Copyright (c) International Business Machines  Corp., 2002
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

/* 06/30/2001	Port to Linux	nsharoff@us.ibm.com */
/* 11/22/2002	Port to Linux	dbarrera@us.ibm.com */

#include <sys/types.h>
#include <assert.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <syslog.h>
#include <time.h>
#include <unistd.h>
#include "test.h"

char *TCID = "syslogtst";
int TST_TOTAL = 1;

void sig_handler(int signal);

int main(int argc, char *argv[])
{
	int status, flag3, fd, ch, ch1;
	int exit_flag = 0;	/* used for syslog test case 6. */
	time_t t;

	ch1 = -1;

	signal(SIGINT, sig_handler);
	signal(SIGTERM, sig_handler);
	signal(SIGHUP, sig_handler);
	signal(SIGABRT, sig_handler);
	signal(SIGSEGV, sig_handler);
	signal(SIGQUIT, sig_handler);

	time(&t);
	srandom((unsigned int)getpid() ^
		(((unsigned int)t << 16) | (unsigned int)t >> 16));

	if (argc < 2) {
		ch = (random() % 10) + 1;
		if (ch == 2)
			ch1 = random() % 8;
		if (ch == 8)
			ch1 = (random() % 5) + 1;
		tst_resm(TINFO,
			 "\nrandom numbers were generated for the case numbers : %d, %d\n",
			 ch, ch1);
	}

	else if (argc == 2) {
		ch = atoi(argv[1]);
		if (ch == 2 || ch == 8) {
			if (ch == 2)
				ch1 = random() % 8;
			if (ch == 8)
				ch1 = (random() % 5) + 1;
			tst_resm(TINFO,
				 "\nrandom number was generated for case %d : %d\n",
				 ch, ch1);
		}
	}

	else {
		ch = atoi(argv[1]);
		if (argc > 2)
			ch1 = atoi(argv[2]);
	}

	/* Ensure ch1 is properly allocated when ch == 2 or ch == 8. */
	assert(!((ch == 2 || ch == 8) && ch1 == -1));

	/*
	 * Send syslog messages according to the case number, which
	 * we will know from command line.
	 */
	switch (ch) {
	case 1:
		syslog(LOG_MAIL | LOG_INFO, "syslogtst: mail info test.");
		break;
	case 2:
		switch (ch1) {
		case 0:
			syslog(LOG_MAIL | LOG_EMERG,
			       "syslogtst: mail emerg test.");
			break;
		case 1:
			syslog(LOG_MAIL | LOG_ALERT,
			       "syslogtst: mail alert test.");
			break;
		case 2:
			syslog(LOG_MAIL | LOG_CRIT,
			       "syslogtst: mail crit test.");
			break;
		case 3:
			syslog(LOG_MAIL | LOG_ERR, "syslogtst: mail err test.");
			break;
		case 4:
			syslog(LOG_MAIL | LOG_WARNING,
			       "syslogtst: mail warning test.");
			break;
		case 5:
			syslog(LOG_MAIL | LOG_NOTICE,
			       "syslogtst: mail notice test.");
			break;
		case 6:
			syslog(LOG_MAIL | LOG_INFO,
			       "syslogtst: mail info test.");
			break;
		case 7:
			syslog(LOG_MAIL | LOG_DEBUG,
			       "syslogtst: mail debug test.");
			break;

		}
		break;
	case 3:
		openlog("SYSLOG_CASE3", LOG_PID, LOG_DAEMON);
		syslog(LOG_DAEMON | LOG_INFO, "syslogtst: daemon info test.");
		closelog();
		break;
	case 4:
		openlog("log_pid_test", LOG_PID, LOG_USER);
		syslog(LOG_USER | LOG_INFO, "syslogtst: user info test.");
		closelog();
		break;
	case 5:
		openlog("log_cons_test", LOG_CONS, LOG_USER);

		/*
		 * Move the /dev/syslog to /dev/syslog.tmp
		 * This way we are forcing syslog to write messages to
		 * console.
		 */
#ifdef DEBUG2
		status =
		    system
		    ("/bin/mv -f /var/log/messages /var/log/messages.tmp");
#else
		status = 0;
#endif
		if (status == 0) {
#ifdef DEBUG
			tst_resm(TINFO,
				 "/var/log/messages is moved to /var/log/messages.tmp...");
#endif
			flag3 = 1;
		} else {
			tst_brkm(TFAIL,
				 NULL,
				 "Cannot move /var/log/messages. Setup failed...exiting...");
		}
		sleep(10);

		syslog(LOG_USER | LOG_INFO, "syslogtst: info to console test.");

		sleep(10);
		/*
		 * Restore /dev/syslog file.
		 */
		if (flag3 == 1) {
#ifdef DEBUG2
			status =
			    system
			    ("/bin/mv -f /var/log/messages.tmp /var/log/messages");
#else
			status = 0;
#endif
			if (status != 0) {
				tst_brkm(TFAIL,
					 NULL,
				         "Restoring /var/log/messages failed...");
			}
#ifdef DEBUG
			else
				tst_resm(TINFO, "/var/log/messages restored..");
#endif
		}
		closelog();
		break;
	case 6:
		openlog("without log_ndelay", LOG_PID, LOG_USER);
		fd = open("/dev/null", O_RDONLY);
#ifdef DEBUG
		tst_resm(TINFO, "openlog() without LOG_NDELAY option...");
#endif
		if (fd >= 3) {
#ifdef DEBUG
			tst_resm(TINFO,
				 "open() has returned the expected fd: %d", fd);
#endif
		} else {
			tst_resm(TFAIL, "open() has returned unexpected fd: %d",
				 fd);
			exit_flag = 1;
			close(fd);
			closelog();
			break;
		}
		close(fd);
		closelog();

		openlog("with log_ndelay", LOG_NDELAY, LOG_USER);
		fd = open("/dev/null", O_RDONLY);
#ifdef DEBUG
		tst_resm(TINFO, "openlog() with LOG_NDELAY option...");
#endif
		if (fd <= 3) {
			tst_resm(TFAIL, "open() returned unexpected fd: %d",
				 fd);
			exit_flag = 1;
			close(fd);
			closelog();
			break;
		}
#ifdef DEBUG
		else
			tst_resm(TINFO, "open() has returned expected fd: %d",
				 fd);
#endif
		close(fd);
		closelog();
		break;
	case 7:
		syslog(LOG_USER | LOG_EMERG, "syslogtst: emergency log");
		syslog(LOG_USER | LOG_ALERT, "syslogtst: alert log");
		syslog(LOG_USER | LOG_CRIT, "syslogtst: critical log");
		syslog(LOG_USER | LOG_ERR, "syslogtst: error log");
		syslog(LOG_USER | LOG_WARNING, "syslogtst: warning log");
		syslog(LOG_USER | LOG_NOTICE, "syslogtst: notice log");
		syslog(LOG_USER | LOG_INFO, "syslogtst: info log");
		syslog(LOG_USER | LOG_DEBUG, "syslogtst: debug log");
		break;
	case 8:
		switch (ch1) {
			/*
			 * Kernel messages cannot be send by user, so skipping the
			 * LOG_KERN facility.
			 */
		case 1:
			syslog(LOG_USER | LOG_INFO,
			       "syslogtst: user info test.");
			break;
		case 2:
			syslog(LOG_MAIL | LOG_INFO,
			       "syslogtst: mail info test.");
			break;
		case 3:
			syslog(LOG_DAEMON | LOG_INFO,
			       "syslogtst: daemon info test.");
			break;
		case 4:
			syslog(LOG_AUTH | LOG_INFO,
			       "syslogtst: auth info test.");
			break;
		case 5:
			syslog(LOG_LPR | LOG_INFO, "syslogtst: lpr info test.");
			break;
		}
		break;
	case 9:
		setlogmask(LOG_UPTO(LOG_ERR));
		syslog(LOG_USER | LOG_ERR, "syslogtst: error level is logged");
		syslog(LOG_USER | LOG_WARNING,
		       "syslogtst: warning level not to be logged");
		break;
	case 10:
		setlogmask(LOG_MASK(LOG_ERR));
		syslog(LOG_USER | LOG_ERR,
		       "syslogtst:10 error level is logged");
		syslog(LOG_USER | LOG_WARNING,
		       "syslogtst:10 warning level not to be logged");
		break;
	}

	/*
	 * Check the exit_flag and if it is set,
	 * exit with status 1, indicating failure.
	 */
	if (exit_flag == 1)
		exit(1);
	else
		exit(0);

}

void sig_handler(int signal)
{

	switch (signal) {
	case SIGINT:
#ifdef DEBUG
		tst_resm(TINFO, "SIGINT is received.");
#endif
		break;
	case SIGTERM:
#ifdef DEBUG
		tst_resm(TINFO, "SIGTERM is received.");
#endif
		break;
	case SIGHUP:
#ifdef DEBUG
		tst_resm(TINFO, "SIGHUP is received.");
#endif
		break;
	case SIGABRT:
#ifdef DEBUG
		tst_resm(TINFO, "SIGABRT is received.");
#endif
		break;
	case SIGSEGV:
#ifdef DEBUG
		tst_resm(TINFO, "SIGSEGV is received.");
#endif
		break;
	case SIGQUIT:
#ifdef DEBUG
		tst_resm(TINFO, "SIGQUIT is received.");
#endif
		break;
	}

	exit(signal);
}
