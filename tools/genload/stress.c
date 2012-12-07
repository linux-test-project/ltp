/* A program to put stress on a POSIX system (stress).
 *
 * Copyright (C) 2001, 2002 Amos Waterland <awaterl@yahoo.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <ctype.h>
#include <errno.h>
#include <libgen.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>

/* By default, print all messages of severity info and above.  */
static int global_debug = 2;

/* By default, just print warning for non-critical errors.  */
static int global_ignore = 1;

/* By default, retry on non-critical errors every 50ms.  */
static int global_retry = 50000;

/* By default, use this as backoff coefficient for good fork throughput.  */
static int global_backoff = 3000;

/* By default, do not timeout.  */
static int global_timeout = 0;

/* Name of this program */
static char *global_progname = PACKAGE;

/* By default, do not hang after allocating memory.  */
static int global_vmhang = 0;

/* Implemention of runtime-selectable severity message printing.  */
#define dbg if (global_debug >= 3) \
            fprintf (stdout, "%s: debug: (%d) ", global_progname, __LINE__), \
            fprintf
#define out if (global_debug >= 2) \
            fprintf (stdout, "%s: info: ", global_progname), \
            fprintf
#define wrn if (global_debug >= 1) \
            fprintf (stderr, "%s: warn: (%d) ", global_progname, __LINE__), \
            fprintf
#define err if (global_debug >= 0) \
            fprintf (stderr, "%s: error: (%d) ", global_progname, __LINE__), \
            fprintf

/* Implementation of check for option argument correctness.  */
#define assert_arg(A) \
          if (++i == argc || ((arg = argv[i])[0] == '-' && \
              !isdigit ((int)arg[1]) )) \
            { \
              err (stderr, "missing argument to option '%s'\n", A); \
              exit (1); \
            }

/* Prototypes for utility functions.  */
int usage(int status);
int version(int status);
long long atoll_s(const char *nptr);
long long atoll_b(const char *nptr);

/* Prototypes for the worker functions.  */
int hogcpu(long long forks);
int hogio(long long forks);
int hogvm(long long forks, long long chunks, long long bytes);
int hoghdd(long long forks, int clean, long long files, long long bytes);

