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
#include "v4l2_show.h"

#include "test_VIDIOC_ENUM_FRAMESIZES.h"

static int valid_framesize_type(__u32 type)
{
	int valid = 0;

	if ((type == V4L2_FRMSIZE_TYPE_DISCRETE) ||
	    (type == V4L2_FRMSIZE_TYPE_CONTINUOUS) ||
	    (type == V4L2_FRMSIZE_TYPE_STEPWISE)) {
		valid = 1;
	} else {
		valid = 0;
	}
	return valid;
}

static void do_test_VIDIOC_ENUM_FRAMESIZES(__u32 fmt)
{
	struct v4l2_frmsizeenum framesize;
	int ret_frame, errno_frame;
	__u32 i;
	__u32 first_type;

	i = 0;
	first_type = 0;
	do {
		memset(&framesize, 0xff, sizeof(framesize));
		framesize.index = i;
		framesize.pixel_format = fmt;
		ret_frame =
		    ioctl(get_video_fd(), VIDIOC_ENUM_FRAMESIZES, &framesize);
		errno_frame = errno;

		dprintf
		    ("\t%s:%u: VIDIOC_ENUM_FRAMESIZES, index=%u, pixel_format=0x%x, ret_frame=%i, errno_frame=%i\n",
		     __FILE__, __LINE__, i, fmt, ret_frame, errno_frame);

		if (i != 0 && first_type != V4L2_FRMSIZE_TYPE_DISCRETE) {
			CU_ASSERT_EQUAL(ret_frame, -1);
			CU_ASSERT_EQUAL(errno_frame, EINVAL);
		}

		if (ret_frame == 0) {
			CU_ASSERT_EQUAL(ret_frame, 0);
			CU_ASSERT_EQUAL(framesize.index, i);
			CU_ASSERT_EQUAL(framesize.pixel_format, fmt);
			CU_ASSERT(valid_framesize_type(framesize.type));

			if (i == 0) {
				first_type = framesize.type;
			} else {
				CU_ASSERT_EQUAL(framesize.type, first_type);
			}

			switch (framesize.type) {
			case V4L2_FRMSIZE_TYPE_DISCRETE:
				CU_ASSERT(0 < framesize.discrete.width);
				CU_ASSERT(0 < framesize.discrete.height);
				break;

			case V4L2_FRMSIZE_TYPE_CONTINUOUS:
				CU_ASSERT(0 < framesize.stepwise.min_width);
				CU_ASSERT(0 < framesize.stepwise.max_width);
				CU_ASSERT_EQUAL(framesize.stepwise.step_width,
						1);

				CU_ASSERT(framesize.stepwise.min_width <
					  framesize.stepwise.max_width);

				CU_ASSERT(0 < framesize.stepwise.min_height);
				CU_ASSERT(0 < framesize.stepwise.max_height);
				CU_ASSERT_EQUAL(framesize.stepwise.step_height,
						1);

				CU_ASSERT(framesize.stepwise.min_height <
					  framesize.stepwise.max_height);
				break;

			case V4L2_FRMSIZE_TYPE_STEPWISE:
				CU_ASSERT(0 < framesize.stepwise.min_width);
				CU_ASSERT(0 < framesize.stepwise.max_width);
				CU_ASSERT(0 < framesize.stepwise.step_width);

				CU_ASSERT(framesize.stepwise.min_width <
					  framesize.stepwise.max_width);

				/* check if the given step is unambigous: min + n * step = max */
				if (framesize.stepwise.step_width != 0) {
					CU_ASSERT_EQUAL((framesize.stepwise.
							 max_width -
							 framesize.stepwise.
							 min_width) %
							framesize.stepwise.
							step_width, 0);
				}

				CU_ASSERT(0 < framesize.stepwise.min_height);
				CU_ASSERT(0 < framesize.stepwise.max_height);
				CU_ASSERT(0 < framesize.stepwise.step_height);

				CU_ASSERT(framesize.stepwise.min_height <
					  framesize.stepwise.max_height);

				/* check if the given step is unambigous: min + n * step = max */
				if (framesize.stepwise.step_height != 0) {
					CU_ASSERT_EQUAL((framesize.stepwise.
							 max_height -
							 framesize.stepwise.
							 min_height) %
							framesize.stepwise.
							step_height, 0);
				}

				break;
			}

			CU_ASSERT_EQUAL(framesize.reserved[0], 0);
			CU_ASSERT_EQUAL(framesize.reserved[1], 0);

			show_v4l2_frmsizeenum(&framesize);

		} else {
			CU_ASSERT_EQUAL(ret_frame, -1);
			CU_ASSERT_EQUAL(errno_frame, EINVAL);
		}
		i++;
	} while (ret_frame == 0 && i != 0);

}

