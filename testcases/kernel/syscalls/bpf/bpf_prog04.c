// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2018 Jann Horn <jannh@google.com>
 * Copyright (c) 2020 SUSE LLC <mdoucha@suse.cz>
 */

/*\
 * CVE 2018-18445
 *
 * Check that eBPF verifier correctly handles 32-bit arithmetic, in particular
 * the right bit shift instruction. It is an error if the BPF program passes
 * verification regardless of whether it then causes any actual damage. Kernel
 * bug fixed in:
 * b799207e1e18 ("bpf: 32-bit RSH verification must truncate input before the ALU op")
 */

#include <stdio.h>
#include <string.h>

#include "config.h"
#include "tst_test.h"
#include "tst_taint.h"
#include "tst_capability.h"
#include "bpf_common.h"

#define CHECK_BPF_RET(x) ((x) >= 0 || ((x) == -1 && errno != EACCES))

static const char MSG[] = "Ahoj!";
static char *msg;

static char *log;
static union bpf_attr *attr;

static int load_prog(int fd)
{
	int ret;
	struct bpf_insn insn[] = {
		BPF_MOV64_IMM(BPF_REG_8, 2),
		BPF_ALU64_IMM(BPF_LSH, BPF_REG_8, 31),
		BPF_ALU32_IMM(BPF_RSH, BPF_REG_8, 31),
		BPF_ALU32_IMM(BPF_SUB, BPF_REG_8, 2),

		// store r8 into map
		BPF_LD_MAP_FD(BPF_REG_1, fd),
		BPF_MOV64_REG(BPF_REG_2, BPF_REG_10),
		BPF_ALU64_IMM(BPF_ADD, BPF_REG_2, -4),
		BPF_ST_MEM(BPF_W, BPF_REG_2, 0, 0),
		BPF_EMIT_CALL(BPF_FUNC_map_lookup_elem),
		BPF_JMP_IMM(BPF_JNE, BPF_REG_0, 0, 1),
		BPF_EXIT_INSN(),
		BPF_ALU64_REG(BPF_ADD, BPF_REG_0, BPF_REG_8),
		BPF_STX_MEM(BPF_DW, BPF_REG_0, BPF_REG_8, 0),

		BPF_MOV64_IMM(BPF_REG_0, 0),
		BPF_EXIT_INSN()
	};

	bpf_init_prog_attr(attr, insn, sizeof(insn), log, BUFSIZE);
	ret = TST_RETRY_FUNC(bpf(BPF_PROG_LOAD, attr, sizeof(*attr)),
		CHECK_BPF_RET);

	if (ret >= 0) {
		tst_res(TINFO, "Verification log:");
		fputs(log, stderr);
		return ret;
	}

	if (ret < -1)
		tst_brk(TBROK, "Invalid bpf() return value %d", ret);

	if (!*log)
		tst_brk(TBROK | TERRNO, "Failed to load BPF program");

	tst_res(TPASS | TERRNO, "BPF program failed verification");
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

	map_fd = bpf_map_array_create(1);
	prog_fd = load_prog(map_fd);

	if (prog_fd >= 0) {
		tst_res(TFAIL, "Malicious eBPF code passed verification. "
			"Now let's try crashing the kernel.");
		bpf_run_prog(prog_fd, msg, sizeof(MSG));
	}

	if (prog_fd >= 0)
		SAFE_CLOSE(prog_fd);

	SAFE_CLOSE(map_fd);
}

static struct tst_test test = {
	.setup = setup,
	.test_all = run,
	.taint_check = TST_TAINT_W | TST_TAINT_D,
	.caps = (struct tst_cap []) {
		TST_CAP(TST_CAP_DROP, CAP_SYS_ADMIN),
		{}
	},
	.bufs = (struct tst_buffers []) {
		{&log, .size = BUFSIZE},
		{&attr, .size = sizeof(*attr)},
		{&msg, .size = sizeof(MSG)},
		{}
	},
	.tags = (const struct tst_tag[]) {
		{"linux-git", "b799207e1e18"},
		{"CVE", "2018-18445"},
		{}
	}
};
