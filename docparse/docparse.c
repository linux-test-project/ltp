// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2019 Cyril Hrubis <chrubis@suse.cz>
 * Copyright (c) 2020 Petr Vorel <pvorel@suse.cz>
 */

#include <stdio.h>
#include <string.h>
#include <libgen.h>
#include <ctype.h>
#include <unistd.h>

#include "data_storage.h"

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
		while (isspace(c[i+1]))
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
				data_node_array_add(doc, line);
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

const char *next_token(FILE *f, struct data_node *doc)
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

#define WARN(str) fprintf(stderr, str "\n")

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

static int parse_test_struct(FILE *f, struct data_node *doc, struct data_node *node)
{
	const char *token;
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

static struct data_node *parse_file(const char *fname)
{
	int state = 0, found = 0;
	const char *token;

	if (access(fname, F_OK)) {
		fprintf(stderr, "file %s is not exist\n", fname);
		return NULL;
	}

	FILE *f = fopen(fname, "r");

	struct data_node *res = data_node_hash();
	struct data_node *doc = data_node_array();

	while ((token = next_token(f, doc))) {
		if (state < 6 && !strcmp(tokens[state], token))
			state++;
		else
			state = 0;

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
	const char *implies;
} implies[] = {
	{"format_device", "needs_device"},
	{"mount_device", "needs_device"},
	{"mount_device", "format_device"},
	{"all_filesystems", "needs_device"},
	{"needs_device", "needs_tmpdir"},
	{NULL, NULL}
};

const char *strip_name(char *path)
{
	char *name = basename(path);
	size_t len = strlen(name);

	if (len > 2 && name[len-1] == 'c' && name[len-2] == '.')
		name[len-2] = '\0';

	return name;
}

int main(int argc, char *argv[])
{
	unsigned int i;
	struct data_node *res;

	if (argc != 2) {
		fprintf(stderr, "Usage: docparse filename.c\n");
		return 1;
	}

	res = parse_file(argv[1]);
	if (!res)
		return 0;

	/* Filter out useless data */
	for (i = 0; filter_out[i]; i++)
		data_node_hash_del(res, filter_out[i]);

	/* Normalize the result */
	for (i = 0; implies[i].flag; i++) {
		if (data_node_hash_get(res, implies[i].flag) &&
		    data_node_hash_get(res, implies[i].implies))
			fprintf(stderr, "%s: useless tag: %s\n", argv[1], implies[i].implies);
	}

	for (i = 0; implies[i].flag; i++) {
		if (data_node_hash_get(res, implies[i].flag) &&
		    !data_node_hash_get(res, implies[i].implies))
			data_node_hash_add(res, implies[i].implies, data_node_string("1"));
	}

	data_node_hash_add(res, "fname", data_node_string(argv[1]));
	printf("  \"%s\": ", strip_name(argv[1]));
	data_to_json(res, stdout, 2);
	data_node_free(res);

	return 0;
}
