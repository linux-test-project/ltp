// SPDX-License-Identifier: GPL-2.0-or-later

/*
 * Copyright (C) 2023 Cyril Hrubis <chrubis@suse.cz>
 */

#ifndef TST_FD_H__
#define TST_FD_H__

enum tst_fd_type {
	TST_FD_FILE,
	TST_FD_PATH,
	TST_FD_DIR,
	TST_FD_DEV_ZERO,
	TST_FD_PROC_MAPS,
	TST_FD_PIPE_READ,
	TST_FD_PIPE_WRITE,
	TST_FD_UNIX_SOCK,
	TST_FD_INET_SOCK,
	TST_FD_EPOLL,
	TST_FD_EVENTFD,
	TST_FD_SIGNALFD,
	TST_FD_TIMERFD,
	TST_FD_PIDFD,
	TST_FD_FANOTIFY,
	TST_FD_INOTIFY,
	TST_FD_USERFAULTFD,
	TST_FD_PERF_EVENT,
	TST_FD_IO_URING,
	TST_FD_BPF_MAP,
	TST_FD_FSOPEN,
	TST_FD_FSPICK,
	TST_FD_OPEN_TREE,
	TST_FD_MEMFD,
	TST_FD_MEMFD_SECRET,
	TST_FD_MAX,
};

struct tst_fd {
	enum tst_fd_type type;
	int fd;
	/* used by the library, do not touch! */
	long priv;
};

#define TST_FD_INIT {.type = TST_FD_FILE, .fd = -1}

/*
 * Advances the iterator to the next fd type, returns zero at the end.
 */
int tst_fd_next(struct tst_fd *fd);

#define TST_FD_FOREACH(fd) \
	for (struct tst_fd fd = TST_FD_INIT; tst_fd_next(&fd); )

/*
 * Returns human readable name for the file descriptor type.
 */
const char *tst_fd_desc(struct tst_fd *fd);

#endif /* TST_FD_H__ */
