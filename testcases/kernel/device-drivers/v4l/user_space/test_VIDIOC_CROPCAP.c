/*
 * v4l-test: Test environment for Video For Linux Two API
 *
 *  3 Apr 2009  0.6  Test case for with NULL parameter reworked
 * 28 Mar 2009  0.5  Clean up ret and errno variable names and dprintf() output
 *  7 Mar 2009  0.4  Typo corrected
 *  9 Feb 2009  0.3  Modify test_VIDIOC_CROPCAP_enum_INPUT() to support drivers
 *                   without any inputs
 *  3 Feb 2009  0.2  Typo fixed
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

static void do_ioctl_VIDIOC_CROPCAP(enum v4l2_buf_type buf_type,
				    int expected_ret)
{
	int ret_cap, errno_cap;
	struct v4l2_cropcap cropcap;
	struct v4l2_cropcap cropcap2;

	memset(&cropcap, 0xff, sizeof(cropcap));
	cropcap.type = buf_type;
	ret_cap = ioctl(get_video_fd(), VIDIOC_CROPCAP, &cropcap);
	errno_cap = errno;

	dprintf("\t%s:%u: type=%i, ret_cap=%i, errno_cap=%i, expected_ret=%i\n",
		__FILE__, __LINE__, buf_type, ret_cap, errno_cap, expected_ret);

	if (expected_ret != 0) {
		CU_ASSERT_EQUAL(ret_cap, expected_ret);
	}
	if (ret_cap == 0) {
		CU_ASSERT_EQUAL(ret_cap, 0);
		CU_ASSERT_EQUAL(cropcap.type, buf_type);

		/*     |   left                                   x   */
		/* ----+----+-------------------------------------->  */
		/*     |    :                                         */
		/* top +    +-------- cropcap ------------+  ^        */
		/*     |    |                             |  |        */
		/*     |    | +------- defrect ---------+ |  |        */
		/*     |    | |                         | |  |        */
		/*     |    | |                         | |  |        */
		/*     |    | |                         | |  | height */
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
			".bounds = { .left = %i, .top = %i, .width = %i, .height = %i }, "
			".defrect = { .left = %i, .top = %i, .width = %i, .height = %i }, "
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
			cropcap.pixelaspect.denominator);

	} else {
		CU_ASSERT_EQUAL(ret_cap, -1);
		CU_ASSERT_EQUAL(errno_cap, EINVAL);

		memset(&cropcap2, 0xff, sizeof(cropcap2));
		cropcap2.type = buf_type;
		CU_ASSERT_EQUAL(memcmp(&cropcap, &cropcap2, sizeof(cropcap)),
				0);

	}

}

void test_VIDIOC_CROPCAP()
{

	do_ioctl_VIDIOC_CROPCAP(0, -1);
	do_ioctl_VIDIOC_CROPCAP(V4L2_BUF_TYPE_VIDEO_CAPTURE, 0);
	do_ioctl_VIDIOC_CROPCAP(V4L2_BUF_TYPE_VIDEO_OUTPUT, 0);
	do_ioctl_VIDIOC_CROPCAP(V4L2_BUF_TYPE_VIDEO_OVERLAY, 0);
	do_ioctl_VIDIOC_CROPCAP(V4L2_BUF_TYPE_VBI_CAPTURE, -1);
	do_ioctl_VIDIOC_CROPCAP(V4L2_BUF_TYPE_VBI_OUTPUT, -1);
	do_ioctl_VIDIOC_CROPCAP(V4L2_BUF_TYPE_SLICED_VBI_CAPTURE, -1);
	do_ioctl_VIDIOC_CROPCAP(V4L2_BUF_TYPE_SLICED_VBI_OUTPUT, -1);
	do_ioctl_VIDIOC_CROPCAP(V4L2_BUF_TYPE_VIDEO_OUTPUT_OVERLAY, -1);
	do_ioctl_VIDIOC_CROPCAP(V4L2_BUF_TYPE_PRIVATE - 1, -1);
	do_ioctl_VIDIOC_CROPCAP(V4L2_BUF_TYPE_PRIVATE, 0);
	do_ioctl_VIDIOC_CROPCAP(V4L2_BUF_TYPE_PRIVATE + 1, 0);
	do_ioctl_VIDIOC_CROPCAP(S32_MAX, -1);
	do_ioctl_VIDIOC_CROPCAP(((__u32) S32_MAX) + 1, -1);
	do_ioctl_VIDIOC_CROPCAP(U32_MAX - 1, -1);
	do_ioctl_VIDIOC_CROPCAP(U32_MAX, -1);

}

