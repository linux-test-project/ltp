/*
 * v4l-test: Test environment for Video For Linux Two API
 *
 * 21 Dec 2008  0.1  First release
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

#include "test_VIDIOC_CROPCAP.h"

static void do_ioctl_VIDIOC_CROPCAP(enum v4l2_buf_type buf_type, int expected_ret) {
	int ret;
	struct v4l2_cropcap cropcap;
	struct v4l2_cropcap cropcap2;

	memset(&cropcap, 0xff, sizeof(cropcap));
	cropcap.type = buf_type;
	ret = ioctl(get_video_fd(), VIDIOC_CROPCAP, &cropcap);

	if (expected_ret != 0) {
		CU_ASSERT_EQUAL(ret, expected_ret);
	}
	if (ret == 0) {
		CU_ASSERT_EQUAL(ret, 0);
		CU_ASSERT_EQUAL(cropcap.type, buf_type);

	/*     |   left                                   x   */
	/* ----+----+-------------------------------------->  */
	/*     |    :                                         */
	/* top +    +-------- cropcap ------------+  ^        */
	/*     |    |                             |  |        */
	/*     |    | +------- defrect ---------+ |  |        */
	/*     |    | |                         | |  |        */
	/*     |    | |                         | |  |        */
	/*     |    | |                         | |  | heigth */
	/*     |    | +-------------------------+ |  |        */
	/*     |    |                             |  |        */
	/*     |    |                             |  |        */
	/*     |    +-----------------------------+  v        */
	/*     |    :                             :           */
	/*     |    <---------- width ------------>           */
	/*     |                                              */
	/*     v y                                            */

		/* top left corner */
		CU_ASSERT(cropcap.bounds.left <= cropcap.defrect.left);
		CU_ASSERT(cropcap.bounds.top <= cropcap.defrect.top);

		/* size of default cropping rectangle should be smaller or */
		/* equal to the cropping bounds */
		CU_ASSERT(cropcap.defrect.width <= cropcap.bounds.width);
		CU_ASSERT(cropcap.defrect.height <= cropcap.bounds.height);

		/* the right bottom corner should not exceed bounds */
		CU_ASSERT(cropcap.defrect.left + cropcap.defrect.width <=
			  cropcap.bounds.left + cropcap.bounds.width);
		CU_ASSERT(cropcap.defrect.top + cropcap.defrect.height <=
			  cropcap.bounds.top + cropcap.bounds.height);

		//CU_ASSERT_EQUAL(cropcap.pixelaspect.numerator, ?);
		CU_ASSERT_NOT_EQUAL(cropcap.pixelaspect.numerator, 0);
		//CU_ASSERT_EQUAL(cropcap.pixelaspect.denominator, ?);
		CU_ASSERT_NOT_EQUAL(cropcap.pixelaspect.denominator, 0);

		dprintf("\tcropcap = { .type = %i, "
			".bounds = { .left = %i, .top = %i, .width = %i, .heigth = %i }, "
			".defrect = { .left = %i, .top = %i, .width = %i, .heigth = %i }, "
			".pixelaspect = { .numerator = %u, .denominator = %u } "
			"}\n",
			cropcap.type,

			cropcap.bounds.left,
			cropcap.bounds.top,
			cropcap.bounds.width,
			cropcap.bounds.height,

			cropcap.defrect.left,
			cropcap.defrect.top,
			cropcap.defrect.width,
			cropcap.defrect.height,

			cropcap.pixelaspect.numerator,
			cropcap.pixelaspect.denominator
			);


	} else {
		CU_ASSERT_EQUAL(ret, -1);
		CU_ASSERT_EQUAL(errno, EINVAL);

		memset(&cropcap2, 0xff, sizeof(cropcap2));
		cropcap2.type = buf_type;
		CU_ASSERT_EQUAL(memcmp(&cropcap, &cropcap2, sizeof(cropcap)), 0);
		dprintf("\ttype=%i, ret=%i, errno=%i, exoected_ret=%i\n", buf_type, ret, errno, expected_ret);

	}

}


