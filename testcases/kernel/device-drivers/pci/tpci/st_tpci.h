//st_tpci.h

struct tpci_user {
	struct pci_dev 		*dev;
	struct pci_bus 		*bus;
	struct pci_driver 	*drv;
	uint32_t		state[16];
};
typedef struct tpci_user tpci_user_t;
