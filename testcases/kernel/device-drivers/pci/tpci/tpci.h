//tpci.h

#define TPCI_TEST_DRIVER_NAME	"pci test module"
#define TPCI_MAJOR      252
#define DEVICE_NAME		"/dev/tpci"
#define MAX_DEVFN		256
#define MAX_BUS			256
#define MAG_NUM 		'k'

#define PCI_PROBE		_IO(MAG_NUM, 1)
#define PCI_ENABLE		_IO(MAG_NUM, 2)
#define PCI_DISABLE		_IO(MAG_NUM, 3)
#define FIND_BUS	        _IO(MAG_NUM, 4)
#define FIND_DEVICE             _IO(MAG_NUM, 5)
#define FIND_CLASS              _IO(MAG_NUM, 6)
#define FIND_SUBSYS             _IO(MAG_NUM, 7)
#define BUS_SCAN		_IO(MAG_NUM, 8)
#define	SLOT_SCAN		_IO(MAG_NUM, 9)
#define ENABLE_BRIDGES		_IO(MAG_NUM, 10)
#define BUS_ADD_DEVICES		_IO(MAG_NUM, 11)
#define MATCH_DEVICE		_IO(MAG_NUM, 12)
#define REG_DRIVER		_IO(MAG_NUM, 13)
#define UNREG_DRIVER		_IO(MAG_NUM, 14)
#define BUS_RESOURCES		_IO(MAG_NUM, 15)
#define PCI_RESOURCES		_IO(MAG_NUM, 16)
#define SAVE_STATE		_IO(MAG_NUM, 19)
#define RESTORE_STATE		_IO(MAG_NUM, 20)
#define TEST_MAX_BUS		_IO(MAG_NUM, 21)
#define FIND_CAP		_IO(MAG_NUM, 22)
#ifndef SET_MODULE_OWNER
#define SET_MODULE_OWNER(dev) ((dev)->owner = THIS_MODULE)
#endif

/*
 * structures for PCI test driver
 */
struct tpci_interface {
	int 	in_len;		// input data length
	caddr_t	in_data;	// input data
	int 	out_rc;		// return code from the test 
	int 	out_len;	// output data length 
	caddr_t	out_data;	// output data 
};
typedef struct tpci_interface tpci_interface_t;


