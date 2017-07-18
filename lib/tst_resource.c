/*
 * Copyright (C) 2012 Cyril Hrubis chrubis@suse.cz
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
 */

#include <pthread.h>
#include "test.h"
#include "old_resource.h"
#include "ltp_priv.h"

#ifndef PATH_MAX
#ifdef MAXPATHLEN
#define PATH_MAX	MAXPATHLEN
#else
#define PATH_MAX	1024
#endif
#endif

static pthread_mutex_t tmutex = PTHREAD_MUTEX_INITIALIZER;
static char dataroot[PATH_MAX];
extern char *TCID;

static void tst_dataroot_init(void)
{
	const char *ltproot = getenv("LTPROOT");
	char curdir[PATH_MAX];
	const char *startdir;
	int ret;

	/* 1. if LTPROOT is set, use $LTPROOT/testcases/data/$TCID
	 * 2. else if startwd is set by tst_tmpdir(), use $STARWD/datafiles
	 * 3. else use $CWD/datafiles */
	if (ltproot) {
		ret = snprintf(dataroot, PATH_MAX, "%s/testcases/data/%s",
			ltproot, TCID);
	} else {
		startdir = tst_get_startwd();
		if (startdir[0] == 0) {
			if (getcwd(curdir, PATH_MAX) == NULL) {
				tst_brkm(TBROK | TERRNO, NULL,
					"tst_dataroot getcwd");
				return;
			}
			startdir = curdir;
		}
		ret = snprintf(dataroot, PATH_MAX, "%s/datafiles", startdir);
	}

	if (ret < 0 || ret >= PATH_MAX)
		tst_brkm(TBROK, NULL, "tst_dataroot snprintf: %d", ret);
}

const char *tst_dataroot(void)
{
	if (dataroot[0] == 0) {
		pthread_mutex_lock(&tmutex);
		if (dataroot[0] == 0)
			tst_dataroot_init();
		pthread_mutex_unlock(&tmutex);
	}
	return dataroot;
}

static int file_copy(const char *file, const int lineno,
                     void (*cleanup_fn)(void), const char *path,
		     const char *filename, const char *dest)
{
	size_t len = strlen(path) + strlen(filename) + 2;
	char buf[len];

	snprintf(buf, sizeof(buf), "%s/%s", path, filename);

	/* check if file exists */
	if (access(buf, R_OK))
		return 0;

	safe_cp(file, lineno, cleanup_fn, buf, dest);

	return 1;
}

void tst_resource_copy(const char *file, const int lineno,
                       void (*cleanup_fn)(void),
		       const char *filename, const char *dest)
{
	if (!tst_tmpdir_created()) {
		tst_brkm(TBROK, cleanup_fn,
		         "Temporary directory doesn't exist at %s:%d",
		         file, lineno);
		return;
	}

	if (dest == NULL)
		dest = ".";

	const char *ltproot = getenv("LTPROOT");
	const char *dataroot = tst_dataroot();

	/* look for data files in $LTP_DATAROOT, $LTPROOT/testcases/bin
	 * and $CWD */
	if (file_copy(file, lineno, cleanup_fn, dataroot, filename, dest))
		return;

	if (ltproot != NULL) {
		char buf[strlen(ltproot) + 64];
		
		snprintf(buf, sizeof(buf), "%s/testcases/bin", ltproot);
		
		if (file_copy(file, lineno, cleanup_fn, buf, filename, dest))
			return;
	}

	/* try directory test started in as last resort */
	const char *startwd = tst_get_startwd();
	if (file_copy(file, lineno, cleanup_fn, startwd, filename, dest))
		return;

	tst_brkm(TBROK, cleanup_fn, "Failed to copy resource '%s' at %s:%d",
	         filename, file, lineno);
}