static void do_test_VIDIOC_ENUM_FRAMESIZES_type(enum v4l2_buf_type type)
{
	int ret_enum, errno_enum;
	struct v4l2_fmtdesc format;
	__u32 i;

	i = 0;
	do {
		memset(&format, 0xff, sizeof(format));
		format.index = i;
		format.type = type;

		ret_enum = ioctl(get_video_fd(), VIDIOC_ENUM_FMT, &format);
		errno_enum = errno;

		dprintf
		    ("\t%s:%u: VIDIOC_ENUM_FMT, index=%u, type=%i, ret_enum=%i, errno_enum=%i\n",
		     __FILE__, __LINE__, i, format.type, ret_enum, errno_enum);

		/* Ensure that VIDIOC_ENUM_FRAMESIZES is called at least once
		 * even if VIDIOC_ENUM_FMT returns error
		 */
		do_test_VIDIOC_ENUM_FRAMESIZES(format.pixelformat);

		i++;
	} while (ret_enum == 0 && i != 0);

}

void test_VIDIOC_ENUM_FRAMESIZES()
{
	do_test_VIDIOC_ENUM_FRAMESIZES_type(V4L2_BUF_TYPE_VIDEO_CAPTURE);
	do_test_VIDIOC_ENUM_FRAMESIZES_type(V4L2_BUF_TYPE_VIDEO_OUTPUT);
	do_test_VIDIOC_ENUM_FRAMESIZES_type(V4L2_BUF_TYPE_VIDEO_OVERLAY);
	do_test_VIDIOC_ENUM_FRAMESIZES_type(V4L2_BUF_TYPE_PRIVATE);
}

static void do_test_VIDIOC_ENUM_FRAMESIZES_invalid_index(__u32 fmt)
{
	struct v4l2_frmsizeenum framesize;
	int ret_frame, errno_frame;
	__u32 i;
	__u32 max_index;

	i = 0;
	do {
		memset(&framesize, 0xff, sizeof(framesize));
		framesize.index = i;
		framesize.pixel_format = fmt;
		ret_frame =
		    ioctl(get_video_fd(), VIDIOC_ENUM_FRAMESIZES, &framesize);
		errno_frame = errno;

		dprintf
		    ("\t%s:%u: VIDIOC_ENUM_FRAMESIZES, index=%u, pixel_format=0x%x, ret_frame=%i, errno_frame=%i\n",
		     __FILE__, __LINE__, i, fmt, ret_frame, errno_frame);

		i++;
	} while (ret_frame == 0 && i != 0);

	max_index = i - 1;

	i = max_index + 1;
	memset(&framesize, 0xff, sizeof(framesize));
	framesize.index = i;
	framesize.pixel_format = fmt;
	ret_frame = ioctl(get_video_fd(), VIDIOC_ENUM_FRAMESIZES, &framesize);
	errno_frame = errno;

	dprintf
	    ("\t%s:%u: VIDIOC_ENUM_FRAMESIZES, index=%u, pixel_format=0x%x, ret_frame=%i, errno_frame=%i\n",
	     __FILE__, __LINE__, i, fmt, ret_frame, errno_frame);

	CU_ASSERT_EQUAL(ret_frame, -1);
	CU_ASSERT_EQUAL(errno_frame, EINVAL);

	i = (__u32) S32_MIN;
	if (max_index < i) {
		memset(&framesize, 0xff, sizeof(framesize));
		framesize.index = i;
		framesize.pixel_format = fmt;
		ret_frame =
		    ioctl(get_video_fd(), VIDIOC_ENUM_FRAMESIZES, &framesize);
		errno_frame = errno;

		dprintf
		    ("\t%s:%u: VIDIOC_ENUM_FRAMESIZES, index=0x%x, pixel_format=0x%x, ret_frame=%i, errno_frame=%i\n",
		     __FILE__, __LINE__, i, fmt, ret_frame, errno_frame);

		CU_ASSERT_EQUAL(ret_frame, -1);
		CU_ASSERT_EQUAL(errno_frame, EINVAL);
	}

	i = (__u32) S32_MAX;
	if (max_index < i) {
		memset(&framesize, 0xff, sizeof(framesize));
		framesize.index = i;
		framesize.pixel_format = fmt;
		ret_frame =
		    ioctl(get_video_fd(), VIDIOC_ENUM_FRAMESIZES, &framesize);
		errno_frame = errno;

		dprintf
		    ("\t%s:%u: VIDIOC_ENUM_FRAMESIZES, index=0x%x, pixel_format=0x%x, ret_frame=%i, errno_frame=%i\n",
		     __FILE__, __LINE__, i, fmt, ret_frame, errno_frame);

		CU_ASSERT_EQUAL(ret_frame, -1);
		CU_ASSERT_EQUAL(errno_frame, EINVAL);
	}

	i = (__u32) U32_MAX;
	if (max_index < i) {
		memset(&framesize, 0xff, sizeof(framesize));
		framesize.index = i;
		framesize.pixel_format = fmt;
		ret_frame =
		    ioctl(get_video_fd(), VIDIOC_ENUM_FRAMESIZES, &framesize);
		errno_frame = errno;

		dprintf
		    ("\t%s:%u: VIDIOC_ENUM_FRAMESIZES, index=0x%x, pixel_format=0x%x, ret_frame=%i, errno_frame=%i\n",
		     __FILE__, __LINE__, i, fmt, ret_frame, errno_frame);

		CU_ASSERT_EQUAL(ret_frame, -1);
		CU_ASSERT_EQUAL(errno_frame, EINVAL);
	}

}

