// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2026 Linux Test Project
 */

/*
 * Core dump collector for the piped core_pattern tested by coredump01.
 *
 * Avoiding the LTP API here is correct, since the kernel spawns the helper
 * through ``call_usermodehelper()`` without the LTP IPC environment.
 *
 * The helper is called as:
 *
 *	coredump01_helper <exe> <pid> <signal> <result file>
 *
 * and stores what it received in the result file, which is published with
 * :manpage:`rename()` so that the polling test never reads a partial line.
 */

#include <elf.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
	unsigned char buf[4096], hdr[EI_NIDENT + sizeof(Elf32_Half)];
	char tmp[PATH_MAX];
	unsigned int hdr_len = 0;
	long long bytes = 0;
	ssize_t rval, i;
	int fd, elf = 0, et_core = 0;
	Elf32_Half e_type;

	if (argc < 5)
		return 1;

	/*
	 * The ELF header is only meaningful at the very beginning of the
	 * stream. Collect enough bytes to read e_ident and e_type.
	 */
	while ((rval = read(STDIN_FILENO, buf, sizeof(buf))) > 0) {
		for (i = 0; i < rval && hdr_len < sizeof(hdr); i++)
			hdr[hdr_len++] = buf[i];

		bytes += rval;
	}

	if (rval < 0)
		return 1;

	if (hdr_len >= sizeof(hdr)) {
		if (!memcmp(hdr, ELFMAG, SELFMAG))
			elf = 1;
		memcpy(&e_type, hdr + EI_NIDENT, sizeof(e_type));
		et_core = (e_type == ET_CORE);
	}

	snprintf(tmp, sizeof(tmp), "%s.tmp", argv[4]);

	fd = open(tmp, O_WRONLY | O_CREAT | O_TRUNC, 0644);
	if (fd < 0)
		return 1;

	dprintf(fd, "exe=%s pid=%s sig=%s bytes=%lld elf=%d et_core=%d\n",
		argv[1], argv[2], argv[3], bytes, elf, et_core);

	close(fd);

	return rename(tmp, argv[4]) ? 1 : 0;
}
