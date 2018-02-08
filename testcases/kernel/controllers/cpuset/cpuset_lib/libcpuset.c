/*
 * cpuset user library implementation.
 *
 * Copyright (c) 2006-2007 Silicon Graphics, Inc. All rights reserved.
 *
 * Paul Jackson <pj@sgi.com>
 */

/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2.1 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 */

#define _GNU_SOURCE	/* need to see pread() and syscall() */
#include <unistd.h>

#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <fts.h>
#include <limits.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <time.h>
#include <utime.h>
#include <sys/utsname.h>	/* for cpuset_would_crash_kernel() */

#include "bitmask.h"
#include "cpuset.h"
#include "common.h"
#include "test.h"
#include "lapi/syscalls.h"
#include "config.h"

#if HAVE_LINUX_MEMPOLICY_H
#include <linux/mempolicy.h>

/* Bump version, and update Change History, when libcpuset API changes */
#define CPUSET_VERSION 3

/*
 * For a history of what changed in each version, see the "Change
 * History" section, at the end of the libcpuset master document.
 */

int cpuset_version(void)
{
	return CPUSET_VERSION;
}

struct cpuset {
	struct bitmask *cpus;
	struct bitmask *mems;
	char cpu_exclusive;
	char mem_exclusive;
	char mem_hardwall;
	char notify_on_release;
	char memory_migrate;
	char memory_pressure_enabled;
	char memory_spread_page;
	char memory_spread_slab;
	char sched_load_balance;
	int sched_relax_domain_level;

	/*
	 * Each field 'x' above gets an 'x_valid' field below.
	 * The apply_cpuset_settings() will only set those fields whose
	 * corresponding *_valid flags are set.  The cpuset_alloc()
	 * routine clears these flags as part of the clear in calloc(),
	 * and the various cpuset_set*() routines set these flags when
	 * setting the corresponding value.
	 *
	 * The purpose of these valid fields is to ensure that when
	 * we create a new cpuset, we don't accidentally overwrite
	 * some non-zero kernel default, such as an inherited
	 * memory_spread_* flag, just because the user application
	 * code didn't override the default zero settings resulting
	 * from the calloc() call in cpuset_alloc().
	 *
	 * The choice of 'char' for the type of the flags above,
	 * but a bitfield for the flags below, is somewhat capricious.
	 */
	unsigned cpus_valid:1;
	unsigned mems_valid:1;
	unsigned cpu_exclusive_valid:1;
	unsigned mem_exclusive_valid:1;
	unsigned mem_hardwall_valid:1;
	unsigned notify_on_release_valid:1;
	unsigned memory_migrate_valid:1;
	unsigned memory_pressure_enabled_valid:1;
	unsigned memory_spread_page_valid:1;
	unsigned memory_spread_slab_valid:1;
	unsigned sched_load_balance_valid:1;
	unsigned sched_relax_domain_level_valid:1;

	/*
	 * if the relative variable was modified, use following flags
	 * to put a mark
	 */
	unsigned cpus_dirty:1;
	unsigned mems_dirty:1;
	unsigned cpu_exclusive_dirty:1;
	unsigned mem_exclusive_dirty:1;
	unsigned mem_hardwall_dirty:1;
	unsigned notify_on_release_dirty:1;
	unsigned memory_migrate_dirty:1;
	unsigned memory_pressure_enabled_dirty:1;
	unsigned memory_spread_page_dirty:1;
	unsigned memory_spread_slab_dirty:1;
	unsigned sched_load_balance_dirty:1;
	unsigned sched_relax_domain_level_dirty:1;
};

/* Presumed cpuset file system mount point */
static const char *cpusetmnt = "/dev/cpuset";

/* Stashed copy of cpunodemap[], mapping each cpu to its node. */
static const char *mapfile = "/var/run/cpunodemap";

/* The primary source for the cpunodemap[] is available below here. */
static const char *sysdevices = "/sys/devices/system";

/* small buffer size - for reading boolean flags or map file (1 or 2 ints) */
#define SMALL_BUFSZ 16

/*
 * The 'mask_size_file' is used to ferrit out the kernel cpumask_t
 * and nodemask_t sizes.  The lines in this file that begin with the
 * strings 'cpumask_prefix' and 'nodemask_prefix' display a cpumask
 * and nodemask string, respectively.  The lengths of these strings
 * reflect the kernel's internal cpumask_t and nodemask_t sizes,
 * which sizes are needed to correctly call the sched_setaffinity
 * and set_mempolicy system calls, and to size user level
 * bitmasks to match the kernels.
 */

static const char *mask_size_file = "/proc/self/status";
static const char *cpumask_prefix = "Cpus_allowed:\t";
static const char *nodemask_prefix = "Mems_allowed:\t";

/*
 * Sizes of kernel cpumask_t and nodemask_t bitmaps, in bits.
 *
 * The first time we need these, we parse the Cpus_allowed and
 * Mems_allowed lines from mask_size_file ("/proc/self/status").
 */

static int cpumask_sz;
static int nodemask_sz;

/*
 * These defaults only kick in if we fail to size the kernel
 * cpumask and nodemask by reading the Cpus_allowed and
 * Mems_allowed fields from the /proc/self/status file.
 */

#define DEFCPUBITS (512)
#define DEFNODEBITS (DEFCPUBITS/2)

/*
 * Arch-neutral API for obtaining NUMA distances between CPUs
 * and Memory Nodes, via the files:
 *	/sys/devices/system/node/nodeN/distance
 * which have lines such as:
 *	46 66 10 20
 * which say that for cpu on node N (from the path above), the
 * distance to nodes 0, 1, 2, and 3 are 44, 66, 10, and 20,
 * respectively.
 */

static const char *distance_directory = "/sys/devices/system/node";

/*
 * Someday, we should disable, then later discard, the SN code
 * marked ALTERNATE_SN_DISTMAP.
 */

#define ALTERNATE_SN_DISTMAP 1
#ifdef ALTERNATE_SN_DISTMAP

/*
 * Alternative SN (SGI ia64) architecture specific API for obtaining
 * NUMA distances between CPUs and Memory Nodes is via the file
 * /proc/sgi_sn/sn_topology, which has lines such as:
 *
 *   node 2 001c14#0 local asic SHub_1.1, nasid 0x4, dist 46:66:10:20
 *
 * which says that for each CPU on node 2, the distance to nodes
 * 0, 1, 2 and 3 are 46, 66, 10 and 20, respectively.
 *
 * This file has other lines as well, which start with other
 * keywords than "node".  Ignore these other lines.
 */

static const char *sn_topology = "/proc/sgi_sn/sn_topology";
static const char *sn_top_node_prefix = "node ";

#endif

/*
 * Check that cpusets supported, /dev/cpuset mounted.
 * If ok, return 0.
 * If not, return -1 and set errno:
 *	ENOSYS - kernel doesn't support cpusets
 *	ENODEV - /dev/cpuset not mounted
 */

static enum {
	check_notdone,
	check_enosys,
	check_enodev,
	check_ok
} check_state = check_notdone;

static int check(void)
{
	if (check_state == check_notdone) {
		struct stat statbuf;

		if (stat("/proc/self/cpuset", &statbuf) < 0) {
			check_state = check_enosys;
			goto done;
		}

		if (stat("/dev/cpuset/tasks", &statbuf) < 0) {
			check_state = check_enodev;
			goto done;
		}

		check_state = check_ok;
	}
done:
	switch (check_state) {
	case check_enosys:
		errno = ENOSYS;
		return -1;
	case check_enodev:
		errno = ENODEV;
		return -1;
	default:
		break;
	}
	return 0;
}

static void chomp(char *s)
{
	char *t;

	for (t = s + strlen(s) - 1; t >= s; t--) {
		if (*t == '\n' || *t == '\r')
			*t = '\0';
		else
			break;
	}
}

/*
 * Determine number of bytes in a seekable open file, without
 * assuming that stat(2) on that file has a useful size.
 * Has side affect of leaving the file rewound to the beginnning.
 */
static int filesize(FILE * fp)
{
	int sz = 0;
	rewind(fp);
	while (fgetc(fp) != EOF)
		sz++;
	rewind(fp);
	return sz;
}

/* Are strings s1 and s2 equal? */
static int streq(const char *s1, const char *s2)
{
	return strcmp(s1, s2) == 0;
}

/* Is string 'pre' a prefix of string 's'? */
static int strprefix(const char *s, const char *pre)
{
	return strncmp(s, pre, strlen(pre)) == 0;
}

/*
 * char *flgets(char *buf, int buflen, FILE *fp)
 *
 * Obtain one line from input file fp.  Copy up to first
 * buflen-1 chars of line into buffer buf, discarding any remainder
 * of line.  Stop reading at newline, discarding newline.
 * Nul terminate result and return pointer to buffer buf
 * on success, or NULL if nothing more to read or failure.
 */

static char *flgets(char *buf, int buflen, FILE * fp)
{
	int c = -1;
	char *bp;

	bp = buf;
	while ((--buflen > 0) && ((c = getc(fp)) >= 0)) {
		if (c == '\n')
			goto newline;
		*bp++ = c;
	}
	if ((c < 0) && (bp == buf))
		return NULL;

	if (c > 0) {
		while ((c = getc(fp)) >= 0) {
			if (c == '\n')
				break;
		}
	}

newline:
	*bp++ = '\0';
	return buf;
}

/*
 * sgetc(const char *inputbuf, int *offsetptr)
 *
 * Return next char from nul-terminated input buffer inputbuf,
 * starting at offset *offsetptr.  Increment *offsetptr.
 * If next char would be nul ('\0'), return EOF and don't
 * increment *offsetptr.
 */

static int sgetc(const char *inputbuf, int *offsetptr)
{
	char c;

	if ((c = inputbuf[*offsetptr]) != 0) {
		*offsetptr = *offsetptr + 1;
		return c;
	} else {
		return EOF;
	}
}

/*
 * char *slgets(char *buf, int buflen, const char *inputbuf, int *offsetptr)
 *
 * Obtain next line from nul-terminated input buffer 'inputbuf',
 * starting at offset *offsetptr.  Copy up to first buflen-1
 * chars of line into output buffer buf, discarding any remainder
 * of line.  Stop reading at newline, discarding newline.
 * Nul terminate result and return pointer to output buffer
 * buf on success, or NULL if nothing more to read.
 */

static char *slgets(char *buf, int buflen, const char *inputbuf, int *offsetptr)
{
	int c = -1;
	char *bp;

	bp = buf;
	while ((--buflen > 0) && ((c = sgetc(inputbuf, offsetptr)) >= 0)) {
		if (c == '\n')
			goto newline;
		*bp++ = c;
	}
	if ((c < 0) && (bp == buf))
		return NULL;

	if (c > 0) {
		while ((c = sgetc(inputbuf, offsetptr)) >= 0) {
			if (c == '\n')
				break;
		}
	}

newline:
	*bp++ = '\0';
	return buf;
}

/*
 * time_t get_mtime(char *path)
 *
 * Return modtime of file at location path, else return 0.
 */

static time_t get_mtime(const char *path)
{
	struct stat statbuf;

	if (stat(path, &statbuf) != 0)
		return 0;
	return statbuf.st_mtime;
}

/*
 * int set_mtime(const char *path, time_t mtime)
 *
 * Set modtime of file 'path' to 'mtime'.  Return 0 on success,
 * or -1 on error, setting errno.
 */

static int set_mtime(const char *path, time_t mtime)
{
	struct utimbuf times;

	times.actime = mtime;
	times.modtime = mtime;
	return utime(path, &times);
}

/*
 * True if two pathnames resolve to same file.
 * False if either path can not be stat'd,
 * or if the two paths resolve to a different file.
 */

static int samefile(const char *path1, const char *path2)
{
	struct stat sb1, sb2;

	if (stat(path1, &sb1) != 0)
		return 0;
	if (stat(path2, &sb2) != 0)
		return 0;
	return sb1.st_ino == sb2.st_ino && sb1.st_dev == sb2.st_dev;
}

#define slash(c) (*(c) == '/')
#define eocomp(c) (slash(c) || !*(c))
#define dot1(c) (*(c) == '.' && eocomp(c+1))

/* In place path compression.  Remove extra dots and slashes. */
static char *pathcomp(char *p)
{
	char *a = p;
	char *b = p;

	if (!p || !*p)
		return p;
	if (slash(p))
		*b++ = *a++;
	for (;;) {
		if (slash(a))
			while (slash(++a))
				continue;
		if (!*a) {
			if (b == p)
				*b++ = '.';
			*b = '\0';
			return (p);
		} else if (dot1(a)) {
			a++;
		} else {
			if ((b != p) && !slash(b - 1))
				*b++ = '/';
			while (!eocomp(a))
				*b++ = *a++;
		}
	}
}

#undef slash
#undef eocomp
#undef dot1

/*
 * pathcat2(buf, buflen, name1, name2)
 *
 * Return buf, of length buflen, with name1/name2 stored in it.
 */

static char *pathcat2(char *buf, int buflen, const char *name1,
		      const char *name2)
{
	(void)snprintf(buf, buflen, "%s/%s", name1, name2);
	return pathcomp(buf);
}

/*
 * pathcat3(buf, buflen, name1, name2, name3)
 *
 * Return buf, of length buflen, with name1/name2/name3 stored in it.
 */

static char *pathcat3(char *buf, int buflen, const char *name1,
		      const char *name2, const char *name3)
{
	(void)snprintf(buf, buflen, "%s/%s/%s", name1, name2, name3);
	return pathcomp(buf);
}

/*
 * fullpath(buf, buflen, name)
 *
 * Put full path of cpuset 'name' in buffer 'buf'.  If name
 * starts with a slash (``/``) character, then this a path
 * relative to ``/dev/cpuset``, otherwise it is relative to
 * the current tasks cpuset.  Return 0 on success, else
 * -1 on error, setting errno.
 */

static int fullpath(char *buf, int buflen, const char *name)
{
	int len;

	/* easy case */
	if (*name == '/') {
		pathcat2(buf, buflen, cpusetmnt, name);
		pathcomp(buf);
		return 0;
	}

	/* hard case */
	snprintf(buf, buflen, "%s/", cpusetmnt);
	len = strlen(buf);
	if (cpuset_getcpusetpath(0, buf + len, buflen - len) == NULL)
		return -1;
	if (strlen(buf) >= buflen - 1 - strlen(name)) {
		errno = E2BIG;
		return -1;
	}
	strcat(buf, "/");
	strcat(buf, name);
	pathcomp(buf);
	return 0;
}

/*
 * fullpath2(buf, buflen, name1, name2)
 *
 * Like fullpath(), only concatenate two pathname components on end.
 */

static int fullpath2(char *buf, int buflen, const char *name1,
		     const char *name2)
{
	if (fullpath(buf, buflen, name1) < 0)
		return -1;
	if (strlen(buf) >= buflen - 1 - strlen(name2)) {
		errno = E2BIG;
		return -1;
	}
	strcat(buf, "/");
	strcat(buf, name2);
	pathcomp(buf);
	return 0;
}

/*
 * Convert the string length of an ascii hex mask to the number
 * of bits represented by that mask.
 *
 * The cpumask and nodemask values in /proc/self/status are in an
 * ascii format that uses 9 characters for each 32 bits of mask.
 */