int main(int argc, char **argv)
{
	int i, pid, children = 0, retval = 0;
	long starttime, stoptime, runtime;

	/* Variables that indicate which options have been selected.  */
	int do_dryrun = 0;
	int do_timeout = 0;
	int do_cpu = 0;		/* Default to 1 fork. */
	long long do_cpu_forks = 1;
	int do_io = 0;		/* Default to 1 fork. */
	long long do_io_forks = 1;
	int do_vm = 0;		/* Default to 1 fork, 1 chunk of 256MB.  */
	long long do_vm_forks = 1;
	long long do_vm_chunks = 1;
	long long do_vm_bytes = 256 * 1024 * 1024;
	int do_hdd = 0;		/* Default to 1 fork, clean, 1 file of 1GB.  */
	long long do_hdd_forks = 1;
	int do_hdd_clean = 0;
	long long do_hdd_files = 1;
	long long do_hdd_bytes = 1024 * 1024 * 1024;

	/* Record our start time.  */
	if ((starttime = time(NULL)) == -1) {
		err(stderr, "failed to acquire current time\n");
		exit(1);
	}

	/* SuSv3 does not define any error conditions for this function.  */
	global_progname = basename(argv[0]);

	/* For portability, parse command line options without getopt_long.  */
	for (i = 1; i < argc; i++) {
		char *arg = argv[i];

		if (strcmp(arg, "--help") == 0 || strcmp(arg, "-?") == 0) {
			usage(0);
		} else if (strcmp(arg, "--version") == 0) {
			version(0);
		} else if (strcmp(arg, "--verbose") == 0
			   || strcmp(arg, "-v") == 0) {
			global_debug = 3;
		} else if (strcmp(arg, "--quiet") == 0
			   || strcmp(arg, "-q") == 0) {
			global_debug = 0;
		} else if (strcmp(arg, "--dry-run") == 0
			   || strcmp(arg, "-n") == 0) {
			do_dryrun = 1;
		} else if (strcmp(arg, "--no-retry") == 0) {
			global_ignore = 0;
			dbg(stdout,
			    "turning off ignore of non-critical errors");
		} else if (strcmp(arg, "--retry-delay") == 0) {
			assert_arg("--retry-delay");
			global_retry = atoll(arg);
			dbg(stdout, "setting retry delay to %dus\n",
			    global_retry);
		} else if (strcmp(arg, "--backoff") == 0) {
			assert_arg("--backoff");
			global_backoff = atoll(arg);
			if (global_backoff < 0) {
				err(stderr, "invalid backoff factor: %i\n",
				    global_backoff);
				exit(1);
			}
			dbg(stdout, "setting backoff coeffient to %dus\n",
			    global_backoff);
		} else if (strcmp(arg, "--timeout") == 0
			   || strcmp(arg, "-t") == 0) {
			do_timeout = 1;
			assert_arg("--timeout");
			global_timeout = atoll_s(arg);
			dbg(stdout, "setting timeout to %ds\n", global_timeout);
		} else if (strcmp(arg, "--cpu") == 0 || strcmp(arg, "-c") == 0) {
			do_cpu = 1;
			assert_arg("--cpu");
			do_cpu_forks = atoll_b(arg);
		} else if (strcmp(arg, "--io") == 0 || strcmp(arg, "-i") == 0) {
			do_io = 1;
			assert_arg("--io");
			do_io_forks = atoll_b(arg);
		} else if (strcmp(arg, "--vm") == 0 || strcmp(arg, "-m") == 0) {
			do_vm = 1;
			assert_arg("--vm");
			do_vm_forks = atoll_b(arg);
		} else if (strcmp(arg, "--vm-chunks") == 0) {
			assert_arg("--vm-chunks");
			do_vm_chunks = atoll_b(arg);
		} else if (strcmp(arg, "--vm-bytes") == 0) {
			assert_arg("--vm-bytes");
			do_vm_bytes = atoll_b(arg);
		} else if (strcmp(arg, "--vm-hang") == 0) {
			global_vmhang = 1;
		} else if (strcmp(arg, "--hdd") == 0 || strcmp(arg, "-d") == 0) {
			do_hdd = 1;
			assert_arg("--hdd");
			do_hdd_forks = atoll_b(arg);
		} else if (strcmp(arg, "--hdd-noclean") == 0) {
			do_hdd_clean = 2;
		} else if (strcmp(arg, "--hdd-files") == 0) {
			assert_arg("--hdd-files");
			do_hdd_files = atoll_b(arg);
		} else if (strcmp(arg, "--hdd-bytes") == 0) {
			assert_arg("--hdd-bytes");
			do_hdd_bytes = atoll_b(arg);
		} else {
			err(stderr, "unrecognized option: %s\n", arg);
			exit(1);
		}
	}

	/* Hog CPU option.  */
	if (do_cpu) {
		out(stdout, "dispatching %lli hogcpu forks\n", do_cpu_forks);

		switch (pid = fork()) {
		case 0:	/* child */
			if (do_dryrun)
				exit(0);
			exit(hogcpu(do_cpu_forks));
		case -1:	/* error */
			err(stderr, "hogcpu dispatcher fork failed\n");
			exit(1);
		default:	/* parent */
			children++;
			dbg(stdout, "--> hogcpu dispatcher forked (%i)\n", pid);
		}
	}

	/* Hog I/O option.  */
	if (do_io) {
		out(stdout, "dispatching %lli hogio forks\n", do_io_forks);

		switch (pid = fork()) {
		case 0:	/* child */
			if (do_dryrun)
				exit(0);
			exit(hogio(do_io_forks));
		case -1:	/* error */
			err(stderr, "hogio dispatcher fork failed\n");
			exit(1);
		default:	/* parent */
			children++;
			dbg(stdout, "--> hogio dispatcher forked (%i)\n", pid);
		}
	}

	/* Hog VM option.  */
	if (do_vm) {
		out(stdout,
		    "dispatching %lli hogvm forks, each %lli chunks of %lli bytes\n",
		    do_vm_forks, do_vm_chunks, do_vm_bytes);

		switch (pid = fork()) {
		case 0:	/* child */
			if (do_dryrun)
				exit(0);
			exit(hogvm(do_vm_forks, do_vm_chunks, do_vm_bytes));
		case -1:	/* error */
			err(stderr, "hogvm dispatcher fork failed\n");
			exit(1);
		default:	/* parent */
			children++;
			dbg(stdout, "--> hogvm dispatcher forked (%i)\n", pid);
		}
	}

	/* Hog HDD option.  */
	if (do_hdd) {
		out(stdout, "dispatching %lli hoghdd forks, each %lli files of "
		    "%lli bytes\n", do_hdd_forks, do_hdd_files, do_hdd_bytes);

		switch (pid = fork()) {
		case 0:	/* child */
			if (do_dryrun)
				exit(0);
			exit(hoghdd
			     (do_hdd_forks, do_hdd_clean, do_hdd_files,
			      do_hdd_bytes));
		case -1:	/* error */
			err(stderr, "hoghdd dispatcher fork failed\n");
			exit(1);
		default:	/* parent */
			children++;
			dbg(stdout, "--> hoghdd dispatcher forked (%i)\n", pid);
		}
	}

	/* We have no work to do, so bail out.  */
	if (children == 0)
		usage(0);

	/* Wait for our children to exit.  */
	while (children) {
		int status, ret;

		if ((pid = wait(&status)) > 0) {
			if ((WIFEXITED(status)) != 0) {
				if ((ret = WEXITSTATUS(status)) != 0) {
					err(stderr,
					    "dispatcher %i returned error %i\n",
					    pid, ret);
					retval += ret;
				} else {
					dbg(stdout,
					    "<-- dispatcher return (%i)\n",
					    pid);
				}
			} else {
				err(stderr,
				    "dispatcher did not exit normally\n");
				++retval;
			}

			--children;
		} else {
			dbg(stdout, "wait() returned error: %s\n",
			    strerror(errno));
			err(stderr, "detected missing dispatcher children\n");
			++retval;
			break;
		}
	}

	/* Record our stop time.  */
	if ((stoptime = time(NULL)) == -1) {
		err(stderr, "failed to acquire current time\n");
		exit(1);
	}

	/* Calculate our runtime.  */
	runtime = stoptime - starttime;

	/* Print final status message.  */
	if (retval) {
		err(stderr, "failed run completed in %lis\n", runtime);
	} else {
		out(stdout, "successful run completed in %lis\n", runtime);
	}

	exit(retval);
}

