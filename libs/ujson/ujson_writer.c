// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2021-2024 Cyril Hrubis <metan@ucw.cz>
 */

#include <string.h>
#include <stdarg.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include "ujson_utf.h"
#include "ujson_writer.h"

static inline int get_depth_bit(ujson_writer *self, char *mask)
{
	int depth = self->depth - 1;

	if (depth < 0)
		return -1;

	return !!(mask[depth/8] & (1<<(depth%8)));
}

static inline void set_depth_bit(ujson_writer *self, int val)
{
	if (val)
		self->depth_type[self->depth/8] |= (1<<(self->depth%8));
	else
		self->depth_type[self->depth/8] &= ~(1<<(self->depth%8));

	self->depth_first[self->depth/8] |= (1<<(self->depth%8));

	self->depth++;
}

static inline void clear_depth_bit(ujson_writer *self)
{
	self->depth--;
}

static inline int in_arr(ujson_writer *self)
{
	return !get_depth_bit(self, self->depth_type);
}

static inline int in_obj(ujson_writer *self)
{
	return get_depth_bit(self, self->depth_type);
}

static inline void clear_depth_first(ujson_writer *self)
{
	int depth = self->depth - 1;

	self->depth_first[depth/8] &= ~(1<<(depth%8));
}

static inline int is_first(ujson_writer *self)
{
	int ret = get_depth_bit(self, self->depth_first);

	if (ret == 1)
		clear_depth_first(self);

	return ret;
}

static inline void err(ujson_writer *buf, const char *fmt, ...)
{
	va_list va;

	va_start(va, fmt);
	vsnprintf(buf->err, UJSON_ERR_MAX, fmt, va);
	va_end(va);
}

static inline int is_err(ujson_writer *buf)
{
	return buf->err[0];
}

static inline int out(ujson_writer *self, const char *buf, size_t len)
{
	return self->out(self, buf, len);
}

static inline int out_str(ujson_writer *self, const char *str)
{
	return out(self, str, strlen(str));
}

static inline int out_ch(ujson_writer *self, char ch)
{
	return out(self, &ch, 1);
}

#define ESC_FLUSH(esc_char) do {\
	out(self, val, i); \
	val += i + 1; \
	i = 0; \
	out_str(self, esc_char); \
} while (0)

static inline int out_esc_str(ujson_writer *self, const char *val)
{
	if (out_ch(self, '"'))
		return 1;

	size_t i = 0;
	int8_t next_chsz;

	do {
		next_chsz = ujson_utf8_next_chsz(val, i);

		if (next_chsz == 1) {
			switch (val[i]) {
			case '\"':
				ESC_FLUSH("\\\"");
			break;
			case '\\':
				ESC_FLUSH("\\\\");
			break;
			case '/':
				ESC_FLUSH("\\/");
			break;
			case '\b':
				ESC_FLUSH("\\b");
			break;
			case '\f':
				ESC_FLUSH("\\f");
			break;
			case '\n':
				ESC_FLUSH("\\n");
			break;
			case '\r':
				ESC_FLUSH("\\r");
			break;
			case '\t':
				ESC_FLUSH("\\t");
			break;
			default:
				i += next_chsz;
			}
		} else {
			i += next_chsz;
		}
	} while (next_chsz);

	if (i) {
		if (out(self, val, i))
			return 1;
	}

	if (out_ch(self, '"'))
		return 1;

	return 0;
}

static int do_padd(ujson_writer *self)
{
	unsigned int i;

	for (i = 0; i < self->depth; i++) {
		if (out_ch(self, ' '))
			return 1;
	}

	return 0;
}

static int newline(ujson_writer *self)
{
	if (out_ch(self, '\n'))
		return 0;

	if (do_padd(self))
		return 1;

	return 0;
}

static int add_common(ujson_writer *self, const char *id)
{
	if (is_err(self))
		return 1;

	if (!self->depth) {
		err(self, "Object/Array has to be started first");
		return 1;
	}

	if (in_arr(self)) {
		if (id) {
			err(self, "Array entries can't have id");
			return 1;
		}
	} else {
		if (!id) {
			err(self, "Object entries must have id");
			return 1;
		}
	}

	if (!is_first(self) && out_ch(self, ','))
		return 1;

	if (self->depth && newline(self))
		return 1;

	if (id) {
		if (out_esc_str(self, id))
			return 1;

		if (out_str(self, ": "))
			return 1;
	}

	return 0;
}

int ujson_obj_start(ujson_writer *self, const char *id)
{
	if (self->depth >= UJSON_RECURSION_MAX)
		return 1;

	if (!self->depth && id) {
		err(self, "Top level object cannot have id");
		return 1;
	}

	if (self->depth && add_common(self, id))
		return 1;

	if (out_ch(self, '{'))
		return 1;

	set_depth_bit(self, 1);

	return 0;
}

int ujson_obj_finish(ujson_writer *self)
{
	if (is_err(self))
		return 1;

	if (!in_obj(self)) {
		err(self, "Not in object!");
		return 1;
	}

	int first = is_first(self);

	clear_depth_bit(self);

	if (!first)
		newline(self);

	return out_ch(self, '}');
}

