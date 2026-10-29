// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Child process: this binary is expected to be exec'd by the parent.
 *
 * It reinitializes the shared memory region using tst_reinit(),
 * verifies the command-line argument, and signals checkpoint 0.
 */

#define TST_NO_DEFAULT_MAIN
#include "tst_test.h"
#include "tst_checkpoint.h"

int main(int argc, char *argv[])
{
	tst_reinit();

	if (argc != 2)
		tst_brk(TFAIL, "argc is %d, expected 2", argc);

	if (strcmp(argv[1], "canary"))
		tst_brk(TFAIL, "argv[1] is %s, expected 'canary'", argv[1]);

	tst_res(TINFO, "Child: signaling checkpoint");
	TST_CHECKPOINT_WAKE(0);

	return 0;
}
