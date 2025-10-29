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

#if defined(__i386__)
#include <asm/ldt.h>
#endif

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
extern struct user_desc *tls_desc;

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
#if defined(__x86_64__) || defined(__aarch64__) || defined(__s390x__)
	tls_ptr = allocate_tls_area();

#elif defined(__i386__)
	tls_ptr = allocate_tls_area();
	tls_desc = SAFE_MALLOC(sizeof(*tls_desc));
	memset(tls_desc, 0, sizeof(*tls_desc));
	tls_desc->entry_number = -1;
	tls_desc->base_addr = (unsigned long)tls_ptr;
	tls_desc->limit = TLS_SIZE;
	tls_desc->seg_32bit = 1;
	tls_desc->contents = 0;
	tls_desc->read_exec_only = 0;
	tls_desc->limit_in_pages = 0;
	tls_desc->seg_not_present = 0;
	tls_desc->useable = 1;

#else
	tst_brk(TCONF, "Unsupported architecture for TLS");
#endif
}

static inline void free_tls(void)
{
	usleep(10000);
#if defined(__x86_64__) || defined(__aarch64__) || defined(__s390x__)
	if (tls_ptr) {
		free(tls_ptr);
		tls_ptr = NULL;
	}
#elif defined(__i386__)
	if (tls_desc) {
		free((void *)(uintptr_t)tls_desc->base_addr);
		free(tls_desc);
		tls_desc = NULL;
	}
#endif
}

#endif /* LAPI_TLS_H__ */
