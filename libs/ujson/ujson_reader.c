// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2021-2024 Cyril Hrubis <metan@ucw.cz>
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdint.h>

#include "ujson_utf.h"
#include "ujson_reader.h"

static const struct ujson_obj empty = {};
const struct ujson_obj *ujson_empty_obj = &empty;

static inline int buf_empty(ujson_reader *buf)
{
	return buf->off >= buf->len;
}

static int eatws(ujson_reader *buf)
{
	while (!buf_empty(buf)) {
		switch (buf->json[buf->off]) {
		case ' ':
		case '\t':
		case '\n':
		case '\r':
		break;
		default:
			goto ret;
		}

		buf->off += 1;
	}
ret:
	return buf_empty(buf);
}

static char getb(ujson_reader *buf)
{
	if (buf_empty(buf))
		return 0;

	return buf->json[buf->off++];
}

static char peekb_off(ujson_reader *buf, size_t off)
{
	if (buf->off + off >= buf->len)
		return 0;

	return buf->json[buf->off + off];
}

static char peekb(ujson_reader *buf)
{
	if (buf_empty(buf))
		return 0;

	return buf->json[buf->off];
}

static int eatb(ujson_reader *buf, char ch)
{
	if (peekb(buf) != ch)
		return 0;

	getb(buf);
	return 1;
}

static int eatb2(ujson_reader *buf, char ch1, char ch2)
{
	if (peekb(buf) != ch1 && peekb(buf) != ch2)
		return 0;

	getb(buf);
	return 1;
}

static int eatstr(ujson_reader *buf, const char *str)
{
	while (*str) {
		if (!eatb(buf, *str))
			return 0;
		str++;
	}

	return 1;
}

static int hex2val(unsigned char b)
{
	switch (b) {
	case '0' ... '9':
		return b - '0';
	case 'a' ... 'f':
		return b - 'a' + 10;
	case 'A' ... 'F':
		return b - 'A' + 10;
	default:
		return -1;
	}
}

static int32_t parse_ucode_cp(ujson_reader *buf)
{
	int ret = 0, v, i;

	for (i = 0; i < 4; i++) {
		if ((v = hex2val(getb(buf))) < 0)
			goto err;
		ret *= 16;
		ret += v;
	}

	return ret;
err:
	ujson_err(buf, "Expected four hexadecimal digits");
	return -1;
}

static unsigned int parse_ucode_esc(ujson_reader *buf, char *str,
                                    size_t off, size_t len)
{
	int32_t ucode = parse_ucode_cp(buf);

	if (ucode < 0)
		return 0;

	if (!str)
		return ucode;

	if (ujson_utf8_bytes(ucode) + 1 >= len - off) {
		ujson_err(buf, "String buffer too short!");
		return 0;
	}

	return ujson_to_utf8(ucode, str+off);
}

static int copy_str(ujson_reader *buf, char *str, size_t len)
{
	size_t pos = 0;
	int esc = 0;
	unsigned int l;

	eatb(buf, '"');

	for (;;) {
		if (buf_empty(buf)) {
			ujson_err(buf, "Unterminated string");
			return 1;
		}

		if (!esc && eatb(buf, '"')) {
			if (str)
				str[pos] = 0;
			return 0;
		}

		unsigned char b = getb(buf);

		if (b < 0x20) {
			if (!peekb(buf))
				ujson_err(buf, "Unterminated string");
			else
				ujson_err(buf, "Invalid string character 0x%02x", b);
			return 1;
		}

		if (!esc && b == '\\') {
			esc = 1;
			continue;
		}

		if (esc) {
			switch (b) {
			case '"':
			case '\\':
			case '/':
			break;
			case 'b':
				b = '\b';
			break;
			case 'f':
				b = '\f';
			break;
			case 'n':
				b = '\n';
			break;
			case 'r':
				b = '\r';
			break;
			case 't':
				b = '\t';
			break;
			case 'u':
				if (!(l = parse_ucode_esc(buf, str, pos, len)))
					return 1;
				pos += l;
				b = 0;
			break;
			default:
				ujson_err(buf, "Invalid escape \\%c", b);
				return 1;
			}
			esc = 0;
		}

		if (str && b) {
			if (pos + 1 >= len) {
				ujson_err(buf, "String buffer too short!");
				return 1;
			}

			str[pos++] = b;
		}
	}

	return 1;
}