static void do_test_VIDIOC_ENUM_FRAMESIZES_type_invalid_index(enum v4l2_buf_type
							      type)
{
	int ret_enum, errno_enum;
	struct v4l2_fmtdesc format;
	__u32 i;

	i = 0;
	do {
		memset(&format, 0xff, sizeof(format));
		format.index = i;
		format.type = type;

		ret_enum = ioctl(get_video_fd(), VIDIOC_ENUM_FMT, &format);
		errno_enum = errno;

		dprintf
		    ("\t%s:%u: VIDIOC_ENUM_FMT, index=%u, type=%i, ret_enum=%i, errno_enum=%i\n",
		     __FILE__, __LINE__, i, format.type, ret_enum, errno_enum);

		/* Ensure that VIDIOC_ENUM_FRAMESIZES is called at least once
		 * even if VIDIOC_ENUM_FMT returns error
		 */
		do_test_VIDIOC_ENUM_FRAMESIZES_invalid_index(format.
							     pixelformat);

		i++;
	} while (ret_enum == 0 && i != 0);

}

void test_VIDIOC_ENUM_FRAMESIZES_invalid_index()
{
	do_test_VIDIOC_ENUM_FRAMESIZES_type_invalid_index
	    (V4L2_BUF_TYPE_VIDEO_CAPTURE);
	do_test_VIDIOC_ENUM_FRAMESIZES_type_invalid_index
	    (V4L2_BUF_TYPE_VIDEO_OUTPUT);
	do_test_VIDIOC_ENUM_FRAMESIZES_type_invalid_index
	    (V4L2_BUF_TYPE_VIDEO_OVERLAY);
	do_test_VIDIOC_ENUM_FRAMESIZES_type_invalid_index
	    (V4L2_BUF_TYPE_PRIVATE);
}

static int supported_pixel_format_type(enum v4l2_buf_type type,
				       __u32 pixel_format)
{
	int ret_enum, errno_enum;
	struct v4l2_fmtdesc format;
	__u32 i;
	int supported = 0;

	i = 0;
	do {
		memset(&format, 0xff, sizeof(format));
		format.index = i;
		format.type = type;

		ret_enum = ioctl(get_video_fd(), VIDIOC_ENUM_FMT, &format);
		errno_enum = errno;

		dprintf
		    ("\t%s:%u: VIDIOC_ENUM_FMT, index=%u, type=%i, ret_enum=%i, errno_enum=%i\n",
		     __FILE__, __LINE__, i, format.type, ret_enum, errno_enum);

		i++;
	} while (ret_enum == 0 && i != 0 && format.pixelformat != pixel_format);

	if (ret_enum == 0 && i != 0 && format.pixelformat == pixel_format) {
		supported = 1;
	}

	return supported;
}

