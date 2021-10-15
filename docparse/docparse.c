// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2019-2021 Cyril Hrubis <chrubis@suse.cz>
 * Copyright (c) 2020 Petr Vorel <pvorel@suse.cz>
 */

#define _GNU_SOURCE

#include <search.h>
#include <stdio.h>
#include <string.h>
#include <libgen.h>
#include <ctype.h>
#include <unistd.h>

#include "data_storage.h"

static int verbose;
static char *includepath;

#define WARN(str) fprintf(stderr, "WARNING: " str "\n")

static void oneline_comment(FILE *f)
{
	int c;

	do {
		c = getc(f);
	} while (c != '\n');
}

static const char *eat_asterisk_space(const char *c)
{
	unsigned int i = 0;

	while (isspace(c[i]))
		i++;

	if (c[i] == '*') {
		if (isspace(c[i+1]))
			i++;
		return &c[i+1];
	}

	return c;
}

static void multiline_comment(FILE *f, struct data_node *doc)
{
	int c;
	int state = 0;
	char buf[4096];
	unsigned int bufp = 0;

	for (;;) {
		c = getc(f);

		if (doc) {
			if (c == '\n') {
				struct data_node *line;
				buf[bufp] = 0;
				line = data_node_string(eat_asterisk_space(buf));
				if (data_node_array_add(doc, line))
					WARN("doc string comment truncated");
				bufp = 0;
				continue;
			}

			if (bufp + 1 >= sizeof(buf))
				continue;

			buf[bufp++] = c;
		}

		switch (state) {
		case 0:
			if (c == '*')
				state = 1;
		break;
		case 1:
			switch (c) {
			case '/':
				return;
			case '*':
				continue;
			default:
				state = 0;
			break;
			}
		break;
		}
	}

}

static const char doc_prefix[] = "\\\n";

static void maybe_doc_comment(FILE *f, struct data_node *doc)
{
	int c, i;

	for (i = 0; doc_prefix[i]; i++) {
		c = getc(f);

		if (c == doc_prefix[i])
			continue;

		if (c == '*')
			ungetc(c, f);

		multiline_comment(f, NULL);
		return;
	}

	multiline_comment(f, doc);
}

static void maybe_comment(FILE *f, struct data_node *doc)
{
	int c = getc(f);

	switch (c) {
	case '/':
		oneline_comment(f);
	break;
	case '*':
		maybe_doc_comment(f, doc);
	break;
	default:
		ungetc(c, f);
	break;
	}
}

static char *next_token(FILE *f, struct data_node *doc)
{
	size_t i = 0;
	static char buf[4096];
	int c;
	int in_str = 0;

	for (;;) {
		c = fgetc(f);

		if (c == EOF)
			goto exit;

		if (in_str) {
			if (c == '"') {
				if (i == 0 || buf[i-1] != '\\')
					goto exit;
			}

			buf[i++] = c;
			continue;
		}

		switch (c) {
		case '{':
		case '}':
		case ';':
		case '(':
		case ')':
		case '=':
		case ',':
		case '[':
		case ']':
		case '#':
			if (i) {
				ungetc(c, f);
				goto exit;
			}

			buf[i++] = c;
			goto exit;
		case '0' ... '9':
		case 'a' ... 'z':
		case 'A' ... 'Z':
		case '.':
		case '_':
		case '-':
			buf[i++] = c;
		break;
		case '/':
			maybe_comment(f, doc);
		break;
		case '"':
			in_str = 1;
		break;
		case ' ':
		case '\n':
		case '\t':
			if (i)
				goto exit;
		break;
		}
	}

exit:
	if (i == 0 && !in_str)
		return NULL;

	buf[i] = 0;
	return buf;
}

static FILE *open_include(const char *includepath, FILE *f)
{
	char buf[256];
	char *path;
	FILE *inc;

	if (!fscanf(f, "%s\n", buf))
		return NULL;

	if (buf[0] != '"')
		return NULL;

	char *filename = buf + 1;

	if (!buf[0])
		return NULL;

	filename[strlen(filename)-1] = 0;

	if (asprintf(&path, "%s/%s", includepath, filename) < 0)
		return NULL;

	inc = fopen(path, "r");

	if (inc && verbose)
		fprintf(stderr, "INCLUDE %s\n", path);

	free(path);

	return inc;
}

