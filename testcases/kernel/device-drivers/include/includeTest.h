//includeTest.h

#define MAG_NUM 'k'
#define MODULE_TEST_COMPONENT   0x00023000
#define OPTION1                 _IO(MAG_NUM, 1)
#define _COMPONENT              MODULE_TEST_COMPONENT
#define TEST_DRIVER_NAME        "David's include test module"
#define DEVICE_NAME             "includeTest"
#define DEV_PATH		"/dev"
#define INCLUDE_DEVICE_PATH	DEV_PATH "/"
#define INCLUDEMAJOR		253
#define MINOR_SHIFT_BITS 	3
#define MAX_PARTITIONS		8
#define MAX_NUM_DISKS		3
#define DEV_NAME		"includeTest"

typedef struct _incdev_cmd {
	u_int32_t cmd;
	u_int32_t status;
} incdev_cmd_t;

typedef enum incdev_ioctl_cmds_s {
	INC_IOCTL_NUMBER = 0x5500
} incdev_ioctl_cmds_t;

#define INCDEV_CMD		_IOR(INCLUDEMAJOR, INC_IOCTL_NUMBER, incdev_cmd_t**)

/*
 * function prototypes
 */

static void option1(void);