static int supported_pixel_format(__u32 pixel_format)
{
	int supported = 0;

	supported =
	    supported_pixel_format_type(V4L2_BUF_TYPE_VIDEO_CAPTURE,
					pixel_format);
	if (!supported) {
		supported =
		    supported_pixel_format_type(V4L2_BUF_TYPE_VIDEO_OUTPUT,
						pixel_format);
		if (!supported) {
			supported =
			    supported_pixel_format_type
			    (V4L2_BUF_TYPE_VIDEO_OVERLAY, pixel_format);
			if (!supported) {
				supported =
				    supported_pixel_format_type
				    (V4L2_BUF_TYPE_PRIVATE, pixel_format);
			}
		}
	}

	return supported;
}

static void do_test_VIDIOC_ENUM_FRAMESIZES_invalid_pixel_format(__u32 fmt)
{
	struct v4l2_frmsizeenum framesize;
	int ret_frame, errno_frame;
	__u32 i;

	if (!supported_pixel_format(fmt)) {
		i = 0;
		memset(&framesize, 0xff, sizeof(framesize));
		framesize.index = i;
		framesize.pixel_format = fmt;
		ret_frame =
		    ioctl(get_video_fd(), VIDIOC_ENUM_FRAMESIZES, &framesize);
		errno_frame = errno;

		dprintf
		    ("\t%s:%u: VIDIOC_ENUM_FRAMESIZES, index=%u, pixel_format=0x%x, ret_frame=%i, errno_frame=%i\n",
		     __FILE__, __LINE__, i, fmt, ret_frame, errno_frame);

		CU_ASSERT_EQUAL(ret_frame, -1);
		CU_ASSERT_EQUAL(errno_frame, EINVAL);
	}

}

