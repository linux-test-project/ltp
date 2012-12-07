/*
 * v4l-test: Test environment for Video For Linux Two API
 *
 *  3 Apr 2009  0.4  Minor style cleanup
 *  7 Mar 2009  0.3  Test cases added for VIDIOC_S_CROP
 * 13 Feb 2009  0.2  Test cases added for VIDIOC_G_CROP
 *  7 Feb 2009  0.1  First release
 *
 * Written by Márton Németh <nm127@freemail.hu>
 * Released under GPL
 */

#include <sys/ioctl.h>
#include <errno.h>
#include <string.h>

#include <linux/videodev2.h>
#include <linux/errno.h>

#include <CUnit/CUnit.h>

#include "v4l2_test.h"
#include "dev_video.h"
#include "video_limits.h"

#include "test_VIDIOC_CROP.h"

void do_get_crop(enum v4l2_buf_type type)
{
	int ret1, errno1;
	struct v4l2_crop crop;

	memset(&crop, 0xff, sizeof(crop));
	crop.type = type;
	ret1 = ioctl(get_video_fd(), VIDIOC_G_CROP, &crop);
	errno1 = errno;

	dprintf("\tVIDIOC_G_CROP: type=%i, ret1=%i, errno1=%i\n",
		type, ret1, errno1);

	if (ret1 == 0) {
		CU_ASSERT_EQUAL(ret1, 0);

	} else {
		CU_ASSERT_EQUAL(ret1, -1);
		CU_ASSERT_EQUAL(errno1, EINVAL);
	}

}

void test_VIDIOC_G_CROP()
{
	do_get_crop(V4L2_BUF_TYPE_VIDEO_CAPTURE);
	do_get_crop(V4L2_BUF_TYPE_VIDEO_OUTPUT);
	do_get_crop(V4L2_BUF_TYPE_VIDEO_OVERLAY);
}

void do_get_crop_invalid(enum v4l2_buf_type type)
{
	int ret1, errno1;
	struct v4l2_crop crop;

	memset(&crop, 0xff, sizeof(crop));
	crop.type = type;
	ret1 = ioctl(get_video_fd(), VIDIOC_G_CROP, &crop);
	errno1 = errno;

	dprintf("\tVIDIOC_G_CROP: type=%i, ret1=%i, errno1=%i\n",
		type, ret1, errno1);

	CU_ASSERT_EQUAL(ret1, -1);
	CU_ASSERT_EQUAL(errno1, EINVAL);
}

void test_VIDIOC_G_CROP_invalid()
{
	do_get_crop_invalid(0);
	do_get_crop_invalid(V4L2_BUF_TYPE_VBI_CAPTURE);
	do_get_crop_invalid(V4L2_BUF_TYPE_VBI_OUTPUT);
	do_get_crop_invalid(V4L2_BUF_TYPE_SLICED_VBI_CAPTURE);
	do_get_crop_invalid(V4L2_BUF_TYPE_SLICED_VBI_OUTPUT);
	do_get_crop_invalid(V4L2_BUF_TYPE_VIDEO_OUTPUT_OVERLAY);
	do_get_crop_invalid(V4L2_BUF_TYPE_PRIVATE);
	do_get_crop_invalid(S32_MAX);
	do_get_crop_invalid(((__u32) S32_MAX) + 1);
	do_get_crop_invalid(U32_MAX);
}