static int s2nbits(const char *s)
{
	return strlen(s) * 32 / 9;
}

static void update_mask_sizes(void)
{
	FILE *fp = NULL;
	char *buf = NULL;
	int fsize;

	if ((fp = fopen(mask_size_file, "r")) == NULL)
		goto done;
	fsize = filesize(fp);
	if ((buf = malloc(fsize)) == NULL)
		goto done;

	/*
	 * Beware: mask sizing arithmetic is fussy.
	 * The trailing newline left by fgets() is required.
	 */
	while (fgets(buf, fsize, fp)) {
		if (strprefix(buf, cpumask_prefix))
			cpumask_sz = s2nbits(buf + strlen(cpumask_prefix));
		if (strprefix(buf, nodemask_prefix))
			nodemask_sz = s2nbits(buf + strlen(nodemask_prefix));
	}
done:
	free(buf);
	if (fp != NULL)
		fclose(fp);
	if (cpumask_sz == 0)
		cpumask_sz = DEFCPUBITS;
	if (nodemask_sz == 0)
		nodemask_sz = DEFNODEBITS;
}

/* Allocate a new struct cpuset */
struct cpuset *cpuset_alloc(void)
{
	struct cpuset *cp = NULL;
	int nbits;

	if ((cp = calloc(1, sizeof(struct cpuset))) == NULL)
		goto err;

	nbits = cpuset_cpus_nbits();
	if ((cp->cpus = bitmask_alloc(nbits)) == NULL)
		goto err;

	nbits = cpuset_mems_nbits();
	if ((cp->mems = bitmask_alloc(nbits)) == NULL)
		goto err;

	return cp;
err:
	if (cp && cp->cpus)
		bitmask_free(cp->cpus);
	if (cp && cp->mems)
		bitmask_free(cp->mems);
	free(cp);
	return NULL;
}

/* Free struct cpuset *cp */
void cpuset_free(struct cpuset *cp)
{
	if (!cp)
		return;
	if (cp->cpus)
		bitmask_free(cp->cpus);
	if (cp->mems)
		bitmask_free(cp->mems);
	free(cp);
}

/* Number of bits in a CPU bitmask on current system */
int cpuset_cpus_nbits(void)
{
	if (cpumask_sz == 0)
		update_mask_sizes();
	return cpumask_sz;
}

/* Number of bits in a Memory bitmask on current system */
int cpuset_mems_nbits(void)
{
	if (nodemask_sz == 0)
		update_mask_sizes();
	return nodemask_sz;
}

/* Set CPUs in cpuset cp to bitmask cpus */
int cpuset_setcpus(struct cpuset *cp, const struct bitmask *cpus)
{
	if (cp->cpus)
		bitmask_free(cp->cpus);
	cp->cpus = bitmask_alloc(bitmask_nbits(cpus));
	if (cp->cpus == NULL)
		return -1;
	bitmask_copy(cp->cpus, cpus);
	cp->cpus_valid = 1;
	cp->cpus_dirty = 1;
	return 0;
}

/* Set Memory Nodes in cpuset cp to bitmask mems */
int cpuset_setmems(struct cpuset *cp, const struct bitmask *mems)
{
	if (cp->mems)
		bitmask_free(cp->mems);
	cp->mems = bitmask_alloc(bitmask_nbits(mems));
	if (cp->mems == NULL)
		return -1;
	bitmask_copy(cp->mems, mems);
	cp->mems_valid = 1;
	cp->mems_dirty = 1;
	return 0;
}

/* Set integer value optname of cpuset cp */
int cpuset_set_iopt(struct cpuset *cp, const char *optionname, int value)
{
	if (streq(optionname, "cpu_exclusive")) {
		cp->cpu_exclusive = ! !value;
		cp->cpu_exclusive_valid = 1;
		cp->cpu_exclusive_dirty = 1;
	} else if (streq(optionname, "mem_exclusive")) {
		cp->mem_exclusive = ! !value;
		cp->mem_exclusive_valid = 1;
		cp->mem_exclusive_dirty = 1;
	} else if (streq(optionname, "mem_hardwall")) {
		cp->mem_hardwall = ! !value;
		cp->mem_hardwall_valid = 1;
		cp->mem_hardwall_dirty = 1;
	} else if (streq(optionname, "notify_on_release")) {
		cp->notify_on_release = ! !value;
		cp->notify_on_release_valid = 1;
		cp->notify_on_release_dirty = 1;
	} else if (streq(optionname, "memory_pressure_enabled")) {
		cp->memory_pressure_enabled = ! !value;
		cp->memory_pressure_enabled_valid = 1;
		cp->memory_pressure_enabled_dirty = 1;
	} else if (streq(optionname, "memory_migrate")) {
		cp->memory_migrate = ! !value;
		cp->memory_migrate_valid = 1;
		cp->memory_migrate_dirty = 1;
	} else if (streq(optionname, "memory_spread_page")) {
		cp->memory_spread_page = ! !value;
		cp->memory_spread_page_valid = 1;
		cp->memory_spread_page_dirty = 1;
	} else if (streq(optionname, "memory_spread_slab")) {
		cp->memory_spread_slab = ! !value;
		cp->memory_spread_slab_valid = 1;
		cp->memory_spread_slab_dirty = 1;
	} else if (streq(optionname, "sched_load_balance")) {
		cp->sched_load_balance = ! !value;
		cp->sched_load_balance_valid = 1;
		cp->sched_load_balance_dirty = 1;
	} else if (streq(optionname, "sched_relax_domain_level")) {
		cp->sched_relax_domain_level = value;
		cp->sched_relax_domain_level_valid = 1;
		cp->sched_relax_domain_level_dirty = 1;
	} else
		return -2;	/* optionname not recognized */
	return 0;
}

/* [optional] Set string value optname */
int cpuset_set_sopt(UNUSED struct cpuset *cp, UNUSED const char *optionname,
		    UNUSED const char *value)
{
	return -2;		/* For now, all string options unrecognized */
}

/* Return handle for reading memory_pressure. */
int cpuset_open_memory_pressure(const char *cpusetpath)
{
	char buf[PATH_MAX];

	fullpath2(buf, sizeof(buf), cpusetpath, "memory_pressure");
	return open(buf, O_RDONLY);
}

/* Return current memory_pressure of cpuset. */
int cpuset_read_memory_pressure(int han)
{
	char buf[SMALL_BUFSZ];

	if (pread(han, buf, sizeof(buf), 0L) < 0)
		return -1;
	return atoi(buf);
}

/* Close handle for reading memory pressure. */
void cpuset_close_memory_pressure(int han)
{
	close(han);
}

/*
 * Resolve cpuset pointer (to that of current task if cp == NULL).
 *
 * If cp not NULL, just return it.  If cp is NULL, return pointer
 * to temporary cpuset for current task, and set *cp_tofree to
 * pointer to that same temporary cpuset, to be freed later.
 *
 * Return NULL and set errno on error.  Errors can occur when
 * resolving the current tasks cpuset.
 */
static const struct cpuset *resolve_cp(const struct cpuset *cp,
				       struct cpuset **cp_tofree)
{
	const struct cpuset *rcp;

	if (cp) {
		rcp = cp;
	} else {
		struct cpuset *cp1 = cpuset_alloc();
		if (cp1 == NULL)
			goto err;
		if (cpuset_cpusetofpid(cp1, 0) < 0) {
			cpuset_free(cp1);
			goto err;
		}
		*cp_tofree = cp1;
		rcp = cp1;
	}
	return rcp;
err:
	return NULL;
}

/* Write CPUs in cpuset cp (current task if cp == NULL) to bitmask cpus */
int cpuset_getcpus(const struct cpuset *cp, struct bitmask *cpus)
{
	struct cpuset *cp_tofree = NULL;
	const struct cpuset *cp1 = resolve_cp(cp, &cp_tofree);

	if (!cp1)
		goto err;
	if (cp1->cpus == NULL) {
		errno = EINVAL;
		goto err;
	}
	bitmask_copy(cpus, cp1->cpus);
	cpuset_free(cp_tofree);
	return 0;
err:
	cpuset_free(cp_tofree);
	return -1;
}

/* Write Memory Nodes in cp (current task if cp == NULL) to bitmask mems */
int cpuset_getmems(const struct cpuset *cp, struct bitmask *mems)
{
	struct cpuset *cp_tofree = NULL;
	const struct cpuset *cp1 = resolve_cp(cp, &cp_tofree);

	if (!cp1)
		goto err;
	if (cp1->mems == NULL) {
		errno = EINVAL;
		goto err;
	}
	bitmask_copy(mems, cp1->mems);
	cpuset_free(cp_tofree);
	return 0;
err:
	cpuset_free(cp_tofree);
	return -1;
}

/* Return number of CPUs in cpuset cp (current task if cp == NULL) */
int cpuset_cpus_weight(const struct cpuset *cp)
{
	struct cpuset *cp_tofree = NULL;
	const struct cpuset *cp1 = resolve_cp(cp, &cp_tofree);
	int w = -1;

	if (!cp1)
		goto err;
	if (cp1->cpus == NULL) {
		errno = EINVAL;
		goto err;
	}
	w = bitmask_weight(cp1->cpus);
	/* fall into ... */
err:
	cpuset_free(cp_tofree);
	return w;
}

/* Return number of Memory Nodes in cpuset cp (current task if cp == NULL) */
int cpuset_mems_weight(const struct cpuset *cp)
{
	struct cpuset *cp_tofree = NULL;
	const struct cpuset *cp1 = resolve_cp(cp, &cp_tofree);
	int w = -1;

	if (!cp1)
		goto err;
	if (cp1->mems == NULL) {
		errno = EINVAL;
		goto err;
	}
	w = bitmask_weight(cp1->mems);
	/* fall into ... */
err:
	cpuset_free(cp_tofree);
	return w;
}

/* Return integer value of option optname in cp */
int cpuset_get_iopt(const struct cpuset *cp, const char *optionname)
{
	if (streq(optionname, "cpu_exclusive"))
		return cp->cpu_exclusive;
	else if (streq(optionname, "mem_exclusive"))
		return cp->mem_exclusive;
	else if (streq(optionname, "mem_hardwall"))
		return cp->mem_hardwall;
	else if (streq(optionname, "notify_on_release"))
		return cp->notify_on_release;
	else if (streq(optionname, "memory_pressure_enabled"))
		return cp->memory_pressure_enabled;
	else if (streq(optionname, "memory_migrate"))
		return cp->memory_migrate;
	else if (streq(optionname, "memory_spread_page"))
		return cp->memory_spread_page;
	else if (streq(optionname, "memory_spread_slab"))
		return cp->memory_spread_slab;
	else if (streq(optionname, "sched_load_balance"))
		return cp->sched_load_balance;
	else if (streq(optionname, "sched_relax_domain_level"))
		return cp->sched_relax_domain_level;
	else
		return -2;	/* optionname not recognized */
}

/* [optional] Return string value of optname */
const char *cpuset_get_sopt(UNUSED const struct cpuset *cp,
			    UNUSED const char *optionname)
{
	return NULL;		/* For now, all string options unrecognized */
}

static int read_flag(const char *filepath, char *flagp)
{
	char buf[SMALL_BUFSZ];	/* buffer a "0" or "1" flag line */
	int fd = -1;

	if ((fd = open(filepath, O_RDONLY)) < 0)
		goto err;
	if (read(fd, buf, sizeof(buf)) < 1)
		goto err;
	if (atoi(buf))
		*flagp = 1;
	else
		*flagp = 0;
	close(fd);
	return 0;
err:
	if (fd >= 0)
		close(fd);
	return -1;
}

static int load_flag(const char *path, char *flagp, const char *flag)
{
	char buf[PATH_MAX];

	pathcat2(buf, sizeof(buf), path, flag);
	return read_flag(buf, flagp);
}

static int read_number(const char *filepath, int *numberp)
{
	char buf[SMALL_BUFSZ];
	int fd = -1;

	if ((fd = open(filepath, O_RDONLY)) < 0)
		goto err;
	if (read(fd, buf, sizeof(buf)) < 1)
		goto err;
	*numberp = atoi(buf);
	close(fd);
	return 0;
err:
	if (fd >= 0)
		close(fd);
	return -1;
}

static int load_number(const char *path, int *numberp, const char *file)
{
	char buf[PATH_MAX];

	pathcat2(buf, sizeof(buf), path, file);
	return read_number(buf, numberp);
}

static int read_mask(const char *filepath, struct bitmask **bmpp, int nbits)
{
	FILE *fp = NULL;
	char *buf = NULL;
	int buflen;
	struct bitmask *bmp = NULL;

	if ((fp = fopen(filepath, "r")) == NULL)
		goto err;
	buflen = filesize(fp) + 1;	/* + 1 for nul term */
	if ((buf = malloc(buflen)) == NULL)
		goto err;
	if (flgets(buf, buflen, fp) == NULL)
		goto err;
	fclose(fp);
	fp = NULL;

	if ((bmp = bitmask_alloc(nbits)) == NULL)
		goto err;
	if (*buf && bitmask_parselist(buf, bmp) < 0)
		goto err;
	if (*bmpp)
		bitmask_free(*bmpp);
	*bmpp = bmp;
	free(buf);
	buf = NULL;
	return 0;
err:
	if (buf != NULL)
		free(buf);
	if (fp != NULL)
		fclose(fp);
	if (bmp != NULL)
		bitmask_free(bmp);
	return -1;
}

static int load_mask(const char *path, struct bitmask **bmpp,
		     int nbits, const char *mask)
{
	char buf[PATH_MAX];

	pathcat2(buf, sizeof(buf), path, mask);
	return read_mask(buf, bmpp, nbits);
}

/* Write string to file at given filepath.  Create or truncate file. */
static int write_string_file(const char *filepath, const char *str)
{
	int fd = -1;

	if ((fd = open(filepath, O_WRONLY | O_CREAT, 0644)) < 0)
		goto err;
	if (write(fd, str, strlen(str)) < 0)
		goto err;
	close(fd);
	return 0;
err:
	if (fd >= 0)
		close(fd);
	return -1;
}

/* Size and allocate buffer.  Write bitmask into it.  Caller must free */
static char *sprint_mask_buf(const struct bitmask *bmp)
{
	char *buf = NULL;
	int buflen;
	char c;

	/* First bitmask_displaylist() call just to get the length */
	buflen = bitmask_displaylist(&c, 1, bmp) + 1;	/* "+ 1" for nul */
	if ((buf = malloc(buflen)) == NULL)
		return NULL;
	bitmask_displaylist(buf, buflen, bmp);
	return buf;
}

static int exists_flag(const char *path, const char *flag)
{
	char buf[PATH_MAX];
	struct stat statbuf;
	int rc;

	pathcat2(buf, sizeof(buf), path, flag);
	rc = (stat(buf, &statbuf) == 0);
	errno = 0;
	return rc;
}

static int store_flag(const char *path, const char *flag, int val)
{
	char buf[PATH_MAX];

	pathcat2(buf, sizeof(buf), path, flag);
	return write_string_file(buf, val ? "1" : "0");
}