void test_VIDIOC_ENUM_FRAMESIZES_invalid_pixel_format()
{

	do_test_VIDIOC_ENUM_FRAMESIZES_invalid_pixel_format(U32_MIN);
	do_test_VIDIOC_ENUM_FRAMESIZES_invalid_pixel_format((__u32) S32_MIN);
	do_test_VIDIOC_ENUM_FRAMESIZES_invalid_pixel_format((__u32) S32_MAX);
	do_test_VIDIOC_ENUM_FRAMESIZES_invalid_pixel_format(U32_MAX);

	do_test_VIDIOC_ENUM_FRAMESIZES_invalid_pixel_format
	    (V4L2_PIX_FMT_RGB332);
	do_test_VIDIOC_ENUM_FRAMESIZES_invalid_pixel_format
	    (V4L2_PIX_FMT_RGB444);
	do_test_VIDIOC_ENUM_FRAMESIZES_invalid_pixel_format
	    (V4L2_PIX_FMT_RGB555);
	do_test_VIDIOC_ENUM_FRAMESIZES_invalid_pixel_format
	    (V4L2_PIX_FMT_RGB565);
	do_test_VIDIOC_ENUM_FRAMESIZES_invalid_pixel_format
	    (V4L2_PIX_FMT_RGB555X);
	do_test_VIDIOC_ENUM_FRAMESIZES_invalid_pixel_format
	    (V4L2_PIX_FMT_RGB565X);
	do_test_VIDIOC_ENUM_FRAMESIZES_invalid_pixel_format(V4L2_PIX_FMT_BGR24);
	do_test_VIDIOC_ENUM_FRAMESIZES_invalid_pixel_format(V4L2_PIX_FMT_RGB24);
	do_test_VIDIOC_ENUM_FRAMESIZES_invalid_pixel_format(V4L2_PIX_FMT_BGR32);
	do_test_VIDIOC_ENUM_FRAMESIZES_invalid_pixel_format(V4L2_PIX_FMT_RGB32);
	do_test_VIDIOC_ENUM_FRAMESIZES_invalid_pixel_format(V4L2_PIX_FMT_GREY);
	do_test_VIDIOC_ENUM_FRAMESIZES_invalid_pixel_format(V4L2_PIX_FMT_Y16);
	do_test_VIDIOC_ENUM_FRAMESIZES_invalid_pixel_format(V4L2_PIX_FMT_PAL8);
	do_test_VIDIOC_ENUM_FRAMESIZES_invalid_pixel_format
	    (V4L2_PIX_FMT_YVU410);
	do_test_VIDIOC_ENUM_FRAMESIZES_invalid_pixel_format
	    (V4L2_PIX_FMT_YVU420);
	do_test_VIDIOC_ENUM_FRAMESIZES_invalid_pixel_format(V4L2_PIX_FMT_YUYV);
	do_test_VIDIOC_ENUM_FRAMESIZES_invalid_pixel_format(V4L2_PIX_FMT_UYVY);
	do_test_VIDIOC_ENUM_FRAMESIZES_invalid_pixel_format
	    (V4L2_PIX_FMT_YUV422P);
	do_test_VIDIOC_ENUM_FRAMESIZES_invalid_pixel_format
	    (V4L2_PIX_FMT_YUV411P);
	do_test_VIDIOC_ENUM_FRAMESIZES_invalid_pixel_format(V4L2_PIX_FMT_Y41P);
	do_test_VIDIOC_ENUM_FRAMESIZES_invalid_pixel_format
	    (V4L2_PIX_FMT_YUV444);
	do_test_VIDIOC_ENUM_FRAMESIZES_invalid_pixel_format
	    (V4L2_PIX_FMT_YUV555);
	do_test_VIDIOC_ENUM_FRAMESIZES_invalid_pixel_format
	    (V4L2_PIX_FMT_YUV565);
	do_test_VIDIOC_ENUM_FRAMESIZES_invalid_pixel_format(V4L2_PIX_FMT_YUV32);
	do_test_VIDIOC_ENUM_FRAMESIZES_invalid_pixel_format(V4L2_PIX_FMT_NV12);
	do_test_VIDIOC_ENUM_FRAMESIZES_invalid_pixel_format(V4L2_PIX_FMT_NV21);
	do_test_VIDIOC_ENUM_FRAMESIZES_invalid_pixel_format
	    (V4L2_PIX_FMT_YUV410);
	do_test_VIDIOC_ENUM_FRAMESIZES_invalid_pixel_format
	    (V4L2_PIX_FMT_YUV420);
	do_test_VIDIOC_ENUM_FRAMESIZES_invalid_pixel_format(V4L2_PIX_FMT_YYUV);
	do_test_VIDIOC_ENUM_FRAMESIZES_invalid_pixel_format(V4L2_PIX_FMT_HI240);
	do_test_VIDIOC_ENUM_FRAMESIZES_invalid_pixel_format(V4L2_PIX_FMT_HM12);
	do_test_VIDIOC_ENUM_FRAMESIZES_invalid_pixel_format
	    (V4L2_PIX_FMT_SBGGR8);
	do_test_VIDIOC_ENUM_FRAMESIZES_invalid_pixel_format
	    (V4L2_PIX_FMT_SBGGR16);
	do_test_VIDIOC_ENUM_FRAMESIZES_invalid_pixel_format(V4L2_PIX_FMT_MJPEG);
	do_test_VIDIOC_ENUM_FRAMESIZES_invalid_pixel_format(V4L2_PIX_FMT_JPEG);
	do_test_VIDIOC_ENUM_FRAMESIZES_invalid_pixel_format(V4L2_PIX_FMT_DV);
	do_test_VIDIOC_ENUM_FRAMESIZES_invalid_pixel_format(V4L2_PIX_FMT_MPEG);
	do_test_VIDIOC_ENUM_FRAMESIZES_invalid_pixel_format(V4L2_PIX_FMT_WNVA);
	do_test_VIDIOC_ENUM_FRAMESIZES_invalid_pixel_format
	    (V4L2_PIX_FMT_SN9C10X);
	do_test_VIDIOC_ENUM_FRAMESIZES_invalid_pixel_format(V4L2_PIX_FMT_PWC1);
	do_test_VIDIOC_ENUM_FRAMESIZES_invalid_pixel_format(V4L2_PIX_FMT_PWC2);
	do_test_VIDIOC_ENUM_FRAMESIZES_invalid_pixel_format
	    (V4L2_PIX_FMT_ET61X251);

#ifdef V4L2_PIX_FMT_VYUY
	do_test_VIDIOC_ENUM_FRAMESIZES_invalid_pixel_format(V4L2_PIX_FMT_VYUY);
#endif

#ifdef V4L2_PIX_FMT_NV16
	do_test_VIDIOC_ENUM_FRAMESIZES_invalid_pixel_format(V4L2_PIX_FMT_NV16);
#endif

#ifdef V4L2_PIX_FMT_NV61
	do_test_VIDIOC_ENUM_FRAMESIZES_invalid_pixel_format(V4L2_PIX_FMT_NV61);
#endif

#ifdef V4L2_PIX_FMT_SGBRG8
	do_test_VIDIOC_ENUM_FRAMESIZES_invalid_pixel_format
	    (V4L2_PIX_FMT_SGBRG8);
#endif

#ifdef V4L2_PIX_FMT_SGRBG8
	do_test_VIDIOC_ENUM_FRAMESIZES_invalid_pixel_format
	    (V4L2_PIX_FMT_SGRBG8);
#endif

#ifdef V4L2_PIX_FMT_SGRBG10
	do_test_VIDIOC_ENUM_FRAMESIZES_invalid_pixel_format
	    (V4L2_PIX_FMT_SGRBG10);
#endif

#ifdef V4L2_PIX_FMT_SGRBG10DPCM8
	do_test_VIDIOC_ENUM_FRAMESIZES_invalid_pixel_format
	    (V4L2_PIX_FMT_SGRBG10DPCM8);
#endif

#ifdef V4L2_PIX_FMT_SPCA501
	do_test_VIDIOC_ENUM_FRAMESIZES_invalid_pixel_format
	    (V4L2_PIX_FMT_SPCA501);
#endif

#ifdef V4L2_PIX_FMT_SPCA505
	do_test_VIDIOC_ENUM_FRAMESIZES_invalid_pixel_format
	    (V4L2_PIX_FMT_SPCA505);
#endif

#ifdef V4L2_PIX_FMT_SPCA508
	do_test_VIDIOC_ENUM_FRAMESIZES_invalid_pixel_format
	    (V4L2_PIX_FMT_SPCA508);
#endif

#ifdef V4L2_PIX_FMT_SPCA561
	do_test_VIDIOC_ENUM_FRAMESIZES_invalid_pixel_format
	    (V4L2_PIX_FMT_SPCA561);
#endif

#ifdef V4L2_PIX_FMT_PAC207
	do_test_VIDIOC_ENUM_FRAMESIZES_invalid_pixel_format
	    (V4L2_PIX_FMT_PAC207);
#endif

#ifdef V4L2_PIX_FMT_MR97310A
	do_test_VIDIOC_ENUM_FRAMESIZES_invalid_pixel_format
	    (V4L2_PIX_FMT_MR97310A);
#endif

#ifdef V4L2_PIX_FMT_SQ905C
	do_test_VIDIOC_ENUM_FRAMESIZES_invalid_pixel_format
	    (V4L2_PIX_FMT_SQ905C);
#endif

#ifdef V4L2_PIX_FMT_PJPG
	do_test_VIDIOC_ENUM_FRAMESIZES_invalid_pixel_format(V4L2_PIX_FMT_PJPG);
#endif

#ifdef V4L2_PIX_FMT_YVYU
	do_test_VIDIOC_ENUM_FRAMESIZES_invalid_pixel_format(V4L2_PIX_FMT_YVYU);
#endif

#ifdef V4L2_PIX_FMT_OV511
	do_test_VIDIOC_ENUM_FRAMESIZES_invalid_pixel_format(V4L2_PIX_FMT_OV511);
#endif

#ifdef V4L2_PIX_FMT_OV518
	do_test_VIDIOC_ENUM_FRAMESIZES_invalid_pixel_format(V4L2_PIX_FMT_OV518);
#endif

}

