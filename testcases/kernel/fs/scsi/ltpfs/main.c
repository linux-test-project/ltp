
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <math.h>
#include <time.h>
#include <ftw.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/kd.h>
#include <linux/errno.h>

#include "Ltpfs.h"

#define M_2PI (M_PI*2)
#define MAXN 4096
#define MAXFSIZE 1024 * 192
#define FILE_CREATE_COUNT 256
#define FAIL 0
#define SUCCESS 1
#define MAXNUM   5000
#define BUFFSIZE 8192
#define AVEFSIZE (MAXFSIZE/2)
#define POOLDISKSPACE (AVEFSIZE*128)
#define MAXERROR  1024
#define FILES_ONLY 0x01
#define ALL        0x00

// Globals

char wbuf[MAXFSIZE];
int startc = 0;
int showchar[] = { 124, 47, 45, 92, 124, 47, 45, 92 };

int nullFileHandle;
static int openlog[2] = { 0, 0 };

int cFileCount, dFileCount, errorCount;
static int disk_space_pool = 0;
char rootPath[BUFFSIZE];

int LTP_fs_open_block_device(void);
int do_fs_thump_tests(char *path);
int do_create_file_test(char *path);
int makedir(char *dir1);
int changedir(char *dir);
int do_random_access_test(int maxNum);
int do_random_create_delete(int maxNum);
int create_file(char *filename);
int delete_file(char *filename);
int gen_random_file_size(int min, int max);
int open_read_close(char *fname);
int create_or_delete(char *fname);
int do_tree_cleanup(char *path, int flag);
int cleanup_files(char *file, struct stat *statBuff, int flag);
int cleanup_dirs(char *file, struct stat *statBuff, int flag);

int ltp_block_dev_handle = 0;	/* handle to LTP Test block device */
int ltp_fileHandle = 0;
char *fileBuf;

