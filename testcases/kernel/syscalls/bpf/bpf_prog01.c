// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2019 Richard Palethorpe <rpalethorpe@suse.com>
 *
 * Trivial Extended Berkeley Packet Filter (eBPF) test.
 *
 * Sanity check loading and running bytecode.
 *
 * Test flow:
 * 1. Create array map
 * 2. Load eBPF program
 * 3. Attach program to socket
 * 4. Send packet on socket
 * 5. This should trigger eBPF program which writes to array map
 * 6. Verify array map was written to
 */
/*
 * If test is executed in a loop and limit for locked memory (ulimit -l) is
 * too low bpf() call can fail with EPERM due to deffered freeing.
 */

#include <limits.h>
#include <string.h>
#include <stdio.h>

#include "config.h"
#include "tst_test.h"
#include "lapi/socket.h"
#include "lapi/bpf.h"
#include "bpf_common.h"

const char MSG[] = "Ahoj!";
static char *msg;

static char *log;
static union bpf_attr *attr;

int load_prog(int fd)
{
	/*
	 * The following is a byte code template. We copy it to a guarded buffer and
	 * substitute the runtime value of our map file descriptor.
	 *
	 * r0 - r10 = registers 0 to 10
	 * r0 = return code
	 * r1 - r5 = scratch registers, used for function arguments
	 * r6 - r9 = registers preserved across function calls
	 * fp/r10 = stack frame pointer
	 */
	struct bpf_insn PROG[] = {
		/* Load the map FD into r1 (place holder) */
		BPF_LD_MAP_FD(BPF_REG_1, fd),
		/* Put (key = 0) on stack and key ptr into r2 */
		BPF_MOV64_REG(BPF_REG_2, BPF_REG_10),   /* r2 = fp */
		BPF_ALU64_IMM(BPF_ADD, BPF_REG_2, -8),  /* r2 = r2 - 8 */
		BPF_ST_MEM(BPF_DW, BPF_REG_2, 0, 0),    /* *r2 = 0 */
		/* r0 = bpf_map_lookup_elem(r1, r2) */
		BPF_EMIT_CALL(BPF_FUNC_map_lookup_elem),
		/* if r0 == 0 goto exit */
		BPF_JMP_IMM(BPF_JEQ, BPF_REG_0, 0, 3),
		/* Set map[0] = 1 */
		BPF_MOV64_REG(BPF_REG_1, BPF_REG_0),     /* r1 = r0 */
		BPF_ST_MEM(BPF_DW, BPF_REG_1, 0, 1),     /* *r1 = 1 */
		BPF_MOV64_IMM(BPF_REG_0, 0),             /* r0 = 0 */
		BPF_EXIT_INSN(),		         /* return r0 */
	};

	bpf_init_prog_attr(attr, PROG, sizeof(PROG), log, BUFSIZ);
	return bpf_load_prog(attr, log);
}

void setup(void)
{
	rlimit_bump_memlock();

	memcpy(msg, MSG, sizeof(MSG));
}

void run(void)
{
	int map_fd, prog_fd;
	int sk[2];
	uint32_t key = 0;
	uint64_t val;

	memset(attr, 0, sizeof(*attr));
	attr->map_type = BPF_MAP_TYPE_ARRAY;
	attr->key_size = 4;
	attr->value_size = 8;
	attr->max_entries = 1;

	map_fd = bpf_map_create(attr);

	prog_fd = load_prog(map_fd);

	SAFE_SOCKETPAIR(AF_UNIX, SOCK_DGRAM, 0, sk);
	SAFE_SETSOCKOPT(sk[1], SOL_SOCKET, SO_ATTACH_BPF,
			&prog_fd, sizeof(prog_fd));

	SAFE_WRITE(1, sk[0], msg, sizeof(MSG));

	memset(attr, 0, sizeof(*attr));
	attr->map_fd = map_fd;
	attr->key = ptr_to_u64(&key);
	attr->value = ptr_to_u64(&val);

	TEST(bpf(BPF_MAP_LOOKUP_ELEM, attr, sizeof(*attr)));
	if (TST_RET == -1) {
		tst_res(TFAIL | TTERRNO, "array map lookup");
	} else if (val != 1) {
		tst_res(TFAIL,
			"val = %lu, but should be val = 1",
			val);
        } else {
	        tst_res(TPASS, "val = 1");
	}

	SAFE_CLOSE(prog_fd);
	SAFE_CLOSE(map_fd);
	SAFE_CLOSE(sk[0]);
	SAFE_CLOSE(sk[1]);
}

static struct tst_test test = {
	.setup = setup,
	.test_all = run,
	.min_kver = "3.19",
	.bufs = (struct tst_buffers []) {
		{&log, .size = BUFSIZ},
		{&attr, .size = sizeof(*attr)},
		{&msg, .size = sizeof(MSG)},
		{},
	}
};
