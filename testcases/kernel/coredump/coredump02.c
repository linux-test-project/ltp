// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2026 Linux Test Project
 */

/*\
 * Verify the ELF structure and note content of a kernel-generated core
 * dump.
 *
 * A core dump written by the kernel is an ELF file of type ET_CORE. It
 * contains:
 *
 * - one PT_NOTE program header holding process metadata
 * - one or more PT_LOAD program headers for the mapped memory segments
 *
 * The PT_NOTE segment carries a stream of notes. Two of them describe
 * the crashed process:
 *
 * - NT_PRPSINFO, whose descriptor is a struct elf_prpsinfo. The pr_fname
 *   field holds the executable name.
 * - NT_PRSTATUS, whose descriptor is a struct elf_prstatus. The pr_pid
 *   field holds the PID and pr_cursig holds the terminating signal.
 *
 * The crashed child is a fork of the test binary, so the core dump has
 * the same ELF class as the test. ElfW() is therefore safe here.
 *
 * The test needs root because it rewrites the system-wide core_pattern.
 * The original value is saved and restored by the test library on all
 * exit paths.
 *
 * [Algorithm]
 *
 * - Point core_pattern into the test temporary directory
 * - Fork a child which aborts itself to produce a core dump
 * - Read the core file into memory and parse the ELF header
 * - Walk the program headers and verify PT_NOTE and PT_LOAD are present
 * - Walk the notes in every PT_NOTE segment
 * - Check that NT_PRPSINFO carries the expected executable name
 * - Check that NT_PRSTATUS carries the expected PID and signal
 */

#include <link.h>
#include <sys/procfs.h>

#include "coredump_common.h"

#define NOTE_ALIGN(x) (((x) + 3) & ~3U)

static char *core_buf;
static size_t core_len;
static int prpsinfo_seen, prstatus_seen;

static void load_core(const char *path)
{
	struct stat st;
	int fd;

	SAFE_STAT(path, &st);
	if (st.st_size <= 0)
		tst_brk(TFAIL, "core file %s is empty", path);

	core_len = st.st_size;
	core_buf = SAFE_MALLOC(core_len);

	fd = SAFE_OPEN(path, O_RDONLY);
	SAFE_READ(1, fd, core_buf, core_len);
	SAFE_CLOSE(fd);
}

static void unload_core(void)
{
	free(core_buf);
	core_buf = NULL;
	core_len = 0;
}

static const void *core_at(size_t off, size_t need)
{
	if (off > core_len || need > core_len - off)
		tst_brk(TFAIL, "core file truncated at %zu (need %zu, have %zu)",
			off, need, core_len);

	return core_buf + off;
}

static void handle_note(uint32_t type, const void *desc, size_t descsz, pid_t pid)
{
	if (type == NT_PRPSINFO && descsz >= sizeof(struct elf_prpsinfo)) {
		const struct elf_prpsinfo *info = desc;

		TST_EXP_EQ_STR(info->pr_fname, "coredump02");
		prpsinfo_seen = 1;
	} else if (type == NT_PRSTATUS && descsz >= sizeof(struct elf_prstatus)) {
		const struct elf_prstatus *st = desc;

		TST_EXP_EQ_LI(st->pr_pid, pid);
		TST_EXP_EQ_LI(st->pr_cursig, SIGABRT);
		prstatus_seen = 1;
	}
}

static void walk_notes(size_t off, size_t size, pid_t pid)
{
	size_t pos = 0;

	while (pos + sizeof(ElfW(Nhdr)) <= size) {
		const ElfW(Nhdr) *nh = core_at(off + pos, sizeof(*nh));
		size_t np = NOTE_ALIGN(nh->n_namesz);
		size_t dp = NOTE_ALIGN(nh->n_descsz);
		size_t total = sizeof(*nh) + np + dp;

		if (pos + total > size)
			tst_brk(TFAIL, "note extends past PT_NOTE (pos=%zu total=%zu size=%zu)",
				pos, total, size);

		handle_note(nh->n_type,
			    core_at(off + pos + sizeof(*nh) + np, nh->n_descsz),
			    nh->n_descsz, pid);

		pos += total;
	}
}

static void run(void)
{
	char dump[PATH_MAX + 32];
	int pt_note_cnt = 0, have_load = 0;
	const ElfW(Ehdr) *eh;
	const ElfW(Phdr) *ph;
	pid_t pid;
	size_t i;

	prpsinfo_seen = prstatus_seen = 0;

	set_pattern("%s/core.%%p", cwd);

	pid = crash_child();

	snprintf(dump, sizeof(dump), "%s/core.%d", cwd, pid);
	load_core(dump);

	eh = core_at(0, sizeof(*eh));

	TST_EXP_EXPR(!memcmp(eh->e_ident, ELFMAG, SELFMAG), "core has ELF magic");
	TST_EXP_EQ_LI(eh->e_type, ET_CORE);

	if (!eh->e_phnum)
		tst_brk(TFAIL, "core has no program headers");

	ph = core_at(eh->e_phoff, (size_t)eh->e_phnum * sizeof(*ph));

	for (i = 0; i < eh->e_phnum; i++) {
		if (ph[i].p_type == PT_NOTE) {
			pt_note_cnt++;
			walk_notes(ph[i].p_offset, ph[i].p_filesz, pid);
		} else if (ph[i].p_type == PT_LOAD) {
			have_load = 1;
			core_at(ph[i].p_offset, ph[i].p_filesz);
		}
	}

	TST_EXP_EQ_LI(pt_note_cnt, 1);
	TST_EXP_EQ_LI(have_load, 1);
	TST_EXP_EQ_LI(prpsinfo_seen, 1);
	TST_EXP_EQ_LI(prstatus_seen, 1);

	SAFE_UNLINK(dump);
	unload_core();
}

static void cleanup(void)
{
	unload_core();
}

static struct tst_test test = {
	.test_all = run,
	.setup = coredump_setup,
	.cleanup = cleanup,
	.needs_root = 1,
	.needs_tmpdir = 1,
	.forks_child = 1,
	.needs_kconfigs = (const char *[]) {
		"CONFIG_COREDUMP=y",
		NULL,
	},
	.save_restore = (const struct tst_path_val[]) {
		{PATH_KERN_CORE_PATTERN, NULL, TST_SR_TCONF},
		{}
	},
};
