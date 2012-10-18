
#define FS_LTP_TEST_COMPONENT	        	0x00020999
#define FS_LTP_TEST_CLASS		        	"ltp_test"
#define FS_LTP_TEST_HID 		        	"FS0999"
#define FS_LTP_TEST_DRIVER_NAME		    "FS LTP Test Driver"
#define FS_LTP_TEST_DEVICE_NAME	    	"LTP Test"
#define FS_LTP_TEST_FILE_STATE    		"state"
#define FS_LTP_TEST_NOTIFY_STATUS		    0x80
#define FS_LTP_TEST_STATUS_OFFLINE		0x00
#define FS_LTP_TEST_STATUS_ONLINE	    	0x01
#define FS_LTP_TEST_STATUS_UNKNOWN		0xFF
#define _COMPONENT		FS_LTP_TEST_COMPONENT
#define FS_TLP_TEST_MODULE_NAME		("fs_ltp_test")
#define FS_NS_SYSTEM_BUS          "_SB_"
#define FS_BATTERY_FORMAT_BIF	"NNNNNNNNNSSSS"
#define FS_BATTERY_FORMAT_BST	"NNNN"


#define FS_TYPE_ANY                   0x00
#define FS_TYPE_INTEGER               0x01  /* Byte/Word/Dword/Zero/One/Ones */
#define FS_TYPE_STRING                0x02
#define FS_TYPE_BUFFER                0x03
#define FS_TYPE_PACKAGE               0x04  /* byte_const, multiple data_term/Constant/super_name */
#define FS_TYPE_FIELD_UNIT            0x05
#define FS_TYPE_DEVICE                0x06  /* Name, multiple Node */
#define FS_TYPE_EVENT                 0x07
#define FS_TYPE_METHOD                0x08  /* Name, byte_const, multiple Code */
#define FS_TYPE_MUTEX                 0x09
#define FS_TYPE_REGION                0x0A
#define FS_TYPE_POWER                 0x0B  /* Name,byte_const,word_const,multi Node */
#define FS_TYPE_PROCESSOR             0x0C  /* Name,byte_const,Dword_const,byte_const,multi nm_o */
#define FS_TYPE_THERMAL               0x0D  /* Name, multiple Node */
#define FS_TYPE_BUFFER_FIELD          0x0E
#define FS_TYPE_DDB_HANDLE            0x0F
#define FS_TYPE_DEBUG_OBJECT          0x10

#define FS_TYPE_EXTERNAL_MAX          0x10
#define LTPMAJOR                       256

/* Use 'k' as magic number */
#define LTPFS_IOC_MAGIC  'k'
#define TOMINOR(x) ((x & 3) | ((x & 4) << 5))


#define DEV_PATH                            "/dev"
#define LTP_FS_DIR_NAME                   ""
#define LTP_FS_DEV_NAME                   "LTPFS"
#define LTP_FS_DEV_NODE_PATH              DEV_PATH "/"
#define LTP_FS_DEVICE_NAME                DEV_PATH "/"  LTP_FS_DEV_NAME
#define MINOR_SHIFT_BITS 3
#define MAX_PARTITIONS 8                    /* partition 0 + 7 more possible due to 3 bit partition number field */
#define	MAX_NUM_DISKS 3                   /* number of real devices */

#define MPDEV_FLAG_CLEAR 0
#define MPDEV_FLAG_SET   1

typedef struct _ltpdev_cmd {
    u_int32_t     cmd;           // input - 0==recover, 1==fail
    u_int32_t     status;        // ouput - 0==success
} ltpdev_cmd_t;

typedef enum ltpdev_ioctl_cmds_s {
	/* version commands */
	LTP_AIO_IOCTL_NUMBER = 0x5500,
	LTP_BIO_IOCTL_NUMBER = 0x5501
} ltpdev_ioctl_cmds_t;

// define the ioctl cmds
#define LTPAIODEV_CMD       _IOR( LTPMAJOR, LTP_AIO_IOCTL_NUMBER, ltpdev_cmd_t **)
#define LTPBIODEV_CMD       _IOR( LTPMAJOR, LTP_BIO_IOCTL_NUMBER, ltpdev_cmd_t **)

