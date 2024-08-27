// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2021-2024 Cyril Hrubis <metan@ucw.cz>
 */

/**
 * @file ujson_reader.h
 * @brief A recursive descend JSON parser.
 *
 * All the function that parse JSON return zero on success and non-zero on a
 * failure. Once an error has happened all subsequent attempts to parse more
 * return with non-zero exit status immediatelly. This is designed so that we
 * can parse several values without checking each return value and only check
 * if error has happened at the end of the sequence.
 */

#ifndef UJSON_READER_H
#define UJSON_READER_H

#include <stdio.h>
#include <ujson_common.h>

/**
 * @brief An ujson_reader initializer with default values.
 *
 * @param buf A pointer to a buffer with JSON data.
 * @param buf_len A JSON data buffer lenght.
 * @param rflags enum ujson_reader_flags.
 *
 * @return An ujson_reader initialized with default values.
 */
#define UJSON_READER_INIT(buf, buf_len, rflags) { \
	.max_depth = UJSON_RECURSION_MAX, \
	.err_print = UJSON_ERR_PRINT, \
	.err_print_priv = UJSON_ERR_PRINT_PRIV, \
	.json = buf, \
	.len = buf_len, \
	.flags = rflags \
}

/** @brief Reader flags. */
enum ujson_reader_flags {
	/** @brief If set warnings are treated as errors. */
	UJSON_READER_STRICT = 0x01,
};

/**
 * @brief A JSON parser internal state.
 */
struct ujson_reader {
	/** Pointer to a null terminated JSON string */
	const char *json;
	/** A length of the JSON string */
	size_t len;
	/** A current offset into the JSON string */
	size_t off;
	/** An offset to the start of the last array or object */
	size_t sub_off;
	/** Recursion depth increased when array/object is entered decreased on leave */
	unsigned int depth;
	/** Maximal recursion depth */
	unsigned int max_depth;

	/** Reader flags. */
	enum ujson_reader_flags flags;

	/** Handler to print errors and warnings */
	void (*err_print)(void *err_print_priv, const char *line);
	void *err_print_priv;

	char err[UJSON_ERR_MAX];
	char buf[];
};

/**
 * @brief An ujson_val initializer.
 *
 * @param sbuf A pointer to a buffer used for string values.
 * @param sbuf_size A length of the buffer used for string values.
 *
 * @return An ujson_val initialized with default values.
 */
#define UJSON_VAL_INIT(sbuf, sbuf_size) { \
	.buf = sbuf, \
	.buf_size = sbuf_size, \
}

/**
 * @brief A parsed JSON key value pair.
 */
struct ujson_val {
	/**
	 * @brief A value type
	 *
	 * UJSON_VALUE_VOID means that no value was parsed.
	 */
	enum ujson_type type;

	/** An user supplied buffer and size to store a string values to. */
	char *buf;
	size_t buf_size;

	/**
	 * @brief An index to attribute list.
	 *
	 * This is set by the ujson_obj_first_filter() and
	 * ujson_obj_next_filter() functions.
	 */
	size_t idx;

	/** An union to store the parsed value into. */
	union {
		/** @brief A boolean value. */
		int val_bool;
		/** @brief An integer value. */
		long long val_int;
		/** @brief A string value. */
		const char *val_str;
	};

	/**
	 * @brief A floating point value.
	 *
	 * Since integer values are subset of floating point values val_float
	 * is always set when val_int was set.
	 */
	double val_float;

	/** @brief An ID for object values */
	char id[UJSON_ID_MAX];

	char buf__[];
};

/**
 * @brief Allocates a JSON value.
 *
 * @param buf_size A maximal buffer size for a string value, pass 0 for default.
 * @return A newly allocated JSON value.
 */
ujson_val *ujson_val_alloc(size_t buf_size);

/**
 * @brief Frees a JSON value.
 *
 * @param self A JSON value previously allocated by ujson_val_alloc().
 */
void ujson_val_free(ujson_val *self);

