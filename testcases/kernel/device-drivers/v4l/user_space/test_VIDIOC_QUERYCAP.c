/*
 * v4l-test: Test environment for Video For Linux Two API
 *
 * 20 Apr 2009  0.5  Added string content validation
 * 18 Apr 2009  0.4  More strict check for strings
 * 29 Mar 2009  0.3  Clean up ret and errno variable names and dprintf() output
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
#include "v4l2_validator.h"

#include "test_VIDIOC_QUERYCAP.h"

int valid_capabilities(__u32 capabilities)
{
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
			      V4L2_CAP_ASYNCIO | V4L2_CAP_STREAMING)) != 0) {
		valid = 0;
	}

	return valid;
}

void test_VIDIOC_QUERYCAP()
{
	int ret;
	struct v4l2_capability cap;
	struct v4l2_capability cap2;

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
		cap.reserved[1], cap.reserved[2], cap.reserved[3]
	    );

	/* This ioctl must be implemented by ALL drivers */
	CU_ASSERT_EQUAL(ret, 0);
	if (ret == 0) {
		CU_ASSERT(0 < strlen((char *)cap.driver));
		CU_ASSERT(valid_string((char *)cap.driver, sizeof(cap.driver)));

		CU_ASSERT(0 < strlen((char *)cap.card));
		CU_ASSERT(valid_string((char *)cap.card, sizeof(cap.card)));

		/* cap.bus_info is allowed to be an empty string ("") if no
		 * is info available
		 */
		CU_ASSERT(valid_string
			  ((char *)cap.bus_info, sizeof(cap.bus_info)));

		//CU_ASSERT_EQUAL(cap.version, ?);
		CU_ASSERT(valid_capabilities(cap.capabilities));

		CU_ASSERT_EQUAL(cap.reserved[0], 0);
		CU_ASSERT_EQUAL(cap.reserved[1], 0);
		CU_ASSERT_EQUAL(cap.reserved[2], 0);
		CU_ASSERT_EQUAL(cap.reserved[3], 0);

		/* Check if the unused bytes of the driver, card and bus_info
		 * strings are also filled with zeros. Also check if there is
		 * any padding byte between any two fields then this padding
		 * byte is also filled with zeros.
		 */
		memset(&cap2, 0, sizeof(cap2));
		strncpy((char *)cap2.driver, (char *)cap.driver,
			sizeof(cap2.driver));
		strncpy((char *)cap2.card, (char *)cap.card, sizeof(cap2.card));
		strncpy((char *)cap2.bus_info, (char *)cap.bus_info,
			sizeof(cap2.bus_info));
		cap2.version = cap.version;
		cap2.capabilities = cap.capabilities;
		CU_ASSERT_EQUAL(memcmp(&cap, &cap2, sizeof(cap)), 0);

	}

}

void test_VIDIOC_QUERYCAP_NULL()
{
	int ret_null, errno_null;

	ret_null = ioctl(get_video_fd(), VIDIOC_QUERYCAP, NULL);
	errno_null = errno;

	dprintf("\t%s:%u: VIDIOC_QUERYCAP, ret_null=%i, errno_null=%i\n",
		__FILE__, __LINE__, ret_null, errno_null);

	/* VIDIOC_QUERYCAP is a mandatory command, all drivers shall
	 * support it. The parameter shall be always tested.
	 */
	CU_ASSERT_EQUAL(ret_null, -1);
	CU_ASSERT_EQUAL(errno_null, EFAULT);

}