static int store_number(const char *path, const char *file, int val)
{
	char buf[PATH_MAX];
	char data[SMALL_BUFSZ];

	memset(data, 0, sizeof(data));
	pathcat2(buf, sizeof(buf), path, file);
	snprintf(data, sizeof(data), "%d", val);
	return write_string_file(buf, data);
}

static int store_mask(const char *path, const char *mask,
		      const struct bitmask *bmp)
{
	char maskpath[PATH_MAX];
	char *bp = NULL;
	int rc;

	if (bmp == NULL)
		return 0;
	pathcat2(maskpath, sizeof(maskpath), path, mask);
	if ((bp = sprint_mask_buf(bmp)) == NULL)
		return -1;
	rc = write_string_file(maskpath, bp);
	free(bp);
	return rc;
}

/*
 * Return 1 if 'cpu' is online, else 0 if offline.  Tests the file
 * /sys/devices/system/cpu/cpuN/online file for 0 or 1 contents
 * were N == cpu number.
 */

char cpu_online(unsigned int cpu)
{
	char online;
	char cpupath[PATH_MAX];

	(void)snprintf(cpupath, sizeof(cpupath),
		       "/sys/devices/system/cpu/cpu%d/online", cpu);
	if (read_flag(cpupath, &online) < 0)
		return 0;	/* oops - guess that cpu's not there */
	return online;
}

/*
 * The cpunodemap maps each cpu in [0 ... cpuset_cpus_nbits()),
 * to the node on which that cpu resides or cpuset_mems_nbits().
 *
 * To avoid every user having to recalculate this relation
 * from various clues in the sysfs file system (below the
 * path /sys/devices/system) a copy of this map is kept at
 * /var/run/cpunodemap.
 *
 * The system automatically cleans out files below
 * /var/run on each system reboot (see the init script
 * /etc/rc.d/boot.d/S*boot.localnet), so we don't have to worry
 * about stale data in this file across reboots.  If the file
 * is missing, let the first process that needs it, and has
 * permission to write in the /var/run directory, rebuild it.
 *
 * If using this cached data, remember the mtime of the mapfile
 * the last time we read it in case something like a hotplug
 * event results in the file being removed and rebuilt, so we
 * can detect if we're using a stale cache, and need to reload.
 *
 * The mtime of this file is set to the time when we did
 * the recalculation of the map, from the clues beneath
 * /sys/devices/system.  This is done so that a program
 * won't see the mapfile it just wrote as being newer than what
 * it just wrote out (store_map) and read the same map back in
 * (load_file).
 */

/*
 * Hold flockfile(stdin) while using cpunodemap for posix thread safety.
 *
 * Note on locking and flockfile(FILE *):
 *
 *  We use flockfile() and funlockfile() instead of directly
 *  calling pthread_mutex_lock and pthread_mutex_unlock on
 *  a pthread_mutex_t, because this avoids forcing the app
 *  to link with libpthread.  The glibc implementation of
 *  flockfile/funlockfile will fall back to no-ops if libpthread
 *  doesn't happen to be linked.
 *
 *  Since flockfile already has the moderately convoluted
 *  combination of weak and strong symbols required to accomplish
 *  this, it is easier to use flockfile() on some handy FILE *
 *  stream as a surrogate for pthread locking than it is to so
 *  re-invent that wheel.
 *
 *  Forcing all apps that use cpusets to link with libpthread
 *  would force non-transparent initialization on apps that
 *  might not be prepared to handle it.
 *
 *  The application using libcpuset should never notice this
 *  odd use of flockfile(), because we never return to the
 *  application from any libcpuset call with any such lock held.
 *  We just use this locking for guarding some non-atomic cached
 *  data updates and accesses, internal to some libcpuset calls.
 *  Also, flockfile() allows recursive nesting, so if the app
 *  calls libcpuset holding such a file lock, we won't deadlock
 *  if we go to acquire the same lock.  We'll just get the lock
 *  and increment its counter while we hold it.
 */

static struct cpunodemap {
	int *map;		/* map[cpumask_sz]: maps cpu to its node */
	time_t mtime;		/* modtime of mapfile when last read */
} cpunodemap;

/*
 * rebuild_map() - Rebuild cpunodemap[] from scratch.
 *
 * Situation:
 *	Neither our in-memory cpunodemap[] array nor the
 *	cache of it in mapfile is current.
 * Action:
 *	Rebuild it from first principles and the information
 *	available below /sys/devices/system.
 */

static void rebuild_map(void)
{
	char buf[PATH_MAX];
	DIR *dir1, *dir2;
	struct dirent *dent1, *dent2;
	int ncpus = cpuset_cpus_nbits();
	int nmems = cpuset_mems_nbits();
	unsigned int cpu, mem;

	for (cpu = 0; cpu < (unsigned int)ncpus; cpu++)
		cpunodemap.map[cpu] = -1;
	pathcat2(buf, sizeof(buf), sysdevices, "node");
	if ((dir1 = opendir(buf)) == NULL)
		return;
	while ((dent1 = readdir(dir1)) != NULL) {
		if (sscanf(dent1->d_name, "node%u", &mem) < 1)
			continue;
		pathcat3(buf, sizeof(buf), sysdevices, "node", dent1->d_name);
		if ((dir2 = opendir(buf)) == NULL)
			continue;
		while ((dent2 = readdir(dir2)) != NULL) {
			if (sscanf(dent2->d_name, "cpu%u", &cpu) < 1)
				continue;
			if (cpu >= (unsigned int)ncpus
			    || mem >= (unsigned int)nmems)
				continue;
			cpunodemap.map[cpu] = mem;
		}
		closedir(dir2);
	}
	closedir(dir1);
	cpunodemap.mtime = time(0);
}

/*
 * load_map() - Load cpunodemap[] from mapfile.
 *
 * Situation:
 *	The cpunodemap in mapfile is more recent than
 *	what we have in the cpunodemap[] array.
 * Action:
 *	Reload the cpunodemap[] array from the file.
 */

static void load_map(void)
{
	char buf[SMALL_BUFSZ];	/* buffer 1 line of mapfile */
	FILE *mapfp;		/* File stream on mapfile */
	int ncpus = cpuset_cpus_nbits();
	int nmems = cpuset_mems_nbits();
	unsigned int cpu, mem;

	if ((cpunodemap.map = calloc(ncpus, sizeof(int))) == NULL)
		return;
	cpunodemap.mtime = get_mtime(mapfile);
	if ((mapfp = fopen(mapfile, "r")) == NULL)
		return;
	for (cpu = 0; cpu < (unsigned int)ncpus; cpu++)
		cpunodemap.map[cpu] = nmems;
	while (flgets(buf, sizeof(buf), mapfp) != NULL) {
		if (sscanf(buf, "%u %u", &cpu, &mem) < 2)
			continue;
		if (cpu >= (unsigned int)ncpus || mem >= (unsigned int)nmems)
			continue;
		cpunodemap.map[cpu] = mem;
	}
	fclose(mapfp);
}

/*
 * store_map() - Write cpunodemap[] out to mapfile.
 *
 * Situation:
 *	The cpunodemap in the cpunodemap[] array is
 *	more recent than the one in mapfile.
 * Action:
 *	Write cpunodemap[] out to mapfile.
 */

static void store_map(void)
{
	char buf[PATH_MAX];
	int fd = -1;
	FILE *mapfp = NULL;
	int ncpus = cpuset_cpus_nbits();
	int nmems = cpuset_mems_nbits();
	unsigned int cpu, mem;

	snprintf(buf, sizeof(buf), "%s.%s", mapfile, "XXXXXX");
	if ((fd = mkstemp(buf)) < 0)
		goto err;
	if ((mapfp = fdopen(fd, "w")) == NULL)
		goto err;
	for (cpu = 0; cpu < (unsigned int)ncpus; cpu++) {
		mem = cpunodemap.map[cpu];
		if (mem < (unsigned int)nmems)
			fprintf(mapfp, "%u %u\n", cpu, mem);
	}
	fclose(mapfp);
	set_mtime(buf, cpunodemap.mtime);
	if (rename(buf, mapfile) < 0)
		goto err;
	/* mkstemp() creates mode 0600 - change to world readable */
	(void)chmod(mapfile, 0444);
	return;
err:
	if (mapfp != NULL) {
		fclose(mapfp);
		fd = -1;
	}
	if (fd >= 0)
		close(fd);
	(void)unlink(buf);
}

/*
 * Load and gain thread safe access to the <cpu, node> map.
 *
 * Return 0 on success with flockfile(stdin) held.
 * Each successful get_map() call must be matched with a
 * following put_map() call to release the lock.
 *
 * On error, return -1 with errno set and no lock held.
 */

static int get_map(void)
{
	time_t file_mtime;

	flockfile(stdin);

	if (cpunodemap.map == NULL) {
		cpunodemap.map = calloc(cpuset_cpus_nbits(), sizeof(int));
		if (cpunodemap.map == NULL)
			goto err;
	}

	/* If no one has a good cpunodemap, rebuild from scratch */
	file_mtime = get_mtime(mapfile);
	if (cpunodemap.mtime == 0 && file_mtime == 0)
		rebuild_map();

	/* If either cpunodemap[] or mapfile newer, update other with it */
	file_mtime = get_mtime(mapfile);
	if (cpunodemap.mtime < file_mtime)
		load_map();
	else if (cpunodemap.mtime > file_mtime)
		store_map();
	return 0;
err:
	funlockfile(stdin);
	return -1;
}

static void put_map(void)
{
	funlockfile(stdin);
}

/* Set cpus to those local to Memory Nodes mems */
int cpuset_localcpus(const struct bitmask *mems, struct bitmask *cpus)
{
	int ncpus = cpuset_cpus_nbits();
	unsigned int cpu;

	if (check() < 0)
		return -1;

	get_map();
	bitmask_clearall(cpus);
	for (cpu = 0; cpu < (unsigned int)ncpus; cpu++) {
		if (bitmask_isbitset(mems, cpunodemap.map[cpu]))
			bitmask_setbit(cpus, cpu);
	}
	put_map();
	return 0;
}

/* Set mems to those local to CPUs cpus */
int cpuset_localmems(const struct bitmask *cpus, struct bitmask *mems)
{
	int ncpus = cpuset_cpus_nbits();
	unsigned int cpu;

	if (check() < 0)
		return -1;

	get_map();
	bitmask_clearall(mems);
	for (cpu = 0; cpu < (unsigned int)ncpus; cpu++) {
		if (bitmask_isbitset(cpus, cpu))
			bitmask_setbit(mems, cpunodemap.map[cpu]);
	}
	put_map();
	return 0;
}

/*
 * distmap[]
 *
 * Array of ints of size cpumask_sz by nodemask_sz.
 *
 * Element distmap[cpu][mem] is the distance between CPU cpu
 * and Memory Node mem.  Distances are weighted to roughly
 * approximate the cost of memory references, and scaled so that
 * the distance from a CPU to its local Memory Node is ten (10).
 *
 * The first call to cpuset_cpumemdist() builds this map, from
 * whatever means the kernel provides to obtain these distances.
 *
 * These distances derive from ACPI SLIT table entries, which are
 * eight bits in size.
 *
 * Hold flockfile(stdout) while using distmap for posix thread safety.
 */

typedef unsigned char distmap_entry_t;	/* type of distmap[] entries */

static distmap_entry_t *distmap;	/* maps <cpu, mem> to distance */

#define DISTMAP_MAX UCHAR_MAX	/* maximum value in distmap[] */

#define I(i,j) ((i) * nmems + (j))	/* 2-D array index simulation */

/*
 * Parse arch neutral lines from 'distance' files of form:
 *
 *	46 66 10 20
 *
 * The lines contain a space separated list of distances, which is parsed
 * into array dists[] of each nodes distance from the specified node.
 *
 * Result is placed in distmap[ncpus][nmems]:
 *
 *	For each cpu c on node:
 *		For each node position n in list of distances:
 *			distmap[c][n] = dists[n]
 */

static int parse_distmap_line(unsigned int node, char *buf)
{
	char *p, *q;
	int ncpus = cpuset_cpus_nbits();
	int nmems = cpuset_mems_nbits();
	unsigned int c, n;
	distmap_entry_t *dists = NULL;
	struct bitmask *cpus = NULL, *mems = NULL;
	int ret = -1;

	p = buf;
	if ((dists = calloc(nmems, sizeof(*dists))) == NULL)
		goto err;
	for (n = 0; n < (unsigned int)nmems; n++)
		dists[n] = DISTMAP_MAX;

	for (n = 0; n < (unsigned int)nmems && *p; n++, p = q) {
		unsigned int d;

		if ((p = strpbrk(p, "0123456789")) == NULL)
			break;
		d = strtoul(p, &q, 10);
		if (p == q)
			break;
		if (d < DISTMAP_MAX)
			dists[n] = (distmap_entry_t) d;
	}

	if ((mems = bitmask_alloc(nmems)) == NULL)
		goto err;
	bitmask_setbit(mems, node);

	if ((cpus = bitmask_alloc(ncpus)) == NULL)
		goto err;
	cpuset_localcpus(mems, cpus);

	for (c = bitmask_first(cpus); c < (unsigned int)ncpus;
	     c = bitmask_next(cpus, c + 1))
		for (n = 0; n < (unsigned int)nmems; n++)
			distmap[I(c, n)] = dists[n];
	ret = 0;
	/* fall into ... */
err:
	bitmask_free(mems);
	bitmask_free(cpus);
	free(dists);
	return ret;
}

static int parse_distance_file(unsigned int node, const char *path)
{
	FILE *fp;
	char *buf = NULL;
	int buflen;

	if ((fp = fopen(path, "r")) == NULL)
		goto err;

	buflen = filesize(fp);

	if ((buf = malloc(buflen)) == NULL)
		goto err;

	if (flgets(buf, buflen, fp) == NULL)
		goto err;

	if (parse_distmap_line(node, buf) < 0)
		goto err;

	free(buf);
	fclose(fp);
	return 0;
err:
	free(buf);
	if (fp)
		fclose(fp);
	return -1;
}

static void build_distmap(void)
{
	static int tried_before = 0;
	int ncpus = cpuset_cpus_nbits();
	int nmems = cpuset_mems_nbits();
	int c, m;
	DIR *dir = NULL;
	struct dirent *dent;

	if (tried_before)
		goto err;
	tried_before = 1;

	if ((distmap = calloc(ncpus * nmems, sizeof(*distmap))) == NULL)
		goto err;

	for (c = 0; c < ncpus; c++)
		for (m = 0; m < nmems; m++)
			distmap[I(c, m)] = DISTMAP_MAX;

	if ((dir = opendir(distance_directory)) == NULL)
		goto err;
	while ((dent = readdir(dir)) != NULL) {
		char buf[PATH_MAX];
		unsigned int node;

		if (sscanf(dent->d_name, "node%u", &node) < 1)
			continue;
		pathcat3(buf, sizeof(buf), distance_directory, dent->d_name,
			 "distance");
		if (parse_distance_file(node, buf) < 0)
			goto err;
	}
	closedir(dir);
	return;
err:
	if (dir)
		closedir(dir);
	free(distmap);
	distmap = NULL;
}

#ifdef ALTERNATE_SN_DISTMAP

