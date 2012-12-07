/*
 * v4l-test: Test environment for Video For Linux Two API
 *
 * 20 Apr 2009  0.9  Added string content validation
 * 18 Apr 2009  0.8  More strict check for strings
 * 13 Apr 2009  0.7  Also show type in debug output;
 *                   Add some debug output
 *  3 Apr 2009  0.6  Test case for NULL parameter reworked
 * 28 Mar 2009  0.5  Clean up ret and errno variable names and dprintf() output
 * 18 Mar 2009  0.4  Duplicated test for V4L2_BUF_TYPE_VIDEO_CAPTURE removed
 *  1 Jan 2009  0.3  Test cases added for index=S32_MAX and S32_MAX+1;
 *                   Test functions renamed
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

#include "test_VIDIOC_ENUM_FMT.h"

static void do_enumerate_formats(enum v4l2_buf_type type)
{
	int ret_enum, errno_enum;
	struct v4l2_fmtdesc format;
	struct v4l2_fmtdesc format2;
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
		     __FILE__, __LINE__, i, type, ret_enum, errno_enum);
		if (ret_enum == 0) {
			CU_ASSERT_EQUAL(ret_enum, 0);
			CU_ASSERT_EQUAL(format.index, i);
			//CU_ASSERT_EQUAL(format.type, ?);
			//CU_ASSERT_EQUAL(format.flags, ?);

			CU_ASSERT(0 < strlen((char *)format.description));
			CU_ASSERT(valid_string
				  ((char *)format.description,
				   sizeof(format.description)));

			//CU_ASSERT_EQUAL(format.pixelformat, ?);
			CU_ASSERT_EQUAL(format.reserved[0], 0);
			CU_ASSERT_EQUAL(format.reserved[1], 0);
			CU_ASSERT_EQUAL(format.reserved[2], 0);
			CU_ASSERT_EQUAL(format.reserved[3], 0);

			/* Check if the unused bytes of the description string is also filled
			 * with zeros. Also check if there is any padding byte between
			 * any two fields then this padding byte is also filled with zeros.
			 */
			memset(&format2, 0, sizeof(format2));
			format2.index = format.index;
			format2.type = format.type;
			format2.flags = format.flags;
			strncpy((char *)format2.description,
				(char *)format.description,
				sizeof(format2.description));
			format2.pixelformat = format.pixelformat;
			CU_ASSERT_EQUAL(memcmp
					(&format, &format2, sizeof(format)), 0);

			dprintf
			    ("\tformat = {.index=%u, .type=0x%X, .flags=0x%X, "
			     ".description=\"%s\", .pixelformat=0x%X, "
			     ".reserved[]={ 0x%X, 0x%X, 0x%X, 0x%X } }\n",
			     format.index, format.type, format.flags,
			     format.description, format.pixelformat,
			     format.reserved[0], format.reserved[1],
			     format.reserved[2], format.reserved[3]
			    );

		} else {
			CU_ASSERT_EQUAL(ret_enum, -1);
			CU_ASSERT_EQUAL(errno_enum, EINVAL);

			memset(&format2, 0xff, sizeof(format2));
			format2.index = i;
			format2.type = type;
			CU_ASSERT_EQUAL(memcmp
					(&format, &format2, sizeof(format)), 0);

		}
		i++;
	} while (ret_enum == 0);

}

void test_VIDIOC_ENUM_FMT()
{
	do_enumerate_formats(V4L2_BUF_TYPE_VIDEO_CAPTURE);
	do_enumerate_formats(V4L2_BUF_TYPE_VIDEO_OUTPUT);
	do_enumerate_formats(V4L2_BUF_TYPE_VIDEO_OVERLAY);
	do_enumerate_formats(V4L2_BUF_TYPE_VBI_CAPTURE);
	do_enumerate_formats(V4L2_BUF_TYPE_VBI_OUTPUT);
	do_enumerate_formats(V4L2_BUF_TYPE_SLICED_VBI_CAPTURE);
	do_enumerate_formats(V4L2_BUF_TYPE_SLICED_VBI_OUTPUT);
	do_enumerate_formats(V4L2_BUF_TYPE_VIDEO_OUTPUT_OVERLAY);
	do_enumerate_formats(V4L2_BUF_TYPE_PRIVATE);
}