int usage(int status)
{
	char *mesg =
	    "`%s' imposes certain types of compute stress on your system\n\n"
	    "Usage: %s [OPTION [ARG]] ...\n\n"
	    " -?, --help            show this help statement\n"
	    "     --version         show version statement\n"
	    " -v, --verbose         be verbose\n"
	    " -q, --quiet           be quiet\n"
	    " -n, --dry-run         show what would have been done\n"
	    "     --no-retry        exit rather than retry non-critical errors\n"
	    "     --retry-delay n   wait n us before continuing past error\n"
	    " -t, --timeout n       timeout after n seconds\n"
	    "     --backoff n       wait for factor of n us before starting work\n"
	    " -c, --cpu n           spawn n procs spinning on sqrt()\n"
	    " -i, --io n            spawn n procs spinning on sync()\n"
	    " -m, --vm n            spawn n procs spinning on malloc()\n"
	    "     --vm-chunks c     malloc c chunks (default is 1)\n"
	    "     --vm-bytes b      malloc chunks of b bytes (default is 256MB)\n"
	    "     --vm-hang         hang in a sleep loop after memory allocated\n"
	    " -d, --hdd n           spawn n procs spinning on write()\n"
	    "     --hdd-noclean     do not unlink file to which random data written\n"
	    "     --hdd-files f     write to f files (default is 1)\n"
	    "     --hdd-bytes b     write b bytes (default is 1GB)\n\n"
	    "Infinity is denoted with 0.  For -m, -d: n=0 means infinite redo,\n"
	    "n<0 means redo abs(n) times. Valid suffixes are m,h,d,y for time;\n"
	    "k,m,g for size.\n\n";

	fprintf(stdout, mesg, global_progname, global_progname);

	if (status <= 0)
		exit(-1 * status);

	return 0;
}

int version(int status)
{
	char *mesg = "%s %s\n";

	fprintf(stdout, mesg, global_progname, VERSION);

	if (status <= 0)
		exit(-1 * status);

	return 0;
}

