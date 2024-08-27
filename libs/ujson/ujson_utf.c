// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2022-2024 Cyril Hrubis <metan@ucw.cz>
 */

#include <stddef.h>
#include <ujson_utf.h>

int8_t ujson_utf8_next_chsz(const char *str, size_t off)
{
	char ch = str[off];
	uint8_t len = 0;

	if (!ch)
		return 0;

	if (UJSON_UTF8_IS_ASCII(ch))
		return 1;

	if (UJSON_UTF8_IS_2BYTE(ch)) {
		len = 2;
		goto ret;
	}

	if (UJSON_UTF8_IS_3BYTE(ch)) {
		len = 3;
		goto ret;
	}

	if (UJSON_UTF8_IS_4BYTE(ch)) {
		len = 4;
		goto ret;
	}

	return -1;
ret:
	if (!UJSON_UTF8_IS_NBYTE(str[off+1]))
		return -1;

	if (len > 2 && !UJSON_UTF8_IS_NBYTE(str[off+2]))
		return -1;

	if (len > 3 && !UJSON_UTF8_IS_NBYTE(str[off+3]))
		return -1;

	return len;
}

int8_t ujson_utf8_prev_chsz(const char *str, size_t off)
{
	char ch;

	if (!off)
		return 0;

	ch = str[--off];

	if (UJSON_UTF8_IS_ASCII(ch))
		return 1;

	if (!UJSON_UTF8_IS_NBYTE(ch))
		return -1;

	if (off < 1)
		return -1;

	ch = str[--off];

	if (UJSON_UTF8_IS_2BYTE(ch))
		return 2;

	if (!UJSON_UTF8_IS_NBYTE(ch))
		return -1;

	if (off < 1)
		return -1;

	ch = str[--off];

	if (UJSON_UTF8_IS_3BYTE(ch))
		return 3;

	if (!UJSON_UTF8_IS_NBYTE(ch))
		return -1;

	if (off < 1)
		return -1;

	ch = str[--off];

	if (UJSON_UTF8_IS_4BYTE(ch))
		return 4;

	return -1;
}

size_t ujson_utf8_strlen(const char *str)
{
	size_t cnt = 0;

	while (ujson_utf8_next(&str))
		cnt++;

	return cnt;
}
