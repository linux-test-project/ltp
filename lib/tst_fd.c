// SPDX-License-Identifier: GPL-2.0-or-later

/*
 * Copyright (C) 2023 Cyril Hrubis <chrubis@suse.cz>
 */

#define TST_NO_DEFAULT_MAIN

#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <sys/signalfd.h>
#include <sys/timerfd.h>
#include <sys/inotify.h>
#include <linux/perf_event.h>
#include <linux/fanotify.h>

#include "tst_test.h"
#include "tst_safe_macros.h"

#include "lapi/pidfd.h"
#include "lapi/io_uring.h"
#include "lapi/bpf.h"
#include "lapi/fsmount.h"

#include "tst_fd.h"

struct tst_fd_desc {
	void (*open_fd)(struct tst_fd *fd);
	void (*destroy)(struct tst_fd *fd);
	const char *desc;
};

static void open_file(struct tst_fd *fd)
{
	fd->fd = SAFE_OPEN("fd_file", O_RDWR | O_CREAT, 0666);
	SAFE_UNLINK("fd_file");
}

static void open_path(struct tst_fd *fd)
{
	int tfd;

	tfd = SAFE_CREAT("fd_file", 0666);
	SAFE_CLOSE(tfd);

	fd->fd = SAFE_OPEN("fd_file", O_PATH);

	SAFE_UNLINK("fd_file");
}

static void open_dir(struct tst_fd *fd)
{
	SAFE_MKDIR("fd_dir", 0700);
	fd->fd = SAFE_OPEN("fd_dir", O_DIRECTORY);
	SAFE_RMDIR("fd_dir");
}

static void open_dev_zero(struct tst_fd *fd)
{
	fd->fd = SAFE_OPEN("/dev/zero", O_RDONLY);
}

static void open_proc_self_maps(struct tst_fd *fd)
{
	fd->fd = SAFE_OPEN("/proc/self/maps", O_RDONLY);
}

static void open_pipe_read(struct tst_fd *fd)
{
	int pipe[2];

	SAFE_PIPE(pipe);
	fd->fd = pipe[0];
	fd->priv = pipe[1];
}

static void open_pipe_write(struct tst_fd *fd)
{
	int pipe[2];

	SAFE_PIPE(pipe);
	fd->fd = pipe[1];
	fd->priv = pipe[0];
}

static void destroy_pipe(struct tst_fd *fd)
{
	SAFE_CLOSE(fd->priv);
}

static void open_unix_sock(struct tst_fd *fd)
{
	fd->fd = SAFE_SOCKET(AF_UNIX, SOCK_STREAM, 0);
}

static void open_inet_sock(struct tst_fd *fd)
{
	fd->fd = SAFE_SOCKET(AF_INET, SOCK_STREAM, 0);
}

static void open_epoll(struct tst_fd *fd)
{
	fd->fd = epoll_create(1);

	if (fd->fd < 0)
		tst_res(TCONF | TERRNO, "epoll_create()");
}

static void open_eventfd(struct tst_fd *fd)
{
	fd->fd = eventfd(0, 0);

	if (fd->fd < 0)
		tst_res(TCONF | TERRNO, "Skipping %s", tst_fd_desc(fd));
}

static void open_signalfd(struct tst_fd *fd)
{
	sigset_t sfd_mask;

	sigemptyset(&sfd_mask);

	fd->fd = signalfd(-1, &sfd_mask, 0);
	if (fd->fd < 0) {
		tst_res(TCONF | TERRNO,
			"Skipping %s", tst_fd_desc(fd));
	}
}

static void open_timerfd(struct tst_fd *fd)
{
	fd->fd = timerfd_create(CLOCK_REALTIME, 0);

	if (fd->fd < 0) {
		tst_res(TCONF | TERRNO,
			"Skipping %s", tst_fd_desc(fd));
	}
}

static void open_pidfd(struct tst_fd *fd)
{
	fd->fd = syscall(__NR_pidfd_open, getpid(), 0);
	if (fd->fd < 0)
		tst_res(TCONF | TERRNO, "pidfd_open()");
}

static void open_fanotify(struct tst_fd *fd)
{
	fd->fd = syscall(__NR_fanotify_init, FAN_CLASS_NOTIF, O_RDONLY);
	if (fd->fd < 0) {
		tst_res(TCONF | TERRNO,
			"Skipping %s", tst_fd_desc(fd));
	}
}

static void open_inotify(struct tst_fd *fd)
{
	fd->fd = inotify_init();
	if (fd->fd < 0) {
		tst_res(TCONF | TERRNO,
			"Skipping %s", tst_fd_desc(fd));
	}
}

static void open_userfaultfd(struct tst_fd *fd)
{
	fd->fd = syscall(__NR_userfaultfd, 0);

	if (fd->fd < 0) {
		tst_res(TCONF | TERRNO,
			"Skipping %s", tst_fd_desc(fd));
	}
}

static void open_perf_event(struct tst_fd *fd)
{
	struct perf_event_attr pe_attr = {
		.type = PERF_TYPE_SOFTWARE,
		.size = sizeof(struct perf_event_attr),
		.config = PERF_COUNT_SW_CPU_CLOCK,
		.disabled = 1,
		.exclude_kernel = 1,
		.exclude_hv = 1,
	};

	fd->fd = syscall(__NR_perf_event_open, &pe_attr, 0, -1, -1, 0);
	if (fd->fd < 0) {
		tst_res(TCONF | TERRNO,
			"Skipping %s", tst_fd_desc(fd));
	}
}

