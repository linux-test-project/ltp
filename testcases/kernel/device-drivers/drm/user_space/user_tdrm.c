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

 * This is the main of your user space test program,
 * which will open the correct kernel module, find the
 * file descriptor value and use that value to make
 * ioctl calls to the system
 *
 *
 * author: Kai Zhao
 * date:   09/03/2003
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <linux/kernel.h>
#include <linux/errno.h>

#include "../kernel_space/tdrm.h"
#include "drm.h"

static int tdrm_fd = -1;	/* file descriptor */

int tdrmopen()
{

	dev_t devt;
	struct stat st;
	int rc = 0;

	devt = makedev(TDRM_MAJOR, 0);

	if (rc) {
		if (errno == ENOENT) {
			/* dev node does not exist. */
			rc = mkdir(DEVICE_NAME, (S_IFDIR | S_IRWXU |
						 S_IRGRP | S_IXGRP |
						 S_IROTH | S_IXOTH));
		} else {
			printf
			    ("ERROR: Problem with Base dev directory.  Error code from stat() is %d\n\n",
			     errno);
		}

	} else {
		if (!(st.st_mode & S_IFDIR)) {
			rc = unlink(DEVICE_NAME);
			if (!rc) {
				rc = mkdir(DEVICE_NAME, (S_IFDIR | S_IRWXU |
							 S_IRGRP | S_IXGRP |
							 S_IROTH | S_IXOTH));
			}
		}
	}

	/*
	 * Check for the /dev/tbase node, and create if it does not
	 * exist.
	 */
	rc = stat(DEVICE_NAME, &st);
	if (rc) {
		if (errno == ENOENT) {
			/* dev node does not exist */
			rc = mknod(DEVICE_NAME,
				   (S_IFCHR | S_IRUSR | S_IWUSR | S_IRGRP |
				    S_IWGRP), devt);
		} else {
			printf
			    ("ERROR:Problem with tbase device node directory.  Error code form stat() is %d\n\n",
			     errno);
		}

	} else {
		/*
		 * /dev/tbase CHR device exists.  Check to make sure it is for a
		 * block device and that it has the right major and minor.
		 */
		if ((!(st.st_mode & S_IFCHR)) || (st.st_rdev != devt)) {

			/* Recreate the dev node. */
			rc = unlink(DEVICE_NAME);
			if (!rc) {
				rc = mknod(DEVICE_NAME,
					   (S_IFCHR | S_IRUSR | S_IWUSR |
					    S_IRGRP | S_IWGRP), devt);
			}
		}
	}

	tdrm_fd = open(DEVICE_NAME, O_RDWR);

	if (tdrm_fd < 0) {
		printf("ERROR: Open of device %s failed %d errno = %d\n",
		       DEVICE_NAME, tdrm_fd, errno);
		return errno;
	} else {
		printf("Device opened successfully \n");
		return 0;
	}

}

int tdrmclose()
{

	if (tdrm_fd != -1) {
		close(tdrm_fd);
		tdrm_fd = -1;
	}
}