/**
 * @brief Checks is result has valid type.
 *
 * @param res An ujson value.
 * @return Zero if result is not valid, non-zero otherwise.
 */
static inline int ujson_val_valid(struct ujson_val *res)
{
	return !!res->type;
}

/**
 * @brief Fills the reader error.
 *
 * Once buffer error is set all parsing functions return immediatelly with type
 * set to UJSON_VOID.
 *
 * @param self An ujson_reader
 * @param fmt A printf like format string
 * @param ... A printf like parameters
 */
void ujson_err(ujson_reader *self, const char *fmt, ...)
               __attribute__((format(printf, 2, 3)));

/**
 * @brief Prints error stored in the buffer.
 *
 * The error takes into consideration the current offset in the buffer and
 * prints a few preceding lines along with the exact position of the error.
 *
 * The error is passed to the err_print() handler.
 *
 * @param self A ujson_reader
 */
void ujson_err_print(ujson_reader *self);

/**
 * @brief Prints a warning.
 *
 * Uses the print handler in the buffer to print a warning along with a few
 * lines of context from the JSON at the current position.
 *
 * @param self A ujson_reader
 * @param fmt A printf-like error string.
 * @param ... A printf-like parameters.
 */
void ujson_warn(ujson_reader *self, const char *fmt, ...)
               __attribute__((format(printf, 2, 3)));

/**
 * @brief Returns true if error was encountered.
 *
 * @param self A ujson_reader
 * @return True if error was encountered false otherwise.
 */
static inline int ujson_reader_err(ujson_reader *self)
{
	return !!self->err[0];
}

/**
 * @brief Returns the type of next element in buffer.
 *
 * @param self An ujson_reader
 * @return A type of next element in the buffer.
 */
enum ujson_type ujson_next_type(ujson_reader *self);

/**
 * @brief Returns if first element in JSON is object or array.
 *
 * @param self A ujson_reader
 * @return On success returns UJSON_OBJ or UJSON_ARR. On failure UJSON_VOID.
 */
enum ujson_type ujson_reader_start(ujson_reader *self);

/**
 * @brief Starts parsing of a JSON object.
 *
 * @param self An ujson_reader
 * @param res An ujson_val to store the parsed value to.
 *
 * @return Zero on success, non-zero otherwise.
 */
int ujson_obj_first(ujson_reader *self, struct ujson_val *res);

/**
 * @brief Parses next value from a JSON object.
 *
 * If the res->type is UJSON_OBJ or UJSON_ARR it has to be parsed or skipped
 * before next call to this function.
 *
 * @param self An ujson_reader.
 * @param res A ujson_val to store the parsed value to.
 *
 * @return Zero on success, non-zero otherwise.
 */
int ujson_obj_next(ujson_reader *self, struct ujson_val *res);

/**
 * @brief A loop over a JSON object.
 *
 * @code
 * UJSON_OBJ_FOREACH(reader, val) {
 *	printf("Got value id '%s' type '%s'", val->id, ujson_type_name(val->type));
 *	...
 * }
 * @endcode
 *
 * @param self An ujson_reader.
 * @param res An ujson_val to store the next parsed value to.
 */
#define UJSON_OBJ_FOREACH(self, res) \
	for (ujson_obj_first(self, res); ujson_val_valid(res); ujson_obj_next(self, res))

/**
 * @brief Utility function for log(n) lookup in a sorted array.
 *
 * @param list Analphabetically sorted array.
 * @param list_len Array length.
 *
 * @return An array index or (size_t)-1 if key wasn't found.
 */
size_t ujson_lookup(const void *arr, size_t memb_size, size_t list_len,
                    const char *key);

/**
 * @brief A JSON object attribute description i.e. key and type.
 */
typedef struct ujson_obj_attr {
	/** @brief A JSON object key name. */
	const char *key;
	/**
	 * @brief A JSON object value type.
	 *
	 * Note that because integer numbers are subset of floating point
         * numbers if requested type was UJSON_FLOAT it will match if parsed
         * type was UJSON_INT and the val_float will be set in addition to
         * val_int.
         */
	enum ujson_type type;
} ujson_obj_attr;

