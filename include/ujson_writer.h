// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2021-2024 Cyril Hrubis <metan@ucw.cz>
 */

/**
 * @file ujson_writer.h
 * @brief A JSON writer.
 *
 * All the function that add values return zero on success and non-zero on a
 * failure. Once an error has happened all subsequent attempts to add more
 * values return with non-zero exit status immediatelly. This is designed
 * so that we can add several values without checking each return value
 * and only check if error has happened at the end of the sequence.
 *
 * Failures may occur:
 * - if we call the functions out of order, e.g. attempt to finish array when
 *   we are not writing out an array.
 * - if we run out of recursion stack
 * - may be propagated from the writer function, e.g. allocation failure, no
 *   space on disk, etc.
 */

#ifndef UJSON_WRITER_H
#define UJSON_WRITER_H

#include <ujson_common.h>

/** @brief A JSON writer */
struct ujson_writer {
	unsigned int depth;
	char depth_type[UJSON_RECURSION_MAX/8];
	char depth_first[UJSON_RECURSION_MAX/8];

	/** Handler to print errors and warnings */
	void (*err_print)(void *err_print_priv, const char *line);
	void *err_print_priv;
	char err[UJSON_ERR_MAX];

	/** Handler to produce JSON output */
	int (*out)(struct ujson_writer *self, const char *buf, size_t buf_size);
	void *out_priv;
};

/**
 * @brief An ujson_writer initializer with default values.
 *
 * @param vout A pointer to function to write out the data.
 * @param vout_priv An user pointer passed to the out function.
 *
 * @return An ujson_writer initialized with default values.
 */
#define UJSON_WRITER_INIT(vout, vout_priv) { \
	.err_print = UJSON_ERR_PRINT, \
	.err_print_priv = UJSON_ERR_PRINT_PRIV, \
	.out = vout, \
	.out_priv = vout_priv \
}

/**
 * @brief Allocates a JSON file writer.
 *
 * The call may fail either when file cannot be opened for writing or if
 * allocation has failed. In all cases errno should be set correctly.
 *
 * @param path A path to the file, file is opened for writing and created if it
 *             does not exist.
 *
 * @return A ujson_writer pointer or NULL in a case of failure.
 */
ujson_writer *ujson_writer_file_open(const char *path);

/**
 * @brief Closes and frees a JSON file writer.
 *
 * @param self A ujson_writer file writer.
 *
 * @return Zero on success, non-zero on a failure and errno is set.
 */
int ujson_writer_file_close(ujson_writer *self);

/**
 * @brief Returns true if writer error happened.
 *
 * @param self A JSON writer.
 *
 * @return True if error has happened.
 */
static inline int ujson_writer_err(ujson_writer *self)
{
	return !!self->err[0];
}

/**
 * @brief Starts a JSON object.
 *
 * For a top level object the id must be NULL, every other object has to have
 * non-NULL id. The call will also fail if maximal recursion depth
 * UJSON_RECURSION_MAX has been reached.
 *
 * @param self A JSON writer.
 * @param id An object name.
 *
 * @return Zero on a success, non-zero otherwise.
 */
int ujson_obj_start(ujson_writer *self, const char *id);

/**
 * @brief Finishes a JSON object.
 *
 * The call will fail if we are currenlty not writing out an object.
 *
 * @param self A JSON writer.
 *
 * @return Zero on success, non-zero otherwise.
 */
int ujson_obj_finish(ujson_writer *self);

/**
 * @brief Starts a JSON array.
 *
 * For a top level array the id must be NULL, every other array has to have
 * non-NULL id. The call will also fail if maximal recursion depth
 * UJSON_RECURSION_MAX has been reached.
 *
 * @param self A JSON writer.
 * @param id An array name.
 *
 * @return Zero on success, non-zero otherwise.
 */
int ujson_arr_start(ujson_writer *self, const char *id);

/**
 * @brief Finishes a JSON array.
 *
 * The call will fail if we are currenlty not writing out an array.
 *
 * @param self A JSON writer.
 *
 * @return Zero on success, non-zero otherwise.
 */
int ujson_arr_finish(ujson_writer *self);

/**
 * @brief Adds a null value.
 *
 * The id must be NULL inside of an array, and must be non-NULL inside of an
 * object.
 *
 * @param self A JSON writer.
 * @param id A null value name.
 *
 * @return Zero on success, non-zero otherwise.
 */
int ujson_null_add(ujson_writer *self, const char *id);

/**
 * @brief Adds an integer value.
 *
 * The id must be NULL inside of an array, and must be non-NULL inside of an
 * object.
 *
 * @param self A JSON writer.
 * @param id An integer value name.
 * @param val An integer value.
 *
 * @return Zero on success, non-zero otherwise.
 */
int ujson_int_add(ujson_writer *self, const char *id, long val);

/**
 * @brief Adds a bool value.
 *
 * The id must be NULL inside of an array, and must be non-NULL inside of an
 * object.
 *
 * @param self A JSON writer.
 * @param id An boolean value name.
 * @param val A boolean value.
 *
 * @return Zero on success, non-zero otherwise.
 */
int ujson_bool_add(ujson_writer *self, const char *id, int val);

/**
 * @brief Adds a float value.
 *
 * The id must be NULL inside of an array, and must be non-NULL inside of an
 * object.
 *
 * @param self A JSON writer.
 * @param id A floating point value name.
 * @param val A floating point value.
 *
 * @return Zero on success, non-zero otherwise.
 */
int ujson_float_add(ujson_writer *self, const char *id, double val);

/**
 * @brief Adds a string value.
 *
 * The id must be NULL inside of an array, and must be non-NULL inside of an
 * object.
 *
 * @param self A JSON writer.
 * @param id A string value name.
 * @param str An UTF8 string value.
 *
 * @return Zero on success, non-zero otherwise.
 */
int ujson_str_add(ujson_writer *self, const char *id, const char *str);

/**
 * @brief Finalizes json writer.
 *
 * Finalizes the json writer, throws possible errors through the error printing
 * function.
 *
 * @param self A JSON writer.
 * @return Overall error value.
 */
int ujson_writer_finish(ujson_writer *self);

#endif /* UJSON_WRITER_H */
