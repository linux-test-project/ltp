/*
 * Copyright (c) 2016 Cyril Hrubis <chrubis@suse.cz>
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
 */

#define TST_NO_DEFAULT_MAIN
#include <stdio.h>
#include <stdlib.h>
#include <tst_test.h>

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
	printf("Kernel version format has either one or two dots: ");
	printf("'2.6' or '4.8.1'\n");
}

int main(int argc, char *argv[])
{
	int i = 1;
	int ret = -1;
	int v1, v2, v3;
	enum op prev_op = ERR;

	if (argc <= 1 || !strcmp(argv[1], "-h")) {
		help(argv[0]);
		return 0;
	}

	while (i < argc) {
		const char *strop = argv[i++];
		const char *strkver;
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

			if (tst_parse_kver(strkver, &v1, &v2, &v3)) {
				fprintf(stderr,
					"Invalid kernel version '%s'\n",
					strkver);
				return 2;
			}
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

		res = tst_kvercmp(v1, v2, v3);

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