static void close_include(FILE *inc)
{
	if (verbose)
		fprintf(stderr, "INCLUDE END\n");

	fclose(inc);
}

static int parse_array(FILE *f, struct data_node *node)
{
	const char *token;

	for (;;) {
		if (!(token = next_token(f, NULL)))
			return 1;

		if (!strcmp(token, "{")) {
			struct data_node *ret = data_node_array();
			parse_array(f, ret);

			if (data_node_array_len(ret))
				data_node_array_add(node, ret);
			else
				data_node_free(ret);

			continue;
		}

		if (!strcmp(token, "}"))
			return 0;

		if (!strcmp(token, ","))
			continue;

		if (!strcmp(token, "NULL"))
			continue;

		struct data_node *str = data_node_string(token);

		data_node_array_add(node, str);
	}

	return 0;
}

static void try_apply_macro(char **res)
{
	ENTRY macro = {
		.key = *res,
	};

	ENTRY *ret;

	ret = hsearch(macro, FIND);

	if (!ret)
		return;

	if (verbose)
		fprintf(stderr, "APPLYING MACRO %s=%s\n", ret->key, (char*)ret->data);

	*res = ret->data;
}

static int parse_test_struct(FILE *f, struct data_node *doc, struct data_node *node)
{
	char *token;
	char *id = NULL;
	int state = 0;
	struct data_node *ret;

	for (;;) {
		if (!(token = next_token(f, doc)))
			return 1;

		if (!strcmp(token, "}"))
			return 0;

		switch (state) {
		case 0:
			id = strdup(token);
			state = 1;
			continue;
		case 1:
			if (!strcmp(token, "="))
				state = 2;
			else
				WARN("Expected '='");
			continue;
		case 2:
			if (!strcmp(token, "(")) {
				state = 3;
				continue;
			}
		break;
		case 3:
			if (!strcmp(token, ")"))
				state = 2;
			continue;

		case 4:
			if (!strcmp(token, ","))
				state = 0;
			continue;
		}

		if (!strcmp(token, "{")) {
			ret = data_node_array();
			parse_array(f, ret);
		} else {
			try_apply_macro(&token);
			ret = data_node_string(token);
		}

		const char *key = id;
		if (key[0] == '.')
			key++;

		data_node_hash_add(node, key, ret);
		free(id);
		state = 4;
	}
}

static const char *tokens[] = {
	"static",
	"struct",
	"tst_test",
	"test",
	"=",
	"{",
};

static void macro_get_string(FILE *f, char *buf, char *buf_end)
{
	int c;
	char *buf_start = buf;

	for (;;) {
		c = fgetc(f);

		switch (c) {
		case EOF:
			*buf = 0;
			return;
		case '"':
			if (buf == buf_start || buf[-1] != '\\') {
				*buf = 0;
				return;
			}
			buf[-1] = '"';
		break;
		default:
			if (buf < buf_end)
				*(buf++) = c;
		}
	}
}

static void macro_get_val(FILE *f, char *buf, size_t buf_len)
{
	int c, prev = 0;
	char *buf_end = buf + buf_len - 1;

	while (isspace(c = fgetc(f)));

	if (c == '"') {
		macro_get_string(f, buf, buf_end);
		return;
	}

	for (;;) {
		switch (c) {
		case '\n':
			if (prev == '\\') {
				buf--;
			} else {
				*buf = 0;
				return;
			}
		break;
		case EOF:
			*buf = 0;
			return;
		case ' ':
		case '\t':
		break;
		default:
			if (buf < buf_end)
				*(buf++) = c;
		}

		prev = c;
		c = fgetc(f);
	}
}

static void parse_macro(FILE *f)
{
	char name[128];
	char val[256];

	if (!fscanf(f, "%s[^\n]", name))
		return;

	if (fgetc(f) == '\n')
		return;

	macro_get_val(f, val, sizeof(val));

	ENTRY e = {
		.key = strdup(name),
		.data = strdup(val),
	};

	if (verbose)
		fprintf(stderr, " MACRO %s=%s\n", e.key, (char*)e.data);

	hsearch(e, ENTER);
}

static void parse_include_macros(FILE *f)
{
	FILE *inc;
	const char *token;
	int hash = 0;

	inc = open_include(includepath, f);
	if (!inc)
		return;

	while ((token = next_token(inc, NULL))) {
		if (token[0] == '#') {
			hash = 1;
			continue;
		}

		if (!hash)
			continue;

		if (!strcmp(token, "define"))
			parse_macro(inc);

		hash = 0;
	}

	close_include(inc);
}

