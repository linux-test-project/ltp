/*
 * Copyright (c) 2013 Oracle and/or its affiliates. All Rights Reserved.
 * Copyright (c) Linux Test Project, 2016-2024
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it would be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write the Free Software Foundation,
 * Inc.,  51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * Author: Alexey Kodanev <alexey.kodanev@oracle.com>
 *
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>

#include "test.h"
#include "tst_kconfig.h"
#include "ltp_priv.h"
#include "old_module.h"

void tst_module_exists_(void (cleanup_fn)(void),
	const char *mod_name, char **mod_path)
{
	/* check current working directory */
	if (access(mod_name, F_OK) == 0) {
		if (mod_path != NULL)
			*mod_path = strdup(mod_name);
		return;
	}
	char *buf = NULL;
	int err = -1;
	/* check LTP installation path */
	const char *ltproot = getenv("LTPROOT");
	if (ltproot != NULL) {
		if (asprintf(&buf, "%s/testcases/bin/%s",
			ltproot, mod_name) == -1) {
			tst_brkm(TBROK | TERRNO, cleanup_fn,
				"asprintf failed at %s:%d",
				__FILE__, __LINE__);
			return;
		}
		err = access(buf, F_OK);
	}
	/* check start working directory */
	if (err == -1 && tst_tmpdir_created()) {
		free(buf);
		if (asprintf(&buf, "%s/%s", tst_get_startwd(),
			mod_name) == -1) {
			tst_brkm(TBROK | TERRNO, cleanup_fn,
				"asprintf failed at %s:%d",
				__FILE__, __LINE__);
			return;
		}
		err = access(buf, F_OK);
	}

	if (err != 0) {
		free(buf);
		tst_brkm(TCONF, cleanup_fn, "Failed to find module '%s'",
			mod_name);
		return;
	}

	if (mod_path != NULL)
		*mod_path = buf;
	else
		free(buf);
}

void tst_module_load_(void (cleanup_fn)(void),
	const char *mod_name, char *const argv[])
{
	char *mod_path = NULL;
	tst_module_exists_(cleanup_fn, mod_name, &mod_path);

	const int offset = 2; /* command name & module path */
	int size = 0;
	while (argv && argv[size])
		++size;
	size += offset;
	const char *mod_argv[size + 1]; /* + NULL in the end */
	mod_argv[size] = NULL;
	mod_argv[0] = "insmod";
	mod_argv[1] = mod_path;

	int i;
	for (i = offset; i < size; ++i)
		mod_argv[i] = argv[i - offset];

	tst_cmd(cleanup_fn, mod_argv, NULL, NULL, 0);
	free(mod_path);
}

void tst_module_unload_(void (cleanup_fn)(void), const char *mod_name)
{
	int i, rc;

	const char *const argv[] = { "rmmod", mod_name, NULL };

	rc = 1;
	for (i = 0; i < 50; i++) {
		rc = tst_cmd(NULL, argv, "/dev/null", "/dev/null",
				 TST_CMD_PASS_RETVAL);
		if (!rc)
			break;

		usleep(20000);
	}

	if (rc) {
		tst_brkm(TBROK, cleanup_fn,
			 "could not unload %s module", mod_name);
	}
}

bool tst_module_signature_enforced_(void)
{
	struct tst_kcmdline_var params = TST_KCMDLINE_INIT("module.sig_enforce");
	struct tst_kconfig_var kconfig = TST_KCONFIG_INIT("CONFIG_MODULE_SIG_FORCE");
	int rc;

	tst_kcmdline_parse(&params, 1);
	tst_kconfig_read(&kconfig, 1);

	rc = params.found || kconfig.choice == 'y';
	tst_resm(TINFO, "module signature enforcement: %s", rc ? "on" : "off");

	return rc;
}

void tst_requires_module_signature_disabled_(void)
{
	if (tst_module_signature_enforced_())
		tst_brkm(TCONF, NULL, "module signature is enforced, skip test");
}
