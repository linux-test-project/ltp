// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2020 ARM. All rights reserved.
 * Copyright (c) 2020 Petr Vorel <pvorel@suse.cz>
 *
 * Mostly copied/adapted from <linux/io_uring.h>
 */

#ifndef LAPI_IO_URING_H__
#define LAPI_IO_URING_H__

#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <stdlib.h>
#include <linux/fs.h>

#include "lapi/syscalls.h"

#ifdef HAVE_LINUX_IO_URING_H
#include <linux/io_uring.h>
#endif

#ifndef IOSQE_FIXED_FILE

#ifndef __kernel_rwf_t
typedef int __kernel_rwf_t;
#endif

/*
 * IO submission data structure (Submission Queue Entry)
 */
struct io_uring_sqe {
	uint8_t	opcode;		/* type of operation for this sqe */
	uint8_t	flags;		/* IOSQE_ flags */
	uint16_t	ioprio;		/* ioprio for the request */
	int32_t	fd;		/* file descriptor to do IO on */
	union {
		uint64_t	off;	/* offset into file */
		uint64_t	addr2;
	};
	uint64_t	addr;		/* pointer to buffer or iovecs */
	uint32_t	len;		/* buffer size or number of iovecs */
	union {
		__kernel_rwf_t	rw_flags;
		uint32_t		fsync_flags;
		uint16_t		poll_events;
		uint32_t		sync_range_flags;
		uint32_t		msg_flags;
		uint32_t		timeout_flags;
		uint32_t		accept_flags;
		uint32_t		cancel_flags;
		uint32_t		open_flags;
		uint32_t		statx_flags;
		uint32_t		fadvise_advice;
	};
	uint64_t	user_data;	/* data to be passed back at completion time */
	union {
		struct {
			/* index into fixed buffers, if used */
			uint16_t	buf_index;
			/* personality to use, if used */
			uint16_t	personality;
		};
		uint64_t	__pad2[3];
	};
};

enum {
	IOSQE_FIXED_FILE_BIT,
	IOSQE_IO_DRAIN_BIT,
	IOSQE_IO_LINK_BIT,
};

/*
 * sqe->flags
 */
/* use fixed fileset */
#define IOSQE_FIXED_FILE	(1U << IOSQE_FIXED_FILE_BIT)
/* issue after inflight IO */
#define IOSQE_IO_DRAIN		(1U << IOSQE_IO_DRAIN_BIT)
/* links next sqe */
#define IOSQE_IO_LINK		(1U << IOSQE_IO_LINK_BIT)

/*
 * io_uring_setup() flags
 */
#define IORING_SETUP_IOPOLL	(1U << 0)	/* io_context is polled */
#define IORING_SETUP_SQPOLL	(1U << 1)	/* SQ poll thread */
#define IORING_SETUP_SQ_AFF	(1U << 2)	/* sq_thread_cpu is valid */
#define IORING_SETUP_CQSIZE	(1U << 3)	/* app defines CQ size */
#define IORING_SETUP_CLAMP	(1U << 4)	/* clamp SQ/CQ ring sizes */
#define IORING_SETUP_ATTACH_WQ	(1U << 5)	/* attach to existing wq */

enum {
	IORING_OP_NOP,
	IORING_OP_READV,
	IORING_OP_WRITEV,
	IORING_OP_FSYNC,
	IORING_OP_READ_FIXED,
	IORING_OP_WRITE_FIXED,
	IORING_OP_POLL_ADD,
	IORING_OP_POLL_REMOVE,
	IORING_OP_SYNC_FILE_RANGE,
	IORING_OP_SENDMSG,
	IORING_OP_RECVMSG,
	IORING_OP_TIMEOUT,
	IORING_OP_TIMEOUT_REMOVE,
	IORING_OP_ACCEPT,
	IORING_OP_ASYNC_CANCEL,
	IORING_OP_LINK_TIMEOUT,
	IORING_OP_CONNECT,
	IORING_OP_FALLOCATE,
	IORING_OP_OPENAT,
	IORING_OP_CLOSE,
	IORING_OP_FILES_UPDATE,
	IORING_OP_STATX,
	IORING_OP_READ,
	IORING_OP_WRITE,
	IORING_OP_FADVISE,
	IORING_OP_MADVISE,
	IORING_OP_SEND,
	IORING_OP_RECV,
	IORING_OP_OPENAT2,
	IORING_OP_EPOLL_CTL,

	/* this goes last, obviously */
	IORING_OP_LAST,
};

/*
 * sqe->fsync_flags
 */
#define IORING_FSYNC_DATASYNC	(1U << 0)

/*
 * sqe->timeout_flags
 */
#define IORING_TIMEOUT_ABS	(1U << 0)

/*
 * IO completion data structure (Completion Queue Entry)
 */
struct io_uring_cqe {
	uint64_t	user_data;	/* sqe->data submission passed back */
	int32_t 	res;		/* result code for this event */
	uint32_t	flags;
};

/*
 * Magic offsets for the application to mmap the data it needs
 */
#define IORING_OFF_SQ_RING		0ULL
#define IORING_OFF_CQ_RING		0x8000000ULL
#define IORING_OFF_SQES			0x10000000ULL

/*
 * Filled with the offset for mmap(2)
 */
