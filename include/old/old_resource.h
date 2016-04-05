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

 /*

   Small helper for preparing files the test needs to copy before the testing.

   We need to support two scenarios.

   1. Test is executed in local directory and this is also the place
      we should look for files


   2. Test is executed after LTP has been installed, in this case we
      look for env LTPROOT (usually /opt/ltp/)

  */

#ifndef TST_RESOURCE
#define TST_RESOURCE

const char *tst_dataroot(void);

/*
 * Copy a file to the CWD. The destination is apended to CWD.
 */
#define TST_RESOURCE_COPY(cleanup_fn, filename, dest) \
	tst_resource_copy(__FILE__, __LINE__, (cleanup_fn), \
	                  (filename), (dest))

void tst_resource_copy(const char *file, const int lineno,
                       void (*cleanup_fn)(void),
		       const char *filename, const char *dest);

#endif /* TST_RESOURCE */
