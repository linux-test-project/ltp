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

 * In this header file keep all flags and other
 * structures that will be needed in both kernel
 * and user space. Specifically the ioctl flags
 * will go in here so that in user space a program
 * can specify flags for the ioctl call.
 *
 * author: Kai Zhao
 * date:   09/03/2003
 *
 */
#ifndef __TDRM_H
#define __TDRM_H

#define TMOD_DRIVER_NAME	"ltp drm module"
#define DEVICE_NAME		"/dev/tdrm"
#define DRIVER_NAME     DEVICE_NAME

#define MAG_NUM			'k'
#define TDRM_MAJOR      2
#define DRIVER_MAJOR    TDRM_MAJOR
#define DRIVER_MINOR     0
#define DRIVER_PATCHLEVEL 0

/*--------------------------------------------------------------------------*/
/* This remains constant for all DRM template files.
 */
#define DRM(x) tdrm_##x

/* General customization:
 */
#define __HAVE_MTRR		1
#define __HAVE_CTX_BITMAP	1


#define __HAVE_AGP		1
#define __MUST_HAVE_AGP		0
//#define __HAVE_DMA              1



//#define TDRM_DRM_IOCTL_INTERFACE          DRM_IOW( 0x40, drm_tdrm_interface_t)
#define TDRM_STUB_REGISTER                DRM_IO( 0x41 )
#define TDRM_STUB_UNREGISTER              DRM_IO( 0x42 )
#define TDRM_UNINIT_AGP                   DRM_IO( 0x43 )
#define TDRM_INIT_AGP                     DRM_IO( 0x44 )
#define TDRM_ADD_MAGIC                    DRM_IO( 0x45 )
#define TDRM_REMOVE_MAGIC                 DRM_IO( 0x46 )
#define TDRM_CTXBITMAP_INIT               DRM_IO( 0x47 )
#define TDRM_CTXBITMAP_UNINIT             DRM_IO( 0x48 )
#define TDRM_ALLOC_PAGES                  DRM_IO( 0x49 )
#define TDRM_FREE_PAGES                   DRM_IO( 0x50 )

#define DRIVER_IOCTLS							    \
	[DRM_IOCTL_NR(TDRM_STUB_REGISTER)] = {tdrm_test_stub_register , 1 , 1 }, \
	[DRM_IOCTL_NR(TDRM_STUB_UNREGISTER)] = {tdrm_test_stub_unregister , 1 , 1 }, \
	[DRM_IOCTL_NR(TDRM_UNINIT_AGP)] = {tdrm_test_uninit_agp , 1 , 1 }, \
	[DRM_IOCTL_NR(TDRM_INIT_AGP)] = {tdrm_test_init_agp , 1 , 1 }, \
	[DRM_IOCTL_NR(TDRM_ADD_MAGIC)] = {tdrm_test_add_magic , 1 , 1 }, \
	[DRM_IOCTL_NR(TDRM_REMOVE_MAGIC)] = {tdrm_test_remove_magic , 1 , 1 }, \
	[DRM_IOCTL_NR(TDRM_CTXBITMAP_INIT)] = {tdrm_test_ctxbitmap_init , 1 , 1 }, \
	[DRM_IOCTL_NR(TDRM_CTXBITMAP_UNINIT)] = {tdrm_test_ctxbitmap_cleanup , 1 , 1 }, \
	[DRM_IOCTL_NR(TDRM_ALLOC_PAGES)] = {tdrm_test_alloc_pages , 1 , 1 }, \
	[DRM_IOCTL_NR(TDRM_FREE_PAGES)] = {tdrm_test_free_pages , 1 , 1 }

#endif