/*
 * Parse SN architecture specific line of form:
 *
 *	node 3 001c14#1 local asic SHub_1.1, nasid 0x6, dist 66:46:20:10
 *
 * Second field is node number.  The "dist" field is the colon separated list
 * of distances, which is parsed into array dists[] of each nodes distance
 * from that node.
 *
 * Result is placed in distmap[ncpus][nmems]:
 *
 *	For each cpu c on that node:
 *		For each node position n in list of distances:
 *			distmap[c][n] = dists[n]
 */

static void parse_distmap_line_sn(char *buf)
{
	char *p, *pend, *q;
	int ncpus = cpuset_cpus_nbits();
	int nmems = cpuset_mems_nbits();
	unsigned long c, n, node;
	distmap_entry_t *dists = NULL;
	struct bitmask *cpus = NULL, *mems = NULL;

	if ((p = strchr(buf, ' ')) == NULL)
		goto err;
	if ((node = strtoul(p, &q, 10)) >= (unsigned int)nmems)
		goto err;
	if ((p = strstr(q, " dist ")) == NULL)
		goto err;
	p += strlen(" dist ");
	if ((pend = strchr(p, ' ')) != NULL)
		*pend = '\0';
	if ((dists = calloc(nmems, sizeof(*dists))) == NULL)
		goto err;
	for (n = 0; n < (unsigned int)nmems; n++)
		dists[n] = DISTMAP_MAX;

	for (n = 0; n < (unsigned int)nmems && *p; n++, p = q) {
		unsigned long d;

		if ((p = strpbrk(p, "0123456789")) == NULL)
			break;
		d = strtoul(p, &q, 10);
		if (p == q)
			break;
		if (d < DISTMAP_MAX)
			dists[n] = (distmap_entry_t) d;
	}

	if ((mems = bitmask_alloc(nmems)) == NULL)
		goto err;
	bitmask_setbit(mems, node);

	if ((cpus = bitmask_alloc(ncpus)) == NULL)
		goto err;
	cpuset_localcpus(mems, cpus);

	for (c = bitmask_first(cpus); c < (unsigned int)ncpus;
	     c = bitmask_next(cpus, c + 1))
		for (n = 0; n < (unsigned int)nmems; n++)
			distmap[I(c, n)] = dists[n];
	/* fall into ... */
err:
	bitmask_free(mems);
	bitmask_free(cpus);
	free(dists);
}

static void build_distmap_sn(void)
{
	int ncpus = cpuset_cpus_nbits();
	int nmems = cpuset_mems_nbits();
	int c, m;
	static int tried_before = 0;
	FILE *fp = NULL;
	char *buf = NULL;
	int buflen;

	if (tried_before)
		goto err;
	tried_before = 1;

	if ((fp = fopen(sn_topology, "r")) == NULL)
		goto err;

	if ((distmap = calloc(ncpus * nmems, sizeof(*distmap))) == NULL)
		goto err;

	for (c = 0; c < ncpus; c++)
		for (m = 0; m < nmems; m++)
			distmap[I(c, m)] = DISTMAP_MAX;

	buflen = filesize(fp);
	if ((buf = malloc(buflen)) == NULL)
		goto err;

	while (flgets(buf, buflen, fp) != NULL)
		if (strprefix(buf, sn_top_node_prefix))
			parse_distmap_line_sn(buf);

	free(buf);
	fclose(fp);
	return;
err:
	free(buf);
	free(distmap);
	distmap = NULL;
	if (fp)
		fclose(fp);
}

#endif

/* [optional] Hardware distance from CPU to Memory Node */
unsigned int cpuset_cpumemdist(int cpu, int mem)
{
	int ncpus = cpuset_cpus_nbits();
	int nmems = cpuset_mems_nbits();
	distmap_entry_t r = DISTMAP_MAX;

	flockfile(stdout);

	if (check() < 0)
		goto err;

	if (distmap == NULL)
		build_distmap();

#ifdef ALTERNATE_SN_DISTMAP
	if (distmap == NULL)
		build_distmap_sn();
#endif

	if (distmap == NULL)
		goto err;

	if (cpu < 0 || cpu >= ncpus || mem < 0 || mem >= nmems)
		goto err;

	r = distmap[I(cpu, mem)];
	/* fall into ... */
err:
	funlockfile(stdout);
	return r;
}

/* [optional] Return Memory Node closest to cpu */
int cpuset_cpu2node(int cpu)
{
	int ncpus = cpuset_cpus_nbits();
	int nmems = cpuset_mems_nbits();
	struct bitmask *cpus = NULL, *mems = NULL;
	int r = -1;

	if (check() < 0)
		goto err;

	if ((cpus = bitmask_alloc(ncpus)) == NULL)
		goto err;
	bitmask_setbit(cpus, cpu);

	if ((mems = bitmask_alloc(nmems)) == NULL)
		goto err;
	cpuset_localmems(cpus, mems);
	r = bitmask_first(mems);
	/* fall into ... */
err:
	bitmask_free(cpus);
	bitmask_free(mems);
	return r;
}

static int apply_cpuset_settings(const char *path, const struct cpuset *cp)
{
	if (cp->cpu_exclusive_valid && cp->cpu_exclusive_dirty) {
		if (store_flag(path, "cpu_exclusive", cp->cpu_exclusive) < 0)
			goto err;
	}

	if (cp->mem_exclusive_valid && cp->mem_exclusive_dirty) {
		if (store_flag(path, "mem_exclusive", cp->mem_exclusive) < 0)
			goto err;
	}

	if (cp->mem_hardwall_valid && cp->mem_hardwall_dirty) {
		if (store_flag(path, "mem_hardwall", cp->mem_hardwall) < 0)
			goto err;
	}

	if (cp->notify_on_release_valid && cp->notify_on_release_dirty) {
		if (store_flag(path, "notify_on_release", cp->notify_on_release)
		    < 0)
			goto err;
	}

	if (cp->memory_migrate_valid &&
	    cp->memory_migrate_dirty && exists_flag(path, "memory_migrate")) {
		if (store_flag(path, "memory_migrate", cp->memory_migrate) < 0)
			goto err;
	}

	if (cp->memory_pressure_enabled_valid &&
	    cp->memory_pressure_enabled_dirty &&
	    exists_flag(path, "memory_pressure_enabled")) {
		if (store_flag
		    (path, "memory_pressure_enabled",
		     cp->memory_pressure_enabled) < 0)
			goto err;
	}

	if (cp->memory_spread_page_valid &&
	    cp->memory_spread_page_dirty &&
	    exists_flag(path, "memory_spread_page")) {
		if (store_flag
		    (path, "memory_spread_page", cp->memory_spread_page) < 0)
			goto err;
	}

	if (cp->memory_spread_slab_valid &&
	    cp->memory_spread_slab_dirty &&
	    exists_flag(path, "memory_spread_slab")) {
		if (store_flag
		    (path, "memory_spread_slab", cp->memory_spread_slab) < 0)
			goto err;
	}

	if (cp->sched_load_balance_valid &&
	    cp->sched_load_balance_dirty &&
	    exists_flag(path, "sched_load_balance")) {
		if (store_flag
		    (path, "sched_load_balance", cp->sched_load_balance) < 0)
			goto err;
	}

	if (cp->sched_relax_domain_level_valid &&
	    cp->sched_relax_domain_level_dirty &&
	    exists_flag(path, "sched_relax_domain_level")) {
		if (store_number
		    (path, "sched_relax_domain_level",
		     cp->sched_relax_domain_level) < 0)
			goto err;
	}

	if (cp->cpus_valid && cp->cpus_dirty) {
		if (store_mask(path, "cpus", cp->cpus) < 0)
			goto err;
	}

	if (cp->mems_valid && cp->mems_dirty) {
		if (store_mask(path, "mems", cp->mems) < 0)
			goto err;
	}
	return 0;
err:
	return -1;
}

/*
 * get_siblings() - helper routine for cpuset_would_crash_kernel(), below.
 *
 * Extract max value of any 'siblings' field in /proc/cpuinfo.
 * Cache the result - only need to extract once in lifetime of task.
 *
 * The siblings field is the number of logical CPUs in a physical
 * processor package.  It is equal to the product of the number of
 * cores in that package, times the number of hyper-threads per core.
 * The bug that cpuset_would_crash_kernel() is detecting arises
 * when a cpu_exclusive cpuset tries to include just some, not all,
 * of the sibling logical CPUs available in a processor package.
 *
 * In the improbable case that a system has mixed values of siblings
 * (some processor packages have more than others, perhaps due to
 * partially enabling Hyper-Threading), we take the worse case value,
 * the largest siblings value.  This might be overkill.  I don't know
 * if this kernel bug considers each processor package's siblings
 * separately or not.  But it sure is easier this way ...
 *
 * This routine takes about 0.7 msecs on a 4 CPU 2.8 MHz Xeon, from
 * open to close, the first time called.
 */

static int get_siblings(void)
{
	static int siblings;
	char buf[32];		/* big enough for one 'siblings' line */
	FILE *fp;

	if (siblings)
		return siblings;

	if ((fp = fopen("/proc/cpuinfo", "r")) == NULL)
		return 4;	/* wing it - /proc not mounted ? */
	while (flgets(buf, sizeof(buf), fp) != NULL) {
		int s;

		if (sscanf(buf, "siblings : %d", &s) < 1)
			continue;
		if (s > siblings)
			siblings = s;
	}
	fclose(fp);
	if (siblings == 0)
		siblings = 1;	/* old kernel, no siblings, default to 1 */
	return siblings;
}

/*
 * Some 2.6.16 and 2.6.17 kernel versions have a bug in the dynamic
 * scheduler domain code invoked for cpu_exclusive cpusets that causes
 * the kernel to freeze, requiring a hardware reset.
 *
 * On kernels built with CONFIG_SCHED_MC enabled, if a 'cpu_exclusive'
 * cpuset is defined where that cpusets 'cpus' are not on package
 * boundaries then the kernel will freeze, usually as soon as this
 * cpuset is created, requiring a hardware reset.
 *
 * A cpusets 'cpus' are not on package boundaries if the cpuset
 * includes a proper non-empty subset (some, but not all) of the
 * logical cpus on a processor package.  This requires multiple
 * logical CPUs per package, available with either Hyper-Thread or
 * Multi-Core support.  Without one of these features, there is only
 * one logical CPU per physical package, and it's not possible to
 * have a proper, non-empty subset of a set of cardinality one.
 *
 * SUSE SLES10 kernels, as first released, only enable CONFIG_SCHED_MC
 * on i386 and x86_64 arch's.
 *
 * The objective of this routine cpuset_would_crash_kernel() is to
 * determine if a proposed cpuset setting would crash the kernel due
 * to this bug, so that the caller can avoid the crash.
 *
 * Ideally we'd check for exactly these conditions here, but computing
 * the package (identified by the 'physical id' field of /proc/cpuinfo)
 * of each cpu in a cpuset is more effort than it's worth here.
 *
 * Also there is no obvious way to identify exactly whether the kernel
 * one is executing on has this bug, short of trying it, and seeing
 * if the kernel just crashed.
 *
 * So for now, we look for a simpler set of conditions, that meets
 * our immediate need - avoid this crash on SUSE SLES10 systems that
 * are susceptible to it.  We look for the kernel version 2.6.16.*,
 * which is the base kernel of SUSE SLES10, and for i386 or x86_64
 * processors, which had CONFIG_SCHED_MC enabled.
 *
 * If these simpler conditions are met, we further simplify the check,
 * by presuming that the logical CPUs are numbered on processor
 * package boundaries.  If each package has S siblings, we assume
 * that CPUs numbered N through N + S -1 are on the same package,
 * for any CPU N such that N mod S == 0.
 *
 * Yes, this is a hack, focused on avoiding kernel freezes on
 * susceptible SUSE SLES10 systems.
 */

static int cpuset_would_crash_kernel(const struct cpuset *cp)
{
	static int susceptible_system = -1;

	if (!cp->cpu_exclusive)
		goto ok;

	if (susceptible_system == -1) {
		struct utsname u;
		int rel_2_6_16, arch_i386, arch_x86_64;

		if (uname(&u) < 0)
			goto fail;
		rel_2_6_16 = strprefix(u.release, "2.6.16.");
		arch_i386 = streq(u.machine, "i386");
		arch_x86_64 = streq(u.machine, "x86_64");
		susceptible_system = rel_2_6_16 && (arch_i386 || arch_x86_64);
	}

	if (susceptible_system) {
		int ncpus = cpuset_cpus_nbits();
		int siblings = get_siblings();
		unsigned int cpu;

		for (cpu = 0; cpu < (unsigned int)ncpus; cpu += siblings) {
			int s, num_set = 0;

			for (s = 0; s < siblings; s++) {
				if (bitmask_isbitset(cp->cpus, cpu + s))
					num_set++;
			}

			/* If none or all siblings set, we're still ok */
			if (num_set == 0 || num_set == siblings)
				continue;

			/* Found one that would crash kernel.  Fail.  */
			errno = ENXIO;
			goto fail;
		}
	}
	/* If not susceptible, or if all ok, fall into "ok" ... */
ok:
	return 0;		/* would not crash */
fail:
	return 1;		/* would crash */
}

/* compare two cpuset and mark the dirty variable */
static void mark_dirty_variable(struct cpuset *cp1, const struct cpuset *cp2)
{
	if (cp1->cpu_exclusive_valid &&
	    cp1->cpu_exclusive != cp2->cpu_exclusive)
		cp1->cpu_exclusive_dirty = 1;

	if (cp1->mem_exclusive_valid &&
	    cp1->mem_exclusive != cp2->mem_exclusive)
		cp1->mem_exclusive_dirty = 1;

	if (cp1->mem_hardwall_valid && cp1->mem_hardwall != cp2->mem_hardwall)
		cp1->mem_hardwall_dirty = 1;

	if (cp1->notify_on_release_valid &&
	    cp1->notify_on_release != cp2->notify_on_release)
		cp1->notify_on_release_dirty = 1;

	if (cp1->memory_migrate_valid &&
	    cp1->memory_migrate != cp2->memory_migrate)
		cp1->memory_migrate_dirty = 1;

	if (cp1->memory_pressure_enabled_valid &&
	    cp1->memory_pressure_enabled != cp2->memory_pressure_enabled)
		cp1->memory_pressure_enabled_dirty = 1;

	if (cp1->memory_spread_page_valid &&
	    cp1->memory_spread_page != cp2->memory_spread_page)
		cp1->memory_spread_page_dirty = 1;

	if (cp1->memory_spread_slab_valid &&
	    cp1->memory_spread_slab != cp2->memory_spread_slab)
		cp1->memory_spread_slab_dirty = 1;

	if (cp1->sched_load_balance_valid &&
	    cp1->sched_load_balance != cp2->sched_load_balance)
		cp1->sched_load_balance_dirty = 1;

	if (cp1->sched_relax_domain_level_valid &&
	    cp1->sched_relax_domain_level != cp2->sched_relax_domain_level)
		cp1->sched_relax_domain_level_dirty = 1;

	if (cp1->cpus_valid && !bitmask_equal(cp1->cpus, cp2->cpus))
		cp1->cpus_dirty = 1;
	if (cp1->mems_valid && !bitmask_equal(cp1->mems, cp2->mems))
		cp1->mems_dirty = 1;
}

