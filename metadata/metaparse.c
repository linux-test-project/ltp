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
#include <errno.h>

#include "data_storage.h"

#define INCLUDE_PATH_MAX 5

static int verbose;
static char *cmdline_includepath[INCLUDE_PATH_MAX];
static unsigned int cmdline_includepaths;
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

static char *next_token2(FILE *f, char *buf, size_t buf_len, struct data_node *doc)
{
	size_t i = 0;
	int c;
	int in_str = 0;

	buf_len--;

	for (;;) {
		c = fgetc(f);

		if (c == EOF)
			goto exit;

		if (in_str) {
			if (c == '"') {
				if (i == 0 || buf[i-1] != '\\')
					goto exit;
			}

			if (i < buf_len)
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
		case '|':
		case '+':
		case '*':
		case '%':
			if (i) {
				ungetc(c, f);
				goto exit;
			}

			if (i < buf_len)
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

static char *next_token(FILE *f, struct data_node *doc)
{
	static char buf[4096];

	return next_token2(f, buf, sizeof(buf), doc);
}

static FILE *open_file(const char *dir, const char *fname)
{
	FILE *f;
	char *path;

	if (asprintf(&path, "%s/%s", dir, fname) < 0)
		return NULL;

	f = fopen(path, "r");

	free(path);

	return f;
}

/**
 * List of includes to be skipped.
 *
 * These define many macros or include many include files that are mostly
 * useless to values expanded in tst_test structure. Or macros that shouldn't
 * be expanded at all.
 */
static const char *skip_includes[] = {
	"\"tst_test.h\"",
	"\"config.h\"",
	"\"tst_taint.h\"",
	NULL
};

static FILE *open_include(FILE *f)
{
	char buf[256], *fname;
	FILE *inc;
	unsigned int i;

	if (!fscanf(f, "%s\n", buf))
		return NULL;

	if (buf[0] != '"')
		return NULL;

	for (i = 0; skip_includes[i]; i++) {
		if (!strcmp(skip_includes[i], buf)) {
			if (verbose)
				fprintf(stderr, "INCLUDE SKIP %s\n", buf);
			return NULL;
		}
	}

	if (!strncmp(buf, "\"lapi/", 6)) {
		if (verbose)
			fprintf(stderr, "INCLUDE SKIP %s\n", buf);
		return NULL;
	}

	fname = buf + 1;

	if (!buf[0])
		return NULL;

	fname[strlen(fname)-1] = 0;

	inc = open_file(includepath, fname);
	if (inc) {
		if (verbose)
			fprintf(stderr, "INCLUDE %s/%s\n", includepath, fname);

		return inc;
	}

	for (i = 0; i < cmdline_includepaths; i++) {
		inc = open_file(cmdline_includepath[i], fname);

		if (!inc)
			continue;

		if (verbose) {
			fprintf(stderr, "INCLUDE %s/%s\n",
				cmdline_includepath[i], fname);
		}

		return inc;
	}

	return NULL;
}

static void close_include(FILE *inc)
{
	if (verbose)
		fprintf(stderr, "INCLUDE END\n");

	fclose(inc);
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

static void finalize_array_entry(char **entry, struct data_node *node)
{
	if (!*entry)
		return;

	data_node_array_add(node, data_node_string(*entry));

	free(*entry);
	*entry = NULL;
}

static void str_append(char **res, char *append)
{
	char *cur_str = *res;

	if (!cur_str) {
		*res = strdup(append);
		if (!*res)
			goto err;
		return;
	}

	if (asprintf(res, "%s%s", cur_str, append) < 0)
		goto err;

	free(cur_str);
	return;
err:
	fprintf(stderr, "Allocation failed :(\n");
	exit(1);
}

static int parse_array(FILE *f, struct data_node *node)
{
	char *token;
	char *entry = NULL;

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

		if (!strcmp(token, "}")) {
			struct data_node *arr_last;

			finalize_array_entry(&entry, node);

			/* Remove NULL terminating entry, if present. */
			arr_last = data_node_array_last(node);
			if (arr_last && arr_last->type == DATA_NULL)
				data_node_array_last_rem(node);

			return 0;
		}

		if (!strcmp(token, ",")) {
			finalize_array_entry(&entry, node);
			continue;
		}

		if (!strcmp(token, "NULL")) {
			data_node_array_add(node, data_node_null());
			continue;
		}

		try_apply_macro(&token);
		str_append(&entry, token);
	}

	return 0;
}

static int parse_get_array_len(FILE *f)
{
	const char *token;
	int cnt = 0, depth = 0, prev_comma = 0;

	if (!(token = next_token(f, NULL)))
		return 0;

	if (strcmp(token, "{"))
		return 0;

	for (;;) {
		if (!(token = next_token(f, NULL)))
			return 0;

		if (!strcmp(token, "{"))
			depth++;

		if (!strcmp(token, "}"))
			depth--;
		else
			prev_comma = 0;

		if (!strcmp(token, ",") && !depth) {
			prev_comma = 1;
			cnt++;
		}

		if (depth < 0)
			return cnt + !prev_comma;
	}
}

static void look_for_array_size(FILE *f, const char *arr_id, struct data_node **res)
{
	const char *token;
	char buf[2][2048] = {};
	int cur_buf = 0;
	int prev_buf = 1;

	for (;;) {
		if (!(token = next_token2(f, buf[cur_buf], sizeof(buf[cur_buf]), NULL)))
			break;

		if (!strcmp(token, "=") && !strcmp(buf[prev_buf], arr_id)) {
			int arr_len = parse_get_array_len(f);

			if (verbose)
				fprintf(stderr, "ARRAY %s LENGTH = %i\n", arr_id, arr_len);

			*res = data_node_int(arr_len);

			break;
		}

		if (strcmp(buf[cur_buf], "]") && strcmp(buf[cur_buf], "[")) {
			cur_buf = !cur_buf;
			prev_buf = !prev_buf;
		}
	}
}

static int parse_array_size(FILE *f, struct data_node **res)
{
	const char *token;
	char *arr_id;
	long pos;
	int hash = 0;

	*res = NULL;

	if (!(token = next_token(f, NULL)))
		return 1;

	if (strcmp(token, "("))
		return 1;

	if (!(token = next_token(f, NULL)))
		return 1;

	arr_id = strdup(token);

	if (verbose)
		fprintf(stderr, "COMPUTING ARRAY '%s' LENGHT\n", arr_id);

	pos = ftell(f);

	rewind(f);

	look_for_array_size(f, arr_id, res);

	if (!*res) {
		FILE *inc;

		rewind(f);

		for (;;) {
			if (!(token = next_token(f, NULL)))
				break;

			if (token[0] == '#') {
				hash = 1;
				continue;
			}

			if (!hash)
				continue;

			if (!strcmp(token, "include")) {
				inc = open_include(f);

				if (inc) {
					look_for_array_size(inc, arr_id, res);
					close_include(inc);
				}
			}

			if (*res)
				break;
		}
	}

	free(arr_id);

	if (fseek(f, pos, SEEK_SET))
		return 1;

	return 0;
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
		} else if (!strcmp(token, "ARRAY_SIZE")) {
			if (parse_array_size(f, &ret))
				return 1;
		} else {
			try_apply_macro(&token);
			ret = data_node_string(token);
		}

		if (!ret)
			continue;

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

	if (name[0] == '_')
		return;

	ENTRY e = {
		.key = strdup(name),
		.data = strdup(val),
	};

	if (verbose)
		fprintf(stderr, " MACRO %s=%s\n", e.key, (char*)e.data);

	hsearch(e, ENTER);
}

static void parse_include_macros(FILE *f, int level)
{
	FILE *inc;
	const char *token;
	int hash = 0;

	/**
	 * Allow only three levels of include indirection.
	 *
	 * Should be more than enough (TM).
	 */
	if (level >= 3)
		return;

	inc = open_include(f);
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
		else if (!strcmp(token, "include"))
			parse_include_macros(inc, level+1);

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
						parse_include_macros(f, 0);
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

static struct typemap {
	const char *id;
	enum data_type type;
} tst_test_typemap[] = {
	{.id = "test_variants", .type = DATA_INT},
	{}
};

static void convert_str2int(struct data_node *res, const char *id, const char *str_val)
{
	long val;
	char *endptr;

	errno = 0;
	val = strtol(str_val, &endptr, 10);

	if (errno || *endptr) {
		fprintf(stderr,	"Cannot convert %s value %s to int!\n", id, str_val);
		exit(1);
	}

	if (verbose)
		fprintf(stderr, "NORMALIZING %s TO INT %li\n", id, val);

	data_node_hash_del(res, id);
	data_node_hash_add(res, id, data_node_int(val));
}

static void check_normalize_types(struct data_node *res)
{
	unsigned int i;

	for (i = 0; tst_test_typemap[i].id; i++) {
		struct data_node *n;
		struct typemap *typemap = &tst_test_typemap[i];

		n = data_node_hash_get(res, typemap->id);
		if (!n)
			continue;

		if (n->type == typemap->type)
			continue;

		if (n->type == DATA_STRING && typemap->type == DATA_INT) {
			convert_str2int(res, typemap->id, n->string.val);
			continue;
		}

		fprintf(stderr, "Cannot convert %s from %s to %s!\n",
			typemap->id, data_type_name(n->type),
			data_type_name(typemap->type));
		exit(1);
	}
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
	printf("-I add include path\n");
	printf("-h prints this help\n\n");
	exit(0);
}

int main(int argc, char *argv[])
{
	unsigned int i, j;
	struct data_node *res;
	int opt;

	while ((opt = getopt(argc, argv, "hI:v")) != -1) {
		switch (opt) {
		case 'h':
			print_help(argv[0]);
		break;
		case 'I':
			if (cmdline_includepaths >= INCLUDE_PATH_MAX) {
				fprintf(stderr, "Too much include paths!");
				exit(1);
			}

			cmdline_includepath[cmdline_includepaths++] = optarg;
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
						argv[optind], implies[i].implies[j]);
			}
		}
	}

	/* Normalize types */
	check_normalize_types(res);

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