void test_VIDIOC_G_CROP_NULL()
{
	int ret_get1, errno_get1;
	int ret_get2, errno_get2;
	int ret_get3, errno_get3;
	int ret_null, errno_null;
	struct v4l2_crop crop;

	memset(&crop, 0, sizeof(crop));
	crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	ret_get1 = ioctl(get_video_fd(), VIDIOC_G_CROP, &crop);
	errno_get1 = errno;

	dprintf("\t%s:%u: VIDIOC_G_CROP ret_get1=%i, errno_get1=%i\n",
		__FILE__, __LINE__, ret_get1, errno_get1);

	memset(&crop, 0, sizeof(crop));
	crop.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;

	ret_get2 = ioctl(get_video_fd(), VIDIOC_G_CROP, &crop);
	errno_get2 = errno;

	dprintf("\t%s:%u: VIDIOC_G_CROP ret_get2=%i, errno_get2=%i\n",
		__FILE__, __LINE__, ret_get2, errno_get2);

	memset(&crop, 0, sizeof(crop));
	crop.type = V4L2_BUF_TYPE_VIDEO_OVERLAY;

	ret_get3 = ioctl(get_video_fd(), VIDIOC_G_CROP, &crop);
	errno_get3 = errno;

	dprintf("\t%s:%u: VIDIOC_G_CROP ret_get3=%i, errno_get3=%i\n",
		__FILE__, __LINE__, ret_get3, errno_get3);

	ret_null = ioctl(get_video_fd(), VIDIOC_G_CROP, NULL);
	errno_null = errno;

	dprintf("\t%s:%u: VIDIOC_G_CROP ret_null=%i, errno_null=%i\n",
		__FILE__, __LINE__, ret_null, errno_null);

	if (ret_get1 == 0 || ret_get2 == 0 || ret_get3 == 0) {
		CU_ASSERT_EQUAL(ret_null, -1);
		CU_ASSERT_EQUAL(errno_null, EFAULT);

	} else {
		CU_ASSERT_EQUAL(ret_get1, -1);
		CU_ASSERT_EQUAL(errno_get1, EINVAL);
		CU_ASSERT_EQUAL(ret_get2, -1);
		CU_ASSERT_EQUAL(errno_get2, EINVAL);
		CU_ASSERT_EQUAL(ret_get3, -1);
		CU_ASSERT_EQUAL(errno_get3, EINVAL);
		CU_ASSERT_EQUAL(ret_null, -1);
		CU_ASSERT_EQUAL(errno_null, EINVAL);

	}

}