static int copy_id_str(ujson_reader *buf, char *str, size_t len)
{
	size_t pos = 0;

	if (eatws(buf))
		goto err0;

	if (!eatb(buf, '"'))
		goto err0;

	for (;;) {
		if (buf_empty(buf)) {
			ujson_err(buf, "Unterminated ID string");
			return 1;
		}

		if (eatb(buf, '"')) {
			str[pos] = 0;
			break;
		}

		if (pos >= len-1) {
			ujson_err(buf, "ID string too long");
			return 1;
		}

		str[pos++] = getb(buf);
	}

	if (eatws(buf))
		goto err1;

	if (!eatb(buf, ':'))
		goto err1;

	return 0;
err0:
	ujson_err(buf, "Expected ID string");
	return 1;
err1:
	ujson_err(buf, "Expected ':' after ID string");
	return 1;
}

static int is_digit(char b)
{
	switch (b) {
	case '0' ... '9':
		return 1;
	default:
		return 0;
	}
}

static int get_int(ujson_reader *buf, struct ujson_val *res)
{
	long val = 0;
	int sign = 1;

	if (eatb(buf, '-')) {
		sign = -1;
		if (!is_digit(peekb(buf))) {
			ujson_err(buf, "Expected digit(s)");
			return 1;
		}
	}

	if (peekb(buf) == '0' && is_digit(peekb_off(buf, 1))) {
		ujson_err(buf, "Leading zero in number!");
		return 1;
	}

	while (is_digit(peekb(buf))) {
		val *= 10;
		val += getb(buf) - '0';
		//TODO: overflow?
	}

	if (sign < 0)
		val = -val;

	res->val_int = val;
	res->val_float = val;

	return 0;
}

static int eat_digits(ujson_reader *buf)
{
	if (!is_digit(peekb(buf))) {
		ujson_err(buf, "Expected digit(s)");
		return 1;
	}

	while (is_digit(peekb(buf)))
		getb(buf);

	return 0;
}

static int get_float(ujson_reader *buf, struct ujson_val *res)
{
	off_t start = buf->off;

	eatb(buf, '-');

	if (peekb(buf) == '0' && is_digit(peekb_off(buf, 1))) {
		ujson_err(buf, "Leading zero in float");
		return 1;
	}

	if (eat_digits(buf))
		return 1;

	switch (getb(buf)) {
	case '.':
		if (eat_digits(buf))
			return 1;

		if (!eatb2(buf, 'e', 'E'))
			break;

		/* fallthrough */
	case 'e':
	case 'E':
		eatb2(buf, '+', '-');

		if (eat_digits(buf))
			return 1;
	break;
	}

	size_t len = buf->off - start;
	char tmp[len+1];

	memcpy(tmp, buf->json + start, len);

	tmp[len] = 0;

	res->val_float = strtod(tmp, NULL);

	return 0;
}

static int get_bool(ujson_reader *buf, struct ujson_val *res)
{
	switch (peekb(buf)) {
	case 'f':
		if (!eatstr(buf, "false")) {
			ujson_err(buf, "Expected 'false'");
			return 1;
		}

		res->val_bool = 0;
	break;
	case 't':
		if (!eatstr(buf, "true")) {
			ujson_err(buf, "Expected 'true'");
			return 1;
		}

		res->val_bool = 1;
	break;
	}

	return 0;
}

static int get_null(ujson_reader *buf)
{
	if (!eatstr(buf, "null")) {
		ujson_err(buf, "Expected 'null'");
		return 1;
	}

	return 0;
}