void test_VIDIOC_CROPCAP() {

	do_ioctl_VIDIOC_CROPCAP(0, -1);
	do_ioctl_VIDIOC_CROPCAP(V4L2_BUF_TYPE_VIDEO_CAPTURE, 0);
	do_ioctl_VIDIOC_CROPCAP(V4L2_BUF_TYPE_VIDEO_OUTPUT, 0);
	do_ioctl_VIDIOC_CROPCAP(V4L2_BUF_TYPE_VIDEO_OVERLAY, 0);
	do_ioctl_VIDIOC_CROPCAP(V4L2_BUF_TYPE_VBI_CAPTURE, -1);
	do_ioctl_VIDIOC_CROPCAP(V4L2_BUF_TYPE_VBI_OUTPUT, -1);
	do_ioctl_VIDIOC_CROPCAP(V4L2_BUF_TYPE_SLICED_VBI_CAPTURE, -1);
	do_ioctl_VIDIOC_CROPCAP(V4L2_BUF_TYPE_SLICED_VBI_OUTPUT, -1);
	do_ioctl_VIDIOC_CROPCAP(V4L2_BUF_TYPE_VIDEO_OUTPUT_OVERLAY, -1);
	do_ioctl_VIDIOC_CROPCAP(V4L2_BUF_TYPE_PRIVATE-1, -1);
	do_ioctl_VIDIOC_CROPCAP(V4L2_BUF_TYPE_PRIVATE, 0);
	do_ioctl_VIDIOC_CROPCAP(V4L2_BUF_TYPE_PRIVATE+1, 0);
	do_ioctl_VIDIOC_CROPCAP(S32_MAX, 0);
	do_ioctl_VIDIOC_CROPCAP(((__u32)S32_MAX)+1, -1);
	do_ioctl_VIDIOC_CROPCAP(U32_MAX-1, -1);
	do_ioctl_VIDIOC_CROPCAP(U32_MAX, -1);

}

void test_VIDIOC_CROPCAP_enum_INPUT() {
	int ret;
	int enum_ret;
	__u32 input_index_orig;
	struct v4l2_input input;
	__u32 i;
	int f;

	f = get_video_fd();

	memset(&input_index_orig, 0xff, sizeof(input_index_orig));
	ret = ioctl(f, VIDIOC_G_INPUT, &input_index_orig);
	CU_ASSERT_EQUAL(ret, 0);
	if (ret == 0) {
		i = 0;
		do {
			memset(&input, 0xff, sizeof(input));
			input.index = i;
			enum_ret = ioctl(f, VIDIOC_ENUMINPUT, &input);

			dprintf("ENUMINPUT: i=%u, enum_ret=%i, errno=%i\n", i, enum_ret, errno);

			if (enum_ret == 0) {
				ret = ioctl(f, VIDIOC_S_INPUT, &input.index);
				CU_ASSERT_EQUAL(ret, 0);
				dprintf("input.index=0x%X, ret=%i, errno=%i\n", input.index, ret, errno);
				if (ret == 0) {
				    test_VIDIOC_CROPCAP();
				}

			}
			i++;
		} while (enum_ret == 0 && i != 0);

		/* Setting the original input_id should not fail */
		ret = ioctl(f, VIDIOC_S_INPUT, &input_index_orig);
		CU_ASSERT_EQUAL(ret, 0);
	}
}

void test_VIDIOC_CROPCAP_NULL() {
	int ret;

	ret = ioctl(get_video_fd(), VIDIOC_CROPCAP, NULL);

	CU_ASSERT_EQUAL(ret, -1);
	CU_ASSERT_EQUAL(errno, EFAULT);
	dprintf("\tret=%i, errno=%i\n", ret, errno);

}
