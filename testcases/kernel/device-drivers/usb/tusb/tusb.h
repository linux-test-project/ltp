//tusb.h

#define TEST_USB_DRIVER_NAME	"Usb test module"
#define DEVICE_NAME		"/dev/tusb"
#define TUSB_MAJOR      252


#define MAG_NUM 		's'
#define FIND_DEV		_IO(MAG_NUM, 1)
#define TEST_FIND_HCD		_IO(MAG_NUM, 2)
#define TEST_HCD_PROBE		_IO(MAG_NUM, 3)
#define TEST_HCD_REMOVE		_IO(MAG_NUM, 4)
#define TEST_HCD_SUSPEND	_IO(MAG_NUM, 5)
#define TEST_HCD_RESUME		_IO(MAG_NUM, 6)
#ifndef SET_MODULE_OWNER
#define SET_MODULE_OWNER(dev) ((dev)->owner = THIS_MODULE)
#endif

/*
 * structures for USB test driver
 */
struct tusb_interface {
        int     in_len;         // input data length
        caddr_t in_data;        // input data
        int     out_rc;         // return code from the test
        int     out_len;        // output data length
        caddr_t out_data;       // output data
};
typedef struct tusb_interface tusb_interface_t;
