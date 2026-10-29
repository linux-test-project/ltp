// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2025 Cyril Hrubis <chrubis@suse.cz>
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <libgen.h>

#include "data_storage.h"

static int started;

static void json_start(char *path)
{
	if (started)
		return;

	started = 1;

	printf("   \"%s\": {\n", basename(path));
}

static void json_finish(const char *path)
{
	if (!started)
		return;

	printf("   \"fname\": \"%s\"\n", path);
	printf("  }");
}

enum state {
	NONE,
	START,
	DOC_FIRST,
	DOC,
	ENV_START,
	ENV_FIRST,
	ENV
};

static void parse_shell(char *path)
{
	char line[4096];
	FILE *f = fopen(path, "r");
	enum state state = NONE;

	if (!f) {
		fprintf(stderr, "Failed to open '%s': %s\n",
			path, strerror(errno));
		exit(1);
	}

	while (fgets(line, sizeof(line), f)) {
		/* Strip newline */
		line[strlen(line)-1] = 0;

		switch (state) {
		case NONE:
			if (!strcmp(line, "# ---"))
				state = START;
		break;
		case START:
			if (!strcmp(line, "# doc")) {
				json_start(path);
				state = DOC_FIRST;
				printf("   \"doc\": [\n");
			} else if (!strcmp(line, "# env")) {
				json_start(path);
				state = ENV_START;
			} else {
				state = NONE;
			}
		break;
		case DOC:
		case DOC_FIRST:
			if (!strcmp(line, "# ---")) {
				state = NONE;
				printf("\n   ],\n");
				continue;
			}

			if (state == DOC_FIRST)
				state = DOC;
			else
				printf(",\n");

			data_fprintf_esc(stdout, 4, line+2);
		break;
		case ENV_START:
			if (!strcmp(line, "# {")) {
				state = ENV_FIRST;
			} else {
				fprintf(stderr,
					"%s: Invalid line in JSON block '%s'",
					path, line);
			}
		break;
		case ENV:
		case ENV_FIRST:
			if (!strcmp(line, "# }")) {
				state = NONE;
				printf(",\n");
				continue;
			}

			if (state == ENV_FIRST)
				state = ENV;
			else
				printf("\n");

			line[0] = ' ';
			line[1] = ' ';

			printf("%s", line);
		break;
		}
	}

	json_finish(path);
}

int main(int argc, char *argv[])
{
	int i;

	for (i = 1; i < argc; i++)
		parse_shell(argv[i]);

	return 0;
}