/* Convert a string representation of a number with an optional size suffix
 * to a long long.
 */
long long atoll_b(const char *nptr)
{
	int pos;
	char suffix;
	long long factor = 1;

	if ((pos = strlen(nptr) - 1) < 0) {
		err(stderr, "invalid string\n");
		exit(1);
	}

	switch (suffix = nptr[pos]) {
	case 'k':
	case 'K':
		factor = 1024;
		break;
	case 'm':
	case 'M':
		factor = 1024 * 1024;
		break;
	case 'g':
	case 'G':
		factor = 1024 * 1024 * 1024;
		break;
	default:
		if (suffix < '0' || suffix > '9') {
			err(stderr, "unrecognized suffix: %c\n", suffix);
			exit(1);
		}
	}

	factor = atoll(nptr) * factor;

	return factor;
}

/* Convert a string representation of a number with an optional time suffix
 * to a long long.
 */
long long atoll_s(const char *nptr)
{
	int pos;
	char suffix;
	long long factor = 1;

	if ((pos = strlen(nptr) - 1) < 0) {
		err(stderr, "invalid string\n");
		exit(1);
	}

	switch (suffix = nptr[pos]) {
	case 's':
	case 'S':
		factor = 1;
		break;
	case 'm':
	case 'M':
		factor = 60;
		break;
	case 'h':
	case 'H':
		factor = 60 * 60;
		break;
	case 'd':
	case 'D':
		factor = 60 * 60 * 24;
		break;
	case 'y':
	case 'Y':
		factor = 60 * 60 * 24 * 360;
		break;
	default:
		if (suffix < '0' || suffix > '9') {
			err(stderr, "unrecognized suffix: %c\n", suffix);
			exit(1);
		}
	}

	factor = atoll(nptr) * factor;

	return factor;
}

int hogcpu(long long forks)
{
	long long i;
	double d;
	int pid, retval = 0;

	/* Make local copies of global variables.  */
	int ignore = global_ignore;
	int retry = global_retry;
	int timeout = global_timeout;
	long backoff = global_backoff * forks;

	dbg(stdout, "using backoff sleep of %lius for hogcpu\n", backoff);

	for (i = 0; forks == 0 || i < forks; i++) {
		switch (pid = fork()) {
		case 0:	/* child */
			alarm(timeout);

			/* Use a backoff sleep to ensure we get good fork throughput.  */
			usleep(backoff);

			while (1)
				d = sqrt(rand());

			/* This case never falls through; alarm signal can cause exit.  */
		case -1:	/* error */
			if (ignore) {
				++retval;
				wrn(stderr,
				    "hogcpu worker fork failed, continuing\n");
				usleep(retry);
				continue;
			}

			err(stderr, "hogcpu worker fork failed\n");
			return 1;
		default:	/* parent */
			dbg(stdout, "--> hogcpu worker forked (%i)\n", pid);
		}
	}

	/* Wait for our children to exit.  */
	while (i) {
		int status, ret;

		if ((pid = wait(&status)) > 0) {
			if ((WIFEXITED(status)) != 0) {
				if ((ret = WEXITSTATUS(status)) != 0) {
					err(stderr,
					    "hogcpu worker %i exited %i\n", pid,
					    ret);
					retval += ret;
				} else {
					dbg(stdout,
					    "<-- hogcpu worker exited (%i)\n",
					    pid);
				}
			} else {
				dbg(stdout,
				    "<-- hogcpu worker signalled (%i)\n", pid);
			}

			--i;
		} else {
			dbg(stdout, "wait() returned error: %s\n",
			    strerror(errno));
			err(stderr,
			    "detected missing hogcpu worker children\n");
			++retval;
			break;
		}
	}

	return retval;
}