/* Create (if new set) or modify cpuset 'cp' at location 'relpath' */
static int cr_or_mod(const char *relpath, const struct cpuset *cp, int new)
{
	char buf[PATH_MAX];
	int do_rmdir_on_err = 0;
	int do_restore_cp_sav_on_err = 0;
	struct cpuset *cp_sav = NULL;
	int sav_errno;

	if (check() < 0)
		goto err;

	if (cpuset_would_crash_kernel(cp))
		goto err;

	fullpath(buf, sizeof(buf), relpath);

	if (new) {
		if (mkdir(buf, 0755) < 0)
			goto err;
		/* we made it, so we should remove it on error */
		do_rmdir_on_err = 1;
	}

	if ((cp_sav = cpuset_alloc()) == NULL)
		goto err;
	if (cpuset_query(cp_sav, relpath) < 0)
		goto err;
	/* we have old settings to restore on error */
	do_restore_cp_sav_on_err = 1;

	/* check which variable need to restore on error */
	mark_dirty_variable(cp_sav, cp);

	if (apply_cpuset_settings(buf, cp) < 0)
		goto err;

	cpuset_free(cp_sav);
	return 0;
err:
	sav_errno = errno;
	if (do_restore_cp_sav_on_err)
		(void)apply_cpuset_settings(buf, cp_sav);
	if (cp_sav)
		cpuset_free(cp_sav);
	if (do_rmdir_on_err)
		(void)rmdir(buf);
	errno = sav_errno;
	return -1;
}

/* Create cpuset 'cp' at location 'relpath' */
int cpuset_create(const char *relpath, const struct cpuset *cp)
{
	return cr_or_mod(relpath, cp, 1);
}

/* Delete cpuset at location 'path' (if empty) */
int cpuset_delete(const char *relpath)
{
	char buf[PATH_MAX];

	if (check() < 0)
		goto err;

	fullpath(buf, sizeof(buf), relpath);
	if (rmdir(buf) < 0)
		goto err;

	return 0;
err:
	return -1;
}

/* Set cpuset cp to the cpuset at location 'path' */
int cpuset_query(struct cpuset *cp, const char *relpath)
{
	char buf[PATH_MAX];

	if (check() < 0)
		goto err;

	fullpath(buf, sizeof(buf), relpath);

	if (load_flag(buf, &cp->cpu_exclusive, "cpu_exclusive") < 0)
		goto err;
	cp->cpu_exclusive_valid = 1;

	if (load_flag(buf, &cp->mem_exclusive, "mem_exclusive") < 0)
		goto err;
	cp->mem_exclusive_valid = 1;

	if (load_flag(buf, &cp->notify_on_release, "notify_on_release") < 0)
		goto err;
	cp->notify_on_release_valid = 1;

	if (exists_flag(buf, "memory_migrate")) {
		if (load_flag(buf, &cp->memory_migrate, "memory_migrate") < 0)
			goto err;
		cp->memory_migrate_valid = 1;
	}

	if (exists_flag(buf, "mem_hardwall")) {
		if (load_flag(buf, &cp->mem_hardwall, "mem_hardwall") < 0)
			goto err;
		cp->mem_hardwall_valid = 1;
	}

	if (exists_flag(buf, "memory_pressure_enabled")) {
		if (load_flag
		    (buf, &cp->memory_pressure_enabled,
		     "memory_pressure_enabled") < 0)
			goto err;
		cp->memory_pressure_enabled_valid = 1;
	}

	if (exists_flag(buf, "memory_spread_page")) {
		if (load_flag
		    (buf, &cp->memory_spread_page, "memory_spread_page") < 0)
			goto err;
		cp->memory_spread_page_valid = 1;
	}

	if (exists_flag(buf, "memory_spread_slab")) {
		if (load_flag
		    (buf, &cp->memory_spread_slab, "memory_spread_slab") < 0)
			goto err;
		cp->memory_spread_slab_valid = 1;
	}

	if (exists_flag(buf, "sched_load_balance")) {
		if (load_flag
		    (buf, &cp->sched_load_balance, "sched_load_balance") < 0)
			goto err;
		cp->sched_load_balance_valid = 1;
	}

	if (exists_flag(buf, "sched_relax_domain_level")) {
		if (load_number
		    (buf, &cp->sched_relax_domain_level,
		     "sched_relax_domain_level") < 0)
			goto err;
		cp->sched_relax_domain_level_valid = 1;
	}

	if (load_mask(buf, &cp->cpus, cpuset_cpus_nbits(), "cpus") < 0)
		goto err;
	cp->cpus_valid = 1;

	if (load_mask(buf, &cp->mems, cpuset_mems_nbits(), "mems") < 0)
		goto err;
	cp->mems_valid = 1;

	return 0;
err:
	return -1;
}

/* Modify cpuset at location 'relpath' to values of 'cp' */
int cpuset_modify(const char *relpath, const struct cpuset *cp)
{
	return cr_or_mod(relpath, cp, 0);
}

/* Get cpuset path of pid into buf */
char *cpuset_getcpusetpath(pid_t pid, char *buf, size_t size)
{
	int fd;			/* dual use: cpuset file for pid and self */
	int rc;			/* dual use: snprintf and read return codes */

	if (check() < 0)
		return NULL;

	/* borrow result buf[] to build cpuset file path */
	if (pid == 0)
		rc = snprintf(buf, size, "/proc/self/cpuset");
	else
		rc = snprintf(buf, size, "/proc/%d/cpuset", pid);
	if (rc >= (int)size) {
		errno = E2BIG;
		return NULL;
	}
	if ((fd = open(buf, O_RDONLY)) < 0) {
		int e = errno;
		if (e == ENOENT)
			e = ESRCH;
		if ((fd = open("/proc/self/cpuset", O_RDONLY)) < 0)
			e = ENOSYS;
		else
			close(fd);
		errno = e;
		return NULL;
	}
	rc = read(fd, buf, size);
	close(fd);
	if (rc < 0)
		return NULL;
	if (rc >= (int)size) {
		errno = E2BIG;
		return NULL;
	}
	buf[rc] = 0;
	chomp(buf);
	return buf;

}

/* Get cpuset 'cp' of pid */
int cpuset_cpusetofpid(struct cpuset *cp, pid_t pid)
{
	char buf[PATH_MAX];

	if (cpuset_getcpusetpath(pid, buf, sizeof(buf)) == NULL)
		return -1;
	if (cpuset_query(cp, buf) < 0)
		return -1;
	return 0;
}

/* [optional] Return mountpoint of cpuset filesystem */
const char *cpuset_mountpoint(void)
{
	if (check() < 0) {
		switch (errno) {
		case ENODEV:
			return "[cpuset filesystem not mounted]";
		default:
			return "[cpuset filesystem not supported]";
		}
	}
	return cpusetmnt;
}

/* Return true if path is a directory. */
static int isdir(const char *path)
{
	struct stat statbuf;

	if (stat(path, &statbuf) < 0)
		return 0;
	return S_ISDIR(statbuf.st_mode);
}

/*
 * [optional] cpuset_collides_exclusive() - True if would collide exclusive.
 *
 * Return true iff the specified cpuset would overlap with any
 * sibling cpusets in either cpus or mems, where either this
 * cpuset or the sibling is cpu_exclusive or mem_exclusive.
 *
 * cpuset_create() fails with errno == EINVAL if the requested cpuset
 * would overlap with any sibling, where either one is cpu_exclusive or
 * mem_exclusive.  This is a common, and not obvious error.  The
 * following routine checks for this particular case, so that code
 * creating cpusets can better identify the situation, perhaps to issue
 * a more informative error message.
 *
 * Can also be used to diagnose cpuset_modify failures.  This
 * routine ignores any existing cpuset with the same path as the
 * given 'cpusetpath', and only looks for exclusive collisions with
 * sibling cpusets of that path.
 *
 * In case of any error, returns (0) -- does not collide.  Presumably
 * any actual attempt to create or modify a cpuset will encounter the
 * same error, and report it usefully.
 *
 * This routine is not particularly efficient; most likely code creating or
 * modifying a cpuset will want to try the operation first, and then if that
 * fails with errno EINVAL, perhaps call this routine to determine if an
 * exclusive cpuset collision caused the error.
 */

int cpuset_collides_exclusive(const char *cpusetpath, const struct cpuset *cp1)
{
	char parent[PATH_MAX];
	char *p;
	char *pathcopy = NULL;
	char *base;
	DIR *dir = NULL;
	struct dirent *dent;
	struct cpuset *cp2 = NULL;
	struct bitmask *cpus1 = NULL, *cpus2 = NULL;
	struct bitmask *mems1 = NULL, *mems2 = NULL;
	int ret;

	if (check() < 0)
		goto err;

	fullpath(parent, sizeof(parent), cpusetpath);
	if (streq(parent, cpusetmnt))
		goto err;	/* only one cpuset root - can't collide */
	pathcopy = strdup(parent);
	p = strrchr(parent, '/');
	if (!p)
		goto err;	/* huh? - impossible - run and hide */
	*p = 0;			/* now parent is dirname of fullpath */

	p = strrchr(pathcopy, '/');
	base = p + 1;		/* now base is basename of fullpath */
	if (!*base)
		goto err;	/* this is also impossible - run away */

	if ((dir = opendir(parent)) == NULL)
		goto err;
	if ((cp2 = cpuset_alloc()) == NULL)
		goto err;
	if ((cpus1 = bitmask_alloc(cpuset_cpus_nbits())) == NULL)
		goto err;
	if ((cpus2 = bitmask_alloc(cpuset_cpus_nbits())) == NULL)
		goto err;
	if ((mems1 = bitmask_alloc(cpuset_mems_nbits())) == NULL)
		goto err;
	if ((mems2 = bitmask_alloc(cpuset_mems_nbits())) == NULL)
		goto err;

	while ((dent = readdir(dir)) != NULL) {
		char child[PATH_MAX];

		if (streq(dent->d_name, ".") || streq(dent->d_name, ".."))
			continue;
		if (streq(dent->d_name, base))
			continue;
		pathcat2(child, sizeof(child), parent, dent->d_name);
		if (!isdir(child))
			continue;
		if (cpuset_query(cp2, child + strlen(cpusetmnt)) < 0)
			goto err;
		if (cp1->cpu_exclusive || cp2->cpu_exclusive) {
			cpuset_getcpus(cp1, cpus1);
			cpuset_getcpus(cp2, cpus2);
			if (bitmask_intersects(cpus1, cpus2))
				goto collides;
		}
		if (cp1->mem_exclusive || cp2->mem_exclusive) {
			cpuset_getmems(cp1, mems1);
			cpuset_getmems(cp2, mems2);
			if (bitmask_intersects(mems1, mems2))
				goto collides;
		}
	}
err:
	/* error, or did not collide */
	ret = 0;
	goto done;
collides:
	/* collides */
	ret = 1;
	/* fall into ... */
done:
	if (dir)
		closedir(dir);
	cpuset_free(cp2);
	free(pathcopy);
	bitmask_free(cpus1);
	bitmask_free(cpus2);
	bitmask_free(mems1);
	bitmask_free(mems2);
	return ret;
}

/*
 * [optional] cpuset_nuke() - Remove cpuset anyway possible
 *
 * Remove a cpuset, including killing tasks in it, and
 * removing any descendent cpusets and killing their tasks.
 *
 * Tasks can take a long time (minutes on some configurations)
 * to exit.  Loop up to 'seconds' seconds, trying to kill them.
 *
 * How we do it:
 *	1) First, kill all the pids, looping until there are
 *	   no more pids in this cpuset or below, or until the
 *	   'seconds' timeout limit is exceeded.
 *	2) Then depth first recursively rmdir the cpuset directories.
 *	3) If by this point the original cpuset is gone, we succeeded.
 *
 * If the timeout is exceeded, and tasks still exist, fail with
 * errno == ETIME.
 *
 * We sleep a variable amount of time.  After the first attempt to
 * kill all the tasks in the cpuset or its descendents, we sleep 1
 * second, the next time 2 seconds, increasing 1 second each loop
 * up to a max of 10 seconds.  If more loops past 10 are required
 * to kill all the tasks, we sleep 10 seconds each subsequent loop.
 * In any case, before the last loop, we sleep however many seconds
 * remain of the original timeout 'seconds' requested.  The total
 * time of all sleeps will be no more than the requested 'seconds'.
 *
 * If the cpuset started out empty of any tasks, or if the passed in
 * 'seconds' was zero, then this routine will return quickly, having
 * not slept at all.  Otherwise, this routine will at a minimum send
 * a SIGKILL to all the tasks in this cpuset subtree, then sleep one
 * second, before looking to see if any tasks remain.  If tasks remain
 * in the cpuset subtree, and a longer 'seconds' timeout was requested
 * (more than one), it will continue to kill remaining tasks and sleep,
 * in a loop, for as long as time and tasks remain.
 *
 * The signal sent for the kill is hardcoded to SIGKILL (9).  If some
 * other signal should be sent first, use a separate code loop,
 * perhaps based on cpuset_init_pidlist and cpuset_get_pidlist, to
 * scan the task pids in a cpuset.  If SIGKILL should -not- be sent,
 * this cpuset_nuke() routine can still be called to recursively
 * remove a cpuset subtree, by specifying a timeout of zero 'seconds'.
 *
 * On success, returns 0 with errno == 0.
 *
 * On failure, returns -1, with errno possibly one of:
 *  EACCES - search permission denied on intervening directory
 *  ETIME - timed out - tasks remain after 'seconds' timeout
 *  EMFILE - too many open files
 *  ENODEV - /dev/cpuset not mounted
 *  ENOENT - component of cpuset path doesn't exist
 *  ENOMEM - out of memory
 *  ENOSYS - kernel doesn't support cpusets
 *  ENOTDIR - component of cpuset path is not a directory
 *  EPERM - lacked permission to kill a task
 *  EPERM - lacked permission to read cpusets or files therein
 */

void cpuset_fts_reverse(struct cpuset_fts_tree *cs_tree);

