/*
 * Copyright (c) 2013 Oracle and/or its affiliates. All Rights Reserved.
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
#include <string.h>
#include <errno.h>

#include "test.h"
#include "tst_kconfig.h"
#include "safe_macros.h"
#include "old_module.h"

#include "../tpci_kernel/tpci.h"

char *TCID = "test_pci";
int TST_TOTAL = PCI_TCASES_NUM;

static const char module_name[]	= PCI_DEVICE_NAME ".ko";
static const char dev_result[]	= "/sys/devices/" PCI_DEVICE_NAME "/result";
static const char dev_tcase[]	= "/sys/devices/" PCI_DEVICE_NAME "/tcase";
static const char dev_busslot[]	= "/sys/devices/" PCI_DEVICE_NAME "/bus_slot";
static int module_loaded;

static void cleanup(void)
{
	if (module_loaded)
		tst_module_unload(NULL, module_name);
}

void setup(void)
{
	struct tst_kcmdline_var params = TST_KCMDLINE_INIT("module.sig_enforce");
	struct tst_kconfig_var kconfig = TST_KCONFIG_INIT("CONFIG_MODULE_SIG_FORCE");

	tst_require_root();

	tst_kcmdline_parse(&params, 1);
	tst_kconfig_read(&kconfig, 1);
	if (params.found || kconfig.choice == 'y')
		tst_brkm(TCONF, tst_exit, "module signature is enforced, skip test");

	tst_sig(FORK, DEF_HANDLER, cleanup);
}

static void run_pci_testcases(int bus, int slot)
{
	int i, res;
	for (i = 0; i < TST_TOTAL; ++i) {
		/* skip pci disable test-case, it is manual */
		if (i == PCI_DISABLE)
			continue;

		SAFE_FILE_PRINTF(cleanup, dev_tcase, "%d", i);
		SAFE_FILE_SCANF(cleanup, dev_result, "%d", &res);

		tst_resm(res, "PCI bus %02x slot %02x : Test-case '%d'",
			bus, slot, i);
	}
}

static void test_run(void)
{
	tst_module_load(cleanup, module_name, NULL);
	module_loaded = 1;

	char buf[6];
	int i, j, fd, count;

	for (i = 0; i < MAX_BUS; ++i) {
		for (j = 0; j < MAX_DEVFN; ++j) {
			/* set pci device for the test */
			fd = SAFE_OPEN(cleanup, dev_busslot, O_WRONLY);
			count = snprintf(buf, 6, "%u", i << 8 | j);
			errno = 0;
			if (write(fd, buf, count) < 0) {
				if (errno == ENODEV) {
					SAFE_CLOSE(cleanup, fd);
					continue;
				}
				tst_brkm(TBROK | TERRNO, cleanup,
					"write to '%s' failed", dev_busslot);
			}
			SAFE_CLOSE(cleanup, fd);

			run_pci_testcases(i, j);

		}
	}
}

int main(void)
{
	setup();

	test_run();

	cleanup();

	tst_exit();
}
