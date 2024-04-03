// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2019 Cyril Hrubis <chrubis@suse.cz>
 */

/**
 * DOC: Guarded buffers introduction
 *
 * Guarded buffer has a page with PROT_NONE allocated right before the start of
 * the buffer and canary after the end of the buffer. That means that any
 * memory access before the buffer ends with EFAULT or SEGFAULT and any write
 * after the end of the buffer will be detected because it would overwrite the
 * canaries.
 *
 * It should be used for all buffers passed to syscalls to make sure off-by-one
 * buffer accesses does not happen.
 */

#ifndef TST_BUFFERS_H__
#define TST_BUFFERS_H__

/**
 * struct tst_buffers - A guarded buffer description for allocator.
 *
 * Buffer description consist of a pointer to a pointer and buffer type/size
 * encoded as a different structure members.
 *
 * @ptr: A pointer to the pointer to buffer. This is dereferenced and set by the
 *       allocator.
 * @size: A buffer size in bytes. Only one of size and iov_sizes can be set.
 * @iov_sizes: An -1 terminated array of sizes used to construct a
 *             struct iovec buffers.
 * @str: If size is zero and iov_sizes is NULL this string is going to be
 *       copied into the buffer.
 */
struct tst_buffers {
	void *ptr;
	size_t size;
	int *iov_sizes;
	char *str;
};

/**
 * tst_buffers_alloc() - Allocates buffers based on the tst_buffers structure.
 *
 * @bufs: A NULL terminated array of test buffer descriptions.
 *
 * This is called from the test library if the tst_test.bufs pointer is set.
 */
void tst_buffers_alloc(struct tst_buffers bufs[]);

/**
 * tst_strdup() - Copies a string into a newly allocated guarded buffer.
 *
 * @str: A string to be duplicated.
 * return: A pointer to the string duplicated in a guarded buffer.
 *
 * Allocates a buffer with tst_alloc() and copies the string into it.
 */
char *tst_strdup(const char *str);

/**
 * tst_alloc() - Allocates a guarded buffer.
 *
 * @size: A size of the buffer.
 * return: A newly allocated guarded buffer.
 */
void *tst_alloc(size_t size);

/**
 * tst_aprintf() - Printf into a newly allocated guarded buffer.
 *
 * @fmt: A printf-like format.
 * @...: A printf-like parameters.
 * return: A newly allocated buffer.
 *
 * Allocates a buffer with tst_alloc() then prints the data into it.
 */
char *tst_aprintf(const char *fmt, ...)
      __attribute__((format (printf, 1, 2)));

/**
 * tst_iovec_alloc() - Allocates a complete iovec structure.
 *
 * @sizes: A -1 terminated array of buffer sizes.
 * return: Newly allocated iovec structure.
 */
struct iovec *tst_iovec_alloc(int sizes[]);

/**
 * tst_free_all() - Frees all allocated buffers.
 *
 * It's important to free all guarded buffers because the canaries after the
 * buffer are checked only when the buffer is being freed.
 *
 * This is called at the end of the test automatically.
 */
void tst_free_all(void);

#endif	/* TST_BUFFERS_H__ */