void test_VIDIOC_ENUM_FMT_S32_MAX()
{
	int ret_enum, errno_enum;
	struct v4l2_fmtdesc format;
	struct v4l2_fmtdesc format2;

	/* test invalid index */
	memset(&format, 0xff, sizeof(format));
	format.index = (__u32) S32_MAX;
	format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	ret_enum = ioctl(get_video_fd(), VIDIOC_ENUM_FMT, &format);
	errno_enum = errno;

	dprintf
	    ("\t%s:%u: VIDIOC_ENUM_FMT, index=%u, type=%i, ret_enum=%i, errno_enum=%i\n",
	     __FILE__, __LINE__, format.index, format.type, ret_enum,
	     errno_enum);

	CU_ASSERT_EQUAL(ret_enum, -1);
	CU_ASSERT_EQUAL(errno_enum, EINVAL);

	/* Check whether the original format struct is untouched */
	memset(&format2, 0xff, sizeof(format2));
	format2.index = (__u32) S32_MAX;
	format2.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	CU_ASSERT_EQUAL(memcmp(&format, &format2, sizeof(format)), 0);
}

void test_VIDIOC_ENUM_FMT_S32_MAX_1()
{
	int ret_enum, errno_enum;
	struct v4l2_fmtdesc format;
	struct v4l2_fmtdesc format2;

	/* test invalid index */
	memset(&format, 0xff, sizeof(format));
	format.index = ((__u32) S32_MAX) + 1;
	format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	ret_enum = ioctl(get_video_fd(), VIDIOC_ENUM_FMT, &format);
	errno_enum = errno;

	dprintf
	    ("\t%s:%u: VIDIOC_ENUM_FMT, index=%u, type=%i, ret_enum=%i, errno_enum=%i\n",
	     __FILE__, __LINE__, format.index, format.type, ret_enum,
	     errno_enum);

	CU_ASSERT_EQUAL(ret_enum, -1);
	CU_ASSERT_EQUAL(errno_enum, EINVAL);

	/* Check whether the original format struct is untouched */
	memset(&format2, 0xff, sizeof(format2));
	format2.index = ((__u32) S32_MAX) + 1;
	format2.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	CU_ASSERT_EQUAL(memcmp(&format, &format2, sizeof(format)), 0);
}

void test_VIDIOC_ENUM_FMT_U32_MAX()
{
	int ret_enum, errno_enum;
	struct v4l2_fmtdesc format;
	struct v4l2_fmtdesc format2;

	/* test invalid index */
	memset(&format, 0xff, sizeof(format));
	format.index = U32_MAX;
	format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	ret_enum = ioctl(get_video_fd(), VIDIOC_ENUM_FMT, &format);
	errno_enum = errno;

	dprintf
	    ("\t%s:%u: VIDIOC_ENUM_FMT, index=%u, type=%i, ret_enum=%i, errno_enum=%i\n",
	     __FILE__, __LINE__, format.index, format.type, ret_enum,
	     errno_enum);

	CU_ASSERT_EQUAL(ret_enum, -1);
	CU_ASSERT_EQUAL(errno_enum, EINVAL);

	/* Check whether the original format struct is untouched */
	memset(&format2, 0xff, sizeof(format2));
	format2.index = U32_MAX;
	format2.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	CU_ASSERT_EQUAL(memcmp(&format, &format2, sizeof(format)), 0);
}

