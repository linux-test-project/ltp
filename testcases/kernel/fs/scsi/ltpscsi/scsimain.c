/* Test code for D. Gilbert's extensions to the Linux OS SCSI generic ("sg")
   device driver.
*  Copyright (C) 1999 - 2002 D. Gilbert
*  This program is free software; you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation; either version 2, or (at your option)
*  any later version.

   This program scans the "sg" device space (ie actual + simulated SCSI
   generic devices).
   Options: -w   open writable (new driver opens readable unless -i)
            -n   numeric scan: scan /dev/sg0,1,2, ....
            -a   alpha scan: scan /dev/sga,b,c, ....
            -i   do SCSI inquiry on device (implies -w)
            -x   extra information output

   By default this program will look for /dev/sg0 first (i.e. numeric scan)

   Note: This program is written to work under both the original and
   the new sg driver.

   Version 1.00 20031022

   F. Jansen - modification to extend beyond 26 sg devices.
   M. Ridgeway - Roll code together for SCSI testing with one command line

6 byte INQUIRY command:
[0x12][   |lu][pg cde][res   ][al len][cntrl ]
*/

#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include <limits.h>
#include <time.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <ctype.h>
#include <pthread.h>
#include <sys/sysmacros.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <linux/major.h>
#include "sg_include.h"
#include "sg_err.h"
#include "llseek.h"

#define ME "scsimain: "

#ifndef O_DIRECT
#define O_DIRECT 040000
#endif

#define NUMERIC_SCAN_DEF 1	/* change to 0 to make alpha scan default */
//static char * version_str = "0.21 20030513";

#define BPI (signed)(sizeof(int))
#define READWRITE_BASE_NUM 0x12345678
#define DEF_BLOCK_SIZE 512
#define DEF_NUM_THREADS 16
#define MAX_NUM_THREADS SG_MAX_QUEUE
#define DEF_BLOCKS_PER_TRANSFER 128
#define DEF_SCSI_CDBSZ 10
#define MAX_SCSI_CDBSZ 16
#define TUR_CMD_LEN 6
#define DEVNAME_SZ 256
#define MAX_HOLES 4

#define OFF sizeof(struct sg_header)
#define INQ_REPLY_LEN 96	/* logic assumes >= sizeof(inqCmdBlk) */
#define INQUIRY_CMDLEN  6
#define INQUIRY_CMD     0x12
#define SENSE_BUFF_LEN 32	/* Arbitrary, could be larger */
#define DEF_TIMEOUT 60000	/* 60,000 millisecs == 60 seconds */
#define REASON_SZ 128

#define SENSE_BUFF_SZ 64
#define RCAP_REPLY_LEN 8
#define LOG_SENSE_CMD     0x4d
#define LOG_SENSE_CMDLEN  10
#define MX_ALLOC_LEN (1024 * 17)
#define D_ROOT_SZ 512
#define STR_SZ 1024
#define INOUTF_SZ 512
#define EBUFF_SZ 512
#define MDEV_NAME_SZ 256

#define PG_CODE_ALL 0x00

#define TRUE 1
#define FALSE 0
#define MAX_DEVICES 50

#define NAME_LEN_MAX 256
#define LEVELS 4

#define SENSE_BUFF_LEN 32	/* Arbitrary, could be larger */
#define INQ_ALLOC_LEN 255

#ifndef SCSI_IOCTL_GET_PCI
#define SCSI_IOCTL_GET_PCI 0x5387
#endif

#define READ_CAP_REPLY_LEN 8

#ifndef RAW_MAJOR
#define RAW_MAJOR 255		/*unlikey value */
#endif

#define FT_OTHER 1		/* filetype is probably normal */
#define FT_SG 2			/* filetype is sg char device or supports
				   SG_IO ioctl */
#define FT_RAW 4		/* filetype is raw char device */
#define FT_DEV_NULL 8		/* either "/dev/null" or "." as filename */
#define FT_ST 16		/* filetype is st char device (tape) */
#define FT_BLOCK 32		/* filetype is block device */

#define DEV_NULL_MINOR_NUM 3

#ifdef SG_GET_RESERVED_SIZE
#define OPEN_FLAG O_RDONLY
#else
#define OPEN_FLAG O_RDWR
#endif

#ifndef SG_MAX_SENSE
#define SG_MAX_SENSE 16
#endif

#define TEST_START 0
#define TEST_BREAK 1
#define TEST_STOP  2
#define MAX_SG_DEVS 128
#define MAX_SD_DEVS 128
#define MAX_SR_DEVS 128
#define MAX_ST_DEVS 128
#define MAX_OSST_DEVS 128
#define MAX_ERRORS 5

#define LIN_DEV_TYPE_UNKNOWN 0
#define LIN_DEV_TYPE_SD 1
#define LIN_DEV_TYPE_SR 2
#define LIN_DEV_TYPE_ST 3
#define LIN_DEV_TYPE_SCD 4
#define LIN_DEV_TYPE_OSST 5

#define MODE_SENSE6_CMD      0x1a
#define MODE_SENSE6_CMDLEN   6
#define MODE_SENSE10_CMD     0x5a
#define MODE_SENSE10_CMDLEN  10
#define INQUIRY_CMD     0x12
#define INQUIRY_CMDLEN  6
#define MODE_ALLOC_LEN (1024 * 4)

#define MODE_CODE_ALL 0x3f

#define RB_MODE_DESC 3
#define RB_MODE_DATA 2
#define RB_DESC_LEN 4
#define RB_MB_TO_READ 200
#define RB_OPCODE 0x3C
#define RB_CMD_LEN 10

/* #define SG_DEBUG */

#ifndef SG_FLAG_MMAP_IO
#define SG_FLAG_MMAP_IO 4
#endif
#ifndef SG_SCSI_RESET
#define SG_SCSI_RESET 0x2284
#endif

#ifndef SG_SCSI_RESET_NOTHING
#define SG_SCSI_RESET_NOTHING 0
#define SG_SCSI_RESET_DEVICE 1
#define SG_SCSI_RESET_BUS 2
#define SG_SCSI_RESET_HOST 3
#endif
#define LONG_TIMEOUT 2400000	/* 2,400,000 millisecs == 40 minutes */

#define SEND_DIAGNOSTIC_CMD     0x1d
#define SEND_DIAGNOSTIC_CMDLEN  6
#define RECEIVE_DIAGNOSTIC_CMD     0x1c
#define RECEIVE_DIAGNOSTIC_CMDLEN  6

#define START_STOP		0x1b
#define SYNCHRONIZE_CACHE	0x35

#define DEF_START_TIMEOUT 120000	/* 120,000 millisecs == 2 minutes */

#define DEVICE_RESET 0
#define HOST_RESET   1
#define BUS_RESET    2
#define SG_HSZ sizeof(struct sg_header)
#define OFFSET_HEADER (SG_HSZ - (2 * sizeof(int)))
#define SIZEOF_BUFFER (256*1024)
#define SIZEOF_BUFFER1 (16*1024)
#define MAXPARM 32

#define SETUP_MODE_PAGE(NPAGE, NPARAM)          \
  status = get_mode_page(NPAGE, page_code);     \
  if (status) { printf("\n"); return status; }   \
  bdlen = buffer[11];                           \
  pagestart = buffer + 12 + bdlen;

typedef struct request_collection {	/* one instance visible to all threads */
	int infd;
	int skip;
	int in_type;
	int in_scsi_type;
	int in_blk;		/* -\ next block address to read */
	int in_count;		/*  | blocks remaining for next read */
	int in_done_count;	/*  | count of completed in blocks */
	int in_partial;		/*  | */
	int in_stop;		/*  | */
	pthread_mutex_t in_mutex;	/* -/ */
	int outfd;
	int seek;
	int out_type;
	int out_scsi_type;
	int out_blk;		/* -\ next block address to write */
	int out_count;		/*  | blocks remaining for next write */
	int out_done_count;	/*  | count of completed out blocks */
	int out_partial;	/*  | */
	int out_stop;		/*  | */
	pthread_mutex_t out_mutex;	/*  | */
	pthread_cond_t out_sync_cv;	/* -/ hold writes until "in order" */
	int bs;
	int bpt;
	int fua_mode;
	int dio;
	int dio_incomplete;	/* -\ */
	int sum_of_resids;	/*  | */
	pthread_mutex_t aux_mutex;	/* -/ (also serializes some printf()s */
	int coe;
	int cdbsz;
	int debug;
} Rq_coll;

typedef struct request_element {	/* one instance per worker thread */
	int infd;
	int outfd;
	int wr;
	int blk;
	int num_blks;
	unsigned char *buffp;
	unsigned char *alloc_bp;
	sg_io_hdr_t io_hdr;
	unsigned char cmd[MAX_SCSI_CDBSZ];
	unsigned char sb[SENSE_BUFF_LEN];
	int bs;
	int fua_mode;
	int dio;
	int dio_incomplete;
	int resid;
	int in_scsi_type;
	int out_scsi_type;
	int cdbsz;
	int debug;
} Rq_elem;

typedef struct my_map_info {
	int active;
	int lin_dev_type;
	int oth_dev_num;
	struct sg_scsi_id sg_dat;
	char vendor[8];
	char product[16];
	char revision[4];
} my_map_info_t;

typedef struct sg_map {
	int bus;
	int channel;
	int target_id;
	int lun;
	char *dev_name;
} Sg_map;

typedef struct my_scsi_idlun {
/* why can't userland see this structure ??? */
	int dev_id;
	int host_unique_id;
} My_scsi_idlun;

struct page_code_desc {
	int page_code;
	const char *desc;
};

static const char *pg_control_str_arr[] = {
	"current",
	"changeable",
	"default",
	"saved"
};

char *devices[] =
    { "/dev/sda", "/dev/sdb", "/dev/sdc", "/dev/sdd", "/dev/sde", "/dev/sdf",
	"/dev/sdg", "/dev/sdh", "/dev/sdi", "/dev/sdj", "/dev/sdk", "/dev/sdl",
	"/dev/sdm", "/dev/sdn", "/dev/sdo", "/dev/sdp", "/dev/sdq", "/dev/sdr",
	"/dev/sds", "/dev/sdt", "/dev/sdu", "/dev/sdv", "/dev/sdw", "/dev/sdx",
	"/dev/sdy", "/dev/sdz", "/dev/sdaa", "/dev/sdab", "/dev/sdac",
	    "/dev/sdad",
	"/dev/scd0", "/dev/scd1", "/dev/scd2", "/dev/scd3", "/dev/scd4",
	    "/dev/scd5",
	"/dev/scd6", "/dev/scd7", "/dev/scd8", "/dev/scd9", "/dev/scd10",
	    "/dev/scd11",
	"/dev/sr0", "/dev/sr1", "/dev/sr2", "/dev/sr3", "/dev/sr4", "/dev/sr5",
	"/dev/sr6", "/dev/sr7", "/dev/sr8", "/dev/sr9", "/dev/sr10",
	    "/dev/sr11",
	"/dev/nst0", "/dev/nst1", "/dev/nst2", "/dev/nst3", "/dev/nst4",
	    "/dev/nst5",
	"/dev/nosst0", "/dev/nosst1", "/dev/nosst2", "/dev/nosst3",
	    "/dev/nosst4"
};

static char *page_names[] = {
	NULL,
	"Read-Write Error Recovery",
	"Disconnect-Reconnect",
	"Format Device",
	"Rigid Disk Geometry",
	/* "Flexible Disk" */ NULL,
	NULL,
	"Verify Error Recovery",
	"Caching",
	"Peripheral Device",
	"Control Mode",
	/* "Medium Types Supported" */ NULL,
	"Notch and Partition",
	/* "CD-ROM" */ NULL,
	/* "CD-ROM Audio Control" */ NULL,
	NULL,
	/* "Medium Partition (1)" */ NULL,
	/* "Medium Partition (2)" */ NULL,
	/* "Medium Partition (3)" */ NULL,
	/* "Medium Partition (4)" */ NULL
};

#define MAX_PAGENO (sizeof(page_names)/sizeof(char *))

/* Following 2 macros from D.R. Butenhof's POSIX threads book:
   ISBN 0-201-63392-2 . [Highly recommended book.] */
#define err_exit(code,text) do { \
    fprintf(stderr, "%s at \"%s\":%d: %s\n", \
        text, __FILE__, __LINE__, strerror(code)); \
    exit(1); \
    } while (0)

static Sg_map sg_map_arr[(sizeof(devices) / sizeof(char *)) + 1];

