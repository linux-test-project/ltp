/*
 * In this header file keep all flags and other 
 * structures that will be needed in both kernel
 * and user space. Specifically the ioctl flags
 * will go in here so that in user space a program 
 * can specify flags for the ioctl call.
 * 
 * author: Sean Ruyle
 * date:   06/11/2003
 * 
 */

#define TMOD_DRIVER_NAME	"ltp example module"
#define DEVICE_NAME		"tmod"
#define MAG_NUM			'k'

/* put ioctl flags here, use the _IO macro which is 
 found in linux/ioctl.h, takes a letter, and an 
 integer */

#define LTP_OPTION1		_IO(MAG_NUM, 1)
#define LTP_OTHER		_IO(MAG_NUM, 2)

/* memory between the kernel and user space is
 seperated, so that if a structure is needed 
 to be passed between kernel and user space 
 a call must be made to copy_to_user or copy
 from user. Use this structure to streamline 
 that process. For example: A function that 
 writes to a disc takes in a ki_write_t 
 pointer from userspace. In the user space 
 program specify the length of the pointer as 
 in_len, and in_data as the actual structure. */

struct tmod_interface {
	int     in_len;         // input data length
        caddr_t in_data;        // input data
        int     out_rc;         // return code from the test
        int     out_len;        // output data length
        caddr_t out_data;       // output data
};
typedef struct tmod_interface tmod_interface_t;
 

 