void test_VIDIOC_CROPCAP_enum_INPUT()
{
	int ret_get, errno_get;
	int ret_set, errno_set;
	int enum_ret;
	__u32 input_index_orig;
	struct v4l2_input input;
	__u32 i;
	int f;

	f = get_video_fd();

	memset(&input_index_orig, 0xff, sizeof(input_index_orig));
	ret_get = ioctl(f, VIDIOC_G_INPUT, &input_index_orig);
	errno_get = errno;

	if (ret_get == 0) {
		CU_ASSERT_EQUAL(ret_get, 0);
		i = 0;
		do {
			memset(&input, 0xff, sizeof(input));
			input.index = i;
			enum_ret = ioctl(f, VIDIOC_ENUMINPUT, &input);

			dprintf
			    ("\t%s:%u: ENUMINPUT: i=%u, enum_ret=%i, errno=%i\n",
			     __FILE__, __LINE__, i, enum_ret, errno);

			if (enum_ret == 0) {
				ret_set =
				    ioctl(f, VIDIOC_S_INPUT, &input.index);
				errno_set = errno;

				dprintf
				    ("\t%s:%u: input.index=0x%X, ret_set=%i, errno_set=%i\n",
				     __FILE__, __LINE__, input.index, ret_set,
				     errno_set);

				CU_ASSERT_EQUAL(ret_set, 0);
				if (ret_set == 0) {
					test_VIDIOC_CROPCAP();
				}

			}
			i++;
		} while (enum_ret == 0 && i != 0);

		/* Setting the original input_id should not fail */
		ret_set = ioctl(f, VIDIOC_S_INPUT, &input_index_orig);
		errno_set = errno;

		CU_ASSERT_EQUAL(ret_set, 0);
	} else {
		CU_ASSERT_EQUAL(ret_get, -1);
		CU_ASSERT_EQUAL(errno_get, EINVAL);
	}
}

void test_VIDIOC_CROPCAP_NULL()
{
	int ret_capture, errno_capture;
	int ret_output, errno_output;
	int ret_overlay, errno_overlay;
	int ret_private, errno_private;
	int ret_private_1, errno_private_1;
	int ret_null, errno_null;
	struct v4l2_cropcap cropcap;

	memset(&cropcap, 0xff, sizeof(cropcap));
	cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	ret_capture = ioctl(get_video_fd(), VIDIOC_CROPCAP, &cropcap);
	errno_capture = errno;

	dprintf("\t%s:%u: VIDIOC_CROPCAP, ret_capture=%i, errno_capture=%i\n",
		__FILE__, __LINE__, ret_capture, errno_capture);

	memset(&cropcap, 0xff, sizeof(cropcap));
	cropcap.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
	ret_output = ioctl(get_video_fd(), VIDIOC_CROPCAP, &cropcap);
	errno_output = errno;

	dprintf("\t%s:%u: VIDIOC_CROPCAP, ret_output=%i, errno_output=%i\n",
		__FILE__, __LINE__, ret_output, errno_output);

	memset(&cropcap, 0xff, sizeof(cropcap));
	cropcap.type = V4L2_BUF_TYPE_VIDEO_OVERLAY;
	ret_overlay = ioctl(get_video_fd(), VIDIOC_CROPCAP, &cropcap);
	errno_overlay = errno;

	dprintf("\t%s:%u: VIDIOC_CROPCAP, ret_overlay=%i, errno_overlay=%i\n",
		__FILE__, __LINE__, ret_overlay, errno_overlay);

	memset(&cropcap, 0xff, sizeof(cropcap));
	cropcap.type = V4L2_BUF_TYPE_PRIVATE;
	ret_private = ioctl(get_video_fd(), VIDIOC_CROPCAP, &cropcap);
	errno_private = errno;

	dprintf("\t%s:%u: VIDIOC_CROPCAP, ret_private=%i, errno_private=%i\n",
		__FILE__, __LINE__, ret_private, errno_private);

	memset(&cropcap, 0xff, sizeof(cropcap));
	cropcap.type = V4L2_BUF_TYPE_PRIVATE + 1;
	ret_private_1 = ioctl(get_video_fd(), VIDIOC_CROPCAP, &cropcap);
	errno_private_1 = errno;

	dprintf
	    ("\t%s:%u: VIDIOC_CROPCAP, ret_private_1=%i, errno_private_1=%i\n",
	     __FILE__, __LINE__, ret_private_1, errno_private_1);

	ret_null = ioctl(get_video_fd(), VIDIOC_CROPCAP, NULL);
	errno_null = errno;

	dprintf("\t%s:%u: VIDIOC_CROPCAP, ret_null=%i, errno_null=%i\n",
		__FILE__, __LINE__, ret_null, errno_null);

	/* Check if at least one type was supported */
	if (ret_capture == 0 || ret_output == 0 || ret_overlay == 0 ||
	    ret_private == 0 || ret_private_1 == 0) {
		/* the parameter shall be validated */
		CU_ASSERT_EQUAL(ret_null, -1);
		CU_ASSERT_EQUAL(errno_null, EFAULT);
	} else {
		/* VIDIOC_CROPCAP is not supported at all, the parameter
		 * shall also not be checked.
		 */
		CU_ASSERT_EQUAL(ret_capture, -1);
		CU_ASSERT_EQUAL(errno_capture, EINVAL);
		CU_ASSERT_EQUAL(ret_output, -1);
		CU_ASSERT_EQUAL(errno_output, EINVAL);
		CU_ASSERT_EQUAL(ret_overlay, -1);
		CU_ASSERT_EQUAL(errno_overlay, EINVAL);
		CU_ASSERT_EQUAL(ret_private, -1);
		CU_ASSERT_EQUAL(errno_private, EINVAL);
		CU_ASSERT_EQUAL(ret_private_1, -1);
		CU_ASSERT_EQUAL(errno_private_1, EINVAL);
		CU_ASSERT_EQUAL(ret_null, -1);
		CU_ASSERT_EQUAL(errno_null, EINVAL);
	}

}
