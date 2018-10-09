/******************************************************************************/
/*                                                                            */
/* Copyright (c) 2008 FUJITSU LIMITED                                         */
/*                                                                            */
/* This program is free software;  you can redistribute it and/or modify      */
/* it under the terms of the GNU General Public License as published by       */
/* the Free Software Foundation; either version 2 of the License, or          */
/* (at your option) any later version.                                        */
/*                                                                            */
/* This program is distributed in the hope that it will be useful,            */
/* but WITHOUT ANY WARRANTY;  without even the implied warranty of            */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See                  */
/* the GNU General Public License for more details.                           */
/*                                                                            */
/* You should have received a copy of the GNU General Public License          */
/* along with this program;  if not, write to the Free Software               */
/* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA    */
/*                                                                            */
/* Author: Li Zefan <lizf@cn.fujitsu.com>                                     */
/*                                                                            */
/******************************************************************************/

#include <sys/socket.h>
#include <sys/poll.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <linux/types.h>
#include <linux/netlink.h>

#ifndef NETLINK_CONNECTOR

int main(void)
{
	return 2;
}

#else

#include <linux/connector.h>

#ifndef CN_IDX_PROC

int main(void)
{
	return 2;
}

#else

#define _LINUX_TIME_H
#include <linux/cn_proc.h>

#define PEC_MSG_SIZE (sizeof(struct cn_msg) + sizeof(struct proc_event))
#define PEC_CTRL_MSG_SIZE (sizeof(struct cn_msg) + sizeof(enum proc_cn_mcast_op))

#define MAX_MSG_SIZE 256

static __u32 seq;

static int exit_flag;
static struct sigaction sigint_action;

struct nlmsghdr *nlhdr;

/*
 * Handler for signal int. Set exit flag.
 *
 * @signo: the signal number, not used
 */
static void sigint_handler(int __attribute__ ((unused)) signo)
{
	exit_flag = 1;
}

/*
 * Send netlink package.
 *
 * @sd: socket descripor
 * @to: the destination sockaddr
 * @cnmsg: the pec control message
 */
static int netlink_send(int sd, struct sockaddr_nl *to, struct cn_msg *cnmsg)
{
	int ret;
	struct iovec iov;
	struct msghdr msg;

	memset(nlhdr, 0, NLMSG_SPACE(MAX_MSG_SIZE));

	nlhdr->nlmsg_seq = seq++;
	nlhdr->nlmsg_pid = getpid();
	nlhdr->nlmsg_type = NLMSG_DONE;
	nlhdr->nlmsg_len = NLMSG_LENGTH(sizeof(*cnmsg) + cnmsg->len);
	nlhdr->nlmsg_flags = 0;
	memcpy(NLMSG_DATA(nlhdr), cnmsg, sizeof(*cnmsg) + cnmsg->len);

	memset(&iov, 0, sizeof(struct iovec));
	iov.iov_base = (void *)nlhdr;
	iov.iov_len = nlhdr->nlmsg_len;

	memset(&msg, 0, sizeof(struct msghdr));
	msg.msg_name = (void *)to;
	msg.msg_namelen = sizeof(*to);
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;

	ret = sendmsg(sd, &msg, 0);

	return ret;
}

/*
 * Receive package from netlink.
 *
 * @sd: socket descripor
 * @from: source sockaddr
 */
static int netlink_recv(int sd, struct sockaddr_nl *from)
{
	int ret;
	struct iovec iov;
	struct msghdr msg;

	memset(nlhdr, 0, NLMSG_SPACE(MAX_MSG_SIZE));
	memset(&iov, 0, sizeof(iov));
	memset(&msg, 0, sizeof(msg));

	iov.iov_base = (void *)nlhdr;
	iov.iov_len = NLMSG_SPACE(MAX_MSG_SIZE);

	msg.msg_name = (void *)from;
	msg.msg_namelen = sizeof(*from);
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;

	ret = recvmsg(sd, &msg, 0);

	return ret;
}

/*
 * Send control message to PEC.
 *
 * @sd: socket descriptor
 * @to: the destination sockaddr
 * @op: control flag
 */
static int control_pec(int sd, struct sockaddr_nl *to, enum proc_cn_mcast_op op)
{
	int ret;
	char buf[PEC_CTRL_MSG_SIZE];
	struct cn_msg *cnmsg;
	enum proc_cn_mcast_op *pec_op;

	memset(buf, 0, sizeof(buf));

	cnmsg = (struct cn_msg *)buf;
	cnmsg->id.idx = CN_IDX_PROC;
	cnmsg->id.val = CN_VAL_PROC;
	cnmsg->seq = seq++;
	cnmsg->ack = 0;
	cnmsg->len = sizeof(op);

	pec_op = (enum proc_cn_mcast_op *)cnmsg->data;
	*pec_op = op;

	ret = netlink_send(sd, to, cnmsg);

	return ret;
}