/** @brief A JSON object description */
typedef struct ujson_obj {
	/**
	 * @brief A list of attributes.
	 *
	 * Attributes we are looking for, the parser sets the val->idx for these.
	 */
	const ujson_obj_attr *attrs;
	/** @brief A size of attrs array. */
	size_t attr_cnt;
} ujson_obj;

static inline size_t ujson_obj_lookup(const ujson_obj *obj, const char *key)
{
	return ujson_lookup(obj->attrs, sizeof(*obj->attrs), obj->attr_cnt, key);
}

/** @brief An ujson_obj_attr initializer. */
#define UJSON_OBJ_ATTR(keyv, typev) \
	{.key = keyv, .type = typev}

/** @brief An ujson_obj_attr intializer with an array index. */
#define UJSON_OBJ_ATTR_IDX(key_idx, keyv, typev) \
	[key_idx] = {.key = keyv, .type = typev}

/**
 * @brief Starts parsing of a JSON object with attribute lists.
 *
 * @param self An ujson_reader.
 * @param res An ujson_val to store the parsed value to.
 * @param obj An ujson_obj object description.
 * @param ign A list of keys to ignore.
 *
 * @return Zero on success, non-zero otherwise.
 */
int ujson_obj_first_filter(ujson_reader *self, struct ujson_val *res,
                           const struct ujson_obj *obj, const struct ujson_obj *ign);

/**
 * @brief An empty object attribute list.
 *
 * To be passed to UJSON_OBJ_FOREACH_FITLER() as ignore list.
 */
extern const struct ujson_obj *ujson_empty_obj;

/**
 * @brief Parses next value from a JSON object with attribute lists.
 *
 * If the res->type is UJSON_OBJ or UJSON_ARR it has to be parsed or skipped
 * before next call to this function.
 *
 * @param self An ujson_reader.
 * @param res An ujson_val to store the parsed value to.
 * @param obj An ujson_obj object description.
 * @param ign A list of keys to ignore. If set to NULL all unknown keys are
 *            ignored, if set to ujson_empty_obj all unknown keys produce warnings.
 *
 * @return Zero on success, non-zero otherwise.
 */
int ujson_obj_next_filter(ujson_reader *self, struct ujson_val *res,
                          const struct ujson_obj *obj, const struct ujson_obj *ign);

/**
 * @brief A loop over a JSON object with a pre-defined list of expected attributes.
 *
 * @code
 * static struct ujson_obj_attr attrs[] = {
 *	UJSON_OBJ_ATTR("bool", UJSON_BOOL),
 *	UJSON_OBJ_ATTR("number", UJSON_INT),
 * };
 *
 * static struct ujson_obj obj = {
 *	.attrs = filter_attrs,
 *	.attr_cnt = UJSON_ARRAY_SIZE(filter_attrs)
 * };
 *
 * UJSON_OBJ_FOREACH_FILTER(reader, val, &obj, NULL) {
 *	printf("Got value id '%s' type '%s'",
 *	       attrs[val->idx].id, ujson_type_name(val->type));
 *	...
 * }
 * @endcode
 *
 * @param self An ujson_reader.
 * @param res An ujson_val to store the next parsed value to.
 * @param obj An ujson_obj with a description of attributes to parse.
 * @param ign An ujson_obj with a description of attributes to ignore.
 */
#define UJSON_OBJ_FOREACH_FILTER(self, res, obj, ign) \
	for (ujson_obj_first_filter(self, res, obj, ign); \
	     ujson_val_valid(res); \
	     ujson_obj_next_filter(self, res, obj, ign))

/**
 * @brief Skips parsing of a JSON object.
 *
 * @param self An ujson_reader.
 *
 * @return Zero on success, non-zero otherwise.
 */
int ujson_obj_skip(ujson_reader *self);

