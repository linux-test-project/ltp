/*
 * v4l-test: Test environment for Video For Linux Two API
 *
 *  4 Apr 2009  0.1  First release
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

#include "test_VIDIOC_FMT.h"

static void do_get_formats(enum v4l2_buf_type type) {
	int ret_get, errno_get;
	struct v4l2_format format;
	struct v4l2_format format2;

	memset(&format, 0xff, sizeof(format));
	format.type = type;

	ret_get = ioctl(get_video_fd(), VIDIOC_G_FMT, &format);
	errno_get = errno;

	dprintf("\t%s:%u: VIDIOC_G_FMT, ret_get=%i, errno_get=%i\n",
		__FILE__, __LINE__, ret_get, errno_get);
	if (ret_get == 0) {
		CU_ASSERT_EQUAL(ret_get, 0);
		CU_ASSERT_EQUAL(format.type, type);

		switch (format.type) {
			case V4L2_BUF_TYPE_VIDEO_CAPTURE:
			case V4L2_BUF_TYPE_VIDEO_OUTPUT:
				dprintf("\tformat = {.type=0x%X, .fmt.pix = { "
					".width=%u, "
					".height=%u, "
					".pixelformat=0x%X, "
					".field=%i, "
					".bytesperline=%i, "
					".sizeimage=%u, "
					".colorspace=%i, "
					".priv=0x%X "
					" } }\n",
					format.type,
					format.fmt.pix.width,
					format.fmt.pix.height,
					format.fmt.pix.pixelformat,
					format.fmt.pix.field,
					format.fmt.pix.bytesperline,
					format.fmt.pix.sizeimage,
					format.fmt.pix.colorspace,
					format.fmt.pix.priv
				);
				/* TODO: check different fields */
				//CU_ASSERT_EQUAL(format.fmt.pix.width, ???);
				//CU_ASSERT_EQUAL(format.fmt.pix.height, ???);
				//CU_ASSERT_EQUAL(format.fmt.pix.pixelformat, ???);
				//CU_ASSERT_EQUAL(format.fmt.pix.field, ???);
				//CU_ASSERT_EQUAL(format.fmt.pix.bytesperline, ???);
				//CU_ASSERT_EQUAL(format.fmt.pix.sizeimage, ???);
				//CU_ASSERT_EQUAL(format.fmt.pix.colorspace, ???);
				//CU_ASSERT_EQUAL(format.fmt.pix.priv, ???);
				break;

			case V4L2_BUF_TYPE_VIDEO_OVERLAY:
			case V4L2_BUF_TYPE_VIDEO_OUTPUT_OVERLAY:
				dprintf("\tformat = {.type=0x%X, .fmt.win={ "
					".w = { .left=%i, .top=%i, .width=%i, .height=%i, }, "
					".field = %i, "
					".chromakey = 0x%X, "
					".clips = %p, "
					".clipcount = %u, "
					".bitmap = %p, "
					".global_alpha = %u "
					"} }\n",
					format.type,
					format.fmt.win.w.left,
					format.fmt.win.w.top,
					format.fmt.win.w.width,
					format.fmt.win.w.height,
					format.fmt.win.field,
					format.fmt.win.chromakey,
					format.fmt.win.clips,
					format.fmt.win.clipcount,
					format.fmt.win.bitmap,
					format.fmt.win.global_alpha
				);
				/* TODO: check different fields */
				//CU_ASSERT_EQUAL(format.fmt.win.w.left, ???);
				//CU_ASSERT_EQUAL(format.fmt.win.w.top, ???);
				//CU_ASSERT_EQUAL(format.fmt.win.w.width, ???);
				//CU_ASSERT_EQUAL(format.fmt.win.w.height, ???);
				//CU_ASSERT_EQUAL(format.fmt.win.field, ???);
				//CU_ASSERT_EQUAL(format.fmt.win.chromakey, ???);
				//CU_ASSERT_EQUAL(format.fmt.win.clips, ???);
				//CU_ASSERT_EQUAL(format.fmt.win.clipcount, ???);
				//CU_ASSERT_EQUAL(format.fmt.win.bitmap, ???);
				//CU_ASSERT_EQUAL(format.fmt.win.global_alpha ???);
				break;

			case V4L2_BUF_TYPE_VBI_CAPTURE:
			case V4L2_BUF_TYPE_VBI_OUTPUT:
				dprintf("\tformat = {.type=0x%X, .fmt.vbi={ "
					".sampling_rate=%u, "
					".offset=%u, "
					".samples_per_line=%u "
					".sample_format=0x%X "
					".start = { %u, %u }, "
					".count = { %u, %u }, "
					".flags = 0x%X, "
					".reserved = { 0x%X, 0x%X } "
					"} }\n",
					format.type,
					format.fmt.vbi.sampling_rate,
					format.fmt.vbi.offset,
					format.fmt.vbi.samples_per_line,
					format.fmt.vbi.sample_format,
					format.fmt.vbi.start[0],
					format.fmt.vbi.start[1],
					format.fmt.vbi.count[0],
					format.fmt.vbi.count[1],
					format.fmt.vbi.flags,
					format.fmt.vbi.reserved[0],
					format.fmt.vbi.reserved[1]
				);

				/* TODO: check different fields */
				//CU_ASSERT_EQUAL(format.fmt.vbi.sampling_rate, ???);
				//CU_ASSERT_EQUAL(format.fmt.vbi.offset, ???);
				//CU_ASSERT_EQUAL(format.fmt.vbi.samples_per_line, ???);
				//CU_ASSERT_EQUAL(format.fmt.vbi.sample_format, ???);
				//CU_ASSERT_EQUAL(format.fmt.vbi.start[0], ???);
				//CU_ASSERT_EQUAL(format.fmt.vbi.start[1], ???);
				//CU_ASSERT_EQUAL(format.fmt.vbi.count[0], ???);
				//CU_ASSERT_EQUAL(format.fmt.vbi.count[1], ???);
				//CU_ASSERT_EQUAL(format.fmt.vbi.flags, ???);
				CU_ASSERT_EQUAL(format.fmt.vbi.reserved[0], 0);
				CU_ASSERT_EQUAL(format.fmt.vbi.reserved[1], 0);
				break;

			case V4L2_BUF_TYPE_SLICED_VBI_CAPTURE:
			case V4L2_BUF_TYPE_SLICED_VBI_OUTPUT:
				dprintf("\tformat = {.type=0x%X, "
					".fmt.sliced = { .service_set = 0x%X, "
					".service_lines = { ... }, "
					".io_size = %u, "
					".reserved[0] = 0x%X, "
					".reserved[1] = 0x%X "
					"} }\n",
					format.type,
					format.fmt.sliced.service_set,
					//format.fmt.sliced.service_lines[][],
					format.fmt.sliced.io_size,
					format.fmt.sliced.reserved[0],
					format.fmt.sliced.reserved[1]
				);
				/* TODO: check different fields */
				//CU_ASSERT_EQUAL(format.fmt.sliced.service_set, ???);
				CU_ASSERT_EQUAL(format.fmt.sliced.service_lines[0][0], 0);
				//CU_ASSERT_EQUAL(format.fmt.sliced.service_lines[0][1], ???);
				//CU_ASSERT_EQUAL(format.fmt.sliced.service_lines[0][2], ???);
				//CU_ASSERT_EQUAL(format.fmt.sliced.service_lines[0][3], ???);
				//CU_ASSERT_EQUAL(format.fmt.sliced.service_lines[0][4], ???);
				//CU_ASSERT_EQUAL(format.fmt.sliced.service_lines[0][5], ???);
				//CU_ASSERT_EQUAL(format.fmt.sliced.service_lines[0][6], ???);
				//CU_ASSERT_EQUAL(format.fmt.sliced.service_lines[0][7], ???);
				//CU_ASSERT_EQUAL(format.fmt.sliced.service_lines[0][8], ???);
				//CU_ASSERT_EQUAL(format.fmt.sliced.service_lines[0][9], ???);
				//CU_ASSERT_EQUAL(format.fmt.sliced.service_lines[0][10], ???);
				//CU_ASSERT_EQUAL(format.fmt.sliced.service_lines[0][11], ???);
				//CU_ASSERT_EQUAL(format.fmt.sliced.service_lines[0][12], ???);
				//CU_ASSERT_EQUAL(format.fmt.sliced.service_lines[0][13], ???);
				//CU_ASSERT_EQUAL(format.fmt.sliced.service_lines[0][14], ???);
				//CU_ASSERT_EQUAL(format.fmt.sliced.service_lines[0][15], ???);
				//CU_ASSERT_EQUAL(format.fmt.sliced.service_lines[0][16], ???);
				//CU_ASSERT_EQUAL(format.fmt.sliced.service_lines[0][17], ???);
				//CU_ASSERT_EQUAL(format.fmt.sliced.service_lines[0][18], ???);
				//CU_ASSERT_EQUAL(format.fmt.sliced.service_lines[0][19], ???);
				//CU_ASSERT_EQUAL(format.fmt.sliced.service_lines[0][20], ???);
				//CU_ASSERT_EQUAL(format.fmt.sliced.service_lines[0][21], ???);
				//CU_ASSERT_EQUAL(format.fmt.sliced.service_lines[0][22], ???);
				//CU_ASSERT_EQUAL(format.fmt.sliced.service_lines[0][23], ???);
				CU_ASSERT_EQUAL(format.fmt.sliced.service_lines[1][0], 0);
				//CU_ASSERT_EQUAL(format.fmt.sliced.service_lines[1][1], ???);
				//CU_ASSERT_EQUAL(format.fmt.sliced.service_lines[1][2], ???);
				//CU_ASSERT_EQUAL(format.fmt.sliced.service_lines[1][3], ???);
				//CU_ASSERT_EQUAL(format.fmt.sliced.service_lines[1][4], ???);
				//CU_ASSERT_EQUAL(format.fmt.sliced.service_lines[1][5], ???);
				//CU_ASSERT_EQUAL(format.fmt.sliced.service_lines[1][6], ???);
				//CU_ASSERT_EQUAL(format.fmt.sliced.service_lines[1][7], ???);
				//CU_ASSERT_EQUAL(format.fmt.sliced.service_lines[1][8], ???);
				//CU_ASSERT_EQUAL(format.fmt.sliced.service_lines[1][9], ???);
				//CU_ASSERT_EQUAL(format.fmt.sliced.service_lines[1][10], ???);
				//CU_ASSERT_EQUAL(format.fmt.sliced.service_lines[1][11], ???);
				//CU_ASSERT_EQUAL(format.fmt.sliced.service_lines[1][12], ???);
				//CU_ASSERT_EQUAL(format.fmt.sliced.service_lines[1][13], ???);
				//CU_ASSERT_EQUAL(format.fmt.sliced.service_lines[1][14], ???);
				//CU_ASSERT_EQUAL(format.fmt.sliced.service_lines[1][15], ???);
				//CU_ASSERT_EQUAL(format.fmt.sliced.service_lines[1][16], ???);
				//CU_ASSERT_EQUAL(format.fmt.sliced.service_lines[1][17], ???);
				//CU_ASSERT_EQUAL(format.fmt.sliced.service_lines[1][18], ???);
				//CU_ASSERT_EQUAL(format.fmt.sliced.service_lines[1][19], ???);
				//CU_ASSERT_EQUAL(format.fmt.sliced.service_lines[1][20], ???);
				//CU_ASSERT_EQUAL(format.fmt.sliced.service_lines[1][21], ???);
				//CU_ASSERT_EQUAL(format.fmt.sliced.service_lines[1][22], ???);
				//CU_ASSERT_EQUAL(format.fmt.sliced.service_lines[1][23], ???);
				//CU_ASSERT_EQUAL(format.fmt.sliced.io_size, ???);
				CU_ASSERT_EQUAL(format.fmt.sliced.reserved[0], 0);
				CU_ASSERT_EQUAL(format.fmt.sliced.reserved[1], 0);

				break;

			case V4L2_BUF_TYPE_PRIVATE:
				dprintf("\tformat = {.type=0x%X, ... } }\n",
					format.type
				);
				/* TODO: check different fields */
		}

	} else {
		CU_ASSERT_EQUAL(ret_get, -1);
		CU_ASSERT_EQUAL(errno_get, EINVAL);

		memset(&format2, 0xff, sizeof(format2));
		format2.type = type;
		CU_ASSERT_EQUAL(memcmp(&format, &format2, sizeof(format)), 0);

	}

}