void test_VIDIOC_ENUM_FMT_invalid_type()
{
	int ret_enum, errno_enum;
	struct v4l2_fmtdesc format;
	struct v4l2_fmtdesc format2;
	int i;

	/* In this test case the .index is valid (0) and only the .type
	 * is invalid. The .type filed is an enum which is stored in an 'int'.
	 */

	/* test invalid .type=0 */
	memset(&format, 0xff, sizeof(format));
	format.index = 0;
	format.type = 0;
	ret_enum = ioctl(get_video_fd(), VIDIOC_ENUM_FMT, &format);
	errno_enum = errno;

	dprintf
	    ("\t%s:%u: VIDIOC_ENUM_FMT, index=%u, type=%i, ret_enum=%i, errno_enum=%i\n",
	     __FILE__, __LINE__, format.index, format.type, ret_enum,
	     errno_enum);

	CU_ASSERT_EQUAL(ret_enum, -1);
	CU_ASSERT_EQUAL(errno_enum, EINVAL);

	/* Check whether the original format struct is untouched */
	memset(&format2, 0xff, sizeof(format2));
	format2.index = 0;
	format2.type = 0;
	CU_ASSERT_EQUAL(memcmp(&format, &format2, sizeof(format)), 0);

	/* test invalid .type=SINT_MIN */
	memset(&format, 0xff, sizeof(format));
	format.index = 0;
	format.type = SINT_MIN;
	ret_enum = ioctl(get_video_fd(), VIDIOC_ENUM_FMT, &format);
	errno_enum = errno;

	dprintf
	    ("\t%s:%u: VIDIOC_ENUM_FMT, index=%u, type=%i, ret_enum=%i, errno_enum=%i\n",
	     __FILE__, __LINE__, format.index, format.type, ret_enum,
	     errno_enum);

	CU_ASSERT_EQUAL(ret_enum, -1);
	CU_ASSERT_EQUAL(errno_enum, EINVAL);

	/* Check whether the original format struct is untouched */
	memset(&format2, 0xff, sizeof(format2));
	format2.index = 0;
	format2.type = SINT_MIN;
	CU_ASSERT_EQUAL(memcmp(&format, &format2, sizeof(format)), 0);

	/* test invalid .type=-1 */
	memset(&format, 0xff, sizeof(format));
	format.index = 0;
	format.type = -1;
	ret_enum = ioctl(get_video_fd(), VIDIOC_ENUM_FMT, &format);
	errno_enum = errno;

	dprintf
	    ("\t%s:%u: VIDIOC_ENUM_FMT, index=%u, type=%i, ret_enum=%i, errno_enum=%i\n",
	     __FILE__, __LINE__, format.index, format.type, ret_enum,
	     errno_enum);

	CU_ASSERT_EQUAL(ret_enum, -1);
	CU_ASSERT_EQUAL(errno_enum, EINVAL);

	/* Check whether the original format struct is untouched */
	memset(&format2, 0xff, sizeof(format2));
	format2.index = 0;
	format2.type = -1;
	CU_ASSERT_EQUAL(memcmp(&format, &format2, sizeof(format)), 0);

	/* test invalid .type= 8..0x7F */
	for (i = V4L2_BUF_TYPE_VIDEO_OUTPUT_OVERLAY + 1;
	     i < V4L2_BUF_TYPE_PRIVATE; i++) {
		memset(&format, 0xff, sizeof(format));
		format.index = 0;
		format.type = i;
		ret_enum = ioctl(get_video_fd(), VIDIOC_ENUM_FMT, &format);
		errno_enum = errno;

		dprintf
		    ("\t%s:%u: VIDIOC_ENUM_FMT, index=%u, type=%i, ret_enum=%i, errno_enum=%i\n",
		     __FILE__, __LINE__, format.index, format.type, ret_enum,
		     errno_enum);

		CU_ASSERT_EQUAL(ret_enum, -1);
		CU_ASSERT_EQUAL(errno_enum, EINVAL);

		/* Check whether the original format struct is untouched */
		memset(&format2, 0xff, sizeof(format2));
		format2.index = 0;
		format2.type = i;
		CU_ASSERT_EQUAL(memcmp(&format, &format2, sizeof(format)), 0);
	}

	/* .type = 0x80..0x7FFF FFFF is the private range */

	/* Assume that 0x7FFF FFFF is invalid in the private range.
	 * This might be a wrong assumption, but let's have a test case like
	 * this for now.
	 */
	memset(&format, 0xff, sizeof(format));
	format.index = 0;
	format.type = SINT_MAX;
	ret_enum = ioctl(get_video_fd(), VIDIOC_ENUM_FMT, &format);
	errno_enum = errno;

	dprintf
	    ("\t%s:%u: VIDIOC_ENUM_FMT, index=%u, type=%i, ret_enum=%i, errno_enum=%i\n",
	     __FILE__, __LINE__, format.index, format.type, ret_enum,
	     errno_enum);

	CU_ASSERT_EQUAL(ret_enum, -1);
	CU_ASSERT_EQUAL(errno_enum, EINVAL);

	/* Check whether the original format struct is untouched */
	memset(&format2, 0xff, sizeof(format2));
	format2.index = 0;
	format2.type = SINT_MAX;
	CU_ASSERT_EQUAL(memcmp(&format, &format2, sizeof(format)), 0);
}

