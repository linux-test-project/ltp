// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2019 Cyril Hrubis <chrubis@suse.cz>
 */

#ifndef TST_BUFFERS_H__
#define TST_BUFFERS_H__

/*
 * Buffer description consist of a pointer to a pointer and buffer type/size
 * encoded as a different structure members.
 *
 * Only one of the size and iov_sizes can be set at a time.
 */
struct tst_buffers {
	/*
	 * This pointer points to a buffer pointer.
	 */
	void *ptr;
	/*
	 * Buffer size.
	 */
	size_t size;
	/*
	 * Array of iov buffer sizes terminated by -1.
	 */
	int *iov_sizes;
	/*
	 * If size and iov_sizes is NULL this is the string we want to strdup()
	 * into the buffer.
	 */
	char *str;
};

/*
 * Allocates buffers based on the tst_buffers structure.
 *
 * @bufs NULL terminated array of test buffer descriptions.
 *
 * This is called from the test library if the tst_test->bufs pointer is set.
 */
void tst_buffers_alloc(struct tst_buffers bufs[]);

/*
 * strdup() that callls tst_alloc().
 */
char *tst_strdup(const char *str);

/*
 * Allocates size bytes, returns pointer to the allocated buffer.
 */
void *tst_alloc(size_t size);

/*
 * Printf into a guarded buffer.
 */
char *tst_aprintf(const char *fmt, ...)
      __attribute__((format (printf, 1, 2)));

/*
 * Allocates iovec structure including the buffers.
 *
 * @sizes -1 terminated array of buffer sizes.
 */
struct iovec *tst_iovec_alloc(int sizes[]);

/*
 * Frees all allocated buffers.
 *
 * This is called at the end of the test automatically.
 */
void tst_free_all(void);

#endif	/* TST_BUFFERS_H__ */
