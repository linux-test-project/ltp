// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2016 Cyril Hrubis <chrubis@suse.cz>
 */

#define TST_NO_DEFAULT_MAIN
#include <stdio.h>
#include <stdlib.h>
#include <sys/utsname.h>
#include "tst_test.h"

enum op {
	EQ,
	NE,
	GE,
	GT,
	LE,
	LT,
	AND,
	OR,
	ERR,
};

static enum op strtop(const char *op)
{
	if (!strcmp(op, "-eq"))
		return EQ;

	if (!strcmp(op, "-ne"))
		return NE;

	if (!strcmp(op, "-ge"))
		return GE;

	if (!strcmp(op, "-gt"))
		return GT;

	if (!strcmp(op, "-le"))
		return LE;

	if (!strcmp(op, "-lt"))
		return LT;

	if (!strcmp(op, "-a"))
		return AND;

	if (!strcmp(op, "-o"))
		return OR;

	return ERR;
}

static void help(const char *fname)
{
	printf("usage: %s -eq|-ne|-gt|-ge|-lt|-le kver [-a|-o] ...\n\n", fname);
	printf("-eq kver\tReturns true if kernel version is equal\n");
	printf("-ne kver\tReturns true if kernel version is not equal\n");
	printf("-gt kver\tReturns true if kernel version is greater\n");
	printf("-ge kver\tReturns true if kernel version is greater or equal\n");
	printf("-lt kver\tReturns true if kernel version is lesser\n");
	printf("-le kver\tReturns true if kernel version is lesser or equal\n");
	printf("-a  \t\tDoes logical and between two expressions\n");
	printf("-o  \t\tDoes logical or between two expressions\n\n");
	printf("Kernel version format has either one or two dots:\n\n");
	printf("'2.6' or '4.8.1'\n\n");
	printf("Kernel version can also be followed by a space separated list\n");
	printf("of extra versions prefixed by distribution which when matched\n");
	printf("take precedence:\n\n'3.0 RHEL6:2.6.18'\n\n");
}

static int compare_kver(const char *cur_kver, char *kver)
{
	const char *ver, *exver;
	const char *distname = tst_kvcmp_distname(cur_kver);
	int v1, v2, v3;

	ver = strtok(kver, " ");

	while ((exver = strtok(NULL, " "))) {
		char *exkver = strchr(exver, ':');

		if (!exkver) {
			fprintf(stderr, "Invalid extra version '%s'\n", exver);
			exit(2);
		}

		*(exkver++) = '\0';

		if (!distname || strcmp(distname, exver))
			continue;

		return tst_kvexcmp(exkver, cur_kver);
	}

	if (tst_parse_kver(ver, &v1, &v2, &v3)) {
		fprintf(stderr,
			"Invalid kernel version '%s'\n",
			ver);
		return 2;
	}

	return tst_kvcmp(cur_kver, v1, v2, v3);
}

int main(int argc, char *argv[])
{
	int i = 1;
	int ret = -1;
	enum op prev_op = ERR;
	struct utsname buf;

	if (argc <= 1 || !strcmp(argv[1], "-h")) {
		help(argv[0]);
		return 0;
	}

	uname(&buf);

	while (i < argc) {
		const char *strop = argv[i++];
		char *strkver;
		int res;

		enum op op = strtop(strop);

		switch (op) {
		case EQ:
		case NE:
		case GE:
		case GT:
		case LE:
		case LT:
			if (ret != -1 && prev_op == ERR) {
				fprintf(stderr, "Expected -a or -o\n");
				return 2;
			}

			if (i >= argc) {
				fprintf(stderr,
					"Expected kernel version after '%s'\n",
					strop);
				return 2;
			}

			strkver = argv[i++];
		break;
		case AND:
		case OR:
			if (ret == -1) {
				fprintf(stderr,
					"The %s must follow expression\n",
					strop);
				return 2;
			}
			prev_op = op;
			continue;
		break;
		case ERR:
			fprintf(stderr, "Invalid operation %s\n", argv[i]);
			return 2;
		}

		res = compare_kver(buf.release, strkver);

		switch (op) {
		case EQ:
			res = (res == 0);
		break;
		case NE:
			res = (res != 0);
		break;
		case GE:
			res = (res >= 0);
		break;
		case GT:
			res = (res > 0);
		break;
		case LE:
			res = (res <= 0);
		break;
		case LT:
			res = (res < 0);
		break;
		default:
		break;
		}

		switch (prev_op) {
		case ERR:
			ret = res;
		break;
		case AND:
			ret = ret && res;
			prev_op = ERR;
		break;
		case OR:
			ret = ret || res;
			prev_op = ERR;
		break;
		default:
		break;
		}
	}

	if (prev_op != ERR) {
		fprintf(stderr, "Useless -a or -o at the end\n");
		return 2;
	}

	return !ret;
}
