/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2021 SUSE LLC <mdoucha@suse.cz>
 *
 * Minimal test library for KVM tests
 */

#ifndef KVM_GUEST_H_
#define KVM_GUEST_H_

#include <stdarg.h>

/* The main LTP include dir is intentionally excluded during payload build */
#include "../../../../include/tst_res_flags.h"
#undef TERRNO
#undef TTERRNO
#undef TRERRNO

#define TST_TEST_TCONF(message) \
	void main(void) { tst_brk(TCONF, message); }

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

/* Round x up to the next multiple of a.
 * a must be a power of 2.
 */
#define LTP_ALIGN(x, a)    __LTP_ALIGN_MASK((x), (typeof(x))(a) - 1)
#define __LTP_ALIGN_MASK(x, mask)  (((x) + (mask)) & ~(mask))

#define INTERRUPT_COUNT 32

typedef unsigned long size_t;
typedef long ssize_t;

typedef signed char int8_t;
typedef unsigned char uint8_t;
typedef short int16_t;
typedef unsigned short uint16_t;
typedef int int32_t;
typedef unsigned int uint32_t;
typedef long long int64_t;
typedef unsigned long long uint64_t;
typedef unsigned long uintptr_t;

#define NULL ((void *)0)

void *memset(void *dest, int val, size_t size);
void *memzero(void *dest, size_t size);
void *memcpy(void *dest, const void *src, size_t size);

char *strcpy(char *dest, const char *src);
char *strcat(char *dest, const char *src);
size_t strlen(const char *str);
char *strchr(const char *s, int c);
char *strrchr(const char *s, int c);

int vsprintf(char *dest, const char *fmt, va_list ap);
int sprintf(char *dest, const char *fmt, ...);

/* Exit the VM by looping on a HLT instruction forever */
void kvm_exit(void) __attribute__((noreturn));

/* Exit the VM using the HLT instruction but allow resume */
void kvm_yield(void);

void tst_res_(const char *file, const int lineno, int result,
	const char *fmt, ...)
	__attribute__ ((format (printf, 4, 5)));
#define tst_res(result, fmt, ...) \
	tst_res_(__FILE__, __LINE__, (result), (fmt), ##__VA_ARGS__)

void tst_brk_(const char *file, const int lineno, int result,
	const char *fmt, ...) __attribute__((noreturn))
	__attribute__ ((format (printf, 4, 5)));
#define tst_brk(result, fmt, ...) \
	tst_brk_(__FILE__, __LINE__, (result), (fmt), ##__VA_ARGS__)

/*
 * Send asynchronous notification to host without stopping VM execution and
 * return immediately. The notification must be handled by another host thread.
 * The data argument will be passed to host in test_result->file_addr and
 * can be used to send additional data both ways.
 */
void tst_signal_host(void *data);

/*
 * Call tst_signal_host(data) and wait for host to call
 * tst_kvm_clear_guest_signal().
 */
void tst_wait_host(void *data);

void *tst_heap_alloc_aligned(size_t size, size_t align);
void *tst_heap_alloc(size_t size);

/* Arch dependent: */

struct kvm_interrupt_frame;

typedef int (*tst_interrupt_callback)(void *userdata,
	struct kvm_interrupt_frame *ifrm, unsigned long errcode);

extern const char *tst_interrupt_names[INTERRUPT_COUNT];

void tst_set_interrupt_callback(unsigned int vector,
	tst_interrupt_callback func, void *userdata);

/* Get the instruction pointer from interrupt frame */
uintptr_t kvm_get_interrupt_ip(const struct kvm_interrupt_frame *ifrm);

#endif /* KVM_GUEST_H_ */
