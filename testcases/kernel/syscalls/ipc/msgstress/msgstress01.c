// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines Corp., 2002
 *   06/30/2001   Port to Linux   nsharoff@us.ibm.com
 *   11/11/2002   Port to LTP     dbarrera@us.ibm.com
 * Copyright (C) 2024 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * [Description]
 *
 * Stress test for SysV IPC. We send multiple messages at the same time,
 * checking that we are not loosing any byte once we receive the messages
 * from multiple children.
 *
 * The number of messages to send is determined by the free slots available
 * in SysV IPC and the available number of children which can be spawned by
 * the process. Each sender will spawn multiple messages at the same time and
 * each receiver will read them one by one.
 */

#include <stdlib.h>
#include "tst_safe_sysv_ipc.h"
#include "tst_safe_stdio.h"
#include "tst_test.h"

#define SYSVIPC_TOTAL "/proc/sys/kernel/msgmni"
#define SYSVIPC_USED "/proc/sysvipc/msg"
#define MSGTYPE 10
#define MAXNREPS 100000

struct sysv_msg {
	long type;
	struct {
		char len;
		char pbytes[99];
	} data;
};

struct sysv_data {
	int id;
	struct sysv_msg msg;
};

static struct sysv_data *ipc_data;
static int ipc_data_len;

static char *str_num_messages;
static char *str_num_iterations;
static int num_messages = 1000;
static int num_iterations = MAXNREPS;
static volatile int *stop;
static volatile int *fail;
static int *finished;
static int *flags;

static int get_used_sysvipc(void)
{
	FILE *fp;
	int used = -1;
	char buf[BUFSIZ];

	fp = SAFE_FOPEN(SYSVIPC_USED, "r");

	while (fgets(buf, BUFSIZ, fp) != NULL)
		used++;

	SAFE_FCLOSE(fp);

	if (used < 0)
		tst_brk(TBROK, "Can't read %s to get used sysvipc resource", SYSVIPC_USED);

	return used;
}

static void reset_messages(void)
{
	ipc_data_len = 0;
	memset(ipc_data, 0, sizeof(struct sysv_data) * num_messages);

	for (int i = 0; i < num_messages; i++)
		ipc_data[i].id = -1;

	*stop = 0;
	*fail = 0;
	*finished = 0;
}

static int create_message(const int id)
{
	int pos = ipc_data_len;
	struct sysv_data *buff = ipc_data + pos;

	buff->id = id;
	buff->msg.type = MSGTYPE;
	buff->msg.data.len = (rand() % 99) + 1;

	for (int i = 0; i < buff->msg.data.len; i++)
		buff->msg.data.pbytes[i] = rand() % 255;

	ipc_data_len++;

	return pos;
}

static void writer(const int id, const int pos)
{
	struct sysv_data *buff = &ipc_data[pos];
	int iter = num_iterations;

	while (--iter >= 0 && !(*stop)) {
		int size = msgsnd(id, &buff->msg, 100, IPC_NOWAIT);

		if (size < 0) {
			if (errno == EAGAIN) {
				usleep(10);
				continue;
			}

			tst_brk(TBROK | TERRNO, "msgsnd() failed");
		}
	}

	tst_atomic_inc(finished);
}

static void reader(const int id, const int pos)
{
	int size;
	int iter = num_iterations;
	struct sysv_msg msg_recv;
	struct sysv_data *buff = &ipc_data[pos];

	while (--iter >= 0 && !(*stop)) {
		memset(&msg_recv, 0, sizeof(struct sysv_msg));

		size = msgrcv(id, &msg_recv, 100, MSGTYPE, IPC_NOWAIT);
		if (size < 0) {
			if (errno == ENOMSG) {
				usleep(10);
				continue;
			}

			tst_brk(TBROK | TERRNO, "msgrcv() failed");
		}

		if (msg_recv.type != buff->msg.type) {
			tst_res(TFAIL, "Received the wrong message type");

			*stop = 1;
			*fail = 1;
			return;
		}

		if (msg_recv.data.len != buff->msg.data.len) {
			tst_res(TFAIL, "Received the wrong message data length");

			*stop = 1;
			*fail = 1;
			return;
		}

		for (int i = 0; i < size; i++) {
			if (msg_recv.data.pbytes[i] != buff->msg.data.pbytes[i]) {
				tst_res(TFAIL, "Received wrong data at index %d: %x != %x", i,
					msg_recv.data.pbytes[i],
					buff->msg.data.pbytes[i]);

				*stop = 1;
				*fail = 1;
				return;
			}
		}

		tst_res(TDEBUG, "Received correct data");
		tst_res(TDEBUG, "msg_recv.type = %ld", msg_recv.type);
		tst_res(TDEBUG, "msg_recv.data.len = %d", msg_recv.data.len);
	}

	tst_atomic_inc(finished);
}

