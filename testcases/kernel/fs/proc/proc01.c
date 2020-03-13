/*
 * proc01.c - Tests Linux /proc file reading.
 *
 * Copyright (C) 2001 Stephane Fillod <f4cfe@free.fr>
 * Copyright (c) 2008, 2009  Red Hat, Inc.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Further, this software is distributed without any warranty that it is
 * free of the rightful claim of any third person regarding infringement
 * or the like.  Any license provided herein, whether implied or
 * otherwise, applies only to this software file.  Patent licenses, if
 * any, provided herein do not apply to combinations of this program with
 * other software, or any other product whatsoever.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

#include "config.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>
#include <fnmatch.h>

#ifdef HAVE_LIBSELINUX_DEVEL
#include <selinux/selinux.h>
#endif

#include "test.h"

#define MAX_BUFF_SIZE 65536
#define MAX_FUNC_NAME 256

char *TCID = "proc01";
int TST_TOTAL = 1;

static int opt_verbose;
static int opt_procpath;
static char *opt_procpathstr;
static int opt_buffsize;
static int opt_readirq;
static char *opt_buffsizestr;
static int opt_maxmbytes;
static char *opt_maxmbytesstr;

static char *procpath = "/proc";
static const char selfpath[] = "/proc/self";
size_t buffsize = 1024;
static long long maxbytes;

unsigned long long total_read;
unsigned int total_obj;

struct mapping {
	char func[MAX_FUNC_NAME];
	char file[PATH_MAX];
	int err;
};

/* Those are known failures for 2.6.18 baremetal kernel and Xen dom0
   kernel on i686, x86_64, ia64, ppc64 and s390x. In addition, It looks
   like if SELinux is disabled, the test may still fail on some other
   entries. */
static const struct mapping known_issues[] = {
	{"open", "/proc/acpi/event", EBUSY},
	{"open", "/proc/sal/cpe/data", EBUSY},
	{"open", "/proc/sal/cmc/data", EBUSY},
	{"open", "/proc/sal/init/data", EBUSY},
	{"open", "/proc/sal/mca/data", EBUSY},
	{"open", "/proc/fs/nfsd/pool_stats", ENODEV},
	{"read", "/proc/fs/nfsd/clients/*/ctl", EINVAL},
	{"read", "/proc/acpi/event", EAGAIN},
	{"read", "/proc/kmsg", EAGAIN},
	{"read", "/proc/sal/cpe/event", EAGAIN},
	{"read", "/proc/sal/cmc/event", EAGAIN},
	{"read", "/proc/sal/init/event", EAGAIN},
	{"read", "/proc/sal/mca/event", EAGAIN},
	{"read", "/proc/xen/privcmd", EIO},
	{"read", "/proc/xen/privcmd", EINVAL},
	{"read", "/proc/self/mem", EIO},
	{"read", "/proc/self/task/[0-9]*/mem", EIO},
	{"read", "/proc/self/attr/*", EINVAL},
	{"read", "/proc/self/task/[0-9]*/attr/*", EINVAL},
	{"read", "/proc/self/ns/*", EINVAL},
	{"read", "/proc/self/task/[0-9]*/ns/*", EINVAL},
	{"read", "/proc/ppc64/rtas/error_log", EINVAL},
	{"read", "/proc/powerpc/rtas/error_log", EINVAL},
	{"read", "/proc/fs/nfsd/unlock_filesystem", EINVAL},
	{"read", "/proc/fs/nfsd/unlock_ip", EINVAL},
	{"read", "/proc/fs/nfsd/filehandle", EINVAL},
	{"read", "/proc/fs/nfsd/.getfs", EINVAL},
	{"read", "/proc/fs/nfsd/.getfd", EINVAL},
	{"read", "/proc/self/net/rpc/use-gss-proxy", EAGAIN},
	{"read", "/proc/sys/net/ipv6/conf/*/stable_secret", EIO},
	{"read", "/proc/sys/vm/nr_hugepages", EOPNOTSUPP},
	{"read", "/proc/sys/vm/nr_overcommit_hugepages", EOPNOTSUPP},
	{"read", "/proc/sys/vm/nr_hugepages_mempolicy", EOPNOTSUPP},
	{"read", "/proc/pressure/*", EOPNOTSUPP},
	{"", "", 0}
};

/*
 * If a particular LSM is enabled, it is expected that some entries can
 * be read successfully. Otherwise, those entries will retrun some
 * failures listed above. Here to add any LSM specific entries.
 */

/*
 * Test macro to indicate that SELinux libraries and headers are
 * installed.
 */
#ifdef HAVE_LIBSELINUX_DEVEL
static const char lsm_should_work[][PATH_MAX] = {
	"/proc/self/attr/*",
	"/proc/self/task/[0-9]*/attr/*",
	""
};

/* Place holder for none of LSM is detected. */
#else
static const char lsm_should_work[][PATH_MAX] = {
	""
};
#endif