int ujson_obj_skip(ujson_reader *buf)
{
	struct ujson_val res = {};

	UJSON_OBJ_FOREACH(buf, &res) {
		switch (res.type) {
		case UJSON_OBJ:
			if (ujson_obj_skip(buf))
				return 1;
		break;
		case UJSON_ARR:
			if (ujson_arr_skip(buf))
				return 1;
		break;
		default:
		break;
		}
	}

	return 0;
}

int ujson_arr_skip(ujson_reader *buf)
{
	struct ujson_val res = {};

	UJSON_ARR_FOREACH(buf, &res) {
		switch (res.type) {
		case UJSON_OBJ:
			if (ujson_obj_skip(buf))
				return 1;
		break;
		case UJSON_ARR:
			if (ujson_arr_skip(buf))
				return 1;
		break;
		default:
		break;
		}
	}

	return 0;
}

static enum ujson_type next_num_type(ujson_reader *buf)
{
	size_t off = 0;

	for (;;) {
		char b = peekb_off(buf, off++);

		switch (b) {
		case 0:
		case ',':
			return UJSON_INT;
		case '.':
		case 'e':
		case 'E':
			return UJSON_FLOAT;
		}
	}

	return UJSON_VOID;
}

enum ujson_type ujson_next_type(ujson_reader *buf)
{
	if (eatws(buf)) {
		ujson_err(buf, "Unexpected end");
		return UJSON_VOID;
	}

	char b = peekb(buf);

	switch (b) {
	case '{':
		return UJSON_OBJ;
	case '[':
		return UJSON_ARR;
	case '"':
		return UJSON_STR;
	case '-':
	case '0' ... '9':
		return next_num_type(buf);
	case 'f':
	case 't':
		return UJSON_BOOL;
	break;
	case 'n':
		return UJSON_NULL;
	break;
	default:
		ujson_err(buf, "Expected object, array, number or string");
		return UJSON_VOID;
	}
}

enum ujson_type ujson_reader_start(ujson_reader *buf)
{
	enum ujson_type type = ujson_next_type(buf);

	switch (type) {
	case UJSON_ARR:
	case UJSON_OBJ:
	case UJSON_VOID:
	break;
	default:
		ujson_err(buf, "JSON can start only with array or object");
		type = UJSON_VOID;
	break;
	}

	return type;
}

static int get_value(ujson_reader *buf, struct ujson_val *res)
{
	int ret = 0;

	res->type = ujson_next_type(buf);

	switch (res->type) {
	case UJSON_STR:
		if (copy_str(buf, res->buf, res->buf_size)) {
			res->type = UJSON_VOID;
			return 0;
		}
		res->val_str = res->buf;
		return 1;
	case UJSON_INT:
		ret = get_int(buf, res);
	break;
	case UJSON_FLOAT:
		ret = get_float(buf, res);
	break;
	case UJSON_BOOL:
		ret = get_bool(buf, res);
	break;
	case UJSON_NULL:
		ret = get_null(buf);
	break;
	case UJSON_VOID:
		return 0;
	case UJSON_ARR:
	case UJSON_OBJ:
		buf->sub_off = buf->off;
		return 1;
	}

	if (ret) {
		res->type = UJSON_VOID;
		return 0;
	}

	return 1;
}

static int pre_next(ujson_reader *buf, struct ujson_val *res)
{
	if (!eatb(buf, ',')) {
		ujson_err(buf, "Expected ','");
		res->type = UJSON_VOID;
		return 1;
	}

	if (eatws(buf)) {
		ujson_err(buf, "Unexpected end");
		res->type = UJSON_VOID;
		return 1;
	}

	return 0;
}

static int check_end(ujson_reader *buf, struct ujson_val *res, char b)
{
	if (eatws(buf)) {
		ujson_err(buf, "Unexpected end");
		return 1;
	}

	if (eatb(buf, b)) {
		res->type = UJSON_VOID;
		eatws(buf);
		eatb(buf, 0);
		buf->depth--;
		return 1;
	}

	return 0;
}

