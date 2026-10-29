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

/*
 * Space allocated large enough to hold a struct pthread.
 *
 * Zero-initialized to ensure THREAD_SELF->cancelhandling starts at 0,
 * avoiding undefined behavior (e.g., in clone10.c) in __pthread_disable_asynccancel(),
 * which is called at thread cancellation points such as write().
 */
#define TLS_PRE_TCB_SIZE (TLS_ALIGN * 256)

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
	char *tls_area = aligned_alloc(TLS_ALIGN, TLS_PRE_TCB_SIZE + TLS_SIZE);
	if (!tls_area)
		tst_brk(TBROK | TERRNO, "aligned_alloc failed");
	memset(tls_area, 0, TLS_PRE_TCB_SIZE + TLS_SIZE);
	tls_area += TLS_PRE_TCB_SIZE;

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
		free(((char *)tls_ptr) - TLS_PRE_TCB_SIZE);
		tls_ptr = NULL;
	}
}

#endif /* LAPI_TLS_H__ */