int hogio(long long forks)
{
	long long i;
	int pid, retval = 0;

	/* Make local copies of global variables.  */
	int ignore = global_ignore;
	int retry = global_retry;
	int timeout = global_timeout;
	long backoff = global_backoff * forks;

	dbg(stdout, "using backoff sleep of %lius for hogio\n", backoff);

	for (i = 0; forks == 0 || i < forks; i++) {
		switch (pid = fork()) {
		case 0:	/* child */
			alarm(timeout);

			/* Use a backoff sleep to ensure we get good fork throughput.  */
			usleep(backoff);

			while (1)
				sync();

			/* This case never falls through; alarm signal can cause exit.  */
		case -1:	/* error */
			if (ignore) {
				++retval;
				wrn(stderr,
				    "hogio worker fork failed, continuing\n");
				usleep(retry);
				continue;
			}

			err(stderr, "hogio worker fork failed\n");
			return 1;
		default:	/* parent */
			dbg(stdout, "--> hogio worker forked (%i)\n", pid);
		}
	}

	/* Wait for our children to exit.  */
	while (i) {
		int status, ret;

		if ((pid = wait(&status)) > 0) {
			if ((WIFEXITED(status)) != 0) {
				if ((ret = WEXITSTATUS(status)) != 0) {
					err(stderr,
					    "hogio worker %i exited %i\n", pid,
					    ret);
					retval += ret;
				} else {
					dbg(stdout,
					    "<-- hogio worker exited (%i)\n",
					    pid);
				}
			} else {
				dbg(stdout, "<-- hogio worker signalled (%i)\n",
				    pid);
			}

			--i;
		} else {
			dbg(stdout, "wait() returned error: %s\n",
			    strerror(errno));
			err(stderr, "detected missing hogio worker children\n");
			++retval;
			break;
		}
	}

	return retval;
}

int hogvm(long long forks, long long chunks, long long bytes)
{
	long long i, j, k;
	int pid, retval = 0;
	char **ptr;

	/* Make local copies of global variables.  */
	int ignore = global_ignore;
	int retry = global_retry;
	int timeout = global_timeout;
	long backoff = global_backoff * forks;

	dbg(stdout, "using backoff sleep of %lius for hogvm\n", backoff);

	if (bytes == 0) {
		/* 512MB is guess at the largest value can than be malloced at once.  */
		bytes = 512 * 1024 * 1024;
	}

	for (i = 0; forks == 0 || i < forks; i++) {
		switch (pid = fork()) {
		case 0:	/* child */
			alarm(timeout);

			/* Use a backoff sleep to ensure we get good fork throughput.  */
			usleep(backoff);

			while (1) {
				ptr = (char **)malloc(chunks * 2);
				for (j = 0; chunks == 0 || j < chunks; j++) {
					if ((ptr[j] =
					     (char *)malloc(bytes *
							    sizeof(char)))) {
						for (k = 0; k < bytes; k++)
							ptr[j][k] = 'Z';	/* Ensure that COW happens.  */
						dbg(stdout,
						    "hogvm worker malloced %lli bytes\n",
						    k);
					} else if (ignore) {
						++retval;
						wrn(stderr,
						    "hogvm malloc failed, continuing\n");
						usleep(retry);
						continue;
					} else {
						++retval;
						err(stderr,
						    "hogvm malloc failed\n");
						break;
					}
				}
				if (global_vmhang && retval == 0) {
					dbg(stdout,
					    "sleeping forever with allocated memory\n");
					while (1)
						sleep(1024);
				}
				if (retval == 0) {
					dbg(stdout,
					    "hogvm worker freeing memory and starting over\n");
					for (j = 0; chunks == 0 || j < chunks;
					     j++) {
						free(ptr[j]);
					}
					free(ptr);
					continue;
				}

				exit(retval);
			}

			/* This case never falls through; alarm signal can cause exit.  */
		case -1:	/* error */
			if (ignore) {
				++retval;
				wrn(stderr,
				    "hogvm worker fork failed, continuing\n");
				usleep(retry);
				continue;
			}

			err(stderr, "hogvm worker fork failed\n");
			return 1;
		default:	/* parent */
			dbg(stdout, "--> hogvm worker forked (%i)\n", pid);
		}
	}

	/* Wait for our children to exit.  */
	while (i) {
		int status, ret;

		if ((pid = wait(&status)) > 0) {
			if ((WIFEXITED(status)) != 0) {
				if ((ret = WEXITSTATUS(status)) != 0) {
					err(stderr,
					    "hogvm worker %i exited %i\n", pid,
					    ret);
					retval += ret;
				} else {
					dbg(stdout,
					    "<-- hogvm worker exited (%i)\n",
					    pid);
				}
			} else {
				dbg(stdout, "<-- hogvm worker signalled (%i)\n",
				    pid);
			}

			--i;
		} else {
			dbg(stdout, "wait() returned error: %s\n",
			    strerror(errno));
			err(stderr, "detected missing hogvm worker children\n");
			++retval;
			break;
		}
	}

	return retval;
}