void do_set_crop(enum v4l2_buf_type type)
{
	int ret_orig, errno_orig;
	int ret_set, errno_set;
	int ret_new, errno_new;
	int ret_cap, errno_cap;
	struct v4l2_crop crop_orig;
	struct v4l2_crop crop;
	struct v4l2_crop crop_new;
	struct v4l2_cropcap cropcap;
	__s32 i;

	memset(&crop_orig, 0, sizeof(crop_orig));
	crop_orig.type = type;
	ret_orig = ioctl(get_video_fd(), VIDIOC_G_CROP, &crop_orig);
	errno_orig = errno;
	dprintf("\t%s:%u: VIDIOC_G_CROP, ret_orig=%i, errno_orig=%i, "
		"crop_orig = { .type=%i, .c={ .left=%i, .top=%i, .width=%i, .height=%i }}\n",
		__FILE__, __LINE__,
		ret_orig, errno_orig,
		crop_orig.type,
		crop_orig.c.left,
		crop_orig.c.top, crop_orig.c.width, crop_orig.c.height);

	memset(&cropcap, 0, sizeof(cropcap));
	cropcap.type = type;
	ret_cap = ioctl(get_video_fd(), VIDIOC_CROPCAP, &cropcap);
	errno_cap = errno;

	dprintf
	    ("\t%s:%u: VIDIOC_CROPCAP, ret_cap=%i, errno_cap=%i, cropcap = { .type = %i, "
	     ".bounds = { .left = %i, .top = %i, .width = %i, .height = %i }, "
	     ".defrect = { .left = %i, .top = %i, .width = %i, .height = %i }, "
	     ".pixelaspect = { .numerator = %u, .denominator = %u } " "}\n",
	     __FILE__, __LINE__, ret_cap, errno_cap, cropcap.type,
	     cropcap.bounds.left, cropcap.bounds.top, cropcap.bounds.width,
	     cropcap.bounds.height, cropcap.defrect.left, cropcap.defrect.top,
	     cropcap.defrect.width, cropcap.defrect.height,
	     cropcap.pixelaspect.numerator, cropcap.pixelaspect.denominator);

	memset(&crop, 0xff, sizeof(crop));
	crop.type = type;
	crop.c = cropcap.bounds;
	ret_set = ioctl(get_video_fd(), VIDIOC_S_CROP, &crop);
	errno_set = errno;
	dprintf("\t%s:%u: VIDIOC_S_CROP, ret_set=%i, errno_set=%i, "
		"crop = { .type=%i, .c={ .left=%i, .top=%i, .width=%i, .height=%i }}\n",
		__FILE__, __LINE__,
		ret_set, errno_set,
		crop.type,
		crop.c.left, crop.c.top, crop.c.width, crop.c.height);

	memset(&crop_new, 0, sizeof(crop_new));
	crop_new.type = type;
	ret_new = ioctl(get_video_fd(), VIDIOC_G_CROP, &crop_new);
	errno_new = errno;
	dprintf("\t%s:%u: VIDIOC_G_CROP, ret_new=%i, errno_new=%i, "
		"crop_new = { .type=%i, .c={ .left=%i, .top=%i, .width=%i, .height=%i }}\n",
		__FILE__, __LINE__,
		ret_new, errno_new,
		crop_new.type,
		crop_new.c.left,
		crop_new.c.top, crop_new.c.width, crop_new.c.height);

	if (ret_cap == 0) {
		CU_ASSERT_EQUAL(ret_cap, 0);
		CU_ASSERT_EQUAL(ret_set, 0);
		CU_ASSERT_EQUAL(ret_new, 0);

		if (ret_cap == 0 && ret_new == 0) {

			/*     |   left                                   x   */
			/* ----+----+-------------------------------------->  */
			/*     |    :                                         */
			/* top +    +------ cropcap.bounds -------+  ^        */
			/*     |    |                             |  |        */
			/*     |    | +------- crop_new --------+ |  |        */
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

			CU_ASSERT(cropcap.bounds.left <= crop_new.c.left);
			CU_ASSERT(cropcap.bounds.top <= crop_new.c.top);

			CU_ASSERT(crop_new.c.left + crop_new.c.width <=
				  cropcap.bounds.left + cropcap.bounds.width);
			CU_ASSERT(crop_new.c.top + crop_new.c.height <=
				  cropcap.bounds.top + cropcap.bounds.height);
		}

	} else {
		CU_ASSERT_EQUAL(ret_cap, -1);
		CU_ASSERT_EQUAL(errno_cap, EINVAL);
		CU_ASSERT_EQUAL(ret_set, -1);
		CU_ASSERT_EQUAL(errno_set, EINVAL);
		CU_ASSERT_EQUAL(ret_new, -1);
		CU_ASSERT_EQUAL(errno_new, EINVAL);

	}

	memset(&crop, 0xff, sizeof(crop));
	crop.type = type;
	crop.c = cropcap.defrect;
	ret_set = ioctl(get_video_fd(), VIDIOC_S_CROP, &crop);
	errno_set = errno;
	dprintf("\t%s:%u: VIDIOC_S_CROP, ret_set=%i, errno_set=%i, "
		"crop = { .type=%i, .c={ .left=%i, .top=%i, .width=%i, .height=%i }}\n",
		__FILE__, __LINE__,
		ret_set, errno_set,
		crop.type,
		crop.c.left, crop.c.top, crop.c.width, crop.c.height);

	memset(&crop_new, 0, sizeof(crop_new));
	crop_new.type = type;
	ret_new = ioctl(get_video_fd(), VIDIOC_G_CROP, &crop_new);
	errno_new = errno;
	dprintf("\t%s:%u: VIDIOC_G_CROP, ret_new=%i, errno_new=%i, "
		"crop_new = { .type=%i, .c={ .left=%i, .top=%i, .width=%i, .height=%i }}\n",
		__FILE__, __LINE__,
		ret_new, errno_new,
		crop_new.type,
		crop_new.c.left,
		crop_new.c.top, crop_new.c.width, crop_new.c.height);

	if (ret_cap == 0) {
		CU_ASSERT_EQUAL(ret_cap, 0);
		CU_ASSERT_EQUAL(ret_set, 0);
		CU_ASSERT_EQUAL(ret_new, 0);

		if (ret_cap == 0 && ret_new == 0) {

			/*     |   left                                   x   */
			/* ----+----+-------------------------------------->  */
			/*     |    :                                         */
			/* top +    +------ cropcap.defrect ------+  ^        */
			/*     |    |                             |  |        */
			/*     |    | +------- crop_new --------+ |  |        */
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

			CU_ASSERT(cropcap.defrect.left <= crop_new.c.left);
			CU_ASSERT(cropcap.defrect.top <= crop_new.c.top);

			CU_ASSERT(crop_new.c.left + crop_new.c.width <=
				  cropcap.defrect.left + cropcap.defrect.width);
			CU_ASSERT(crop_new.c.top + crop_new.c.height <=
				  cropcap.defrect.top + cropcap.defrect.height);
		}

	} else {
		CU_ASSERT_EQUAL(ret_cap, -1);
		CU_ASSERT_EQUAL(errno_cap, EINVAL);
		CU_ASSERT_EQUAL(ret_set, -1);
		CU_ASSERT_EQUAL(errno_set, EINVAL);
		CU_ASSERT_EQUAL(ret_new, -1);
		CU_ASSERT_EQUAL(errno_new, EINVAL);

	}

	/*     |   left                                   x   */
	/* ----+----+-------------------------------------->  */
	/*     |    :                                         */
	/* top +    +-------- crop.c -------------+  ^        */
	/*     |    |                       :     |  |        */
	/*     |    |                       :     |  |        */
	/*     |    |                       :     |  |        */
	/*     |    |                       :<----|  |        */
	/*     |    |                       :     |  | height */
	/*     |    |                       :     |  |        */
	/*     |    |                       :     |  |        */
	/*     |    |                       :     |  |        */
	/*     |    +-----------------------------+  v        */
	/*     |    :                             :           */
	/*     |    <---------- width ------------>           */
	/*     |                                              */
	/*     v y                                            */
	for (i = 0; i < cropcap.bounds.width; i++) {
		memset(&crop, 0xff, sizeof(crop));
		crop.type = type;
		crop.c.left = cropcap.bounds.left;
		crop.c.top = cropcap.bounds.top;
		crop.c.width = cropcap.bounds.width - i;
		crop.c.height = cropcap.bounds.height;
		ret_set = ioctl(get_video_fd(), VIDIOC_S_CROP, &crop);
		errno_set = errno;
		dprintf("\t%s:%u: VIDIOC_S_CROP, ret_set=%i, errno_set=%i, "
			"crop = { .type=%i, .c={ .left=%i, .top=%i, .width=%i, .height=%i }}\n",
			__FILE__, __LINE__,
			ret_set, errno_set,
			crop.type,
			crop.c.left, crop.c.top, crop.c.width, crop.c.height);

		memset(&crop_new, 0, sizeof(crop_new));
		crop_new.type = type;
		ret_new = ioctl(get_video_fd(), VIDIOC_G_CROP, &crop_new);
		errno_new = errno;
		dprintf("\t%s:%u: VIDIOC_G_CROP, ret_new=%i, errno_new=%i, "
			"crop_new = { .type=%i, .c={ .left=%i, .top=%i, .width=%i, .height=%i }}\n",
			__FILE__, __LINE__,
			ret_new, errno_new,
			crop_new.type,
			crop_new.c.left,
			crop_new.c.top, crop_new.c.width, crop_new.c.height);

		if (ret_cap == 0) {
			CU_ASSERT_EQUAL(ret_cap, 0);
			CU_ASSERT_EQUAL(ret_set, 0);
			CU_ASSERT_EQUAL(ret_new, 0);

			if (ret_cap == 0 && ret_new == 0) {

				CU_ASSERT(cropcap.defrect.left <=
					  crop_new.c.left);
				CU_ASSERT(cropcap.defrect.top <=
					  crop_new.c.top);

				CU_ASSERT(crop_new.c.left + crop_new.c.width <=
					  cropcap.defrect.left +
					  cropcap.defrect.width);
				CU_ASSERT(crop_new.c.top + crop_new.c.height <=
					  cropcap.defrect.top +
					  cropcap.defrect.height);
			}

		} else {
			CU_ASSERT_EQUAL(ret_cap, -1);
			CU_ASSERT_EQUAL(errno_cap, EINVAL);
			CU_ASSERT_EQUAL(ret_set, -1);
			CU_ASSERT_EQUAL(errno_set, EINVAL);
			CU_ASSERT_EQUAL(ret_new, -1);
			CU_ASSERT_EQUAL(errno_new, EINVAL);
		}
	}

	/*     |   left                                   x   */
	/* ----+----+-------------------------------------->  */
	/*     |    :                                         */
	/* top +    +---------- crop.c -----------+  ^        */
	/*     |    |                             |  |        */
	/*     |    |                             |  |        */
	/*     |    |                             |  |        */
	/*     |    |                             |  |        */
	/*     |    |.............................|  | height */
	/*     |    |             ^               |  |        */
	/*     |    |             |               |  |        */
	/*     |    |             |               |  |        */
	/*     |    +-----------------------------+  v        */
	/*     |    :                             :           */
	/*     |    <---------- width ------------>           */
	/*     |                                              */
	/*     v y                                            */
	for (i = 0; i < cropcap.bounds.height; i++) {
		memset(&crop, 0xff, sizeof(crop));
		crop.type = type;
		crop.c.left = cropcap.bounds.left;
		crop.c.top = cropcap.bounds.top;
		crop.c.width = cropcap.bounds.width;
		crop.c.height = cropcap.bounds.height - i;
		ret_set = ioctl(get_video_fd(), VIDIOC_S_CROP, &crop);
		errno_set = errno;
		dprintf("\t%s:%u: VIDIOC_S_CROP, ret_set=%i, errno_set=%i, "
			"crop = { .type=%i, .c={ .left=%i, .top=%i, .width=%i, .height=%i }}\n",
			__FILE__, __LINE__,
			ret_set, errno_set,
			crop.type,
			crop.c.left, crop.c.top, crop.c.width, crop.c.height);

		memset(&crop_new, 0, sizeof(crop_new));
		crop_new.type = type;
		ret_new = ioctl(get_video_fd(), VIDIOC_G_CROP, &crop_new);
		errno_new = errno;
		dprintf("\t%s:%u: VIDIOC_G_CROP, ret_new=%i, errno_new=%i, "
			"crop_new = { .type=%i, .c={ .left=%i, .top=%i, .width=%i, .height=%i }}\n",
			__FILE__, __LINE__,
			ret_new, errno_new,
			crop_new.type,
			crop_new.c.left,
			crop_new.c.top, crop_new.c.width, crop_new.c.height);

		if (ret_cap == 0) {
			CU_ASSERT_EQUAL(ret_cap, 0);
			CU_ASSERT_EQUAL(ret_set, 0);
			CU_ASSERT_EQUAL(ret_new, 0);

			if (ret_cap == 0 && ret_new == 0) {

				CU_ASSERT(cropcap.defrect.left <=
					  crop_new.c.left);
				CU_ASSERT(cropcap.defrect.top <=
					  crop_new.c.top);

				CU_ASSERT(crop_new.c.left + crop_new.c.width <=
					  cropcap.defrect.left +
					  cropcap.defrect.width);
				CU_ASSERT(crop_new.c.top + crop_new.c.height <=
					  cropcap.defrect.top +
					  cropcap.defrect.height);
			}

		} else {
			CU_ASSERT_EQUAL(ret_cap, -1);
			CU_ASSERT_EQUAL(errno_cap, EINVAL);
			CU_ASSERT_EQUAL(ret_set, -1);
			CU_ASSERT_EQUAL(errno_set, EINVAL);
			CU_ASSERT_EQUAL(ret_new, -1);
			CU_ASSERT_EQUAL(errno_new, EINVAL);
		}
	}

	/*     |   left                                   x   */
	/* ----+----+-------------------------------------->  */
	/*     |    :                                         */
	/* top +    +---------- crop.c -----------+  ^        */
	/*     |    |    :                        |  |        */
	/*     |    |    :                        |  |        */
	/*     |    |    :                        |  |        */
	/*     |    |--->:                        |  |        */
	/*     |    |    :                        |  | height */
	/*     |    |    :                        |  |        */
	/*     |    |    :                        |  |        */
	/*     |    |    :                        |  |        */
	/*     |    +-----------------------------+  v        */
	/*     |    :                             :           */
	/*     |    <---------- width ------------>           */
	/*     |                                              */
	/*     v y                                            */
	for (i = 0; i < cropcap.bounds.width; i++) {
		memset(&crop, 0xff, sizeof(crop));
		crop.type = type;
		crop.c.left = cropcap.bounds.left + i;
		crop.c.top = cropcap.bounds.top;
		crop.c.width = cropcap.bounds.width - i;
		crop.c.height = cropcap.bounds.height;
		ret_set = ioctl(get_video_fd(), VIDIOC_S_CROP, &crop);
		errno_set = errno;
		dprintf("\t%s:%u: VIDIOC_S_CROP, ret_set=%i, errno_set=%i, "
			"crop = { .type=%i, .c={ .left=%i, .top=%i, .width=%i, .height=%i }}\n",
			__FILE__, __LINE__,
			ret_set, errno_set,
			crop.type,
			crop.c.left, crop.c.top, crop.c.width, crop.c.height);

		memset(&crop_new, 0, sizeof(crop_new));
		crop_new.type = type;
		ret_new = ioctl(get_video_fd(), VIDIOC_G_CROP, &crop_new);
		errno_new = errno;
		dprintf("\t%s:%u: VIDIOC_G_CROP, ret_new=%i, errno_new=%i, "
			"crop_new = { .type=%i, .c={ .left=%i, .top=%i, .width=%i, .height=%i }}\n",
			__FILE__, __LINE__,
			ret_new, errno_new,
			crop_new.type,
			crop_new.c.left,
			crop_new.c.top, crop_new.c.width, crop_new.c.height);

		if (ret_cap == 0) {
			CU_ASSERT_EQUAL(ret_cap, 0);
			CU_ASSERT_EQUAL(ret_set, 0);
			CU_ASSERT_EQUAL(ret_new, 0);

			if (ret_cap == 0 && ret_new == 0) {

				CU_ASSERT(cropcap.defrect.left <=
					  crop_new.c.left);
				CU_ASSERT(cropcap.defrect.top <=
					  crop_new.c.top);

				CU_ASSERT(crop_new.c.left + crop_new.c.width <=
					  cropcap.defrect.left +
					  cropcap.defrect.width);
				CU_ASSERT(crop_new.c.top + crop_new.c.height <=
					  cropcap.defrect.top +
					  cropcap.defrect.height);
			}

		} else {
			CU_ASSERT_EQUAL(ret_cap, -1);
			CU_ASSERT_EQUAL(errno_cap, EINVAL);
			CU_ASSERT_EQUAL(ret_set, -1);
			CU_ASSERT_EQUAL(errno_set, EINVAL);
			CU_ASSERT_EQUAL(ret_new, -1);
			CU_ASSERT_EQUAL(errno_new, EINVAL);
		}
	}

	/*     |   left                                   x   */
	/* ----+----+-------------------------------------->  */
	/*     |    :                                         */
	/* top +    +---------- crop.c -----------+  ^        */
	/*     |    |         |                   |  |        */
	/*     |    |         |                   |  |        */
	/*     |    |         v                   |  |        */
	/*     |    |.............................|  |        */
	/*     |    |                             |  | height */
	/*     |    |                             |  |        */
	/*     |    |                             |  |        */
	/*     |    |                             |  |        */
	/*     |    +-----------------------------+  v        */
	/*     |    :                             :           */
	/*     |    <---------- width ------------>           */
	/*     |                                              */
	/*     v y                                            */
	for (i = 0; i < cropcap.bounds.height; i++) {
		memset(&crop, 0xff, sizeof(crop));
		crop.type = type;
		crop.c.left = cropcap.bounds.left;
		crop.c.top = cropcap.bounds.top + i;
		crop.c.width = cropcap.bounds.width;
		crop.c.height = cropcap.bounds.height - i;
		ret_set = ioctl(get_video_fd(), VIDIOC_S_CROP, &crop);
		errno_set = errno;
		dprintf("\t%s:%u: VIDIOC_S_CROP, ret_set=%i, errno_set=%i, "
			"crop = { .type=%i, .c={ .left=%i, .top=%i, .width=%i, .height=%i }}\n",
			__FILE__, __LINE__,
			ret_set, errno_set,
			crop.type,
			crop.c.left, crop.c.top, crop.c.width, crop.c.height);

		memset(&crop_new, 0, sizeof(crop_new));
		crop_new.type = type;
		ret_new = ioctl(get_video_fd(), VIDIOC_G_CROP, &crop_new);
		errno_new = errno;
		dprintf("\t%s:%u: VIDIOC_G_CROP, ret_new=%i, errno_new=%i, "
			"crop_new = { .type=%i, .c={ .left=%i, .top=%i, .width=%i, .height=%i }}\n",
			__FILE__, __LINE__,
			ret_new, errno_new,
			crop_new.type,
			crop_new.c.left,
			crop_new.c.top, crop_new.c.width, crop_new.c.height);

		if (ret_cap == 0) {
			CU_ASSERT_EQUAL(ret_cap, 0);
			CU_ASSERT_EQUAL(ret_set, 0);
			CU_ASSERT_EQUAL(ret_new, 0);

			if (ret_cap == 0 && ret_new == 0) {

				CU_ASSERT(cropcap.defrect.left <=
					  crop_new.c.left);
				CU_ASSERT(cropcap.defrect.top <=
					  crop_new.c.top);

				CU_ASSERT(crop_new.c.left + crop_new.c.width <=
					  cropcap.defrect.left +
					  cropcap.defrect.width);
				CU_ASSERT(crop_new.c.top + crop_new.c.height <=
					  cropcap.defrect.top +
					  cropcap.defrect.height);
			}

		} else {
			CU_ASSERT_EQUAL(ret_cap, -1);
			CU_ASSERT_EQUAL(errno_cap, EINVAL);
			CU_ASSERT_EQUAL(ret_set, -1);
			CU_ASSERT_EQUAL(errno_set, EINVAL);
			CU_ASSERT_EQUAL(ret_new, -1);
			CU_ASSERT_EQUAL(errno_new, EINVAL);
		}
	}

	if (ret_orig == 0) {
		/* it shall be possible to restore the original settings */
		ret_set = ioctl(get_video_fd(), VIDIOC_S_CROP, &crop_orig);
		errno_set = errno;
		dprintf("\t%s:%u: VIDIOC_S_CROP, ret_set=%i, errno_set=%i\n",
			__FILE__, __LINE__, ret_set, errno_set);
		CU_ASSERT_EQUAL(ret_set, 0);
	}
}

