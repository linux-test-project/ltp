// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2021-2024 Cyril Hrubis <metan@ucw.cz>
 */

/**
 * @file ujson_common.h
 * @brief Common JSON reader/writer definitions.
 */

#ifndef UJSON_COMMON_H
#define UJSON_COMMON_H

/** @brief Maximal error message length. */
#define UJSON_ERR_MAX 128
/** @brief Maximal id string lenght including terminating null element. */
#define UJSON_ID_MAX 64
/** @brief Maximal recursion depth allowed. */
#define UJSON_RECURSION_MAX 128

#define UJSON_ERR_PRINT ujson_err_handler
#define UJSON_ERR_PRINT_PRIV stderr

/**
 * @brief A JSON data type.
 */
enum ujson_type {
	/** @brief No type. Returned when parser finishes. */
	UJSON_VOID = 0,
	/** @brief An integer. */
	UJSON_INT,
	/** @brief A floating point. */
	UJSON_FLOAT,
	/** @brief A boolean. */
	UJSON_BOOL,
	/** @brief NULL */
	UJSON_NULL,
	/** @brief A string. */
	UJSON_STR,
	/** @brief A JSON object. */
	UJSON_OBJ,
	/** @brief A JSON array. */
	UJSON_ARR,
};

/**
 * @brief Returns type name.
 *
 * @param type A json type.
 * @return A type name.
 */
const char *ujson_type_name(enum ujson_type type);

/**
 * @brief Default error print handler.
 *
 * @param print_priv A json buffer print_priv pointer.
 * @param line A line of text to be printed.
 */
void ujson_err_handler(void *print_priv, const char *line);

typedef struct ujson_reader ujson_reader;
typedef struct ujson_writer ujson_writer;
typedef struct ujson_val ujson_val;

/** @brief An array size macro. */
#define UJSON_ARRAY_SIZE(array) (sizeof(array) / sizeof(*array))

#endif /* UJSON_COMMON_H */