int hoghdd(long long forks, int clean, long long files, long long bytes)
{
	long long i, j;
	int fd, pid, retval = 0;
	int chunk = (1024 * 1024) - 1;	/* Minimize slow writing.  */
	char buff[chunk];

	/* Make local copies of global variables.  */
	int ignore = global_ignore;
	int retry = global_retry;
	int timeout = global_timeout;
	long backoff = global_backoff * forks;

	/* Initialize buffer with some random ASCII data.  */
	dbg(stdout, "seeding buffer with random data\n");
	for (i = 0; i < chunk - 1; i++) {
		j = rand();
		j = (j < 0) ? -j : j;
		j %= 95;
		j += 32;
		buff[i] = j;
	}
	buff[i] = '\n';

	dbg(stdout, "using backoff sleep of %lius for hoghdd\n", backoff);

	for (i = 0; forks == 0 || i < forks; i++) {
		switch (pid = fork()) {
		case 0:	/* child */
			alarm(timeout);

			/* Use a backoff sleep to ensure we get good fork throughput.  */
			usleep(backoff);

			while (1) {
				for (i = 0; i < files; i++) {
					char name[] = "./stress.XXXXXX";

					if ((fd = mkstemp(name)) < 0) {
						perror("mkstemp");
						err(stderr, "mkstemp failed\n");
						exit(1);
					}

					if (clean == 0) {
						dbg(stdout, "unlinking %s\n",
						    name);
						if (unlink(name)) {
							err(stderr,
							    "unlink failed\n");
							exit(1);
						}
					}

					dbg(stdout, "fast writing to %s\n",
					    name);
					for (j = 0;
					     bytes == 0 || j + chunk < bytes;
					     j += chunk) {
						if (write(fd, buff, chunk) !=
						    chunk) {
							err(stderr,
							    "write failed\n");
							exit(1);
						}
					}

					dbg(stdout, "slow writing to %s\n",
					    name);
					for (; bytes == 0 || j < bytes - 1; j++) {
						if (write(fd, "Z", 1) != 1) {
							err(stderr,
							    "write failed\n");
							exit(1);
						}
					}
					if (write(fd, "\n", 1) != 1) {
						err(stderr, "write failed\n");
						exit(1);
					}
					++j;

					dbg(stdout,
					    "closing %s after writing %lli bytes\n",
					    name, j);
					close(fd);

					if (clean == 1) {
						if (unlink(name)) {
							err(stderr,
							    "unlink failed\n");
							exit(1);
						}
					}
				}
				if (retval == 0) {
					dbg(stdout,
					    "hoghdd worker starting over\n");
					continue;
				}

				exit(retval);
			}

			/* This case never falls through; alarm signal can cause exit.  */
		case -1:	/* error */
			if (ignore) {
				++retval;
				wrn(stderr,
				    "hoghdd worker fork failed, continuing\n");
				usleep(retry);
				continue;
			}

			err(stderr, "hoghdd worker fork failed\n");
			return 1;
		default:	/* parent */
			dbg(stdout, "--> hoghdd worker forked (%i)\n", pid);
		}
	}

	/* Wait for our children to exit.  */
	while (i) {
		int status, ret;

		if ((pid = wait(&status)) > 0) {
			if ((WIFEXITED(status)) != 0) {
				if ((ret = WEXITSTATUS(status)) != 0) {
					err(stderr,
					    "hoghdd worker %i exited %i\n", pid,
					    ret);
					retval += ret;
				} else {
					dbg(stdout,
					    "<-- hoghdd worker exited (%i)\n",
					    pid);
				}
			} else {
				dbg(stdout,
				    "<-- hoghdd worker signalled (%i)\n", pid);
			}

			--i;
		} else {
			dbg(stdout, "wait() returned error: %s\n",
			    strerror(errno));
			err(stderr,
			    "detected missing hoghdd worker children\n");
			++retval;
			break;
		}
	}

	return retval;
}