void test_VIDIOC_S_CROP()
{

	do_set_crop(V4L2_BUF_TYPE_VIDEO_CAPTURE);
	do_set_crop(V4L2_BUF_TYPE_VIDEO_OUTPUT);
	do_set_crop(V4L2_BUF_TYPE_VIDEO_OVERLAY);
	do_set_crop(V4L2_BUF_TYPE_PRIVATE);

}

void do_set_crop_invalid(enum v4l2_buf_type type)
{
	int ret_set, errno_set;
	int ret_new, errno_new;
	int ret_cap, errno_cap;
	struct v4l2_crop crop;
	struct v4l2_crop crop_new;
	struct v4l2_cropcap cropcap;

	memset(&cropcap, 0, sizeof(cropcap));
	cropcap.type = type;
	ret_cap = ioctl(get_video_fd(), VIDIOC_CROPCAP, &cropcap);
	errno_cap = errno;

	dprintf
	    ("\t%s:%u: VIDIOC_CROPCAP, ret_cap=%i, errno_cap=%i, cropcap = { .type = %i, "
	     ".bounds = { .left = %i, .top = %i, .width = %i, .height = %i }, "
	     ".defrect = { .left = %i, .top = %i, .width = %i, .height = %i }, "
	     ".pixelaspect = { .numerator = %u, .denominator = %u } " "}\n",
	     __FILE__, __LINE__, ret_cap, errno_cap, cropcap.type,
	     cropcap.bounds.left, cropcap.bounds.top, cropcap.bounds.width,
	     cropcap.bounds.height, cropcap.defrect.left, cropcap.defrect.top,
	     cropcap.defrect.width, cropcap.defrect.height,
	     cropcap.pixelaspect.numerator, cropcap.pixelaspect.denominator);

	memset(&crop, 0xff, sizeof(crop));
	crop.type = type;
	crop.c = cropcap.bounds;
	ret_set = ioctl(get_video_fd(), VIDIOC_S_CROP, &crop);
	errno_set = errno;
	dprintf("\t%s:%u: VIDIOC_S_CROP, ret_set=%i, errno_set=%i, "
		"crop = { .type=%i, .c={ .left=%i, .top=%i, .width=%i, .height=%i }}\n",
		__FILE__, __LINE__,
		ret_set, errno_set,
		crop.type,
		crop.c.left, crop.c.top, crop.c.width, crop.c.height);

	memset(&crop_new, 0, sizeof(crop_new));
	crop_new.type = type;
	ret_new = ioctl(get_video_fd(), VIDIOC_G_CROP, &crop_new);
	errno_new = errno;
	dprintf("\t%s:%u: VIDIOC_G_CROP, ret_new=%i, errno_new=%i, "
		"crop_new = { .type=%i, .c={ .left=%i, .top=%i, .width=%i, .height=%i }}\n",
		__FILE__, __LINE__,
		ret_new, errno_new,
		crop_new.type,
		crop_new.c.left,
		crop_new.c.top, crop_new.c.width, crop_new.c.height);

	CU_ASSERT_EQUAL(ret_cap, -1);
	CU_ASSERT_EQUAL(errno_cap, EINVAL);
	CU_ASSERT_EQUAL(ret_set, -1);
	CU_ASSERT_EQUAL(errno_set, EINVAL);
	CU_ASSERT_EQUAL(ret_new, -1);
	CU_ASSERT_EQUAL(errno_new, EINVAL);

}