struct io_sqring_offsets {
	uint32_t head;
	uint32_t tail;
	uint32_t ring_mask;
	uint32_t ring_entries;
	uint32_t flags;
	uint32_t dropped;
	uint32_t array;
	uint32_t resv1;
	uint64_t resv2;
};

/*
 * sq_ring->flags
 */
#define IORING_SQ_NEED_WAKEUP	(1U << 0) /* needs io_uring_enter wakeup */

struct io_cqring_offsets {
	uint32_t head;
	uint32_t tail;
	uint32_t ring_mask;
	uint32_t ring_entries;
	uint32_t overflow;
	uint32_t cqes;
	uint64_t resv[2];
};

/*
 * io_uring_enter(2) flags
 */
#define IORING_ENTER_GETEVENTS	(1U << 0)
#define IORING_ENTER_SQ_WAKEUP	(1U << 1)

/*
 * Passed in for io_uring_setup(2). Copied back with updated info on success
 */
struct io_uring_params {
	uint32_t sq_entries;
	uint32_t cq_entries;
	uint32_t flags;
	uint32_t sq_thread_cpu;
	uint32_t sq_thread_idle;
	uint32_t features;
	uint32_t wq_fd;
	uint32_t resv[3];
	struct io_sqring_offsets sq_off;
	struct io_cqring_offsets cq_off;
};

/*
 * io_uring_params->features flags
 */
#define IORING_FEAT_SINGLE_MMAP		(1U << 0)
#define IORING_FEAT_NODROP		(1U << 1)
#define IORING_FEAT_SUBMIT_STABLE	(1U << 2)
#define IORING_FEAT_RW_CUR_POS		(1U << 3)
#define IORING_FEAT_CUR_PERSONALITY	(1U << 4)

/*
 * io_uring_register(2) opcodes and arguments
 */
#define IORING_REGISTER_BUFFERS		0
#define IORING_UNREGISTER_BUFFERS	1
#define IORING_REGISTER_FILES		2
#define IORING_UNREGISTER_FILES		3
#define IORING_REGISTER_EVENTFD		4
#define IORING_UNREGISTER_EVENTFD	5
#define IORING_REGISTER_FILES_UPDATE	6
#define IORING_REGISTER_EVENTFD_ASYNC	7
#define IORING_REGISTER_PROBE		8
#define IORING_REGISTER_PERSONALITY	9
#define IORING_UNREGISTER_PERSONALITY	10

struct io_uring_files_update {
	uint32_t offset;
	uint32_t resv;
	uint64_t __attribute__((aligned(8))) fds;
};

#define IO_URING_OP_SUPPORTED	(1U << 0)

struct io_uring_probe_op {
	uint8_t op;
	uint8_t resv;
	uint16_t flags;	/* IO_URING_OP_* flags */
	uint32_t resv2;
};

struct io_uring_probe {
	uint8_t last_op;	/* last opcode supported */
	uint8_t ops_len;	/* length of ops[] array below */
	uint16_t resv;
	uint32_t resv2[3];
	struct io_uring_probe_op ops[0];
};

#endif /* IOSQE_FIXED_FILE */

#ifndef IOSQE_IO_HADRLINK
/* like LINK, but stronger */
#define IOSQE_IO_HARDLINK_BIT	3
#define IOSQE_IO_HARDLINK	(1U << IOSQE_IO_HARDLINK_BIT)
#endif /* IOSQE_IO_HADRLINK */

#ifndef IOSQE_ASYNC
/* always go async */
#define IOSQE_ASYNC_BIT		4
#define IOSQE_ASYNC		(1U << IOSQE_ASYNC_BIT)
#endif /* IOSQE_ASYNC */

#ifndef HAVE_IO_URING_REGISTER
static inline int io_uring_register(int fd, unsigned int opcode, void *arg,
	unsigned int nr_args)
{
	return tst_syscall(__NR_io_uring_register, fd, opcode, arg, nr_args);
}
#endif /* HAVE_IO_URING_REGISTER */


#ifndef HAVE_IO_URING_SETUP
static inline int io_uring_setup(unsigned int entries,
	struct io_uring_params *p)
{
	return tst_syscall(__NR_io_uring_setup, entries, p);
}
#endif /* HAVE_IO_URING_SETUP */

#ifndef HAVE_IO_URING_ENTER
static inline int io_uring_enter(int fd, unsigned int to_submit,
	unsigned int min_complete, unsigned int flags, sigset_t *sig)
{
	return tst_syscall(__NR_io_uring_enter, fd, to_submit, min_complete,
			flags, sig, _NSIG / 8);
}
#endif /* HAVE_IO_URING_ENTER */

static inline void io_uring_setup_supported_by_kernel(void)
{
	long ret;
	ret = syscall(__NR_io_uring_setup, NULL, 0);
	if (ret != -1) {
		SAFE_CLOSE(ret);
		return;
	}

	if (errno == ENOSYS) {
		if ((tst_kvercmp(5, 1, 0)) < 0) {
			tst_brk(TCONF,
				"Test not supported on kernel version < v5.1");
		}
		tst_brk(TCONF, "CONFIG_IO_URING not set?");
	}
}

#endif /* LAPI_IO_URING_H__ */
