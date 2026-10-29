// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2002
 * Copyright (c) 2013 Oracle and/or its affiliates. All Rights Reserved.
 * Copyright (c) Linux Test Project, 2026
 */

#ifndef TSE_MSGCTL_H__
#define TSE_MSGCTL_H__

#define FAIL	1
#define PASS	0

struct mbuffer {
	long type;
	struct {
		char len;
		char pbytes[99];
	} data;
};

int doreader(long key, int tid, long type, int child, int nreps);
int dowriter(long key, int tid, long type, int child, int nreps);
int fill_buffer(char *buf, char val, int size);
int verify(char *buf, char val, int size, int child);

#endif /* TSE_MSGCTL_H__ */