/*
 * Process PEC event.
 *
 * @nlhdr: the netlinke pacakge
 */
static void process_event(struct nlmsghdr *nlhdr)
{
	struct cn_msg *msg;
	struct proc_event *pe;

	msg = (struct cn_msg *)NLMSG_DATA(nlhdr);

	pe = (struct proc_event *)msg->data;

	switch (pe->what) {
	case PROC_EVENT_NONE:
		printf("none err: %u\n", pe->event_data.ack.err);
		break;
	case PROC_EVENT_FORK:
		printf("fork parent: %d, child: %d\n",
		       pe->event_data.fork.parent_pid,
		       pe->event_data.fork.child_pid);
		break;
	case PROC_EVENT_EXEC:
		printf("exec pid: %d\n", pe->event_data.exec.process_pid);
		break;
	case PROC_EVENT_UID:
		printf("uid pid: %d euid: %d ruid: %d\n",
		       pe->event_data.id.process_pid,
		       pe->event_data.id.e.euid, pe->event_data.id.r.ruid);
		break;
	case PROC_EVENT_GID:
		printf("gid pid: %d egid: %d rgid: %d\n",
		       pe->event_data.id.process_pid,
		       pe->event_data.id.e.egid, pe->event_data.id.r.rgid);
		break;
	case PROC_EVENT_EXIT:
		printf("exit pid: %d exit_code: %d exit_signal: %d\n",
		       pe->event_data.exit.process_pid,
		       pe->event_data.exit.exit_code,
		       pe->event_data.exit.exit_signal);
		break;
	default:
		printf("unknown event\n");
		break;
	}
}

int main(int argc, char **argv)
{
	int ret;
	int sd;
	struct sockaddr_nl l_local;
	struct sockaddr_nl src_addr;
	struct pollfd pfd;

	sigint_action.sa_flags = SA_RESETHAND;
	sigint_action.sa_handler = &sigint_handler;
	sigaction(SIGINT, &sigint_action, NULL);

	nlhdr = malloc(NLMSG_SPACE(MAX_MSG_SIZE));
	if (!nlhdr) {
		fprintf(stderr, "lack of memory\n");
		exit(1);
	}

	sd = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_CONNECTOR);
	if (sd == -1) {
		fprintf(stderr, "failed to create socket\n");
		exit(1);
	}

	memset(&src_addr, 0, sizeof(src_addr));
	src_addr.nl_family = AF_NETLINK;
	src_addr.nl_pid = 0;
	src_addr.nl_groups = 0;

	memset(&l_local, 0, sizeof(l_local));
	l_local.nl_family = AF_NETLINK;
	l_local.nl_pid = getpid();
	l_local.nl_groups = CN_IDX_PROC;

	ret = bind(sd, (struct sockaddr *)&l_local, sizeof(struct sockaddr_nl));
	if (ret == -1) {
		fprintf(stderr, "failed to bind socket\n");
		exit(1);
	}

	/* Open PEC listening */
	ret = control_pec(sd, &src_addr, PROC_CN_MCAST_LISTEN);
	if (!ret) {
		fprintf(stderr, "failed to open PEC listening\n");
		exit(1);
	}

	/* Receive msg from PEC */
	pfd.fd = sd;
	pfd.events = POLLIN;
	pfd.revents = 0;
	while (!exit_flag) {

		ret = poll(&pfd, 1, -1);
		if (ret == 0 || (ret == -1 && errno != EINTR)) {
			control_pec(sd, &src_addr, PROC_CN_MCAST_IGNORE);
			fprintf(stderr, "failed to poll\n");
			exit(1);
		} else if (ret == -1 && errno == EINTR)
			break;

		ret = netlink_recv(sd, &src_addr);

		if (ret == 0)
			break;
		else if (ret == -1 && errno == EINTR)
			break;
		else if (ret == -1 && errno != EINTR) {
			control_pec(sd, &src_addr, PROC_CN_MCAST_IGNORE);
			fprintf(stderr, "failed to receive from netlink\n");
			exit(1);
		} else {
			switch (nlhdr->nlmsg_type) {
			case NLMSG_ERROR:
				fprintf(stderr, "err message received.\n");
				exit(1);
				break;
			case NLMSG_DONE:
				/* message sent from kernel */
				if (nlhdr->nlmsg_pid == 0)
					process_event(nlhdr);
				break;
			default:
				break;
			}
		}
	}

	/* Close PEC listening */
	ret = control_pec(sd, &src_addr, PROC_CN_MCAST_IGNORE);
	if (!ret) {
		fprintf(stderr, "failed to close PEC listening\n");
		exit(1);
	}

	close(sd);
	free(nlhdr);

	while (fsync(STDOUT_FILENO) == -1) {
		if (errno != EIO)
			break;
		/* retry once every 10 secodns */
		sleep(10);
	}

	return 0;
}

#endif /* CN_IDX_PROC */

#endif /* NETLINK_CONNECTOR */