/*
 * This is supposed to return a pointer to a string stored as a first member of
 * a structure given an array.
 *
 * e.g.
 *
 *	struct foo {
 *		const char *key;
 *		...
 *	};
 *
 *	const struct foo bar[10] = {...};
 *
 *      // Returns a pointer to the key string in a second structure in bar[].
 *	const char *key = list_elem(bar, sizeof(struct foo), 1);
 */
static inline const char *list_elem(const void *arr, size_t memb_size, size_t idx)
{
	return *(const char**)(arr + idx * memb_size);
}

size_t ujson_lookup(const void *arr, size_t memb_size, size_t list_len,
                    const char *key)
{
	size_t l = 0;
	size_t r = list_len-1;
	size_t mid = -1;

	if (!list_len)
		return (size_t)-1;

	while (r - l > 1) {
		mid = (l+r)/2;

		int ret = strcmp(list_elem(arr, memb_size, mid), key);
		if (!ret)
			return mid;

		if (ret < 0)
			l = mid;
		else
			r = mid;
	}

	if (r != mid && !strcmp(list_elem(arr, memb_size, r), key))
		return r;

	if (l != mid && !strcmp(list_elem(arr, memb_size, l), key))
		return l;

	return -1;
}

static int skip_obj_val(ujson_reader *buf)
{
	struct ujson_val dummy = {};

	if (!get_value(buf, &dummy))
		return 0;

	switch (dummy.type) {
	case UJSON_OBJ:
		return !ujson_obj_skip(buf);
	case UJSON_ARR:
		return !ujson_arr_skip(buf);
	default:
		return 1;
	}
}

static int obj_next(ujson_reader *buf, struct ujson_val *res)
{
	if (copy_id_str(buf, res->id, sizeof(res->id)))
		return 0;

	return get_value(buf, res);
}

static int obj_pre_next(ujson_reader *buf, struct ujson_val *res)
{
	if (ujson_reader_err(buf))
		return 1;

	if (check_end(buf, res, '}'))
		return 1;

	if (pre_next(buf, res))
		return 1;

	return 0;
}

static int obj_next_filter(ujson_reader *buf, struct ujson_val *res,
                           const struct ujson_obj *obj, const struct ujson_obj *ign)
{
	const struct ujson_obj_attr *attr;

	for (;;) {
		if (copy_id_str(buf, res->id, sizeof(res->id)))
			return 0;

		res->idx = obj ? ujson_obj_lookup(obj, res->id) : (size_t)-1;

		if (res->idx != (size_t)-1) {
			if (!get_value(buf, res))
				return 0;

			attr = &obj->attrs[res->idx];

			if (attr->type == UJSON_VOID)
				return 1;

			if (attr->type == res->type)
				return 1;

			if (attr->type == UJSON_FLOAT &&
			    res->type == UJSON_INT)
				return 1;

			ujson_warn(buf, "Wrong '%s' type expected %s",
				     attr->key, ujson_type_name(attr->type));
		} else {
			if (!skip_obj_val(buf))
				return 0;

			if (ign && ujson_obj_lookup(ign, res->id) == (size_t)-1)
				ujson_warn(buf, "Unexpected key '%s'", res->id);
		}

		if (obj_pre_next(buf, res))
			return 0;
	}
}

static int check_err(ujson_reader *buf, struct ujson_val *res)
{
	if (ujson_reader_err(buf)) {
		res->type = UJSON_VOID;
		return 1;
	}

	return 0;
}

int ujson_obj_next_filter(ujson_reader *buf, struct ujson_val *res,
                            const struct ujson_obj *obj, const struct ujson_obj *ign)
{
	if (check_err(buf, res))
		return 0;

	if (obj_pre_next(buf, res))
		return 0;

	return obj_next_filter(buf, res, obj, ign);
}