int cpuset_nuke(const char *relpath, unsigned int seconds)
{
	unsigned int secs_left = seconds;	/* total sleep seconds left */
	unsigned int secs_loop = 1;	/* how much sleep next loop */
	unsigned int secs_slept;	/* seconds slept in sleep() */
	struct cpuset_pidlist *pl = NULL;	/* pids in cpuset subtree */
	struct cpuset_fts_tree *cs_tree;
	const struct cpuset_fts_entry *cs_entry;
	int ret, sav_errno = 0;

	if (check() < 0)
		return -1;

	if (seconds == 0)
		goto rmdir_cpusets;

	while (1) {
		int plen, j;

		if ((pl = cpuset_init_pidlist(relpath, 1)) == NULL) {
			/* missing cpuset is as good as if already nuked */
			if (errno == ENOENT) {
				ret = 0;
				goto no_more_cpuset;
			}

			/* other problems reading cpuset are bad news */
			sav_errno = errno;
			goto failed;
		}

		if ((plen = cpuset_pidlist_length(pl)) == 0)
			goto rmdir_cpusets;

		for (j = 0; j < plen; j++) {
			pid_t pid;

			if ((pid = cpuset_get_pidlist(pl, j)) > 1) {
				if (kill(pid, SIGKILL) < 0 && errno != ESRCH) {
					sav_errno = errno;
					goto failed;
				}
			}
		}

		if (secs_left == 0)
			goto took_too_long;

		cpuset_freepidlist(pl);
		pl = NULL;

		secs_slept = secs_loop - sleep(secs_loop);

		/* Ensure forward progress */
		if (secs_slept == 0)
			secs_slept = 1;

		/* Ensure sane sleep() return (unnecessary?) */
		if (secs_slept > secs_loop)
			secs_slept = secs_loop;

		secs_left -= secs_slept;

		if (secs_loop < 10)
			secs_loop++;

		secs_loop = MIN(secs_left, secs_loop);
	}

took_too_long:
	sav_errno = ETIME;
	/* fall into ... */
failed:
	cpuset_freepidlist(pl);
	errno = sav_errno;
	return -1;

rmdir_cpusets:
	/* Let's try removing cpuset(s) now. */
	cpuset_freepidlist(pl);

	if ((cs_tree = cpuset_fts_open(relpath)) == NULL && errno != ENOENT)
		return -1;
	ret = 0;
	cpuset_fts_reverse(cs_tree);	/* rmdir's must be done bottom up */
	while ((cs_entry = cpuset_fts_read(cs_tree)) != NULL) {
		char buf[PATH_MAX];

		fullpath(buf, sizeof(buf), cpuset_fts_get_path(cs_entry));
		if (rmdir(buf) < 0 && errno != ENOENT) {
			sav_errno = errno;
			ret = -1;
		}
	}
	cpuset_fts_close(cs_tree);
	/* fall into ... */
no_more_cpuset:
	if (ret == 0)
		errno = 0;
	else
		errno = sav_errno;
	return ret;
}

/*
 * When recursively reading all the tasks files from a subtree,
 * chain together the read results, one pidblock per tasks file,
 * containing the raw unprocessed ascii as read(2) in.  After
 * we gather up this raw data, we then go back to count how
 * many pid's there are in total, allocate an array of pid_t
 * of that size, and transform the raw ascii data into this
 * array of pid_t's.
 */

struct pidblock {
	char *buf;
	int buflen;
	struct pidblock *next;
};

/*
 * Chain the raw contents of a file onto the pbhead list.
 *
 * We malloc "+ 1" extra byte for a nul-terminator, so that
 * the strtoul() loop in pid_transform() won't scan past
 * the end of pb->buf[] and accidentally find more pids.
 */
static void add_pidblock(const char *file, struct pidblock **ppbhead)
{
	FILE *fp = NULL;
	struct pidblock *pb = NULL;
	int fsz;

	if ((fp = fopen(file, "r")) == NULL)
		goto err;
	fsz = filesize(fp);
	if (fsz == 0)
		goto err;
	if ((pb = calloc(1, sizeof(*pb))) == NULL)
		goto err;
	pb->buflen = fsz;
	if ((pb->buf = malloc(pb->buflen + 1)) == NULL)
		goto err;
	if (fread(pb->buf, 1, pb->buflen, fp) > 0) {
		pb->buf[pb->buflen] = '\0';
		pb->next = *ppbhead;
		*ppbhead = pb;
	}
	fclose(fp);
	return;
err:
	if (fp)
		fclose(fp);
	free(pb);
}

static void read_task_file(const char *relpath, struct pidblock **ppbhead)
{
	char buf[PATH_MAX];

	fullpath2(buf, sizeof(buf), relpath, "tasks");
	add_pidblock(buf, ppbhead);
}

struct cpuset_pidlist {
	pid_t *pids;
	int npids;
};

/* Count how many pids in buf (one per line - just count newlines) */
static int pidcount(const char *buf, int buflen)
{
	int n = 0;
	const char *cp;

	for (cp = buf; cp < buf + buflen; cp++) {
		if (*cp == '\n')
			n++;
	}
	return n;
}

/* Transform one-per-line ascii pids in pb to pid_t entries in pl */
static int pid_transform(struct pidblock *pb, struct cpuset_pidlist *pl, int n)
{
	char *a, *b;

	for (a = pb->buf; a < pb->buf + pb->buflen; a = b) {
		pid_t p = strtoul(a, &b, 10);
		if (a == b)
			break;
		pl->pids[n++] = p;
	}
	return n;
}

static void free_pidblocks(struct pidblock *pbhead)
{
	struct pidblock *pb, *nextpb;

	for (pb = pbhead; pb; pb = nextpb) {
		nextpb = pb->next;
		free(pb->buf);
		free(pb);
	}
}

/* numeric comparison routine for qsort */
static int numericsort(const void *m1, const void *m2)
{
	pid_t p1 = *(pid_t *) m1;
	pid_t p2 = *(pid_t *) m2;

	return p1 - p2;
}

/* Return list pids in cpuset 'path' */
struct cpuset_pidlist *cpuset_init_pidlist(const char *relpath,
					   int recursiveflag)
{
	struct pidblock *pb = NULL;
	struct cpuset_pidlist *pl = NULL;
	struct pidblock *pbhead = NULL;
	int n;

	if (check() < 0)
		goto err;

	if (recursiveflag) {
		struct cpuset_fts_tree *cs_tree;
		const struct cpuset_fts_entry *cs_entry;

		if ((cs_tree = cpuset_fts_open(relpath)) == NULL)
			goto err;
		while ((cs_entry = cpuset_fts_read(cs_tree)) != NULL) {
			if (cpuset_fts_get_info(cs_entry) != CPUSET_FTS_CPUSET)
				continue;
			read_task_file(cpuset_fts_get_path(cs_entry), &pbhead);
		}
		cpuset_fts_close(cs_tree);
	} else {
		read_task_file(relpath, &pbhead);
	}

	if ((pl = calloc(1, sizeof(*pl))) == NULL)
		goto err;
	pl->npids = 0;
	for (pb = pbhead; pb; pb = pb->next)
		pl->npids += pidcount(pb->buf, pb->buflen);
	if ((pl->pids = calloc(pl->npids, sizeof(pid_t))) == NULL)
		goto err;
	n = 0;
	for (pb = pbhead; pb; pb = pb->next)
		n = pid_transform(pb, pl, n);
	free_pidblocks(pbhead);
	qsort(pl->pids, pl->npids, sizeof(pid_t), numericsort);
	return pl;
err:
	cpuset_freepidlist(pl);
	free_pidblocks(pbhead);
	return NULL;
}

/* Return number of elements in pidlist */
int cpuset_pidlist_length(const struct cpuset_pidlist *pl)
{
	if (pl)
		return pl->npids;
	else
		return 0;
}

/* Return i'th element of pidlist */
pid_t cpuset_get_pidlist(const struct cpuset_pidlist * pl, int i)
{
	if (pl && i >= 0 && i < pl->npids)
		return pl->pids[i];
	else
		return (pid_t) - 1;
}

/* Free pidlist */
void cpuset_freepidlist(struct cpuset_pidlist *pl)
{
	if (pl && pl->pids)
		free(pl->pids);
	free(pl);
}

static int __cpuset_move(pid_t pid, const char *path)
{
	char buf[SMALL_BUFSZ];

	snprintf(buf, sizeof(buf), "%u", pid);
	return write_string_file(path, buf);
}

/* Move task (pid == 0 for current) to a cpuset */
int cpuset_move(pid_t pid, const char *relpath)
{
	char buf[PATH_MAX];

	if (check() < 0)
		return -1;

	if (pid == 0)
		pid = getpid();

	fullpath2(buf, sizeof(buf), relpath, "tasks");
	return __cpuset_move(pid, buf);
}

/* Move all tasks in pidlist to a cpuset */
int cpuset_move_all(struct cpuset_pidlist *pl, const char *relpath)
{
	int i;
	char buf[PATH_MAX];
	int ret;

	if (check() < 0)
		return -1;

	fullpath2(buf, sizeof(buf), relpath, "tasks");

	ret = 0;
	for (i = 0; i < pl->npids; i++)
		if (__cpuset_move(pl->pids[i], buf) < 0)
			ret = -1;
	return ret;
}

/*
 * [optional] cpuset_move_cpuset_tasks() - Move all tasks in a
 *                                      cpuset to another cpuset
 *
 * Move all tasks in cpuset fromrelpath to cpuset torelpath. This may
 * race with tasks being added to or forking into fromrelpath. Loop
 * repeatedly, reading the tasks file of cpuset fromrelpath and writing
 * any task pid's found there to the tasks file of cpuset torelpath,
 * up to ten attempts, or until the tasks file of cpuset fromrelpath
 * is empty, or until fromrelpath is no longer present.
 *
 * Returns 0 with errno == 0 if able to empty the tasks file of cpuset
 * fromrelpath. Of course it is still possible that some independent
 * task could add another task to cpuset fromrelpath at the same time
 * that such a successful result is being returned, so there can be
 * no guarantee that a successful return means that fromrelpath is
 * still empty of tasks.
 *
 * We are careful to allow for the possibility that the cpuset
 * fromrelpath might disappear out from under us, perhaps because it
 * has notify_on_release set and gets automatically removed as soon
 * as we detach its last task from it.  Consider a missing fromrelpath
 * to be a successful move.
 *
 * If called with fromrelpath and torelpath pathnames that evaluate to
 * the same cpuset, then treat that as if cpuset_reattach() was called,
 * rebinding each task in this cpuset one time, and return success or
 * failure depending on the return of that cpuset_reattach() call.
 *
 * On failure, returns -1, with errno possibly one of:
 *  EACCES - search permission denied on intervening directory
 *  ENOTEMPTY - tasks remain after multiple attempts to move them
 *  EMFILE - too many open files
 *  ENODEV - /dev/cpuset not mounted
 *  ENOENT - component of cpuset path doesn't exist
 *  ENOMEM - out of memory
 *  ENOSYS - kernel doesn't support cpusets
 *  ENOTDIR - component of cpuset path is not a directory
 *  EPERM - lacked permission to kill a task
 *  EPERM - lacked permission to read cpusets or files therein
 *
 * This is an [optional] function. Use cpuset_function to invoke it.
 */

#define NUMBER_MOVE_TASK_ATTEMPTS 10

int cpuset_move_cpuset_tasks(const char *fromrelpath, const char *torelpath)
{
	char fromfullpath[PATH_MAX];
	char tofullpath[PATH_MAX];
	int i;
	struct cpuset_pidlist *pl = NULL;
	int sav_errno;

	fullpath(fromfullpath, sizeof(fromfullpath), fromrelpath);
	fullpath(tofullpath, sizeof(tofullpath), torelpath);

	if (samefile(fromfullpath, tofullpath))
		return cpuset_reattach(fromrelpath);

	for (i = 0; i < NUMBER_MOVE_TASK_ATTEMPTS; i++) {
		int plen, j;

		if ((pl = cpuset_init_pidlist(fromrelpath, 0)) == NULL) {
			/* missing cpuset is as good as if all moved */
			if (errno == ENOENT)
				goto no_more_cpuset;

			/* other problems reading cpuset are bad news */
			sav_errno = errno;
			goto failed;
		}

		if ((plen = cpuset_pidlist_length(pl)) == 0)
			goto no_more_pids;

		for (j = 0; j < plen; j++) {
			pid_t pid;

			pid = cpuset_get_pidlist(pl, j);
			if (cpuset_move(pid, torelpath) < 0) {
				/* missing task is as good as if moved */
				if (errno == ESRCH)
					continue;

				/* other per-task errors are bad news */
				sav_errno = errno;
				goto failed;
			}
		}

		cpuset_freepidlist(pl);
		pl = NULL;
	}

	sav_errno = ENOTEMPTY;
	/* fall into ... */
failed:
	cpuset_freepidlist(pl);
	errno = sav_errno;
	return -1;

no_more_pids:
no_more_cpuset:
	/* Success - all tasks (or entire cpuset ;) gone. */
	cpuset_freepidlist(pl);
	errno = 0;
	return 0;
}

/* Migrate task (pid == 0 for current) to a cpuset (moves task and memory) */
int cpuset_migrate(pid_t pid, const char *relpath)
{
	char buf[PATH_MAX];
	char buf2[PATH_MAX];
	char memory_migrate_flag;
	int r;

	if (check() < 0)
		return -1;

	if (pid == 0)
		pid = getpid();

	fullpath(buf2, sizeof(buf2), relpath);

	if (load_flag(buf2, &memory_migrate_flag, "memory_migrate") < 0)
		return -1;
	if (store_flag(buf2, "memory_migrate", 1) < 0)
		return -1;

	fullpath2(buf, sizeof(buf), relpath, "tasks");

	r = __cpuset_move(pid, buf);

	store_flag(buf2, "memory_migrate", memory_migrate_flag);
	return r;
}

/* Migrate all tasks in pidlist to a cpuset (moves task and memory) */
int cpuset_migrate_all(struct cpuset_pidlist *pl, const char *relpath)
{
	int i;
	char buf[PATH_MAX];
	char buf2[PATH_MAX];
	char memory_migrate_flag;
	int ret;

	if (check() < 0)
		return -1;

	fullpath(buf2, sizeof(buf2), relpath);

	if (load_flag(buf2, &memory_migrate_flag, "memory_migrate") < 0)
		return -1;
	if (store_flag(buf2, "memory_migrate", 1) < 0)
		return -1;

	fullpath2(buf, sizeof(buf), relpath, "tasks");

	ret = 0;
	for (i = 0; i < pl->npids; i++)
		if (__cpuset_move(pl->pids[i], buf) < 0)
			ret = -1;

	if (store_flag(buf2, "memory_migrate", memory_migrate_flag) < 0)
		ret = -1;
	return ret;
}

/* Rebind cpus_allowed of each task in cpuset 'path' */
int cpuset_reattach(const char *relpath)
{
	struct cpuset_pidlist *pl;
	int rc;

	if ((pl = cpuset_init_pidlist(relpath, 0)) == NULL)
		return -1;
	rc = cpuset_move_all(pl, relpath);
	cpuset_freepidlist(pl);
	return rc;
}

/* Map cpuset relative cpu number to system wide cpu number */
int cpuset_c_rel_to_sys_cpu(const struct cpuset *cp, int cpu)
{
	struct cpuset *cp_tofree = NULL;
	const struct cpuset *cp1 = resolve_cp(cp, &cp_tofree);
	int pos = -1;

	if (!cp1)
		goto err;
	pos = bitmask_rel_to_abs_pos(cp1->cpus, cpu);
	/* fall into ... */
err:
	cpuset_free(cp_tofree);
	return pos;
}

/* Map system wide cpu number to cpuset relative cpu number */
int cpuset_c_sys_to_rel_cpu(const struct cpuset *cp, int cpu)
{
	struct cpuset *cp_tofree = NULL;
	const struct cpuset *cp1 = resolve_cp(cp, &cp_tofree);
	int pos = -1;

	if (!cp1)
		goto err;
	pos = bitmask_abs_to_rel_pos(cp1->cpus, cpu);
	/* fall into ... */
err:
	cpuset_free(cp_tofree);
	return pos;
}