void test_VIDIOC_ENUM_FRAMESIZES_NULL()
{
	struct v4l2_fmtdesc format_capture;
	struct v4l2_fmtdesc format_output;
	struct v4l2_fmtdesc format_overlay;
	struct v4l2_fmtdesc format_private;
	struct v4l2_frmsizeenum framesize;
	int ret_fmt_capture, errno_fmt_capture;
	int ret_fmt_output, errno_fmt_output;
	int ret_fmt_overlay, errno_fmt_overlay;
	int ret_fmt_private, errno_fmt_private;
	int ret_size, errno_size;
	int ret_null, errno_null;
	__u32 fmt;

	memset(&format_capture, 0xff, sizeof(format_capture));
	format_capture.index = 0;
	format_capture.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	ret_fmt_capture =
	    ioctl(get_video_fd(), VIDIOC_ENUM_FMT, &format_capture);
	errno_fmt_capture = errno;

	dprintf
	    ("\t%s:%u: VIDIOC_ENUM_FMT, index=%u, type=%i, ret_fmt_capture=%i, errno_fmt_capture=%i\n",
	     __FILE__, __LINE__, format_capture.index, format_capture.type,
	     ret_fmt_capture, errno_fmt_capture);

	memset(&format_output, 0xff, sizeof(format_output));
	format_output.index = 0;
	format_output.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;

	ret_fmt_output = ioctl(get_video_fd(), VIDIOC_ENUM_FMT, &format_output);
	errno_fmt_output = errno;

	dprintf
	    ("\t%s:%u: VIDIOC_ENUM_FMT, index=%u, type=%i, ret_fmt_output=%i, errno_fmt_output=%i\n",
	     __FILE__, __LINE__, format_output.index, format_output.type,
	     ret_fmt_output, errno_fmt_output);

	memset(&format_overlay, 0xff, sizeof(format_overlay));
	format_overlay.index = 0;
	format_overlay.type = V4L2_BUF_TYPE_VIDEO_OVERLAY;

	ret_fmt_overlay =
	    ioctl(get_video_fd(), VIDIOC_ENUM_FMT, &format_overlay);
	errno_fmt_overlay = errno;

	dprintf
	    ("\t%s:%u: VIDIOC_ENUM_FMT, index=%u, type=%i, ret_fmt_overlay=%i, errno_fmt_overlay=%i\n",
	     __FILE__, __LINE__, format_overlay.index, format_overlay.type,
	     ret_fmt_overlay, errno_fmt_overlay);

	memset(&format_private, 0xff, sizeof(format_private));
	format_private.index = 0;
	format_private.type = V4L2_BUF_TYPE_PRIVATE;

	ret_fmt_private =
	    ioctl(get_video_fd(), VIDIOC_ENUM_FMT, &format_private);
	errno_fmt_private = errno;

	dprintf
	    ("\t%s:%u: VIDIOC_ENUM_FMT, index=%u, type=%i, ret_fmt_private=%i, errno_fmt_private=%i\n",
	     __FILE__, __LINE__, format_private.index, format_private.type,
	     ret_fmt_private, errno_fmt_private);

	if (ret_fmt_capture == 0) {
		fmt = format_capture.pixelformat;
	} else if (ret_fmt_output == 0) {
		fmt = format_output.pixelformat;
	} else if (ret_fmt_overlay == 0) {
		fmt = format_overlay.pixelformat;
	} else if (ret_fmt_private == 0) {
		fmt = format_private.pixelformat;
	} else {
		fmt = 0;
	}

	memset(&framesize, 0xff, sizeof(framesize));
	framesize.index = 0;
	framesize.pixel_format = fmt;
	ret_size = ioctl(get_video_fd(), VIDIOC_ENUM_FRAMESIZES, &framesize);
	errno_size = errno;

	dprintf
	    ("\tVIDIOC_ENUM_FRAMESIZES, index=%u, pixel_format=0x%x, ret_size=%i, errno_size=%i\n",
	     framesize.index, framesize.pixel_format, ret_size, errno_size);

	ret_null = ioctl(get_video_fd(), VIDIOC_ENUM_FRAMESIZES, NULL);
	errno_null = errno;

	dprintf("\tVIDIOC_ENUM_FRAMESIZES, ret_null=%i, errno_null=%i\n",
		ret_null, errno_null);

	if (ret_size == 0) {
		CU_ASSERT_EQUAL(ret_size, 0);
		CU_ASSERT_EQUAL(ret_null, -1);
		CU_ASSERT_EQUAL(errno_null, EFAULT);
	} else {
		CU_ASSERT_EQUAL(ret_size, -1);
		CU_ASSERT_EQUAL(errno_size, EINVAL);
		CU_ASSERT_EQUAL(ret_null, -1);
		CU_ASSERT_EQUAL(errno_null, EINVAL);
	}

}