int ujson_obj_next(ujson_reader *buf, struct ujson_val *res)
{
	if (check_err(buf, res))
		return 0;

	if (obj_pre_next(buf, res))
		return 0;

	return obj_next(buf, res);
}

static int any_first(ujson_reader *buf, char b)
{
	if (eatws(buf)) {
		ujson_err(buf, "Unexpected end");
		return 1;
	}

	if (!eatb(buf, b)) {
		ujson_err(buf, "Expected '%c'", b);
		return 1;
	}

	buf->depth++;

	if (buf->depth > buf->max_depth) {
		ujson_err(buf, "Recursion too deep");
		return 1;
	}

	return 0;
}

int ujson_obj_first_filter(ujson_reader *buf, struct ujson_val *res,
                             const struct ujson_obj *obj, const struct ujson_obj *ign)
{
	if (check_err(buf, res))
		return 0;

	if (any_first(buf, '{'))
		return 0;

	if (check_end(buf, res, '}'))
		return 0;

	return obj_next_filter(buf, res, obj, ign);
}

int ujson_obj_first(ujson_reader *buf, struct ujson_val *res)
{
	if (check_err(buf, res))
		return 0;

	if (any_first(buf, '{'))
		return 0;

	if (check_end(buf, res, '}'))
		return 0;

	return obj_next(buf, res);
}

static int arr_next(ujson_reader *buf, struct ujson_val *res)
{
	return get_value(buf, res);
}

int ujson_arr_first(ujson_reader *buf, struct ujson_val *res)
{
	if (check_err(buf, res))
		return 0;

	if (any_first(buf, '['))
		return 0;

	if (check_end(buf, res, ']'))
		return 0;

	return arr_next(buf, res);
}

int ujson_arr_next(ujson_reader *buf, struct ujson_val *res)
{
	if (check_err(buf, res))
		return 0;

	if (check_end(buf, res, ']'))
		return 0;

	if (pre_next(buf, res))
		return 0;

	return arr_next(buf, res);
}

static void ujson_err_va(ujson_reader *buf, const char *fmt, va_list va)
{
	vsnprintf(buf->err, UJSON_ERR_MAX, fmt, va);
}

void ujson_err(ujson_reader *buf, const char *fmt, ...)
{
	va_list va;

	va_start(va, fmt);
	ujson_err_va(buf, fmt, va);
	va_end(va);
}

static void vprintf_line(ujson_reader *buf, const char *fmt, va_list va)
{
	char line[UJSON_ERR_MAX+1];

	vsnprintf(line, sizeof(line), fmt, va);

	line[UJSON_ERR_MAX] = 0;

	buf->err_print(buf->err_print_priv, line);
}

static void printf_line(ujson_reader *buf, const char *fmt, ...)
{
	va_list va;

	va_start(va, fmt);
	vprintf_line(buf, fmt, va);
	va_end(va);
}

static void printf_json_line(ujson_reader *buf, size_t line_nr, const char *buf_pos)
{
	char line[UJSON_ERR_MAX+1];
	size_t plen, i;

	plen = sprintf(line, "%03zu: ", line_nr);

	for (i = 0; i < UJSON_ERR_MAX-plen && buf_pos[i] && buf_pos[i] != '\n'; i++)
		line[i+plen] = buf_pos[i];

	line[i+plen] = 0;

	buf->err_print(buf->err_print_priv, line);
}

static void print_arrow(ujson_reader *buf, const char *buf_pos, size_t count)
{
	char line[count + 7];
	size_t i;

	/* The '000: ' prefix */
	for (i = 0; i <= 5; i++)
		line[i] = ' ';

	for (i = 0; i < count; i++)
		line[i+5] = buf_pos[i] == '\t' ? '\t' : ' ';

	line[count+5] = '^';
	line[count+6] = 0;

	buf->err_print(buf->err_print_priv, line);
}

#define ERR_LINES 10

#define MIN(A, B) ((A < B) ? (A) : (B))