void test_VIDIOC_G_FMT() {
	do_get_formats(V4L2_BUF_TYPE_VIDEO_CAPTURE);
	do_get_formats(V4L2_BUF_TYPE_VIDEO_OUTPUT);
	do_get_formats(V4L2_BUF_TYPE_VIDEO_OVERLAY);
	do_get_formats(V4L2_BUF_TYPE_VBI_CAPTURE);
	do_get_formats(V4L2_BUF_TYPE_VBI_OUTPUT);
	do_get_formats(V4L2_BUF_TYPE_SLICED_VBI_CAPTURE);
	do_get_formats(V4L2_BUF_TYPE_SLICED_VBI_OUTPUT);
	do_get_formats(V4L2_BUF_TYPE_VIDEO_OUTPUT_OVERLAY);
	do_get_formats(V4L2_BUF_TYPE_PRIVATE);
}

void test_VIDIOC_G_FMT_invalid_type() {
	int ret_get, errno_get;
	struct v4l2_format format;
	struct v4l2_format format2;
	int i;

	/* In this test case the .index is valid (0) and only the .type 
	 * is invalid. The .type filed is an enum which is stored in an 'int'.
	 */

	/* test invalid .type=0 */
	memset(&format, 0xff, sizeof(format));
	format.type = 0;
	ret_get = ioctl(get_video_fd(), VIDIOC_G_FMT, &format);
	errno_get = errno;

	dprintf("\t%s:%u: VIDIOC_G_FMT, ret_get=%i, errno_get=%i\n",
		__FILE__, __LINE__, ret_get, errno_get);

	CU_ASSERT_EQUAL(ret_get, -1);
	CU_ASSERT_EQUAL(errno_get, EINVAL);

	/* Check whether the original format struct is untouched */
	memset(&format2, 0xff, sizeof(format2));
	format2.type = 0;
	CU_ASSERT_EQUAL(memcmp(&format, &format2, sizeof(format)), 0);

	/* test invalid .type=SINT_MIN */
	memset(&format, 0xff, sizeof(format));
	format.type = SINT_MIN;
	ret_get = ioctl(get_video_fd(), VIDIOC_G_FMT, &format);
	errno_get = errno;

	dprintf("\t%s:%u: VIDIOC_G_FMT, ret_get=%i, errno_get=%i\n",
		__FILE__, __LINE__, ret_get, errno_get);

	CU_ASSERT_EQUAL(ret_get, -1);
	CU_ASSERT_EQUAL(errno_get, EINVAL);

	/* Check whether the original format struct is untouched */
	memset(&format2, 0xff, sizeof(format2));
	format2.type = SINT_MIN;
	CU_ASSERT_EQUAL(memcmp(&format, &format2, sizeof(format)), 0);

	/* test invalid .type=-1 */
	memset(&format, 0xff, sizeof(format));
	format.type = -1;
	ret_get = ioctl(get_video_fd(), VIDIOC_G_FMT, &format);
	errno_get = errno;

	dprintf("\t%s:%u: VIDIOC_G_FMT, ret_get=%i, errno_get=%i\n",
		__FILE__, __LINE__, ret_get, errno_get);

	CU_ASSERT_EQUAL(ret_get, -1);
	CU_ASSERT_EQUAL(errno_get, EINVAL);

	/* Check whether the original format struct is untouched */
	memset(&format2, 0xff, sizeof(format2));
	format2.type = -1;
	CU_ASSERT_EQUAL(memcmp(&format, &format2, sizeof(format)), 0);

	/* test invalid .type= 8..0x7F */
	for (i = V4L2_BUF_TYPE_VIDEO_OUTPUT_OVERLAY+1; i < V4L2_BUF_TYPE_PRIVATE; i++) {
		memset(&format, 0xff, sizeof(format));
		format.type = i;
		ret_get = ioctl(get_video_fd(), VIDIOC_G_FMT, &format);
		errno_get = errno;

		dprintf("\t%s:%u: VIDIOC_G_FMT, ret_get=%i, errno_get=%i\n",
			__FILE__, __LINE__, ret_get, errno_get);

		CU_ASSERT_EQUAL(ret_get, -1);
		CU_ASSERT_EQUAL(errno_get, EINVAL);

		/* Check whether the original format struct is untouched */
		memset(&format2, 0xff, sizeof(format2));
		format2.type = i;
		CU_ASSERT_EQUAL(memcmp(&format, &format2, sizeof(format)), 0);
	}

	/* .type = 0x80..0x7FFF FFFF is the private range */

	/* Assume that 0x7FFF FFFF is invalid in the private range.
	 * This might be a wrong assumption, but let's have a test case like
	 * this for now.
	 */
	memset(&format, 0xff, sizeof(format));
	format.type = SINT_MAX;
	ret_get = ioctl(get_video_fd(), VIDIOC_G_FMT, &format);
	errno_get = errno;

	dprintf("\t%s:%u: VIDIOC_G_FMT, ret_get=%i, errno_get=%i\n",
		__FILE__, __LINE__, ret_get, errno_get);

	CU_ASSERT_EQUAL(ret_get, -1);
	CU_ASSERT_EQUAL(errno_get, EINVAL);

	/* Check whether the original format struct is untouched */
	memset(&format2, 0xff, sizeof(format2));
	format2.type = SINT_MAX;
	CU_ASSERT_EQUAL(memcmp(&format, &format2, sizeof(format)), 0);
}