static void remove_queues(void)
{
	for (int pos = 0; pos < num_messages; pos++) {
		struct sysv_data *buff = &ipc_data[pos];

		if (buff->id != -1)
			SAFE_MSGCTL(buff->id, IPC_RMID, NULL);
	}
}

static void run(void)
{
	int id, pos;

	reset_messages();

	for (int i = 0; i < num_messages; i++) {
		id = SAFE_MSGGET(IPC_PRIVATE, IPC_CREAT | 0600);
		pos = create_message(id);

		if (!SAFE_FORK()) {
			writer(id, pos);
			return;
		}

		if (!SAFE_FORK()) {
			reader(id, pos);
			return;
		}

		if (*stop)
			break;

		if (!tst_remaining_runtime()) {
			tst_res(TWARN, "Out of runtime during forking...");
			*stop = 1;
			break;
		}
	}

	if (!(*stop))
		tst_res(TINFO, "All processes running");

	for (;;) {
		if (tst_atomic_load(finished) == 2 * num_messages)
			break;

		if (*stop)
			break;

		if (!tst_remaining_runtime()) {
			tst_res(TINFO, "Out of runtime, stopping processes...");
			*stop = 1;
			break;
		}

		sleep(1);
	}

	tst_reap_children();
	remove_queues();

	if (!(*fail))
		tst_res(TPASS, "Test passed. All messages have been received");
}

static void setup(void)
{
	int total_msg;
	int avail_msg;
	int free_msgs;
	int free_pids;

	srand(time(0));

	SAFE_FILE_SCANF(SYSVIPC_TOTAL, "%i", &total_msg);

	free_msgs = total_msg - get_used_sysvipc();

	/* We remove 10% of free pids, just to be sure
	 * we won't saturate the sysyem with processes.
	 */
	free_pids = tst_get_free_pids() / 2.1;

	avail_msg = MIN(free_msgs, free_pids);
	if (!avail_msg)
		tst_brk(TCONF, "Unavailable messages slots");

	tst_res(TINFO, "Available messages slots: %d", avail_msg);

	if (tst_parse_int(str_num_messages, &num_messages, 1, avail_msg))
		tst_brk(TBROK, "Invalid number of messages '%s'", str_num_messages);

	if (tst_parse_int(str_num_iterations, &num_iterations, 1, MAXNREPS))
		tst_brk(TBROK, "Invalid number of messages iterations '%s'", str_num_iterations);

	ipc_data = SAFE_MMAP(
		NULL,
		sizeof(struct sysv_data) * num_messages,
		PROT_READ | PROT_WRITE,
		MAP_SHARED | MAP_ANONYMOUS,
		-1, 0);

	flags = SAFE_MMAP(
		NULL,
		sizeof(int) * 3,
		PROT_READ | PROT_WRITE,
		MAP_SHARED | MAP_ANONYMOUS,
		-1, 0);

	stop = &flags[0];
	fail = &flags[1];
	finished = &flags[2];
}

static void cleanup(void)
{
	if (!ipc_data)
		return;

	remove_queues();

	SAFE_MUNMAP(ipc_data, sizeof(struct sysv_data) * num_messages);
	SAFE_MUNMAP(flags, sizeof(int) * 3);
}

static struct tst_test test = {
	.test_all = run,
	.setup = setup,
	.cleanup = cleanup,
	.forks_child = 1,
	.max_runtime = 180,
	.options = (struct tst_option[]) {
		{"n:", &str_num_messages, "Number of messages to send (default: 1000)"},
		{"l:", &str_num_iterations, "Number iterations per message (default: "
			TST_TO_STR(MAXNREPS) ")"},
		{},
	},
};