/* Map cpuset relative mem number to system wide mem number */
int cpuset_c_rel_to_sys_mem(const struct cpuset *cp, int mem)
{
	struct cpuset *cp_tofree = NULL;
	const struct cpuset *cp1 = resolve_cp(cp, &cp_tofree);
	int pos = -1;

	if (!cp1)
		goto err;
	pos = bitmask_rel_to_abs_pos(cp1->mems, mem);
	/* fall into ... */
err:
	cpuset_free(cp_tofree);
	return pos;
}

/* Map system wide mem number to cpuset relative mem number */
int cpuset_c_sys_to_rel_mem(const struct cpuset *cp, int mem)
{
	struct cpuset *cp_tofree = NULL;
	const struct cpuset *cp1 = resolve_cp(cp, &cp_tofree);
	int pos = -1;

	if (!cp1)
		goto err;
	pos = bitmask_abs_to_rel_pos(cp1->mems, mem);
	/* fall into ... */
err:
	cpuset_free(cp_tofree);
	return pos;
}

/* Map pid's cpuset relative cpu number to system wide cpu number */
int cpuset_p_rel_to_sys_cpu(pid_t pid, int cpu)
{
	struct cpuset *cp;
	int rc = -1;

	if ((cp = cpuset_alloc()) == NULL)
		goto done;
	if (cpuset_cpusetofpid(cp, pid) < 0)
		goto done;
	rc = cpuset_c_rel_to_sys_cpu(cp, cpu);
done:
	cpuset_free(cp);
	return rc;
}

/* Map system wide cpu number to pid's cpuset relative cpu number */
int cpuset_p_sys_to_rel_cpu(pid_t pid, int cpu)
{
	struct cpuset *cp;
	int rc = -1;

	if ((cp = cpuset_alloc()) == NULL)
		goto done;
	if (cpuset_cpusetofpid(cp, pid) < 0)
		goto done;
	rc = cpuset_c_sys_to_rel_cpu(cp, cpu);
done:
	cpuset_free(cp);
	return rc;
}

/* Map pid's cpuset relative mem number to system wide mem number */
int cpuset_p_rel_to_sys_mem(pid_t pid, int mem)
{
	struct cpuset *cp;
	int rc = -1;

	if ((cp = cpuset_alloc()) == NULL)
		goto done;
	if (cpuset_cpusetofpid(cp, pid) < 0)
		goto done;
	rc = cpuset_c_rel_to_sys_mem(cp, mem);
done:
	cpuset_free(cp);
	return rc;
}

/* Map system wide mem number to pid's cpuset relative mem number */
int cpuset_p_sys_to_rel_mem(pid_t pid, int mem)
{
	struct cpuset *cp;
	int rc = -1;

	if ((cp = cpuset_alloc()) == NULL)
		goto done;
	if (cpuset_cpusetofpid(cp, pid) < 0)
		goto done;
	rc = cpuset_c_sys_to_rel_mem(cp, mem);
done:
	cpuset_free(cp);
	return rc;
}

/*
 * Override glibc's calls for get/set affinity - they have
 * something using cpu_set_t that will die when NR_CPUS > 1024.
 * Go directly to the 'real' system calls.  Also override calls
 * for get_mempolicy and set_mempolicy.  None of these
 * calls are yet (July 2004) guaranteed to be in all glibc versions
 * that we care about.
 */

static int sched_setaffinity(pid_t pid, unsigned len, unsigned long *mask)
{
	return ltp_syscall(__NR_sched_setaffinity, pid, len, mask);
}

static int get_mempolicy(int *policy, unsigned long *nmask,
			 unsigned long maxnode, void *addr, int flags)
{
	return ltp_syscall(__NR_get_mempolicy, policy, nmask, maxnode,
		addr, flags);
}

static int set_mempolicy(int mode, unsigned long *nmask, unsigned long maxnode)
{
	return ltp_syscall(__NR_set_mempolicy, mode, nmask, maxnode);
}

struct cpuset_placement {
	struct bitmask *cpus;
	struct bitmask *mems;
	char *path;
};

/* Allocate and fill in a placement struct - cpatures current placement */
struct cpuset_placement *cpuset_get_placement(pid_t pid)
{
	struct cpuset_placement *plc;
	struct cpuset *cp = NULL;
	char buf[PATH_MAX];
	int nbits;

	if ((plc = calloc(1, sizeof(*plc))) == NULL)
		goto err;

	nbits = cpuset_cpus_nbits();
	if ((plc->cpus = bitmask_alloc(nbits)) == NULL)
		goto err;

	nbits = cpuset_mems_nbits();
	if ((plc->mems = bitmask_alloc(nbits)) == NULL)
		goto err;

	if ((cp = cpuset_alloc()) == NULL)
		goto err;
	if (cpuset_getcpusetpath(pid, buf, sizeof(buf)) == NULL)
		goto err;
	if (cpuset_query(cp, buf) < 0)
		goto err;

	bitmask_copy(plc->cpus, cp->cpus);
	bitmask_copy(plc->mems, cp->mems);
	plc->path = strdup(buf);

	cpuset_free(cp);
	return plc;
err:
	cpuset_free(cp);
	cpuset_free_placement(plc);
	return NULL;
}

/* Compare two placement structs - use to detect changes in placement */
int cpuset_equal_placement(const struct cpuset_placement *plc1,
			   const struct cpuset_placement *plc2)
{
	return bitmask_equal(plc1->cpus, plc2->cpus) &&
	    bitmask_equal(plc1->mems, plc2->mems) &&
	    streq(plc1->path, plc2->path);
}

/* Free a placement struct */
void cpuset_free_placement(struct cpuset_placement *plc)
{
	if (!plc)
		return;
	bitmask_free(plc->cpus);
	bitmask_free(plc->mems);
	free(plc->path);
	free(plc);
}

/*
 * A cpuset_fts_open() call constructs a linked list of entries
 * called a "cpuset_fts_tree", with one entry per cpuset below
 * the specified path.  The cpuset_fts_read() routine returns the
 * next entry on this list.  The various cpuset_fts_get_*() calls
 * return attributes of the specified entry.  The cpuset_fts_close()
 * call frees the linked list and all associated data.  All cpuset
 * entries and attributes for the cpuset_fts_tree returned from a
 * given cpuset_fts_open() call remain allocated and unchanged until
 * that cpuset_fts_tree is closed by a cpuset_fts_close() call.  Any
 * subsequent changes to the cpuset filesystem will go unnoticed
 * (not affect open cpuset_fts_tree's.)
 */

struct cpuset_fts_entry;
void cpuset_fts_rewind(struct cpuset_fts_tree *cs_tree);

struct cpuset_fts_tree {
	struct cpuset_fts_entry *head;	/* head of linked entry list */
	struct cpuset_fts_entry *next;	/* cpuset_fts_read() offset */
};

struct cpuset_fts_entry {
	struct cpuset_fts_entry *next;	/* linked entry list chain */
	struct cpuset *cpuset;
	struct stat *stat;
	char *path;
	int info;
	int err;
};

/* Open a handle on a cpuset hierarchy.  All the real work is done here. */
struct cpuset_fts_tree *cpuset_fts_open(const char *cpusetpath)
{
	FTS *fts = NULL;
	FTSENT *ftsent;
	char *path_argv[2];
	char buf[PATH_MAX];
	struct cpuset_fts_tree *cs_tree = NULL;
	struct cpuset_fts_entry *ep;	/* the latest new list entry */
	struct cpuset_fts_entry **pnlep;	/* ptr to next list entry ptr */
	char *relpath;
	int fts_flags;

	fullpath(buf, sizeof(buf), cpusetpath);
	path_argv[0] = buf;
	path_argv[1] = NULL;

	fts_flags = FTS_PHYSICAL | FTS_NOCHDIR | FTS_NOSTAT | FTS_XDEV;
	fts = fts_open(path_argv, fts_flags, NULL);
	if (fts == NULL)
		goto err;

	cs_tree = malloc(sizeof(*cs_tree));
	if (cs_tree == NULL)
		goto err;
	pnlep = &cs_tree->head;
	*pnlep = NULL;

	while ((ftsent = fts_read(fts)) != NULL) {
		if (ftsent->fts_info != FTS_D && ftsent->fts_info != FTS_DNR)
			continue;

		/* ftsent is a directory (perhaps unreadable) ==> cpuset */
		ep = calloc(1, sizeof(*ep));
		if (ep == NULL)
			goto err;
		*pnlep = ep;
		pnlep = &ep->next;

		/* Set entry's path, and if DNR, error */
		relpath = ftsent->fts_path + strlen(cpusetmnt);
		if (strlen(relpath) == 0)
			relpath = "/";
		ep->path = strdup(relpath);
		if (ep->path == NULL)
			goto err;
		if (ftsent->fts_info == FTS_DNR) {
			ep->info = CPUSET_FTS_ERR_DNR;
			ep->err = ftsent->fts_errno;
			continue;
		}

		/* ftsent is a -readable- cpuset: set entry's stat, etc */
		ep->stat = calloc(1, sizeof(struct stat));
		if (ep->stat == NULL)
			goto err;
		if (stat(ftsent->fts_path, ep->stat) < 0) {
			ep->info = CPUSET_FTS_ERR_STAT;
			ep->err = ftsent->fts_errno;
			continue;
		}

		ep->cpuset = calloc(1, sizeof(struct cpuset));
		if (ep->cpuset == NULL)
			goto err;
		if (cpuset_query(ep->cpuset, relpath) < 0) {
			ep->info = CPUSET_FTS_ERR_CPUSET;
			ep->err = errno;
			continue;
		}
		ep->info = CPUSET_FTS_CPUSET;
	}

	(void)fts_close(fts);
	cpuset_fts_rewind(cs_tree);
	return cs_tree;

err:
	if (cs_tree)
		cpuset_fts_close(cs_tree);
	if (fts)
		(void)fts_close(fts);
	return NULL;
}

/* Return pointer to next cpuset entry in hierarchy */
const struct cpuset_fts_entry *cpuset_fts_read(struct cpuset_fts_tree *cs_tree)
{
	const struct cpuset_fts_entry *cs_entry = cs_tree->next;
	if (cs_tree->next != NULL)	/* seek to next entry */
		cs_tree->next = cs_tree->next->next;
	return cs_entry;
}

/* Reverse list of cpusets, in place.  Simulates pre-order/post-order flip. */
void cpuset_fts_reverse(struct cpuset_fts_tree *cs_tree)
{
	struct cpuset_fts_entry *cs1, *cs2, *cs3;

	/*
	 * At each step, cs1 < cs2 < cs3 and the cs2->next pointer
	 * is redirected from cs3 to cs1.
	 */

	cs1 = cs2 = NULL;
	cs3 = cs_tree->head;
	while (cs3) {
		cs1 = cs2;
		cs2 = cs3;
		cs3 = cs3->next;
		cs2->next = cs1;
	}
	cs_tree->head = cs2;
	cpuset_fts_rewind(cs_tree);
}

/* Rewind cpuset list to beginning */
void cpuset_fts_rewind(struct cpuset_fts_tree *cs_tree)
{
	cs_tree->next = cs_tree->head;
}

/* Return pointer to nul-terminated cpuset path of entry in hierarchy */
const char *cpuset_fts_get_path(const struct cpuset_fts_entry *cs_entry)
{
	return cs_entry->path;
}

/* Return pointer to stat(2) structure of a cpuset entry's directory */
const struct stat *cpuset_fts_get_stat(const struct cpuset_fts_entry *cs_entry)
{
	return cs_entry->stat;
}

/* Return pointer to cpuset structure of a cpuset entry */
const struct cpuset *cpuset_fts_get_cpuset(const struct cpuset_fts_entry
					   *cs_entry)
{
	return cs_entry->cpuset;
}

/* Return value of errno (0 if no error) on attempted cpuset operations */
int cpuset_fts_get_errno(const struct cpuset_fts_entry *cs_entry)
{
	return cs_entry->err;
}

/* Return operation identity causing error */
int cpuset_fts_get_info(const struct cpuset_fts_entry *cs_entry)
{
	return cs_entry->info;
}

/* Close a cpuset hierarchy handle (free's all associated memory) */
void cpuset_fts_close(struct cpuset_fts_tree *cs_tree)
{
	struct cpuset_fts_entry *cs_entry = cs_tree->head;

	while (cs_entry) {
		struct cpuset_fts_entry *ep = cs_entry;

		cs_entry = cs_entry->next;
		free(ep->path);
		free(ep->stat);
		cpuset_free(ep->cpuset);
		free(ep);
	}
	free(cs_tree);
}

/* Bind current task to cpu (uses sched_setaffinity(2)) */
int cpuset_cpubind(int cpu)
{
	struct bitmask *bmp;
	int r;

	if ((bmp = bitmask_alloc(cpuset_cpus_nbits())) == NULL)
		return -1;
	bitmask_setbit(bmp, cpu);
	r = sched_setaffinity(0, bitmask_nbytes(bmp), bitmask_mask(bmp));
	bitmask_free(bmp);
	return r;
}

/*
 * int cpuset_latestcpu(pid_t pid)
 *
 * Return most recent CPU on which task pid executed.  If pid == 0,
 * examine current task.
 *
 * The last used CPU is visible for a given pid as field #39 (starting
 * with #1) in the file /proc/pid/stat.  Currently this file has 41
 * fields, in which case this is the 3rd to the last field.
 *
 * Unfortunately field #2 is a command name and might have embedded
 * whitespace.  So we can't just count white space separated fields.
 * Fortunately, this command name is surrounded by parentheses, as
 * for example "(sh)", and that closing parenthesis is the last ')'
 * character in the line.  No remaining fields can have embedded
 * whitespace or parentheses.  So instead of looking for the 39th
 * white space separated field, we can look for the 37th white space
 * separated field past the last ')' character on the line.
 */

/* Return most recent CPU on which task pid executed */
int cpuset_latestcpu(pid_t pid)
{
	char buf[PATH_MAX];
	char *bp;
	int fd = -1;
	int cpu = -1;

	if (pid == 0)
		snprintf(buf, sizeof(buf), "/proc/self/stat");
	else
		snprintf(buf, sizeof(buf), "/proc/%d/stat", pid);

	if ((fd = open(buf, O_RDONLY)) < 0)
		goto err;
	if (read(fd, buf, sizeof(buf)) < 1)
		goto err;
	close(fd);

	bp = strrchr(buf, ')');
	if (bp)
		sscanf(bp + 1, "%*s %*u %*u %*u %*u %*u %*u %*u " "%*u %*u %*u %*u %*u %*u %*u %*u %*u %*u " "%*u %*u %*u %*u %*u %*u %*u %*u %*u %*u " "%*u %*u %*u %*u %*u %*u %*u %*u %u",	/* 37th field past ')' */
		       &cpu);
	if (cpu < 0)
		errno = EINVAL;
	return cpu;
err:
	if (fd >= 0)
		close(fd);
	return -1;
}

/* Bind current task to memory (uses set_mempolicy(2)) */
int cpuset_membind(int mem)
{
	struct bitmask *bmp;
	int r;

	if ((bmp = bitmask_alloc(cpuset_mems_nbits())) == NULL)
		return -1;
	bitmask_setbit(bmp, mem);
	r = set_mempolicy(MPOL_BIND, bitmask_mask(bmp), bitmask_nbits(bmp) + 1);
	bitmask_free(bmp);
	return r;
}