void test_VIDIOC_ENUM_FMT_NULL()
{
	int ret_capture, errno_capture;
	int ret_output, errno_output;
	int ret_video_overlay, errno_video_overlay;
	int ret_vbi_capture, errno_vbi_capture;
	int ret_vbi_output, errno_vbi_output;
	int ret_sliced_vbi_capture, errno_sliced_vbi_capture;
	int ret_sliced_vbi_output, errno_sliced_vbi_output;
	int ret_video_output_overlay, errno_video_output_overlay;
	int ret_private, errno_private;
	int ret_null, errno_null;
	struct v4l2_fmtdesc format;

	memset(&format, 0xff, sizeof(format));
	format.index = 0;
	format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	ret_capture = ioctl(get_video_fd(), VIDIOC_ENUM_FMT, &format);
	errno_capture = errno;

	dprintf("\t%s:%u: VIDIOC_ENUM_FMT, ret_capture=%i, errno_capture=%i\n",
		__FILE__, __LINE__, ret_capture, errno_capture);

	memset(&format, 0xff, sizeof(format));
	format.index = 0;
	format.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
	ret_output = ioctl(get_video_fd(), VIDIOC_ENUM_FMT, &format);
	errno_output = errno;

	dprintf("\t%s:%u: VIDIOC_ENUM_FMT, ret_output=%i, errno_output=%i\n",
		__FILE__, __LINE__, ret_output, errno_output);

	memset(&format, 0xff, sizeof(format));
	format.index = 0;
	format.type = V4L2_BUF_TYPE_VIDEO_OVERLAY;
	ret_video_overlay = ioctl(get_video_fd(), VIDIOC_ENUM_FMT, &format);
	errno_video_overlay = errno;

	dprintf
	    ("\t%s:%u: VIDIOC_ENUM_FMT, ret_video_overlay=%i, errno_video_overlay=%i\n",
	     __FILE__, __LINE__, ret_video_overlay, errno_video_overlay);

	memset(&format, 0xff, sizeof(format));
	format.index = 0;
	format.type = V4L2_BUF_TYPE_VBI_CAPTURE;
	ret_vbi_capture = ioctl(get_video_fd(), VIDIOC_ENUM_FMT, &format);
	errno_vbi_capture = errno;

	dprintf
	    ("\t%s:%u: VIDIOC_ENUM_FMT, ret_vbi_capture=%i, errno_vbi_capture=%i\n",
	     __FILE__, __LINE__, ret_vbi_capture, errno_vbi_capture);

	memset(&format, 0xff, sizeof(format));
	format.index = 0;
	format.type = V4L2_BUF_TYPE_VBI_OUTPUT;
	ret_vbi_output = ioctl(get_video_fd(), VIDIOC_ENUM_FMT, &format);
	errno_vbi_output = errno;

	dprintf
	    ("\t%s:%u: VIDIOC_ENUM_FMT, ret_vbi_output=%i, errno_vbi_output=%i\n",
	     __FILE__, __LINE__, ret_vbi_output, errno_vbi_output);

	memset(&format, 0xff, sizeof(format));
	format.index = 0;
	format.type = V4L2_BUF_TYPE_SLICED_VBI_CAPTURE;
	ret_sliced_vbi_capture =
	    ioctl(get_video_fd(), VIDIOC_ENUM_FMT, &format);
	errno_sliced_vbi_capture = errno;

	dprintf
	    ("\t%s:%u: VIDIOC_ENUM_FMT, ret_sliced_vbi_capture=%i, errno_sliced_vbi_capture=%i\n",
	     __FILE__, __LINE__, ret_sliced_vbi_capture,
	     errno_sliced_vbi_capture);

	memset(&format, 0xff, sizeof(format));
	format.index = 0;
	format.type = V4L2_BUF_TYPE_SLICED_VBI_OUTPUT;
	ret_sliced_vbi_output = ioctl(get_video_fd(), VIDIOC_ENUM_FMT, &format);
	errno_sliced_vbi_output = errno;

	dprintf
	    ("\t%s:%u: VIDIOC_ENUM_FMT, ret_sliced_vbi_output=%i, errno_sliced_vbi_output=%i\n",
	     __FILE__, __LINE__, ret_sliced_vbi_output,
	     errno_sliced_vbi_output);

	memset(&format, 0xff, sizeof(format));
	format.index = 0;
	format.type = V4L2_BUF_TYPE_VIDEO_OUTPUT_OVERLAY;
	ret_video_output_overlay =
	    ioctl(get_video_fd(), VIDIOC_ENUM_FMT, &format);
	errno_video_output_overlay = errno;

	dprintf
	    ("\t%s:%u: VIDIOC_ENUM_FMT, ret_video_output_overlay=%i, errno_video_output_overlay=%i\n",
	     __FILE__, __LINE__, ret_video_output_overlay,
	     errno_video_output_overlay);

	memset(&format, 0xff, sizeof(format));
	format.index = 0;
	format.type = V4L2_BUF_TYPE_PRIVATE;
	ret_private = ioctl(get_video_fd(), VIDIOC_ENUM_FMT, &format);
	errno_private = errno;

	dprintf("\t%s:%u: VIDIOC_ENUM_FMT, ret_private=%i, errno_private=%i\n",
		__FILE__, __LINE__, ret_private, errno_private);

	ret_null = ioctl(get_video_fd(), VIDIOC_ENUM_FMT, NULL);
	errno_null = errno;

	dprintf("\t%s:%u: VIDIOC_ENUM_FMT, ret_null=%i, errno_null=%i\n",
		__FILE__, __LINE__, ret_null, errno_null);

	if (ret_capture == 0 || ret_output == 0 ||
	    ret_video_overlay == 0 || ret_vbi_capture == 0 ||
	    ret_vbi_output == 0 || ret_sliced_vbi_capture == 0 ||
	    ret_sliced_vbi_output == 0 || ret_video_output_overlay == 0 ||
	    ret_private == 0) {
		CU_ASSERT_EQUAL(ret_null, -1);
		CU_ASSERT_EQUAL(errno_null, EFAULT);
	} else {
		CU_ASSERT_EQUAL(ret_capture, -1);
		CU_ASSERT_EQUAL(errno_null, EINVAL);
		CU_ASSERT_EQUAL(ret_output, -1);
		CU_ASSERT_EQUAL(errno_output, EINVAL);
		CU_ASSERT_EQUAL(ret_video_overlay, -1);
		CU_ASSERT_EQUAL(errno_video_overlay, EINVAL);
		CU_ASSERT_EQUAL(ret_vbi_capture, -1);
		CU_ASSERT_EQUAL(errno_vbi_capture, EINVAL);
		CU_ASSERT_EQUAL(ret_vbi_output, -1);
		CU_ASSERT_EQUAL(errno_vbi_output, EINVAL);
		CU_ASSERT_EQUAL(ret_sliced_vbi_capture, -1);
		CU_ASSERT_EQUAL(errno_sliced_vbi_capture, EINVAL);
		CU_ASSERT_EQUAL(ret_sliced_vbi_output, -1);
		CU_ASSERT_EQUAL(errno_sliced_vbi_output, EINVAL);
		CU_ASSERT_EQUAL(ret_video_output_overlay, -1);
		CU_ASSERT_EQUAL(errno_video_output_overlay, EINVAL);
		CU_ASSERT_EQUAL(ret_private, -1);
		CU_ASSERT_EQUAL(errno_private, EINVAL);
		CU_ASSERT_EQUAL(ret_null, -1);
		CU_ASSERT_EQUAL(errno_null, EINVAL);
	}

}
