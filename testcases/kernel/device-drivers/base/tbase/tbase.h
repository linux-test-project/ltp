// tbase.h

#define TMOD_DRIVER_NAME	"ltp test drivers/base"
#define DEVICE_NAME		"tbase"
#define MAG_NUM			'k'

/* put ioctl flags here, use the _IO macro which is 
 found in linux/ioctl.h, takes a letter, and an 
 integer */

#define DEV_PROBE		_IO(MAG_NUM, 1)
#define REG_DEVICE		_IO(MAG_NUM, 2)
#define UNREG_DEVICE		_IO(MAG_NUM, 3)
#define BUS_ADD			_IO(MAG_NUM, 4)
#define FIND_BUS		_IO(MAG_NUM, 5)
#define BUS_REMOVE		_IO(MAG_NUM, 6)
#define GET_DRV			_IO(MAG_NUM, 7)
#define PUT_DRV			_IO(MAG_NUM, 8)
#define DRV_REG			_IO(MAG_NUM, 9)
#define DRV_UNREG		_IO(MAG_NUM, 10)
#define REG_FIRM		_IO(MAG_NUM, 11)
#define CREATE_FILE		_IO(MAG_NUM, 12)
#define DEV_SUSPEND		_IO(MAG_NUM, 13)
#define DEV_FILE		_IO(MAG_NUM, 14)
#define BUS_RESCAN		_IO(MAG_NUM, 15)
#define BUS_FILE		_IO(MAG_NUM, 16)
#define CLASS_REG		_IO(MAG_NUM, 17)
#define CLASS_UNREG		_IO(MAG_NUM, 18)
#define CLASS_FILE		_IO(MAG_NUM, 19)
#define CLASS_GET		_IO(MAG_NUM, 20)
#define CLASSDEV_REG		_IO(MAG_NUM, 21)
#define CLASSINT_REG		_IO(MAG_NUM, 22)
#define SYSDEV_REG		_IO(MAG_NUM, 23)
#define SYSDEV_UNREG		_IO(MAG_NUM, 24)
#define SYSDEV_CLS_REG		_IO(MAG_NUM, 25)
#define SYSDEV_CLS_UNREG	_IO(MAG_NUM, 26)

/* interface for passing structures between user 
 space and kernel space easily */

struct tmod_interface {
	int     in_len;         // input data length
        caddr_t in_data;        // input data
        int     out_rc;         // return code from the test
        int     out_len;        // output data length
        caddr_t out_data;       // output data
};
typedef struct tmod_interface tmod_interface_t;
 

 


