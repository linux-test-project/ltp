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

 * Remember that you want to seperate your header
 * files between what is needed in kernel space
 * only, and what will also be needed by a user
 * space program that is using this module. For
 * that reason keep all structures that will need
 * kernel space pointers in a seperate header file
 * from where ioctl flags aer kept
 *
 * author: Kai Zhao
 * date:   09/03/2003
 *
 */

/* test function export to user space*/
extern int tdrm_test_interface(struct inode *inode, struct file *filp,
			unsigned int cmd, unsigned long arg);
extern int tdrm_test_stub_register(struct inode *inode,struct file *filp,
		unsigned int cmd , unsigned long arg);
extern int tdrm_test_stub_unregister(struct inode *inode,struct file *filp,
		unsigned int cmd , unsigned long arg);
extern int tdrm_test_uninit_agp(struct inode *inode,struct file *filp,
		unsigned int cmd , unsigned long arg);
extern int tdrm_test_init_agp(struct inode *inode,struct file *filp,
		unsigned int cmd , unsigned long arg);
extern int tdrm_test_add_magic(struct inode *inode,struct file *filp,
		unsigned int cmd , unsigned long arg);
extern int tdrm_test_remove_magic(struct inode *inode,struct file *filp,
		unsigned int cmd , unsigned long arg);
extern int tdrm_test_ctxbitmap_init(struct inode *inode,struct file *filp,
		unsigned int cmd , unsigned long arg);
extern int tdrm_test_ctxbitmap_cleanup(struct inode *inode,struct file *filp,
		unsigned int cmd , unsigned long arg);
extern int tdrm_test_alloc_pages(struct inode *inode,struct file *filp,
		unsigned int cmd , unsigned long arg);
extern int tdrm_test_free_pages(struct inode *inode,struct file *filp,
		unsigned int cmd , unsigned long arg);



typedef int tdrm_ioctl_t( struct inode *inode, struct file *filp,
			 unsigned int cmd, unsigned long arg );

typedef struct tdrm_ioctl_desc {
	tdrm_ioctl_t	     *func;
	int		     auth_needed;
	int		     root_only;
} tdrm_ioctl_desc_t;

