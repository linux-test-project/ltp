// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2021-2024 Cyril Hrubis <metan@ucw.cz>
 */

#include <stdio.h>
#include "ujson_common.h"

void ujson_err_handler(void *err_print_priv, const char *line)
{
	fputs(line, err_print_priv);
	putc('\n', err_print_priv);
}

const char *ujson_type_name(enum ujson_type type)
{
	switch (type) {
	case UJSON_VOID:
		return "void";
	case UJSON_INT:
		return "integer";
	case UJSON_FLOAT:
		return "float";
	case UJSON_BOOL:
		return "boolean";
	case UJSON_NULL:
		return "null";
	case UJSON_STR:
		return "string";
	case UJSON_OBJ:
		return "object";
	case UJSON_ARR:
		return "array";
	default:
		return "invalid";
	}
}
