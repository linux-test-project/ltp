/*
 *
 *   Copyright (c) International Business Machines  Corp., 2001
 *
 *   This program is free software;  you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY;  without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 *   the GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program;  if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 *
*/
//extern struct usb_bus *usb_alloc_bus(struct usb_operations *);
extern void usb_connect(struct usb_device *dev);
extern int usb_new_device(struct usb_device *dev);
extern void usb_free_bus(struct usb_bus *bus);
extern void usb_register_bus(struct usb_bus *bus);
extern void usb_deregister_bus(struct usb_bus *bus);
extern int usb_hcd_pci_probe(struct pci_dev *, const struct pci_device_id *);
extern void usb_hcd_pci_remove(struct pci_dev *);
extern int usb_hcd_pci_suspend(struct pci_dev *, u32);
extern int usb_hcd_pci_resume(struct pci_dev *);

#ifndef usb_operations
	struct usb_operations {
	        int (*allocate)(struct usb_device *);
	        int (*deallocate)(struct usb_device *);
	        int (*get_frame_number) (struct usb_device *dev);
	        int (*submit_urb) (struct urb *urb);
	        int (*unlink_urb) (struct urb *urb);

		/* allocate dma-consistent buffer for URB_DMA_NOMAPPING */
	        void *(*buffer_alloc)(struct usb_bus *bus, size_t size,
        	                int mem_flags,
        	                dma_addr_t *dma);
        	void (*buffer_free)(struct usb_bus *bus, size_t size,
                	        void *addr, dma_addr_t dma);
	};
#endif

#ifndef usb_hcd
	struct usb_hcd {
		struct usb_bus          self;           /* hcd is-a bus */
	        const char              *product_desc;  /* product/vendor string */
	        const char              *description;   /* "ehci-hcd" etc */
	        struct timer_list       rh_timer;       /* drives root hub */
	        struct list_head        dev_list;       /* devices on this bus */
	        struct work_struct      work;
	        struct hc_driver        *driver;        /* hw-specific hooks */
	        int                     irq;            /* irq allocated */
	        void                    *regs;          /* device memory/io */
	        struct device           *controller;    /* handle to hardware */
	        struct pci_dev          *pdev;          /* pci is typical */
	        int                     region;         /* pci region for regs */
	        u32                     pci_state [16]; /* for PM state save */
	        atomic_t                resume_count;   /* multiple resumes issue */
	        struct pci_pool         *pool [4];
	        int                     state;
	};
#endif


#ifndef hc_driver
	struct hc_driver {
		const char      *description;   /* "ehci-hcd" etc */
        	void    (*irq) (struct usb_hcd *hcd, struct pt_regs *regs);
	        int     flags;
        	int     (*start) (struct usb_hcd *hcd);
        	int     (*suspend) (struct usb_hcd *hcd, __u32 state);
        	int     (*resume) (struct usb_hcd *hcd);
        	void    (*stop) (struct usb_hcd *hcd);
        	int     (*get_frame_number) (struct usb_hcd *hcd);
        	struct usb_hcd  *(*hcd_alloc) (void);
        	void            (*hcd_free) (struct usb_hcd *hcd);
	        int     (*urb_enqueue) (struct usb_hcd *hcd, struct urb *urb,
				int mem_flags);
        	int     (*urb_dequeue) (struct usb_hcd *hcd, struct urb *urb);
        	void            (*free_config) (struct usb_hcd *hcd,
                                struct usb_device *dev);

		int     (*hub_status_data) (struct usb_hcd *hcd, char *buf);
		int     (*hub_control) (struct usb_hcd *hcd,
                                u16 typeReq, u16 wValue, u16 wIndex,
                                char *buf, u16 wLength);
	};
#endif

#ifndef USB_MAXBUS
	#define USB_MAXBUS              64
#endif

struct tusb_user {
	struct usb_bus		*bus;
	struct usb_device	*dev;
	struct pci_dev		*pdev;
};
typedef struct tusb_user tusb_user_t;