/* Known files that does not honor O_NONBLOCK, so they will hang
   the test while being read. */
static const char error_nonblock[][PATH_MAX] = {
	"/proc/xen/xenbus",
	""
};

/*
 * Verify expected failures, and then let the test to continue.
 *
 * Return 0 when a problem errno is found.
 * Return 1 when a known issue is found.
 *
 */
static int found_errno(const char *syscall, const char *obj, int tmperr)
{
	int i;

	/* Should not see any error for certain entries if a LSM is enabled. */
#ifdef HAVE_LIBSELINUX_DEVEL
	if (is_selinux_enabled()) {
		for (i = 0; lsm_should_work[i][0] != '\0'; i++) {
			if (!strcmp(obj, lsm_should_work[i]) ||
			    !fnmatch(lsm_should_work[i], obj, FNM_PATHNAME)) {
				return 0;
			}
		}
	}
#endif
	for (i = 0; known_issues[i].err != 0; i++) {
		if (tmperr == known_issues[i].err &&
		    (!strcmp(obj, known_issues[i].file) ||
		     !fnmatch(known_issues[i].file, obj, FNM_PATHNAME)) &&
		    !strcmp(syscall, known_issues[i].func)) {
			/* Using strcmp / fnmatch could have messed up the
			 * errno value. */
			errno = tmperr;
			tst_resm(TINFO | TERRNO, "%s: known issue", obj);
			return 1;
		}
	}
	return 0;
}

static void cleanup(void)
{
	tst_rmdir();
}

static void setup(void)
{
	tst_sig(FORK, DEF_HANDLER, cleanup);
	TEST_PAUSE;
	tst_tmpdir();
}

void help(void)
{
	printf("  -b x    read byte count\n");
	printf("  -m x    max megabytes to read from single file\n");
	printf("  -q      read .../irq/... entries\n");
	printf("  -r x    proc pathname\n");
	printf("  -v      verbose mode\n");
}

/*
 * add the -m option whose parameter is the
 * pages that should be mapped.
 */
static option_t options[] = {
	{"b:", &opt_buffsize, &opt_buffsizestr},
	{"m:", &opt_maxmbytes, &opt_maxmbytesstr},
	{"q", &opt_readirq, NULL},
	{"r:", &opt_procpath, &opt_procpathstr},
	{"v", &opt_verbose, NULL},
	{NULL, NULL, NULL}
};

/*
 * NB: this function is recursive
 * returns 0 if no error encountered, otherwise number of errors (objs)
 *
 * REM: Funny enough, while developing this function (actually replacing
 *	streamed fopen by standard open), I hit a real /proc bug.
 *	On a 2.2.13-SuSE kernel, "cat /proc/tty/driver/serial" would fail
 *	with EFAULT, while "cat /proc/tty/driver/serial > somefile" wouldn't.
 *	Okay, this might be due to a slight serial misconfiguration, but still.
 *	Analysis with strace showed up the difference was on the count size
 *	of read (1024 bytes vs 4096 bytes). So I tested further..
 *	read count of 512 bytes adds /proc/tty/drivers to the list
 *	of broken proc files, while 64 bytes reads removes
 *	/proc/tty/driver/serial from the list. Interesting, isn't it?
 *	Now, there's a -b option to this test, so you can try your luck. --SF
 *
 * It's more fun to run this test it as root, as all the files will be accessible!
 * (however, be careful, there might be some bufferoverflow holes..)
 * reading proc files might be also a good kernel latency killer.
 */