static struct data_node *parse_file(const char *fname)
{
	int state = 0, found = 0;
	const char *token;

	if (access(fname, F_OK)) {
		fprintf(stderr, "file %s does not exist\n", fname);
		return NULL;
	}

	FILE *f = fopen(fname, "r");

	includepath = dirname(strdup(fname));

	struct data_node *res = data_node_hash();
	struct data_node *doc = data_node_array();

	while ((token = next_token(f, doc))) {
		if (state < 6 && !strcmp(tokens[state], token)) {
			state++;
		} else {
			if (token[0] == '#') {
				token = next_token(f, doc);
				if (token) {
					if (!strcmp(token, "define"))
						parse_macro(f);

					if (!strcmp(token, "include"))
						parse_include_macros(f);
				}
			}

			state = 0;
		}

		if (state < 6)
			continue;

		found = 1;
		parse_test_struct(f, doc, res);
	}


	if (data_node_array_len(doc)) {
		data_node_hash_add(res, "doc", doc);
		found = 1;
	} else {
		data_node_free(doc);
	}

	fclose(f);

	if (!found) {
		data_node_free(res);
		return NULL;
	}

	return res;
}

static const char *filter_out[] = {
	"bufs",
	"cleanup",
	"mntpoint",
	"setup",
	"tcnt",
	"test",
	"test_all",
	NULL
};

static struct implies {
	const char *flag;
	const char **implies;
} implies[] = {
	{"mount_device", (const char *[]) {"format_device", "needs_device",
		"needs_tmpdir", NULL}},
	{"format_device", (const char *[]) {"needs_device", "needs_tmpdir",
		NULL}},
	{"all_filesystems", (const char *[]) {"needs_device", "needs_tmpdir",
		NULL}},
	{"needs_device", (const char *[]) {"needs_tmpdir", NULL}},
	{"needs_checkpoints", (const char *[]) {"needs_tmpdir", NULL}},
	{"resource_files", (const char *[]) {"needs_tmpdir", NULL}},
	{NULL, (const char *[]) {NULL}}
};

const char *strip_name(char *path)
{
	char *name = basename(path);
	size_t len = strlen(name);

	if (len > 2 && name[len-1] == 'c' && name[len-2] == '.')
		name[len-2] = '\0';

	return name;
}

static void print_help(const char *prgname)
{
	printf("usage: %s [-vh] input.c\n\n", prgname);
	printf("-v sets verbose mode\n");
	printf("-h prints this help\n\n");
	exit(0);
}

int main(int argc, char *argv[])
{
	unsigned int i, j;
	struct data_node *res;
	int opt;

	while ((opt = getopt(argc, argv, "hv")) != -1) {
		switch (opt) {
		case 'h':
			print_help(argv[0]);
		break;
		case 'v':
			verbose = 1;
		break;
		}
	}

	if (optind >= argc) {
		fprintf(stderr, "No input filename.c\n");
		return 1;
	}

	if (!hcreate(128)) {
		fprintf(stderr, "Failed to initialize hash table\n");
		return 1;
	}

	res = parse_file(argv[optind]);
	if (!res)
		return 0;

	/* Filter out useless data */
	for (i = 0; filter_out[i]; i++)
		data_node_hash_del(res, filter_out[i]);

	/* Normalize the result */
	for (i = 0; implies[i].flag; i++) {
		if (data_node_hash_get(res, implies[i].flag)) {
			for (j = 0; implies[i].implies[j]; j++) {
				if (data_node_hash_get(res, implies[i].implies[j]))
					fprintf(stderr, "%s: useless tag: %s\n",
						argv[1], implies[i].implies[j]);
			}
		}
	}

	for (i = 0; implies[i].flag; i++) {
		if (data_node_hash_get(res, implies[i].flag)) {
			for (j = 0; implies[i].implies[j]; j++) {
				if (!data_node_hash_get(res, implies[i].implies[j]))
					data_node_hash_add(res, implies[i].implies[j],
							   data_node_string("1"));
			}
		}
	}

	data_node_hash_add(res, "fname", data_node_string(argv[optind]));
	printf("  \"%s\": ", strip_name(argv[optind]));
	data_to_json(res, stdout, 2);
	data_node_free(res);

	return 0;
}
