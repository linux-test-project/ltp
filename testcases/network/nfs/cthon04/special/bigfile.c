/*
 * @(#)bigfile.c	1.2	98/12/19 Connectathon Testsuite
 */

/*
 * Write and reread a large file.  This potentially covers a few problems
 * that have appeared in the past:
 * - inability of server to commit a large file range with one RPC
 * - client's dirtying memory faster than it can clean it
 * - server's returning bogus file attributes, confusing the client
 * - client and server not propagating "filesystem full" errors back to the
 *   application
 */

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#ifdef MMAP
#include <sys/mman.h>
#endif

#include "../tests.h"

static char usage[] = "usage: bigfile [-s size_in_MB] filename";

static off_t file_size = 30 * 1024 * 1024;

static char *filename;			/* name of test file */
static int buffer_size = 8192;		/* size of read/write buffer */

#ifdef MMAP
static long pagesize;
#endif

static void dump_buf ARGS_((char *, int));
static void io_error ARGS_((int, char *));
static unsigned char testval ARGS_((off_t));
static int verify ARGS_((char *, long, int));
static void write_read ARGS_((int));
#ifdef MMAP
static void write_read_mmap ARGS_((int));
#endif

int
main(argc, argv)
	int argc;
	char **argv;
{
	int c;
	off_t size;
	int fd;
	extern int optind;
	extern char *optarg;

#ifdef MMAP
	pagesize = sysconf(_SC_PAGESIZE);
	if (pagesize < 0) {
		fprintf(stderr, "can't get page size\n");
		exit(1);
	}
#endif

	while ((c = getopt(argc, argv, "s:")) != EOF)
		switch (c) {
		case 's':
			size = atol(optarg) * 1024 * 1024;
			if (size > 0)
				file_size = size;
			break;
		case '?':
			fprintf(stderr, "%s\n", usage);
			exit(1);
			break;
		}
	if (optind != argc - 1) {
		fprintf(stderr, "%s\n", usage);
		exit(1);
	}
	filename = argv[optind];
	fd = open(filename, O_RDWR | O_CREAT | O_TRUNC, 0666);
	if (fd < 0) {
		fprintf(stderr, "can't create %s: %s\n",
			filename, strerror(errno));
		exit(1);
	}

	write_read(fd);
#ifdef MMAP
	write_read_mmap(fd);
#endif

	if (unlink(filename) < 0) {
		fprintf(stderr, "can't unlink %s: %s\n",
			filename, strerror(errno));
		exit(1);
	}
	exit(0);
}

/*
 * Write and then reread the file, using regular read/write calls.  If the
 * filesystem fills up, exit with a warning message.  For any other error,
 * exit with an error message.
 */

static void
write_read(fd)
	int fd;
{
	long numbufs = file_size / buffer_size;
	char *buf;
	int i;

	buf = malloc(buffer_size);
	if (buf == 0) {
		fprintf(stderr, "can't allocate read/write buffer\n");
		exit(1);
	}

	/*
	 * Fill the file with unsigned chars.  Change the value for each
	 * buffer written.
	 */

	for (i = 0; i < numbufs; i++) {
		unsigned char val = testval(i);
		int bytes_written;

		memset(buf, val, buffer_size);
		bytes_written = write(fd, buf, buffer_size);
		if (bytes_written < 0) {
			int error = errno;
			char errmsg[1024];

			sprintf(errmsg, "write to %s failed: %s",
				filename, strerror(errno));
			io_error(error, errmsg);
		} else if (bytes_written < buffer_size) {
			fprintf(stderr, "short write (%d) to %s\n",
				bytes_written, filename);
			exit(1);
		}
	}

	if (fsync(fd) < 0) {
		char errmsg[1024];
		int error = errno;

		sprintf(errmsg, "can't sync %s: %s", filename,
			strerror(error));
		io_error(error, errmsg);
	}

	/*
	 * Close and reopen the file, in case that prompts the client to
	 * throw anything away.
	 */

	if (close(fd) < 0) {
		char errmsg[1024];
		int error = errno;

		sprintf(errmsg, "can't close %s: %s", filename,
			strerror(error));
		io_error(error, errmsg);
	}
	fd = open(filename, O_RDWR, 0666);
	if (fd < 0) {
		fprintf(stderr, "can't reopen %s: %s\n",
			filename, strerror(errno));
		exit(1);
	}

	/*
	 * Reread the file and make sure it has correct bits.
	 */

	for (i = 0; i < numbufs; i++) {
		unsigned char val = testval(i);
		int bytes_read;

		if (lseek(fd, i * buffer_size, SEEK_SET) < 0) {
			fprintf(stderr, "seek to %ld failed: %s\n",
				(long)i * buffer_size,
				strerror(errno));
			exit(1);
		}
		bytes_read = read(fd, buf, buffer_size);
		if (bytes_read < 0) {
			int error = errno;
			char errmsg[1024];

			sprintf(errmsg, "read from %s failed: %s",
				filename, strerror(errno));
			io_error(error, errmsg);
		} else if (bytes_read < buffer_size) {
			fprintf(stderr, "short read (%d) to %s\n",
				bytes_read, filename);
			exit(1);
		}
		if (!verify(buf, buffer_size, val)) {
			fprintf(stderr, "verify failed, offset %ld; ",
				(long)i * buffer_size);
			fprintf(stderr, "expected %x, got \n",
				val);
			dump_buf(buf, buffer_size);
			exit(1);
		}
	}
}

