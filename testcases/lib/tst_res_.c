// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2024 Cyril Hrubis <chrubis@suse.cz>
 * Copyright (c) Linux Test Project, 2025
 */

#define TST_NO_DEFAULT_MAIN
#include "tst_test.h"

static void print_help(void)
{
	printf("Usage: tst_{res,brk} filename lineno [TPASS|TBROK|TFAIL|TWARN|TCONF|TINFO|TDEBUG] 'A short description'\n");
}

int main(int argc, char *argv[])
{
	int i, lineno, type;

	if (argc < 5) {
		fprintf(stderr, "%s:%d: invalid argc=%i, expected 5\n", __FILE__, __LINE__, argc);
		goto help;
	}

	lineno = atoi(argv[2]);

	if (!strcmp(argv[3], "TPASS")) {
		type = TPASS;
	} else if (!strcmp(argv[3], "TFAIL")) {
		type = TFAIL;
	} else if (!strcmp(argv[3], "TCONF")) {
		type = TCONF;
	} else if (!strcmp(argv[3], "TINFO")) {
		type = TINFO;
	} else if (!strcmp(argv[3], "TWARN")) {
		type = TWARN;
	} else if (!strcmp(argv[3], "TDEBUG")) {
		type = TDEBUG;
	} else if (!strcmp(argv[3], "TBROK")) {
		type = TBROK;
	} else {
		fprintf(stderr, "%s:%d: Wrong type '%s' (invoked by %s:%d)\n", __FILE__,
				__LINE__, argv[3], argv[1], lineno);
		goto help;
	}

	size_t len = 0;

	for (i = 4; i < argc; i++)
		len += strlen(argv[i]) + 1;

	char *msg = SAFE_MALLOC(len);
	char *msgp = msg;

	for (i = 4; i < argc; i++) {
		msgp = strcpy(msgp, argv[i]);
		msgp += strlen(argv[i]);
		*(msgp++) = ' ';
	}

	*(msgp - 1) = 0;

	tst_reinit();

	tst_res_(argv[1], lineno, type, "%s", msg);

	return 0;
help:
	print_help();
	return 1;
}
