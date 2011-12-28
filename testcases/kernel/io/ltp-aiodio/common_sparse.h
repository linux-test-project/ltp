/*
 * Copyright (c) 2011 Cyril Hrubis <chrubis@suse.cz>
 *
 *   This program is free software;  you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY;  without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 *   the GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program;  if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#ifndef LTP_AIODIO_COMMON_SPARSE
#define LTP_AIODIO_COMMON_SPARSE

#define barrier() __asm__ __volatile__("": : :"memory")

#define WITH_SIGNALS_BLOCKED(code) { \
		sigset_t held_sigs_; \
		sigfillset(&held_sigs_); \
		sigprocmask(SIG_SETMASK, &held_sigs_, &held_sigs_); \
		barrier(); \
		code; \
		barrier(); \
		sigprocmask(SIG_SETMASK, &held_sigs_, NULL); \
	}

/*
 * This code tries to create dirty free blocks on 
 * the HDD so there is a chance that blocks to be allocated
 * for a file are filled with something else than zeroes.
 *
 * The usefulness of this is IMHO questionable.
 */
static void dirty_freeblocks(int size)
{
	int fd;
	void *p;
	int pg;
	char *filename = "dirty_freeblocks";

	pg = getpagesize();
	size = ((size + pg - 1) / pg) * pg;

	fd = open(filename, O_CREAT|O_RDWR|O_EXCL, 0600);
	
	if (fd < 0)
		tst_brkm(TBROK|TERRNO, cleanup, "failed to open '%s'", filename); 
	
	SAFE_FTRUNCATE(cleanup, fd, size);
	
	p = SAFE_MMAP(cleanup, NULL, size, PROT_WRITE|PROT_READ, MAP_SHARED|MAP_FILE, fd, 0);
	
	memset(p, 0xaa, size);
	msync(p, size, MS_SYNC);
	munmap(p, size);
	close(fd);
	unlink(filename);
}

char *check_zero(char *buf, int size)
{
	char *p;

	p = buf;

	while (size > 0) {
		if (*buf != 0) {
			fprintf(stderr, "non zero buffer at buf[%d] => 0x%02x,%02x,%02x,%02x\n",
				buf - p, (unsigned int)buf[0],
				size > 1 ? (unsigned int)buf[1] : 0,
				size > 2 ? (unsigned int)buf[2] : 0,
				size > 3 ? (unsigned int)buf[3] : 0);
			return buf;
		}
		buf++;
		size--;
	}
	return 0;
}

#endif /* LTP_AIODIO_COMMON_SPARSE */