/*
 * Return non-zero if the given buffer is full of the given value.
 * Otherwise, return zero.
 */

static int
verify(buf, bufsize, val)
	char *buf;
	long bufsize;
	unsigned char val;
{
	int i;

	for (i = 0; i < bufsize; i++) {
		if ((unsigned char)(buf[i]) != val)
			return (0);
	}

	return (1);
}

/*
 * Print the contents of the buffer in hex to stderr.
 */

static void
dump_buf(buf, bufsize)
	char *buf;
	int bufsize;
{
	int i;

	for (i = 0; i < bufsize; i++) {
		fprintf(stderr, "%x ", buf[i]);
		if ((i + 1) % 10 == 0)
			fprintf(stderr, "\n");
	}
	fprintf(stderr, "\n");
}

/*
 * Write out the given error message and exit.  If the error is because
 * there is no more space, flag it as a warning, and delete the file.
 * Otherwise, flag it as an error and leave the file alone.
 */

static void
io_error(error, errmsg)
	int error;			/* errno value */
	char *errmsg;
{
	if (error == EDQUOT || error == ENOSPC)
		fprintf(stderr, "Warning: can't complete test: ");
	else
		fprintf(stderr, "Error: ");
	fprintf(stderr, "%s\n", errmsg);

	if (error == EDQUOT || error == ENOSPC)
		unlink(filename);

	exit(1);
}

/*
 * Return the test value for the given offset.
 */

static unsigned char
testval(offset)
	off_t offset;
{
	return 'a' + (offset % 26);
}

#ifdef MMAP

/*
 * Write and then randomly reread the file, by mapping it.  Same error
 * handling as write_read().
 */

static void
write_read_mmap(fd)
	int fd;
{
	long numpages = file_size / pagesize;
	char *buf;
	int i;

	/*
	 * Truncate the file and then map it in (the entire file).  Then
	 * fill it with unsigned chars, the same as write_read().
	 */

	if (ftruncate(fd, 0) < 0) {
		fprintf(stderr, "can't truncate %s: %s\n",
			filename, strerror(errno));
		exit(1);
	}
	if (ftruncate(fd, file_size) < 0) {
			int error = errno;
			char errmsg[1024];

			sprintf(errmsg, "write to %s failed: %s",
				filename, strerror(errno));
			io_error(error, errmsg);
	}
	buf = mmap(0, file_size, PROT_READ | PROT_WRITE,
		   MAP_SHARED, fd, 0);
	if (buf == (char *)MAP_FAILED) {
		fprintf(stderr, "can't map %s for writing: %s\n",
			filename, strerror(errno));
		exit(1);
	}

	for (i = 0; i < numpages; i++) {
		unsigned char val = testval(i);

		memset(buf + i * pagesize, val, pagesize);
	}

	if (msync(buf, file_size, MS_SYNC | MS_INVALIDATE) < 0) {
		char errmsg[1024];
		int error = errno;

		sprintf(errmsg, "can't msync %s: %s", filename,
			strerror(error));
		io_error(error, errmsg);
	}
	if (munmap(buf, file_size) < 0) {
		char errmsg[1024];
		int error = errno;

		sprintf(errmsg, "can't munmap %s: %s", filename,
			strerror(error));
		io_error(error, errmsg);
	}

	/*
	 * Reread the file, a page at a time, and make sure it has correct
	 * bits.
	 */

	for (i = 0; i < numpages; i++) {
		unsigned char val = testval(i);

		buf = mmap(0, pagesize, PROT_READ, MAP_SHARED, fd,
			   i * pagesize);
		if (buf == (char *)MAP_FAILED) {
			fprintf(stderr, "can't map %s for reading: %s\n",
				filename, strerror(errno));
			exit(1);
		}
		if (!verify(buf, pagesize, val)) {
			fprintf(stderr,
				"verify of mapped file failed, offset %ld; ",
				(long)i * pagesize);
			fprintf(stderr, "expected %x, got \n",
				val);
			dump_buf(buf, pagesize);
			exit(1);
		}

		if (munmap(buf, pagesize) < 0) {
			fprintf(stderr,
				"can't unmap file after verifying: %s\n",
				strerror(errno));
			exit(1);
		}
	}
}

#endif /* MMAP */