static const unsigned char scsi_command_size[8] = { 6, 10, 10, 12,
	12, 12, 10, 10
};
const unsigned char rbCmdBlk[10] = { READ_BUFFER, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
static const char *level_arr[LEVELS] = { "host", "bus", "target", "lun" };

static const char *proc_allow_dio = "/proc/scsi/sg/allow_dio";
static const char *devfs_id = "/dev/.devfsd";
static my_map_info_t map_arr[MAX_SG_DEVS];
static char ebuff[EBUFF_SZ];
static int glob_fd;
static char defectformat = 0x4;
static sigset_t signal_set;
static pthread_t sig_listen_thread_id;

static int do_ide = 0;
static int do_inq = 1;
static int do_leaf = 1;
static int do_extra = 1;
static int do_quiet = 0;
static int checked_sg = 1;
static int sum_of_resids = 0;

static int dd_count = -1;
static int in_full = 0;
static int in_partial = 0;
static int out_full = 0;
static int out_partial = 0;
static int do_coe = 0;
int base = READWRITE_BASE_NUM;
unsigned char *cmpbuf = 0;
static unsigned char buff_a[SIZEOF_BUFFER + SG_HSZ + 12];
static unsigned char *buffer = buff_a + OFFSET_HEADER;

typedef struct my_sg_scsi_id {
	int host_no;		/* as in "scsi<n>" where 'n' is one of 0, 1, 2 etc */
	int channel;
	int scsi_id;		/* scsi id of target device */
	int lun;
	int scsi_type;		/* TYPE_... defined in scsi/scsi.h */
	short h_cmd_per_lun;	/* host (adapter) maximum commands per lun */
	short d_queue_depth;	/* device (or adapter) maximum queue length */
	int unused1;		/* probably find a good use, set 0 for now */
	int unused2;		/* ditto */
} My_sg_scsi_id;

// Prototypes
int do_scsi_sgp_read_write(char *device);
int do_scsi_sgm_read_write(char *device);
void sg_in_operation(Rq_coll * clp, Rq_elem * rep);
void sg_out_operation(Rq_coll * clp, Rq_elem * rep);
int normal_in_operation(Rq_coll * clp, Rq_elem * rep, int blocks);
void normal_out_operation(Rq_coll * clp, Rq_elem * rep, int blocks);
int sg_start_io(Rq_elem * rep);
int sg_finish_io(int wr, Rq_elem * rep, pthread_mutex_t * a_mutp);
int run_sg_scan_tests(void);
int show_scsi_logs(char *device);
int validate_device(char *device);
int show_devfs_devices(void);
void usage(void);
int do_scsi_device_read_write(char *device);
int do_scsi_inquiry(char *device, int hex_flag);
int show_scsi_maps(void);
int show_scsi_modes(char *device);
int do_scsi_read_buffer(char *device);
int show_scsi_read_capacity(char *device);
int do_scsi_reset_devices(char *device, int reset_opts);
int do_scsi_send_diagnostics(char *device);
int do_scsi_start_stop(char *device, int startstop);
int do_scsi_read_write_buffer(char *device);
int do_scsi_test_unit_ready(char *device);
int show_scsi_info(char *device);
void print_msg(int msg_num, const char *msg);
static void scan_dev_type(const char *leadin, int max_dev, int do_numeric,
			  int lin_dev_type, int last_sg_ind);

#ifdef SG_IO
int sg3_inq(int sg_fd, unsigned char *inqBuff, int do_extra);
#endif

static unsigned char inqCmdBlk[INQUIRY_CMDLEN] =
    { 0x12, 0, 0, 0, INQ_REPLY_LEN, 0 };

void print_msg(int msg_num, const char *msg)
{
	switch (msg_num) {
	case TEST_START:
		printf
		    ("\n****************** Starting Tests ***************************\n");
		break;
	case TEST_STOP:
		printf
		    ("\n****************** Tests Complete ***************************\n");
		break;
	case TEST_BREAK:
		printf("\n------------------ %s Test ------------------\n\n",
		       msg);
		break;
	}
}

int main(int argc, char *argv[])
{
	int rc = 0;

	if (argc < 2) {
		printf("\n\nERROR:No device passed to test\n\n");
		usage();
		return 1;
	}

	rc = validate_device(argv[1]);
	if (rc == 0) {

		print_msg(TEST_START, NULL);

		rc = run_sg_scan_tests();
		if (rc != 0) {
			printf("ERROR: run_sg_scan_tests failed %d\n", rc);
		}

		rc = show_scsi_logs(argv[1]);
		if (rc != 0) {
			printf("ERROR: show_scsi_logs failed %d\n", rc);
		}

		rc = show_devfs_devices();
		if (rc != 0) {
			printf("ERROR: show_devfs_devices failed %d\n", rc);
		}

		rc = do_scsi_device_read_write(argv[1]);
		if (rc != 0) {
			printf("ERROR: do_scsi_devices_read_write failed %d\n",
			       rc);
		}

		rc = do_scsi_inquiry(argv[1], TRUE);
		if (rc != 0) {
			printf("ERROR: do_scsi_inquiry HEX failed %d\n", rc);
		} else {
			rc = do_scsi_inquiry(argv[1], FALSE);
			if (rc != 0) {
				printf("ERROR: do_scsi_inquiry PCI failed %d\n",
				       rc);
			}
		}

		rc = show_scsi_maps();
		if (rc != 0) {
			printf("ERROR: show_scsi_maps failed %d\n", rc);
		}

		rc = show_scsi_modes(argv[1]);
		if (rc != 0) {
			printf("ERROR: show_scsi_modes failed %d\n", rc);
		}

		rc = do_scsi_read_buffer(argv[1]);
		if (rc != 0 && rc != 1) {
			printf("ERROR: do_scsi_read_buffer failed %d\n", rc);
		}

		rc = show_scsi_read_capacity(argv[1]);
		if (rc != 0) {
			printf("ERROR: show_scsi_read_capacity failed %d\n",
			       rc);
		}

		rc |= do_scsi_reset_devices(argv[1], DEVICE_RESET);
		rc |= do_scsi_reset_devices(argv[1], BUS_RESET);
		rc |= do_scsi_reset_devices(argv[1], HOST_RESET);
		if (rc != 0) {
			printf("ERROR: do_scsi_reset_devices failed %d\n", rc);
		}

		rc = do_scsi_send_diagnostics(argv[1]);
		if (rc != 0) {
			printf("ERROR: do_scsi_send_diagnostics failed %d\n",
			       rc);
		}

		rc |= do_scsi_start_stop(argv[1], FALSE);
		rc |= do_scsi_start_stop(argv[1], TRUE);
		if (rc != 0) {
			printf("ERROR: do_scsi_start_top failed %d\n", rc);
		}

		rc = do_scsi_read_write_buffer(argv[1]);
		if (rc != 0 && rc != 1) {
			printf("ERROR: do_scsi_read_write_buffer failed %d\n",
			       rc);
		}

		rc = do_scsi_test_unit_ready(argv[1]);
		if (rc != 0) {
			printf("ERROR: do_scsi_test_unit_ready failed %d\n",
			       rc);
		}

		rc = show_scsi_info(argv[1]);
		if (rc != 0) {
			printf("ERROR: show_scsi_info failed %d\n", rc);
		}

		rc = do_scsi_sgp_read_write(argv[1]);
		if (rc != 0) {
			printf("ERROR: do_scsi_sgp_read_write failed %d\n", rc);
		}

		rc = do_scsi_sgm_read_write(argv[1]);
		if (rc != 0) {
			printf("ERROR: do_scsi_sgm_read_write failed %d\n", rc);
		}

		print_msg(TEST_STOP, NULL);
	} else {
		printf("\nERROR: Invalid device passed to test\n\n\n");
		usage();

	}

	return 0;
}

int validate_device(char *device)
{
	int rc = 0;
	int i, found = FALSE;
	char device_string[25];

	for (i = 0; i < MAX_DEVICES && !found; i++) {
		sprintf(device_string, "/dev/sg%d", i);
		//printf("checking %s \n", device_string);
		if (strcmp(device, device_string) == 0) {
			found = TRUE;
		}
	}

	return rc;
}

void usage()
{
	printf("Usage: 'sg_scan [-a] [-n] [-w] [-i] [-x]'\n");
	printf("    where: -a   do alpha scan (ie sga, sgb, sgc)\n");
	printf("           -n   do numeric scan (ie sg0, sg1...) [default]\n");
	printf("           -w   force open with read/write flag\n");
	printf("           -i   do SCSI INQUIRY, output results\n");
	printf("           -x   extra information output about queuing\n\n\n");
}

void make_dev_name(char *fname, const char *leadin, int k, int do_numeric)
{
	char buff[64];
	int big, little;

	strcpy(fname, leadin ? leadin : "/dev/sg");
	if (do_numeric) {
		sprintf(buff, "%d", k);
		strcat(fname, buff);
	} else {
		if (k < 26) {
			buff[0] = 'a' + (char)k;
			buff[1] = '\0';
			strcat(fname, buff);
		} else if (k <= 255) {	/* assumes sequence goes x,y,z,aa,ab,ac etc */
			big = k / 26;
			little = k - (26 * big);
			big = big - 1;

			buff[0] = 'a' + (char)big;
			buff[1] = 'a' + (char)little;
			buff[2] = '\0';
			strcat(fname, buff);
		} else
			strcat(fname, "xxxx");
	}
}

int run_sg_scan_tests()
{
	int sg_fd, res, k, f;
	unsigned char inqBuff[OFF + INQ_REPLY_LEN];
	int inqInLen = OFF + sizeof(inqCmdBlk);
	int inqOutLen = OFF + INQ_REPLY_LEN;
	unsigned char *buffp = inqBuff + OFF;
	struct sg_header *isghp = (struct sg_header *)inqBuff;
	int do_numeric = NUMERIC_SCAN_DEF;
	int do_inquiry = 0;
	int do_extra = 1;
	int writeable = 0;
	int num_errors = 0;
	int num_silent = 0;
	int eacces_err = 0;
	char fname[64];
	My_scsi_idlun my_idlun;
	int host_no;
	int flags;
	int emul;

	print_msg(TEST_BREAK, __FUNCTION__);

	flags = writeable ? O_RDWR : OPEN_FLAG;

	do_numeric = 1;
	writeable = O_RDONLY;
	do_inquiry = 1;
	do_extra = 1;

	for (k = 0, res = 0; (k < 1000) && (num_errors < MAX_ERRORS);
	     ++k, res = (sg_fd >= 0) ? close(sg_fd) : 0) {
		if (res < 0) {
			snprintf(ebuff, EBUFF_SZ, ME "Error closing %s ",
				 fname);
			perror(ME "close error");
			return 1;
		}
		make_dev_name(fname, NULL, k, do_numeric);

		sg_fd = open(fname, flags | O_NONBLOCK);
		if (sg_fd < 0) {
			if (EBUSY == errno) {
				printf
				    ("%s: device busy (O_EXCL lock), skipping\n",
				     fname);
				continue;
			} else if ((ENODEV == errno) || (ENOENT == errno) ||
				   (ENXIO == errno)) {
				++num_errors;
				++num_silent;
				continue;
			} else {
				if (EACCES == errno)
					eacces_err = 1;
				snprintf(ebuff, EBUFF_SZ,
					 ME "Error opening %s ", fname);
				perror(ebuff);
				++num_errors;
				continue;
			}
		}
		res = ioctl(sg_fd, SCSI_IOCTL_GET_IDLUN, &my_idlun);
		if (res < 0) {
			snprintf(ebuff, EBUFF_SZ,
				 ME "device %s failed on scsi ioctl, skip",
				 fname);
			perror(ebuff);
			++num_errors;
			continue;
		}
		res = ioctl(sg_fd, SCSI_IOCTL_GET_BUS_NUMBER, &host_no);
		if (res < 0) {
			snprintf(ebuff, EBUFF_SZ, ME "device %s failed on scsi "
				 "ioctl(2), skip", fname);
			perror(ebuff);
			++num_errors;
			continue;
		}
#ifdef SG_EMULATED_HOST
		res = ioctl(sg_fd, SG_EMULATED_HOST, &emul);
		if (res < 0) {
			snprintf(ebuff, EBUFF_SZ,
				 ME "device %s failed on sg ioctl(3), skip",
				 fname);
			perror(ebuff);
			++num_errors;
			continue;
		}
#else
		emul = 0;
#endif
		printf("%s: scsi%d channel=%d id=%d lun=%d", fname, host_no,
		       (my_idlun.dev_id >> 16) & 0xff, my_idlun.dev_id & 0xff,
		       (my_idlun.dev_id >> 8) & 0xff);
		if (emul)
			printf(" [em]");
#if 0
		printf(", huid=%d", my_idlun.host_unique_id);
#endif
#ifdef SG_GET_RESERVED_SIZE
		{
			My_sg_scsi_id m_id;	/* compatible with sg_scsi_id_t in sg.h */

			res = ioctl(sg_fd, SG_GET_SCSI_ID, &m_id);
			if (res < 0) {
				snprintf(ebuff, EBUFF_SZ,
					 ME "device %s ioctls(4), skip", fname);
				perror(ebuff);
				++num_errors;
				continue;
			}
			printf("  type=%d", m_id.scsi_type);
			if (do_extra)
				printf(" cmd_per_lun=%hd queue_depth=%hd\n",
				       m_id.h_cmd_per_lun, m_id.d_queue_depth);
			else
				printf("\n");
		}
#else
		printf("\n");
#endif
		if (!do_inquiry)
			continue;

#ifdef SG_IO
		if ((ioctl(sg_fd, SG_GET_VERSION_NUM, &f) >= 0) && (f >= 30000)) {
			res = sg3_inq(sg_fd, inqBuff, do_extra);
			continue;
		}
#endif
		memset(isghp, 0, sizeof(struct sg_header));
		isghp->reply_len = inqOutLen;
		memcpy(inqBuff + OFF, inqCmdBlk, INQUIRY_CMDLEN);

		if (O_RDWR == (flags & O_ACCMODE)) {	/* turn on blocking */
			f = fcntl(sg_fd, F_GETFL);
			fcntl(sg_fd, F_SETFL, f & (~O_NONBLOCK));
		} else {
			close(sg_fd);
			sg_fd = open(fname, O_RDWR);
		}

		res = write(sg_fd, inqBuff, inqInLen);
		if (res < 0) {
			snprintf(ebuff, EBUFF_SZ, ME "device %s writing, skip",
				 fname);
			perror(ebuff);
			++num_errors;
			continue;
		}
		res = read(sg_fd, inqBuff, inqOutLen);
		if (res < 0) {
			snprintf(ebuff, EBUFF_SZ, ME "device %s reading, skip",
				 fname);
			perror(ebuff);
			++num_errors;
			continue;
		}
#ifdef SG_GET_RESERVED_SIZE
		if (!sg_chk_n_print("Error from Inquiry", isghp->target_status,
				    isghp->host_status, isghp->driver_status,
				    isghp->sense_buffer, SG_MAX_SENSE))
			continue;
#else
		if ((isghp->result != 0) || (0 != isghp->sense_buffer[0])) {
			printf("Error from Inquiry: result=%d\n",
			       isghp->result);
			if (0 != isghp->sense_buffer[0])
				sg_print_sense("Error from Inquiry",
					       isghp->sense_buffer,
					       SG_MAX_SENSE);
			continue;
		}
#endif
		f = (int)*(buffp + 7);
		printf("    %.8s  %.16s  %.4s ", buffp + 8, buffp + 16,
		       buffp + 32);
		printf("[wide=%d sync=%d cmdq=%d sftre=%d pq=0x%x]\n",
		       ! !(f & 0x20), ! !(f & 0x10), ! !(f & 2), ! !(f & 1),
		       (*buffp & 0xe0) >> 5);
	}
	if ((num_errors >= MAX_ERRORS) && (num_silent < num_errors)) {
		printf("Stopping because there are too many error\n");
		if (eacces_err)
			printf("    root access may be required\n");
	}
	return 0;
}

#ifdef SG_IO
int sg3_inq(int sg_fd, unsigned char *inqBuff, int do_extra)
{
	sg_io_hdr_t io_hdr;
	unsigned char sense_buffer[32];
	int ok;

	memset(&io_hdr, 0, sizeof(sg_io_hdr_t));
	io_hdr.interface_id = 'S';
	io_hdr.cmd_len = sizeof(inqCmdBlk);
	io_hdr.mx_sb_len = sizeof(sense_buffer);
	io_hdr.dxfer_direction = SG_DXFER_FROM_DEV;
	io_hdr.dxfer_len = INQ_REPLY_LEN;
	io_hdr.dxferp = inqBuff;
	io_hdr.cmdp = inqCmdBlk;
	io_hdr.sbp = sense_buffer;
	io_hdr.timeout = 20000;	/* 20000 millisecs == 20 seconds */

	if (ioctl(sg_fd, SG_IO, &io_hdr) < 0) {
		perror(ME "Inquiry SG_IO ioctl error");
		return 1;
	}

	/* now for the error processing */
	ok = 0;
	switch (sg_err_category3(&io_hdr)) {
	case SG_ERR_CAT_CLEAN:
	case SG_ERR_CAT_RECOVERED:
		ok = 1;
		break;
	default:		/* won't bother decoding other categories */
		sg_chk_n_print3("INQUIRY command error", &io_hdr);
		break;
	}

	if (ok) {		/* output result if it is available */
		char *p = (char *)inqBuff;
		int f = (int)*(p + 7);
		printf("    %.8s  %.16s  %.4s ", p + 8, p + 16, p + 32);
		printf("[wide=%d sync=%d cmdq=%d sftre=%d pq=0x%x] ",
		       ! !(f & 0x20), ! !(f & 0x10), ! !(f & 2), ! !(f & 1),
		       (*p & 0xe0) >> 5);
		if (do_extra)
			printf("dur=%ums\n", io_hdr.duration);
		else
			printf("\n");
	}
	return 0;
}
#endif

static int do_logs(int sg_fd, int ppc, int sp, int pc, int pg_code,
		   int paramp, void *resp, int mx_resp_len, int noisy)
{
	int res;
	unsigned char logsCmdBlk[LOG_SENSE_CMDLEN] =
	    { LOG_SENSE_CMD, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	unsigned char sense_b[SENSE_BUFF_LEN];
	sg_io_hdr_t io_hdr;

	logsCmdBlk[1] = (unsigned char)((ppc ? 2 : 0) | (sp ? 1 : 0));
	logsCmdBlk[2] = (unsigned char)(((pc << 6) & 0xc0) | (pg_code & 0x3f));
	logsCmdBlk[5] = (unsigned char)((paramp >> 8) & 0xff);
	logsCmdBlk[6] = (unsigned char)(paramp & 0xff);
	if (mx_resp_len > 0xffff) {
		printf(ME "mx_resp_len too big\n");
		return -1;
	}
	logsCmdBlk[7] = (unsigned char)((mx_resp_len >> 8) & 0xff);
	logsCmdBlk[8] = (unsigned char)(mx_resp_len & 0xff);

	memset(&io_hdr, 0, sizeof(sg_io_hdr_t));
	io_hdr.interface_id = 'S';
	io_hdr.cmd_len = sizeof(logsCmdBlk);
	io_hdr.mx_sb_len = sizeof(sense_b);
	io_hdr.dxfer_direction = SG_DXFER_FROM_DEV;
	io_hdr.dxfer_len = mx_resp_len;
	io_hdr.dxferp = resp;
	io_hdr.cmdp = logsCmdBlk;
	io_hdr.sbp = sense_b;
	io_hdr.timeout = DEF_TIMEOUT;

	if (ioctl(sg_fd, SG_IO, &io_hdr) < 0) {
		perror("SG_IO (log sense) error");
		return -1;
	}
#if 0
	printf("SG_IO ioctl: status=%d, info=%d, sb_len_wr=%d\n",
	       io_hdr.status, io_hdr.info, io_hdr.sb_len_wr);
#endif
	res = sg_err_category3(&io_hdr);
	switch (res) {
	case SG_ERR_CAT_CLEAN:
	case SG_ERR_CAT_RECOVERED:
		return 0;
	default:
		if (noisy) {
			char ebuff[EBUFF_SZ];
			snprintf(ebuff, EBUFF_SZ, ME "ppc=%d, sp=%d, "
				 "pc=%d, page_code=%x, paramp=%x\n    ", ppc,
				 sp, pc, pg_code, paramp);
			sg_chk_n_print3(ebuff, &io_hdr);
		}
		return -1;
	}
}

static void dStrHex(const char *str, int len, int no_ascii)
{
	const char *p = str;
	unsigned char c;
	char buff[82];
	int a = 0;
	const int bpstart = 5;
	const int cpstart = 60;
	int cpos = cpstart;
	int bpos = bpstart;
	int i, k;

	if (len <= 0)
		return;
	memset(buff, ' ', 80);
	buff[80] = '\0';
	k = sprintf(buff + 1, "%.2x", a);
	buff[k + 1] = ' ';
	if (bpos >= ((bpstart + (9 * 3))))
		bpos++;

	for (i = 0; i < len; i++) {
		c = *p++;
		bpos += 3;
		if (bpos == (bpstart + (9 * 3)))
			bpos++;
		sprintf(&buff[bpos], "%.2x", (int)(unsigned char)c);
		buff[bpos + 2] = ' ';
		if (no_ascii)
			buff[cpos++] = ' ';
		else {
			if ((c < ' ') || (c >= 0x7f))
				c = '.';
			buff[cpos++] = c;
		}
		if (cpos > (cpstart + 15)) {
			printf("%s\n", buff);
			bpos = bpstart;
			cpos = cpstart;
			a += 16;
			memset(buff, ' ', 80);
			k = sprintf(buff + 1, "%.2x", a);
			buff[k + 1] = ' ';
		}
	}
	if (cpos > cpstart) {
		printf("%s\n", buff);
	}
}

static void show_page_name(int page_no)
{
	switch (page_no) {
	case 0x0:
		printf("    0x00    Supported log pages\n");
		break;
	case 0x1:
		printf("    0x01    Buffer over-run/under-run\n");
		break;
	case 0x2:
		printf("    0x02    Error counters (write)\n");
		break;
	case 0x3:
		printf("    0x03    Error counters (read)\n");
		break;
	case 0x4:
		printf("    0x04    Error counters (read reverse)\n");
		break;
	case 0x5:
		printf("    0x05    Error counters (verify)\n");
		break;
	case 0x6:
		printf("    0x06    Non-medium errors\n");
		break;
	case 0x7:
		printf("    0x07    Last n error events\n");
		break;
	case 0x8:
		printf("    0x08    Format status (sbc2)\n");
		break;
	case 0xb:
		printf("    0x0b    Last n deferred errors of "
		       "asynchronous events\n");
		break;
	case 0xc:
		printf("    0x0c    Sequential Access (ssc-2)\n");
		break;
	case 0xd:
		printf("    0x0d    Temperature\n");
		break;
	case 0xe:
		printf("    0x0e    Start-stop cycle counter\n");
		break;
	case 0xf:
		printf("    0x0f    Application client\n");
		break;
	case 0x10:
		printf("    0x10    Self-test results\n");
		break;
	case 0x18:
		printf("    0x18    Protocol specific port\n");
		break;
	case 0x2e:
		printf("    0x2e    Tape alerts (ssc-2)\n");
		break;
	case 0x2f:
		printf("    0x2f    Informational exceptions (SMART)\n");
		break;
	default:
		printf("    0x%.2x\n", page_no);
		break;
	}
}

static void show_buffer_under_overrun_page(unsigned char *resp, int len)
{
	int k, j, num, pl, count_basis, cause;
	unsigned char *ucp;
	unsigned char *xp;
	unsigned long long ull;

	printf("Buffer over-run/under-run page\n");
	num = len - 4;
	ucp = &resp[0] + 4;
	while (num > 3) {
		pl = ucp[3] + 4;
		count_basis = (ucp[1] >> 5) & 0x7;
		printf("  Count basis: ");
		switch (count_basis) {
		case 0:
			printf("undefined");
			break;
		case 1:
			printf("per command");
			break;
		case 2:
			printf("per failed reconnect");
			break;
		case 3:
			printf("per unit of time");
			break;
		default:
			printf("reserved [0x%x]", count_basis);
			break;
		}
		cause = (ucp[1] >> 1) & 0xf;
		printf(", Cause: ");
		switch (cause) {
		case 0:
			printf("bus busy");
			break;
		case 1:
			printf("transfer rate too slow");
			break;
		default:
			printf("reserved [0x%x]", cause);
			break;
		}
		printf(", Type: ");
		if (ucp[1] & 1)
			printf("over-run");
		else
			printf("under-run");
		printf(", count");
		k = pl - 4;
		xp = ucp + 4;
		if (k > sizeof(ull)) {
			xp += (k - sizeof(ull));
			k = sizeof(ull);
		}
		ull = 0;
		for (j = 0; j < k; ++j) {
			if (j > 0)
				ull <<= 8;
			ull |= xp[j];
		}
		printf(" = %llu\n", ull);
		num -= pl;
		ucp += pl;
	}
}

static void show_error_counter_page(unsigned char *resp, int len)
{
	int k, j, num, pl, pc;
	unsigned char *ucp;
	unsigned char *xp;
	unsigned long long ull;

	switch (resp[0]) {
	case 2:
		printf("Write error counter page\n");
		break;
	case 3:
		printf("Read error counter page\n");
		break;
	case 4:
		printf("Read Reverse error counter page\n");
		break;
	case 5:
		printf("Verify error counter page\n");
		break;
	default:
		printf("expecting error counter page, got page=0x%x\n",
		       resp[0]);
		return;
	}
	num = len - 4;
	ucp = &resp[0] + 4;
	while (num > 3) {
		pc = (ucp[0] << 8) | ucp[1];
		pl = ucp[3] + 4;
		switch (pc) {
		case 0:
			printf("  Errors corrected without substantion delay");
			break;
		case 1:
			printf("  Errors corrected with possible delays");
			break;
		case 2:
			printf("  Total operations");
			break;
		case 3:
			printf("  Total errors corrected");
			break;
		case 4:
			printf("  Total times correction algorithm processed");
			break;
		case 5:
			printf("  Total bytes processed");
			break;
		case 6:
			printf("  Total uncorrected errors");
			break;
		default:
			printf("  Reserved or vendor specific [0x%x]", pc);
			break;
		}
		k = pl - 4;
		xp = ucp + 4;
		if (k > sizeof(ull)) {
			xp += (k - sizeof(ull));
			k = sizeof(ull);
		}
		ull = 0;
		for (j = 0; j < k; ++j) {
			if (j > 0)
				ull <<= 8;
			ull |= xp[j];
		}
		printf(" = %llu\n", ull);
		num -= pl;
		ucp += pl;
	}
}

static void show_non_medium_error_page(unsigned char *resp, int len)
{
	int k, j, num, pl, pc;
	unsigned char *ucp;
	unsigned char *xp;
	unsigned long long ull;

	printf("Non-medium error page\n");
	num = len - 4;
	ucp = &resp[0] + 4;
	while (num > 3) {
		pc = (ucp[0] << 8) | ucp[1];
		pl = ucp[3] + 4;
		switch (pc) {
		case 0:
			printf("  Non-medium error count");
			break;
		default:
			if (pc <= 0x7fff)
				printf("  Reserved [0x%x]", pc);
			else
				printf("  Vendor specific [0x%x]", pc);
			break;
		}
		k = pl - 4;
		xp = ucp + 4;
		if (k > sizeof(ull)) {
			xp += (k - sizeof(ull));
			k = sizeof(ull);
		}
		ull = 0;
		for (j = 0; j < k; ++j) {
			if (j > 0)
				ull <<= 8;
			ull |= xp[j];
		}
		printf(" = %llu\n", ull);
		num -= pl;
		ucp += pl;
	}
}

const char *self_test_code[] = {
	"default", "background short", "background extended", "reserved",
	"aborted background", "foreground short", "foreground extended",
	"reserved"
};

const char *self_test_result[] = {
	"completed without error",
	"aborted by SEND DIAGNOSTIC",
	"aborted other than by SEND DIAGNOSTIC",
	"unknown error, unable to complete",
	"self test completed with failure in test segment (which one unkown)",
	"first segment in self test failed",
	"second segment in self test failed",
	"another segment in self test failed",
	"reserved", "reserved", "reserved", "reserved", "reserved", "reserved",
	"reserved",
	"self test in progress"
};

static void show_self_test_page(unsigned char *resp, int len)
{
	int k, num, n, res;
	unsigned char *ucp;
	unsigned long long ull;

	num = len - 4;
	if (num < 0x190) {
		printf("badly formed self-test results page\n");
		return;
	}
	printf("Self-test results page\n");
	for (k = 0, ucp = resp + 4; k < 20; ++k, ucp += 20) {
		n = (ucp[6] << 8) | ucp[7];
		if ((0 == n) && (0 == ucp[4]))
			break;
		printf("  Parameter code=%d, accumulated power-on hours=%d\n",
		       (ucp[0] << 8) | ucp[1], n);
		printf("    self test code: %s [%d]\n",
		       self_test_code[(ucp[4] >> 5) & 0x7],
		       (ucp[4] >> 5) & 0x7);
		res = ucp[4] & 0xf;
		printf("    self test result: %s [%d]\n",
		       self_test_result[res], res);
		if (ucp[5])
			printf("    self-test number=%d\n", (int)ucp[5]);
		ull = ucp[8];
		ull <<= 8;
		ull |= ucp[9];
		ull <<= 8;
		ull |= ucp[10];
		ull <<= 8;
		ull |= ucp[11];
		ull <<= 8;
		ull |= ucp[12];
		ull <<= 8;
		ull |= ucp[13];
		ull <<= 8;
		ull |= ucp[14];
		ull <<= 8;
		ull |= ucp[14];
		ull <<= 8;
		ull |= ucp[15];
		if ((0xffffffffffffffffULL != ull) && (res > 0) && (res < 0xf))
			printf("    address of first error=0x%llx\n", ull);
		if (ucp[16] & 0xf)
			printf("    sense key=0x%x, asc=0x%x, asq=0x%x\n",
			       ucp[16] & 0xf, ucp[17], ucp[18]);
	}
}

static void show_Temperature_page(unsigned char *resp, int len, int hdr)
{
	int k, num, extra, pc;
	unsigned char *ucp;

	num = len - 4;
	ucp = &resp[0] + 4;
	if (num < 4) {
		printf("badly formed Temperature log page\n");
		return;
	}
	if (hdr)
		printf("Temperature log page\n");
	for (k = num; k > 0; k -= extra, ucp += extra) {
		if (k < 3) {
			printf("short Temperature log page\n");
			return;
		}
		extra = ucp[3] + 4;
		pc = ((ucp[0] << 8) & 0xff) + ucp[1];
		if (0 == pc) {
			if (extra > 5) {
				if (ucp[5] < 0xff)
					printf("  Current temperature= %d C\n",
					       ucp[5]);
				else
					printf
					    ("  Current temperature=<not available>\n");
			}
		} else if (1 == pc) {
			if (extra > 5) {
				if (ucp[5] < 0xff)
					printf
					    ("  Reference temperature= %d C\n",
					     ucp[5]);
				else
					printf
					    ("  Reference temperature=<not available>\n");
			}

		} else {
			printf("  parameter code=0x%x, contents in hex:\n", pc);
			dStrHex((const char *)ucp, extra, 1);
		}
	}
}

static void show_IE_page(unsigned char *resp, int len, int full)
{
	int k, num, extra, pc;
	unsigned char *ucp;

	num = len - 4;
	ucp = &resp[0] + 4;
	if (num < 4) {
		printf("badly formed Informational Exceptions log page\n");
		return;
	}
	if (full)
		printf("Informational Exceptions log page\n");
	for (k = num; k > 0; k -= extra, ucp += extra) {
		if (k < 3) {
			printf("short Informational Exceptions log page\n");
			return;
		}
		extra = ucp[3] + 4;
		pc = ((ucp[0] << 8) & 0xff) + ucp[1];
		if (0 == pc) {
			if (extra > 5) {
				if (full)
					printf("  IE asc=0x%x, ascq=0x%x",
					       ucp[4], ucp[5]);
				if (extra > 6) {
					if (full)
						printf(",");
					if (ucp[6] < 0xff)
						printf
						    ("  Current temperature=%d C",
						     ucp[6]);
					else
						printf
						    ("  Current temperature=<not available>");
				}
				printf("\n");
			}
		} else if (full) {
			printf("  parameter code=0x%x, contents in hex:\n", pc);
			dStrHex((const char *)ucp, extra, 1);
		}
	}
}

static void show_ascii_page(unsigned char *resp, int len)
{
	int k, n, num;

	if (len < 0) {
		printf("response has bad length\n");
		return;
	}
	num = len - 4;
	switch (resp[0]) {
	case 0:
		printf("Supported pages:\n");
		for (k = 0; k < num; ++k)
			show_page_name((int)resp[4 + k]);
		break;
	case 0x1:
		show_buffer_under_overrun_page(resp, len);
		break;
	case 0x2:
	case 0x3:
	case 0x4:
	case 0x5:
		show_error_counter_page(resp, len);
		break;
	case 0x6:
		show_non_medium_error_page(resp, len);
		break;
	case 0xd:
		show_Temperature_page(resp, len, 1);
		break;
	case 0xe:
		if (len < 40) {
			printf("badly formed start-stop cycle counter page\n");
			break;
		}
		printf("Start-stop cycle counter page\n");
		printf("  Date of manufacture, year: %.4s, week: %.2s\n",
		       &resp[8], &resp[12]);
		printf("  Accounting date, year: %.4s, week: %.2s\n",
		       &resp[18], &resp[22]);
		n = (resp[28] << 24) | (resp[29] << 16) | (resp[30] << 8) |
		    resp[31];
		printf("  Specified cycle count over device lifetime=%d\n", n);
		n = (resp[36] << 24) | (resp[37] << 16) | (resp[38] << 8) |
		    resp[39];
		printf("  Accumulated start-stop cycles=%d\n", n);
		break;
	case 0x10:
		show_self_test_page(resp, len);
		break;
	case 0x2f:
		show_IE_page(resp, len, 1);
		break;
	default:
		printf("No ascii information for page=0x%x, here is hex:\n",
		       resp[0]);
		dStrHex((const char *)resp, len, 1);
		break;
	}
}

static int fetchTemperature(int sg_fd, int do_hex, unsigned char *resp,
			    int max_len)
{
	int res = 0;

	if (0 == do_logs(sg_fd, 0, 0, 1, 0xd, 0, resp, max_len, 0))
		show_Temperature_page(resp, (resp[2] << 8) + resp[3] + 4, 0);
	else if (0 == do_logs(sg_fd, 0, 0, 1, 0x2f, 0, resp, max_len, 0))
		show_IE_page(resp, (resp[2] << 8) + resp[3] + 4, 0);
	else {
		printf
		    ("Unable to find temperature in either log page (temperature "
		     "or IE)\n");
		res = 1;
	}
	close(sg_fd);
	return res;
}

int show_scsi_logs(char *device)
{
	int sg_fd, k, pg_len;
	char *file_name = 0;
	unsigned char rsp_buff[MX_ALLOC_LEN];
	int pg_code = 0;
	int pc = 1;		/* N.B. some disks only give data for current cumulative */
	int paramp = 0;
	int do_list = 0;
	int do_ppc = 0;
	int do_sp = 0;
	int do_hex = 0;
	int do_all = 1;
	int do_temp = 0;
	int oflags = O_RDWR | O_NONBLOCK;

	file_name = device;
	print_msg(TEST_BREAK, __FUNCTION__);

	if ((sg_fd = open(file_name, oflags)) < 0) {
		snprintf(ebuff, EBUFF_SZ, ME "error opening file: %s",
			 file_name);
		perror(ebuff);
		return 1;
	}
	/* Just to be safe, check we have a new sg device by trying an ioctl */
	if ((ioctl(sg_fd, SG_GET_VERSION_NUM, &k) < 0) || (k < 30000)) {
		printf(ME "%s doesn't seem to be a version 3 sg device\n",
		       file_name);
		close(sg_fd);
		return 1;
	}
	if (do_list || do_all)
		pg_code = PG_CODE_ALL;
	pg_len = 0;
	if (1 == do_temp)
		return fetchTemperature(sg_fd, do_hex, rsp_buff, MX_ALLOC_LEN);

	if (0 == do_logs(sg_fd, do_ppc, do_sp, pc, pg_code, paramp,
			 rsp_buff, MX_ALLOC_LEN, 1)) {
		pg_len = (rsp_buff[2] << 8) + rsp_buff[3];
		if ((pg_len + 4) > MX_ALLOC_LEN) {
			printf
			    ("Only fetched %d bytes of response, truncate output\n",
			     MX_ALLOC_LEN);
			pg_len = MX_ALLOC_LEN - 4;
		}
		if (do_hex) {
			printf("Returned log page code=0x%x,  page len=0x%x\n",
			       rsp_buff[0], pg_len);
			dStrHex((const char *)rsp_buff, pg_len + 4, 1);
		} else
			show_ascii_page(rsp_buff, pg_len + 4);
	}
	if (do_all && (pg_len > 1)) {
		int my_len = pg_len - 1;
		unsigned char parr[256];

		memcpy(parr, rsp_buff + 5, my_len);
		for (k = 0; k < my_len; ++k) {
			printf("\n");
			pg_code = parr[k];
			if (0 ==
			    do_logs(sg_fd, do_ppc, do_sp, pc, pg_code, paramp,
				    rsp_buff, MX_ALLOC_LEN, 1)) {
				pg_len = (rsp_buff[2] << 8) + rsp_buff[3];
				if ((pg_len + 4) > MX_ALLOC_LEN) {
					printf
					    ("Only fetched %d bytes of response, truncate "
					     "output\n", MX_ALLOC_LEN);
					pg_len = MX_ALLOC_LEN - 4;
				}
				if (do_hex) {
					printf
					    ("Returned log page code=0x%x,  page len=0x%x\n",
					     rsp_buff[0], pg_len);
					dStrHex((const char *)rsp_buff,
						pg_len + 4, 1);
				} else
					show_ascii_page(rsp_buff, pg_len + 4);
			}
		}
	}
	close(sg_fd);
	return 0;
}

static int do_inquiry(int sg_fd, void *resp, int mx_resp_len)
{
	int res;
	unsigned char inqCmdBlk[INQUIRY_CMDLEN] =
	    { INQUIRY_CMD, 0, 0, 0, 0, 0 };
	unsigned char sense_b[SENSE_BUFF_LEN];
	sg_io_hdr_t io_hdr;

	inqCmdBlk[4] = (unsigned char)mx_resp_len;
	memset(&io_hdr, 0, sizeof(sg_io_hdr_t));
	io_hdr.interface_id = 'S';
	io_hdr.cmd_len = sizeof(inqCmdBlk);
	io_hdr.mx_sb_len = sizeof(sense_b);
	io_hdr.dxfer_direction = SG_DXFER_TO_FROM_DEV;
	io_hdr.dxfer_len = mx_resp_len;
	io_hdr.dxferp = resp;
	io_hdr.cmdp = inqCmdBlk;
	io_hdr.timeout = DEF_TIMEOUT;

	if (ioctl(sg_fd, SG_IO, &io_hdr) < 0) {
		perror("SG_IO (inquiry) error");
		return -1;
	}
	res = sg_err_category3(&io_hdr);
	switch (res) {
	case SG_ERR_CAT_CLEAN:
	case SG_ERR_CAT_RECOVERED:
		return 0;
	default:
		sg_chk_n_print3("Failed INQUIRY", &io_hdr);
		return -1;
	}
}

void leaf_dir(const char *lf, unsigned int *larr)
{
	char name[NAME_LEN_MAX * 2];
	int res;

	if (do_quiet) {
		printf("%u\t%u\t%u\t%u\n", larr[0], larr[1], larr[2], larr[3]);
		return;
	}
	printf("%u\t%u\t%u\t%u\t%s\n", larr[0], larr[1], larr[2], larr[3], lf);
	if (do_leaf) {
		struct dirent *de_entry;
		struct dirent *de_result;
		DIR *sdir;
		int outpos;

		if (NULL == (sdir = opendir(lf))) {
			fprintf(stderr, "leaf_dir: opendir of %s: failed\n",
				lf);
			return;
		}
		de_entry = malloc(sizeof(struct dirent) + NAME_LEN_MAX);
		if (NULL == de_entry)
			return;
		res = 0;
		printf("\t");
		outpos = 8;
		while (1) {
			res = readdir_r(sdir, de_entry, &de_result);
			if (0 != res) {
				fprintf(stderr,
					"leaf_dir: readdir_r of %s: %s\n", lf,
					strerror(res));
				res = -2;
				break;
			}
			if (de_result == NULL)
				break;
			strncpy(name, de_entry->d_name, NAME_LEN_MAX * 2);
			if ((0 == strcmp("..", name))
			    || (0 == strcmp(".", name)))
				continue;
			if (do_extra) {
				struct stat st;
				char devname[NAME_LEN_MAX * 2];

				strncpy(devname, lf, NAME_LEN_MAX * 2);
				strcat(devname, "/");
				strcat(devname, name);
				if (stat(devname, &st) < 0)
					return;
				if (S_ISCHR(st.st_mode)) {
					strcat(name, "(c ");
					sprintf(name + strlen(name), "%d %d)",
						major(st.st_rdev),
						minor(st.st_rdev));
				} else if (S_ISBLK(st.st_mode)) {
					strcat(name, "(b ");
					sprintf(name + strlen(name), "%d %d)",
						major(st.st_rdev),
						minor(st.st_rdev));
				}
			}
			res = strlen(name);
			if ((outpos + res + 2) > 80) {
				printf("\n\t");
				outpos = 8;
			}
			printf("%s  ", name);
			outpos += res + 2;
		}
		printf("\n");
	}
	if (do_inq) {
		int sg_fd;
		char buff[64];

		memset(buff, 0, sizeof(buff));
		strncpy(name, lf, NAME_LEN_MAX * 2);
		strcat(name, "/generic");
		if ((sg_fd = open(name, O_RDONLY)) < 0) {
			if (!checked_sg) {
				checked_sg = 1;
				if ((sg_fd = open("/dev/sg0", O_RDONLY)) >= 0)
					close(sg_fd);	/* try and get sg module loaded */
				sg_fd = open(name, O_RDONLY);
			}
			if (sg_fd < 0) {
				printf("Unable to open sg device: %s, %s\n",
				       name, strerror(errno));
				return;
			}
		}
		if (0 != do_inquiry(sg_fd, buff, 64))
			return;
		close(sg_fd);
		dStrHex(buff, 64, 0);
	}
}

/* Return 0 -> ok, -1 -> opendir() error, -2 -> readdir_r error,
         -3 -> malloc error */
int hbtl_scan(const char *path, int level, unsigned int *larr)
{
	struct dirent *de_entry;
	struct dirent *de_result;
	char new_path[NAME_LEN_MAX * 2];
	DIR *sdir;
	int res;
	size_t level_slen;

	level_slen = strlen(level_arr[level]);
	if (NULL == (sdir = opendir(path))) {
		fprintf(stderr, "hbtl_scan: opendir of %s: failed\n", path);
		return -1;
	}
	de_entry = malloc(sizeof(struct dirent) + NAME_LEN_MAX);
	if (NULL == de_entry)
		return -3;
	res = 0;
	while (1) {
		res = readdir_r(sdir, de_entry, &de_result);
		if (0 != res) {
			fprintf(stderr, "hbtl_scan: readdir_r of %s: %s\n",
				path, strerror(res));
			res = -2;
			break;
		}
		if (de_result == NULL)
			break;
		if (0 ==
		    strncmp(level_arr[level], de_entry->d_name, level_slen)) {
			if (1 !=
			    sscanf(de_entry->d_name + level_slen, "%u",
				   larr + level))
				larr[level] = UINT_MAX;
			strncpy(new_path, path, NAME_LEN_MAX * 2);
			strcat(new_path, "/");
			strcat(new_path, de_entry->d_name);
			if ((level + 1) < LEVELS) {
				res = hbtl_scan(new_path, level + 1, larr);
				if (res < 0)
					break;
			} else
				leaf_dir(new_path, larr);
		}
	}
	free(de_entry);
	closedir(sdir);
	return res;
}

int show_devfs_devices()
{
	int res;
	char ds_root[D_ROOT_SZ];
	char di_root[D_ROOT_SZ];
	unsigned int larr[LEVELS];
	struct stat st;

	print_msg(TEST_BREAK, __FUNCTION__);
	strncpy(ds_root, "/dev", D_ROOT_SZ);

	strncpy(di_root, ds_root, D_ROOT_SZ);

	strcat(di_root, "/.devfsd");

	if (stat(di_root, &st) < 0) {
		printf("Didn't find %s so perhaps devfs is not present,"
		       " attempting to continue ...\n", di_root);
	}

	strncpy(di_root, ds_root, D_ROOT_SZ);
	strcat(ds_root, "/scsi");
	strcat(di_root, "/ide");

	if (!do_ide)
		printf("SCSI scan:\n");

	res = hbtl_scan(ds_root, 0, larr);

	if (res < 0)
		printf("main: scsi hbtl_scan res=%d\n", res);

	do_ide = TRUE;
	do_inq = 0;		/* won't try SCSI INQUIRY on IDE devices */

	if (do_ide) {
		printf("\nIDE scan:\n");
		res = hbtl_scan(di_root, 0, larr);

		if (res < 0)
			printf("main: ide hbtl_scan res=%d\n", res);
	}
	return 0;
}

static void install_handler(int sig_num, void (*sig_handler) (int sig))
{
	struct sigaction sigact;
	sigaction(sig_num, NULL, &sigact);
	if (sigact.sa_handler != SIG_IGN) {
		sigact.sa_handler = sig_handler;
		sigemptyset(&sigact.sa_mask);
		sigact.sa_flags = 0;
		sigaction(sig_num, &sigact, NULL);
	}
}

void print_stats()
{
	if (0 != dd_count)
		fprintf(stderr, "  remaining block count=%d\n", dd_count);
	fprintf(stderr, "%d+%d records in\n", in_full - in_partial, in_partial);
	fprintf(stderr, "%d+%d records out\n", out_full - out_partial,
		out_partial);
}

static void interrupt_handler(int sig)
{
	struct sigaction sigact;

	sigact.sa_handler = SIG_DFL;
	sigemptyset(&sigact.sa_mask);
	sigact.sa_flags = 0;
	sigaction(sig, &sigact, NULL);
	fprintf(stderr, "Interrupted by signal,");
	print_stats();
	kill(getpid(), sig);
}

static void siginfo_handler(int sig)
{
	fprintf(stderr, "Progress report, continuing ...\n");
	print_stats();
}

int dd_filetype(const char *filename)
{
	struct stat st;
	size_t len = strlen(filename);

	if ((1 == len) && ('.' == filename[0]))
		return FT_DEV_NULL;
	if (stat(filename, &st) < 0)
		return FT_OTHER;
	if (S_ISCHR(st.st_mode)) {
		if ((MEM_MAJOR == major(st.st_rdev)) &&
		    (DEV_NULL_MINOR_NUM == minor(st.st_rdev)))
			return FT_DEV_NULL;
		if (RAW_MAJOR == major(st.st_rdev))
			return FT_RAW;
		if (SCSI_GENERIC_MAJOR == major(st.st_rdev))
			return FT_SG;
		if (SCSI_TAPE_MAJOR == major(st.st_rdev))
			return FT_ST;
	} else if (S_ISBLK(st.st_mode))
		return FT_BLOCK;
	return FT_OTHER;
}

int read_capacity(int sg_fd, int *num_sect, int *sect_sz)
{
	int res;
	unsigned char rcCmdBlk[10] =
	    { READ_CAPACITY, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	unsigned char rcBuff[READ_CAP_REPLY_LEN];
	unsigned char sense_b[64];
	sg_io_hdr_t io_hdr;

	memset(&io_hdr, 0, sizeof(sg_io_hdr_t));
	io_hdr.interface_id = 'S';
	io_hdr.cmd_len = sizeof(rcCmdBlk);
	io_hdr.mx_sb_len = sizeof(sense_b);
	io_hdr.dxfer_direction = SG_DXFER_FROM_DEV;
	io_hdr.dxfer_len = sizeof(rcBuff);
	io_hdr.dxferp = rcBuff;
	io_hdr.cmdp = rcCmdBlk;
	io_hdr.sbp = sense_b;
	io_hdr.timeout = DEF_TIMEOUT;

	if (ioctl(sg_fd, SG_IO, &io_hdr) < 0) {
		perror("read_capacity (SG_IO) error");
		return -1;
	}
	res = sg_err_category3(&io_hdr);
	if (SG_ERR_CAT_MEDIA_CHANGED == res)
		return 2;	/* probably have another go ... */
	else if (SG_ERR_CAT_CLEAN != res) {
		sg_chk_n_print3("read capacity", &io_hdr);
		return -1;
	}
	*num_sect = 1 + ((rcBuff[0] << 24) | (rcBuff[1] << 16) |
			 (rcBuff[2] << 8) | rcBuff[3]);
	*sect_sz = (rcBuff[4] << 24) | (rcBuff[5] << 16) |
	    (rcBuff[6] << 8) | rcBuff[7];
	return 0;
}

/* Return of 0 -> success, -1 -> failure, 2 -> try again */
int sync_cache(int sg_fd)
{
	int res;
	unsigned char scCmdBlk[10] = { SYNCHRONIZE_CACHE, 0, 0, 0, 0, 0, 0,
		0, 0, 0
	};
	unsigned char sense_b[64];
	sg_io_hdr_t io_hdr;

	memset(&io_hdr, 0, sizeof(sg_io_hdr_t));
	io_hdr.interface_id = 'S';
	io_hdr.cmd_len = sizeof(scCmdBlk);
	io_hdr.mx_sb_len = sizeof(sense_b);
	io_hdr.dxfer_direction = SG_DXFER_NONE;
	io_hdr.dxfer_len = 0;
	io_hdr.dxferp = NULL;
	io_hdr.cmdp = scCmdBlk;
	io_hdr.sbp = sense_b;
	io_hdr.timeout = DEF_TIMEOUT;

	if (ioctl(sg_fd, SG_IO, &io_hdr) < 0) {
		perror("synchronize_cache (SG_IO) error");
		return -1;
	}
	res = sg_err_category3(&io_hdr);
	if (SG_ERR_CAT_MEDIA_CHANGED == res)
		return 2;	/* probably have another go ... */
	else if (SG_ERR_CAT_CLEAN != res) {
		sg_chk_n_print3("synchronize cache", &io_hdr);
		return -1;
	}
	return 0;
}

int sg_build_scsi_cdb(unsigned char *cdbp, int cdb_sz, unsigned int blocks,
		      unsigned int start_block, int write_true, int fua,
		      int dpo)
{
	int rd_opcode[] = { 0x8, 0x28, 0xa8, 0x88 };
	int wr_opcode[] = { 0xa, 0x2a, 0xaa, 0x8a };
	int sz_ind;

	memset(cdbp, 0, cdb_sz);
	if (dpo)
		cdbp[1] |= 0x10;
	if (fua)
		cdbp[1] |= 0x8;
	switch (cdb_sz) {
	case 6:
		sz_ind = 0;
		cdbp[0] = (unsigned char)(write_true ? wr_opcode[sz_ind] :
					  rd_opcode[sz_ind]);
		cdbp[1] = (unsigned char)((start_block >> 16) & 0x1f);
		cdbp[2] = (unsigned char)((start_block >> 8) & 0xff);
		cdbp[3] = (unsigned char)(start_block & 0xff);
		cdbp[4] = (256 == blocks) ? 0 : (unsigned char)blocks;
		if (blocks > 256) {
			fprintf(stderr,
				ME "for 6 byte commands, maximum number of "
				"blocks is 256\n");
			return 1;
		}
		if ((start_block + blocks - 1) & (~0x1fffff)) {
			fprintf(stderr,
				ME "for 6 byte commands, can't address blocks"
				" beyond %d\n", 0x1fffff);
			return 1;
		}
		if (dpo || fua) {
			fprintf(stderr,
				ME "for 6 byte commands, neither dpo nor fua"
				" bits supported\n");
			return 1;
		}
		break;
	case 10:
		sz_ind = 1;
		cdbp[0] = (unsigned char)(write_true ? wr_opcode[sz_ind] :
					  rd_opcode[sz_ind]);
		cdbp[2] = (unsigned char)((start_block >> 24) & 0xff);
		cdbp[3] = (unsigned char)((start_block >> 16) & 0xff);
		cdbp[4] = (unsigned char)((start_block >> 8) & 0xff);
		cdbp[5] = (unsigned char)(start_block & 0xff);
		cdbp[7] = (unsigned char)((blocks >> 8) & 0xff);
		cdbp[8] = (unsigned char)(blocks & 0xff);
		if (blocks & (~0xffff)) {
			fprintf(stderr,
				ME "for 10 byte commands, maximum number of "
				"blocks is %d\n", 0xffff);
			return 1;
		}
		break;
	case 12:
		sz_ind = 2;
		cdbp[0] = (unsigned char)(write_true ? wr_opcode[sz_ind] :
					  rd_opcode[sz_ind]);
		cdbp[2] = (unsigned char)((start_block >> 24) & 0xff);
		cdbp[3] = (unsigned char)((start_block >> 16) & 0xff);
		cdbp[4] = (unsigned char)((start_block >> 8) & 0xff);
		cdbp[5] = (unsigned char)(start_block & 0xff);
		cdbp[6] = (unsigned char)((blocks >> 24) & 0xff);
		cdbp[7] = (unsigned char)((blocks >> 16) & 0xff);
		cdbp[8] = (unsigned char)((blocks >> 8) & 0xff);
		cdbp[9] = (unsigned char)(blocks & 0xff);
		break;
	case 16:
		sz_ind = 3;
		cdbp[0] = (unsigned char)(write_true ? wr_opcode[sz_ind] :
					  rd_opcode[sz_ind]);
		/* can't cope with block number > 32 bits (yet) */
		cdbp[6] = (unsigned char)((start_block >> 24) & 0xff);
		cdbp[7] = (unsigned char)((start_block >> 16) & 0xff);
		cdbp[8] = (unsigned char)((start_block >> 8) & 0xff);
		cdbp[9] = (unsigned char)(start_block & 0xff);
		cdbp[10] = (unsigned char)((blocks >> 24) & 0xff);
		cdbp[11] = (unsigned char)((blocks >> 16) & 0xff);
		cdbp[12] = (unsigned char)((blocks >> 8) & 0xff);
		cdbp[13] = (unsigned char)(blocks & 0xff);
		break;
	default:
		fprintf(stderr,
			ME "expected cdb size of 6, 10, 12, or 16 but got"
			"=%d\n", cdb_sz);
		return 1;
	}
	return 0;
}

/* -1 -> unrecoverable error, 0 -> successful, 1 -> recoverable (ENOMEM),
   2 -> try again */
int sg_read(int sg_fd, unsigned char *buff, int blocks, int from_block,
	    int bs, int cdbsz, int fua, int *diop)
{
	unsigned char rdCmd[MAX_SCSI_CDBSZ];
	unsigned char senseBuff[SENSE_BUFF_LEN];
	sg_io_hdr_t io_hdr;

	if (sg_build_scsi_cdb(rdCmd, cdbsz, blocks, from_block, 0, fua, 0)) {
		fprintf(stderr,
			ME "bad rd cdb build, from_block=%d, blocks=%d\n",
			from_block, blocks);
		return -1;
	}

	memset(&io_hdr, 0, sizeof(sg_io_hdr_t));
	io_hdr.interface_id = 'S';
	io_hdr.cmd_len = cdbsz;
	io_hdr.cmdp = rdCmd;
	io_hdr.dxfer_direction = SG_DXFER_FROM_DEV;
	io_hdr.dxfer_len = bs * blocks;
	io_hdr.dxferp = buff;
	io_hdr.mx_sb_len = SENSE_BUFF_LEN;
	io_hdr.sbp = senseBuff;
	io_hdr.timeout = DEF_TIMEOUT;
	io_hdr.pack_id = from_block;
	if (diop && *diop)
		io_hdr.flags |= SG_FLAG_DIRECT_IO;

	if (ioctl(sg_fd, SG_IO, &io_hdr)) {
		if (ENOMEM == errno)
			return 1;
		perror("reading (SG_IO) on sg device, error");
		return -1;
	}
	switch (sg_err_category3(&io_hdr)) {
	case SG_ERR_CAT_CLEAN:
		break;
	case SG_ERR_CAT_RECOVERED:
		fprintf(stderr,
			"Recovered error while reading block=%d, num=%d\n",
			from_block, blocks);
		break;
	case SG_ERR_CAT_MEDIA_CHANGED:
		return 2;
	default:
		sg_chk_n_print3("reading", &io_hdr);
		if (do_coe) {
			memset(buff, 0, bs * blocks);
			fprintf(stderr, ">> unable to read at blk=%d for "
				"%d bytes, use zeros\n", from_block,
				bs * blocks);
			return 0;	/* fudge success */
		} else
			return -1;
	}
	if (diop && *diop &&
	    ((io_hdr.info & SG_INFO_DIRECT_IO_MASK) != SG_INFO_DIRECT_IO))
		*diop = 0;	/* flag that dio not done (completely) */
	sum_of_resids += io_hdr.resid;
#if SG_DEBUG
	fprintf(stderr, "duration=%u ms\n", io_hdr.duration);
#endif
	return 0;
}

/* -1 -> unrecoverable error, 0 -> successful, 1 -> recoverable (ENOMEM),
   2 -> try again */
int sg_write(int sg_fd, unsigned char *buff, int blocks, int to_block,
	     int bs, int cdbsz, int fua, int *diop)
{
	unsigned char wrCmd[MAX_SCSI_CDBSZ];
	unsigned char senseBuff[SENSE_BUFF_LEN];
	sg_io_hdr_t io_hdr;

	if (sg_build_scsi_cdb(wrCmd, cdbsz, blocks, to_block, 1, fua, 0)) {
		fprintf(stderr, ME "bad wr cdb build, to_block=%d, blocks=%d\n",
			to_block, blocks);
		return -1;
	}

	memset(&io_hdr, 0, sizeof(sg_io_hdr_t));
	io_hdr.interface_id = 'S';
	io_hdr.cmd_len = cdbsz;
	io_hdr.cmdp = wrCmd;
	io_hdr.dxfer_direction = SG_DXFER_TO_DEV;
	io_hdr.dxfer_len = bs * blocks;
	io_hdr.dxferp = buff;
	io_hdr.mx_sb_len = SENSE_BUFF_LEN;
	io_hdr.sbp = senseBuff;
	io_hdr.timeout = DEF_TIMEOUT;
	io_hdr.pack_id = to_block;
	if (diop && *diop)
		io_hdr.flags |= SG_FLAG_DIRECT_IO;

	if (ioctl(sg_fd, SG_IO, &io_hdr)) {
		if (ENOMEM == errno)
			return 1;
		perror("writing (SG_IO) on sg device, error");
		return -1;
	}
	switch (sg_err_category3(&io_hdr)) {
	case SG_ERR_CAT_CLEAN:
		break;
	case SG_ERR_CAT_RECOVERED:
		fprintf(stderr,
			"Recovered error while writing block=%d, num=%d\n",
			to_block, blocks);
		break;
	case SG_ERR_CAT_MEDIA_CHANGED:
		return 2;
	default:
		sg_chk_n_print3("writing", &io_hdr);
		if (do_coe) {
			fprintf(stderr, ">> ignored errors for out blk=%d for "
				"%d bytes\n", to_block, bs * blocks);
			return 0;	/* fudge success */
		} else
			return -1;
	}
	if (diop && *diop &&
	    ((io_hdr.info & SG_INFO_DIRECT_IO_MASK) != SG_INFO_DIRECT_IO))
		*diop = 0;	/* flag that dio not done (completely) */
	return 0;
}

int get_num(char *buf)
{
	int res, num;
	char c;

	res = sscanf(buf, "%d%c", &num, &c);
	if (0 == res)
		return -1;
	else if (1 == res)
		return num;
	else {
		switch (c) {
		case 'c':
		case 'C':
			return num;
		case 'b':
		case 'B':
			return num * 512;
		case 'k':
			return num * 1024;
		case 'K':
			return num * 1000;
		case 'm':
			return num * 1024 * 1024;
		case 'M':
			return num * 1000000;
		case 'g':
			return num * 1024 * 1024 * 1024;
		case 'G':
			return num * 1000000000;
		default:
			fprintf(stderr, "unrecognized multiplier\n");
			return -1;
		}
	}
}

int do_scsi_device_read_write(char *device)
{
	int skip = 0;
	int seek = 0;
	int bs = 0;
	int ibs = 0;
	int obs = 0;
	int bpt = DEF_BLOCKS_PER_TRANSFER;
	char inf[INOUTF_SZ];
	int in_type = FT_OTHER;
	char outf[INOUTF_SZ];
	int out_type = FT_OTHER;
	int dio = 0;
	int dio_incomplete = 0;
	int do_time = 1;
	int do_odir = 1;
	int scsi_cdbsz = DEF_SCSI_CDBSZ;
	int fua_mode = 0;
	int do_sync = 1;
	int do_blk_sgio = 1;
	int do_append = 1;
	int res, t, buf_sz, dio_tmp;
	int infd, outfd, blocks;
	unsigned char *wrkBuff;
	unsigned char *wrkPos;
	int in_num_sect = 0;
	int out_num_sect = 0;
	int in_sect_sz, out_sect_sz;
	char ebuff[EBUFF_SZ];
	int blocks_per;
	int req_count;
	struct timeval start_tm, end_tm;

	print_msg(TEST_BREAK, __FUNCTION__);
	strcpy(inf, "/dev/zero");
	strcpy(outf, device);

	if (bs <= 0) {
		bs = DEF_BLOCK_SIZE;
		fprintf(stderr,
			"Assume default 'bs' (block size) of %d bytes\n", bs);
	}
	if ((ibs && (ibs != bs)) || (obs && (obs != bs))) {
		fprintf(stderr,
			"If 'ibs' or 'obs' given must be same as 'bs'\n");
		usage();
		return 1;
	}
	if ((skip < 0) || (seek < 0)) {
		fprintf(stderr, "skip and seek cannot be negative\n");
		return 1;
	}
	if ((do_append > 0) && (seek > 0)) {
		fprintf(stderr, "Can't use both append and seek switches\n");
		return 1;
	}
#ifdef SG_DEBUG
	fprintf(stderr, ME "if=%s skip=%d of=%s seek=%d count=%d\n",
		inf, skip, outf, seek, dd_count);
#endif
	install_handler(SIGINT, interrupt_handler);
	install_handler(SIGQUIT, interrupt_handler);
	install_handler(SIGPIPE, interrupt_handler);
	install_handler(SIGUSR1, siginfo_handler);

	infd = STDIN_FILENO;
	outfd = STDOUT_FILENO;
	if (inf[0] && ('-' != inf[0])) {
		in_type = dd_filetype(inf);

		if ((FT_BLOCK & in_type) && do_blk_sgio)
			in_type |= FT_SG;

		if (FT_ST == in_type) {
			fprintf(stderr,
				ME "unable to use scsi tape device %s\n", inf);
			return 1;
		} else if (FT_SG & in_type) {
			if ((infd = open(inf, O_RDWR)) < 0) {
				snprintf(ebuff, EBUFF_SZ,
					 ME "could not open %s for sg reading",
					 inf);
				perror(ebuff);
				return 1;
			}
			t = bs * bpt;
			res = ioctl(infd, SG_SET_RESERVED_SIZE, &t);
			if (res < 0)
				perror(ME "SG_SET_RESERVED_SIZE error");
			res = ioctl(infd, SG_GET_VERSION_NUM, &t);
			if ((res < 0) || (t < 30000)) {
				if (FT_BLOCK & in_type)
					fprintf(stderr,
						ME
						"SG_IO unsupported on this block"
						" device\n");
				else
					fprintf(stderr,
						ME
						"sg driver prior to 3.x.y\n");
				return 1;
			}
		} else {
			if (do_odir && (FT_BLOCK == in_type))
				infd = open(inf, O_RDONLY | O_DIRECT);
			else
				infd = open(inf, O_RDONLY);
			if (infd < 0) {
				snprintf(ebuff, EBUFF_SZ,
					 ME "could not open %s for reading",
					 inf);
				perror(ebuff);
				return 1;
			} else if (skip > 0) {
				llse_loff_t offset = skip;

				offset *= bs;	/* could exceed 32 bits here! */
				if (llse_llseek(infd, offset, SEEK_SET) < 0) {
					snprintf(ebuff, EBUFF_SZ,
						 ME
						 "couldn't skip to required position on %s",
						 inf);
					perror(ebuff);
					return 1;
				}
			}
		}
	}

	if (outf[0] && ('-' != outf[0])) {
		out_type = dd_filetype(outf);

		if ((FT_BLOCK & out_type) && do_blk_sgio)
			out_type |= FT_SG;

		if (FT_ST == out_type) {
			fprintf(stderr,
				ME "unable to use scsi tape device %s\n", outf);
			return 1;
		} else if (FT_SG & out_type) {
			if ((outfd = open(outf, O_RDWR)) < 0) {
				snprintf(ebuff, EBUFF_SZ,
					 ME "could not open %s for sg writing",
					 outf);
				perror(ebuff);
				return 1;
			}
			t = bs * bpt;
			res = ioctl(outfd, SG_SET_RESERVED_SIZE, &t);
			if (res < 0)
				perror(ME "SG_SET_RESERVED_SIZE error");
			res = ioctl(outfd, SG_GET_VERSION_NUM, &t);
			if ((res < 0) || (t < 30000)) {
				fprintf(stderr,
					ME "sg driver prior to 3.x.y\n");
				return 1;
			}
		} else if (FT_DEV_NULL & out_type)
			outfd = -1;	/* don't bother opening */
		else {
			if (FT_RAW != out_type) {
				int flags = O_WRONLY | O_CREAT;

				if (do_odir && (FT_BLOCK == out_type))
					flags |= O_DIRECT;
				else if (do_append)
					flags |= O_APPEND;
				if ((outfd = open(outf, flags, 0666)) < 0) {
					snprintf(ebuff, EBUFF_SZ,
						 ME
						 "could not open %s for writing",
						 outf);
					perror(ebuff);
					return 1;
				}
			} else {
				if ((outfd = open(outf, O_WRONLY)) < 0) {
					snprintf(ebuff, EBUFF_SZ,
						 ME
						 "could not open %s for raw writing",
						 outf);
					perror(ebuff);
					return 1;
				}
			}
			if (seek > 0) {
				llse_loff_t offset = seek;

				offset *= bs;	/* could exceed 32 bits here! */
				if (llse_llseek(outfd, offset, SEEK_SET) < 0) {
					snprintf(ebuff, EBUFF_SZ,
						 ME
						 "couldn't seek to required position on %s",
						 outf);
					perror(ebuff);
					return 1;
				}
			}
		}
	}
	if ((STDIN_FILENO == infd) && (STDOUT_FILENO == outfd)) {
		fprintf(stderr,
			"Can't have both 'if' as stdin _and_ 'of' as stdout\n");
		return 1;
	}

	if (dd_count < 0) {
		if (FT_SG & in_type) {
			res = read_capacity(infd, &in_num_sect, &in_sect_sz);
			if (2 == res) {
				fprintf(stderr,
					"Unit attention, media changed(in), continuing\n");
				res =
				    read_capacity(infd, &in_num_sect,
						  &in_sect_sz);
			}
			if (0 != res) {
				fprintf(stderr,
					"Unable to read capacity on %s\n", inf);
				in_num_sect = -1;
			} else {
				if (in_num_sect > skip)
					in_num_sect -= skip;
			}
		}
		if (FT_SG & out_type) {
			res = read_capacity(outfd, &out_num_sect, &out_sect_sz);
			if (2 == res) {
				fprintf(stderr,
					"Unit attention, media changed(out), continuing\n");
				res =
				    read_capacity(outfd, &out_num_sect,
						  &out_sect_sz);
			}
			if (0 != res) {
				fprintf(stderr,
					"Unable to read capacity on %s\n",
					outf);
				out_num_sect = -1;
			} else {
				if (out_num_sect > seek)
					out_num_sect -= seek;
			}
		}
#ifdef SG_DEBUG
		fprintf(stderr,
			"Start of loop, count=%d, in_num_sect=%d, out_num_sect=%d\n",
			dd_count, in_num_sect, out_num_sect);
#endif
		if (in_num_sect > 0) {
			if (out_num_sect > 0)
				dd_count =
				    (in_num_sect >
				     out_num_sect) ? out_num_sect : in_num_sect;
			else
				dd_count = in_num_sect;
		} else
			dd_count = out_num_sect;
	}
	if (dd_count < 0) {
		fprintf(stderr, "Couldn't calculate count, please give one\n");
		return 1;
	}

	if (dio || do_odir || (FT_RAW == in_type) || (FT_RAW == out_type)) {
		size_t psz = getpagesize();
		wrkBuff = malloc(bs * bpt + psz);
		if (0 == wrkBuff) {
			fprintf(stderr, "Not enough user memory for raw\n");
			return 1;
		}
		wrkPos = (unsigned char *)(((unsigned long)wrkBuff + psz - 1) &
					   (~(psz - 1)));
	} else {
		wrkBuff = malloc(bs * bpt);
		if (0 == wrkBuff) {
			fprintf(stderr, "Not enough user memory\n");
			return 1;
		}
		wrkPos = wrkBuff;
	}

	blocks_per = bpt;
#ifdef SG_DEBUG
	fprintf(stderr, "Start of loop, count=%d, blocks_per=%d\n",
		dd_count, blocks_per);
#endif
	if (do_time) {
		start_tm.tv_sec = 0;
		start_tm.tv_usec = 0;
		gettimeofday(&start_tm, NULL);
	}
	req_count = dd_count;

	while (dd_count > 0) {
		blocks = (dd_count > blocks_per) ? blocks_per : dd_count;
		if (FT_SG & in_type) {
			int fua = fua_mode & 2;

			dio_tmp = dio;
			res =
			    sg_read(infd, wrkPos, blocks, skip, bs, scsi_cdbsz,
				    fua, &dio_tmp);
			if (1 == res) {	/* ENOMEM, find what's available+try that */
				if (ioctl(infd, SG_GET_RESERVED_SIZE, &buf_sz) <
				    0) {
					perror("RESERVED_SIZE ioctls failed");
					break;
				}
				blocks_per = (buf_sz + bs - 1) / bs;
				blocks = blocks_per;
				fprintf(stderr,
					"Reducing read to %d blocks per loop\n",
					blocks_per);
				res =
				    sg_read(infd, wrkPos, blocks, skip, bs,
					    scsi_cdbsz, fua, &dio_tmp);
			} else if (2 == res) {
				fprintf(stderr,
					"Unit attention, media changed, continuing (r)\n");
				res =
				    sg_read(infd, wrkPos, blocks, skip, bs,
					    scsi_cdbsz, fua, &dio_tmp);
			}
			if (0 != res) {
				fprintf(stderr, "sg_read failed, skip=%d\n",
					skip);
				break;
			} else {
				in_full += blocks;
				if (dio && (0 == dio_tmp))
					dio_incomplete++;
			}
		} else {
			while (((res = read(infd, wrkPos, blocks * bs)) < 0) &&
			       (EINTR == errno)) ;
			if (res < 0) {
				snprintf(ebuff, EBUFF_SZ,
					 ME "reading, skip=%d ", skip);
				perror(ebuff);
				break;
			} else if (res < blocks * bs) {
				dd_count = 0;
				blocks = res / bs;
				if ((res % bs) > 0) {
					blocks++;
					in_partial++;
				}
			}
			in_full += blocks;
		}

		if (FT_SG & out_type) {
			int fua = fua_mode & 1;

			dio_tmp = dio;
			res =
			    sg_write(outfd, wrkPos, blocks, seek, bs,
				     scsi_cdbsz, fua, &dio_tmp);
			if (1 == res) {	/* ENOMEM, find what's available+try that */
				if (ioctl(outfd, SG_GET_RESERVED_SIZE, &buf_sz)
				    < 0) {
					perror("RESERVED_SIZE ioctls failed");
					break;
				}
				blocks_per = (buf_sz + bs - 1) / bs;
				blocks = blocks_per;
				fprintf(stderr,
					"Reducing write to %d blocks per loop\n",
					blocks);
				res =
				    sg_write(outfd, wrkPos, blocks, seek, bs,
					     scsi_cdbsz, fua, &dio_tmp);
			} else if (2 == res) {
				fprintf(stderr,
					"Unit attention, media changed, continuing (w)\n");
				res =
				    sg_write(outfd, wrkPos, blocks, seek, bs,
					     scsi_cdbsz, fua, &dio_tmp);
			} else if (0 != res) {
				fprintf(stderr, "sg_write failed, seek=%d\n",
					seek);
				break;
			} else {
				out_full += blocks;
				if (dio && (0 == dio_tmp))
					dio_incomplete++;
			}
		} else if (FT_DEV_NULL & out_type)
			out_full += blocks;	/* act as if written out without error */
		else {
			while (((res = write(outfd, wrkPos, blocks * bs)) < 0)
			       && (EINTR == errno)) ;
			if (res < 0) {
				snprintf(ebuff, EBUFF_SZ,
					 ME "writing, seek=%d ", seek);
				perror(ebuff);
				break;
			} else if (res < blocks * bs) {
				fprintf(stderr,
					"output file probably full, seek=%d ",
					seek);
				blocks = res / bs;
				out_full += blocks;
				if ((res % bs) > 0)
					out_partial++;
				break;
			} else
				out_full += blocks;
		}
		if (dd_count > 0)
			dd_count -= blocks;
		skip += blocks;
		seek += blocks;
	}
	if ((do_time) && (start_tm.tv_sec || start_tm.tv_usec)) {
		struct timeval res_tm;
		double a, b;

		gettimeofday(&end_tm, NULL);
		res_tm.tv_sec = end_tm.tv_sec - start_tm.tv_sec;
		res_tm.tv_usec = end_tm.tv_usec - start_tm.tv_usec;
		if (res_tm.tv_usec < 0) {
			--res_tm.tv_sec;
			res_tm.tv_usec += 1000000;
		}
		a = res_tm.tv_sec;
		a += (0.000001 * res_tm.tv_usec);
		b = (double)bs *(req_count - dd_count);
		printf("time to transfer data was %d.%06d secs",
		       (int)res_tm.tv_sec, (int)res_tm.tv_usec);
		if ((a > 0.00001) && (b > 511))
			printf(", %.2f MB/sec\n", b / (a * 1000000.0));
		else
			printf("\n");
	}
	if (do_sync) {
		if (FT_SG & out_type) {
			fprintf(stderr, ">> Synchronizing cache on %s\n", outf);
			res = sync_cache(outfd);
			if (2 == res) {
				fprintf(stderr,
					"Unit attention, media changed(in), continuing\n");
				res = sync_cache(outfd);
			}
			if (0 != res)
				fprintf(stderr,
					"Unable to synchronize cache\n");
		}
	}
	free(wrkBuff);
	if (STDIN_FILENO != infd)
		close(infd);
	if ((STDOUT_FILENO != outfd) && (FT_DEV_NULL != out_type))
		close(outfd);
	res = 0;
	if (0 != dd_count) {
		fprintf(stderr, "Some error occurred,");
		res = 2;
	}
	print_stats();
	if (dio_incomplete) {
		int fd;
		char c;

		fprintf(stderr,
			">> Direct IO requested but incomplete %d times\n",
			dio_incomplete);
		if ((fd = open(proc_allow_dio, O_RDONLY)) >= 0) {
			if (1 == read(fd, &c, 1)) {
				if ('0' == c)
					fprintf(stderr,
						">>> %s set to '0' but should be set "
						"to '1' for direct IO\n",
						proc_allow_dio);
			}
			close(fd);
		}
	}
	if (sum_of_resids)
		fprintf(stderr, ">> Non-zero sum of residual counts=%d\n",
			sum_of_resids);
	return res;
}

/* Returns 0 when successful, else -1 */
static int do_scsi_inq(int sg_fd, int cmddt, int evpd, unsigned int pg_op,
		       void *resp, int mx_resp_len, int noisy)
{
	int res;
	unsigned char inqCmdBlk[INQUIRY_CMDLEN] =
	    { INQUIRY_CMD, 0, 0, 0, 0, 0 };
	unsigned char sense_b[SENSE_BUFF_LEN];
	sg_io_hdr_t io_hdr;

	if (cmddt)
		inqCmdBlk[1] |= 2;
	if (evpd)
		inqCmdBlk[1] |= 1;
	inqCmdBlk[2] = (unsigned char)pg_op;
	inqCmdBlk[4] = (unsigned char)mx_resp_len;
	memset(&io_hdr, 0, sizeof(sg_io_hdr_t));
	io_hdr.interface_id = 'S';
	io_hdr.cmd_len = sizeof(inqCmdBlk);
	io_hdr.mx_sb_len = sizeof(sense_b);
	io_hdr.dxfer_direction = SG_DXFER_FROM_DEV;
	io_hdr.dxfer_len = mx_resp_len;
	io_hdr.dxferp = resp;
	io_hdr.cmdp = inqCmdBlk;
	io_hdr.sbp = sense_b;
	io_hdr.timeout = DEF_TIMEOUT;

	if (ioctl(sg_fd, SG_IO, &io_hdr) < 0) {
		perror("SG_IO (inquiry) error");
		return -1;
	}
	res = sg_err_category3(&io_hdr);
	switch (res) {
	case SG_ERR_CAT_CLEAN:
	case SG_ERR_CAT_RECOVERED:
		return 0;
	default:
		if (noisy) {
			char ebuff[EBUFF_SZ];
			snprintf(ebuff, EBUFF_SZ, "Inquiry error, CmdDt=%d, "
				 "EVPD=%d, page_opcode=%x ", cmddt, evpd,
				 pg_op);
			sg_chk_n_print3(ebuff, &io_hdr);
		}
		return -1;
	}
}

int do_scsi_inquiry(char *device, int hex_flag)
{
	int sg_fd, k, j, num, len, act_len;
	int support_num;
	char *file_name = 0;
	char buff[MX_ALLOC_LEN + 1];
	unsigned char rsp_buff[MX_ALLOC_LEN + 1];
	unsigned int num_opcode = 0;
	int do_evpd = 0;
	int do_cmddt = 0;
	int do_cmdlst = 0;
	int do_hex = 0;
	int do_raw = 0;
	int do_pci = 0;
	int do_36 = 0;
	int oflags = O_RDONLY | O_NONBLOCK;
	int ansi_version = 0;
	int ret = 0;

	file_name = device;

	if (hex_flag) {
		do_hex = TRUE;
		print_msg(TEST_BREAK, __FUNCTION__);
	} else {
		do_pci = TRUE;
	}

	if (do_pci)
		oflags = O_RDWR | O_NONBLOCK;
	if ((sg_fd = open(file_name, oflags)) < 0) {
		snprintf(ebuff, EBUFF_SZ, "sg_inq: error opening file: %s",
			 file_name);
		perror(ebuff);
		return 1;
	}
	/* Just to be safe, check we have a new sg device by trying an ioctl */
	if ((ioctl(sg_fd, SG_GET_VERSION_NUM, &k) < 0) || (k < 30000)) {
		fprintf(stderr,
			"sg_inq: %s doesn't seem to be a version 3 sg device\n",
			file_name);
		close(sg_fd);
		return 1;
	}
	memset(rsp_buff, 0, MX_ALLOC_LEN + 1);

	if (!(do_cmddt || do_evpd)) {
		if (!do_raw)
			printf("standard INQUIRY:\n");
		if (num_opcode > 0)
			printf
			    (" <<given opcode or page_code is being ignored>>\n");

		if (0 == do_scsi_inq(sg_fd, 0, 0, 0, rsp_buff, 36, 1)) {
			len = rsp_buff[4] + 5;
			ansi_version = rsp_buff[2] & 0x7;
			if ((len > 36) && (len < 256) && (!do_36)) {
				if (do_scsi_inq
				    (sg_fd, 0, 0, 0, rsp_buff, len, 1)) {
					fprintf(stderr,
						"second INQUIRY (%d byte) failed\n",
						len);
					return 1;
				}
				if (len != (rsp_buff[4] + 5)) {
					fprintf(stderr,
						"strange, twin INQUIRYs yield different "
						"'additional length'\n");
					ret = 2;
				}
			}
			if (do_36) {
				act_len = len;
				len = 36;
			} else
				act_len = len;
			if (do_hex)
				dStrHex((const char *)rsp_buff, len, 0);
			else {
				printf
				    ("  PQual=%d, Device type=%d, RMB=%d, ANSI version=%d, ",
				     (rsp_buff[0] & 0xe0) >> 5,
				     rsp_buff[0] & 0x1f,
				     ! !(rsp_buff[1] & 0x80), ansi_version);
				printf("[full version=0x%02x]\n",
				       (unsigned int)rsp_buff[2]);
				printf
				    ("  AERC=%d, TrmTsk=%d, NormACA=%d, HiSUP=%d, "
				     "Resp data format=%d, SCCS=%d\n",
				     ! !(rsp_buff[3] & 0x80),
				     ! !(rsp_buff[3] & 0x40),
				     ! !(rsp_buff[3] & 0x20),
				     ! !(rsp_buff[3] & 0x10),
				     rsp_buff[3] & 0x0f,
				     ! !(rsp_buff[5] & 0x80));
				printf
				    ("  BQue=%d, EncServ=%d, MultiP=%d, MChngr=%d, "
				     "ACKREQQ=%d, ", ! !(rsp_buff[6] & 0x80),
				     ! !(rsp_buff[6] & 0x40),
				     ! !(rsp_buff[6] & 0x10),
				     ! !(rsp_buff[6] & 0x08),
				     ! !(rsp_buff[6] & 0x04));
				printf("Addr16=%d\n  RelAdr=%d, ",
				       ! !(rsp_buff[6] & 0x01),
				       ! !(rsp_buff[7] & 0x80));
				printf
				    ("WBus16=%d, Sync=%d, Linked=%d, TranDis=%d, ",
				     ! !(rsp_buff[7] & 0x20),
				     ! !(rsp_buff[7] & 0x10),
				     ! !(rsp_buff[7] & 0x08),
				     ! !(rsp_buff[7] & 0x04));
				printf("CmdQue=%d\n", ! !(rsp_buff[7] & 0x02));
				if (len > 56)
					printf
					    ("  Clocking=0x%x, QAS=%d, IUS=%d\n",
					     (rsp_buff[56] & 0x0c) >> 2,
					     ! !(rsp_buff[56] & 0x2),
					     ! !(rsp_buff[56] & 0x1));
				if (act_len == len)
					printf("    length=%d (0x%x)", len,
					       len);
				else
					printf
					    ("    length=%d (0x%x), but only read 36 bytes",
					     len, len);
				if ((ansi_version >= 2) && (len < 36))
					printf
					    ("  [for SCSI>=2, len>=36 is expected]\n");
				else
					printf("\n");

				if (len <= 8)
					printf
					    (" Inquiry response length=%d\n, no vendor, "
					     "product or revision data\n", len);
				else {
					if (len < 36)
						rsp_buff[len] = '\0';
					memcpy(buff, &rsp_buff[8], 8);
					buff[8] = '\0';
					printf(" Vendor identification: %s\n",
					       buff);
					if (len <= 16)
						printf
						    (" Product identification: <none>\n");
					else {
						memcpy(buff, &rsp_buff[16], 16);
						buff[16] = '\0';
						printf
						    (" Product identification: %s\n",
						     buff);
					}
					if (len <= 32)
						printf
						    (" Product revision level: <none>\n");
					else {
						memcpy(buff, &rsp_buff[32], 4);
						buff[4] = '\0';
						printf
						    (" Product revision level: %s\n",
						     buff);
					}
				}
			}
			if (!do_raw &&
			    (0 ==
			     do_scsi_inq(sg_fd, 0, 1, 0x80, rsp_buff,
					 MX_ALLOC_LEN, 0))) {
				len = rsp_buff[3];
				if (len > 0) {
					memcpy(buff, rsp_buff + 4, len);
					buff[len] = '\0';
					printf(" Product serial number: %s\n",
					       buff);
				}
			}
		} else {
			printf("36 byte INQUIRY failed\n");
			return 1;
		}
	} else if (do_cmddt) {
		int reserved_cmddt;
		char op_name[128];

		if (do_cmdlst) {
			printf("Supported command list:\n");
			for (k = 0; k < 256; ++k) {
				if (0 ==
				    do_scsi_inq(sg_fd, 1, 0, k, rsp_buff,
						MX_ALLOC_LEN, 1)) {
					support_num = rsp_buff[1] & 7;
					reserved_cmddt = rsp_buff[4];
					if ((3 == support_num)
					    || (5 == support_num)) {
						num = rsp_buff[5];
						for (j = 0; j < num; ++j)
							printf(" %.2x",
							       (int)rsp_buff[6 +
									     j]);
						if (5 == support_num)
							printf
							    ("  [vendor specific manner (5)]");
						sg_get_command_name((unsigned
								     char)k,
								    sizeof
								    (op_name) -
								    1, op_name);
						op_name[sizeof(op_name) - 1] =
						    '\0';
						printf("  %s\n", op_name);
					} else if ((4 == support_num)
						   || (6 == support_num))
						printf
						    ("  opcode=0x%.2x vendor specific (%d)\n",
						     k, support_num);
					else if ((0 == support_num)
						 && (reserved_cmddt > 0)) {
						printf
						    ("  opcode=0x%.2x ignored cmddt bit, "
						     "given standard INQUIRY response, stop\n",
						     k);
						break;
					}
				} else {
					fprintf(stderr,
						"CmdDt INQUIRY on opcode=0x%.2x: failed\n",
						k);
					break;
				}
			}
		} else {
			if (!do_raw) {
				printf("CmdDt INQUIRY, opcode=0x%.2x:  [",
				       num_opcode);
				sg_get_command_name((unsigned char)num_opcode,
						    sizeof(op_name) - 1,
						    op_name);
				op_name[sizeof(op_name) - 1] = '\0';
				printf("%s]\n", op_name);
			}
			if (0 == do_scsi_inq(sg_fd, 1, 0, num_opcode, rsp_buff,
					     MX_ALLOC_LEN, 1)) {
				len = rsp_buff[5] + 6;
				reserved_cmddt = rsp_buff[4];
				if (do_hex)
					dStrHex((const char *)rsp_buff, len, 0);
				else {
					const char *desc_p;
					int prnt_cmd = 0;

					support_num = rsp_buff[1] & 7;
					num = rsp_buff[5];
					switch (support_num) {
					case 0:
						if (0 == reserved_cmddt)
							desc_p =
							    "no data available";
						else
							desc_p =
							    "ignored cmddt bit, standard INQUIRY "
							    "response";
						break;
					case 1:
						desc_p = "not supported";
						break;
					case 2:
						desc_p = "reserved (2)";
						break;
					case 3:
						desc_p =
						    "supported as per standard";
						prnt_cmd = 1;
						break;
					case 4:
						desc_p = "vendor specific (4)";
						break;
					case 5:
						desc_p =
						    "supported in vendor specific way";
						prnt_cmd = 1;
						break;
					case 6:
						desc_p = "vendor specific (6)";
						break;
					case 7:
						desc_p = "reserved (7)";
						break;
					default:
						desc_p = "impossible value > 7";
						break;
					}
					if (prnt_cmd) {
						printf("  Support field: %s [",
						       desc_p);
						for (j = 0; j < num; ++j)
							printf(" %.2x",
							       (int)rsp_buff[6 +
									     j]);
						printf(" ]\n");
					} else
						printf("  Support field: %s\n",
						       desc_p);
				}
			} else {
				fprintf(stderr,
					"CmdDt INQUIRY on opcode=0x%.2x: failed\n",
					num_opcode);
				return 1;
			}

		}
	} else if (do_evpd) {
		if (!do_raw)
			printf("EVPD INQUIRY, page code=0x%.2x:\n", num_opcode);
		if (0 ==
		    do_scsi_inq(sg_fd, 0, 1, num_opcode, rsp_buff, MX_ALLOC_LEN,
				1)) {
			len = rsp_buff[3] + 4;
			if (num_opcode != rsp_buff[1])
				printf
				    ("non evpd respone; probably a STANDARD INQUIRY "
				     "response\n");
			else {
				if (!do_hex)
					printf(" Only hex output supported\n");
				dStrHex((const char *)rsp_buff, len, 0);
			}
		} else {
			fprintf(stderr,
				"EVPD INQUIRY, page code=0x%.2x: failed\n",
				num_opcode);
			return 1;
		}
	}

	if (do_pci) {
		unsigned char slot_name[16];

		printf("\n");
		memset(slot_name, '\0', sizeof(slot_name));
		if (ioctl(sg_fd, SCSI_IOCTL_GET_PCI, slot_name) < 0) {
			if (EINVAL == errno)
				printf
				    ("ioctl(SCSI_IOCTL_GET_PCI) not supported by this "
				     "kernel\n");
			else if (ENXIO == errno)
				printf
				    ("associated adapter not a PCI device?\n");
			else
				perror("ioctl(SCSI_IOCTL_GET_PCI) failed");
		} else
			printf("PCI:slot_name: %s\n", slot_name);
	}

	close(sg_fd);
	return ret;
}

int show_scsi_maps()
{
	int sg_fd, res, k;
	int do_numeric = NUMERIC_SCAN_DEF;
	int do_all_s = 1;
	int do_sd = 0;
	int do_st = 0;
	int do_osst = 0;
	int do_sr = 0;
	int do_scd = 0;
	int do_extra = 1;
	int do_inquiry = 0;
	char fname[64];
	int num_errors = 0;
	int num_silent = 0;
	int eacces_err = 0;
	int last_sg_ind = -1;
	struct stat stat_buf;

	print_msg(TEST_BREAK, __FUNCTION__);

	if (stat(devfs_id, &stat_buf) == 0)
		printf("# Note: the devfs pseudo file system is present\n");

	for (k = 0, res = 0; (k < MAX_SG_DEVS) && (num_errors < MAX_ERRORS);
	     ++k, res = (sg_fd >= 0) ? close(sg_fd) : 0) {
		if (res < 0) {
			snprintf(ebuff, EBUFF_SZ, "Error closing %s ", fname);
			perror("sg_map: close error");
			return 1;
		}
		make_dev_name(fname, "/dev/sg", k, do_numeric);

		sg_fd = open(fname, O_RDONLY | O_NONBLOCK);
		if (sg_fd < 0) {
			if (EBUSY == errno) {
				map_arr[k].active = -2;
				continue;
			} else if ((ENODEV == errno) || (ENOENT == errno) ||
				   (ENXIO == errno)) {
				++num_errors;
				++num_silent;
				map_arr[k].active = -1;
				continue;
			} else {
				if (EACCES == errno)
					eacces_err = 1;
				snprintf(ebuff, EBUFF_SZ, "Error opening %s ",
					 fname);
				perror(ebuff);
				++num_errors;
				continue;
			}
		}
		res = ioctl(sg_fd, SG_GET_SCSI_ID, &map_arr[k].sg_dat);
		if (res < 0) {
			snprintf(ebuff, EBUFF_SZ,
				 "device %s failed on sg ioctl, skip", fname);
			perror(ebuff);
			++num_errors;
			continue;
		}
		if (do_inquiry) {
			char buff[36];

			if (0 ==
			    do_scsi_inq(sg_fd, 0, 0, 0, buff, sizeof(buff),
					1)) {
				memcpy(map_arr[k].vendor, &buff[8], 8);
				memcpy(map_arr[k].product, &buff[16], 16);
				memcpy(map_arr[k].revision, &buff[32], 4);
			}
		}
		map_arr[k].active = 1;
		map_arr[k].oth_dev_num = -1;
		last_sg_ind = k;
	}
	if ((num_errors >= MAX_ERRORS) && (num_silent < num_errors)) {
		printf("Stopping because there are too many error\n");
		if (eacces_err)
			printf("    root access may be required\n");
		return 1;
	}
	if (last_sg_ind < 0) {
		printf("Stopping because no sg devices found\n");
	}

	if (do_all_s || do_sd)
		scan_dev_type("/dev/sd", MAX_SD_DEVS, 0, LIN_DEV_TYPE_SD,
			      last_sg_ind);
	if (do_all_s || do_sr)
		scan_dev_type("/dev/sr", MAX_SR_DEVS, 1, LIN_DEV_TYPE_SR,
			      last_sg_ind);
	if (do_all_s || do_scd)
		scan_dev_type("/dev/scd", MAX_SR_DEVS, 1, LIN_DEV_TYPE_SCD,
			      last_sg_ind);
	if (do_all_s || do_st)
		scan_dev_type("/dev/st", MAX_ST_DEVS, 1, LIN_DEV_TYPE_ST,
			      last_sg_ind);
	if (do_all_s || do_osst)
		scan_dev_type("/dev/osst", MAX_OSST_DEVS, 1, LIN_DEV_TYPE_OSST,
			      last_sg_ind);

	for (k = 0; k <= last_sg_ind; ++k) {
		make_dev_name(fname, "/dev/sg", k, do_numeric);
		printf("%s", fname);
		switch (map_arr[k].active) {
		case -2:
			printf(do_extra ? "  -2 -2 -2 -2  -2" : "  busy");
			break;
		case -1:
			printf(do_extra ? "  -1 -1 -1 -1  -1" :
			       "  not present");
			break;
		case 0:
			printf(do_extra ? "  -3 -3 -3 -3  -3" :
			       "  some error\n");
			break;
		case 1:
			if (do_extra)
				printf("  %d %d %d %d  %d",
				       map_arr[k].sg_dat.host_no,
				       map_arr[k].sg_dat.channel,
				       map_arr[k].sg_dat.scsi_id,
				       map_arr[k].sg_dat.lun,
				       map_arr[k].sg_dat.scsi_type);
			switch (map_arr[k].lin_dev_type) {
			case LIN_DEV_TYPE_SD:
				make_dev_name(fname, "/dev/sd",
					      map_arr[k].oth_dev_num, 0);
				printf("  %s", fname);
				break;
			case LIN_DEV_TYPE_ST:
				make_dev_name(fname, "/dev/st",
					      map_arr[k].oth_dev_num, 1);
				printf("  %s", fname);
				break;
			case LIN_DEV_TYPE_OSST:
				make_dev_name(fname, "/dev/osst",
					      map_arr[k].oth_dev_num, 1);
				printf("  %s", fname);
				break;
			case LIN_DEV_TYPE_SR:
				make_dev_name(fname, "/dev/sr",
					      map_arr[k].oth_dev_num, 1);
				printf("  %s", fname);
				break;
			case LIN_DEV_TYPE_SCD:
				make_dev_name(fname, "/dev/scd",
					      map_arr[k].oth_dev_num, 1);
				printf("  %s", fname);
				break;
			default:
				break;
			}
			if (do_inquiry)
				printf("  %.8s  %.16s  %.4s", map_arr[k].vendor,
				       map_arr[k].product, map_arr[k].revision);
			break;
		default:
			printf("  bad logic\n");
			break;
		}
		printf("\n");
	}
	return 0;
}

static int find_dev_in_sg_arr(My_scsi_idlun * my_idlun, int host_no,
			      int last_sg_ind)
{
	int k;
	struct sg_scsi_id *sidp;

	for (k = 0; k <= last_sg_ind; ++k) {
		sidp = &(map_arr[k].sg_dat);
		if ((host_no == sidp->host_no) &&
		    ((my_idlun->dev_id & 0xff) == sidp->scsi_id) &&
		    (((my_idlun->dev_id >> 8) & 0xff) == sidp->lun) &&
		    (((my_idlun->dev_id >> 16) & 0xff) == sidp->channel))
			return k;
	}
	return -1;
}

static void scan_dev_type(const char *leadin, int max_dev, int do_numeric,
			  int lin_dev_type, int last_sg_ind)
{
	int k, res, ind, sg_fd = 0;
	int num_errors = 0;
	int num_silent = 0;
	int host_no = -1;
	int nonMappedDevicesPresent = FALSE;
	My_scsi_idlun my_idlun;
	char fname[64];

	for (k = 0, res = 0; (k < max_dev) && (num_errors < MAX_ERRORS);
	     ++k, res = (sg_fd >= 0) ? close(sg_fd) : 0) {

/* ignore close() errors */
#if 0
		if (res < 0) {
			snprintf(ebuff, EBUFF_SZ, "Error closing %s ", fname);
			perror("sg_map: close error");
#ifndef IGN_CLOSE_ERR
			return;
#else
			++num_errors;
			sg_fd = 0;
#endif
		}
#endif
		make_dev_name(fname, leadin, k, do_numeric);
#ifdef DEBUG
		printf("Trying %s: ", fname);
#endif

		sg_fd = open(fname, O_RDONLY | O_NONBLOCK);
		if (sg_fd < 0) {
#ifdef DEBUG
			printf("ERROR %i\n", errno);
#endif
			if (EBUSY == errno) {
				printf("Device %s is busy\n", fname);
				++num_errors;
				continue;
			} else if ((ENODEV == errno) || (ENOENT == errno) ||
				   (ENXIO == errno)) {
				++num_errors;
				++num_silent;
				continue;
			} else {
				snprintf(ebuff, EBUFF_SZ, "Error opening %s ",
					 fname);
				perror(ebuff);
				++num_errors;
				continue;
			}
		}

		res = ioctl(sg_fd, SCSI_IOCTL_GET_IDLUN, &my_idlun);
		if (res < 0) {
			snprintf(ebuff, EBUFF_SZ,
				 "device %s failed on scsi ioctl(idlun), skip",
				 fname);
			perror(ebuff);
			++num_errors;
#ifdef DEBUG
			printf("Couldn't get IDLUN!\n");
#endif
			continue;
		}
		res = ioctl(sg_fd, SCSI_IOCTL_GET_BUS_NUMBER, &host_no);
		if (res < 0) {
			snprintf(ebuff, EBUFF_SZ,
				 "device %s failed on scsi ioctl(bus_number), skip",
				 fname);
			perror(ebuff);
			++num_errors;
#ifdef DEBUG
			printf("Couldn't get BUS!\n");
#endif
			continue;
		}
#ifdef DEBUG
		printf("%i(%x) %i %i %i %i\n", host_no, my_idlun.host_unique_id,
		       (my_idlun.dev_id >> 24) & 0xff,
		       (my_idlun.dev_id >> 16) & 0xff,
		       (my_idlun.dev_id >> 8) & 0xff, my_idlun.dev_id & 0xff);
#endif
		ind = find_dev_in_sg_arr(&my_idlun, host_no, last_sg_ind);
		if (ind >= 0) {
			map_arr[ind].oth_dev_num = k;
			map_arr[ind].lin_dev_type = lin_dev_type;
		} else if (ind != -1) {
			printf
			    ("Strange, could not find device %s mapped to sg device error %d??\n",
			     fname, ind);
		} else {
			nonMappedDevicesPresent = TRUE;
		}
	}
	if (nonMappedDevicesPresent) {
		printf("Unmapped Devices found...\n\n");
	}
}

/* Returns 0 when successful, else -1 */
static int do_simple_inq(int sg_fd, void *resp, int mx_resp_len, int noisy)
{
	int res;
	unsigned char inqCmdBlk[INQUIRY_CMDLEN] =
	    { INQUIRY_CMD, 0, 0, 0, 0, 0 };
	unsigned char sense_b[SENSE_BUFF_LEN];
	sg_io_hdr_t io_hdr;

	inqCmdBlk[4] = (unsigned char)mx_resp_len;
	memset(&io_hdr, 0, sizeof(sg_io_hdr_t));
	io_hdr.interface_id = 'S';
	io_hdr.cmd_len = sizeof(inqCmdBlk);
	io_hdr.mx_sb_len = sizeof(sense_b);
	io_hdr.dxfer_direction = SG_DXFER_FROM_DEV;
	io_hdr.dxfer_len = mx_resp_len;
	io_hdr.dxferp = resp;
	io_hdr.cmdp = inqCmdBlk;
	io_hdr.sbp = sense_b;
	io_hdr.timeout = DEF_TIMEOUT;

	if (ioctl(sg_fd, SG_IO, &io_hdr) < 0) {
		perror("SG_IO (inquiry) error");
		return -1;
	}
	res = sg_err_category3(&io_hdr);
	switch (res) {
	case SG_ERR_CAT_CLEAN:
	case SG_ERR_CAT_RECOVERED:
		return 0;
	default:
		if (noisy) {
			char ebuff[EBUFF_SZ];
			snprintf(ebuff, EBUFF_SZ, "Inquiry error ");
			sg_chk_n_print3(ebuff, &io_hdr);
		}
		return -1;
	}
}

static int do_modes(int sg_fd, int dbd, int pc, int pg_code, int sub_pg_code,
		    void *resp, int mx_resp_len, int noisy, int mode6)
{
	int res;
	unsigned char modesCmdBlk[MODE_SENSE10_CMDLEN] =
	    { MODE_SENSE10_CMD, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	unsigned char sense_b[SENSE_BUFF_LEN];
	sg_io_hdr_t io_hdr;

	modesCmdBlk[1] = (unsigned char)(dbd ? 0x8 : 0);
	modesCmdBlk[2] = (unsigned char)(((pc << 6) & 0xc0) | (pg_code & 0x3f));
	modesCmdBlk[3] = (unsigned char)(sub_pg_code & 0xff);
	if (mx_resp_len > (mode6 ? 0xff : 0xffff)) {
		printf(ME "mx_resp_len too big\n");
		return -1;
	}
	if (mode6) {
		modesCmdBlk[0] = MODE_SENSE6_CMD;
		modesCmdBlk[4] = (unsigned char)(mx_resp_len & 0xff);
	} else {
		modesCmdBlk[7] = (unsigned char)((mx_resp_len >> 8) & 0xff);
		modesCmdBlk[8] = (unsigned char)(mx_resp_len & 0xff);
	}

	memset(&io_hdr, 0, sizeof(sg_io_hdr_t));
	memset(sense_b, 0, sizeof(sense_b));
	io_hdr.interface_id = 'S';
	io_hdr.cmd_len = mode6 ? MODE_SENSE6_CMDLEN : MODE_SENSE10_CMDLEN;
	io_hdr.mx_sb_len = sizeof(sense_b);
	io_hdr.dxfer_direction = SG_DXFER_FROM_DEV;
	io_hdr.dxfer_len = mx_resp_len;
	io_hdr.dxferp = resp;
	io_hdr.cmdp = modesCmdBlk;
	io_hdr.sbp = sense_b;
	io_hdr.timeout = DEF_TIMEOUT;

	if (ioctl(sg_fd, SG_IO, &io_hdr) < 0) {
		perror("SG_IO (mode sense) error");
		return -1;
	}
	res = sg_err_category3(&io_hdr);
	switch (res) {
	case SG_ERR_CAT_CLEAN:
	case SG_ERR_CAT_RECOVERED:
		return 0;
	default:
		if (noisy) {
			char ebuff[EBUFF_SZ];
			snprintf(ebuff, EBUFF_SZ, "Mode sense error, dbd=%d "
				 "pc=%d page_code=%x sub_page_code=%x\n     ",
				 dbd, pc, pg_code, sub_pg_code);
			sg_chk_n_print3(ebuff, &io_hdr);
		}
		if ((0x70 == (0x7f & sense_b[0])) && (0x20 == sense_b[12]) &&
		    (0x0 == sense_b[13])) {
			if (mode6)
				fprintf(stderr,
					">>>>>> drop '-6' switch and try again with "
					"a 10 byte MODE SENSE\n");
			else
				fprintf(stderr,
					">>>>>> add '-6' switch and try again with "
					"a 6 byte MODE SENSE\n");
		}
		return -1;
	}
}

const char *scsi_ptype_strs[] = {
	"disk",
	"tape",
	"printer",
	"processor",
	"write once optical disk",
	"cd/dvd",
	"scanner",
	"optical memory device",
	"medium changer",
	"communications",
	"graphics",
	"graphics",
	"storage array controller",
	"enclosure services device",
	"simplified direct access device",
	"optical card reader/writer device",
};

const char *get_ptype_str(int scsi_ptype)
{
	int num = sizeof(scsi_ptype_strs) / sizeof(scsi_ptype_strs[0]);

	return (scsi_ptype < num) ? scsi_ptype_strs[scsi_ptype] : "";
}

static struct page_code_desc pc_desc_all[] = {
	{0x0, "Unit Attention condition [vendor: page format not required]"},
	{0x2, "Disconnect-Reconnect"},
	{0xa, "Control"},
	{0x15, "Extended"},
	{0x16, "Extended device-type specific"},
	{0x18, "Protocol specific LUN"},
	{0x19, "Protocol specific port"},
	{0x1a, "Power condition"},
	{0x1c, "Informational exceptions control"},
	{0x3f, "[yields all supported pages]"},
};

static struct page_code_desc pc_desc_disk[] = {
	{0x1, "Read-Write error recovery"},
	{0x3, "Format"},
	{0x4, "Rigid disk geometry"},
	{0x5, "Flexible geometry"},
	{0x7, "Verify error recovery"},
	{0x8, "Caching"},
	{0x9, "Peripheral device (spc-2 ?)"},
	{0xb, "Medium types supported"},
	{0xc, "Notch and partition"},
	{0xd, "Power condition (obsolete)"},
	{0x10, "XOR control"},
};

static struct page_code_desc pc_desc_tape[] = {
	{0xf, "Data Compression"},
	{0x10, "Device config"},
	{0x11, "Medium Partition [1]"},
	{0x12, "Medium Partition [2]"},
	{0x13, "Medium Partition [3]"},
	{0x14, "Medium Partition [4]"},
	{0x1c, "Informational exceptions control (tape version)"},
};

static struct page_code_desc pc_desc_cddvd[] = {
	{0x1, "Read-Write error recovery"},
	{0x3, "MRW"},
	{0x5, "Write parameters"},
	{0xd, "CD device parameters (obsolete)"},
	{0xe, "CD audio"},
	{0x1a, "Power condition"},
	{0x1c, "Fault/failure reporting control"},
	{0x1d, "Timeout and protect"},
	{0x2a, "MM capabilities and mechanical status (obsolete)"},
};

static struct page_code_desc pc_desc_smc[] = {
	{0x1d, "Element address assignment"},
	{0x1e, "Transport geometry parameters"},
	{0x1f, "Device capabilities"},
};

static struct page_code_desc pc_desc_scc[] = {
	{0x1b, "LUN mapping"},
};

static struct page_code_desc pc_desc_ses[] = {
	{0x14, "Enclosure services management"},
};

struct page_code_desc *find_mode_page_table(int scsi_ptype, int *size)
{
	switch (scsi_ptype) {
	case 0:		/* disk (direct access) type devices */
	case 4:
	case 7:
	case 0xe:
		*size = sizeof(pc_desc_disk) / sizeof(pc_desc_disk[0]);
		return &pc_desc_disk[0];
	case 1:		/* tape devices */
	case 2:
		*size = sizeof(pc_desc_tape) / sizeof(pc_desc_tape[0]);
		return &pc_desc_tape[0];
	case 5:		/* cd/dvd devices */
		*size = sizeof(pc_desc_cddvd) / sizeof(pc_desc_cddvd[0]);
		return &pc_desc_cddvd[0];
	case 8:		/* medium changer devices */
		*size = sizeof(pc_desc_smc) / sizeof(pc_desc_smc[0]);
		return &pc_desc_smc[0];
	case 0xc:		/* storage array devices */
		*size = sizeof(pc_desc_scc) / sizeof(pc_desc_scc[0]);
		return &pc_desc_scc[0];
	case 0xd:		/* enclosure services devices */
		*size = sizeof(pc_desc_ses) / sizeof(pc_desc_ses[0]);
		return &pc_desc_ses[0];
	}
	*size = 0;
	return NULL;
}

const char *find_page_code_desc(int page_num, int scsi_ptype)
{
	int k;
	int num;
	const struct page_code_desc *pcdp;

	pcdp = find_mode_page_table(scsi_ptype, &num);
	if (pcdp) {
		for (k = 0; k < num; ++k, ++pcdp) {
			if (page_num == pcdp->page_code)
				return pcdp->desc;
			else if (page_num < pcdp->page_code)
				break;
		}
	}
	pcdp = &pc_desc_all[0];
	num = sizeof(pc_desc_all) / sizeof(pc_desc_all[0]);
	for (k = 0; k < num; ++k, ++pcdp) {
		if (page_num == pcdp->page_code)
			return pcdp->desc;
		else if (page_num < pcdp->page_code)
			break;
	}
	return NULL;
}

static void list_page_codes(int scsi_ptype)
{
	int k;
	int num = sizeof(pc_desc_all) / sizeof(pc_desc_all[0]);
	const struct page_code_desc *pcdp = &pc_desc_all[0];
	int num_ptype;
	const struct page_code_desc *pcd_ptypep;

	pcd_ptypep = find_mode_page_table(scsi_ptype, &num_ptype);
	printf("Page_Code  Description\n");
	for (k = 0; k < 0x3f; ++k) {
		if (pcd_ptypep && (num_ptype > 0)) {
			if (k == pcd_ptypep->page_code) {
				printf(" 0x%02x      %s\n",
				       pcd_ptypep->page_code, pcd_ptypep->desc);
				++pcd_ptypep;
				--num_ptype;
				continue;
			} else if (k > pcd_ptypep->page_code) {
				pcd_ptypep++;
				--num_ptype;
			}
		}
		if (pcdp && (num > 0)) {
			if (k == pcdp->page_code) {
				printf(" 0x%02x      %s\n", pcdp->page_code,
				       pcdp->desc);
				++pcdp;
				--num;
				continue;
			} else if (k > pcdp->page_code) {
				pcdp++;
				--num;
			}
		}
	}
}

int show_scsi_modes(char *device)
{
	int sg_fd, k, num, len, md_len, bd_len, longlba, page_num;
	char *file_name = 0;
	char ebuff[EBUFF_SZ];
	const char *descp;
	unsigned char rsp_buff[MODE_ALLOC_LEN];
	int rsp_buff_size = MODE_ALLOC_LEN;
	int pg_code = 0;
	int sub_pg_code = 0;
	int pc = 0;
	int do_all = 1;
	int do_dbd = 0;
	int do_hex = 0;
	int do_mode6 = 0;	/* Use MODE SENSE(6) instead of MODE SENSE(10) */
	int oflags = O_RDONLY | O_NONBLOCK;
	struct sg_scsi_id a_sid;
	int scsi_ptype, density_code_off;
	unsigned char *ucp;
	unsigned char uc;

	print_msg(TEST_BREAK, __FUNCTION__);

	file_name = device;

	list_page_codes(0);

	/* The 6 bytes command only allows up to 255 bytes of response data */
	if (do_mode6)
		rsp_buff_size = 255;

	if ((sg_fd = open(file_name, oflags)) < 0) {
		snprintf(ebuff, EBUFF_SZ, ME "error opening file: %s",
			 file_name);
		perror(ebuff);
		return 1;
	}
	/* Just to be safe, check we have a new sg device by trying an ioctl */
	if ((ioctl(sg_fd, SG_GET_VERSION_NUM, &k) < 0) || (k < 30000)) {
		printf(ME "%s doesn't seem to be a version 3 sg device\n",
		       file_name);
		close(sg_fd);
		return 1;
	}
	if (ioctl(sg_fd, SG_GET_SCSI_ID, &a_sid) < 0) {
		unsigned char inqBuff[36];

		if (do_simple_inq(sg_fd, inqBuff, sizeof(inqBuff), 1)) {
			printf(ME "%s doesn't respond to a SCSI INQUIRY\n",
			       file_name);
			close(sg_fd);
			return 1;
		}
		scsi_ptype = inqBuff[0] & 0x1f;	/* fetch peripheral device type */
	} else
		scsi_ptype = a_sid.scsi_type;
	printf("  SCSI peripheral type: %s [0x%x] (from INQUIRY)\n",
	       get_ptype_str(scsi_ptype), scsi_ptype);

	if (do_all)
		pg_code = MODE_CODE_ALL;

	if (0 == do_modes(sg_fd, do_dbd, pc, pg_code, sub_pg_code,
			  rsp_buff, rsp_buff_size, 1, do_mode6)) {
		int medium_type, specific, headerlen;

		printf("Mode parameter header from %s byte MODE SENSE:\n",
		       (do_mode6 ? "6" : "10"));
		if (do_mode6) {
			headerlen = 4;
			if (do_hex)
				dStrHex((const char *)rsp_buff, headerlen, 1);
			md_len = rsp_buff[0] + 1;
			bd_len = rsp_buff[3];
			medium_type = rsp_buff[1];
			specific = rsp_buff[2];
			longlba = 0;	/* what is this field? */
		} else {
			headerlen = 8;
			md_len = (rsp_buff[0] << 8) + rsp_buff[1] + 2;
			bd_len = (rsp_buff[6] << 8) + rsp_buff[7];
			medium_type = rsp_buff[2];
			specific = rsp_buff[3];
			longlba = rsp_buff[4] & 1;
		}
		if (do_hex)
			dStrHex((const char *)rsp_buff, headerlen, 1);
		printf("  Mode data length=%d, medium type=0x%.2x, specific"
		       " param=0x%.2x, longlba=%d\n", md_len, medium_type,
		       specific, longlba);
		if (md_len > rsp_buff_size) {
			printf
			    ("Only fetched %d bytes of response, truncate output\n",
			     rsp_buff_size);
			md_len = rsp_buff_size;
			if (bd_len + headerlen > rsp_buff_size)
				bd_len = rsp_buff_size - headerlen;
		}
		printf("  Block descriptor length=%d\n", bd_len);
		if (bd_len > 0) {
			len = 8;
			density_code_off = 0;
			num = bd_len;
			if (longlba) {
				printf("> longlba block descriptors:\n");
				len = 16;
				density_code_off = 8;
			} else if (0 == scsi_ptype) {
				printf
				    ("> Direct access device block descriptors:\n");
				density_code_off = 4;
			} else
				printf
				    ("> General mode parameter block descriptors:\n");

			ucp = rsp_buff + headerlen;
			while (num > 0) {
				printf("   Density code=0x%x\n",
				       *(ucp + density_code_off));
				dStrHex((const char *)ucp, len, 1);
				ucp += len;
				num -= len;
			}
			printf("\n");
		}
		ucp = rsp_buff + bd_len + headerlen;	/* start of mode page(s) */
		md_len -= bd_len + headerlen;	/* length of mode page(s) */
		while (md_len > 0) {	/* got mode page(s) */
			uc = *ucp;
			page_num = ucp[0] & 0x3f;
			if (do_hex)
				descp = NULL;
			else {
				descp =
				    find_page_code_desc(page_num, scsi_ptype);
				if (NULL == descp)
					snprintf(ebuff, EBUFF_SZ,
						 "vendor[0x%x]", page_num);
			}
			if (uc & 0x40) {
				len = (ucp[2] << 8) + ucp[3] + 4;
				if (do_hex)
					printf
					    (">> page_code=0x%x, subpage_code=0x%x, "
					     "page_control=%d\n", page_num,
					     ucp[1], pc);
				else
					printf
					    (">> page_code: %s, subpage_code=0x%x, "
					     "page_control: %s\n",
					     (descp ? descp : ebuff), ucp[1],
					     pg_control_str_arr[pc]);
			} else {
				len = ucp[1] + 2;
				if (do_hex)
					printf
					    (">> page_code=0x%x, page_control=%d\n",
					     page_num, pc);
				else
					printf
					    (">> page_code: %s, page_control: %s\n",
					     (descp ? descp : ebuff),
					     pg_control_str_arr[pc]);
			}
			dStrHex((const char *)ucp, len, 1);
			ucp += len;
			md_len -= len;
		}
	}

	close(sg_fd);
	return 0;
}

int do_scsi_read_buffer(char *device)
{
	int sg_fd, res;
	unsigned int k, num;
	unsigned char rbCmdBlk[RB_CMD_LEN];
	unsigned char *rbBuff = NULL;
	void *rawp = NULL;
	unsigned char sense_buffer[32];
	int buf_capacity = 0;
	int do_quick = 0;
	int do_dio = 0;
	int do_mmap = 1;
	int do_time = 0;
	int buf_size = 0;
	unsigned int total_size_mb = RB_MB_TO_READ;
	char *file_name = 0;
	size_t psz = getpagesize();
	int dio_incomplete = 0;
	sg_io_hdr_t io_hdr;
	struct timeval start_tm, end_tm;
#ifdef SG_DEBUG
	int clear = 1;
#endif

	print_msg(TEST_BREAK, __FUNCTION__);

	file_name = device;

	sg_fd = open(file_name, O_RDONLY);
	if (sg_fd < 0) {
		perror(ME "open error");
		return 1;
	}
	/* Don't worry, being very careful not to write to a none-sg file ... */
	res = ioctl(sg_fd, SG_GET_VERSION_NUM, &k);
	if ((res < 0) || (k < 30000)) {
		printf(ME "not a sg device, or driver prior to 3.x\n");
		return 1;
	}
	if (do_mmap) {
		do_dio = 0;
		do_quick = 0;
	}
	if (NULL == (rawp = malloc(512))) {
		printf(ME "out of memory (query)\n");
		return 1;
	}
	rbBuff = rawp;

	memset(rbCmdBlk, 0, RB_CMD_LEN);
	rbCmdBlk[0] = RB_OPCODE;
	rbCmdBlk[1] = RB_MODE_DESC;
	rbCmdBlk[8] = RB_DESC_LEN;
	memset(&io_hdr, 0, sizeof(sg_io_hdr_t));
	io_hdr.interface_id = 'S';
	io_hdr.cmd_len = sizeof(rbCmdBlk);
	io_hdr.mx_sb_len = sizeof(sense_buffer);
	io_hdr.dxfer_direction = SG_DXFER_FROM_DEV;
	io_hdr.dxfer_len = RB_DESC_LEN;
	io_hdr.dxferp = rbBuff;
	io_hdr.cmdp = rbCmdBlk;
	io_hdr.sbp = sense_buffer;
	io_hdr.timeout = 60000;	/* 60000 millisecs == 60 seconds */
	/* do normal IO to find RB size (not dio or mmap-ed at this stage) */

	if (ioctl(sg_fd, SG_IO, &io_hdr) < 0) {
		perror(ME "SG_IO READ BUFFER descriptor error");
		if (rawp)
			free(rawp);
		return 1;
	}

	/* now for the error processing */
	switch (sg_err_category3(&io_hdr)) {
	case SG_ERR_CAT_CLEAN:
		break;
	case SG_ERR_CAT_RECOVERED:
		printf
		    ("Recovered error on READ BUFFER descriptor, continuing\n");
		break;
	default:		/* won't bother decoding other categories */
		sg_chk_n_print3("READ BUFFER descriptor error", &io_hdr);
		if (rawp)
			free(rawp);
		return 1;
	}

	buf_capacity = ((rbBuff[1] << 16) | (rbBuff[2] << 8) | rbBuff[3]);
	printf("READ BUFFER reports: buffer capacity=%d, offset boundary=%d\n",
	       buf_capacity, (int)rbBuff[0]);

	if (0 == buf_size)
		buf_size = buf_capacity;
	else if (buf_size > buf_capacity) {
		printf
		    ("Requested buffer size=%d exceeds reported capacity=%d\n",
		     buf_size, buf_capacity);
		if (rawp)
			free(rawp);
		return 1;
	}
	if (rawp) {
		free(rawp);
		rawp = NULL;
	}

	if (!do_dio) {
		k = buf_size;
		if (do_mmap && (0 != (k % psz)))
			k = ((k / psz) + 1) * psz;	/* round up to page size */
		res = ioctl(sg_fd, SG_SET_RESERVED_SIZE, &k);
		if (res < 0)
			perror(ME "SG_SET_RESERVED_SIZE error");
	}

	if (do_mmap) {
		rbBuff = mmap(NULL, buf_size, PROT_READ, MAP_SHARED, sg_fd, 0);
		if (MAP_FAILED == rbBuff) {
			if (ENOMEM == errno)
				printf(ME "mmap() out of memory, try a smaller "
				       "buffer size than %d KB\n",
				       buf_size / 1024);
			else
				perror(ME "error using mmap()");
			return 1;
		}
	} else {		/* non mmap-ed IO */
		rawp = malloc(buf_size + (do_dio ? psz : 0));
		if (NULL == rawp) {
			printf(ME "out of memory (data)\n");
			return 1;
		}
		if (do_dio)	/* align to page boundary */
			rbBuff =
			    (unsigned char *)(((unsigned long)rawp + psz - 1) &
					      (~(psz - 1)));
		else
			rbBuff = rawp;
	}

	num = (total_size_mb * 1024U * 1024U) / (unsigned int)buf_size;
	if (do_time) {
		start_tm.tv_sec = 0;
		start_tm.tv_usec = 0;
		gettimeofday(&start_tm, NULL);
	}
	/* main data reading loop */
	for (k = 0; k < num; ++k) {
		memset(rbCmdBlk, 0, RB_CMD_LEN);
		rbCmdBlk[0] = RB_OPCODE;
		rbCmdBlk[1] = RB_MODE_DATA;
		rbCmdBlk[6] = 0xff & (buf_size >> 16);
		rbCmdBlk[7] = 0xff & (buf_size >> 8);
		rbCmdBlk[8] = 0xff & buf_size;
#ifdef SG_DEBUG
		memset(rbBuff, 0, buf_size);
#endif

		memset(&io_hdr, 0, sizeof(sg_io_hdr_t));
		io_hdr.interface_id = 'S';
		io_hdr.cmd_len = sizeof(rbCmdBlk);
		io_hdr.mx_sb_len = sizeof(sense_buffer);
		io_hdr.dxfer_direction = SG_DXFER_FROM_DEV;
		io_hdr.dxfer_len = buf_size;
		if (!do_mmap)
			io_hdr.dxferp = rbBuff;
		io_hdr.cmdp = rbCmdBlk;
		io_hdr.sbp = sense_buffer;
		io_hdr.timeout = 20000;	/* 20000 millisecs == 20 seconds */
		io_hdr.pack_id = k;
		if (do_mmap)
			io_hdr.flags |= SG_FLAG_MMAP_IO;
		else if (do_dio)
			io_hdr.flags |= SG_FLAG_DIRECT_IO;
		else if (do_quick)
			io_hdr.flags |= SG_FLAG_NO_DXFER;

		if (ioctl(sg_fd, SG_IO, &io_hdr) < 0) {
			if (ENOMEM == errno)
				printf(ME
				       "SG_IO data; out of memory, try a smaller "
				       "buffer size than %d KB\n",
				       buf_size / 1024);
			else
				perror(ME "SG_IO READ BUFFER data error");
			if (rawp)
				free(rawp);
			return 1;
		}

		/* now for the error processing */
		switch (sg_err_category3(&io_hdr)) {
		case SG_ERR_CAT_CLEAN:
			break;
		case SG_ERR_CAT_RECOVERED:
			printf
			    ("Recovered error on READ BUFFER data, continuing\n");
			break;
		default:	/* won't bother decoding other categories */
			sg_chk_n_print3("READ BUFFER data error", &io_hdr);
			if (rawp)
				free(rawp);
			return 1;
		}
		if (do_dio &&
		    ((io_hdr.info & SG_INFO_DIRECT_IO_MASK) !=
		     SG_INFO_DIRECT_IO))
			dio_incomplete = 1;	/* flag that dio not done (completely) */

#ifdef SG_DEBUG
		if (clear) {
			for (j = 0; j < buf_size; ++j) {
				if (rbBuff[j] != 0) {
					clear = 0;
					break;
				}
			}
		}
#endif
	}
	if ((do_time) && (start_tm.tv_sec || start_tm.tv_usec)) {
		struct timeval res_tm;
		double a, b;

		gettimeofday(&end_tm, NULL);
		res_tm.tv_sec = end_tm.tv_sec - start_tm.tv_sec;
		res_tm.tv_usec = end_tm.tv_usec - start_tm.tv_usec;
		if (res_tm.tv_usec < 0) {
			--res_tm.tv_sec;
			res_tm.tv_usec += 1000000;
		}
		a = res_tm.tv_sec;
		a += (0.000001 * res_tm.tv_usec);
		b = (double)buf_size *num;
		printf("time to read data from buffer was %d.%06d secs",
		       (int)res_tm.tv_sec, (int)res_tm.tv_usec);
		if ((a > 0.00001) && (b > 511))
			printf(", %.2f MB/sec\n", b / (a * 1000000.0));
		else
			printf("\n");
	}
	if (dio_incomplete)
		printf(">> direct IO requested but not done\n");
	printf
	    ("Read %u MBytes (actual %u MB, %u bytes), buffer size=%d KBytes\n",
	     total_size_mb, (num * buf_size) / 1048576, num * buf_size,
	     buf_size / 1024);

	if (rawp)
		free(rawp);
	res = close(sg_fd);
	if (res < 0) {
		perror(ME "close error");
		return 0;
	}
#ifdef SG_DEBUG
	if (clear)
		printf("read buffer always zero\n");
	else
		printf("read buffer non-zero\n");
#endif
	return 0;
}

/* Performs a 10 byte READ CAPACITY command and fetches response. There is
 * evidently a 16 byte READ CAPACITY command coming.
 * Return of 0 -> success, -1 -> failure */
int do_readcap_10(int sg_fd, int pmi, unsigned int lba,
		  unsigned int *last_sect, unsigned int *sect_sz)
{
	int res;
	unsigned char rcCmdBlk[10] = { 0x25, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	unsigned char rcBuff[RCAP_REPLY_LEN];
	unsigned char sense_b[SENSE_BUFF_SZ];
	sg_io_hdr_t io_hdr;

	if (pmi) {		/* lbs only valid when pmi set */
		rcCmdBlk[8] |= 1;
		rcCmdBlk[2] = (lba >> 24) & 0xff;
		rcCmdBlk[3] = (lba >> 16) & 0xff;
		rcCmdBlk[4] = (lba >> 8) & 0xff;
		rcCmdBlk[5] = lba & 0xff;
	}
	memset(&io_hdr, 0, sizeof(sg_io_hdr_t));
	io_hdr.interface_id = 'S';
	io_hdr.cmd_len = sizeof(rcCmdBlk);
	io_hdr.mx_sb_len = sizeof(sense_b);
	io_hdr.dxfer_direction = SG_DXFER_FROM_DEV;
	io_hdr.dxfer_len = sizeof(rcBuff);
	io_hdr.dxferp = rcBuff;
	io_hdr.cmdp = rcCmdBlk;
	io_hdr.sbp = sense_b;
	io_hdr.timeout = 60000;

	while (1) {
		if (ioctl(sg_fd, SG_IO, &io_hdr) < 0) {
			perror("read_capacity (SG_IO) error");
			return -1;
		}
		res = sg_err_category3(&io_hdr);
		if (SG_ERR_CAT_MEDIA_CHANGED == res)
			continue;
		else if (SG_ERR_CAT_CLEAN != res) {
			sg_chk_n_print3("READ CAPACITY command error", &io_hdr);
			return -1;
		} else
			break;
	}
	*last_sect = ((rcBuff[0] << 24) | (rcBuff[1] << 16) |
		      (rcBuff[2] << 8) | rcBuff[3]);
	*sect_sz = (rcBuff[4] << 24) | (rcBuff[5] << 16) |
	    (rcBuff[6] << 8) | rcBuff[7];
	return 0;
}

int show_scsi_read_capacity(char *device)
{
	int sg_fd, k, res;
	unsigned int lba = 0;
	int pmi = 1;
	unsigned int last_blk_addr, block_size;
	char ebuff[EBUFF_SZ];
	const char *file_name = 0;

	print_msg(TEST_BREAK, __FUNCTION__);

	file_name = device;

	if ((0 == pmi) && (lba > 0)) {
		fprintf(stderr,
			ME "lba can only be non-zero when pmi is set\n");
		usage();
		return 1;
	}
	if ((sg_fd = open(file_name, O_RDONLY)) < 0) {
		snprintf(ebuff, EBUFF_SZ, ME "error opening file: %s",
			 file_name);
		perror(ebuff);
		return 1;
	}
	/* Just to be safe, check we have a new sg device by trying an ioctl */
	if ((ioctl(sg_fd, SG_GET_VERSION_NUM, &k) < 0) || (k < 30000)) {
		printf(ME "%s doesn't seem to be a version 3 sg device\n",
		       file_name);
		close(sg_fd);
		return 1;
	}
	res = do_readcap_10(sg_fd, pmi, lba, &last_blk_addr, &block_size);

	if (0 == res) {
		printf("Read Capacity results:\n");
		if (pmi)
			printf("   PMI mode: given lba=0x%x, last block before "
			       "delay=0x%x\n", lba, last_blk_addr);
		else
			printf
			    ("   Last block address=%u (0x%x), Number of blocks=%u\n",
			     last_blk_addr, last_blk_addr, last_blk_addr + 1);
		printf("   Block size = %u bytes\n", block_size);
	}
	close(sg_fd);
	return 0;
}

int do_scsi_reset_devices(char *device, int reset_opt)
{
	int sg_fd, res, k;
	int do_device_reset = 0;
	int do_bus_reset = 0;
	int do_host_reset = 0;
	char *file_name = 0;

	switch (reset_opt) {
	case DEVICE_RESET:
		print_msg(TEST_BREAK, __FUNCTION__);
		do_device_reset = 1;
		break;
	case HOST_RESET:
		do_host_reset = 1;
		break;
	case BUS_RESET:
		do_bus_reset = 1;
		break;
	}

	file_name = device;

	sg_fd = open(file_name, O_RDWR | O_NONBLOCK);
	if (sg_fd < 0) {
		perror("sg_reset: open error");
		return 1;
	}

	k = SG_SCSI_RESET_NOTHING;
	if (do_device_reset) {
		printf("sg_reset: starting device reset\n");
		k = SG_SCSI_RESET_DEVICE;
	} else if (do_bus_reset) {
		printf("sg_reset: starting bus reset\n");
		k = SG_SCSI_RESET_BUS;
	} else if (do_host_reset) {
		printf("sg_reset: starting host reset\n");
		k = SG_SCSI_RESET_HOST;
	}

	res = ioctl(sg_fd, SG_SCSI_RESET, &k);
	if (res < 0) {
		if (EBUSY == errno)
			printf("sg_reset: BUSY, may be resetting now\n");
		else if (EIO == errno)
			printf
			    ("sg_reset: requested type of reset may not be available\n");
		else if (EACCES == errno)
			printf("sg_reset: reset requires CAP_SYS_ADMIN (root) "
			       "permission\n");
		else if (EINVAL == errno)
			printf("sg_reset: SG_SCSI_RESET not supported\n");
		else if (EIO == errno)
			printf("sg_reset: scsi_reset_provider() call failed\n");
		else
			perror("sg_reset: SG_SCSI_RESET failed");
		return 1;
	}
	if (SG_SCSI_RESET_NOTHING == k)
		printf("sg_reset: did nothing, device is normal mode\n");
	else if (SG_SCSI_RESET_DEVICE == k)
		printf("sg_reset: completed device reset\n");
	else if (SG_SCSI_RESET_BUS == k)
		printf("sg_reset: completed bus reset\n");
	else if (SG_SCSI_RESET_HOST == k)
		printf("sg_reset: completed host reset\n");

	if (close(sg_fd) < 0) {
		perror("sg_reset: close error");
		return 1;
	}
	return 0;
}

static int do_senddiag(int sg_fd, int sf_code, int pf_bit, int sf_bit,
		       int devofl_bit, int unitofl_bit, void *outgoing_pg,
		       int outgoing_len, int noisy)
{
	int res;
	unsigned char senddiagCmdBlk[SEND_DIAGNOSTIC_CMDLEN] =
	    { SEND_DIAGNOSTIC_CMD, 0, 0, 0, 0, 0 };
	unsigned char sense_b[SENSE_BUFF_LEN];
	sg_io_hdr_t io_hdr;

	senddiagCmdBlk[1] = (unsigned char)((sf_code << 5) | (pf_bit << 4) |
					    (sf_bit << 2) | (devofl_bit << 1) |
					    unitofl_bit);
	senddiagCmdBlk[3] = (unsigned char)((outgoing_len >> 8) & 0xff);
	senddiagCmdBlk[4] = (unsigned char)(outgoing_len & 0xff);

	memset(&io_hdr, 0, sizeof(sg_io_hdr_t));
	io_hdr.interface_id = 'S';
	io_hdr.cmd_len = SEND_DIAGNOSTIC_CMDLEN;
	io_hdr.mx_sb_len = sizeof(sense_b);
	io_hdr.dxfer_direction = outgoing_len ? SG_DXFER_TO_DEV : SG_DXFER_NONE;
	io_hdr.dxfer_len = outgoing_len;
	io_hdr.dxferp = outgoing_pg;
	io_hdr.cmdp = senddiagCmdBlk;
	io_hdr.sbp = sense_b;
	io_hdr.timeout = LONG_TIMEOUT;

	if (ioctl(sg_fd, SG_IO, &io_hdr) < 0) {
		perror("SG_IO (send diagnostic) error");
		return -1;
	}
	res = sg_err_category3(&io_hdr);
	switch (res) {
	case SG_ERR_CAT_CLEAN:
	case SG_ERR_CAT_RECOVERED:
		return 0;
	default:
		if (noisy) {
			char ebuff[EBUFF_SZ];
			snprintf(ebuff, EBUFF_SZ,
				 "Send diagnostic error, sf_code=0x%x, "
				 "pf_bit=%d, sf_bit=%d ", sf_code, pf_bit,
				 sf_bit);
			sg_chk_n_print3(ebuff, &io_hdr);
		}
		return -1;
	}
}

static int do_rcvdiag(int sg_fd, int pcv, int pg_code, void *resp,
		      int mx_resp_len, int noisy)
{
	int res;
	unsigned char rcvdiagCmdBlk[RECEIVE_DIAGNOSTIC_CMDLEN] =
	    { RECEIVE_DIAGNOSTIC_CMD, 0, 0, 0, 0, 0 };
	unsigned char sense_b[SENSE_BUFF_LEN];
	sg_io_hdr_t io_hdr;

	rcvdiagCmdBlk[1] = (unsigned char)(pcv ? 0x1 : 0);
	rcvdiagCmdBlk[2] = (unsigned char)(pg_code);
	rcvdiagCmdBlk[3] = (unsigned char)((mx_resp_len >> 8) & 0xff);
	rcvdiagCmdBlk[4] = (unsigned char)(mx_resp_len & 0xff);

	memset(&io_hdr, 0, sizeof(sg_io_hdr_t));
	io_hdr.interface_id = 'S';
	io_hdr.cmd_len = RECEIVE_DIAGNOSTIC_CMDLEN;
	io_hdr.mx_sb_len = sizeof(sense_b);
	io_hdr.dxfer_direction = SG_DXFER_FROM_DEV;
	io_hdr.dxfer_len = mx_resp_len;
	io_hdr.dxferp = resp;
	io_hdr.cmdp = rcvdiagCmdBlk;
	io_hdr.sbp = sense_b;
	io_hdr.timeout = DEF_TIMEOUT;

	if (ioctl(sg_fd, SG_IO, &io_hdr) < 0) {
		perror("SG_IO (receive diagnostic) error");
		return -1;
	}
	res = sg_err_category3(&io_hdr);
	switch (res) {
	case SG_ERR_CAT_CLEAN:
	case SG_ERR_CAT_RECOVERED:
		return 0;
	default:
		if (noisy) {
			char ebuff[EBUFF_SZ];
			snprintf(ebuff, EBUFF_SZ,
				 "Receive diagnostic error, pcv=%d, "
				 "page_code=%x ", pcv, pg_code);
			sg_chk_n_print3(ebuff, &io_hdr);
		}
		return -1;
	}
}

/* Get last extended self-test time from mode page 0xa (for '-e' option) */
static int do_modes_0a(int sg_fd, void *resp, int mx_resp_len, int noisy,
		       int mode6)
{
	int res;
	unsigned char modesCmdBlk[MODE_SENSE10_CMDLEN] =
	    { MODE_SENSE10_CMD, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	unsigned char sense_b[SENSE_BUFF_LEN];
	sg_io_hdr_t io_hdr;
	int dbd = 1;
	int pc = 0;
	int pg_code = 0xa;

	modesCmdBlk[1] = (unsigned char)(dbd ? 0x8 : 0);
	modesCmdBlk[2] = (unsigned char)(((pc << 6) & 0xc0) | (pg_code & 0x3f));
	if (mx_resp_len > (mode6 ? 0xff : 0xffff)) {
		printf(ME "mx_resp_len too big\n");
		return -1;
	}
	if (mode6) {
		modesCmdBlk[0] = MODE_SENSE6_CMD;
		modesCmdBlk[4] = (unsigned char)(mx_resp_len & 0xff);
	} else {
		modesCmdBlk[7] = (unsigned char)((mx_resp_len >> 8) & 0xff);
		modesCmdBlk[8] = (unsigned char)(mx_resp_len & 0xff);
	}

	memset(&io_hdr, 0, sizeof(sg_io_hdr_t));
	io_hdr.interface_id = 'S';
	io_hdr.cmd_len = mode6 ? MODE_SENSE6_CMDLEN : MODE_SENSE10_CMDLEN;
	io_hdr.mx_sb_len = sizeof(sense_b);
	io_hdr.dxfer_direction = SG_DXFER_FROM_DEV;
	io_hdr.dxfer_len = mx_resp_len;
	io_hdr.dxferp = resp;
	io_hdr.cmdp = modesCmdBlk;
	io_hdr.sbp = sense_b;
	io_hdr.timeout = DEF_TIMEOUT;

	if (ioctl(sg_fd, SG_IO, &io_hdr) < 0) {
		perror("SG_IO (mode sense) error");
		return -1;
	}
	res = sg_err_category3(&io_hdr);
	switch (res) {
	case SG_ERR_CAT_CLEAN:
	case SG_ERR_CAT_RECOVERED:
		return 0;
	default:
		if (noisy) {
			char ebuff[EBUFF_SZ];
			snprintf(ebuff, EBUFF_SZ, "Mode sense error, dbd=%d, "
				 "pc=%d, page_code=%x ", dbd, pc, pg_code);
			sg_chk_n_print3(ebuff, &io_hdr);
		}
		return -1;
	}
}

int do_scsi_send_diagnostics(char *device)
{
	int sg_fd, k, num, rsp_len;
	char *file_name = 0;
	unsigned char rsp_buff[MODE_ALLOC_LEN];
	int rsp_buff_size = MODE_ALLOC_LEN;
	int self_test_code = 6;
	int do_pf = 0;
	int do_doff = 0;
	int do_def_test = 0;
	int do_uoff = 0;
	int oflags = O_RDWR;

	print_msg(TEST_BREAK, __FUNCTION__);

	file_name = device;

	if ((sg_fd = open(file_name, oflags)) < 0) {
		snprintf(ebuff, EBUFF_SZ, ME "error opening file: %s",
			 file_name);
		perror(ebuff);
		return 1;
	}
	/* Just to be safe, check we have a new sg device by trying an ioctl */
	if ((ioctl(sg_fd, SG_GET_VERSION_NUM, &k) < 0) || (k < 30000)) {
		printf(ME "%s doesn't seem to be a version 3 sg device\n",
		       file_name);
		close(sg_fd);
		return 1;
	}

	if (0 == do_modes_0a(sg_fd, rsp_buff, 32, 1, 0)) {
		/* Assume mode sense(10) response without block descriptors */
		num = (rsp_buff[0] << 8) + rsp_buff[1] - 6;
		if (num >= 0xc) {
			int secs;

			secs = (rsp_buff[18] << 8) + rsp_buff[19];
			printf
			    ("Previous extended self-test duration=%d seconds "
			     "(%.2f minutes)\n", secs, secs / 60.0);
		} else
			printf("Extended self-test duration not available\n");
	} else
		printf("Extended self-test duration (mode page 0xa) failed\n");

	memset(rsp_buff, 0, sizeof(rsp_buff));
	if (0 == do_senddiag(sg_fd, 0, do_pf, 0, 0, 0, rsp_buff, 4, 1)) {
		if (0 == do_rcvdiag(sg_fd, 0, 0, rsp_buff, rsp_buff_size, 1)) {
			printf("Supported diagnostic pages response:\n");
			rsp_len = (rsp_buff[2] << 8) + rsp_buff[3] + 4;
			for (k = 0; k < (rsp_len - 4); ++k)
				printf("  %s\n",
				       find_page_code_desc(rsp_buff[k + 4], 0));
		}
	}

	if (0 == do_senddiag(sg_fd, self_test_code, do_pf, do_def_test,
			     do_doff, do_uoff, NULL, 0, 1)) {
		if ((5 == self_test_code) || (6 == self_test_code))
			printf("Foreground self test returned GOOD status\n");
		else if (do_def_test && (!do_doff) && (!do_uoff))
			printf("Default self test returned GOOD status\n");
	}
	close(sg_fd);
	return 0;
}

static void do_start_stop(int fd, int start, int immed, int loej,
			  int power_conditions)
{
	unsigned char cmdblk[6] = {
		START_STOP,	/* Command */
		0,		/* Resvd/Immed */
		0,		/* Reserved */
		0,		/* Reserved */
		0,		/* PowCond/Resvd/LoEj/Start */
		0
	};			/* Reserved/Flag/Link */
	unsigned char sense_b[32];
	sg_io_hdr_t io_hdr;
	int k, res, debug = 1;

	memset(&io_hdr, 0, sizeof(sg_io_hdr_t));
	cmdblk[1] = immed & 1;
	cmdblk[4] = ((power_conditions & 0xf) << 4) |
	    ((loej & 1) << 1) | (start & 1);
	io_hdr.interface_id = 'S';
	io_hdr.cmd_len = sizeof(cmdblk);
	io_hdr.mx_sb_len = sizeof(sense_b);
	io_hdr.dxfer_direction = SG_DXFER_NONE;
	io_hdr.dxfer_len = 0;
	io_hdr.dxferp = NULL;
	io_hdr.cmdp = cmdblk;
	io_hdr.sbp = sense_b;
	io_hdr.timeout = DEF_START_TIMEOUT;

	if (debug) {
		printf("  Start/Stop command:");
		for (k = 0; k < 6; ++k)
			printf(" %02x", cmdblk[k]);
		printf("\n");
	}

	if (ioctl(fd, SG_IO, &io_hdr) < 0) {
		perror("start_stop (SG_IO) error");
		return;
	}
	res = sg_err_category3(&io_hdr);
	if (SG_ERR_CAT_MEDIA_CHANGED == res) {
		fprintf(stderr, "media change report, try start_stop again\n");
		if (ioctl(fd, SG_IO, &io_hdr) < 0) {
			perror("start_stop (SG_IO) error");
			return;
		}
	}
	if (SG_ERR_CAT_CLEAN != res) {
		sg_chk_n_print3("start_stop", &io_hdr);
		return;
	}
	if (debug)
		fprintf(stderr, "start_stop [%s] successful\n",
			start ? "start" : "stop");
}

static void do_sync_cache(int fd)
{
	unsigned char cmdblk[10] = {
		SYNCHRONIZE_CACHE,	/* Command */
		0,		/* Immed (2) */
		0, 0, 0, 0,	/* LBA */
		0,		/* Reserved */
		0, 0,		/* No of blocks */
		0
	};			/* Reserved/Flag/Link */
	unsigned char sense_b[32];
	sg_io_hdr_t io_hdr;
	int res, debug = 1;

	memset(&io_hdr, 0, sizeof(sg_io_hdr_t));
	io_hdr.interface_id = 'S';
	io_hdr.cmd_len = sizeof(cmdblk);
	io_hdr.mx_sb_len = sizeof(sense_b);
	io_hdr.dxfer_direction = SG_DXFER_NONE;
	io_hdr.dxfer_len = 0;
	io_hdr.dxferp = NULL;
	io_hdr.cmdp = cmdblk;
	io_hdr.sbp = sense_b;
	io_hdr.timeout = DEF_START_TIMEOUT;

	if (ioctl(fd, SG_IO, &io_hdr) < 0) {
		perror("sync_cache (SG_IO) error");
		return;
	}
	res = sg_err_category3(&io_hdr);
	if (SG_ERR_CAT_MEDIA_CHANGED == res) {
		fprintf(stderr, "media change report, try sync_cache again\n");
		if (ioctl(fd, SG_IO, &io_hdr) < 0) {
			perror("sync_cache (SG_IO) error");
			return;
		}
	}
	if (SG_ERR_CAT_CLEAN != res) {
		sg_chk_n_print3("sync_cache", &io_hdr);
		return;
	}
	if (debug)
		fprintf(stderr, "synchronize cache successful\n");
}

int do_scsi_start_stop(char *device, int startstop)
{
	int synccache = 1;
	char *file_name = 0;
	int fd;
	int immed = 1;
	int loej = 0;
	int power_conds = 0;

	print_msg(TEST_BREAK, __FUNCTION__);

	file_name = device;

	fd = open(file_name, O_RDWR | O_NONBLOCK);
	if (fd < 0) {
		fprintf(stderr, "Error trying to open %s\n", file_name);
		perror("");
		usage();
		return 2;
	}
	if (ioctl(fd, SG_GET_TIMEOUT, 0) < 0) {
		fprintf(stderr, "Given file not block or SCSI "
			"generic device\n");
		close(fd);
		return 3;
	}

	if (synccache)
		do_sync_cache(fd);

	if (power_conds > 0)
		do_start_stop(fd, 0, immed, 0, power_conds);
	else if (startstop != -1)
		do_start_stop(fd, startstop, immed, loej, 0);

	close(fd);
	return 0;
}

int find_out_about_buffer(int sg_fd, int *buf_capacity, char *file_name)
{
	int res, buf_granul = 255;
	unsigned char *rbBuff = malloc(OFF + sizeof(rbCmdBlk) + 512);
	struct sg_header *rsghp = (struct sg_header *)rbBuff;
	int rbInLen = OFF + RB_DESC_LEN;
	int rbOutLen = OFF + sizeof(rbCmdBlk);
	unsigned char *buffp = rbBuff + OFF;
	rsghp->pack_len = 0;	/* don't care */
	rsghp->pack_id = 0;
	rsghp->reply_len = rbInLen;
	rsghp->twelve_byte = 0;
	rsghp->result = 0;
#ifndef SG_GET_RESERVED_SIZE
	rsghp->sense_buffer[0] = 0;
#endif
	memcpy(rbBuff + OFF, rbCmdBlk, sizeof(rbCmdBlk));
	rbBuff[OFF + 1] = RB_MODE_DESC;
	rbBuff[OFF + 8] = RB_DESC_LEN;

	res = write(sg_fd, rbBuff, rbOutLen);
	if (res < 0) {
		perror("sg_test_rwbuf: write (desc) error");
		if (rbBuff)
			free(rbBuff);
		return 1;
	}
	if (res < rbOutLen) {
		printf("sg_test_rwbuf: wrote less (desc), ask=%d, got=%d\n",
		       rbOutLen, res);
		if (rbBuff)
			free(rbBuff);
		return 1;
	}

	memset(rbBuff + OFF, 0, RB_DESC_LEN);
	res = read(sg_fd, rbBuff, rbInLen);
	if (res < 0) {
		perror("sg_test_rwbuf: read (desc) error");
		if (rbBuff)
			free(rbBuff);
		return 1;
	}
	if (res < rbInLen) {
		printf("sg_test_rwbuf: read less (desc), ask=%d, got=%d\n",
		       rbInLen, res);
		if (rbBuff)
			free(rbBuff);
		return 1;
	}
#ifdef SG_GET_RESERVED_SIZE
	if (!sg_chk_n_print("sg_test_rwbuf: desc", rsghp->target_status,
			    rsghp->host_status, rsghp->driver_status,
			    rsghp->sense_buffer, SG_MAX_SENSE)) {
		printf
		    ("sg_test_rwbuf: perhaps %s doesn't support READ BUFFER\n",
		     file_name);
		if (rbBuff)
			free(rbBuff);
		return 1;
	}
#else
	if ((rsghp->result != 0) || (0 != rsghp->sense_buffer[0])) {
		printf("sg_test_rwbuf: read(desc) result=%d\n", rsghp->result);
		if (0 != rsghp->sense_buffer[0])
			sg_print_sense("sg_test_rwbuf: desc",
				       rsghp->sense_buffer, SG_MAX_SENSE);
		printf
		    ("sg_test_rwbuf: perhaps %s doesn't support READ BUFFER\n",
		     file_name);
		if (rbBuff)
			free(rbBuff);
		return 1;
	}
#endif
	*(buf_capacity) = ((buffp[1] << 16) | (buffp[2] << 8) | buffp[3]);
	buf_granul = (unsigned char)buffp[0];

	printf("READ BUFFER reports: %02x %02x %02x %02x %02x %02x %02x %02x\n",
	       buffp[0], buffp[1], buffp[2], buffp[3],
	       buffp[4], buffp[5], buffp[6], buffp[7]);

	printf("READ BUFFER reports: buffer capacity=%d, offset boundary=%d\n",
	       *(buf_capacity), buf_granul);
#ifdef SG_DEF_RESERVED_SIZE
	res = ioctl(sg_fd, SG_SET_RESERVED_SIZE, buf_capacity);
	if (res < 0)
		perror("sg_test_rwbuf: SG_SET_RESERVED_SIZE error");
#endif
	return 0;
}

int mymemcmp(unsigned char *bf1, unsigned char *bf2, int len)
{
	int df;
	for (df = 0; df < len; df++)
		if (bf1[df] != bf2[df])
			return df;
	return 0;
}

int do_checksum(int *buf, int len, int quiet)
{
	int sum = base;
	int i;
	int rln = len;
	for (i = 0; i < len / BPI; i++)
		sum += buf[i];
	while (rln % BPI)
		sum += ((char *)buf)[--rln];
	if (sum != READWRITE_BASE_NUM) {
		if (!quiet)
			printf("sg_test_rwbuf: Checksum error (sz=%i): %08x\n",
			       len, sum);
		if (cmpbuf && !quiet) {
			int diff = mymemcmp(cmpbuf, (unsigned char *)buf, len);
			printf("Differ at pos %i/%i:\n", diff, len);
			for (i = 0; i < 24 && i + diff < len; i++)
				printf(" %02x", cmpbuf[i + diff]);
			printf("\n");
			for (i = 0; i < 24 && i + diff < len; i++)
				printf(" %02x",
				       ((unsigned char *)buf)[i + diff]);
			printf("\n");
		}
		return 2;
	} else
		return 0;
}

void do_fill_buffer(int *buf, int len)
{
	int sum;
	int i;
	int rln = len;
	srand(time(0));
retry:
	if (len >= BPI)
		base = READWRITE_BASE_NUM + rand();
	else
		base = READWRITE_BASE_NUM + (char)rand();
	sum = base;
	for (i = 0; i < len / BPI - 1; i++) {
		/* we rely on rand() giving full range of int */
		buf[i] = rand();
		sum += buf[i];
	}
	while (rln % BPI) {
		((char *)buf)[--rln] = rand();
		sum += ((char *)buf)[rln];
	}
	if (len >= BPI)
		buf[len / BPI - 1] = READWRITE_BASE_NUM - sum;
	else
		((char *)buf)[0] = READWRITE_BASE_NUM + ((char *)buf)[0] - sum;
	if (do_checksum(buf, len, 1)) {
		if (len < BPI)
			goto retry;
		printf("sg_test_rwbuf: Memory corruption?\n");
		exit(1);
	}
	if (cmpbuf)
		memcpy(cmpbuf, (char *)buf, len);
}

int read_buffer(int sg_fd, unsigned size)
{
	int res;
	unsigned char *rbBuff = malloc(OFF + sizeof(rbCmdBlk) + size);
	struct sg_header *rsghp = (struct sg_header *)rbBuff;

	int rbInLen = OFF + size;
	int rbOutLen = OFF + sizeof(rbCmdBlk);
	memset(rbBuff, 0, OFF + sizeof(rbCmdBlk) + size);
	rsghp->pack_len = 0;	/* don't care */
	rsghp->reply_len = rbInLen;
	rsghp->twelve_byte = 0;
	rsghp->result = 0;
	memcpy(rbBuff + OFF, rbCmdBlk, sizeof(rbCmdBlk));
	rbBuff[OFF + 1] = RB_MODE_DATA;
	rbBuff[OFF + 6] = 0xff & ((size) >> 16);
	rbBuff[OFF + 7] = 0xff & ((size) >> 8);
	rbBuff[OFF + 8] = 0xff & (size);

	rsghp->pack_id = 2;
	res = write(sg_fd, rbBuff, rbOutLen);
	if (res < 0) {
		perror("sg_test_rwbuf: write (data) error");
		if (rbBuff)
			free(rbBuff);
		return 1;
	}
	if (res < rbOutLen) {
		printf("sg_test_rwbuf: wrote less (data), ask=%d, got=%d\n",
		       rbOutLen, res);
		if (rbBuff)
			free(rbBuff);
		return 1;
	}

	res = read(sg_fd, rbBuff, rbInLen);
	if (res < 0) {
		perror("sg_test_rwbuf: read (data) error");
		if (rbBuff)
			free(rbBuff);
		return 1;
	}
	if (res < rbInLen) {
		printf("sg_test_rwbuf: read less (data), ask=%d, got=%d\n",
		       rbInLen, res);
		if (rbBuff)
			free(rbBuff);
		return 1;
	}
	res = do_checksum((int *)(rbBuff + OFF), size, 0);
	if (rbBuff)
		free(rbBuff);
	return res;
}

int write_buffer(int sg_fd, unsigned size)
{
	int res;
	unsigned char *rbBuff = malloc(OFF + sizeof(rbCmdBlk) + size);
	struct sg_header *rsghp = (struct sg_header *)rbBuff;
	//unsigned char * buffp = rbBuff + OFF;

	int rbInLen = OFF;
	int rbOutLen = OFF + sizeof(rbCmdBlk) + size;

	do_fill_buffer((int *)(rbBuff + OFF + sizeof(rbCmdBlk)), size);
	rsghp->pack_len = 0;	/* don't care */
	rsghp->reply_len = rbInLen;
	rsghp->twelve_byte = 0;
	rsghp->result = 0;
	memcpy(rbBuff + OFF, rbCmdBlk, sizeof(rbCmdBlk));
	rbBuff[OFF + 0] = WRITE_BUFFER;
	rbBuff[OFF + 1] = RB_MODE_DATA;
	rbBuff[OFF + 6] = 0xff & ((size) >> 16);
	rbBuff[OFF + 7] = 0xff & ((size) >> 8);
	rbBuff[OFF + 8] = 0xff & (size);

	rsghp->pack_id = 1;
	res = write(sg_fd, rbBuff, rbOutLen);
	if (res < 0) {
		perror("sg_test_rwbuf: write (data) error");
		if (rbBuff)
			free(rbBuff);
		return 1;
	}
	if (res < rbOutLen) {
		printf("sg_test_rwbuf: wrote less (data), ask=%d, got=%d\n",
		       rbOutLen, res);
		if (rbBuff)
			free(rbBuff);
		return 1;
	}

	res = read(sg_fd, rbBuff, rbInLen);
	if (res < 0) {
		perror("sg_test_rwbuf: read (status) error");
		if (rbBuff)
			free(rbBuff);
		return 1;
	}
	if (rbBuff)
		free(rbBuff);
	return 0;
}

int do_scsi_read_write_buffer(char *device)
{
	int sg_fd;
	int res, buf_capacity;
	char *file_name = device;
	struct stat a_st;
	int block_dev = 0;

	print_msg(TEST_BREAK, __FUNCTION__);

	sg_fd = open(file_name, O_RDWR);
	if (sg_fd < 0) {
		perror("sg_test_rwbuf: open error");
		return 1;
	}
	if (fstat(sg_fd, &a_st) < 0) {
		fprintf(stderr, "could do fstat() on fd ??\n");
		close(sg_fd);
		return 1;
	}
	if (S_ISBLK(a_st.st_mode))
		block_dev = 1;
	/* Don't worry, being very careful not to write to a none-sg file ... */
	if (block_dev || (ioctl(sg_fd, SG_GET_TIMEOUT, 0) < 0)) {
		/* perror("ioctl on generic device, error"); */
		printf("sg_test_rwbuf: not a sg device, or wrong driver\n");
		return 1;
	}
	if (find_out_about_buffer(sg_fd, &buf_capacity, file_name))
		return 1;

	cmpbuf = malloc(buf_capacity);
	if (write_buffer(sg_fd, buf_capacity))
		return 3;
	res = read_buffer(sg_fd, buf_capacity);
	if (res)
		return (res + 4);

	res = close(sg_fd);
	if (res < 0) {
		perror("sg_test_rwbuf: close error");
		return 6;
	}
	printf("Success\n");
	return 0;
}

int do_scsi_test_unit_ready(char *device)
{
	int sg_fd, k;
	unsigned char turCmdBlk[TUR_CMD_LEN] = { 0x00, 0, 0, 0, 0, 0 };
	sg_io_hdr_t io_hdr;
	char *file_name = device;
	char ebuff[EBUFF_SZ];
	unsigned char sense_buffer[32];
	int num_turs = 10240;
	int num_errs = 0;
	int do_time = 1;
	struct timeval start_tm, end_tm;

	print_msg(TEST_BREAK, __FUNCTION__);

	if ((sg_fd = open(file_name, O_RDONLY)) < 0) {
		snprintf(ebuff, EBUFF_SZ,
			 "sg_turs: error opening file: %s", file_name);
		perror(ebuff);
		return 1;
	}
	/* Just to be safe, check we have a new sg driver by trying an ioctl */
	if ((ioctl(sg_fd, SG_GET_VERSION_NUM, &k) < 0) || (k < 30000)) {
		printf
		    ("sg_turs: %s isn't an sg device (or the sg driver is old)\n",
		     file_name);
		close(sg_fd);
		return 1;
	}
	/* Prepare TEST UNIT READY command */
	memset(&io_hdr, 0, sizeof(sg_io_hdr_t));
	io_hdr.interface_id = 'S';
	io_hdr.cmd_len = sizeof(turCmdBlk);
	io_hdr.mx_sb_len = sizeof(sense_buffer);
	io_hdr.dxfer_direction = SG_DXFER_NONE;
	io_hdr.cmdp = turCmdBlk;
	io_hdr.sbp = sense_buffer;
	io_hdr.timeout = 20000;	/* 20000 millisecs == 20 seconds */
	if (do_time) {
		start_tm.tv_sec = 0;
		start_tm.tv_usec = 0;
		gettimeofday(&start_tm, NULL);
	}
	for (k = 0; k < num_turs; ++k) {
		io_hdr.pack_id = k;
		if (ioctl(sg_fd, SG_IO, &io_hdr) < 0) {
			perror("sg_turs: Test Unit Ready SG_IO ioctl error");
			close(sg_fd);
			return 1;
		}
		if (io_hdr.info & SG_INFO_OK_MASK) {
			++num_errs;
			if (1 == num_turs) {	/* then print out the error message */
				if (SG_ERR_CAT_CLEAN !=
				    sg_err_category3(&io_hdr))
					sg_chk_n_print3("tur", &io_hdr);
			}
		}
	}
	if ((do_time) && (start_tm.tv_sec || start_tm.tv_usec)) {
		struct timeval res_tm;
		double a, b;

		gettimeofday(&end_tm, NULL);
		res_tm.tv_sec = end_tm.tv_sec - start_tm.tv_sec;
		res_tm.tv_usec = end_tm.tv_usec - start_tm.tv_usec;
		if (res_tm.tv_usec < 0) {
			--res_tm.tv_sec;
			res_tm.tv_usec += 1000000;
		}
		a = res_tm.tv_sec;
		a += (0.000001 * res_tm.tv_usec);
		b = (double)num_turs;
		printf("time to perform commands was %d.%06d secs",
		       (int)res_tm.tv_sec, (int)res_tm.tv_usec);
		if (a > 0.00001)
			printf("; %.2f operations/sec\n", b / a);
		else
			printf("\n");
	}

	printf("Completed %d Test Unit Ready commands with %d errors\n",
	       num_turs, num_errs);
	close(sg_fd);
	return 0;
}

/* Returns 0 -> ok, 1 -> err, 2 -> recovered error */
static int do_sg_io(int sg_fd, unsigned char *buff)
{
/* N.B. Assuming buff contains pointer 'buffer' or 'buffer1' */
	struct sg_header *sghp = (struct sg_header *)(buff - OFF);
	int res;

	sghp->pack_len = 0;
	sghp->reply_len = SG_HSZ + *(((int *)buff) + 1);
	sghp->pack_id = 0;
	sghp->twelve_byte = 0;
	sghp->other_flags = 0;
#ifndef SG_GET_RESERVED_SIZE
	sghp->sense_buffer[0] = 0;
#endif
#if 0
	sg_print_command(buff + 8);
	printf(" write_len=%d, read_len=%d\n",
	       SG_HSZ + sg_get_command_size(buff[8]) + *((int *)buff),
	       sghp->reply_len);
#endif
	res = write(sg_fd, (const void *)sghp,
		    SG_HSZ + sg_get_command_size(buff[8]) + *((int *)buff));
	if (res < 0) {
#ifdef SG_IO_DEBUG
		perror("write to sg failed");
#endif
		return 1;
	}
	res = read(sg_fd, (void *)sghp, sghp->reply_len);
	if (res < 0) {
#ifdef SG_IO_DEBUG
		perror("read from sg failed");
#endif
		return 1;
	}
#ifdef SG_GET_RESERVED_SIZE
	res = sg_err_category(sghp->target_status, sghp->host_status,
			      sghp->driver_status, sghp->sense_buffer,
			      SG_MAX_SENSE);
	switch (res) {
	case SG_ERR_CAT_CLEAN:
		return 0;
	case SG_ERR_CAT_RECOVERED:
		return 2;
	default:
#ifdef SG_IO_DEBUG
		sg_chk_n_print("read from sg", sghp->target_status,
			       sghp->host_status, sghp->driver_status,
			       sghp->sense_buffer, SG_MAX_SENSE);
#endif
		return 1;
	}
#else
	if (0 != sghp->sense_buffer[0]) {
#ifdef SG_IO_DEBUG
		int k;
		printf("read from sg, sense buffer (in hex):\n    ");
		for (k = 0; k < 16; ++k)
			printf("%02x ", (int)sghp->sense_buffer[k]);
		printf("\n");
#endif
		return 1;
	} else if (0 != sghp->result) {
#ifdef SG_IO_DEBUG
		printf("read from sg, bad result=%d\n", sghp->result);
#endif
		return 1;
	} else
		return 0;
#endif
}

static char *get_page_name(int pageno)
{
	if ((pageno <= 0) || (pageno >= MAX_PAGENO) || (!page_names[pageno]))
		return "Mode";
	return page_names[pageno];
}

static int getnbyte(unsigned char *pnt, int nbyte)
{
	unsigned int result;
	int i;
	result = 0;
	for (i = 0; i < nbyte; i++)
		result = (result << 8) | (pnt[i] & 0xff);
	return result;
}

static void bitfield(unsigned char *pageaddr, char *text, int mask, int shift)
{
	printf("%-35s%d\n", text, (*pageaddr >> shift) & mask);
}

static void notbitfield(unsigned char *pageaddr, char *text, int mask,
			int shift)
{
	printf("%-35s%d\n", text, !((*pageaddr >> shift) & mask));
}

static void intfield(unsigned char *pageaddr, int nbytes, char *text)
{
	printf("%-35s%d\n", text, getnbyte(pageaddr, nbytes));
}

static void hexfield(unsigned char *pageaddr, int nbytes, char *text)
{
	printf("%-35s0x%x\n", text, getnbyte(pageaddr, nbytes));
}

static void hexdatafield(unsigned char *pageaddr, int nbytes, char *text)
{
	printf("%-35s0x", text);
	while (nbytes-- > 0)
		printf("%02x", *pageaddr++);
	putchar('\n');
}

static int get_mode_page(int page, int page_code)
{
	int status, quiet;
	unsigned char *cmd;

	memset(buffer, 0, SIZEOF_BUFFER);

	quiet = page_code & ~3;
	page_code &= 3;

	*((int *)buffer) = 0;	/* length of input data */
	*(((int *)buffer) + 1) = 0xff;	/* length of output data */

	cmd = (unsigned char *)(((int *)buffer) + 2);

	cmd[0] = MODE_SENSE;	/* MODE SENSE (6) */
	cmd[1] = 0x00;		/* lun = 0, inhibitting BD makes this fail
				   for me */
	cmd[2] = (page_code << 6) | page;
	cmd[3] = 0x00;		/* (reserved) */
	cmd[4] = (unsigned char)0xff;	/* allocation length */
	cmd[5] = 0x00;		/* control */

	status = do_sg_io(glob_fd, buffer);
	if (status && (!quiet))
		fprintf(stdout, ">>> Unable to read %s Page %02xh\n",
			get_page_name(page), page);
	//dump (buffer+2, 46);
	return status;
}

/* Same as above, but this time with MODE_SENSE_10 */
static int get_mode_page10(int page, int page_code)
{
	int status, quiet;
	unsigned char *cmd;

	memset(buffer, 0, SIZEOF_BUFFER);

	quiet = page_code & ~3;
	page_code &= 3;

	*((int *)buffer) = 0;	/* length of input data */
	*(((int *)buffer) + 1) = 0xffff;	/* length of output buffer */

	cmd = (unsigned char *)(((int *)buffer) + 2);

	cmd[0] = MODE_SENSE_10;	/* MODE SENSE (10) */
	cmd[1] = 0x00;		/* lun = 0, inhibitting BD makes this fail
				   for me */
	cmd[2] = (page_code << 6) | page;
	cmd[3] = 0x00;		/* (reserved) */
	cmd[4] = 0x00;		/* (reserved) */
	cmd[5] = 0x00;		/* (reserved) */
	cmd[6] = 0x00;		/* (reserved) */
	cmd[7] = 0xff;		/* allocation length hi */
	cmd[8] = 0xff;		/* allocation length lo */
	cmd[9] = 0x00;		/* control */

	status = do_sg_io(glob_fd, buffer);
	if (status && (!quiet))
		fprintf(stdout,
			">>> Unable to read %s Page %02xh with MODESENSE(10)\n",
			get_page_name(page), page);
	return status;
}

/* Contents should point to the mode parameter header that we obtained
   in a prior read operation.  This way we do not have to work out the
   format of the beast */

static int read_geometry(int page_code)
{
	int status;
	int bdlen;
	unsigned char *pagestart;

	SETUP_MODE_PAGE(4, 9);

	printf("Data from Rigid Disk Drive Geometry Page\n");
	printf("----------------------------------------\n");
	intfield(pagestart + 2, 3, "Number of cylinders");
	intfield(pagestart + 5, 1, "Number of heads");
	intfield(pagestart + 6, 3, "Starting write precomp");
	intfield(pagestart + 9, 3, "Starting reduced current");
	intfield(pagestart + 12, 2, "Drive step rate");
	intfield(pagestart + 14, 3, "Landing Zone Cylinder");
	bitfield(pagestart + 17, "RPL", 3, 0);
	intfield(pagestart + 18, 1, "Rotational Offset");
	intfield(pagestart + 20, 2, "Rotational Rate");
	printf("\n");
	return 0;

}

static int read_disconnect_reconnect_data(int page_code)
{
	int status;
	int bdlen;
	unsigned char *pagestart;

	SETUP_MODE_PAGE(2, 7);

	printf("Data from Disconnect-Reconnect Page\n");
	printf("-----------------------------------\n");
	intfield(pagestart + 2, 1, "Buffer full ratio");
	intfield(pagestart + 3, 1, "Buffer empty ratio");
	intfield(pagestart + 4, 2, "Bus Inactivity Limit");
	intfield(pagestart + 6, 2, "Disconnect Time Limit");
	intfield(pagestart + 8, 2, "Connect Time Limit");
	intfield(pagestart + 10, 2, "Maximum Burst Size");
	hexfield(pagestart + 12, 1, "DTDC");
	printf("\n");
	return 0;

}

static int read_control_page(int page_code)
{
	int status;
	int bdlen;
	unsigned char *pagestart;

	SETUP_MODE_PAGE(10, 9);

	printf("Data from Control Page\n");
	printf("----------------------\n");
	bitfield(pagestart + 2, "RLEC", 1, 0);
	bitfield(pagestart + 3, "QErr", 1, 1);
	bitfield(pagestart + 3, "DQue", 1, 0);
	bitfield(pagestart + 4, "EECA", 1, 7);
	bitfield(pagestart + 4, "RAENP", 1, 2);
	bitfield(pagestart + 4, "UUAENP", 1, 1);
	bitfield(pagestart + 4, "EAENP", 1, 0);
	bitfield(pagestart + 3, "Queue Algorithm Modifier", 0xf, 4);
	intfield(pagestart + 6, 2, "Ready AEN Holdoff Period");
	printf("\n");
	return 0;

}

static int error_recovery_page(int page_code)
{
	int status;
	int bdlen;
	unsigned char *pagestart;

	SETUP_MODE_PAGE(1, 14);
	printf("Data from Error Recovery Page\n");
	printf("-----------------------------\n");
	bitfield(pagestart + 2, "AWRE", 1, 7);
	bitfield(pagestart + 2, "ARRE", 1, 6);
	bitfield(pagestart + 2, "TB", 1, 5);
	bitfield(pagestart + 2, "RC", 1, 4);
	bitfield(pagestart + 2, "EER", 1, 3);
	bitfield(pagestart + 2, "PER", 1, 2);
	bitfield(pagestart + 2, "DTE", 1, 1);
	bitfield(pagestart + 2, "DCR", 1, 0);
	intfield(pagestart + 3, 1, "Read Retry Count");
	intfield(pagestart + 4, 1, "Correction Span");
	intfield(pagestart + 5, 1, "Head Offset Count");
	intfield(pagestart + 6, 1, "Data Strobe Offset Count");
	intfield(pagestart + 8, 1, "Write Retry Count");
	intfield(pagestart + 10, 2, "Recovery Time Limit");
	printf("\n");
	return 0;
}

static int notch_parameters_page(int page_code)
{
	int status;
	int bdlen;
	unsigned char *pagestart;

	SETUP_MODE_PAGE(0xc, 7);

	printf("Data from Notch Parameters Page\n");
	printf("-------------------------------\n");
	bitfield(pagestart + 2, "Notched Drive", 1, 7);
	bitfield(pagestart + 2, "Logical or Physical Notch", 1, 6);
	intfield(pagestart + 4, 2, "Max # of notches");
	intfield(pagestart + 6, 2, "Active Notch");
	if (pagestart[2] & 0x40) {
		intfield(pagestart + 8, 4, "Starting Boundary");
		intfield(pagestart + 12, 4, "Ending Boundary");
	} else {		/* Hex is more meaningful for physical notches */
		hexfield(pagestart + 8, 4, "Starting Boundary");
		hexfield(pagestart + 12, 4, "Ending Boundary");
	}

	printf("0x%8.8x%8.8x", getnbyte(pagestart + 16, 4),
	       getnbyte(pagestart + 20, 4));

	printf("\n");
	return 0;
}

static char *formatname(int format)
{
	switch (format) {
	case 0x0:
		return "logical blocks";
	case 0x4:
		return "bytes from index [Cyl:Head:Off]\n"
		    "Offset -1 marks whole track as bad.\n";
	case 0x5:
		return "physical blocks [Cyl:Head:Sect]\n"
		    "Sector -1 marks whole track as bad.\n";
	}
	return "Weird, unknown format";
}

static int read_defect_list(int page_code)
{
	int status = 0, i, len, reallen, table, k;
	unsigned char *cmd, *df = 0;
	int trunc;

	printf("Data from Defect Lists\n" "----------------------\n");
	for (table = 0; table < 2; table++) {
		memset(buffer, 0, SIZEOF_BUFFER);
		trunc = 0;

		*((int *)buffer) = 0;	/* length of input data */
		*(((int *)buffer) + 1) = 4;	/* length of output buffer */

		cmd = (unsigned char *)(((int *)buffer) + 2);

		cmd[0] = 0x37;	/* READ DEFECT DATA */
		cmd[1] = 0x00;	/* lun=0 */
		cmd[2] = (table ? 0x08 : 0x10) | defectformat;	/*  List, Format */
		cmd[3] = 0x00;	/* (reserved) */
		cmd[4] = 0x00;	/* (reserved) */
		cmd[5] = 0x00;	/* (reserved) */
		cmd[6] = 0x00;	/* (reserved) */
		cmd[7] = 0x00;	/* Alloc len */
		cmd[8] = 0x04;	/* Alloc len */
		cmd[9] = 0x00;	/* control */

		i = do_sg_io(glob_fd, buffer);
		if (2 == i)
			i = 0;	/* Recovered error, probably returned a different
				   format */
		if (i) {
			fprintf(stdout, ">>> Unable to read %s defect data.\n",
				(table ? "grown" : "manufacturer"));
			status |= i;
			continue;
		}
		len = (buffer[10] << 8) | buffer[11];
		reallen = len;
		if (len > 0) {
			if (len >= 0xfff8) {
				len = SIZEOF_BUFFER - 8;
				k = len + 8;	/* length of defect list */
				*((int *)buffer) = 0;	/* length of input data */
				*(((int *)buffer) + 1) = k;	/* length of output buffer */
				((struct sg_header *)buffer)->twelve_byte = 1;
				cmd[0] = 0xB7;	/* READ DEFECT DATA */
				cmd[1] = (table ? 0x08 : 0x10) | defectformat;	/*  List, Format */
				cmd[2] = 0x00;	/* (reserved) */
				cmd[3] = 0x00;	/* (reserved) */
				cmd[4] = 0x00;	/* (reserved) */
				cmd[5] = 0x00;	/* (reserved) */
				cmd[6] = 0x00;	/* Alloc len */
				cmd[7] = (k >> 16);	/* Alloc len */
				cmd[8] = (k >> 8);	/* Alloc len */
				cmd[9] = (k & 0xff);	/* Alloc len */
				cmd[10] = 0x00;	/* reserved */
				cmd[11] = 0x00;	/* control */
				i = do_sg_io(glob_fd, buffer);
				if (i == 2)
					i = 0;
				if (i)
					goto trytenbyte;
				reallen =
				    (buffer[12] << 24 | buffer[13] << 16 |
				     buffer[14] << 8 | buffer[15]);
				len = reallen;
				if (len > SIZEOF_BUFFER - 8) {
					len = SIZEOF_BUFFER - 8;
					trunc = 1;
				}
				df = (unsigned char *)(buffer + 16);
			} else {
trytenbyte:
				if (len > 0xfff8) {
					len = 0xfff8;
					trunc = 1;
				}
				k = len + 4;	/* length of defect list */
				*((int *)buffer) = 0;	/* length of input data */
				*(((int *)buffer) + 1) = k;	/* length of output buffer */
				cmd[0] = 0x37;	/* READ DEFECT DATA */
				cmd[1] = 0x00;	/* lun=0 */
				cmd[2] = (table ? 0x08 : 0x10) | defectformat;	/*  List, Format */
				cmd[3] = 0x00;	/* (reserved) */
				cmd[4] = 0x00;	/* (reserved) */
				cmd[5] = 0x00;	/* (reserved) */
				cmd[6] = 0x00;	/* (reserved) */
				cmd[7] = (k >> 8);	/* Alloc len */
				cmd[8] = (k & 0xff);	/* Alloc len */
				cmd[9] = 0x00;	/* control */
				i = do_sg_io(glob_fd, buffer);
				df = (unsigned char *)(buffer + 12);
			}
		}
		if (2 == i)
			i = 0;	/* Recovered error, probably returned a different
				   format */
		if (i) {
			fprintf(stdout, ">>> Unable to read %s defect data.\n",
				(table ? "grown" : "manufacturer"));
			status |= i;
			continue;
		} else {
			if (table && !status)
				printf("\n");
			printf("%d entries (%d bytes) in %s table.\n"
			       "Format (%x) is: %s\n",
			       reallen / ((buffer[9] & 7) ? 8 : 4), reallen,
			       (table ? "grown" : "manufacturer"),
			       buffer[9] & 7, formatname(buffer[9] & 7));
			i = 0;
			if ((buffer[9] & 7) == 4) {
				while (len > 0) {
					snprintf((char *)buffer, 40,
						 "%6d:%3u:%8d", getnbyte(df, 3),
						 df[3], getnbyte(df + 4, 4));
					printf("%19s", (char *)buffer);
					len -= 8;
					df += 8;
					i++;
					if (i >= 4) {
						printf("\n");
						i = 0;
					} else
						printf("|");
				}
			} else if ((buffer[9] & 7) == 5) {
				while (len > 0) {
					snprintf((char *)buffer, 40,
						 "%6d:%2u:%5d", getnbyte(df, 3),
						 df[3], getnbyte(df + 4, 4));
					printf("%15s", (char *)buffer);
					len -= 8;
					df += 8;
					i++;
					if (i >= 5) {
						printf("\n");
						i = 0;
					} else
						printf("|");
				}
			} else {
				while (len > 0) {
					printf("%10d", getnbyte(df, 4));
					len -= 4;
					df += 4;
					i++;
					if (i >= 7) {
						printf("\n");
						i = 0;
					} else
						printf("|");
				}
			}
			if (i)
				printf("\n");
		}
		if (trunc)
			printf("[truncated]\n");
	}
	printf("\n");
	return status;
}

static int read_cache(int page_code)
{
	int status;
	int bdlen;
	unsigned char *pagestart;

	SETUP_MODE_PAGE(8, 9);

	printf("Data from Caching Page\n");
	printf("----------------------\n");
	bitfield(pagestart + 2, "Write Cache", 1, 2);
	notbitfield(pagestart + 2, "Read Cache", 1, 0);
	bitfield(pagestart + 2, "Prefetch units", 1, 1);
	bitfield(pagestart + 3, "Demand Read Retention Priority", 0xf, 4);
	bitfield(pagestart + 3, "Demand Write Retention Priority", 0xf, 0);
	intfield(pagestart + 4, 2, "Disable Pre-fetch Transfer Length");
	intfield(pagestart + 6, 2, "Minimum Pre-fetch");
	intfield(pagestart + 8, 2, "Maximum Pre-fetch");
	intfield(pagestart + 10, 2, "Maximum Pre-fetch Ceiling");
	printf("\n");
	return 0;
}

static int read_format_info(int page_code)
{
	int status;
	int bdlen;
	unsigned char *pagestart;

	SETUP_MODE_PAGE(3, 13);

	printf("Data from Format Device Page\n");
	printf("----------------------------\n");
	bitfield(pagestart + 20, "Removable Medium", 1, 5);
	bitfield(pagestart + 20, "Supports Hard Sectoring", 1, 6);
	bitfield(pagestart + 20, "Supports Soft Sectoring", 1, 7);
	bitfield(pagestart + 20, "Addresses assigned by surface", 1, 4);
	intfield(pagestart + 2, 2, "Tracks per Zone");
	intfield(pagestart + 4, 2, "Alternate sectors per zone");
	intfield(pagestart + 6, 2, "Alternate tracks per zone");
	intfield(pagestart + 8, 2, "Alternate tracks per lun");
	intfield(pagestart + 10, 2, "Sectors per track");
	intfield(pagestart + 12, 2, "Bytes per sector");
	intfield(pagestart + 14, 2, "Interleave");
	intfield(pagestart + 16, 2, "Track skew factor");
	intfield(pagestart + 18, 2, "Cylinder skew factor");
	printf("\n");
	return 0;

}

static int verify_error_recovery(int page_code)
{
	int status;
	int bdlen;
	unsigned char *pagestart;

	SETUP_MODE_PAGE(7, 7);

	printf("Data from Verify Error Recovery Page\n");
	printf("------------------------------------\n");
	bitfield(pagestart + 2, "EER", 1, 3);
	bitfield(pagestart + 2, "PER", 1, 2);
	bitfield(pagestart + 2, "DTE", 1, 1);
	bitfield(pagestart + 2, "DCR", 1, 0);
	intfield(pagestart + 3, 1, "Verify Retry Count");
	intfield(pagestart + 4, 1, "Verify Correction Span (bits)");
	intfield(pagestart + 10, 2, "Verify Recovery Time Limit (ms)");

	printf("\n");
	return 0;
}

static int peripheral_device_page(int page_code)
{
	static char *idents[] = {
		"X3.131: Small Computer System Interface",
		"X3.91M-1987: Storage Module Interface",
		"X3.170: Enhanced Small Device Interface",
		"X3.130-1986; X3T9.3/87-002: IPI-2",
		"X3.132-1987; X3.147-1988: IPI-3"
	};
	int status;
	int bdlen;
	unsigned ident;
	unsigned char *pagestart;
	char *name;

	SETUP_MODE_PAGE(9, 2);

	printf("Data from Peripheral Device Page\n");
	printf("--------------------------------\n");

	ident = getnbyte(pagestart + 2, 2);
	if (ident < (sizeof(idents) / sizeof(char *)))
		name = idents[ident];
	else if (ident < 0x8000)
		name = "Reserved";
	else
		name = "Vendor Specific";

	bdlen = pagestart[1] - 6;
	if (bdlen < 0)
		bdlen = 0;
	else
		SETUP_MODE_PAGE(9, 2);

	hexfield(pagestart + 2, 2, "Interface Identifier");
	for (ident = 0; ident < 35; ident++)
		putchar(' ');
	puts(name);

	hexdatafield(pagestart + 8, bdlen, "Vendor Specific Data");

	printf("\n");
	return 0;
}

/*  end  */

static int do_user_page(int page_code, int page_no)
{
	int status;
	int bdlen;
	int i;
	//unsigned ident;
	unsigned char *pagestart;
	char *name;

	SETUP_MODE_PAGE(page_no, 0);
	//printf ("Page 0x%02x len: %i\n", page_code, pagestart[1]);

	name = "Vendor specific";
	for (i = 2; i < pagestart[1] + 2; i++) {
		char nm[8];
		snprintf(nm, 8, "%02x", i);
		hexdatafield(pagestart + i, 1, nm);
	}

	printf("\n");
	puts(name);
	return 0;
}

/*  end  */

static int do_scsi_info_inquiry(int page_code)
{
	int status, i, x_interface = 0;
	unsigned char *cmd;
	unsigned char *pagestart;
	unsigned char tmp;

	for (i = 0; i < 1024; i++) {
		buffer[i] = 0;
	}

	*((int *)buffer) = 0;	/* length of input data */
	*(((int *)buffer) + 1) = 36;	/* length of output buffer */

	cmd = (unsigned char *)(((int *)buffer) + 2);

	cmd[0] = 0x12;		/* INQUIRY */
	cmd[1] = 0x00;		/* lun=0, evpd=0 */
	cmd[2] = 0x00;		/* page code = 0 */
	cmd[3] = 0x00;		/* (reserved) */
	cmd[4] = 0x24;		/* allocation length */
	cmd[5] = 0x00;		/* control */

	status = do_sg_io(glob_fd, buffer);
	if (status) {
		printf("Error doing INQUIRY (1)");
		return status;
	}

	pagestart = buffer + 8;

	printf("Inquiry command\n");
	printf("---------------\n");
	bitfield(pagestart + 7, "Relative Address", 1, 7);
	bitfield(pagestart + 7, "Wide bus 32", 1, 6);
	bitfield(pagestart + 7, "Wide bus 16", 1, 5);
	bitfield(pagestart + 7, "Synchronous neg.", 1, 4);
	bitfield(pagestart + 7, "Linked Commands", 1, 3);
	bitfield(pagestart + 7, "Command Queueing", 1, 1);
	bitfield(pagestart + 7, "SftRe", 1, 0);
	bitfield(pagestart + 0, "Device Type", 0x1f, 0);
	bitfield(pagestart + 0, "Peripheral Qualifier", 0x7, 5);
	bitfield(pagestart + 1, "Removable?", 1, 7);
	bitfield(pagestart + 1, "Device Type Modifier", 0x7f, 0);
	bitfield(pagestart + 2, "ISO Version", 3, 6);
	bitfield(pagestart + 2, "ECMA Version", 7, 3);
	bitfield(pagestart + 2, "ANSI Version", 7, 0);
	bitfield(pagestart + 3, "AENC", 1, 7);
	bitfield(pagestart + 3, "TrmIOP", 1, 6);
	bitfield(pagestart + 3, "Response Data Format", 0xf, 0);
	tmp = pagestart[16];
	pagestart[16] = 0;
	printf("%s%s\n", (!x_interface ? "Vendor:                    " : ""),
	       pagestart + 8);
	pagestart[16] = tmp;

	tmp = pagestart[32];
	pagestart[32] = 0;
	printf("%s%s\n", (!x_interface ? "Product:                   " : ""),
	       pagestart + 16);
	pagestart[32] = tmp;

	printf("%s%s\n", (!x_interface ? "Revision level:            " : ""),
	       pagestart + 32);

	printf("\n");
	return status;

}

static int do_serial_number(int page_code)
{
	int status, i, pagelen;
	unsigned char *cmd;
	unsigned char *pagestart;

	for (i = 0; i < 1024; i++) {
		buffer[i] = 0;
	}

	*((int *)buffer) = 0;	/* length of input data */
	*(((int *)buffer) + 1) = 4;	/* length of output buffer */

	cmd = (unsigned char *)(((int *)buffer) + 2);

	cmd[0] = 0x12;		/* INQUIRY */
	cmd[1] = 0x01;		/* lun=0, evpd=1 */
	cmd[2] = 0x80;		/* page code = 0x80, serial number */
	cmd[3] = 0x00;		/* (reserved) */
	cmd[4] = 0x04;		/* allocation length */
	cmd[5] = 0x00;		/* control */

	status = do_sg_io(glob_fd, buffer);
	if (status) {
		printf("Error doing INQUIRY (evpd=1, serial number)\n");
		return status;
	}

	pagestart = buffer + 8;

	pagelen = 4 + pagestart[3];
	*((int *)buffer) = 0;	/* length of input data */
	*(((int *)buffer) + 1) = pagelen;	/* length of output buffer */

	cmd[0] = 0x12;		/* INQUIRY */
	cmd[1] = 0x01;		/* lun=0, evpd=1 */
	cmd[2] = 0x80;		/* page code = 0x80, serial number */
	cmd[3] = 0x00;		/* (reserved) */
	cmd[4] = (unsigned char)pagelen;	/* allocation length */
	cmd[5] = 0x00;		/* control */

	status = do_sg_io(glob_fd, buffer);
	if (status) {
		printf("Error doing INQUIRY (evpd=1, serial number, len)\n");
		return status;
	}

	printf("Serial Number '");
	for (i = 0; i < pagestart[3]; i++)
		printf("%c", pagestart[4 + i]);
	printf("'\n");
	printf("\n");

	return status;
}

/* Print out a list of the known devices on the system */
static void show_devices()
{
	int k, j, fd, err, bus;
	My_scsi_idlun m_idlun;
	char name[MDEV_NAME_SZ];
	char ebuff[EBUFF_SZ];
	int do_numeric = 1;
	int max_holes = MAX_HOLES;

	for (k = 0, j = 0; k < sizeof(devices) / sizeof(char *); k++) {
		fd = open(devices[k], O_RDONLY | O_NONBLOCK);
		if (fd < 0)
			continue;
		err =
		    ioctl(fd, SCSI_IOCTL_GET_BUS_NUMBER, &(sg_map_arr[j].bus));
		if (err < 0) {
			snprintf(ebuff, EBUFF_SZ,
				 "SCSI(1) ioctl on %s failed", devices[k]);
			perror(ebuff);
			close(fd);
			continue;
		}
		err = ioctl(fd, SCSI_IOCTL_GET_IDLUN, &m_idlun);
		if (err < 0) {
			snprintf(ebuff, EBUFF_SZ,
				 "SCSI(2) ioctl on %s failed", devices[k]);
			perror(ebuff);
			close(fd);
			continue;
		}
		sg_map_arr[j].channel = (m_idlun.dev_id >> 16) & 0xff;
		sg_map_arr[j].lun = (m_idlun.dev_id >> 8) & 0xff;
		sg_map_arr[j].target_id = m_idlun.dev_id & 0xff;
		sg_map_arr[j].dev_name = devices[k];

		printf("[scsi%d ch=%d id=%d lun=%d %s] ", sg_map_arr[j].bus,
		       sg_map_arr[j].channel, sg_map_arr[j].target_id,
		       sg_map_arr[j].lun, sg_map_arr[j].dev_name);

		++j;
		printf("%s ", devices[k]);
		close(fd);
	};
	printf("\n");
	for (k = 0; k < MAX_SG_DEVS; k++) {
		make_dev_name(name, NULL, k, do_numeric);
		fd = open(name, O_RDWR | O_NONBLOCK);
		if (fd < 0) {
			if ((ENOENT == errno) && (0 == k)) {
				do_numeric = 0;
				make_dev_name(name, NULL, k, do_numeric);
				fd = open(name, O_RDWR | O_NONBLOCK);
			}
			if (fd < 0) {
				if (EBUSY == errno)
					continue;	/* step over if O_EXCL already on it */
				else {
#if 0
					snprintf(ebuff, EBUFF_SZ,
						 "open on %s failed (%d)", name,
						 errno);
					perror(ebuff);
#endif
					if (max_holes-- > 0)
						continue;
					else
						break;
				}
			}
		}
		max_holes = MAX_HOLES;
		err = ioctl(fd, SCSI_IOCTL_GET_BUS_NUMBER, &bus);
		if (err < 0) {
			snprintf(ebuff, EBUFF_SZ, "SCSI(3) ioctl on %s failed",
				 name);
			perror(ebuff);
			close(fd);
			continue;
		}
		err = ioctl(fd, SCSI_IOCTL_GET_IDLUN, &m_idlun);
		if (err < 0) {
			snprintf(ebuff, EBUFF_SZ, "SCSI(3) ioctl on %s failed",
				 name);
			perror(ebuff);
			close(fd);
			continue;
		}

		printf("[scsi%d ch=%d id=%d lun=%d %s]", bus,
		       (m_idlun.dev_id >> 16) & 0xff, m_idlun.dev_id & 0xff,
		       (m_idlun.dev_id >> 8) & 0xff, name);

		for (j = 0; sg_map_arr[j].dev_name; ++j) {
			if ((bus == sg_map_arr[j].bus) &&
			    ((m_idlun.dev_id & 0xff) == sg_map_arr[j].target_id)
			    && (((m_idlun.dev_id >> 16) & 0xff) ==
				sg_map_arr[j].channel)
			    && (((m_idlun.dev_id >> 8) & 0xff) ==
				sg_map_arr[j].lun)) {
				printf("%s [=%s  scsi%d ch=%d id=%d lun=%d]\n",
				       name, sg_map_arr[j].dev_name, bus,
				       ((m_idlun.dev_id >> 16) & 0xff),
				       m_idlun.dev_id & 0xff,
				       ((m_idlun.dev_id >> 8) & 0xff));
				break;
			}
		}
		if (NULL == sg_map_arr[j].dev_name)
			printf("%s [scsi%d ch=%d id=%d lun=%d]\n", name, bus,
			       ((m_idlun.dev_id >> 16) & 0xff),
			       m_idlun.dev_id & 0xff,
			       ((m_idlun.dev_id >> 8) & 0xff));
		close(fd);
	}
	printf("\n");
}

static int show_pages(int page_code)
{
	int offset;
	int length;
	int i;
	unsigned long long pages_sup = 0;
	unsigned long long pages_mask = 0;

	if (!get_mode_page10(0x3f, page_code | 0x10)) {
		length = 9 + getnbyte(buffer + 8, 2);
		offset = 16 + getnbyte(buffer + 14, 2);
	} else if (!get_mode_page(0x3f, page_code | 0x10)) {
		length = 9 + buffer[8];
		offset = 12 + buffer[11];
	} else {		/* Assume SCSI-1 and fake settings to report NO pages */
		offset = 10;
		length = 0;
	}

	/* Get mask of pages supported by prog: */
	for (i = 0; i < MAX_PAGENO; i++)
		if (page_names[i])
			pages_mask |= (1LL << i);

	/* Get pages listed in mode_pages */
	while (offset < length) {
		pages_sup |= (1LL << (buffer[offset] & 0x3f));
		offset += 2 + buffer[offset + 1];
	}

	/* Mask out pages unsupported by this binary */
	pages_sup &= pages_mask;

	/* Notch page supported? */
	if (pages_sup & (1LL << 12)) {
		if (get_mode_page(12, 0))
			return 2;
		offset = 12 + buffer[11];
	} else {		/* Fake empty notch page */
		memset(buffer, 0, SIZEOF_BUFFER);
		offset = 0;
	}

	pages_mask = getnbyte(buffer + offset + 16, 4);
	pages_mask <<= 32;
	pages_mask += getnbyte(buffer + offset + 20, 4);

	puts("Mode Pages supported by this binary and target:");
	puts("-----------------------------------------------");
	for (i = 0; i < MAX_PAGENO; i++)
		if (pages_sup & (1LL << i))
			printf("%02xh: %s Page%s\n", i, get_page_name(i),
			       (pages_mask & (1LL << i)) ? " (notched)" : "");
	if (pages_sup & (1LL << 12)) {
		printf("\nCurrent notch is %d.\n",
		       getnbyte(buffer + offset + 6, 2));
	}
	if (!pages_sup)
		puts("No mode pages supported (SCSI-1?).");

	return 0;
}

static int open_sg_dev(char *devname)
{
	int fd, err, bus, bbus, k;
	My_scsi_idlun m_idlun, mm_idlun;
	int do_numeric = 1;
	char name[DEVNAME_SZ];
	struct stat a_st;
	int block_dev = 0;

	strncpy(name, devname, DEVNAME_SZ);
	name[DEVNAME_SZ - 1] = '\0';
	fd = open(name, O_RDONLY);
	if (fd < 0)
		return fd;
	if (fstat(fd, &a_st) < 0) {
		fprintf(stderr, "could do fstat() on fd ??\n");
		close(fd);
		return -9999;
	}
	if (S_ISBLK(a_st.st_mode))
		block_dev = 1;
	if (block_dev || (ioctl(fd, SG_GET_TIMEOUT, 0) < 0)) {
		err = ioctl(fd, SCSI_IOCTL_GET_BUS_NUMBER, &bus);
		if (err < 0) {
			perror("A SCSI device name is required\n");
			close(fd);
			return -9999;
		}
		err = ioctl(fd, SCSI_IOCTL_GET_IDLUN, &m_idlun);
		if (err < 0) {
			perror("A SCSI device name is required\n");
			close(fd);
			return -9999;
		}
		close(fd);

		for (k = 0; k < MAX_SG_DEVS; k++) {
			make_dev_name(name, NULL, k, do_numeric);
			fd = open(name, O_RDWR | O_NONBLOCK);
			if (fd < 0) {
				if ((ENOENT == errno) && (0 == k)) {
					do_numeric = 0;
					make_dev_name(name, NULL, k,
						      do_numeric);
					fd = open(name, O_RDWR | O_NONBLOCK);
				}
				if (fd < 0) {
					if (EBUSY == errno)
						continue;	/* step over if O_EXCL already on it */
					else
						break;
				}
			}
			err = ioctl(fd, SCSI_IOCTL_GET_BUS_NUMBER, &bbus);
			if (err < 0) {
				perror("sg ioctl failed");
				close(fd);
				fd = -9999;
			}
			err = ioctl(fd, SCSI_IOCTL_GET_IDLUN, &mm_idlun);
			if (err < 0) {
				perror("sg ioctl failed");
				close(fd);
				fd = -9999;
			}
			if ((bus == bbus) &&
			    ((m_idlun.dev_id & 0xff) ==
			     (mm_idlun.dev_id & 0xff))
			    && (((m_idlun.dev_id >> 8) & 0xff) ==
				((mm_idlun.dev_id >> 8) & 0xff))
			    && (((m_idlun.dev_id >> 16) & 0xff) ==
				((mm_idlun.dev_id >> 16) & 0xff)))
				break;
			else {
				close(fd);
				fd = -9999;
			}
		}
	}
	if (fd >= 0) {
#ifdef SG_GET_RESERVED_SIZE
		int size;

		if (ioctl(fd, SG_GET_RESERVED_SIZE, &size) < 0) {
			fprintf(stderr,
				"Compiled with new driver, running on old!!\n");
			close(fd);
			return -9999;
		}
#endif
		close(fd);
		return open(name, O_RDWR);
	} else
		return fd;
}

int show_scsi_info(char *device)
{
	int page_code = 0;
	int status = 0;

	print_msg(TEST_BREAK, __FUNCTION__);

	show_devices();

	glob_fd = open_sg_dev(device);
	if (glob_fd < 0) {
		if (-9999 == glob_fd)
			fprintf(stderr,
				"Couldn't find sg device corresponding to %s\n",
				device);
		else {
			perror("sginfo(open)");
			fprintf(stderr,
				"file=%s, or no corresponding sg device found\n",
				device);
			fprintf(stderr, "Is sg driver loaded?\n");
		}
		return 1;
	}

	status |= do_scsi_info_inquiry(page_code);

	status |= do_serial_number(page_code);

	status |= read_geometry(page_code);

	status |= read_cache(page_code);

	status |= read_format_info(page_code);

	status |= error_recovery_page(page_code);

	status |= read_control_page(page_code);

	status |= read_disconnect_reconnect_data(page_code);

	status |= read_defect_list(page_code);

	status |= notch_parameters_page(page_code);

	status |= verify_error_recovery(page_code);

	status |= peripheral_device_page(page_code);

	status |= do_user_page(page_code, 0);

	status |= show_pages(page_code);

	return status;
}

/* -1 -> unrecoverable error, 0 -> successful, 1 -> recoverable (ENOMEM),
   2 -> try again */
int sg_read2(int sg_fd, unsigned char *buff, int blocks, int from_block,
	     int bs, int cdbsz, int fua, int do_mmap)
{
	unsigned char rdCmd[MAX_SCSI_CDBSZ];
	unsigned char senseBuff[SENSE_BUFF_LEN];
	sg_io_hdr_t io_hdr;
	int res;

	if (sg_build_scsi_cdb(rdCmd, cdbsz, blocks, from_block, 0, fua, 0)) {
		fprintf(stderr,
			ME "bad rd cdb build, from_block=%d, blocks=%d\n",
			from_block, blocks);
		return -1;
	}
	memset(&io_hdr, 0, sizeof(sg_io_hdr_t));
	io_hdr.interface_id = 'S';
	io_hdr.cmd_len = cdbsz;
	io_hdr.cmdp = rdCmd;
	io_hdr.dxfer_direction = SG_DXFER_FROM_DEV;
	io_hdr.dxfer_len = bs * blocks;
	if (!do_mmap)
		io_hdr.dxferp = buff;
	io_hdr.mx_sb_len = SENSE_BUFF_LEN;
	io_hdr.sbp = senseBuff;
	io_hdr.timeout = DEF_TIMEOUT;
	io_hdr.pack_id = from_block;
	if (do_mmap)
		io_hdr.flags |= SG_FLAG_MMAP_IO;

	while (((res = write(sg_fd, &io_hdr, sizeof(io_hdr))) < 0) &&
	       (EINTR == errno)) ;
	if (res < 0) {
		if (ENOMEM == errno)
			return 1;
		perror("reading (wr) on sg device, error");
		return -1;
	}

	while (((res = read(sg_fd, &io_hdr, sizeof(io_hdr))) < 0) &&
	       (EINTR == errno)) ;
	if (res < 0) {
		perror("reading (rd) on sg device, error");
		return -1;
	}
	switch (sg_err_category3(&io_hdr)) {
	case SG_ERR_CAT_CLEAN:
		break;
	case SG_ERR_CAT_RECOVERED:
		fprintf(stderr,
			"Recovered error while reading block=%d, num=%d\n",
			from_block, blocks);
		break;
	case SG_ERR_CAT_MEDIA_CHANGED:
		return 2;
	default:
		sg_chk_n_print3("reading", &io_hdr);
		return -1;
	}
	sum_of_resids += io_hdr.resid;
#if SG_DEBUG
	fprintf(stderr, "duration=%u ms\n", io_hdr.duration);
#endif
	return 0;
}

/* -1 -> unrecoverable error, 0 -> successful, 1 -> recoverable (ENOMEM),
   2 -> try again */
int sg_write2(int sg_fd, unsigned char *buff, int blocks, int to_block,
	      int bs, int cdbsz, int fua, int do_mmap, int *diop)
{
	unsigned char wrCmd[MAX_SCSI_CDBSZ];
	unsigned char senseBuff[SENSE_BUFF_LEN];
	sg_io_hdr_t io_hdr;
	int res;

	if (sg_build_scsi_cdb(wrCmd, cdbsz, blocks, to_block, 1, fua, 0)) {
		fprintf(stderr, ME "bad wr cdb build, to_block=%d, blocks=%d\n",
			to_block, blocks);
		return -1;
	}

	memset(&io_hdr, 0, sizeof(sg_io_hdr_t));
	io_hdr.interface_id = 'S';
	io_hdr.cmd_len = cdbsz;
	io_hdr.cmdp = wrCmd;
	io_hdr.dxfer_direction = SG_DXFER_TO_DEV;
	io_hdr.dxfer_len = bs * blocks;
	if (!do_mmap)
		io_hdr.dxferp = buff;
	io_hdr.mx_sb_len = SENSE_BUFF_LEN;
	io_hdr.sbp = senseBuff;
	io_hdr.timeout = DEF_TIMEOUT;
	io_hdr.pack_id = to_block;
	if (do_mmap)
		io_hdr.flags |= SG_FLAG_MMAP_IO;
	if (diop && *diop)
		io_hdr.flags |= SG_FLAG_DIRECT_IO;

	while (((res = write(sg_fd, &io_hdr, sizeof(io_hdr))) < 0) &&
	       (EINTR == errno)) ;
	if (res < 0) {
		if (ENOMEM == errno)
			return 1;
		perror("writing (wr) on sg device, error");
		return -1;
	}

	while (((res = read(sg_fd, &io_hdr, sizeof(io_hdr))) < 0) &&
	       (EINTR == errno)) ;
	if (res < 0) {
		perror("writing (rd) on sg device, error");
		return -1;
	}
	switch (sg_err_category3(&io_hdr)) {
	case SG_ERR_CAT_CLEAN:
		break;
	case SG_ERR_CAT_RECOVERED:
		fprintf(stderr,
			"Recovered error while writing block=%d, num=%d\n",
			to_block, blocks);
		break;
	case SG_ERR_CAT_MEDIA_CHANGED:
		return 2;
	default:
		sg_chk_n_print3("writing", &io_hdr);
		return -1;
	}
	if (diop && *diop &&
	    ((io_hdr.info & SG_INFO_DIRECT_IO_MASK) != SG_INFO_DIRECT_IO))
		*diop = 0;	/* flag that dio not done (completely) */
	return 0;
}

int do_scsi_sgm_read_write(char *device)
{
	int skip = 0;
	int seek = 0;
	int bs = 0;
	int bpt = DEF_BLOCKS_PER_TRANSFER;
	char inf[INOUTF_SZ];
	int in_type = FT_OTHER;
	char outf[INOUTF_SZ];
	int out_type = FT_OTHER;
	int res, t;
	int infd, outfd, blocks;
	unsigned char *wrkPos;
	unsigned char *wrkBuff = NULL;
	unsigned char *wrkMmap = NULL;
	int in_num_sect = 0;
	int in_res_sz = 0;
	int out_num_sect = 0;
	int out_res_sz = 0;
	int do_time = 1;
	int scsi_cdbsz = DEF_SCSI_CDBSZ;
	int do_sync = 1;
	int do_dio = 0;
	int num_dio_not_done = 0;
	int fua_mode = 0;
	int in_sect_sz, out_sect_sz;
	char ebuff[EBUFF_SZ];
	int blocks_per;
	int req_count;
	size_t psz = getpagesize();
	struct timeval start_tm, end_tm;

	print_msg(TEST_BREAK, __FUNCTION__);

	strcpy(inf, "/dev/zero");
	strcpy(outf, device);

	install_handler(SIGINT, interrupt_handler);
	install_handler(SIGQUIT, interrupt_handler);
	install_handler(SIGPIPE, interrupt_handler);
	install_handler(SIGUSR1, siginfo_handler);

	infd = STDIN_FILENO;
	outfd = STDOUT_FILENO;

	in_type = dd_filetype(inf);

	if (FT_ST == in_type) {
		fprintf(stderr, ME "unable to use scsi tape device %s\n", inf);
		return 1;
	} else if (FT_SG == in_type) {
		if ((infd = open(inf, O_RDWR)) < 0) {
			snprintf(ebuff, EBUFF_SZ,
				 ME "could not open %s for sg reading", inf);
			perror(ebuff);
			return 1;
		}
		res = ioctl(infd, SG_GET_VERSION_NUM, &t);
		if ((res < 0) || (t < 30122)) {
			fprintf(stderr, ME "sg driver prior to 3.1.22\n");
			return 1;
		}
		in_res_sz = bs * bpt;
		if (0 != (in_res_sz % psz))	/* round up to next page */
			in_res_sz = ((in_res_sz / psz) + 1) * psz;
		if (ioctl(infd, SG_GET_RESERVED_SIZE, &t) < 0) {
			perror(ME "SG_GET_RESERVED_SIZE error");
			return 1;
		}
		if (in_res_sz > t) {
			if (ioctl(infd, SG_SET_RESERVED_SIZE, &in_res_sz) < 0) {
				perror(ME "SG_SET_RESERVED_SIZE error");
				return 1;
			}
		}
		wrkMmap = mmap(NULL, in_res_sz, PROT_READ | PROT_WRITE,
			       MAP_SHARED, infd, 0);
		if (MAP_FAILED == wrkMmap) {
			snprintf(ebuff, EBUFF_SZ,
				 ME "error using mmap() on file: %s", inf);
			perror(ebuff);
			return 1;
		}
	} else {
		if ((infd = open(inf, O_RDONLY)) < 0) {
			snprintf(ebuff, EBUFF_SZ,
				 ME "could not open %s for reading", inf);
			perror(ebuff);
			return 1;
		} else if (skip > 0) {
			llse_loff_t offset = skip;

			offset *= bs;	/* could exceed 32 bits here! */
			if (llse_llseek(infd, offset, SEEK_SET) < 0) {
				snprintf(ebuff, EBUFF_SZ, ME "couldn't skip to "
					 "required position on %s", inf);
				perror(ebuff);
				return 1;
			}
		}
	}

	if (outf[0] && ('-' != outf[0])) {
		out_type = dd_filetype(outf);

		if (FT_ST == out_type) {
			fprintf(stderr,
				ME "unable to use scsi tape device %s\n", outf);
			return 1;
		} else if (FT_SG == out_type) {
			if ((outfd = open(outf, O_RDWR)) < 0) {
				snprintf(ebuff, EBUFF_SZ,
					 ME "could not open %s for "
					 "sg writing", outf);
				perror(ebuff);
				return 1;
			}
			res = ioctl(outfd, SG_GET_VERSION_NUM, &t);
			if ((res < 0) || (t < 30122)) {
				fprintf(stderr,
					ME "sg driver prior to 3.1.22\n");
				return 1;
			}
			if (ioctl(outfd, SG_GET_RESERVED_SIZE, &t) < 0) {
				perror(ME "SG_GET_RESERVED_SIZE error");
				return 1;
			}
			out_res_sz = bs * bpt;
			if (out_res_sz > t) {
				if (ioctl
				    (outfd, SG_SET_RESERVED_SIZE,
				     &out_res_sz) < 0) {
					perror(ME "SG_SET_RESERVED_SIZE error");
					return 1;
				}
			}
			if (NULL == wrkMmap) {
				wrkMmap =
				    mmap(NULL, out_res_sz,
					 PROT_READ | PROT_WRITE, MAP_SHARED,
					 outfd, 0);
				if (MAP_FAILED == wrkMmap) {
					snprintf(ebuff, EBUFF_SZ,
						 ME
						 "error using mmap() on file: %s",
						 outf);
					perror(ebuff);
					return 1;
				}
			}
		} else if (FT_DEV_NULL == out_type)
			outfd = -1;	/* don't bother opening */
		else {
			if (FT_RAW != out_type) {
				if ((outfd =
				     open(outf, O_WRONLY | O_CREAT,
					  0666)) < 0) {
					snprintf(ebuff, EBUFF_SZ,
						 ME
						 "could not open %s for writing",
						 outf);
					perror(ebuff);
					return 1;
				}
			} else {
				if ((outfd = open(outf, O_WRONLY)) < 0) {
					snprintf(ebuff, EBUFF_SZ,
						 ME "could not open %s "
						 "for raw writing", outf);
					perror(ebuff);
					return 1;
				}
			}
			if (seek > 0) {
				llse_loff_t offset = seek;

				offset *= bs;	/* could exceed 32 bits here! */
				if (llse_llseek(outfd, offset, SEEK_SET) < 0) {
					snprintf(ebuff, EBUFF_SZ,
						 ME "couldn't seek to "
						 "required position on %s",
						 outf);
					perror(ebuff);
					return 1;
				}
			}
		}
	}
	if ((STDIN_FILENO == infd) && (STDOUT_FILENO == outfd)) {
		fprintf(stderr,
			"Can't have both 'if' as stdin _and_ 'of' as stdout\n");
		return 1;
	}
#if 0
	if ((FT_OTHER == in_type) && (FT_OTHER == out_type)) {
		fprintf(stderr, "Both 'if' and 'of' can't be ordinary files\n");
		return 1;
	}
#endif
	if (dd_count < 0) {
		if (FT_SG == in_type) {
			res = read_capacity(infd, &in_num_sect, &in_sect_sz);
			if (2 == res) {
				fprintf(stderr,
					"Unit attention, media changed(in), continuing\n");
				res =
				    read_capacity(infd, &in_num_sect,
						  &in_sect_sz);
			}
			if (0 != res) {
				fprintf(stderr,
					"Unable to read capacity on %s\n", inf);
				in_num_sect = -1;
			} else {
#if 0
				if (0 == in_sect_sz)
					in_sect_sz = bs;
				else if (in_sect_sz > bs)
					in_num_sect *= (in_sect_sz / bs);
				else if (in_sect_sz < bs)
					in_num_sect /= (bs / in_sect_sz);
#endif
				if (in_num_sect > skip)
					in_num_sect -= skip;
			}
		}
		if (FT_SG == out_type) {
			res = read_capacity(outfd, &out_num_sect, &out_sect_sz);
			if (2 == res) {
				fprintf(stderr,
					"Unit attention, media changed(out), continuing\n");
				res =
				    read_capacity(outfd, &out_num_sect,
						  &out_sect_sz);
			}
			if (0 != res) {
				fprintf(stderr,
					"Unable to read capacity on %s\n",
					outf);
				out_num_sect = -1;
			} else {
				if (out_num_sect > seek)
					out_num_sect -= seek;
			}
		}
#ifdef SG_DEBUG
		fprintf(stderr,
			"Start of loop, count=%d, in_num_sect=%d, out_num_sect=%d\n",
			dd_count, in_num_sect, out_num_sect);
#endif
		if (in_num_sect > 0) {
			if (out_num_sect > 0)
				dd_count =
				    (in_num_sect >
				     out_num_sect) ? out_num_sect : in_num_sect;
			else
				dd_count = in_num_sect;
		} else
			dd_count = out_num_sect;
	}
	if (dd_count < 0) {
		fprintf(stderr, "Couldn't calculate count, please give one\n");
		return 1;
	}
	if (do_dio && (FT_SG != in_type)) {
		do_dio = 0;
		fprintf(stderr,
			">>> dio only performed on 'of' side when 'if' is"
			" an sg device\n");
	}
	if (do_dio) {
		int fd;
		char c;

		if ((fd = open(proc_allow_dio, O_RDONLY)) >= 0) {
			if (1 == read(fd, &c, 1)) {
				if ('0' == c)
					fprintf(stderr,
						">>> %s set to '0' but should be set "
						"to '1' for direct IO\n",
						proc_allow_dio);
			}
			close(fd);
		}
	}

	if (wrkMmap)
		wrkPos = wrkMmap;
	else {
		if ((FT_RAW == in_type) || (FT_RAW == out_type)) {
			wrkBuff = malloc(bs * bpt + psz);
			if (0 == wrkBuff) {
				fprintf(stderr,
					"Not enough user memory for raw\n");
				return 1;
			}
			wrkPos =
			    (unsigned char *)(((unsigned long)wrkBuff + psz - 1)
					      & (~(psz - 1)));
		} else {
			wrkBuff = malloc(bs * bpt);
			if (0 == wrkBuff) {
				fprintf(stderr, "Not enough user memory\n");
				return 1;
			}
			wrkPos = wrkBuff;
		}
	}

	blocks_per = bpt;
#ifdef SG_DEBUG
	fprintf(stderr, "Start of loop, count=%d, blocks_per=%d\n",
		dd_count, blocks_per);
#endif
	if (do_time) {
		start_tm.tv_sec = 0;
		start_tm.tv_usec = 0;
		gettimeofday(&start_tm, NULL);
	}
	req_count = dd_count;

	while (dd_count > 0) {
		blocks = (dd_count > blocks_per) ? blocks_per : dd_count;
		if (FT_SG == in_type) {
			int fua = fua_mode & 2;

			res =
			    sg_read2(infd, wrkPos, blocks, skip, bs, scsi_cdbsz,
				     fua, 1);
			if (2 == res) {
				fprintf(stderr,
					"Unit attention, media changed, continuing (r)\n");
				res =
				    sg_read2(infd, wrkPos, blocks, skip, bs,
					     scsi_cdbsz, fua, 1);
			}
			if (0 != res) {
				fprintf(stderr, "sg_read2 failed, skip=%d\n",
					skip);
				break;
			} else
				in_full += blocks;
		} else {
			while (((res = read(infd, wrkPos, blocks * bs)) < 0) &&
			       (EINTR == errno)) ;
			if (res < 0) {
				snprintf(ebuff, EBUFF_SZ,
					 ME "reading, skip=%d ", skip);
				perror(ebuff);
				break;
			} else if (res < blocks * bs) {
				dd_count = 0;
				blocks = res / bs;
				if ((res % bs) > 0) {
					blocks++;
					in_partial++;
				}
			}
			in_full += blocks;
		}

		if (FT_SG == out_type) {
			int do_mmap = (FT_SG == in_type) ? 0 : 1;
			int fua = fua_mode & 1;
			int dio_res = do_dio;

			res =
			    sg_write2(outfd, wrkPos, blocks, seek, bs,
				      scsi_cdbsz, fua, do_mmap, &dio_res);
			if (2 == res) {
				fprintf(stderr,
					"Unit attention, media changed, continuing (w)\n");
				res =
				    sg_write2(outfd, wrkPos, blocks, seek, bs,
					      scsi_cdbsz, fua, do_mmap,
					      &dio_res);
			} else if (0 != res) {
				fprintf(stderr, "sg_write2 failed, seek=%d\n",
					seek);
				break;
			} else {
				out_full += blocks;
				if (do_dio && (0 == dio_res))
					num_dio_not_done++;
			}
		} else if (FT_DEV_NULL == out_type)
			out_full += blocks;	/* act as if written out without error */
		else {
			while (((res = write(outfd, wrkPos, blocks * bs)) < 0)
			       && (EINTR == errno)) ;
			if (res < 0) {
				snprintf(ebuff, EBUFF_SZ,
					 ME "writing, seek=%d ", seek);
				perror(ebuff);
				break;
			} else if (res < blocks * bs) {
				fprintf(stderr,
					"output file probably full, seek=%d ",
					seek);
				blocks = res / bs;
				out_full += blocks;
				if ((res % bs) > 0)
					out_partial++;
				break;
			} else
				out_full += blocks;
		}
		if (dd_count > 0)
			dd_count -= blocks;
		skip += blocks;
		seek += blocks;
	}
	if ((do_time) && (start_tm.tv_sec || start_tm.tv_usec)) {
		struct timeval res_tm;
		double a, b;

		gettimeofday(&end_tm, NULL);
		res_tm.tv_sec = end_tm.tv_sec - start_tm.tv_sec;
		res_tm.tv_usec = end_tm.tv_usec - start_tm.tv_usec;
		if (res_tm.tv_usec < 0) {
			--res_tm.tv_sec;
			res_tm.tv_usec += 1000000;
		}
		a = res_tm.tv_sec;
		a += (0.000001 * res_tm.tv_usec);
		b = (double)bs *(req_count - dd_count);
		printf("time to transfer data was %d.%06d secs",
		       (int)res_tm.tv_sec, (int)res_tm.tv_usec);
		if ((a > 0.00001) && (b > 511))
			printf(", %.2f MB/sec\n", b / (a * 1000000.0));
		else
			printf("\n");
	}
	if (do_sync) {
		if (FT_SG == out_type) {
			fprintf(stderr, ">> Synchronizing cache on %s\n", outf);
			res = sync_cache(outfd);
			if (2 == res) {
				fprintf(stderr,
					"Unit attention, media changed(in), continuing\n");
				res = sync_cache(outfd);
			}
			if (0 != res)
				fprintf(stderr,
					"Unable to synchronize cache\n");
		}
	}

	if (wrkBuff)
		free(wrkBuff);
	if (STDIN_FILENO != infd)
		close(infd);
	if ((STDOUT_FILENO != outfd) && (FT_DEV_NULL != out_type))
		close(outfd);
	res = 0;
	if (0 != dd_count) {
		fprintf(stderr, "Some error occurred,");
		res = 2;
	}
	print_stats();
	if (sum_of_resids)
		fprintf(stderr, ">> Non-zero sum of residual counts=%d\n",
			sum_of_resids);
	if (num_dio_not_done)
		fprintf(stderr, ">> dio requested but _not done %d times\n",
			num_dio_not_done);
	return res;
}

static void guarded_stop_in(Rq_coll * clp)
{
	pthread_mutex_lock(&clp->in_mutex);
	clp->in_stop = 1;
	pthread_mutex_unlock(&clp->in_mutex);
}

static void guarded_stop_out(Rq_coll * clp)
{
	pthread_mutex_lock(&clp->out_mutex);
	clp->out_stop = 1;
	pthread_mutex_unlock(&clp->out_mutex);
}

static void guarded_stop_both(Rq_coll * clp)
{
	guarded_stop_in(clp);
	guarded_stop_out(clp);
}

void *sig_listen_thread(void *v_clp)
{
	Rq_coll *clp = (Rq_coll *) v_clp;
	int sig_number;

	while (1) {
		sigwait(&signal_set, &sig_number);
		if (SIGINT == sig_number) {
			fprintf(stderr, ME "interrupted by SIGINT\n");
			guarded_stop_both(clp);
			pthread_cond_broadcast(&clp->out_sync_cv);
		}
	}
	return NULL;
}

void cleanup_in(void *v_clp)
{
	Rq_coll *clp = (Rq_coll *) v_clp;

	fprintf(stderr, "thread cancelled while in mutex held\n");
	clp->in_stop = 1;
	pthread_mutex_unlock(&clp->in_mutex);
	guarded_stop_out(clp);
	pthread_cond_broadcast(&clp->out_sync_cv);
}

void cleanup_out(void *v_clp)
{
	Rq_coll *clp = (Rq_coll *) v_clp;

	fprintf(stderr, "thread cancelled while out mutex held\n");
	clp->out_stop = 1;
	pthread_mutex_unlock(&clp->out_mutex);
	guarded_stop_in(clp);
	pthread_cond_broadcast(&clp->out_sync_cv);
}

void *read_write_thread(void *v_clp)
{
	Rq_coll *clp = (Rq_coll *) v_clp;
	Rq_elem rel;
	Rq_elem *rep = &rel;
	size_t psz = 0;
	int sz = clp->bpt * clp->bs;
	int stop_after_write = 0;
	int seek_skip = clp->seek - clp->skip;
	int blocks, status;

	memset(rep, 0, sizeof(Rq_elem));
	psz = getpagesize();
	if (NULL == (rep->alloc_bp = malloc(sz + psz)))
		err_exit(ENOMEM, "out of memory creating user buffers\n");
	rep->buffp =
	    (unsigned char *)(((unsigned long)rep->alloc_bp + psz - 1) &
			      (~(psz - 1)));
	/* Follow clp members are constant during lifetime of thread */
	rep->bs = clp->bs;
	rep->fua_mode = clp->fua_mode;
	rep->dio = clp->dio;
	rep->infd = clp->infd;
	rep->outfd = clp->outfd;
	rep->debug = clp->debug;
	rep->in_scsi_type = clp->in_scsi_type;
	rep->out_scsi_type = clp->out_scsi_type;
	rep->cdbsz = clp->cdbsz;

	while (1) {
		status = pthread_mutex_lock(&clp->in_mutex);
		if (0 != status)
			err_exit(status, "lock in_mutex");
		if (clp->in_stop || (clp->in_count <= 0)) {
			/* no more to do, exit loop then thread */
			status = pthread_mutex_unlock(&clp->in_mutex);
			if (0 != status)
				err_exit(status, "unlock in_mutex");
			break;
		}
		blocks = (clp->in_count > clp->bpt) ? clp->bpt : clp->in_count;
		rep->wr = 0;
		rep->blk = clp->in_blk;
		rep->num_blks = blocks;
		clp->in_blk += blocks;
		clp->in_count -= blocks;

		pthread_cleanup_push(cleanup_in, (void *)clp);
		if (FT_SG == clp->in_type)
			sg_in_operation(clp, rep);	/* lets go of in_mutex mid operation */
		else {
			stop_after_write =
			    normal_in_operation(clp, rep, blocks);
			status = pthread_mutex_unlock(&clp->in_mutex);
			if (0 != status)
				err_exit(status, "unlock in_mutex");
		}
		pthread_cleanup_pop(0);

		status = pthread_mutex_lock(&clp->out_mutex);
		if (0 != status)
			err_exit(status, "lock out_mutex");
		if (FT_DEV_NULL != clp->out_type) {
			while ((!clp->out_stop) &&
			       ((rep->blk + seek_skip) != clp->out_blk)) {
				/* if write would be out of sequence then wait */
				pthread_cleanup_push(cleanup_out, (void *)clp);
				status =
				    pthread_cond_wait(&clp->out_sync_cv,
						      &clp->out_mutex);
				if (0 != status)
					err_exit(status, "cond out_sync_cv");
				pthread_cleanup_pop(0);
			}
		}

		if (clp->out_stop || (clp->out_count <= 0)) {
			if (!clp->out_stop)
				clp->out_stop = 1;
			status = pthread_mutex_unlock(&clp->out_mutex);
			if (0 != status)
				err_exit(status, "unlock out_mutex");
			break;
		}
		if (stop_after_write)
			clp->out_stop = 1;
		rep->wr = 1;
		rep->blk = clp->out_blk;
		/* rep->num_blks = blocks; */
		clp->out_blk += blocks;
		clp->out_count -= blocks;

		pthread_cleanup_push(cleanup_out, (void *)clp);
		if (FT_SG == clp->out_type)
			sg_out_operation(clp, rep);	/* releases out_mutex mid operation */
		else if (FT_DEV_NULL == clp->out_type) {
			/* skip actual write operation */
			clp->out_done_count -= blocks;
			status = pthread_mutex_unlock(&clp->out_mutex);
			if (0 != status)
				err_exit(status, "unlock out_mutex");
		} else {
			normal_out_operation(clp, rep, blocks);
			status = pthread_mutex_unlock(&clp->out_mutex);
			if (0 != status)
				err_exit(status, "unlock out_mutex");
		}
		pthread_cleanup_pop(0);

		if (stop_after_write)
			break;
		pthread_cond_broadcast(&clp->out_sync_cv);
	}			/* end of while loop */
	if (rep->alloc_bp)
		free(rep->alloc_bp);
	status = pthread_mutex_lock(&clp->in_mutex);
	if (0 != status)
		err_exit(status, "lock in_mutex");
	if (!clp->in_stop)
		clp->in_stop = 1;	/* flag other workers to stop */
	status = pthread_mutex_unlock(&clp->in_mutex);
	if (0 != status)
		err_exit(status, "unlock in_mutex");
	pthread_cond_broadcast(&clp->out_sync_cv);
	return stop_after_write ? NULL : v_clp;
}

int normal_in_operation(Rq_coll * clp, Rq_elem * rep, int blocks)
{
	int res;
	int stop_after_write = 0;

	/* enters holding in_mutex */
	while (((res = read(clp->infd, rep->buffp,
			    blocks * clp->bs)) < 0) && (EINTR == errno)) ;
	if (res < 0) {
		if (clp->coe) {
			memset(rep->buffp, 0, rep->num_blks * rep->bs);
			fprintf(stderr,
				">> substituted zeros for in blk=%d for "
				"%d bytes, %s\n", rep->blk,
				rep->num_blks * rep->bs, strerror(errno));
			res = rep->num_blks * clp->bs;
		} else {
			fprintf(stderr, "error in normal read, %s\n",
				strerror(errno));
			clp->in_stop = 1;
			guarded_stop_out(clp);
			return 1;
		}
	}
	if (res < blocks * clp->bs) {
		int o_blocks = blocks;
		stop_after_write = 1;
		blocks = res / clp->bs;
		if ((res % clp->bs) > 0) {
			blocks++;
			clp->in_partial++;
		}
		/* Reverse out + re-apply blocks on clp */
		clp->in_blk -= o_blocks;
		clp->in_count += o_blocks;
		rep->num_blks = blocks;
		clp->in_blk += blocks;
		clp->in_count -= blocks;
	}
	clp->in_done_count -= blocks;
	return stop_after_write;
}

void normal_out_operation(Rq_coll * clp, Rq_elem * rep, int blocks)
{
	int res;

	/* enters holding out_mutex */
	while (((res = write(clp->outfd, rep->buffp,
			     rep->num_blks * clp->bs)) < 0)
	       && (EINTR == errno)) ;
	if (res < 0) {
		if (clp->coe) {
			fprintf(stderr, ">> ignored error for out blk=%d for "
				"%d bytes, %s\n", rep->blk,
				rep->num_blks * rep->bs, strerror(errno));
			res = rep->num_blks * clp->bs;
		} else {
			fprintf(stderr, "error normal write, %s\n",
				strerror(errno));
			guarded_stop_in(clp);
			clp->out_stop = 1;
			return;
		}
	}
	if (res < blocks * clp->bs) {
		blocks = res / clp->bs;
		if ((res % clp->bs) > 0) {
			blocks++;
			clp->out_partial++;
		}
		rep->num_blks = blocks;
	}
	clp->out_done_count -= blocks;
}

void sg_in_operation(Rq_coll * clp, Rq_elem * rep)
{
	int res;
	int status;

	/* enters holding in_mutex */
	while (1) {
		res = sg_start_io(rep);
		if (1 == res)
			err_exit(ENOMEM, "sg starting in command");
		else if (res < 0) {
			fprintf(stderr, ME "inputting to sg failed, blk=%d\n",
				rep->blk);
			status = pthread_mutex_unlock(&clp->in_mutex);
			if (0 != status)
				err_exit(status, "unlock in_mutex");
			guarded_stop_both(clp);
			return;
		}
		/* Now release in mutex to let other reads run in parallel */
		status = pthread_mutex_unlock(&clp->in_mutex);
		if (0 != status)
			err_exit(status, "unlock in_mutex");

		res = sg_finish_io(rep->wr, rep, &clp->aux_mutex);
		if (res < 0) {
			if (clp->coe) {
				memset(rep->buffp, 0, rep->num_blks * rep->bs);
				fprintf(stderr,
					">> substituted zeros for in blk=%d for "
					"%d bytes\n", rep->blk,
					rep->num_blks * rep->bs);
			} else {
				fprintf(stderr,
					"error finishing sg in command\n");
				guarded_stop_both(clp);
				return;
			}
		}
		if (res <= 0) {	/* looks good, going to return */
			if (rep->dio_incomplete || rep->resid) {
				status = pthread_mutex_lock(&clp->aux_mutex);
				if (0 != status)
					err_exit(status, "lock aux_mutex");
				clp->dio_incomplete += rep->dio_incomplete;
				clp->sum_of_resids += rep->resid;
				status = pthread_mutex_unlock(&clp->aux_mutex);
				if (0 != status)
					err_exit(status, "unlock aux_mutex");
			}
			status = pthread_mutex_lock(&clp->in_mutex);
			if (0 != status)
				err_exit(status, "lock in_mutex");
			clp->in_done_count -= rep->num_blks;
			status = pthread_mutex_unlock(&clp->in_mutex);
			if (0 != status)
				err_exit(status, "unlock in_mutex");
			return;
		}
		/* else assume 1 == res so try again with same addr, count info */
		/* now re-acquire read mutex for balance */
		/* N.B. This re-read could now be out of read sequence */
		status = pthread_mutex_lock(&clp->in_mutex);
		if (0 != status)
			err_exit(status, "lock in_mutex");
	}
}

void sg_out_operation(Rq_coll * clp, Rq_elem * rep)
{
	int res;
	int status;

	/* enters holding out_mutex */
	while (1) {
		res = sg_start_io(rep);
		if (1 == res)
			err_exit(ENOMEM, "sg starting out command");
		else if (res < 0) {
			fprintf(stderr,
				ME "outputting from sg failed, blk=%d\n",
				rep->blk);
			status = pthread_mutex_unlock(&clp->out_mutex);
			if (0 != status)
				err_exit(status, "unlock out_mutex");
			guarded_stop_both(clp);
			return;
		}
		/* Now release in mutex to let other reads run in parallel */
		status = pthread_mutex_unlock(&clp->out_mutex);
		if (0 != status)
			err_exit(status, "unlock out_mutex");

		res = sg_finish_io(rep->wr, rep, &clp->aux_mutex);
		if (res < 0) {
			if (clp->coe)
				fprintf(stderr,
					">> ignored error for out blk=%d for "
					"%d bytes\n", rep->blk,
					rep->num_blks * rep->bs);
			else {
				fprintf(stderr,
					"error finishing sg out command\n");
				guarded_stop_both(clp);
				return;
			}
		}
		if (res <= 0) {
			if (rep->dio_incomplete || rep->resid) {
				status = pthread_mutex_lock(&clp->aux_mutex);
				if (0 != status)
					err_exit(status, "lock aux_mutex");
				clp->dio_incomplete += rep->dio_incomplete;
				clp->sum_of_resids += rep->resid;
				status = pthread_mutex_unlock(&clp->aux_mutex);
				if (0 != status)
					err_exit(status, "unlock aux_mutex");
			}
			status = pthread_mutex_lock(&clp->out_mutex);
			if (0 != status)
				err_exit(status, "lock out_mutex");
			clp->out_done_count -= rep->num_blks;
			status = pthread_mutex_unlock(&clp->out_mutex);
			if (0 != status)
				err_exit(status, "unlock out_mutex");
			return;
		}
		/* else assume 1 == res so try again with same addr, count info */
		/* now re-acquire out mutex for balance */
		/* N.B. This re-write could now be out of write sequence */
		status = pthread_mutex_lock(&clp->out_mutex);
		if (0 != status)
			err_exit(status, "lock out_mutex");
	}
}

int sg_start_io(Rq_elem * rep)
{
	sg_io_hdr_t *hp = &rep->io_hdr;
	int fua = rep->wr ? (rep->fua_mode & 1) : (rep->fua_mode & 2);
	int res;

	if (sg_build_scsi_cdb(rep->cmd, rep->cdbsz, rep->num_blks, rep->blk,
			      rep->wr, fua, 0)) {
		fprintf(stderr, ME "bad cdb build, start_blk=%d, blocks=%d\n",
			rep->blk, rep->num_blks);
		return -1;
	}
	memset(hp, 0, sizeof(sg_io_hdr_t));
	hp->interface_id = 'S';
	hp->cmd_len = rep->cdbsz;
	hp->cmdp = rep->cmd;
	hp->dxfer_direction = rep->wr ? SG_DXFER_TO_DEV : SG_DXFER_FROM_DEV;
	hp->dxfer_len = rep->bs * rep->num_blks;
	hp->dxferp = rep->buffp;
	hp->mx_sb_len = sizeof(rep->sb);
	hp->sbp = rep->sb;
	hp->timeout = DEF_TIMEOUT;
	hp->usr_ptr = rep;
	hp->pack_id = rep->blk;
	if (rep->dio)
		hp->flags |= SG_FLAG_DIRECT_IO;
	if (rep->debug > 8) {
		fprintf(stderr, "sg_start_io: SCSI %s, blk=%d num_blks=%d\n",
			rep->wr ? "WRITE" : "READ", rep->blk, rep->num_blks);
		sg_print_command(hp->cmdp);
		fprintf(stderr, "dir=%d, len=%d, dxfrp=%p, cmd_len=%d\n",
			hp->dxfer_direction, hp->dxfer_len, hp->dxferp,
			hp->cmd_len);
	}

	while (((res = write(rep->wr ? rep->outfd : rep->infd, hp,
			     sizeof(sg_io_hdr_t))) < 0) && (EINTR == errno)) ;
	if (res < 0) {
		if (ENOMEM == errno)
			return 1;
		perror("starting io on sg device, error");
		return -1;
	}
	return 0;
}

/* -1 -> unrecoverable error, 0 -> successful, 1 -> try again */
int sg_finish_io(int wr, Rq_elem * rep, pthread_mutex_t * a_mutp)
{
	int res, status;
	sg_io_hdr_t io_hdr;
	sg_io_hdr_t *hp;
#if 0
	static int testing = 0;	/* thread dubious! */
#endif

	memset(&io_hdr, 0, sizeof(sg_io_hdr_t));
	/* FORCE_PACK_ID active set only read packet with matching pack_id */
	io_hdr.interface_id = 'S';
	io_hdr.dxfer_direction = rep->wr ? SG_DXFER_TO_DEV : SG_DXFER_FROM_DEV;
	io_hdr.pack_id = rep->blk;

	while (((res = read(wr ? rep->outfd : rep->infd, &io_hdr,
			    sizeof(sg_io_hdr_t))) < 0) && (EINTR == errno)) ;
	if (res < 0) {
		perror("finishing io on sg device, error");
		return -1;
	}
	if (rep != (Rq_elem *) io_hdr.usr_ptr)
		err_exit(0,
			 "sg_finish_io: bad usr_ptr, request-response mismatch\n");
	memcpy(&rep->io_hdr, &io_hdr, sizeof(sg_io_hdr_t));
	hp = &rep->io_hdr;

	switch (sg_err_category3(hp)) {
	case SG_ERR_CAT_CLEAN:
		break;
	case SG_ERR_CAT_RECOVERED:
		fprintf(stderr, "Recovered error on block=%d, num=%d\n",
			rep->blk, rep->num_blks);
		break;
	case SG_ERR_CAT_MEDIA_CHANGED:
		return 1;
	default:
		{
			char ebuff[EBUFF_SZ];

			snprintf(ebuff, EBUFF_SZ,
				 "%s blk=%d", rep->wr ? "writing" : "reading",
				 rep->blk);
			status = pthread_mutex_lock(a_mutp);
			if (0 != status)
				err_exit(status, "lock aux_mutex");
			sg_chk_n_print3(ebuff, hp);
			status = pthread_mutex_unlock(a_mutp);
			if (0 != status)
				err_exit(status, "unlock aux_mutex");
			return -1;
		}
	}
#if 0
	if (0 == (++testing % 100))
		return -1;
#endif
	if (rep->dio &&
	    ((hp->info & SG_INFO_DIRECT_IO_MASK) != SG_INFO_DIRECT_IO))
		rep->dio_incomplete = 1;	/* count dios done as indirect IO */
	else
		rep->dio_incomplete = 0;
	rep->resid = hp->resid;
	if (rep->debug > 8)
		fprintf(stderr, "sg_finish_io: completed %s\n",
			wr ? "WRITE" : "READ");
	return 0;
}

int sg_prepare(int fd, int bs, int bpt, int *scsi_typep)
{
	int res, t;

	res = ioctl(fd, SG_GET_VERSION_NUM, &t);
	if ((res < 0) || (t < 30000)) {
		fprintf(stderr, ME "sg driver prior to 3.x.y\n");
		return 1;
	}
	res = 0;
	t = bs * bpt;
	res = ioctl(fd, SG_SET_RESERVED_SIZE, &t);
	if (res < 0)
		perror(ME "SG_SET_RESERVED_SIZE error");
	t = 1;
	res = ioctl(fd, SG_SET_FORCE_PACK_ID, &t);
	if (res < 0)
		perror(ME "SG_SET_FORCE_PACK_ID error");
	if (scsi_typep) {
		struct sg_scsi_id info;

		res = ioctl(fd, SG_GET_SCSI_ID, &info);
		if (res < 0)
			perror(ME "SG_SET_SCSI_ID error");
		*scsi_typep = info.scsi_type;
	}
	return 0;
}

int do_scsi_sgp_read_write(char *device)
{
	int skip = 0;
	int seek = 0;
	int count = -1;
	char inf[INOUTF_SZ];
	char outf[INOUTF_SZ];
	int res, k;
	int in_num_sect = 0;
	int out_num_sect = 0;
	int num_threads = DEF_NUM_THREADS;
	pthread_t threads[MAX_NUM_THREADS];
	int do_time = 1;
	int do_sync = 1;
	int in_sect_sz, out_sect_sz, status, infull, outfull;
	void *vp;
	char ebuff[EBUFF_SZ];
	struct timeval start_tm, end_tm;
	Rq_coll rcoll;

	print_msg(TEST_BREAK, __FUNCTION__);

	memset(&rcoll, 0, sizeof(Rq_coll));
	rcoll.bpt = DEF_BLOCKS_PER_TRANSFER;
	rcoll.in_type = FT_OTHER;
	rcoll.out_type = FT_OTHER;
	rcoll.cdbsz = DEF_SCSI_CDBSZ;

	strcpy(inf, "/dev/zero");
	strcpy(outf, device);

	if (rcoll.bs <= 0) {
		rcoll.bs = DEF_BLOCK_SIZE;
		fprintf(stderr,
			"Assume default 'bs' (block size) of %d bytes\n",
			rcoll.bs);
	}

	if (rcoll.debug)
		fprintf(stderr, ME "if=%s skip=%d of=%s seek=%d count=%d\n",
			inf, skip, outf, seek, count);

	rcoll.infd = STDIN_FILENO;
	rcoll.outfd = STDOUT_FILENO;
	if (inf[0] && ('-' != inf[0])) {
		rcoll.in_type = dd_filetype(inf);

		if (FT_ST == rcoll.in_type) {
			fprintf(stderr,
				ME "unable to use scsi tape device %s\n", inf);
			return 1;
		} else if (FT_SG == rcoll.in_type) {
			if ((rcoll.infd = open(inf, O_RDWR)) < 0) {
				snprintf(ebuff, EBUFF_SZ,
					 ME "could not open %s for sg reading",
					 inf);
				perror(ebuff);
				return 1;
			}
			if (sg_prepare(rcoll.infd, rcoll.bs, rcoll.bpt,
				       &rcoll.in_scsi_type))
				return 1;
		} else {
			if ((rcoll.infd = open(inf, O_RDONLY)) < 0) {
				snprintf(ebuff, EBUFF_SZ,
					 ME "could not open %s for reading",
					 inf);
				perror(ebuff);
				return 1;
			} else if (skip > 0) {
				llse_loff_t offset = skip;

				offset *= rcoll.bs;	/* could exceed 32 here! */
				if (llse_llseek(rcoll.infd, offset, SEEK_SET) <
				    0) {
					snprintf(ebuff, EBUFF_SZ,
						 ME
						 "couldn't skip to required position on %s",
						 inf);
					perror(ebuff);
					return 1;
				}
			}
		}
	}
	if (outf[0] && ('-' != outf[0])) {
		rcoll.out_type = dd_filetype(outf);

		if (FT_ST == rcoll.out_type) {
			fprintf(stderr,
				ME "unable to use scsi tape device %s\n", outf);
			return 1;
		} else if (FT_SG == rcoll.out_type) {
			if ((rcoll.outfd = open(outf, O_RDWR)) < 0) {
				snprintf(ebuff, EBUFF_SZ,
					 ME "could not open %s for sg writing",
					 outf);
				perror(ebuff);
				return 1;
			}

			if (sg_prepare(rcoll.outfd, rcoll.bs, rcoll.bpt,
				       &rcoll.out_scsi_type))
				return 1;
		} else if (FT_DEV_NULL == rcoll.out_type)
			rcoll.outfd = -1;	/* don't bother opening */
		else {
			if (FT_RAW != rcoll.out_type) {
				if ((rcoll.outfd =
				     open(outf, O_WRONLY | O_CREAT,
					  0666)) < 0) {
					snprintf(ebuff, EBUFF_SZ,
						 ME
						 "could not open %s for writing",
						 outf);
					perror(ebuff);
					return 1;
				}
			} else {
				if ((rcoll.outfd = open(outf, O_WRONLY)) < 0) {
					snprintf(ebuff, EBUFF_SZ,
						 ME
						 "could not open %s for raw writing",
						 outf);
					perror(ebuff);
					return 1;
				}
			}
			if (seek > 0) {
				llse_loff_t offset = seek;

				offset *= rcoll.bs;	/* could exceed 32 bits here! */
				if (llse_llseek(rcoll.outfd, offset, SEEK_SET) <
				    0) {
					snprintf(ebuff, EBUFF_SZ,
						 ME
						 "couldn't seek to required position on %s",
						 outf);
					perror(ebuff);
					return 1;
				}
			}
		}
	}
	if ((STDIN_FILENO == rcoll.infd) && (STDOUT_FILENO == rcoll.outfd)) {
		fprintf(stderr,
			"Disallow both if and of to be stdin and stdout");
		return 1;
	}
	if (count < 0) {
		if (FT_SG == rcoll.in_type) {
			res =
			    read_capacity(rcoll.infd, &in_num_sect,
					  &in_sect_sz);
			if (2 == res) {
				fprintf(stderr,
					"Unit attention, media changed(in), continuing\n");
				res =
				    read_capacity(rcoll.infd, &in_num_sect,
						  &in_sect_sz);
			}
			if (0 != res) {
				fprintf(stderr,
					"Unable to read capacity on %s\n", inf);
				in_num_sect = -1;
			} else {
				if (in_num_sect > skip)
					in_num_sect -= skip;
			}
		}
		if (FT_SG == rcoll.out_type) {
			res =
			    read_capacity(rcoll.outfd, &out_num_sect,
					  &out_sect_sz);
			if (2 == res) {
				fprintf(stderr,
					"Unit attention, media changed(out), continuing\n");
				res =
				    read_capacity(rcoll.outfd, &out_num_sect,
						  &out_sect_sz);
			}
			if (0 != res) {
				fprintf(stderr,
					"Unable to read capacity on %s\n",
					outf);
				out_num_sect = -1;
			} else {
				if (out_num_sect > seek)
					out_num_sect -= seek;
			}
		}
		if (in_num_sect > 0) {
			if (out_num_sect > 0)
				count =
				    (in_num_sect >
				     out_num_sect) ? out_num_sect : in_num_sect;
			else
				count = in_num_sect;
		} else
			count = out_num_sect;
	}
	if (rcoll.debug > 1)
		fprintf(stderr, "Start of loop, count=%d, in_num_sect=%d, "
			"out_num_sect=%d\n", count, in_num_sect, out_num_sect);
	if (count < 0) {
		fprintf(stderr, "Couldn't calculate count, please give one\n");
		return 1;
	}

	rcoll.in_count = count;
	rcoll.in_done_count = count;
	rcoll.skip = skip;
	rcoll.in_blk = skip;
	rcoll.out_count = count;
	rcoll.out_done_count = count;
	rcoll.seek = seek;
	rcoll.out_blk = seek;
	status = pthread_mutex_init(&rcoll.in_mutex, NULL);
	if (0 != status)
		err_exit(status, "init in_mutex");
	status = pthread_mutex_init(&rcoll.out_mutex, NULL);
	if (0 != status)
		err_exit(status, "init out_mutex");
	status = pthread_mutex_init(&rcoll.aux_mutex, NULL);
	if (0 != status)
		err_exit(status, "init aux_mutex");
	status = pthread_cond_init(&rcoll.out_sync_cv, NULL);
	if (0 != status)
		err_exit(status, "init out_sync_cv");

	sigemptyset(&signal_set);
	sigaddset(&signal_set, SIGINT);
	status = pthread_sigmask(SIG_BLOCK, &signal_set, NULL);
	if (0 != status)
		err_exit(status, "pthread_sigmask");
	status = pthread_create(&sig_listen_thread_id, NULL,
				sig_listen_thread, (void *)&rcoll);
	if (0 != status)
		err_exit(status, "pthread_create, sig...");

	if (do_time) {
		start_tm.tv_sec = 0;
		start_tm.tv_usec = 0;
		gettimeofday(&start_tm, NULL);
	}

/* vvvvvvvvvvv  Start worker threads  vvvvvvvvvvvvvvvvvvvvvvvv */
	if ((rcoll.out_done_count > 0) && (num_threads > 0)) {
		/* Run 1 work thread to shake down infant retryable stuff */
		status = pthread_mutex_lock(&rcoll.out_mutex);
		if (0 != status)
			err_exit(status, "lock out_mutex");
		status = pthread_create(&threads[0], NULL, read_write_thread,
					(void *)&rcoll);
		if (0 != status)
			err_exit(status, "pthread_create");
		if (rcoll.debug)
			fprintf(stderr, "Starting worker thread k=0\n");

		/* wait for any broadcast */
		pthread_cleanup_push(cleanup_out, (void *)&rcoll);
		status =
		    pthread_cond_wait(&rcoll.out_sync_cv, &rcoll.out_mutex);
		if (0 != status)
			err_exit(status, "cond out_sync_cv");
		pthread_cleanup_pop(0);
		status = pthread_mutex_unlock(&rcoll.out_mutex);
		if (0 != status)
			err_exit(status, "unlock out_mutex");

		/* now start the rest of the threads */
		for (k = 1; k < num_threads; ++k) {
			status =
			    pthread_create(&threads[k], NULL, read_write_thread,
					   (void *)&rcoll);
			if (0 != status)
				err_exit(status, "pthread_create");
			if (rcoll.debug)
				fprintf(stderr, "Starting worker thread k=%d\n",
					k);
		}

		/* now wait for worker threads to finish */
		for (k = 0; k < num_threads; ++k) {
			status = pthread_join(threads[k], &vp);
			if (0 != status)
				err_exit(status, "pthread_join");
			if (rcoll.debug)
				fprintf(stderr,
					"Worker thread k=%d terminated\n", k);
		}
	}

	if ((do_time) && (start_tm.tv_sec || start_tm.tv_usec)) {
		struct timeval res_tm;
		double a, b;

		gettimeofday(&end_tm, NULL);
		res_tm.tv_sec = end_tm.tv_sec - start_tm.tv_sec;
		res_tm.tv_usec = end_tm.tv_usec - start_tm.tv_usec;
		if (res_tm.tv_usec < 0) {
			--res_tm.tv_sec;
			res_tm.tv_usec += 1000000;
		}
		a = res_tm.tv_sec;
		a += (0.000001 * res_tm.tv_usec);
		b = (double)rcoll.bs * (count - rcoll.out_done_count);
		printf("time to transfer data was %d.%06d secs",
		       (int)res_tm.tv_sec, (int)res_tm.tv_usec);
		if ((a > 0.00001) && (b > 511))
			printf(", %.2f MB/sec\n", b / (a * 1000000.0));
		else
			printf("\n");
	}
	if (do_sync) {
		if (FT_SG == rcoll.out_type) {
			fprintf(stderr, ">> Synchronizing cache on %s\n", outf);
			res = sync_cache(rcoll.outfd);
			if (2 == res) {
				fprintf(stderr,
					"Unit attention, media changed(in), continuing\n");
				res = sync_cache(rcoll.outfd);
			}
			if (0 != res)
				fprintf(stderr,
					"Unable to synchronize cache\n");
		}
	}

	status = pthread_cancel(sig_listen_thread_id);
	if (0 != status)
		err_exit(status, "pthread_cancel");
	if (STDIN_FILENO != rcoll.infd)
		close(rcoll.infd);
	if ((STDOUT_FILENO != rcoll.outfd) && (FT_DEV_NULL != rcoll.out_type))
		close(rcoll.outfd);
	res = 0;
	if (0 != rcoll.out_count) {
		fprintf(stderr,
			">>>> Some error occurred, remaining blocks=%d\n",
			rcoll.out_count);
		res = 2;
	}
	infull = count - rcoll.in_done_count - rcoll.in_partial;
	fprintf(stderr, "%d+%d records in\n", infull, rcoll.in_partial);
	outfull = count - rcoll.out_done_count - rcoll.out_partial;
	fprintf(stderr, "%d+%d records out\n", outfull, rcoll.out_partial);
	if (rcoll.dio_incomplete) {
		int fd;
		char c;

		fprintf(stderr,
			">> Direct IO requested but incomplete %d times\n",
			rcoll.dio_incomplete);
		if ((fd = open(proc_allow_dio, O_RDONLY)) >= 0) {
			if (1 == read(fd, &c, 1)) {
				if ('0' == c)
					fprintf(stderr,
						">>> %s set to '0' but should be set "
						"to '1' for direct IO\n",
						proc_allow_dio);
			}
			close(fd);
		}
	}
	if (rcoll.sum_of_resids)
		fprintf(stderr, ">> Non-zero sum of residual counts=%d\n",
			rcoll.sum_of_resids);
	return res;
}
