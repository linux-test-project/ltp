// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2026 Hungyu Lin <dennylin0707@gmail.com>
 */

/*\
 * Verify that two IN_MOVED_FROM events that differ only in their rename
 * cookie are not coalesced.
 *
 * :manpage:`inotify(7)` allows coalescing only of events with identical wd,
 * mask, cookie and name. A regression introduced in v2.6.31 dropped the cookie
 * comparison. On affected kernels, the second IN_MOVED_FROM is coalesced into
 * the first, leaving the following IN_MOVED_TO without a matching IN_MOVED_FROM
 * cookie.
 *
 * With source/ and target/ watched for IN_MOVE, and without reading the queue:
 *
 * 1. Move source/entry to unwatched/ (IN_MOVED_FROM, cookie A).
 * 2. Recreate source/entry.
 * 3. Move source/entry to target/ (IN_MOVED_FROM and IN_MOVED_TO, cookie B).
 *
 * All three events must be queued, with A != B. IN_CREATE is not watched, so
 * the two IN_MOVED_FROM events are adjacent; the queue is not read in between,
 * because only the last unread event can be merged with a new one. Both a
 * regular file and a directory are tested.
 */

#include "config.h"
#include <fcntl.h>
#include <string.h>
#include "tst_test.h"
#include "inotify.h"

#ifdef HAVE_SYS_INOTIFY_H
#include <sys/inotify.h>

#define SOURCE "source/entry"
#define UNWATCHED "unwatched/entry"
#define TARGET "target/entry"

struct event_t {
	const char *name;
	unsigned int mask;
	int wd;
};

static struct tcase {
	const char *type;
	unsigned int isdir;
} tcases[] = {
	{ "file", 0 },
	{ "directory", IN_ISDIR },
};

static int fd_notify = -1;

static void create_entry(unsigned int isdir, const char *path)
{
	if (isdir)
		SAFE_MKDIR(path, 0700);
	else
		SAFE_TOUCH(path, 0600, NULL);
}

static void delete_entry(unsigned int isdir, const char *path)
{
	if (isdir)
		SAFE_RMDIR(path);
	else
		SAFE_UNLINK(path);
}

static void verify_inotify(unsigned int n)
{
	const char *type = tcases[n].type;
	unsigned int isdir = tcases[n].isdir;
	struct event_t event_set[] = {
		{ "entry", IN_MOVED_FROM | isdir, 0 },
		{ "entry", IN_MOVED_FROM | isdir, 0 },
		{ "entry", IN_MOVED_TO | isdir, 0 },
	};
	union {
		struct inotify_event align;
		char data[4096];
	} buf;
	struct inotify_event *event;
	const struct event_t *expected;
	unsigned int cookies[ARRAY_SIZE(event_set)] = { 0 };
	unsigned int count = 0, i;
	size_t pos = 0, len;
	int wd_source, wd_target;

	fd_notify = SAFE_MYINOTIFY_INIT1(O_NONBLOCK);
	wd_source = SAFE_MYINOTIFY_ADD_WATCH(fd_notify, "source", IN_MOVE);
	wd_target = SAFE_MYINOTIFY_ADD_WATCH(fd_notify, "target", IN_MOVE);

	event_set[0].wd = wd_source;
	event_set[1].wd = wd_source;
	event_set[2].wd = wd_target;

	create_entry(isdir, SOURCE);
	SAFE_RENAME(SOURCE, UNWATCHED);
	create_entry(isdir, SOURCE);
	SAFE_RENAME(SOURCE, TARGET);

	len = SAFE_READ(SAFE_READ_ANY_EAGAIN, fd_notify, buf.data, sizeof(buf.data));
	SAFE_CLOSE(fd_notify);

	/* Reuse these paths for the next case and for -i iterations. */
	delete_entry(isdir, UNWATCHED);
	delete_entry(isdir, TARGET);

	while (pos < len) {
		if (len - pos < sizeof(*event))
			tst_brk(TBROK, "%s: event %u has %zu header bytes (expected at least %zu)",
				type, count, len - pos, sizeof(*event));

		event = (struct inotify_event *)(buf.data + pos);
		if (event->len > len - pos - sizeof(*event))
			tst_brk(TBROK, "%s: event %u claims %u name bytes but only %zu are left in the buffer, wd=%d mask=%#x cookie=%u",
				type, count, event->len, len - pos - sizeof(*event),
				event->wd, event->mask, event->cookie);

		if (count >= ARRAY_SIZE(event_set)) {
			tst_res(TFAIL, "%s: got unnecessary event %u: mask=%#x wd=%d cookie=%u len=%u name=\"%.*s\"",
				type, count, event->mask, event->wd, event->cookie,
				event->len, (int)event->len, event->name);
			return;
		}

		expected = &event_set[count];
		if (event->mask != expected->mask) {
			tst_res(TFAIL, "%s: got event %u: mask=%#x (expected %#x) wd=%d cookie=%u name=\"%.*s\"",
				type, count, event->mask, expected->mask, event->wd,
				event->cookie, (int)event->len, event->name);
			return;
		}

		if (event->wd != expected->wd) {
			tst_res(TFAIL, "%s: got event %u: mask=%#x wd=%d (expected %d) cookie=%u name=\"%.*s\"",
				type, count, event->mask, event->wd, expected->wd,
				event->cookie, (int)event->len, event->name);
			return;
		}

		if (event->len < strlen(expected->name) + 1 ||
		    strncmp(expected->name, event->name, event->len)) {
			tst_res(TFAIL, "%s: got event %u: mask=%#x wd=%d cookie=%u len=%u name=\"%.*s\" (expected \"%s\")",
				type, count, event->mask, event->wd, event->cookie,
				event->len, (int)event->len, event->name,
				expected->name);
			return;
		}

		cookies[count] = event->cookie;
		tst_res(TINFO, "%s: event %u: wd=%d mask=%#x cookie=%u len=%u name=\"%.*s\"",
			type, count, event->wd, event->mask, event->cookie,
			event->len, (int)event->len, event->name);
		count++;
		pos += sizeof(*event) + event->len;
	}

	for (i = count; i < ARRAY_SIZE(event_set); i++) {
		expected = &event_set[i];
		tst_res(TFAIL, "%s: didn't get event %u: mask=%#x wd=%d name=\"%s\"",
			type, i, expected->mask, expected->wd, expected->name);
	}

	if (count < ARRAY_SIZE(event_set))
		return;

	if (cookies[0] == cookies[1]) {
		tst_res(TFAIL, "%s: got second FROM cookie=%u (expected != %u)",
			type, cookies[1], cookies[0]);
		return;
	}

	if (cookies[1] != cookies[2]) {
		tst_res(TFAIL, "%s: got TO cookie=%u (expected %u from second FROM)",
			type, cookies[2], cookies[1]);
		return;
	}

	tst_res(TPASS, "%s: FROM cookies %u and %u differ, TO cookie %u matches the second",
		type, cookies[0], cookies[1], cookies[2]);
}

static void setup(void)
{
	SAFE_MKDIR("source", 0700);
	SAFE_MKDIR("unwatched", 0700);
	SAFE_MKDIR("target", 0700);
}

static void cleanup(void)
{
	if (fd_notify != -1)
		SAFE_CLOSE(fd_notify);
}

static struct tst_test test = {
	.test = verify_inotify,
	.tcnt = ARRAY_SIZE(tcases),
	.setup = setup,
	.cleanup = cleanup,
	.needs_tmpdir = 1,
};

#else
TST_TEST_TCONF("system doesn't have required inotify support");
#endif
