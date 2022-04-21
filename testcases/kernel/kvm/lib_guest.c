// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2021 SUSE LLC <mdoucha@suse.cz>
 *
 * Minimal testing library for KVM tests
 */

#include "kvm_test.h"

extern char kvm_heap_begin[];

static struct tst_kvm_result *const test_result =
	(struct tst_kvm_result *)KVM_RESULT_BASEADDR;

static char *heap_end = kvm_heap_begin;

static struct tst_intr_handler {
	tst_interrupt_callback callback;
	void *userdata;
} intr_handlers[INTERRUPT_COUNT];

void *memset(void *dest, int val, size_t size)
{
	char *ptr = dest;

	while (size--)
		*ptr++ = val;

	return dest;
}

void *memzero(void *dest, size_t size)
{
	return memset(dest, 0, size);
}

void *memcpy(void *dest, const void *src, size_t size)
{
	char *dptr = dest;
	const char *sptr = src;

	while (size--)
		*dptr++ = *sptr++;

	return dest;
}

char *strcpy(char *dest, const char *src)
{
	char *ret = dest;

	while ((*dest++ = *src++))
		;

	return ret;
}

char *strcat(char *dest, const char *src)
{
	char *ret = dest;

	for (; *dest; dest++)
		;

	strcpy(dest, src);
	return ret;
}

size_t strlen(const char *str)
{
	size_t ret;

	for (ret = 0; str[ret]; ret++)
		;

	return ret;
}

char *ptr2hex(char *dest, uintptr_t val)
{
	unsigned int i;
	uintptr_t tmp;
	char *ret = dest;

	for (i = 4; val >> i; i += 4)
		;

	do {
		i -= 4;
		tmp = (val >> i) & 0xf;
		*dest++ = tmp + (tmp >= 10 ? 'A' - 10 : '0');
	} while (i);

	*dest = '\0';
	return ret;
}

void *tst_heap_alloc_aligned(size_t size, size_t align)
{
	uintptr_t addr = (uintptr_t)heap_end;
	void *ret;

	addr += align - 1;
	addr -= addr % align;
	ret = (void *)addr;
	heap_end = (char *)LTP_ALIGN(addr + size, 4);
	return ret;
}

void *tst_heap_alloc(size_t size)
{
	void *ret = heap_end;

	heap_end += LTP_ALIGN(size, 4);
	return ret;
}

void tst_set_interrupt_callback(unsigned int vector,
	tst_interrupt_callback func, void *userdata)
{
	if (vector >= INTERRUPT_COUNT)
		tst_brk(TBROK, "Set interrupt callback: vector out of range");

	intr_handlers[vector].callback = func;
	intr_handlers[vector].userdata = userdata;
}

static void tst_fatal_error(const char *file, const int lineno,
	const char *message, uintptr_t ip)
{
	test_result->result = TBROK;
	test_result->lineno = lineno;
	test_result->file_addr = (uintptr_t)file;
	strcpy(test_result->message, message);
	strcat(test_result->message, " at address 0x");
	ptr2hex(test_result->message + strlen(test_result->message), ip);
	kvm_yield();
	kvm_exit();
}

void tst_res_(const char *file, const int lineno, int result,
	const char *message)
{
	test_result->result = result;
	test_result->lineno = lineno;
	test_result->file_addr = (uintptr_t)file;
	strcpy(test_result->message, message);
	kvm_yield();
}

void tst_brk_(const char *file, const int lineno, int result,
	const char *message)
{
	tst_res_(file, lineno, result, message);
	kvm_exit();
}

void tst_handle_interrupt(struct kvm_interrupt_frame *ifrm, long vector,
	unsigned long errcode)
{
	uintptr_t ip = kvm_get_interrupt_ip(ifrm);
	const char *iname;
	tst_interrupt_callback callback;
	int ret = 0;

	if (vector < 0 || vector >= INTERRUPT_COUNT)
		tst_fatal_error(__FILE__, __LINE__, "Unexpected interrupt", ip);

	callback = intr_handlers[vector].callback;

	if (callback)
		ret = callback(intr_handlers[vector].userdata, ifrm, errcode);

	iname = tst_interrupt_names[vector];
	iname = iname ? iname : "Unexpected interrupt";

	if (!ret)
		tst_fatal_error(__FILE__, __LINE__, iname, ip);
}