void test_VIDIOC_G_FMT_NULL() {
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
	struct v4l2_format format;

	memset(&format, 0xff, sizeof(format));
	format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	ret_capture = ioctl(get_video_fd(), VIDIOC_G_FMT, &format);
	errno_capture = errno;

	dprintf("\t%s:%u: VIDIOC_G_FMT, ret_capture=%i, errno_capture=%i\n",
		__FILE__, __LINE__, ret_capture, errno_capture);

	memset(&format, 0xff, sizeof(format));
	format.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
	ret_output = ioctl(get_video_fd(), VIDIOC_G_FMT, &format);
	errno_output = errno;

	dprintf("\t%s:%u: VIDIOC_G_FMT, ret_output=%i, errno_output=%i\n",
		__FILE__, __LINE__, ret_output, errno_output);

	memset(&format, 0xff, sizeof(format));
	format.type = V4L2_BUF_TYPE_VIDEO_OVERLAY;
	ret_video_overlay = ioctl(get_video_fd(), VIDIOC_G_FMT, &format);
	errno_video_overlay = errno;

	dprintf("\t%s:%u: VIDIOC_G_FMT, ret_video_overlay=%i, errno_video_overlay=%i\n",
		__FILE__, __LINE__, ret_video_overlay, errno_video_overlay);

	memset(&format, 0xff, sizeof(format));
	format.type = V4L2_BUF_TYPE_VBI_CAPTURE;
	ret_vbi_capture = ioctl(get_video_fd(), VIDIOC_G_FMT, &format);
	errno_vbi_capture = errno;

	dprintf("\t%s:%u: VIDIOC_G_FMT, ret_vbi_capture=%i, errno_vbi_capture=%i\n",
		__FILE__, __LINE__, ret_vbi_capture, errno_vbi_capture);

	memset(&format, 0xff, sizeof(format));
	format.type = V4L2_BUF_TYPE_VBI_OUTPUT;
	ret_vbi_output = ioctl(get_video_fd(), VIDIOC_G_FMT, &format);
	errno_vbi_output = errno;

	dprintf("\t%s:%u: VIDIOC_G_FMT, ret_vbi_output=%i, errno_vbi_output=%i\n",
		__FILE__, __LINE__, ret_vbi_output, errno_vbi_output);

	memset(&format, 0xff, sizeof(format));
	format.type = V4L2_BUF_TYPE_SLICED_VBI_CAPTURE;
	ret_sliced_vbi_capture = ioctl(get_video_fd(), VIDIOC_G_FMT, &format);
	errno_sliced_vbi_capture = errno;

	dprintf("\t%s:%u: VIDIOC_G_FMT, ret_sliced_vbi_capture=%i, errno_sliced_vbi_capture=%i\n",
		__FILE__, __LINE__, ret_sliced_vbi_capture, errno_sliced_vbi_capture);

	memset(&format, 0xff, sizeof(format));
	format.type = V4L2_BUF_TYPE_SLICED_VBI_OUTPUT;
	ret_sliced_vbi_output = ioctl(get_video_fd(), VIDIOC_G_FMT, &format);
	errno_sliced_vbi_output = errno;

	dprintf("\t%s:%u: VIDIOC_G_FMT, ret_sliced_vbi_output=%i, errno_sliced_vbi_output=%i\n",
		__FILE__, __LINE__, ret_sliced_vbi_output, errno_sliced_vbi_output);

	memset(&format, 0xff, sizeof(format));
	format.type = V4L2_BUF_TYPE_VIDEO_OUTPUT_OVERLAY;
	ret_video_output_overlay = ioctl(get_video_fd(), VIDIOC_G_FMT, &format);
	errno_video_output_overlay = errno;

	dprintf("\t%s:%u: VIDIOC_G_FMT, ret_video_output_overlay=%i, errno_video_output_overlay=%i\n",
		__FILE__, __LINE__, ret_video_output_overlay, errno_video_output_overlay);

	memset(&format, 0xff, sizeof(format));
	format.type = V4L2_BUF_TYPE_PRIVATE;
	ret_private = ioctl(get_video_fd(), VIDIOC_G_FMT, &format);
	errno_private = errno;

	dprintf("\t%s:%u: VIDIOC_G_FMT, ret_private=%i, errno_private=%i\n",
		__FILE__, __LINE__, ret_private, errno_private);

	ret_null = ioctl(get_video_fd(), VIDIOC_G_FMT, NULL);
	errno_null = errno;

	dprintf("\t%s:%u: VIDIOC_G_FMT, ret_null=%i, errno_null=%i\n",
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

/* TODO: test cases for VIDIOC_S_FMT and VIDIOC_TRY_FMT */
