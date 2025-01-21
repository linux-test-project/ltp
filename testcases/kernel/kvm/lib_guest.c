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

int memcmp(const void *a, const void *b, size_t length)
{
	const unsigned char *x = a, *y = b;

	for (; length; x++, y++, length--) {
		if (*x != *y)
			return (int)*x - (int)*y;
	}

	return 0;
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

char *strchr(const char *s, int c)
{
	for (; *s; s++) {
		if (*s == c)
			return (char *)s;
	}

	return NULL;
}

char *strrchr(const char *s, int c)
{
	const char *ret = NULL;

	for (; *s; s++) {
		if (*s == c)
			ret = s;
	}

	return (char *)ret;
}

#if defined(__x86_64__) && !defined(__ILP32__)
uint64_t u64divu16(uint64_t a, uint16_t b)
{
	return a / b;
}

unsigned int u64modu16(uint64_t a, uint16_t b)
{
	return a % b;
}

#else /* defined(__x86_64__) && !defined(__ILP32__) */

/* u64 short division helpers to avoid need to link libgcc on 32bit archs */
uint64_t u64divu16(uint64_t a, uint16_t b)
{
	uint64_t ret = 0;
	uint32_t tmp = a >> 32;

	ret = tmp / b;
	ret <<= 32;
	tmp %= b;
	tmp <<= 16;
	tmp |= (a >> 16) & 0xffff;
	ret |= (tmp / b) << 16;
	tmp %= b;
	tmp <<= 16;
	tmp |= a & 0xffff;
	ret |= tmp / b;
	return ret;
}

unsigned int u64modu16(uint64_t a, uint16_t b)
{
	uint32_t tmp = a >> 32;

	tmp %= b;
	tmp <<= 16;
	tmp |= (a >> 16) & 0xffff;
	tmp %= b;
	tmp <<= 16;
	tmp |= a & 0xffff;
	return tmp % b;
}
#endif /* defined(__x86_64__) && !defined(__ILP32__) */

char *ptr2hex(char *dest, uintptr_t val)
{
	unsigned int i;
	uintptr_t tmp;
	char *ret = dest;

	for (i = 4, tmp = val >> 4; tmp; i += 4, tmp >>= 4)
		;

	do {
		i -= 4;
		tmp = (val >> i) & 0xf;
		*dest++ = tmp + (tmp >= 10 ? 'A' - 10 : '0');
	} while (i);

	*dest = '\0';
	return ret;
}

char *u64tostr(char *dest, uint64_t val, uint16_t base, int caps)
{
	unsigned int i;
	uintptr_t tmp = u64divu16(val, base);
	char hex = caps ? 'A' : 'a';
	char *ret = dest;

	for (i = 1; tmp; i++, tmp = u64divu16(tmp, base))
		;

	dest[i] = '\0';

	do {
		tmp = u64modu16(val, base);
		dest[--i] = tmp + (tmp >= 10 ? hex - 10 : '0');
		val = u64divu16(val, base);
	} while (i);

	return ret;
}

char *i64tostr(char *dest, int64_t val)
{
	if (val < 0) {
		dest[0] = '-';
		u64tostr(dest + 1, -val, 10, 0);
		return dest;
	}

	return u64tostr(dest, val, 10, 0);
}

int vsprintf(char *dest, const char *fmt, va_list ap)
{
	va_list args;
	int ret = 0;
	char conv;
	uint64_t u64val = 0;
	int64_t i64val = 0;
	const char * const uint_conv = "ouxX";

	va_copy(args, ap);

	for (; *fmt; fmt++) {
		if (*fmt != '%') {
			dest[ret++] = *fmt;
			continue;
		}

		conv = 0;
		fmt++;

		switch (*fmt) {
		case '%':
			dest[ret++] = *fmt;
			break;

		case 'c':
			dest[ret++] = va_arg(args, int);
			break;

		case 's':
			strcpy(dest + ret, va_arg(args, const char *));
			ret += strlen(dest + ret);
			break;

		case 'p':
			strcpy(dest + ret, "0x");
			ptr2hex(dest + ret + 2,
				(uintptr_t)va_arg(args, void *));
			ret += strlen(dest + ret);
			break;

		case 'l':
			fmt++;

			switch (*fmt) {
			case 'l':
				fmt++;

				if (*fmt == 'd' || *fmt == 'i') {
					i64val = va_arg(args, long long);
					conv = *fmt;
					break;
				}

				if (strchr(uint_conv, *fmt)) {
					u64val = va_arg(args,
						unsigned long long);
					conv = *fmt;
					break;
				}

				va_end(args);
				return -1;

			case 'd':
			case 'i':
				i64val = va_arg(args, long);
				conv = *fmt;
				break;

			default:
				if (strchr(uint_conv, *fmt)) {
					u64val = va_arg(args,
						unsigned long);
					conv = *fmt;
					break;
				}

				va_end(args);
				return -1;
			}
			break;

		case 'h':
			fmt++;

			switch (*fmt) {
			case 'h':
				fmt++;

				if (*fmt == 'd' || *fmt == 'i') {
					i64val = (signed char)va_arg(args, int);
					conv = *fmt;
					break;
				}

				if (strchr(uint_conv, *fmt)) {
					u64val = (unsigned char)va_arg(args,
						unsigned int);
					conv = *fmt;
					break;
				}

				va_end(args);
				return -1;

			case 'd':
			case 'i':
				i64val = (short int)va_arg(args, int);
				conv = *fmt;
				break;

			default:
				if (strchr(uint_conv, *fmt)) {
					u64val = (unsigned short int)va_arg(
						args, unsigned int);
					conv = *fmt;
					break;
				}

				va_end(args);
				return -1;
			}
			break;

		case 'z':
			fmt++;

			if (*fmt == 'd' || *fmt == 'i') {
				i64val = va_arg(args, ssize_t);
				conv = *fmt;
				break;
			}

			if (strchr(uint_conv, *fmt)) {
				u64val = va_arg(args, size_t);
				conv = *fmt;
				break;
			}

			va_end(args);
			return -1;

		case 'd':
		case 'i':
			i64val = va_arg(args, int);
			conv = *fmt;
			break;

		default:
			if (strchr(uint_conv, *fmt)) {
				u64val = va_arg(args, unsigned int);
				conv = *fmt;
				break;
			}

			va_end(args);
			return -1;
		}

		switch (conv) {
		case 0:
			continue;

		case 'd':
		case 'i':
			i64tostr(dest + ret, i64val);
			ret += strlen(dest + ret);
			break;

		case 'o':
			u64tostr(dest + ret, u64val, 8, 0);
			ret += strlen(dest + ret);
			break;

		case 'u':
			u64tostr(dest + ret, u64val, 10, 0);
			ret += strlen(dest + ret);
			break;

		case 'x':
			u64tostr(dest + ret, u64val, 16, 0);
			ret += strlen(dest + ret);
			break;

		case 'X':
			u64tostr(dest + ret, u64val, 16, 1);
			ret += strlen(dest + ret);
			break;

		default:
			va_end(args);
			return -1;
		}
	}

	va_end(args);
	dest[ret++] = '\0';
	return ret;
}

int sprintf(char *dest, const char *fmt, ...)
{
	va_list args;
	int ret;

	va_start(args, fmt);
	ret = vsprintf(dest, fmt, args);
	va_end(args);
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
	/* Avoid sprintf() here in case of bugs */
	strcpy(test_result->message, message);
	strcat(test_result->message, " at address 0x");
	ptr2hex(test_result->message + strlen(test_result->message), ip);
	kvm_yield();
	kvm_exit();
}

void tst_res_(const char *file, const int lineno, int result,
	const char *fmt, ...)
{
	va_list args;
	int ret;

	va_start(args, fmt);
	test_result->result = result;
	test_result->lineno = lineno;
	test_result->file_addr = (uintptr_t)file;
	ret = vsprintf(test_result->message, fmt, args);
	va_end(args);

	if (ret < 0) {
		tst_brk_(file, lineno, TBROK, "Invalid tst_res() format: %s",
			fmt);
	}

	kvm_yield();
}

void tst_brk_(const char *file, const int lineno, int result,
	const char *fmt, ...)
{
	va_list args;
	int ret;

	va_start(args, fmt);
	test_result->result = result;
	test_result->lineno = lineno;
	test_result->file_addr = (uintptr_t)file;
	ret = vsprintf(test_result->message, fmt, args);
	va_end(args);

	if (ret < 0) {
		test_result->result = TBROK;
		strcpy(test_result->message, "Invalid tst_brk() format: ");
		strcat(test_result->message, fmt);
	}

	kvm_yield();
	kvm_exit();
}

void tst_signal_host(void *data)
{
	test_result->file_addr = (uintptr_t)data;
	test_result->result = KVM_TSYNC;
}

void tst_wait_host(void *data)
{
	volatile int32_t *vres = &test_result->result;

	tst_signal_host(data);

	while (*vres != KVM_TNONE)
		;
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