int main(int argc, char **argv)
{

	ltpdev_cmd_t cmd = { 0, 0 };
	int rc, i, tmpHandle;
	struct stat statBuf;

	printf("[%s] - Running test program\n", argv[0]);

	rc = LTP_fs_open_block_device();

	if (!rc) {

		ltp_block_dev_handle = open(LTP_FS_DEVICE_NAME, O_RDWR);

		if (ltp_block_dev_handle < 0) {
			printf
			    ("ERROR: Open of device %s failed %d errno = %d\n",
			     LTP_FS_DEVICE_NAME, ltp_block_dev_handle, errno);
		} else {
			rc = ioctl(ltp_block_dev_handle, LTPAIODEV_CMD, &cmd);

			printf("return from AIO ioctl %d \n", rc);

			rc = ioctl(ltp_block_dev_handle, LTPBIODEV_CMD, &cmd);

			printf("return from BIO ioctl %d \n", rc);
		}

	} else {
		printf("ERROR: Create/open block device failed\n");
	}

	ltp_fileHandle =
	    open("/tmp/testfile", O_CREAT | O_RDWR | O_SYNC | FASYNC,
		 S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

	if (ltp_fileHandle > 0) {

		tmpHandle = open("/usr/include/ctype.h", O_RDONLY);

		if (tmpHandle > 0) {

			rc = fstat(tmpHandle, &statBuf);

			if (!rc) {
				fileBuf = malloc(statBuf.st_size);

				if (fileBuf) {

					read(tmpHandle, fileBuf,
					     statBuf.st_size);
					close(tmpHandle);
					write(ltp_fileHandle, fileBuf,
					      statBuf.st_size);

					for (i = 0; i < 100; i++) {
						read(ltp_fileHandle, fileBuf,
						     statBuf.st_size * i);
						write(ltp_fileHandle, fileBuf,
						      statBuf.st_size * i);
					}
				}

			}

		} else {
			printf("ERROR: Create/open file failed\n");
		}
	}

	printf("*** Starting FileSystem thump tests....****\n");
	printf("*** Please be patient, this may take a little while... ***\n");

	for (i = 1; i < argc; i++) {
		printf("Running test %d of %d on FileSystem %s \n", i, argc - 1,
		       argv[i]);
		if (strcmp(argv[i], "|") != 0) {
			strcpy(rootPath, argv[i]);
			rc = do_fs_thump_tests(argv[i]);
			if (rc != 0 && rc != ENOSPC) {
				printf
				    ("ERROR: Failed on FileSystem %s with errno %d \n",
				     argv[i], rc);
			}
		} else {
			printf("Test Program complete..\n");
			break;
		}

	}

	printf("Test Program complete..\n");

	return 0;
}

int do_fs_thump_tests(char *path)
{
	int rc = 0;

	printf("Changing to directory %s \n", path);

	changedir(path);

	cFileCount = 0;
	dFileCount = 0;

	rc |= do_create_file_test(path);
	rc |= do_random_access_test(MAXNUM);
	rc |= do_tree_cleanup(path, FILES_ONLY);
	rc |= do_random_create_delete(MAXNUM);
	rc |= do_tree_cleanup(path, ALL);

	return rc;

}

int do_tree_cleanup(char *path, int flag)
{

	if (flag == FILES_ONLY) {
		printf("Cleaning up test files...\n");
		ftw(path, (void *)cleanup_files, MAXNUM);
	} else {
		printf("Cleaning up everything in the test directory...\n");
		ftw(path, (void *)cleanup_files, MAXNUM);
		ftw(path, (void *)cleanup_dirs, MAXNUM);
	}

	return 0;
}

int cleanup_files(char *file, struct stat *statBuff, int flag)
{
	int rc = 0;

	if (flag == FTW_F) {
		if (unlink(file)) {
			printf("ERROR:%d removing file %s\n", errno, file);
		}
	}

	return rc;
}

int cleanup_dirs(char *file, struct stat *statBuff, int flag)
{
	int rc = 0;

	//printf("%s:Cleaning up directory %s \n", __FUNCTION__, file);

	if (strcmp(rootPath, file) == 0) {
		return 0;
	}

	if (flag == FTW_F) {
		if (unlink(file)) {
			printf("ERROR:%d removing file %s\n", errno, file);
		}
	} else if (flag == FTW_D) {
		changedir(file);
		ftw(file, (void *)cleanup_dirs, MAXNUM);
		rmdir(file);

	} else {
		printf("No idea what we found here\n");
	}

	return rc;
}

int do_create_file_test(char *path)
{
	int i = 0;
	int j = 0;
	int k = 0;
	int l = 0;
	int rc = 0;

	char dir1[MAXN];
	char dir2[MAXN];
	char dir3[MAXN];
	char filename[MAXN];

	time_t t;

	int maxfiles = 0xFFFFFF;

	time(&t);

	srandom((unsigned int)getpid() ^
		(((unsigned int)t << 16) | (unsigned int)t >> 16));

	printf("Creating files...\n");

	for (i = 0; i < FILE_CREATE_COUNT; i++) {

		sprintf(dir1, "%2.2x", i);

		makedir(dir1);

		changedir(dir1);

		for (j = 0; j < FILE_CREATE_COUNT; j++) {

			sprintf(dir2, "%2.2x", j);

			makedir(dir2);

			changedir(dir2);

			for (k = 0; k < FILE_CREATE_COUNT; k++) {

				sprintf(dir3, "%2.2x", k);
				makedir(dir3);
				changedir(dir3);

				for (l = 0; l < FILE_CREATE_COUNT; l++) {
					sprintf(filename, "%s%s%s%2.2x", dir1,
						dir2, dir3, l);
					rc = create_file(filename);
					if (rc != 0 || maxfiles < dFileCount++) {
						if (rc != ENOSPC) {
							printf
							    ("ERROR: failed error:%d creating all the test files ! \n",
							     errno);
							printf
							    ("ERROR2: rc:%d -- dFileCount:%d \n",
							     rc, dFileCount);
						}
						goto end;
					}
				}
				changedir("../");
			}
			changedir("../");
		}
		changedir("../");
	}
end:
	fprintf(stderr, "\nTotal create files: %d\n", cFileCount);
	printf("Done\n");
	return rc;
}

int makedir(char *dir1)
{
	if (mkdir(dir1, S_IRWXU) < 0) {
		perror(dir1);
		return (errno);
	}
	return 0;
}

int changedir(char *dir)
{
	if (chdir(dir) < 0) {
		perror(dir);
		return (errno);
	}

	return 0;
}

int create_file(char *filename)
{
	int fileHandle;
	int randomsize;

	if ((fileHandle = creat(filename, S_IRWXU)) < 0) {

		fprintf(stderr, "\nERROR line %d: Total create files: %d\n",
			__LINE__, cFileCount);
		perror(filename);
		return (errno);
	}

	if ((randomsize = gen_random_file_size(0, MAXFSIZE)) < 0) {
		randomsize = MAXFSIZE;
	}
	if (write(fileHandle, wbuf, randomsize) < 0) {

		fprintf(stderr, "\nERROR:%d line%d: Total create files: %d\n",
			errno, __LINE__, cFileCount);
		close(fileHandle);

		perror(filename);
		return (errno);
	}

	cFileCount++;
	close(fileHandle);
	return 0;
}

int delete_file(char *filename)
{
	struct stat buf;
	int st;

	st = stat(filename, &buf);

	if (st < 0) {
		errorCount++;
		printf("ERROR line %d: Getting file stats %s \n", __LINE__,
		       filename);
		return (-1);
	}

	disk_space_pool += buf.st_size;

	if (unlink(filename) < 0) {
		errorCount++;
		printf("ERROR line %d: Removing file %s \n", __LINE__,
		       filename);
		return (-1);
	}

	dFileCount++;
	return 0;
}

int LTP_fs_open_block_device()
{
	dev_t devt;
	struct stat statbuf;
	int rc = 0;

	if (ltp_block_dev_handle == 0) {

		/* check for the /dev/LTPFSTest subdir, and create if it does not exist.
		 *
		 * If devfs is running and mounted on /dev, these checks will all pass,
		 * so a new node will not be created.
		 */
		devt = makedev(LTPMAJOR, 0);

		rc = stat(LTP_FS_DEV_NODE_PATH, &statbuf);

		if (rc) {
			if (errno == ENOENT) {
				/* dev node does not exist. */
				rc = mkdir(LTP_FS_DEV_NODE_PATH,
					   (S_IFDIR | S_IRWXU | S_IRGRP |
					    S_IXGRP | S_IROTH | S_IXOTH));
			} else {
				printf
				    ("ERROR: Problem with LTP FS dev directory.  Error code from stat() is %d\n\n",
				     errno);
			}

		} else {
			if (!(statbuf.st_mode & S_IFDIR)) {
				rc = unlink(LTP_FS_DEV_NODE_PATH);
				if (!rc) {
					rc = mkdir(LTP_FS_DEV_NODE_PATH,
						   (S_IFDIR | S_IRWXU | S_IRGRP
						    | S_IXGRP | S_IROTH |
						    S_IXOTH));
				}
			}
		}

		/*
		 * Check for the /dev/ltp-fs/block_device node, and create if it does not
		 * exist.
		 */
		rc = stat(LTP_FS_DEVICE_NAME, &statbuf);
		if (rc) {
			if (errno == ENOENT) {
				/* dev node does not exist */
				rc = mknod(LTP_FS_DEVICE_NAME,
					   (S_IFBLK | S_IRUSR | S_IWUSR |
					    S_IRGRP | S_IWGRP), devt);
			} else {
				printf
				    ("ERROR:Problem with LTP FS block device node directory.  Error code form stat() is %d\n\n",
				     errno);
			}

		} else {
			/*
			 * /dev/ltp-fs/block_device exists.  Check to make sure it is for a
			 * block device and that it has the right major and minor.
			 */
			if ((!(statbuf.st_mode & S_IFBLK)) ||
			    (statbuf.st_rdev != devt)) {

				/* Recreate the dev node. */
				rc = unlink(LTP_FS_DEVICE_NAME);
				if (!rc) {
					rc = mknod(LTP_FS_DEVICE_NAME,
						   (S_IFBLK | S_IRUSR | S_IWUSR
						    | S_IRGRP | S_IWGRP), devt);
				}
			}
		}

	}

	return rc;
}

int gen_random_file_size(int min, int max)
{
	double u1, u2, z;
	int i;
	int ave;
	int range;
	int ZZ;
	if (min >= max) {
		return (-1);
	}
	range = max - min;
	ave = range / 2;
	for (i = 0; i < 10; i++) {
		u1 = ((double)(random() % 1000000)) / 1000000;
		u2 = ((double)(random() % 1000000)) / 1000000;
		z = sqrt(-2.0 * log(u1)) * cos(M_2PI * u2);
		ZZ = min + (ave + (z * (ave / 4)));
		if (ZZ >= min && ZZ < max) {
			return (ZZ);
		}
	}
	return (-1);
}

int do_random_access_test(int maxNum)
{
	int r;
	char fname[1024];
	time_t t;
	int i;

	printf("Running random access test...\n");
	changedir(rootPath);

	if (maxNum < 1 || maxNum > MAXNUM) {
		printf("out of size %d\n", maxNum);
		return 1;
	}

	time(&t);
	srandom((unsigned int)getpid() ^
		(((unsigned int)t << 16) | (unsigned int)t >> 16));

	if ((nullFileHandle = open("/dev/null", O_WRONLY)) < 0) {
		perror("/dev/null");
		return (errno);
	}

	/* 00/00/00/00 */
	for (i = 0; i < maxNum; i++) {

		r = random() % maxNum;

		sprintf(fname, "00/%2.2x/%2.2x/00%2.2x%2.2x%2.2x",
			((r >> 16) & 0xFF),
			((r >> 8) & 0xFF),
			((r >> 16) & 0xFF), ((r >> 8) & 0xFF), (r & 0xFF));

		open_read_close(fname);
	}
	close(nullFileHandle);
	printf("Success:\t%d\nFail:\t%d\n", openlog[SUCCESS], openlog[FAIL]);
	return 0;
}

int open_read_close(char *fname)
{
	int fileHandle, fileHandle2;
	char buffer[BUFFSIZE];
	int c;

	if ((fileHandle = open(fname, O_RDONLY | O_SYNC | O_ASYNC)) < 0) {
		openlog[FAIL]++;
		printf("ERROR:opening file %s failed %d \n", fname, errno);
		return (errno);
	}

	if ((fileHandle2 = open(fname, O_RDONLY | O_SYNC | O_ASYNC)) < 0) {
		openlog[FAIL]++;
		printf("ERROR:2nd opening file %s failed %d \n", fname, errno);
		return (errno);
	}

	openlog[SUCCESS]++;

	while ((c = read(fileHandle, buffer, BUFFSIZE)) > 0) {
		if (write(nullFileHandle, buffer, c) < 0) {
			perror("/dev/null");
			printf("Opened\t %d\nUnopend:\t%d\n", openlog[SUCCESS],
			       openlog[FAIL]);
			close(fileHandle2);
			close(fileHandle);
			return (errno);
		}
		if ((c = read(fileHandle2, buffer, BUFFSIZE)) > 0) {
			if (write(nullFileHandle, buffer, c) < 0) {
				perror("/dev/null");
				printf("Opened\t %d\nUnopend:\t%d\n",
				       openlog[SUCCESS], openlog[FAIL]);
				close(fileHandle2);
				close(fileHandle);
				return (errno);
			}
		}
	}

	if (c < 0) {
		perror(fname);
		printf("Opened\t %d\nUnopend:\t%d\n", openlog[SUCCESS],
		       openlog[FAIL]);
		return (errno);
	}

	close(fileHandle2);
	close(fileHandle);
	return 0;
}

int create_or_delete(char *fname)
{
	int r, rc;

	r = (random() & 1);

	/* create */
	if ((create_file(fname) == 0)) {
		rc = delete_file(fname);
	} else {
		printf("Error: %d creating random file \n", errno);
	}

	if ((errorCount > dFileCount || errorCount > cFileCount)
	    && (errorCount > MAXERROR)) {
		fprintf(stderr, "Too many errors -- Aborting test\n");
		fprintf(stderr, "Total create files: %d\n", cFileCount);
		fprintf(stderr, "Total delete files: %d\n", dFileCount);
		fprintf(stderr, "Total error       : %d\n", errorCount);
		return (MAXERROR);
	}

	return 0;
}

int do_random_create_delete(int maxNum)
{
	int r, rc = 0;
	char fname[1024];
	time_t t;
	int i;

	printf("Running random create/delete test...\n");

	if (maxNum < 1 || maxNum > MAXNUM) {
		printf("MAX out of size %d\n", maxNum);
		return (maxNum);
	}

	time(&t);
	srandom((unsigned int)getpid() ^
		(((unsigned int)t << 16) | (unsigned int)t >> 16));

	/* 00/00/00/00 */
	for (i = 0; i < maxNum && rc != MAXERROR; i++) {
		r = random() % maxNum;
		sprintf(fname, "00/%2.2x/%2.2x/00%2.2x%2.2x%2.2x",
			((r >> 16) & 0xFF),
			((r >> 8) & 0xFF),
			((r >> 16) & 0xFF), ((r >> 8) & 0xFF), (r & 0xFF));

		rc = create_or_delete(fname);
	}

	fprintf(stderr, "Total create files: %d\n", cFileCount);
	fprintf(stderr, "Total delete files: %d\n", dFileCount);
	fprintf(stderr, "Total error       : %d\n", errorCount);
	return (rc);
}