int ujson_arr_start(ujson_writer *self, const char *id)
{
	if (self->depth >= UJSON_RECURSION_MAX) {
		err(self, "Recursion too deep");
		return 1;
	}

	if (!self->depth && id) {
		err(self, "Top level array cannot have id");
		return 1;
	}

	if (self->depth && add_common(self, id))
		return 1;

	if (out_ch(self, '['))
		return 1;

	set_depth_bit(self, 0);

	return 0;
}

int ujson_arr_finish(ujson_writer *self)
{
	if (is_err(self))
		return 1;

	if (!in_arr(self)) {
		err(self, "Not in array!");
		return 1;
	}

	int first = is_first(self);

	clear_depth_bit(self);

	if (!first)
		newline(self);

	return out_ch(self, ']');
}

int ujson_null_add(ujson_writer *self, const char *id)
{
	if (add_common(self, id))
		return 1;

	return out_str(self, "null");
}

int ujson_int_add(ujson_writer *self, const char *id, long val)
{
	char buf[64];

	if (add_common(self, id))
		return 1;

	snprintf(buf, sizeof(buf), "%li", val);

	return out_str(self, buf);
}

int ujson_bool_add(ujson_writer *self, const char *id, int val)
{
	if (add_common(self, id))
		return 1;

	if (val)
		return out_str(self, "true");
	else
		return out_str(self, "false");
}

int ujson_str_add(ujson_writer *self, const char *id, const char *val)
{
	if (add_common(self, id))
		return 1;

	if (out_esc_str(self, val))
		return 1;

	return 0;
}

int ujson_float_add(ujson_writer *self, const char *id, double val)
{
	char buf[64];

	if (add_common(self, id))
		return 1;

	snprintf(buf, sizeof(buf), "%lg", val);

	return out_str(self, buf);
}

int ujson_writer_finish(ujson_writer *self)
{
	if (is_err(self))
		goto err;

	if (self->depth) {
		err(self, "Objects and/or Arrays not finished");
		goto err;
	}

	if (newline(self))
		return 1;

	return 0;
err:
	if (self->err_print)
		self->err_print(self->err_print_priv, self->err);

	return 1;
}

struct json_writer_file {
	int fd;
	size_t buf_used;
	char buf[1024];
};

static int out_writer_file_write(ujson_writer *self, int fd, const char *buf, ssize_t buf_len)
{
	do {
		ssize_t ret = write(fd, buf, buf_len);
		if (ret <= 0) {
			err(self, "Failed to write to a file");
			return 1;
		}

		if (ret > buf_len) {
			err(self, "Wrote more bytes than requested?!");
			return 1;
		}

		buf_len -= ret;
	} while (buf_len);

	return 0;
}

static int out_writer_file(ujson_writer *self, const char *buf, size_t buf_len)
{
	struct json_writer_file *writer_file = self->out_priv;
	size_t buf_size = sizeof(writer_file->buf);
	size_t buf_avail = buf_size - writer_file->buf_used;

	if (buf_len > buf_size/4)
		return out_writer_file_write(self, writer_file->fd, buf, buf_len);

	if (buf_len >= buf_avail) {
		if (out_writer_file_write(self, writer_file->fd,
		                          writer_file->buf, writer_file->buf_used))
			return 1;

		memcpy(writer_file->buf, buf, buf_len);
		writer_file->buf_used = buf_len;
		return 0;
	}

	memcpy(writer_file->buf + writer_file->buf_used, buf, buf_len);
	writer_file->buf_used += buf_len;

	return 0;
}

int ujson_writer_file_close(ujson_writer *self)
{
	struct json_writer_file *writer_file = self->out_priv;
	int saved_errno = 0;

	if (writer_file->buf_used) {
		if (out_writer_file_write(self, writer_file->fd,
		                          writer_file->buf, writer_file->buf_used))

			saved_errno = errno;
	}

	if (close(writer_file->fd)) {
		if (!saved_errno)
			saved_errno = errno;
	}

	free(self);

	if (saved_errno) {
		errno = saved_errno;
		return 1;
	}

	return 0;
}

ujson_writer *ujson_writer_file_open(const char *path)
{
	ujson_writer *ret;
	struct json_writer_file *writer_file;

	ret = malloc(sizeof(ujson_writer) + sizeof(struct json_writer_file));
	if (!ret)
		return NULL;

	writer_file = (void*)ret + sizeof(ujson_writer);

	writer_file->fd = open(path, O_CREAT | O_WRONLY | O_TRUNC, 0664);
	if (!writer_file->fd) {
		free(ret);
		return NULL;
	}

	writer_file->buf_used = 0;

	memset(ret, 0, sizeof(*ret));

	ret->err_print = UJSON_ERR_PRINT;
	ret->err_print_priv = UJSON_ERR_PRINT_PRIV;
	ret->out = out_writer_file;
	ret->out_priv = writer_file;

	return ret;
}