/* [optional] Return Memory Node holding page at specified addr */
int cpuset_addr2node(void *addr)
{
	int node = -1;

	if (get_mempolicy(&node, NULL, 0, addr, MPOL_F_NODE | MPOL_F_ADDR)) {
		/* I realize this seems redundant, but I _want_ to make sure
		 * that this value is -1. */
		node = -1;
	}
	return node;
}

/*
 * Transform cpuset into Text Format Representation in buffer 'buf',
 * of length 'buflen', nul-terminated if space allows.  Return number
 * of characters that would have been written, if enough space had
 * been available, in the same way that snprintf() does.
 */

/* Export cpuset settings to a regular file */
int cpuset_export(const struct cpuset *cp, char *buf, int buflen)
{
	char *tmp = NULL;
	int n = 0;

	if (cp->cpu_exclusive)
		n += snprintf(buf + n, MAX(buflen - n, 0), "cpu_exclusive\n");

	if (cp->mem_exclusive)
		n += snprintf(buf + n, MAX(buflen - n, 0), "mem_exclusive\n");

	if (cp->notify_on_release)
		n += snprintf(buf + n, MAX(buflen - n, 0),
			      "notify_on_release\n");

	if (cp->memory_pressure_enabled)
		n += snprintf(buf + n, MAX(buflen - n, 0),
			      "memory_pressure_enabled\n");

	if (cp->memory_migrate)
		n += snprintf(buf + n, MAX(buflen - n, 0), "memory_migrate\n");

	if (cp->memory_spread_page)
		n += snprintf(buf + n, MAX(buflen - n, 0),
			      "memory_spread_page\n");

	if (cp->memory_spread_slab)
		n += snprintf(buf + n, MAX(buflen - n, 0),
			      "memory_spread_slab\n");

	if ((tmp = sprint_mask_buf(cp->cpus)) == NULL)
		return -1;
	n += snprintf(buf + n, MAX(buflen - n, 0), "cpus %s\n", tmp);
	free(tmp);
	tmp = NULL;

	if ((tmp = sprint_mask_buf(cp->mems)) == NULL)
		return -1;
	n += snprintf(buf + n, MAX(buflen - n, 0), "mems %s\n", tmp);
	free(tmp);
	tmp = NULL;

	return n;
}

static int import_list(UNUSED const char *tok, const char *arg,
		       struct bitmask *bmp, char *emsg, int elen)
{
	if (bitmask_parselist(arg, bmp) < 0) {
		if (emsg)
			snprintf(emsg, elen, "Invalid list format: %s", arg);
		return -1;
	}
	return 0;
}

static void stolower(char *s)
{
	while (*s) {
		unsigned char c = *s;
		*s = tolower(c);
		s++;
	}
}

/* Import cpuset settings from a regular file */
int cpuset_import(struct cpuset *cp, const char *buf, int *elinenum,
		  char *emsg, int elen)
{
	char *linebuf = NULL;
	int linebuflen;
	int linenum = 0;
	int offset = 0;

	linebuflen = strlen(buf) + 1;
	if ((linebuf = malloc(linebuflen)) == NULL) {
		if (emsg)
			snprintf(emsg, elen, "Insufficient memory");
		goto err;
	}

	while (slgets(linebuf, linebuflen, buf, &offset)) {
		char *tok, *arg;
		char *ptr;	/* for strtok_r */

		linenum++;
		if ((tok = strchr(linebuf, '#')) != NULL)
			*tok = 0;
		if ((tok = strtok_r(linebuf, " \t", &ptr)) == NULL)
			continue;
		stolower(tok);

		arg = strtok_r(0, " \t", &ptr);

		if (streq(tok, "cpu_exclusive")) {
			cp->cpu_exclusive = 1;
			goto eol;
		}
		if (streq(tok, "mem_exclusive")) {
			cp->mem_exclusive = 1;
			goto eol;
		}
		if (streq(tok, "notify_on_release")) {
			cp->notify_on_release = 1;
			goto eol;
		}
		if (streq(tok, "memory_pressure_enabled")) {
			cp->memory_pressure_enabled = 1;
			goto eol;
		}
		if (streq(tok, "memory_migrate")) {
			cp->memory_migrate = 1;
			goto eol;
		}
		if (streq(tok, "memory_spread_page")) {
			cp->memory_spread_page = 1;
			goto eol;
		}
		if (streq(tok, "memory_spread_slab")) {
			cp->memory_spread_slab = 1;
			goto eol;
		}
		if (streq(tok, "cpu") || streq(tok, "cpus")) {
			if (import_list(tok, arg, cp->cpus, emsg, elen) < 0)
				goto err;
			goto eol;
		}
		if (streq(tok, "mem") || streq(tok, "mems")) {
			if (import_list(tok, arg, cp->mems, emsg, elen) < 0)
				goto err;
			goto eol;
		}
		if (emsg)
			snprintf(emsg, elen, "Unrecognized token: '%s'", tok);
		goto err;
eol:
		if ((tok = strtok_r(0, " \t", &ptr)) != NULL) {
			if (emsg)
				snprintf(emsg, elen, "Surplus token: '%s'",
					 tok);
			goto err;
		}
		continue;
	}

	free(linebuf);

	if (bitmask_isallclear(cp->cpus) && !bitmask_isallclear(cp->mems))
		cpuset_localcpus(cp->mems, cp->cpus);
	else if (!bitmask_isallclear(cp->cpus) && bitmask_isallclear(cp->mems))
		cpuset_localmems(cp->cpus, cp->mems);

	/*
	 * All cpuset attributes are determined in an import.
	 * Those that aren't explicitly specified are presumed
	 * to be unchanged (zero, if it's a freshly allocated
	 * struct cpuset.)
	 */

	cp->cpus_valid = 1;
	cp->mems_valid = 1;
	cp->cpu_exclusive_valid = 1;
	cp->mem_exclusive_valid = 1;
	cp->notify_on_release_valid = 1;
	cp->memory_migrate_valid = 1;
	cp->memory_pressure_enabled_valid = 1;
	cp->memory_spread_page_valid = 1;
	cp->memory_spread_slab_valid = 1;

	return 0;
err:
	if (elinenum)
		*elinenum = linenum;
	free(linebuf);
	return -1;
}

/* Pin current task CPU (and memory) */
int cpuset_pin(int relcpu)
{
	struct cpuset_placement *plc1 = NULL, *plc2 = NULL;
	int cpu, r;

	if (check() < 0)
		return -1;

	do {
		cpuset_free_placement(plc1);
		plc1 = cpuset_get_placement(0);

		r = 0;
		if (cpuset_unpin() < 0)
			r = -1;
		cpu = cpuset_p_rel_to_sys_cpu(0, relcpu);
		if (cpuset_cpubind(cpu) < 0)
			r = -1;

		cpuset_free_placement(plc2);
		plc2 = cpuset_get_placement(0);
	} while (!cpuset_equal_placement(plc1, plc2));

	cpuset_free_placement(plc1);
	cpuset_free_placement(plc2);
	return r;
}

/* Return number CPUs in current tasks cpuset */
int cpuset_size(void)
{
	struct cpuset_placement *plc1 = NULL, *plc2 = NULL;
	int r;

	if (check() < 0)
		return -1;

	do {
		cpuset_free_placement(plc1);
		plc1 = cpuset_get_placement(0);

		r = cpuset_cpus_weight(0);

		cpuset_free_placement(plc2);
		plc2 = cpuset_get_placement(0);
	} while (!cpuset_equal_placement(plc1, plc2));

	cpuset_free_placement(plc1);
	cpuset_free_placement(plc2);
	return r;
}

/* Return relative CPU number, within current cpuset, last executed on */
int cpuset_where(void)
{
	struct cpuset_placement *plc1 = NULL, *plc2 = NULL;
	int r;

	if (check() < 0)
		return -1;

	do {
		cpuset_free_placement(plc1);
		plc1 = cpuset_get_placement(0);

		r = cpuset_p_sys_to_rel_cpu(0, cpuset_latestcpu(0));

		cpuset_free_placement(plc2);
		plc2 = cpuset_get_placement(0);
	} while (!cpuset_equal_placement(plc1, plc2));

	cpuset_free_placement(plc1);
	cpuset_free_placement(plc2);
	return r;
}

/* Undo cpuset_pin - let current task have the run of all CPUs in its cpuset */
int cpuset_unpin(void)
{
	struct bitmask *cpus = NULL, *mems = NULL;
	int r = -1;

	if (check() < 0)
		goto err;

	/*
	 * Don't need cpuset_*_placement() guard against concurrent
	 * cpuset migration, because none of the following depends
	 * on the tasks cpuset placement.
	 */

	if ((cpus = bitmask_alloc(cpuset_cpus_nbits())) == NULL)
		goto err;
	bitmask_setall(cpus);
	if (sched_setaffinity(0, bitmask_nbytes(cpus), bitmask_mask(cpus)) < 0)
		goto err;

	if ((mems = bitmask_alloc(cpuset_mems_nbits())) == NULL)
		goto err;
	if (set_mempolicy(MPOL_DEFAULT, bitmask_mask(mems),
			  bitmask_nbits(mems) + 1) < 0)
		goto err;
	r = 0;
	/* fall into ... */
err:
	bitmask_free(cpus);
	bitmask_free(mems);
	return r;

}

struct cpuset_function_list {
	const char *fname;
	void *func;
} flist[] = {
	{
	"cpuset_version", cpuset_version}, {
	"cpuset_alloc", cpuset_alloc}, {
	"cpuset_free", cpuset_free}, {
	"cpuset_cpus_nbits", cpuset_cpus_nbits}, {
	"cpuset_mems_nbits", cpuset_mems_nbits}, {
	"cpuset_setcpus", cpuset_setcpus}, {
	"cpuset_setmems", cpuset_setmems}, {
	"cpuset_set_iopt", cpuset_set_iopt}, {
	"cpuset_set_sopt", cpuset_set_sopt}, {
	"cpuset_getcpus", cpuset_getcpus}, {
	"cpuset_getmems", cpuset_getmems}, {
	"cpuset_cpus_weight", cpuset_cpus_weight}, {
	"cpuset_mems_weight", cpuset_mems_weight}, {
	"cpuset_get_iopt", cpuset_get_iopt}, {
	"cpuset_get_sopt", cpuset_get_sopt}, {
	"cpuset_localcpus", cpuset_localcpus}, {
	"cpuset_localmems", cpuset_localmems}, {
	"cpuset_cpumemdist", cpuset_cpumemdist}, {
	"cpuset_cpu2node", cpuset_cpu2node}, {
	"cpuset_addr2node", cpuset_addr2node}, {
	"cpuset_create", cpuset_create}, {
	"cpuset_delete", cpuset_delete}, {
	"cpuset_query", cpuset_query}, {
	"cpuset_modify", cpuset_modify}, {
	"cpuset_getcpusetpath", cpuset_getcpusetpath}, {
	"cpuset_cpusetofpid", cpuset_cpusetofpid}, {
	"cpuset_mountpoint", cpuset_mountpoint}, {
	"cpuset_collides_exclusive", cpuset_collides_exclusive}, {
	"cpuset_nuke", cpuset_nuke}, {
	"cpuset_init_pidlist", cpuset_init_pidlist}, {
	"cpuset_pidlist_length", cpuset_pidlist_length}, {
	"cpuset_get_pidlist", cpuset_get_pidlist}, {
	"cpuset_freepidlist", cpuset_freepidlist}, {
	"cpuset_move", cpuset_move}, {
	"cpuset_move_all", cpuset_move_all}, {
	"cpuset_move_cpuset_tasks", cpuset_move_cpuset_tasks}, {
	"cpuset_migrate", cpuset_migrate}, {
	"cpuset_migrate_all", cpuset_migrate_all}, {
	"cpuset_reattach", cpuset_reattach}, {
	"cpuset_open_memory_pressure", cpuset_open_memory_pressure}, {
	"cpuset_read_memory_pressure", cpuset_read_memory_pressure}, {
	"cpuset_close_memory_pressure", cpuset_close_memory_pressure}, {
	"cpuset_c_rel_to_sys_cpu", cpuset_c_rel_to_sys_cpu}, {
	"cpuset_c_sys_to_rel_cpu", cpuset_c_sys_to_rel_cpu}, {
	"cpuset_c_rel_to_sys_mem", cpuset_c_rel_to_sys_mem}, {
	"cpuset_c_sys_to_rel_mem", cpuset_c_sys_to_rel_mem}, {
	"cpuset_p_rel_to_sys_cpu", cpuset_p_rel_to_sys_cpu}, {
	"cpuset_p_sys_to_rel_cpu", cpuset_p_sys_to_rel_cpu}, {
	"cpuset_p_rel_to_sys_mem", cpuset_p_rel_to_sys_mem}, {
	"cpuset_p_sys_to_rel_mem", cpuset_p_sys_to_rel_mem}, {
	"cpuset_get_placement", cpuset_get_placement}, {
	"cpuset_equal_placement", cpuset_equal_placement}, {
	"cpuset_free_placement", cpuset_free_placement}, {
	"cpuset_fts_open", cpuset_fts_open}, {
	"cpuset_fts_read", cpuset_fts_read}, {
	"cpuset_fts_reverse", cpuset_fts_reverse}, {
	"cpuset_fts_rewind", cpuset_fts_rewind}, {
	"cpuset_fts_get_path", cpuset_fts_get_path}, {
	"cpuset_fts_get_stat", cpuset_fts_get_stat}, {
	"cpuset_fts_get_cpuset", cpuset_fts_get_cpuset}, {
	"cpuset_fts_get_errno", cpuset_fts_get_errno}, {
	"cpuset_fts_get_info", cpuset_fts_get_info}, {
	"cpuset_fts_close", cpuset_fts_close}, {
	"cpuset_cpubind", cpuset_cpubind}, {
	"cpuset_latestcpu", cpuset_latestcpu}, {
	"cpuset_membind", cpuset_membind}, {
	"cpuset_export", cpuset_export}, {
	"cpuset_import", cpuset_import}, {
	"cpuset_function", cpuset_function}, {
	"cpuset_pin", cpuset_pin}, {
	"cpuset_size", cpuset_size}, {
	"cpuset_where", cpuset_where}, {
"cpuset_unpin", cpuset_unpin},};

/* Return pointer to a libcpuset.so function, or NULL */
void *cpuset_function(const char *function_name)
{
	unsigned int i;

	for (i = 0; i < sizeof(flist) / sizeof(flist[0]); i++)
		if (streq(function_name, flist[i].fname))
			return flist[i].func;
	return NULL;
}

/* Fortran interface to basic cpuset routines */
int cpuset_pin_(int *ptr_relcpu)
{
	return cpuset_pin(*ptr_relcpu);
}

int cpuset_size_(void)
{
	return cpuset_size();
}

int cpuset_where_(void)
{
	return cpuset_where();
}

int cpuset_unpin_(void)
{
	return cpuset_unpin();
}

#endif /* HAVE_LINUX_MEMPOLICY_H */
