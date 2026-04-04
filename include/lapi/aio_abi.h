// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2025 Wei Gao <wegao@suse.com>
 */

#ifndef LAPI_AIO_ABI_H__
#define LAPI_AIO_ABI_H__

#include <endian.h>
#include <linux/aio_abi.h>

#ifndef RWF_NOWAIT
# define RWF_NOWAIT 0x00000008
#endif

struct iocb_fallback {
	uint64_t aio_data;
#if __BYTE_ORDER == __LITTLE_ENDIAN
	uint32_t   aio_key;
	uint32_t aio_rw_flags;
#elif __BYTE_ORDER == __BIG_ENDIAN
	uint32_t aio_rw_flags;
	uint32_t   aio_key;
#else
#error edit for your odd byteorder.
#endif
	uint16_t   aio_lio_opcode;
	int16_t   aio_reqprio;
	uint32_t   aio_fildes;
	uint64_t   aio_buf;
	uint64_t   aio_nbytes;
	int64_t   aio_offset;
	uint64_t   aio_reserved2;
	uint32_t   aio_flags;
	uint32_t   aio_resfd;
};

#ifndef HAVE_STRUCT_IOCB_AIO_RW_FLAGS
typedef struct iocb_fallback iocb;
#else
typedef struct iocb iocb;
#endif

#endif /* LAPI_AIO_ABI_H__ */