int main()
{
	int rc;

	/* open the module */
	rc = tdrmopen();
	if (rc) {
		printf("Test DRM Driver may not be loaded\n");
		exit(1);
	}

	printf("\tBegin drm read system ioctl\n");
	//
	drm_auth_t auth;
	if (ioctl(tdrm_fd, DRM_IOCTL_GET_MAGIC, &auth))
		printf("Failed on drm ioctl DRM(get_magic) test\n");
	else
		printf("Success on drm ioctl DRM(get_magic) test\n");

	//
	drm_stats_t stats;
	if (ioctl(tdrm_fd, DRM_IOCTL_GET_STATS, &stats))
		printf("Failed on drm ioctl DRM(get_status) test\n");
	else
		printf("Success on drm ioctl DRM(get_status) test\n");

	/*--------------------------------------------------------------
	 *                      for drm read write ioctl test
	 */

	printf("\tBegin read write ioctl test\n");
	if (ioctl(tdrm_fd, DRM_IOCTL_IRQ_BUSID))
		printf("Failed on drm ioctl DRM(irq_busid)\n");
	else
		printf("Success on drm ioctl DRM(irq_busid)\n");
	if (ioctl(tdrm_fd, DRM_IOCTL_GET_CLIENT))
		printf("Failed on drm ioctl DRM(get_client)\n");
	else
		printf("Success on drm ioctl DRM(get_client)\n");
	if (ioctl(tdrm_fd, DRM_IOCTL_BLOCK))
		printf("Failed on drm ioctl DRM(block)\n");
	else
		printf("Success on drm ioctl DRM(block)\n");
	if (ioctl(tdrm_fd, DRM_IOCTL_UNBLOCK))
		printf("Failed on drm ioctl DRM(unblock)\n");
	else
		printf("Success on drm ioctl DRM(unblock)\n");
	if (ioctl(tdrm_fd, DRM_IOCTL_ADD_CTX))
		printf("Failed on drm ioctl DRM(addctx)\n");
	else
		printf("Success on drm ioctl DRM(addctx)\n");
	if (ioctl(tdrm_fd, DRM_IOCTL_RM_CTX))
		printf("Failed on drm ioctl DRM(rmctx)\n");
	else
		printf("Success on drm ioctl DRM(rmctx)\n");
	if (ioctl(tdrm_fd, DRM_IOCTL_GET_CTX))
		printf("Failed on drm ioctl DRM(getctx)\n");
	else
		printf("Success on drm ioctl DRM(getctx)\n");
	if (ioctl(tdrm_fd, DRM_IOCTL_ADD_DRAW))
		printf("Failed on drm ioctl DRM(adddraw)\n");
	else
		printf("Success on drm ioctl DRM(adddraw)\n");
	if (ioctl(tdrm_fd, DRM_IOCTL_RM_DRAW))
		printf("Failed on drm ioctl DRM(rmdraw)\n");
	else
		printf("Success on drm ioctl DRM(rmdraw)\n");

	/* test drm stub_register */

	if (ioctl(tdrm_fd, TDRM_STUB_REGISTER))
		printf("Failed on drm stub_register test\n");
	else
		printf("Success on drm stub_register test\n");

	// test drm DRM(agp_uninit)

	if (ioctl(tdrm_fd, TDRM_UNINIT_AGP))
		printf("Failed on DRM(agp_uninit) test\n");
	else
		printf("Success on DRM(agp_uninit) test\n");

	// test drm DRM(agp_init)

	if (ioctl(tdrm_fd, TDRM_INIT_AGP))
		printf("Failed on DRM(agp_init) test\n");
	else
		printf("Success on DRM(agp_init) test\n");

	// test drm stub_unregister

	if (ioctl(tdrm_fd, TDRM_STUB_UNREGISTER))
		printf("Failed on drm stub_unregister test\n");
	else
		printf("Success on drm stub_unregister test\n");
	// test drm add magic
	if (ioctl(tdrm_fd, TDRM_ADD_MAGIC))
		printf("Failed on drm DRM(add_magic) test\n");
	else
		printf("Success on drm DRM(add_magic) test\n");

	// test drm remove magic
	if (ioctl(tdrm_fd, TDRM_REMOVE_MAGIC))
		printf("Failed on drm DRM(remove_magic) test\n");
	else
		printf("Success on drm DRM(remove_magic) test\n");

	// test drm alloc pages
	if (ioctl(tdrm_fd, TDRM_ALLOC_PAGES))
		printf("Failed on drm DRM(alloc_pages) test\n");
	else
		printf("Success on drm DRM(alloc_pages) test\n");
	// test drm free pages
	if (ioctl(tdrm_fd, TDRM_FREE_PAGES))
		printf("Failed on drm DRM(free_pages) test\n");
	else
		printf("Success on drm DRM(free_pages) test\n");
	// test drm ctxbitmap_cleanup
	if (ioctl(tdrm_fd, TDRM_CTXBITMAP_UNINIT))
		printf("Failed on drm DRM(ctxbitmap_cleanup) test\n");
	else
		printf("Success on drm DRM(ctxbitmap_cleanup) test\n");

	// test drm ctxbitmap init
	if (ioctl(tdrm_fd, TDRM_CTXBITMAP_INIT))
		printf("Failed on drm DRM(ctxbitmap_init) test\n");
	else
		printf("Success on drm DRM(ctxbitmap_init) test\n");

	static drm_version_t version;
	if (ioctl(tdrm_fd, DRM_IOCTL_VERSION, &version))
		printf("Failed on drm DRM(version) test\n");
	else
		printf("Success on drm DRM(version) test\n");

	static drm_auth_t myauth;
	if (ioctl(tdrm_fd, DRM_IOCTL_GET_MAGIC, &myauth))
		printf("Failed on drm DRM(get_magic) test\n");
	else
		printf("Success on drm DRM(get_magic) test\n");

	if (ioctl(tdrm_fd, DRM_IOCTL_AUTH_MAGIC, &auth))
		printf("Failed on drm authmagic test\n");
	else
		printf("Success on drm authmagic test\n");

	//  test for drm_agpsupport.h
	/*
	   if (ioctl(tdrm_fd, DRM_IOCTL_AGP_RELEASE))
	   printf("Failed on drm DRM(agp_release) test\n");
	   else
	   printf("Success on drm DRM(agp_release) test\n");
	 */
	if (ioctl(tdrm_fd, DRM_IOCTL_AGP_ACQUIRE))
		printf("Failed on drm DRM(agp_acquire) test\n");
	else
		printf("Success on drm DRM(agp_acquire) test\n");

	if (ioctl(tdrm_fd, DRM_IOCTL_AGP_RELEASE))
		printf("Failed on drm DRM(agp_release) test\n");
	else
		printf("Success on drm DRM(agp_release) test\n");

	if (ioctl(tdrm_fd, DRM_IOCTL_AGP_ACQUIRE))
		printf("Failed on drm DRM(agp_acquire) test\n");
	else
		printf("Success on drm DRM(agp_acquire) test\n");

	static drm_agp_info_t agp_info;
	if (ioctl(tdrm_fd, DRM_IOCTL_AGP_INFO, &agp_info))
		printf("Failed on drm DRM(agp_info) test\n");
	else
		printf("Success on drm DRM(agp_info) test\n");

	static drm_agp_buffer_t agp_buffer;
	agp_buffer.size = 64;
	if (ioctl(tdrm_fd, DRM_IOCTL_AGP_ALLOC, &agp_buffer))
		printf("Failed on drm DRM(agp_alloc) test\n");
	else
		printf("Success on drm DRM(agp_alloc) test\n");

	static drm_agp_binding_t bind_buffer;
	bind_buffer.handle = agp_buffer.handle;
	bind_buffer.offset = 64;

	if (ioctl(tdrm_fd, DRM_IOCTL_AGP_BIND, &bind_buffer))
		printf("Failed on drm DRM(agp_bind) test\n");
	else
		printf("Success on drm DRM(agp_bind) test\n");

	if (ioctl(tdrm_fd, DRM_IOCTL_AGP_UNBIND, &bind_buffer))
		printf("Failed on drm DRM(agp_unbind) test\n");
	else
		printf("Success on drm DRM(agp_unbind) test\n");
	if (ioctl(tdrm_fd, DRM_IOCTL_AGP_FREE, &agp_buffer))
		printf("Failed on drm DRM(agp_free) test\n");
	else
		printf("Success on drm DRM(agp_free) test\n");

	// test drm_ctxbitmap.h
	static drm_ctx_t getctx;
	if (ioctl(tdrm_fd, DRM_IOCTL_GET_CTX, &getctx))
		printf("Failed on drm DRM(getctx) test\n");
	else
		printf("Success on drm DRM(getctx) test\n");

	static drm_ctx_t ctx;
	if (ioctl(tdrm_fd, DRM_IOCTL_ADD_CTX, &ctx))
		printf("Failed on drm DRM(addctx) test\n");
	else
		printf("Success on drm DRM(addctx) test\n");

	if (ioctl(tdrm_fd, DRM_IOCTL_RM_CTX, &ctx))
		printf("Failed on drm DRM(rmctx) test\n");
	else
		printf("Success on drm DRM(rmctx) test\n");

	/*
	   static drm_ctx_priv_map_t map;
	   memset(&map,0,sizeof(drm_ctx_priv_map_t));
	   if (ioctl(tdrm_fd, DRM_IOCTL_GET_SAREA_CTX,&map))
	   printf("Failed on drm DRM(getsareactx) test\n");
	   else
	   printf("Success on drm DRM(getsareactx) test\n");
	 */
	// for drm_drawtable.h
	static drm_draw_t draw;
	if (ioctl(tdrm_fd, DRM_IOCTL_ADD_DRAW, &draw))
		printf("Failed on drm DRM(adddraw) test\n");
	else
		printf("Success on drm DRM(adddraw) test\n");

	if (ioctl(tdrm_fd, DRM_IOCTL_RM_DRAW, &draw))
		printf("Failed on drm DRM(rmdraw) test\n");
	else
		printf("Success on drm DRM(rmdraw) test\n");

	//for drm_ioctl.h//
	static drm_stats_t status;
	if (ioctl(tdrm_fd, DRM_IOCTL_GET_STATS, &status))
		printf("Failed on drm DRM(getstatus) test\n");
	else
		printf("Success on drm DRM(getstatus) test\n");

	static drm_client_t client;
	if (ioctl(tdrm_fd, DRM_IOCTL_GET_CLIENT, &client))
		printf("Failed on drm DRM(getclient) test\n");
	else
		printf("Success on drm DRM(getclient) test\n");
	/*
	   static drm_map_t getmap;
	   getmap.offset = 0;
	   if (ioctl(tdrm_fd, DRM_IOCTL_GET_MAP,&getmap))
	   printf("Failed on drm DRM(getmap) test\n");
	   else
	   printf("Success on drm DRM(getmap) test\n");
	 */
	static drm_unique_t unique;
	unique.unique_len = 0;
	if (ioctl(tdrm_fd, DRM_IOCTL_GET_UNIQUE, &unique))
		printf("Failed on drm DRM(getunique) test\n");
	else
		printf("Success on drm DRM(getunique) test\n");
	/*
	   if (ioctl(tdrm_fd, DRM_IOCTL_SET_UNIQUE,&unique))
	   printf("Failed on drm DRM(setunique) test\n");

	   else
	   printf("Success on drm DRM(setunique) test\n");

	 */
	rc = tdrmclose();
	if (rc) {
		printf("Test MOD Driver may not be closed\n");
		exit(1);
	}

}
