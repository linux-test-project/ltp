/*
 * v4l-test: Test environment for Video For Linux Two API
 *
 * 22 Dec 2008  0.2  Test case with NULL parameter added
 * 18 Dec 2008  0.1  First release
 *
 * Written by Márton Németh <nm127@freemail.hu>
 * Released under GPL
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <string.h>

#include <linux/videodev2.h>
#include <linux/errno.h>

#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>

#include "v4l2_test.h"
#include "dev_video.h"
#include "video_limits.h"

#include "test_VIDIOC_QUERYCAP.h"

int valid_capabilities(__u32 capabilities) {
	int valid = 1;

	if ((capabilities & ~(V4L2_CAP_VIDEO_CAPTURE |
			     V4L2_CAP_VIDEO_OUTPUT |
			     V4L2_CAP_VIDEO_OVERLAY |
			     V4L2_CAP_VBI_CAPTURE |
			     V4L2_CAP_VBI_OUTPUT |
			     V4L2_CAP_SLICED_VBI_CAPTURE |
			     V4L2_CAP_SLICED_VBI_OUTPUT |
			     V4L2_CAP_RDS_CAPTURE |
			     V4L2_CAP_VIDEO_OUTPUT_OVERLAY |
			     V4L2_CAP_TUNER |
			     V4L2_CAP_AUDIO |
			     V4L2_CAP_RADIO |
			     V4L2_CAP_READWRITE |
			     V4L2_CAP_ASYNCIO |
			     V4L2_CAP_STREAMING)) != 0) {
		valid = 0;
	}

	return valid;
}


void test_VIDIOC_QUERYCAP() {
	int ret;
	struct v4l2_capability cap;

	memset(&cap, 0xff, sizeof(cap));

	ret = ioctl(get_video_fd(), VIDIOC_QUERYCAP, &cap);

	dprintf("VIDIOC_QUERYCAP, ret=%i\n", ret);
	dprintf("\tcap = { .driver = \"%s\", .card = \"%s\", "
		".bus_info = \"%s\", "
		".version = %u.%u.%u, "
		".capabilities = 0x%X, "
		".reserved[]={ 0x%X, 0x%X, 0x%X, 0x%X } }\n",
		cap.driver,
		cap.card,
		cap.bus_info,
		(cap.version >> 16) & 0xFF,
		(cap.version >> 8) & 0xFF,
		cap.version & 0xFF,
		cap.capabilities,
		cap.reserved[0],
		cap.reserved[1],
		cap.reserved[2],
		cap.reserved[3]
	);

	/* This ioctl must be implemented by ALL drivers */
	CU_ASSERT_EQUAL(ret, 0);
	if (ret == 0) {
		//CU_ASSERT_EQUAL(cap.driver, ?);
		CU_ASSERT(0 < strlen( (char*)cap.driver) );

		//CU_ASSERT_EQUAL(cap.card, ?);
		CU_ASSERT(0 < strlen( (char*)cap.card) );

		//CU_ASSERT_EQUAL(cap.bus_info, ?);

		//CU_ASSERT_EQUAL(cap.version, ?);
		CU_ASSERT(valid_capabilities(cap.capabilities));

		CU_ASSERT_EQUAL(cap.reserved[0], 0);
		CU_ASSERT_EQUAL(cap.reserved[1], 0);
		CU_ASSERT_EQUAL(cap.reserved[2], 0);
		CU_ASSERT_EQUAL(cap.reserved[3], 0);

	}

}

void test_VIDIOC_QUERYCAP_NULL() {
	int ret;

	ret = ioctl(get_video_fd(), VIDIOC_QUERYCAP, NULL);
	CU_ASSERT_EQUAL(ret, -1);
	CU_ASSERT_EQUAL(errno, EFAULT);

}