void test_VIDIOC_S_CROP_invalid()
{
	do_set_crop_invalid(0);
	do_set_crop_invalid(V4L2_BUF_TYPE_VBI_CAPTURE);
	do_set_crop_invalid(V4L2_BUF_TYPE_VBI_OUTPUT);
	do_set_crop_invalid(V4L2_BUF_TYPE_SLICED_VBI_CAPTURE);
	do_set_crop_invalid(V4L2_BUF_TYPE_SLICED_VBI_OUTPUT);
	do_set_crop_invalid(V4L2_BUF_TYPE_VIDEO_OUTPUT_OVERLAY);
	do_set_crop_invalid(V4L2_BUF_TYPE_PRIVATE);
	do_set_crop_invalid(S32_MAX);
	do_set_crop_invalid(((__u32) S32_MAX) + 1);
	do_set_crop_invalid(U32_MAX);
}

void do_set_crop_null(enum v4l2_buf_type type)
{
	int ret_orig, errno_orig;
	int ret_set, errno_set;
	int ret_cap, errno_cap;
	int ret_null, errno_null;
	struct v4l2_crop crop;
	struct v4l2_crop crop_orig;
	struct v4l2_cropcap cropcap;

	memset(&crop_orig, 0, sizeof(crop_orig));
	crop_orig.type = type;
	ret_orig = ioctl(get_video_fd(), VIDIOC_G_CROP, &crop_orig);
	errno_orig = errno;
	dprintf("\t%s:%u: VIDIOC_G_CROP, ret_orig=%i, errno_orig=%i, "
		"crop_orig = { .type=%i, .c={ .left=%i, .top=%i, .width=%i, .height=%i }}\n",
		__FILE__, __LINE__,
		ret_orig, errno_orig,
		crop_orig.type,
		crop_orig.c.left,
		crop_orig.c.top, crop_orig.c.width, crop_orig.c.height);

	memset(&cropcap, 0, sizeof(cropcap));
	cropcap.type = type;
	ret_cap = ioctl(get_video_fd(), VIDIOC_CROPCAP, &cropcap);
	errno_cap = errno;

	dprintf
	    ("\t%s:%u: VIDIOC_CROPCAP, ret_cap=%i, errno_cap=%i, cropcap = { .type = %i, "
	     ".bounds = { .left = %i, .top = %i, .width = %i, .height = %i }, "
	     ".defrect = { .left = %i, .top = %i, .width = %i, .height = %i }, "
	     ".pixelaspect = { .numerator = %u, .denominator = %u } " "}\n",
	     __FILE__, __LINE__, ret_cap, errno_cap, cropcap.type,
	     cropcap.bounds.left, cropcap.bounds.top, cropcap.bounds.width,
	     cropcap.bounds.height, cropcap.defrect.left, cropcap.defrect.top,
	     cropcap.defrect.width, cropcap.defrect.height,
	     cropcap.pixelaspect.numerator, cropcap.pixelaspect.denominator);

	memset(&crop, 0, sizeof(crop));
	crop.type = type;
	crop.c = cropcap.bounds;
	ret_set = ioctl(get_video_fd(), VIDIOC_S_CROP, &crop);
	errno_set = errno;
	dprintf("\t%s:%u: VIDIOC_S_CROP, ret_set=%i, errno_set=%i\n",
		__FILE__, __LINE__, ret_set, errno_set);

	ret_null = ioctl(get_video_fd(), VIDIOC_S_CROP, NULL);
	errno_null = errno;
	dprintf("\t%s:%u: VIDIOC_S_CROP, ret_null=%i, errno_null=%i\n",
		__FILE__, __LINE__, ret_null, errno_null);

	if (ret_set == 0) {
		CU_ASSERT_EQUAL(ret_set, 0);
		CU_ASSERT_EQUAL(ret_null, -1);
		CU_ASSERT_EQUAL(errno_null, EFAULT);

	} else {
		CU_ASSERT_EQUAL(ret_set, -1);
		CU_ASSERT_EQUAL(errno_set, EINVAL);
		CU_ASSERT_EQUAL(ret_null, -1);
		CU_ASSERT_EQUAL(errno_null, EINVAL);

	}

	if (ret_orig == 0) {
		/* it shall be possible to restore the original settings */
		ret_set = ioctl(get_video_fd(), VIDIOC_S_CROP, &crop_orig);
		errno_set = errno;
		dprintf("\t%s:%u: VIDIOC_S_CROP, ret_set=%i, errno_set=%i\n",
			__FILE__, __LINE__, ret_set, errno_set);
		CU_ASSERT_EQUAL(ret_set, 0);
	}

}

void test_VIDIOC_S_CROP_NULL()
{

	do_set_crop_null(V4L2_BUF_TYPE_VIDEO_CAPTURE);
	do_set_crop_null(V4L2_BUF_TYPE_VIDEO_OUTPUT);
	do_set_crop_null(V4L2_BUF_TYPE_VIDEO_OVERLAY);
	do_set_crop_null(V4L2_BUF_TYPE_PRIVATE);

}