/**
 * @brief Starts parsing of a JSON array.
 *
 * @param self An ujson_reader.
 * @param res An ujson_val to store the parsed value to.
 *
 * @return Zero on success, non-zero otherwise.
 */
int ujson_arr_first(ujson_reader *self, struct ujson_val *res);

/**
 * @brief Parses next value from a JSON array.
 *
 * If the res->type is UJSON_OBJ or UJSON_ARR it has to be parsed or skipped
 * before next call to this function.
 *
 * @param self An ujson_reader.
 * @param res An ujson_val to store the parsed value to.
 *
 * @return Zero on success, non-zero otherwise.
 */
int ujson_arr_next(ujson_reader *self, struct ujson_val *res);

/**
 * @brief A loop over a JSON array.
 *
 * @code
 * UJSON_ARR_FOREACH(reader, val) {
 *	printf("Got value type '%s'", ujson_type_name(val->type));
 *	...
 * }
 * @endcode
 *
 * @param self An ujson_reader.
 * @param res An ujson_val to store the next parsed value to.
 */
#define UJSON_ARR_FOREACH(self, res) \
	for (ujson_arr_first(self, res); ujson_val_valid(res); ujson_arr_next(self, res))

/**
 * @brief Skips parsing of a JSON array.
 *
 * @param self A ujson_reader.
 *
 * @return Zero on success, non-zero otherwise.
 */
int ujson_arr_skip(ujson_reader *self);

/**
 * @brief A JSON reader state.
 */
typedef struct ujson_reader_state {
	size_t off;
	unsigned int depth;
} ujson_reader_state;

/**
 * @brief Returns a parser state at the start of current object/array.
 *
 * This function could be used for the parser to return to the start of the
 * currently parsed object or array.
 *
 * @param self A ujson_reader
 * @return A state that points to a start of the last object or array.
 */
static inline ujson_reader_state ujson_reader_state_save(ujson_reader *self)
{
	struct ujson_reader_state ret = {
		.off = self->sub_off,
		.depth = self->depth,
	};

	return ret;
}

/**
 * @brief Returns the parser to a saved state.
 *
 * This function could be used for the parser to return to the start of
 * object or array saved by t the ujson_reader_state_get() function.
 *
 * @param self A ujson_reader
 * @param state An parser state as returned by the ujson_reader_state_get().
 */
static inline void ujson_reader_state_load(ujson_reader *self, ujson_reader_state state)
{
	if (ujson_reader_err(self))
		return;

	self->off = state.off;
	self->sub_off = state.off;
	self->depth = state.depth;
}

/**
 * @brief Resets the parser to a start.
 *
 * @param self A ujson_reader
 */
static inline void ujson_reader_reset(ujson_reader *self)
{
	self->off = 0;
	self->sub_off = 0;
	self->depth = 0;
	self->err[0] = 0;
}

/**
 * @brief Loads a file into an ujson_reader buffer.
 *
 * The reader has to be later freed by ujson_reader_free().
 *
 * @param path A path to a file.
 * @return A ujson_reader or NULL in a case of a failure.
 */
ujson_reader *ujson_reader_load(const char *path);

/**
 * @brief Frees an ujson_reader buffer.
 *
 * @param self A ujson_reader allocated by ujson_reader_load() function.
 */
void ujson_reader_free(ujson_reader *self);

/**
 * @brief Prints errors and warnings at the end of parsing.
 *
 * Checks if self->err is set and prints the error with ujson_reader_err()
 *
 * Checks if there is any text left after the parser has finished with
 * ujson_reader_consumed() and prints a warning if there were any non-whitespace
 * characters left.
 *
 * @param self A ujson_reader
 */
void ujson_reader_finish(ujson_reader *self);

/**
 * @brief Returns non-zero if whole buffer has been consumed.
 *
 * @param self A ujson_reader.
 * @return Non-zero if whole buffer was consumed.
 */
static inline int ujson_reader_consumed(ujson_reader *self)
{
	return self->off >= self->len;
}

#endif /* UJSON_H */
