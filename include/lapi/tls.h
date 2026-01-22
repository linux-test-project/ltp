// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2025 Red Hat Inc. All Rights Reserved.
 * Author: Chunfu Wen <chwen@redhat.com>
 */

/*\
 * CLONE_SETTLS init/alloc/free common functions.
 */

#ifndef LAPI_TLS_H__
#define LAPI_TLS_H__

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

#include "tst_test.h"

#define TLS_SIZE 4096
#define TLS_ALIGN 16

#if defined(__x86_64__)
typedef struct {
	void *tcb;
	void *dtv;
	void *self;
	int multiple_threads;
	char padding[64];
} tcb_t;
#endif

extern void *tls_ptr;

static inline void *allocate_tls_area(void)
{
	void *tls_area = aligned_alloc(TLS_ALIGN, TLS_SIZE);
	if (!tls_area)
		tst_brk(TBROK | TERRNO, "aligned_alloc failed");
	memset(tls_area, 0, TLS_SIZE);

#if defined(__x86_64__)
	tcb_t *tcb = (tcb_t *)tls_area;
	tcb->tcb = tls_area;
	tcb->self = tls_area;
	tcb->multiple_threads = 1;
#endif
	return tls_area;
}

static inline void init_tls(void)
{
	tls_ptr = allocate_tls_area();
}

static inline void free_tls(void)
{
	usleep(10000);
	if (tls_ptr) {
		free(tls_ptr);
		tls_ptr = NULL;
	}
}

#endif /* LAPI_TLS_H__ */