static long readproc(const char *obj)
{
	DIR *dir = NULL;	/* pointer to a directory */
	struct dirent *dir_ent;	/* pointer to directory entries */
	char dirobj[PATH_MAX];	/* object inside directory to modify */
	struct stat statbuf;	/* used to hold stat information */
	int fd, tmperr, i;
	ssize_t nread;
	static char buf[MAX_BUFF_SIZE];	/* static kills reentrancy, but we don't care about the contents */
	unsigned long long file_total_read = 0;

	/* Determine the file type */
	if (lstat(obj, &statbuf) < 0) {

		/* permission denied is not considered as error */
		if (errno != EACCES) {
			tst_resm(TFAIL | TERRNO, "%s: lstat", obj);
			return 1;
		}
		return 0;

	}

	/* Prevent loops, but read /proc/self. */
	if (S_ISLNK(statbuf.st_mode) && strcmp(obj, selfpath))
		return 0;

	total_obj++;

	/* Take appropriate action, depending on the file type */
	if (S_ISDIR(statbuf.st_mode) || !strcmp(obj, selfpath)) {

		/* object is a directory */

		/*
		 * Skip over the /proc/irq directory, unless the user
		 * requested that we read the directory because it could
		 * map to a broken driver which effectively `hangs' the
		 * test.
		 */
		if (!opt_readirq && !strcmp("/proc/irq", obj)) {
			return 0;
			/* Open the directory to get access to what is in it */
		} else if ((dir = opendir(obj)) == NULL) {
			if (errno != EACCES) {
				tst_resm(TFAIL | TERRNO, "%s: opendir", obj);
				return 1;
			}
			return 0;
		} else {

			long ret_val = 0;

			/* Loop through the entries in the directory */
			for (dir_ent = (struct dirent *)readdir(dir);
			     dir_ent != NULL;
			     dir_ent = (struct dirent *)readdir(dir)) {

				/* Ignore ".", "..", "kcore", and
				 * "/proc/<pid>" (unless this is our
				 * starting point as directed by the
				 * user).
				 */
				if (strcmp(dir_ent->d_name, ".") &&
				    strcmp(dir_ent->d_name, "..") &&
				    strcmp(dir_ent->d_name, "kcore") &&
				    (fnmatch("[0-9]*", dir_ent->d_name,
					     FNM_PATHNAME) ||
				     strcmp(obj, procpath))) {

					if (opt_verbose) {
						fprintf(stderr, "%s\n",
							dir_ent->d_name);
					}

					/* Recursively call this routine to test the
					 * current entry */
					snprintf(dirobj, PATH_MAX,
						 "%s/%s", obj, dir_ent->d_name);
					ret_val += readproc(dirobj);

				}

			}

			/* Close the directory */
			if (dir)
				(void)closedir(dir);

			return ret_val;

		}

	} else {		/* if it's not a dir, read it! */

		if (!S_ISREG(statbuf.st_mode))
			return 0;

#ifdef DEBUG
		fprintf(stderr, "%s", obj);
#endif

		/* is O_NONBLOCK enough to escape from FIFO's ? */
		fd = open(obj, O_RDONLY | O_NONBLOCK);
		if (fd < 0) {
			tmperr = errno;

			if (!found_errno("open", obj, tmperr)) {

				errno = tmperr;

				if (errno != EACCES) {
					tst_resm(TFAIL | TERRNO,
						 "%s: open failed", obj);
					return 1;
				}

			}
			return 0;

		}

		/* Skip write-only files. */
		if ((statbuf.st_mode & S_IRUSR) == 0 &&
		    (statbuf.st_mode & S_IWUSR) != 0) {
			tst_resm(TINFO, "%s: is write-only.", obj);
			return 0;
		}

		/* Skip files does not honor O_NONBLOCK. */
		for (i = 0; error_nonblock[i][0] != '\0'; i++) {
			if (!strcmp(obj, error_nonblock[i])) {
				tst_resm(TINFO, "%s: does not honor "
					 "O_NONBLOCK", obj);
				return 0;
			}
		}

		file_total_read = 0;
		do {

			nread = read(fd, buf, buffsize);

			if (nread < 0) {

				tmperr = errno;
				(void)close(fd);

				/* ignore no perm (not root) and no
				 * process (terminated) errors */
				if (!found_errno("read", obj, tmperr)) {

					errno = tmperr;

					if (errno != EACCES && errno != ESRCH) {
						tst_resm(TFAIL | TERRNO,
							 "read failed: "
							 "%s", obj);
						return 1;
					}
					return 0;

				}

			} else
				file_total_read += nread;

			if (opt_verbose) {
#ifdef DEBUG
				fprintf(stderr, "%ld", nread);
#endif
				fprintf(stderr, ".");
			}

			if ((maxbytes > 0) && (file_total_read > maxbytes)) {
				tst_resm(TINFO, "%s: reached maxmbytes (-m)",
					 obj);
				break;
			}
		} while (0 < nread);
		total_read += file_total_read;

		if (opt_verbose)
			fprintf(stderr, "\n");

		if (0 <= fd)
			(void)close(fd);

	}

	/* It's better to assume success by default rather than failure. */
	return 0;

}

int main(int argc, char *argv[])
{
	int lc;

	tst_parse_opts(argc, argv, options, help);

	if (opt_buffsize) {
		size_t bs;
		bs = atoi(opt_buffsizestr);
		if (bs <= MAX_BUFF_SIZE)
			buffsize = bs;
		else
			tst_brkm(TBROK, cleanup,
				 "Invalid arg for -b (max: %u): %s",
				 MAX_BUFF_SIZE, opt_buffsizestr);
	}
	if (opt_maxmbytes)
		maxbytes = atoi(opt_maxmbytesstr) * 1024 * 1024;

	if (opt_procpath)
		procpath = opt_procpathstr;

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		TEST(readproc(procpath));

		if (TEST_RETURN != 0) {
			tst_resm(TFAIL, "readproc() failed with %ld errors.",
				 TEST_RETURN);
		} else {
			tst_resm(TPASS, "readproc() completed successfully, "
				 "total read: %llu bytes, %u objs", total_read,
				 total_obj);
		}
	}

	cleanup();
	tst_exit();
}