static void open_io_uring(struct tst_fd *fd)
{
	struct io_uring_params uring_params = {};

	fd->fd = syscall(__NR_io_uring_setup, 1, &uring_params);
	if (fd->fd < 0) {
		tst_res(TCONF | TERRNO,
			"Skipping %s", tst_fd_desc(fd));
	}
}

static void open_bpf_map(struct tst_fd *fd)
{
	union bpf_attr array_attr = {
		.map_type = BPF_MAP_TYPE_ARRAY,
		.key_size = 4,
		.value_size = 8,
		.max_entries = 1,
	};

	fd->fd = syscall(__NR_bpf, BPF_MAP_CREATE, &array_attr, sizeof(array_attr));
	if (fd->fd < 0) {
		tst_res(TCONF | TERRNO,
			"Skipping %s", tst_fd_desc(fd));
	}
}

static void open_fsopen(struct tst_fd *fd)
{
	fd->fd = syscall(__NR_fsopen, "ext2", 0);
	if (fd->fd < 0) {
		tst_res(TCONF | TERRNO,
			"Skipping %s", tst_fd_desc(fd));
	}
}

static void open_fspick(struct tst_fd *fd)
{
	fd->fd = syscall(__NR_fspick, AT_FDCWD, "/", 0);
	if (fd->fd < 0) {
		tst_res(TCONF | TERRNO,
			"Skipping %s", tst_fd_desc(fd));
	}
}

static void open_open_tree(struct tst_fd *fd)
{
	fd->fd = syscall(__NR_open_tree, AT_FDCWD, "/", 0);
	if (fd->fd < 0) {
		tst_res(TCONF | TERRNO,
			"Skipping %s", tst_fd_desc(fd));
	}
}

static void open_memfd(struct tst_fd *fd)
{
	fd->fd = syscall(__NR_memfd_create, "ltp_memfd", 0);
	if (fd->fd < 0) {
		tst_res(TCONF | TERRNO,
			"Skipping %s", tst_fd_desc(fd));
	}
}

static void open_memfd_secret(struct tst_fd *fd)
{
	fd->fd = syscall(__NR_memfd_secret, 0);
	if (fd->fd < 0) {
		tst_res(TCONF | TERRNO,
			"Skipping %s", tst_fd_desc(fd));
	}
}

static struct tst_fd_desc fd_desc[] = {
	[TST_FD_FILE] = {.open_fd = open_file, .desc = "file"},
	[TST_FD_PATH] = {.open_fd = open_path, .desc = "O_PATH file"},
	[TST_FD_DIR] = {.open_fd = open_dir, .desc = "directory"},
	[TST_FD_DEV_ZERO] = {.open_fd = open_dev_zero, .desc = "/dev/zero"},
	[TST_FD_PROC_MAPS] = {.open_fd = open_proc_self_maps, .desc = "/proc/self/maps"},
	[TST_FD_PIPE_READ] = {.open_fd = open_pipe_read, .desc = "pipe read end", .destroy = destroy_pipe},
	[TST_FD_PIPE_WRITE] = {.open_fd = open_pipe_write, .desc = "pipe write end", .destroy = destroy_pipe},
	[TST_FD_UNIX_SOCK] = {.open_fd = open_unix_sock, .desc = "unix socket"},
	[TST_FD_INET_SOCK] = {.open_fd = open_inet_sock, .desc = "inet socket"},
	[TST_FD_EPOLL] = {.open_fd = open_epoll, .desc = "epoll"},
	[TST_FD_EVENTFD] = {.open_fd = open_eventfd, .desc = "eventfd"},
	[TST_FD_SIGNALFD] = {.open_fd = open_signalfd, .desc = "signalfd"},
	[TST_FD_TIMERFD] = {.open_fd = open_timerfd, .desc = "timerfd"},
	[TST_FD_PIDFD] = {.open_fd = open_pidfd, .desc = "pidfd"},
	[TST_FD_FANOTIFY] = {.open_fd = open_fanotify, .desc = "fanotify"},
	[TST_FD_INOTIFY] = {.open_fd = open_inotify, .desc = "inotify"},
	[TST_FD_USERFAULTFD] = {.open_fd = open_userfaultfd, .desc = "userfaultfd"},
	[TST_FD_PERF_EVENT] = {.open_fd = open_perf_event, .desc = "perf event"},
	[TST_FD_IO_URING] = {.open_fd = open_io_uring, .desc = "io uring"},
	[TST_FD_BPF_MAP] = {.open_fd = open_bpf_map, .desc = "bpf map"},
	[TST_FD_FSOPEN] = {.open_fd = open_fsopen, .desc = "fsopen"},
	[TST_FD_FSPICK] = {.open_fd = open_fspick, .desc = "fspick"},
	[TST_FD_OPEN_TREE] = {.open_fd = open_open_tree, .desc = "open_tree"},
	[TST_FD_MEMFD] = {.open_fd = open_memfd, .desc = "memfd"},
	[TST_FD_MEMFD_SECRET] = {.open_fd = open_memfd_secret, .desc = "memfd secret"},
};

const char *tst_fd_desc(struct tst_fd *fd)
{
	if (fd->type >= ARRAY_SIZE(fd_desc))
		return "invalid";

	return fd_desc[fd->type].desc;
}

int tst_fd_next(struct tst_fd *fd)
{
	size_t len = ARRAY_SIZE(fd_desc);

	if (fd->fd >= 0) {
		SAFE_CLOSE(fd->fd);

		if (fd_desc[fd->type].destroy)
			fd_desc[fd->type].destroy(fd);

		fd->type++;
	}

	for (;;) {
		if (fd->type >= len)
			return 0;

		fd_desc[fd->type].open_fd(fd);

		if (fd->fd >= 0)
			return 1;

		fd->type++;
	}
}