static void print_snippet(ujson_reader *buf, const char *type)
{
	ssize_t i;
	const char *lines[ERR_LINES] = {};
	size_t cur_line = 0;
	size_t cur_off = 0;
	size_t last_off = buf->off;

	for (;;) {
		lines[(cur_line++) % ERR_LINES] = buf->json + cur_off;

		while (cur_off < buf->len && buf->json[cur_off] != '\n')
			cur_off++;

		if (cur_off >= buf->off)
			break;

		cur_off++;
		last_off = buf->off - cur_off;
	}

	printf_line(buf, "%s at line %03zu", type, cur_line);
	buf->err_print(buf->err_print_priv, "");

	size_t idx = 0;

	for (i = MIN(ERR_LINES, cur_line); i > 0; i--) {
		idx = (cur_line - i) % ERR_LINES;
		printf_json_line(buf, cur_line - i + 1, lines[idx]);
	}

	print_arrow(buf, lines[idx], last_off);
}

void ujson_err_print(ujson_reader *buf)
{
	if (!buf->err_print)
		return;

	print_snippet(buf, "Parse error");
	buf->err_print(buf->err_print_priv, buf->err);
}

void ujson_warn(ujson_reader *buf, const char *fmt, ...)
{
	va_list va;

	if (buf->flags & UJSON_READER_STRICT) {
		va_start(va, fmt);
		ujson_err_va(buf, fmt, va);
		va_end(va);
		return;
	}

	if (!buf->err_print)
		return;

	print_snippet(buf, "Warning");

	va_start(va, fmt);
	vprintf_line(buf, fmt, va);
	va_end(va);
}

void ujson_print(void *err_print_priv, const char *line)
{
	fputs(line, err_print_priv);
	putc('\n', err_print_priv);
}

ujson_reader *ujson_reader_load(const char *path)
{
	int fd = open(path, O_RDONLY);
	ujson_reader *ret;
	ssize_t res;
	off_t len, off = 0;

	if (fd < 0)
		return NULL;

	len = lseek(fd, 0, SEEK_END);
	if (len == (off_t)-1) {
		fprintf(stderr, "lseek() failed\n");
		goto err0;
	}

	if (lseek(fd, 0, SEEK_SET) == (off_t)-1) {
		fprintf(stderr, "lseek() failed\n");
		goto err0;
	}

	ret = malloc(sizeof(ujson_reader) + len + 1);
	if (!ret) {
		fprintf(stderr, "malloc() failed\n");
		goto err0;
	}

	memset(ret, 0, sizeof(*ret));

	ret->buf[len] = 0;
	ret->len = len;
	ret->max_depth = UJSON_RECURSION_MAX;
	ret->json = ret->buf;
	ret->err_print = UJSON_ERR_PRINT;
	ret->err_print_priv = UJSON_ERR_PRINT_PRIV;

	while (off < len) {
		res = read(fd, ret->buf + off, len - off);
		if (res < 0) {
			fprintf(stderr, "read() failed\n");
			goto err1;
		}

		off += res;
	}

	close(fd);

	return ret;
err1:
	free(ret);
err0:
	close(fd);
	return NULL;
}

void ujson_reader_finish(ujson_reader *self)
{
	if (ujson_reader_err(self))
		ujson_err_print(self);
	else if (!ujson_reader_consumed(self)) {
		ujson_warn(self, "Garbage after JSON string!");

		if (ujson_reader_err(self))
			ujson_err_print(self);
	}
}

void ujson_reader_free(ujson_reader *buf)
{
	free(buf);
}

ujson_val *ujson_val_alloc(size_t buf_size)
{
	buf_size = buf_size == 0 ? 4096 : buf_size;
	ujson_val *ret;

	ret = malloc(sizeof(ujson_val) + buf_size);
	if (!ret)
		return NULL;

	memset(ret, 0, sizeof(ujson_val) + buf_size);

	ret->buf = ret->buf__;
	ret->buf_size = buf_size;

	return ret;
}

void ujson_val_free(ujson_val *self)
{
	free(self);
}
