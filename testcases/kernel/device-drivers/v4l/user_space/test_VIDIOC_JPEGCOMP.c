/*
 * v4l-test: Test environment for Video For Linux Two API
 *
 * 16 Jun 2009  0.1  First release
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

#include "v4l2_test.h"
#include "dev_video.h"
#include "video_limits.h"
#include "v4l2_validator.h"
#include "v4l2_foreach.h"

#include "test_VIDIOC_JPEGCOMP.h"

static int valid_jpeg_markers(__u32 jpeg_markers)
{
	int valid = 0;

	if ((jpeg_markers & ~(V4L2_JPEG_MARKER_DHT |
			      V4L2_JPEG_MARKER_DQT |
			      V4L2_JPEG_MARKER_DRI |
			      V4L2_JPEG_MARKER_COM | V4L2_JPEG_MARKER_APP))
	    == 0) {
		valid = 1;
	} else {
		valid = 0;
	}
	return valid;
}

void test_VIDIOC_G_JPEGCOMP()
{
	struct v4l2_jpegcompression jpegcomp;
	int ret_get, errno_get;

	memset(&jpegcomp, 0xff, sizeof(jpegcomp));
	ret_get = ioctl(get_video_fd(), VIDIOC_G_JPEGCOMP, &jpegcomp);
	errno_get = errno;

	dprintf("\tVIDIOC_G_JPEGCOMP, ret_get=%i, errno_get=%i\n", ret_get,
		errno_get);

	if (ret_get == 0) {
		CU_ASSERT_EQUAL(ret_get, 0);

		//CU_ASSERT_EQUAL(jpegcomp.quality, ???);
		//CU_ASSERT_EQUAL(jpegcomp.APPn, ???);
		CU_ASSERT(0 <= jpegcomp.APP_len);
		CU_ASSERT(jpegcomp.APP_len <= (int)sizeof(jpegcomp.APP_data));
		//CU_ASSERT_EQUAL(jpegcomp.APP_data, ???);
		CU_ASSERT(0 <= jpegcomp.COM_len);
		CU_ASSERT(jpegcomp.COM_len <= (int)sizeof(jpegcomp.COM_data));
		//CU_ASSERT_EQUAL(jpegcomp.COM_data, ???);
		CU_ASSERT(valid_jpeg_markers(jpegcomp.jpeg_markers));

		dprintf("\tjpegcomp = { .quality=%i, "
			".APPn=%i, "
			".APP_len=%i, "
			".APP_data=..., "
			".COM_len=%i, "
			".COM_data=..., "
			".jpeg_markers=0x%x ",
			jpegcomp.quality, jpegcomp.APPn, jpegcomp.APP_len,
			//jpegcomp.APP_data,
			jpegcomp.COM_len,
			//jpegcomp.COM_data,
			jpegcomp.jpeg_markers);

	} else {
		CU_ASSERT_EQUAL(ret_get, -1);
		CU_ASSERT_EQUAL(errno_get, EINVAL);
	}

}

void test_VIDIOC_G_JPEGCOMP_NULL()
{
	struct v4l2_jpegcompression jpegcomp;
	int ret_get, errno_get;
	int ret_null, errno_null;

	memset(&jpegcomp, 0, sizeof(jpegcomp));
	ret_get = ioctl(get_video_fd(), VIDIOC_G_JPEGCOMP, &jpegcomp);
	errno_get = errno;

	dprintf("\tVIDIOC_G_JPEGCOMP, ret_get=%i, errno_get=%i\n", ret_get,
		errno_get);

	ret_null = ioctl(get_video_fd(), VIDIOC_G_JPEGCOMP, NULL);
	errno_null = errno;

	dprintf("\tVIDIOC_G_JPEGCOMP, ret_null=%i, errno_null=%i\n", ret_null,
		errno_null);

	if (ret_get == 0) {
		CU_ASSERT_EQUAL(ret_get, 0);
		CU_ASSERT_EQUAL(ret_null, -1);
		CU_ASSERT_EQUAL(errno_null, EFAULT);
	} else {
		CU_ASSERT_EQUAL(ret_get, -1);
		CU_ASSERT_EQUAL(errno_get, EINVAL);
		CU_ASSERT_EQUAL(ret_null, -1);
		CU_ASSERT_EQUAL(errno_null, EINVAL);
	}

}
