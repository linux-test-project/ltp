/*
 * v4l-test: Test environment for Video For Linux Two API
 *
 * 19 Jul 2009  0.4  More V4L2_PIX_FMT_* formats added to valid_poxelformat()
 * 18 Apr 2009  0.3  Type added to debug printouts
 * 15 Apr 2009  0.2  Added test case for VIDIOC_S_FMT
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

int valid_pixelformat(__u32 pixelformat)
{
	int valid = 0;

	switch (pixelformat) {
	case V4L2_PIX_FMT_RGB332:
	case V4L2_PIX_FMT_RGB444:
	case V4L2_PIX_FMT_RGB555:
	case V4L2_PIX_FMT_RGB565:
	case V4L2_PIX_FMT_RGB555X:
	case V4L2_PIX_FMT_RGB565X:
	case V4L2_PIX_FMT_BGR24:
	case V4L2_PIX_FMT_RGB24:
	case V4L2_PIX_FMT_BGR32:
	case V4L2_PIX_FMT_RGB32:
	case V4L2_PIX_FMT_GREY:
	case V4L2_PIX_FMT_Y16:
	case V4L2_PIX_FMT_PAL8:
	case V4L2_PIX_FMT_YVU410:
	case V4L2_PIX_FMT_YVU420:
	case V4L2_PIX_FMT_YUYV:
	case V4L2_PIX_FMT_UYVY:
	case V4L2_PIX_FMT_YUV422P:
	case V4L2_PIX_FMT_YUV411P:
	case V4L2_PIX_FMT_Y41P:
	case V4L2_PIX_FMT_YUV444:
	case V4L2_PIX_FMT_YUV555:
	case V4L2_PIX_FMT_YUV565:
	case V4L2_PIX_FMT_YUV32:
	case V4L2_PIX_FMT_NV12:
	case V4L2_PIX_FMT_NV21:
	case V4L2_PIX_FMT_YUV410:
	case V4L2_PIX_FMT_YUV420:
	case V4L2_PIX_FMT_YYUV:
	case V4L2_PIX_FMT_HI240:
	case V4L2_PIX_FMT_HM12:
	case V4L2_PIX_FMT_SBGGR8:
	case V4L2_PIX_FMT_SBGGR16:
	case V4L2_PIX_FMT_MJPEG:
	case V4L2_PIX_FMT_JPEG:
	case V4L2_PIX_FMT_DV:
	case V4L2_PIX_FMT_MPEG:
	case V4L2_PIX_FMT_WNVA:
	case V4L2_PIX_FMT_SN9C10X:
	case V4L2_PIX_FMT_PWC1:
	case V4L2_PIX_FMT_PWC2:
	case V4L2_PIX_FMT_ET61X251:

		/* formats from Linux kernel 2.6.31-rc2 */

#ifdef V4L2_PIX_FMT_VYUY
	case V4L2_PIX_FMT_VYUY:
#endif

#ifdef V4L2_PIX_FMT_NV16
	case V4L2_PIX_FMT_NV16:
#endif

#ifdef V4L2_PIX_FMT_NV61
	case V4L2_PIX_FMT_NV61:
#endif

#ifdef V4L2_PIX_FMT_SGBRG8
	case V4L2_PIX_FMT_SGBRG8:
#endif

#ifdef V4L2_PIX_FMT_SGRBG8
	case V4L2_PIX_FMT_SGRBG8:
#endif

#ifdef V4L2_PIX_FMT_SGRBG10
	case V4L2_PIX_FMT_SGRBG10:
#endif

#ifdef V4L2_PIX_FMT_SGRBG10DPCM8
	case V4L2_PIX_FMT_SGRBG10DPCM8:
#endif

#ifdef V4L2_PIX_FMT_SPCA501
	case V4L2_PIX_FMT_SPCA501:
#endif

#ifdef V4L2_PIX_FMT_SPCA505
	case V4L2_PIX_FMT_SPCA505:
#endif

#ifdef V4L2_PIX_FMT_SPCA508
	case V4L2_PIX_FMT_SPCA508:
#endif

#ifdef V4L2_PIX_FMT_SPCA561
	case V4L2_PIX_FMT_SPCA561:
#endif

#ifdef V4L2_PIX_FMT_PAC207
	case V4L2_PIX_FMT_PAC207:
#endif

#ifdef V4L2_PIX_FMT_MR97310A
	case V4L2_PIX_FMT_MR97310A:
#endif

#ifdef V4L2_PIX_FMT_SQ905C
	case V4L2_PIX_FMT_SQ905C:
#endif

#ifdef V4L2_PIX_FMT_PJPG
	case V4L2_PIX_FMT_PJPG:
#endif

#ifdef V4L2_PIX_FMT_YVYU
	case V4L2_PIX_FMT_YVYU:
#endif

#ifdef V4L2_PIX_FMT_OV511
	case V4L2_PIX_FMT_OV511:
#endif

#ifdef V4L2_PIX_FMT_OV518
	case V4L2_PIX_FMT_OV518:
#endif

		valid = 1;
		break;
	default:
		valid = 0;
	}

	return valid;
}

int valid_colorspace(enum v4l2_colorspace colorspace)
{
	int valid = 0;

	switch (colorspace) {
	case V4L2_COLORSPACE_SMPTE170M:
	case V4L2_COLORSPACE_SMPTE240M:
	case V4L2_COLORSPACE_REC709:
	case V4L2_COLORSPACE_BT878:
	case V4L2_COLORSPACE_470_SYSTEM_M:
	case V4L2_COLORSPACE_470_SYSTEM_BG:
	case V4L2_COLORSPACE_JPEG:
	case V4L2_COLORSPACE_SRGB:
		valid = 1;
		break;
	default:
		valid = 0;
	}

	return valid;
}

static void do_get_formats(enum v4l2_buf_type type)
{
	int ret_get, errno_get;
	struct v4l2_format format;
	struct v4l2_format format2;
	unsigned int j;

	memset(&format, 0xff, sizeof(format));
	format.type = type;

	ret_get = ioctl(get_video_fd(), VIDIOC_G_FMT, &format);
	errno_get = errno;

	dprintf("\t%s:%u: VIDIOC_G_FMT, type=%i, ret_get=%i, errno_get=%i\n",
		__FILE__, __LINE__, type, ret_get, errno_get);
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
				format.fmt.pix.colorspace, format.fmt.pix.priv);
			if (sizeof(format.fmt.pix) <
			    sizeof(format.fmt.raw_data)) {
				dprintf1
				    ("\tformat = { ..., .fmt.raw_data[] = { ...");
				for (j = sizeof(format.fmt.pix);
				     j < sizeof(format.fmt.raw_data); j++) {
					dprintf(", 0x%x",
						format.fmt.raw_data[j]);
				}
				dprintf1(" }}\n");
			}

			/* TODO: check different fields */
			//CU_ASSERT_EQUAL(format.fmt.pix.width, ???);
			//CU_ASSERT_EQUAL(format.fmt.pix.height, ???);
			//CU_ASSERT_EQUAL(format.fmt.pix.pixelformat, ???);
			CU_ASSERT(valid_pixelformat
				  (format.fmt.pix.pixelformat));

			//CU_ASSERT_EQUAL(format.fmt.pix.field, ???);
			//CU_ASSERT_EQUAL(format.fmt.pix.bytesperline, ???);
			//CU_ASSERT_EQUAL(format.fmt.pix.sizeimage, ???);
			//CU_ASSERT_EQUAL(format.fmt.pix.colorspace, ???);
			CU_ASSERT(valid_colorspace(format.fmt.pix.colorspace));
			//CU_ASSERT_EQUAL(format.fmt.pix.priv, ???);

			/* Check whether the remaining bytes of rawdata is set to zero */
			memset(&format2, 0, sizeof(format2));
			CU_ASSERT_EQUAL(memcmp
					(format.fmt.raw_data +
					 sizeof(format.fmt.pix),
					 format2.fmt.raw_data +
					 sizeof(format2.fmt.pix),
					 sizeof(format.fmt.raw_data) -
					 sizeof(format.fmt.pix)), 0);
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
				format.fmt.win.global_alpha);
			if (sizeof(format.fmt.win) <
			    sizeof(format.fmt.raw_data)) {
				dprintf1
				    ("\tformat = { ..., .fmt.raw_data[] = { ...");
				for (j = sizeof(format.fmt.win);
				     j < sizeof(format.fmt.raw_data); j++) {
					dprintf(", 0x%x",
						format.fmt.raw_data[j]);
				}
				dprintf1(" }}\n");
			}

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

			/* Check whether the remaining bytes of raw_data is set to zero */
			memset(&format2, 0, sizeof(format2));
			CU_ASSERT_EQUAL(memcmp
					(format.fmt.raw_data +
					 sizeof(format.fmt.win),
					 format2.fmt.raw_data +
					 sizeof(format2.fmt.win),
					 sizeof(format.fmt.raw_data) -
					 sizeof(format.fmt.win)), 0);
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
			if (sizeof(format.fmt.vbi) <
			    sizeof(format.fmt.raw_data)) {
				dprintf1
				    ("\tformat = { ..., .fmt.raw_data[] = { ...");
				for (j = sizeof(format.fmt.vbi);
				     j < sizeof(format.fmt.raw_data); j++) {
					dprintf(", 0x%x",
						format.fmt.raw_data[j]);
				}
				dprintf1(" }}\n");
			}

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

			/* Check whether the remaining bytes of raw_data is set to zero */
			memset(&format2, 0, sizeof(format2));
			CU_ASSERT_EQUAL(memcmp
					(format.fmt.raw_data +
					 sizeof(format.fmt.vbi),
					 format2.fmt.raw_data +
					 sizeof(format2.fmt.vbi),
					 sizeof(format.fmt.raw_data) -
					 sizeof(format.fmt.vbi)), 0);
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
				format.type, format.fmt.sliced.service_set,
				//format.fmt.sliced.service_lines[][],
				format.fmt.sliced.io_size,
				format.fmt.sliced.reserved[0],
				format.fmt.sliced.reserved[1]
			    );
			if (sizeof(format.fmt.sliced) <
			    sizeof(format.fmt.raw_data)) {
				dprintf1
				    ("\tformat = { ..., .fmt.raw_data[] = { ...");
				for (j = sizeof(format.fmt.sliced);
				     j < sizeof(format.fmt.raw_data); j++) {
					dprintf(", 0x%x",
						format.fmt.raw_data[j]);
				}
				dprintf1(" }}\n");
			}

			/* TODO: check different fields */
			//CU_ASSERT_EQUAL(format.fmt.sliced.service_set, ???);
			CU_ASSERT_EQUAL(format.fmt.sliced.service_lines[0][0],
					0);
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
			CU_ASSERT_EQUAL(format.fmt.sliced.service_lines[1][0],
					0);
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

			/* Check whether the remaining bytes of raw_data is set to zero */
			memset(&format2, 0, sizeof(format2));
			CU_ASSERT_EQUAL(memcmp
					(format.fmt.raw_data +
					 sizeof(format.fmt.sliced),
					 format2.fmt.raw_data +
					 sizeof(format2.fmt.sliced),
					 sizeof(format.fmt.raw_data) -
					 sizeof(format.fmt.sliced)), 0);
			break;

		case V4L2_BUF_TYPE_PRIVATE:
			dprintf("\tformat = {.type=0x%X, ... }\n", format.type);
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

void test_VIDIOC_G_FMT()
{
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

static void do_get_format_invalid(enum v4l2_buf_type type)
{
	int ret_get, errno_get;
	struct v4l2_format format;
	struct v4l2_format format2;

	memset(&format, 0xff, sizeof(format));
	format.type = type;
	ret_get = ioctl(get_video_fd(), VIDIOC_G_FMT, &format);
	errno_get = errno;

	dprintf("\t%s:%u: VIDIOC_G_FMT, type=%i, ret_get=%i, errno_get=%i\n",
		__FILE__, __LINE__, type, ret_get, errno_get);

	CU_ASSERT_EQUAL(ret_get, -1);
	CU_ASSERT_EQUAL(errno_get, EINVAL);

	/* Check whether the original format struct is untouched */
	memset(&format2, 0xff, sizeof(format2));
	format2.type = type;
	CU_ASSERT_EQUAL(memcmp(&format, &format2, sizeof(format)), 0);
}

void test_VIDIOC_G_FMT_invalid_type()
{
	int i;

	/* In this test case the .index is valid (0) and only the .type
	 * is invalid. The .type filed is an enum which is stored in an 'int'.
	 */

	/* test invalid .type=0 */
	do_get_format_invalid(0);

	/* test invalid .type=SINT_MIN */
	do_get_format_invalid(SINT_MIN);

	/* test invalid .type=-1 */
	do_get_format_invalid(-1);

	/* test invalid .type= 8..0x7F */
	for (i = V4L2_BUF_TYPE_VIDEO_OUTPUT_OVERLAY + 1;
	     i < V4L2_BUF_TYPE_PRIVATE; i++) {
		do_get_format_invalid(i);
	}

	/* .type = 0x80..0x7FFF FFFF is the private range */

	/* Assume that 0x7FFF FFFF is invalid in the private range.
	 * This might be a wrong assumption, but let's have a test case like
	 * this for now.
	 */
	do_get_format_invalid(SINT_MAX);

}

void test_VIDIOC_G_FMT_NULL()
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
	struct v4l2_format format;
	enum v4l2_buf_type type;

	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	memset(&format, 0xff, sizeof(format));
	format.type = type;
	ret_capture = ioctl(get_video_fd(), VIDIOC_G_FMT, &format);
	errno_capture = errno;

	dprintf
	    ("\t%s:%u: VIDIOC_G_FMT, type=%i, ret_capture=%i, errno_capture=%i\n",
	     __FILE__, __LINE__, type, ret_capture, errno_capture);

	type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
	memset(&format, 0xff, sizeof(format));
	format.type = type;
	ret_output = ioctl(get_video_fd(), VIDIOC_G_FMT, &format);
	errno_output = errno;

	dprintf
	    ("\t%s:%u: VIDIOC_G_FMT, type=%i, ret_output=%i, errno_output=%i\n",
	     __FILE__, __LINE__, type, ret_output, errno_output);

	type = V4L2_BUF_TYPE_VIDEO_OVERLAY;
	memset(&format, 0xff, sizeof(format));
	format.type = type;
	ret_video_overlay = ioctl(get_video_fd(), VIDIOC_G_FMT, &format);
	errno_video_overlay = errno;

	dprintf
	    ("\t%s:%u: VIDIOC_G_FMT, type=%i, ret_video_overlay=%i, errno_video_overlay=%i\n",
	     __FILE__, __LINE__, type, ret_video_overlay, errno_video_overlay);

	type = V4L2_BUF_TYPE_VBI_CAPTURE;
	memset(&format, 0xff, sizeof(format));
	format.type = type;
	ret_vbi_capture = ioctl(get_video_fd(), VIDIOC_G_FMT, &format);
	errno_vbi_capture = errno;

	dprintf
	    ("\t%s:%u: VIDIOC_G_FMT, type=%i, ret_vbi_capture=%i, errno_vbi_capture=%i\n",
	     __FILE__, __LINE__, type, ret_vbi_capture, errno_vbi_capture);

	type = V4L2_BUF_TYPE_VBI_OUTPUT;
	memset(&format, 0xff, sizeof(format));
	format.type = type;
	ret_vbi_output = ioctl(get_video_fd(), VIDIOC_G_FMT, &format);
	errno_vbi_output = errno;

	dprintf
	    ("\t%s:%u: VIDIOC_G_FMT, type=%i, ret_vbi_output=%i, errno_vbi_output=%i\n",
	     __FILE__, __LINE__, type, ret_vbi_output, errno_vbi_output);

	type = V4L2_BUF_TYPE_SLICED_VBI_CAPTURE;
	memset(&format, 0xff, sizeof(format));
	format.type = type;
	ret_sliced_vbi_capture = ioctl(get_video_fd(), VIDIOC_G_FMT, &format);
	errno_sliced_vbi_capture = errno;

	dprintf
	    ("\t%s:%u: VIDIOC_G_FMT, type=%i, ret_sliced_vbi_capture=%i, errno_sliced_vbi_capture=%i\n",
	     __FILE__, __LINE__, type, ret_sliced_vbi_capture,
	     errno_sliced_vbi_capture);

	type = V4L2_BUF_TYPE_SLICED_VBI_OUTPUT;
	memset(&format, 0xff, sizeof(format));
	format.type = type;
	ret_sliced_vbi_output = ioctl(get_video_fd(), VIDIOC_G_FMT, &format);
	errno_sliced_vbi_output = errno;

	dprintf
	    ("\t%s:%u: VIDIOC_G_FMT, type=%i, ret_sliced_vbi_output=%i, errno_sliced_vbi_output=%i\n",
	     __FILE__, __LINE__, type, ret_sliced_vbi_output,
	     errno_sliced_vbi_output);

	type = V4L2_BUF_TYPE_VIDEO_OUTPUT_OVERLAY;
	memset(&format, 0xff, sizeof(format));
	format.type = type;
	ret_video_output_overlay = ioctl(get_video_fd(), VIDIOC_G_FMT, &format);
	errno_video_output_overlay = errno;

	dprintf
	    ("\t%s:%u: VIDIOC_G_FMT, type=%i, ret_video_output_overlay=%i, errno_video_output_overlay=%i\n",
	     __FILE__, __LINE__, type, ret_video_output_overlay,
	     errno_video_output_overlay);

	type = V4L2_BUF_TYPE_PRIVATE;
	memset(&format, 0xff, sizeof(format));
	format.type = type;
	ret_private = ioctl(get_video_fd(), VIDIOC_G_FMT, &format);
	errno_private = errno;

	dprintf
	    ("\t%s:%u: VIDIOC_G_FMT, type=%i, ret_private=%i, errno_private=%i\n",
	     __FILE__, __LINE__, type, ret_private, errno_private);

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

static void do_set_formats_enum(enum v4l2_buf_type type)
{
	int ret_get, errno_get;
	int ret_enum, errno_enum;
	int ret_max, errno_max;
	int ret_min, errno_min;
	int ret_set, errno_set;
	struct v4l2_format format_orig;
	struct v4l2_format format_min;
	struct v4l2_format format_max;
	struct v4l2_format format_set;
	struct v4l2_format format2;
	struct v4l2_fmtdesc fmtdesc;
	__u32 i;
	unsigned int j;

	memset(&format_orig, 0xff, sizeof(format_orig));
	format_orig.type = type;

	ret_get = ioctl(get_video_fd(), VIDIOC_G_FMT, &format_orig);
	errno_get = errno;

	dprintf("\t%s:%u: VIDIOC_G_FMT, type=%i, ret_get=%i, errno_get=%i\n",
		__FILE__, __LINE__, type, ret_get, errno_get);

	i = 0;
	do {
		memset(&fmtdesc, 0, sizeof(fmtdesc));
		fmtdesc.index = i;
		fmtdesc.type = type;

		ret_enum = ioctl(get_video_fd(), VIDIOC_ENUM_FMT, &fmtdesc);
		errno_enum = errno;

		dprintf
		    ("\t%s:%u: VIDIOC_ENUM_FMT, index=%u, type=%i, ret_enum=%i, errno_enum=%i\n",
		     __FILE__, __LINE__, i, type, ret_enum, errno_enum);

		switch (type) {
		case V4L2_BUF_TYPE_VIDEO_CAPTURE:
		case V4L2_BUF_TYPE_VIDEO_OUTPUT:
			memset(&format_max, 0xff, sizeof(format_max));
			format_max.type = type;
			format_max.fmt.pix.pixelformat = fmtdesc.pixelformat;
			format_max.fmt.pix.field = V4L2_FIELD_ANY;

			ret_max =
			    ioctl(get_video_fd(), VIDIOC_S_FMT, &format_max);
			errno_max = errno;

			dprintf
			    ("\t%s:%u: VIDIOC_S_FMT, type=%i, ret_max=%i, errno_max=%i\n",
			     __FILE__, __LINE__, type, ret_max, errno_max);

			if (ret_max == 0) {
				CU_ASSERT_EQUAL(ret_enum, 0);

				dprintf
				    ("\tformat_max = {.type=0x%X, .fmt.pix = { "
				     ".width=%u, " ".height=%u, "
				     ".pixelformat=0x%X, " ".field=%i, "
				     ".bytesperline=%i, " ".sizeimage=%u, "
				     ".colorspace=%i, " ".priv=0x%X " "} }\n",
				     format_max.type, format_max.fmt.pix.width,
				     format_max.fmt.pix.height,
				     format_max.fmt.pix.pixelformat,
				     format_max.fmt.pix.field,
				     format_max.fmt.pix.bytesperline,
				     format_max.fmt.pix.sizeimage,
				     format_max.fmt.pix.colorspace,
				     format_max.fmt.pix.priv);
				if (sizeof(format_max.fmt.pix) <
				    sizeof(format_max.fmt.raw_data)) {
					dprintf1
					    ("\tformat_max = { ..., .fmt.raw_data[] = { ...");
					for (j = sizeof(format_max.fmt.pix);
					     j <
					     sizeof(format_max.fmt.raw_data);
					     j++) {
						dprintf(", 0x%x",
							format_max.fmt.
							raw_data[j]);
					}
					dprintf1(" }}\n");
				}

				CU_ASSERT_EQUAL(ret_max, 0);
				CU_ASSERT(valid_pixelformat
					  (format_max.fmt.pix.pixelformat));
				CU_ASSERT_EQUAL(format_max.fmt.pix.pixelformat,
						fmtdesc.pixelformat);
				CU_ASSERT(0 < format_max.fmt.pix.width);
				CU_ASSERT(0 < format_max.fmt.pix.height);
				CU_ASSERT_NOT_EQUAL(format_max.fmt.pix.field,
						    V4L2_FIELD_ANY);
				CU_ASSERT(0 < format_max.fmt.pix.bytesperline);
				CU_ASSERT(0 < format_max.fmt.pix.sizeimage);
				CU_ASSERT(valid_colorspace
					  (format_max.fmt.pix.colorspace));
				//CU_ASSERT_EQUAL(format_max.fmt.pix.priv, ???);

				/* Check whether the remaining bytes of rawdata is set to zero */
				memset(&format2, 0, sizeof(format2));
				CU_ASSERT_EQUAL(memcmp
						(format_max.fmt.raw_data +
						 sizeof(format_max.fmt.pix),
						 format2.fmt.raw_data +
						 sizeof(format2.fmt.pix),
						 sizeof(format_max.fmt.
							raw_data) -
						 sizeof(format_max.fmt.pix)),
						0);

			} else {
				CU_ASSERT_EQUAL(ret_max, -1);
				CU_ASSERT_EQUAL(errno_max, EINVAL);
			}

			memset(&format_min, 0, sizeof(format_min));
			format_min.type = type;
			format_min.fmt.pix.pixelformat = fmtdesc.pixelformat;
			format_min.fmt.pix.field = V4L2_FIELD_ANY;

			ret_min =
			    ioctl(get_video_fd(), VIDIOC_S_FMT, &format_min);
			errno_min = errno;

			dprintf
			    ("\t%s:%u: VIDIOC_S_FMT, type=%i, ret_min=%i, errno_min=%i\n",
			     __FILE__, __LINE__, type, ret_min, errno_min);

			if (ret_min == 0) {
				CU_ASSERT_EQUAL(ret_enum, 0);

				dprintf
				    ("\tformat_min = {.type=0x%X, .fmt.pix = { "
				     ".width=%u, " ".height=%u, "
				     ".pixelformat=0x%X, " ".field=%i, "
				     ".bytesperline=%i, " ".sizeimage=%u, "
				     ".colorspace=%i, " ".priv=0x%X " "} }\n",
				     format_min.type, format_min.fmt.pix.width,
				     format_min.fmt.pix.height,
				     format_min.fmt.pix.pixelformat,
				     format_min.fmt.pix.field,
				     format_min.fmt.pix.bytesperline,
				     format_min.fmt.pix.sizeimage,
				     format_min.fmt.pix.colorspace,
				     format_min.fmt.pix.priv);
				if (sizeof(format_min.fmt.pix) <
				    sizeof(format_min.fmt.raw_data)) {
					dprintf1
					    ("\tformat_min = { ..., .fmt.raw_data[] = { ...");
					for (j = sizeof(format_min.fmt.pix);
					     j <
					     sizeof(format_min.fmt.raw_data);
					     j++) {
						dprintf(", 0x%x",
							format_min.fmt.
							raw_data[j]);
					}
					dprintf1(" }}\n");
				}

				CU_ASSERT_EQUAL(ret_min, 0);
				CU_ASSERT(valid_pixelformat
					  (format_min.fmt.pix.pixelformat));
				CU_ASSERT_EQUAL(format_min.fmt.pix.pixelformat,
						fmtdesc.pixelformat);
				CU_ASSERT(0 < format_min.fmt.pix.width);
				CU_ASSERT(0 < format_min.fmt.pix.height);
				CU_ASSERT_NOT_EQUAL(format_min.fmt.pix.field,
						    V4L2_FIELD_ANY);
				CU_ASSERT(0 < format_min.fmt.pix.bytesperline);
				CU_ASSERT(0 < format_min.fmt.pix.sizeimage);
				CU_ASSERT(valid_colorspace
					  (format_min.fmt.pix.colorspace));
				//CU_ASSERT_EQUAL(format_min.fmt.pix.priv, ???);

				/* Check whether the remaining bytes of rawdata is set to zero */
				memset(&format2, 0, sizeof(format2));
				CU_ASSERT_EQUAL(memcmp
						(format_min.fmt.raw_data +
						 sizeof(format_min.fmt.pix),
						 format2.fmt.raw_data +
						 sizeof(format2.fmt.pix),
						 sizeof(format_min.fmt.
							raw_data) -
						 sizeof(format_min.fmt.pix)),
						0);
			} else {
				CU_ASSERT_EQUAL(ret_min, -1);
				CU_ASSERT_EQUAL(errno_min, EINVAL);
			}

			if (ret_max == 0 && ret_min == 0) {
				CU_ASSERT(format_min.fmt.pix.width <=
					  format_max.fmt.pix.width);
				CU_ASSERT(format_min.fmt.pix.height <=
					  format_max.fmt.pix.height);
				CU_ASSERT_EQUAL(format_min.fmt.pix.colorspace,
						format_max.fmt.pix.colorspace);

				/* If priv equals zero then this field is not used and shall
				 * be set to zero each case. Otherwise it can have any driver
				 * specific value which cannot be checked here.
				 */
				if (format_min.fmt.pix.priv == 0) {
					CU_ASSERT_EQUAL(format_min.fmt.pix.priv,
							0);
					CU_ASSERT_EQUAL(format_max.fmt.pix.priv,
							0);
				}
			}

			if (ret_max == -1 && ret_min == -1) {
				CU_ASSERT_EQUAL(ret_enum, -1);
				CU_ASSERT_EQUAL(errno_enum, EINVAL);
			}

			break;

		case V4L2_BUF_TYPE_VIDEO_OVERLAY:
		case V4L2_BUF_TYPE_VIDEO_OUTPUT_OVERLAY:
			memset(&format_max, 0xff, sizeof(format_max));
			format_max.type = type;

			ret_max =
			    ioctl(get_video_fd(), VIDIOC_S_FMT, &format_max);
			errno_max = errno;

			dprintf
			    ("\t%s:%u: VIDIOC_S_FMT, type=%i, ret_max=%i, errno_max=%i\n",
			     __FILE__, __LINE__, type, ret_max, errno_max);

			if (ret_max == 0) {
				CU_ASSERT_EQUAL(ret_enum, 0);

				dprintf
				    ("\tformat_max = {.type=0x%X, .fmt.win={ "
				     ".w = { .left=%i, .top=%i, .width=%i, .height=%i, }, "
				     ".field = %i, " ".chromakey = 0x%X, "
				     ".clips = %p, " ".clipcount = %u, "
				     ".bitmap = %p, " ".global_alpha = %u "
				     "} }\n", format_max.type,
				     format_max.fmt.win.w.left,
				     format_max.fmt.win.w.top,
				     format_max.fmt.win.w.width,
				     format_max.fmt.win.w.height,
				     format_max.fmt.win.field,
				     format_max.fmt.win.chromakey,
				     format_max.fmt.win.clips,
				     format_max.fmt.win.clipcount,
				     format_max.fmt.win.bitmap,
				     format_max.fmt.win.global_alpha);
				if (sizeof(format_max.fmt.win) <
				    sizeof(format_max.fmt.raw_data)) {
					dprintf1
					    ("\tformat_max = { ..., .fmt.raw_data[] = { ...");
					for (j = sizeof(format_max.fmt.win);
					     j <
					     sizeof(format_max.fmt.raw_data);
					     j++) {
						dprintf(", 0x%x",
							format_max.fmt.
							raw_data[j]);
					}
					dprintf1(" }}\n");
				}

				/* TODO: check the different fields */
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
			} else {
				CU_ASSERT_EQUAL(ret_max, -1);
				CU_ASSERT_EQUAL(errno_max, EINVAL);
			}

			memset(&format_min, 0, sizeof(format_min));
			format_min.type = type;
			format_min.fmt.pix.pixelformat = fmtdesc.pixelformat;

			ret_min =
			    ioctl(get_video_fd(), VIDIOC_S_FMT, &format_min);
			errno_min = errno;

			dprintf
			    ("\t%s:%u: VIDIOC_S_FMT, type=%i, ret_min=%i, errno_min=%i\n",
			     __FILE__, __LINE__, type, ret_min, errno_min);

			if (ret_min == 0) {
				CU_ASSERT_EQUAL(ret_enum, 0);
				dprintf
				    ("\tformat_min = {.type=0x%X, .fmt.win={ "
				     ".w = { .left=%i, .top=%i, .width=%i, .height=%i, }, "
				     ".field = %i, " ".chromakey = 0x%X, "
				     ".clips = %p, " ".clipcount = %u, "
				     ".bitmap = %p, " ".global_alpha = %u "
				     "} }\n", format_min.type,
				     format_min.fmt.win.w.left,
				     format_min.fmt.win.w.top,
				     format_min.fmt.win.w.width,
				     format_min.fmt.win.w.height,
				     format_min.fmt.win.field,
				     format_min.fmt.win.chromakey,
				     format_min.fmt.win.clips,
				     format_min.fmt.win.clipcount,
				     format_min.fmt.win.bitmap,
				     format_min.fmt.win.global_alpha);
				if (sizeof(format_min.fmt.win) <
				    sizeof(format_min.fmt.raw_data)) {
					dprintf1
					    ("\tformat_min = { ..., .fmt.raw_data[] = { ...");
					for (j = sizeof(format_min.fmt.win);
					     j <
					     sizeof(format_min.fmt.raw_data);
					     j++) {
						dprintf(", 0x%x",
							format_min.fmt.
							raw_data[j]);
					}
					dprintf1(" }}\n");
				}

				/* TODO: check the different fields */
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

			} else {
				CU_ASSERT_EQUAL(ret_min, -1);
				CU_ASSERT_EQUAL(errno_min, EINVAL);
			}

			if (ret_max == -1 && ret_min == -1) {
				CU_ASSERT_EQUAL(ret_enum, -1);
				CU_ASSERT_EQUAL(errno_enum, EINVAL);
			}
			break;

		case V4L2_BUF_TYPE_VBI_CAPTURE:
		case V4L2_BUF_TYPE_VBI_OUTPUT:
			memset(&format_max, 0xff, sizeof(format_max));
			format_max.type = type;

			ret_max =
			    ioctl(get_video_fd(), VIDIOC_S_FMT, &format_max);
			errno_max = errno;

			dprintf
			    ("\t%s:%u: VIDIOC_S_FMT, type=%i, ret_max=%i, errno_max=%i\n",
			     __FILE__, __LINE__, type, ret_max, errno_max);

			if (ret_max == 0) {
				CU_ASSERT_EQUAL(ret_enum, 0);
				dprintf
				    ("\tformat_max = {.type=0x%X, .fmt.vbi={ "
				     ".sampling_rate=%u, " ".offset=%u, "
				     ".samples_per_line=%u "
				     ".sample_format=0x%X "
				     ".start = { %u, %u }, "
				     ".count = { %u, %u }, " ".flags = 0x%X, "
				     ".reserved = { 0x%X, 0x%X } " "} }\n",
				     format_max.type,
				     format_max.fmt.vbi.sampling_rate,
				     format_max.fmt.vbi.offset,
				     format_max.fmt.vbi.samples_per_line,
				     format_max.fmt.vbi.sample_format,
				     format_max.fmt.vbi.start[0],
				     format_max.fmt.vbi.start[1],
				     format_max.fmt.vbi.count[0],
				     format_max.fmt.vbi.count[1],
				     format_max.fmt.vbi.flags,
				     format_max.fmt.vbi.reserved[0],
				     format_max.fmt.vbi.reserved[1]
				    );
				if (sizeof(format_max.fmt.vbi) <
				    sizeof(format_max.fmt.raw_data)) {
					dprintf1
					    ("\tformat_max = { ..., .fmt.raw_data[] = { ...");
					for (j = sizeof(format_max.fmt.vbi);
					     j <
					     sizeof(format_max.fmt.raw_data);
					     j++) {
						dprintf(", 0x%x",
							format_max.fmt.
							raw_data[j]);
					}
					dprintf1(" }}\n");
				}

				/* TODO: check the different fields */
				//CU_ASSERT_EQUAL(format_max.fmt.vbi.sampling_rate, ???);
				//CU_ASSERT_EQUAL(format_max.fmt.vbi.offset, ???);
				//CU_ASSERT_EQUAL(format_max.fmt.vbi.samples_per_line, ???);
				//CU_ASSERT_EQUAL(format_max.fmt.vbi.sample_format, ???);
				//CU_ASSERT_EQUAL(format_max.fmt.vbi.start[0], ???);
				//CU_ASSERT_EQUAL(format_max.fmt.vbi.start[1], ???);
				//CU_ASSERT_EQUAL(format_max.fmt.vbi.count[0], ???);
				//CU_ASSERT_EQUAL(format_max.fmt.vbi.count[1], ???);
				//CU_ASSERT_EQUAL(format_max.fmt.vbi.flags, ???);
				CU_ASSERT_EQUAL(format_max.fmt.vbi.reserved[0],
						0);
				CU_ASSERT_EQUAL(format_max.fmt.vbi.reserved[1],
						0);

			} else {
				CU_ASSERT_EQUAL(ret_max, -1);
				CU_ASSERT_EQUAL(errno_max, EINVAL);
			}

			memset(&format_min, 0, sizeof(format_min));
			format_min.type = type;
			format_min.fmt.pix.pixelformat = fmtdesc.pixelformat;

			ret_min =
			    ioctl(get_video_fd(), VIDIOC_S_FMT, &format_min);
			errno_min = errno;

			dprintf
			    ("\t%s:%u: VIDIOC_S_FMT, type=%i, ret_min=%i, errno_min=%i\n",
			     __FILE__, __LINE__, type, ret_min, errno_min);

			if (ret_min == 0) {
				CU_ASSERT_EQUAL(ret_enum, 0);
				dprintf
				    ("\tformat_min = {.type=0x%X, .fmt.vbi={ "
				     ".sampling_rate=%u, " ".offset=%u, "
				     ".samples_per_line=%u "
				     ".sample_format=0x%X "
				     ".start = { %u, %u }, "
				     ".count = { %u, %u }, " ".flags = 0x%X, "
				     ".reserved = { 0x%X, 0x%X } " "} }\n",
				     format_min.type,
				     format_min.fmt.vbi.sampling_rate,
				     format_min.fmt.vbi.offset,
				     format_min.fmt.vbi.samples_per_line,
				     format_min.fmt.vbi.sample_format,
				     format_min.fmt.vbi.start[0],
				     format_min.fmt.vbi.start[1],
				     format_min.fmt.vbi.count[0],
				     format_min.fmt.vbi.count[1],
				     format_min.fmt.vbi.flags,
				     format_min.fmt.vbi.reserved[0],
				     format_min.fmt.vbi.reserved[1]
				    );
				if (sizeof(format_min.fmt.vbi) <
				    sizeof(format_min.fmt.raw_data)) {
					dprintf1
					    ("\tformat_min = { ..., .fmt.raw_data[] = { ...");
					for (j = sizeof(format_min.fmt.vbi);
					     j <
					     sizeof(format_min.fmt.raw_data);
					     j++) {
						dprintf(", 0x%x",
							format_min.fmt.
							raw_data[j]);
					}
					dprintf1(" }}\n");
				}

				/* TODO: check the different fields */
				//CU_ASSERT_EQUAL(format_min.fmt.vbi.sampling_rate, ???);
				//CU_ASSERT_EQUAL(format_min.fmt.vbi.offset, ???);
				//CU_ASSERT_EQUAL(format_min.fmt.vbi.samples_per_line, ???);
				//CU_ASSERT_EQUAL(format_min.fmt.vbi.sample_format, ???);
				//CU_ASSERT_EQUAL(format_min.fmt.vbi.start[0], ???);
				//CU_ASSERT_EQUAL(format_min.fmt.vbi.start[1], ???);
				//CU_ASSERT_EQUAL(format_min.fmt.vbi.count[0], ???);
				//CU_ASSERT_EQUAL(format_min.fmt.vbi.count[1], ???);
				//CU_ASSERT_EQUAL(format_min.fmt.vbi.flags, ???);
				CU_ASSERT_EQUAL(format_min.fmt.vbi.reserved[0],
						0);
				CU_ASSERT_EQUAL(format_min.fmt.vbi.reserved[1],
						0);
			} else {
				CU_ASSERT_EQUAL(ret_min, -1);
				CU_ASSERT_EQUAL(errno_min, EINVAL);
			}

			if (ret_max == -1 && ret_min == -1) {
				CU_ASSERT_EQUAL(ret_enum, -1);
				CU_ASSERT_EQUAL(errno_enum, EINVAL);
			}
			break;

		case V4L2_BUF_TYPE_SLICED_VBI_CAPTURE:
		case V4L2_BUF_TYPE_SLICED_VBI_OUTPUT:
			memset(&format_max, 0xff, sizeof(format_max));
			format_max.type = type;

			ret_max =
			    ioctl(get_video_fd(), VIDIOC_S_FMT, &format_max);
			errno_max = errno;

			dprintf
			    ("\t%s:%u: VIDIOC_S_FMT, type=%i, ret_max=%i, errno_max=%i\n",
			     __FILE__, __LINE__, type, ret_max, errno_max);

			if (ret_max == 0) {
				CU_ASSERT_EQUAL(ret_enum, 0);
				dprintf("\tformat_max = {.type=0x%X, "
					".fmt.sliced = { .service_set = 0x%X, "
					".service_lines = { ... }, "
					".io_size = %u, "
					".reserved[0] = 0x%X, "
					".reserved[1] = 0x%X "
					"} }\n",
					format_max.type,
					format_max.fmt.sliced.service_set,
					//format_max.fmt.sliced.service_lines[][],
					format_max.fmt.sliced.io_size,
					format_max.fmt.sliced.reserved[0],
					format_max.fmt.sliced.reserved[1]
				    );
				if (sizeof(format_max.fmt.sliced) <
				    sizeof(format_max.fmt.raw_data)) {
					dprintf1
					    ("\tformat_max = { ..., .fmt.raw_data[] = { ...");
					for (j = sizeof(format_max.fmt.sliced);
					     j <
					     sizeof(format_max.fmt.raw_data);
					     j++) {
						dprintf(", 0x%x",
							format_max.fmt.
							raw_data[j]);
					}
					dprintf1(" }}\n");
				}

				/* TODO: check the different fields */
				//CU_ASSERT_EQUAL(format_max.fmt.sliced.service_set, ???);
				CU_ASSERT_EQUAL(format_max.fmt.sliced.
						service_lines[0][0], 0);
				//CU_ASSERT_EQUAL(format_max.fmt.sliced.service_lines[0][1], ???);
				//CU_ASSERT_EQUAL(format_max.fmt.sliced.service_lines[0][2], ???);
				//CU_ASSERT_EQUAL(format_max.fmt.sliced.service_lines[0][3], ???);
				//CU_ASSERT_EQUAL(format_max.fmt.sliced.service_lines[0][4], ???);
				//CU_ASSERT_EQUAL(format_max.fmt.sliced.service_lines[0][5], ???);
				//CU_ASSERT_EQUAL(format_max.fmt.sliced.service_lines[0][6], ???);
				//CU_ASSERT_EQUAL(format_max.fmt.sliced.service_lines[0][7], ???);
				//CU_ASSERT_EQUAL(format_max.fmt.sliced.service_lines[0][8], ???);
				//CU_ASSERT_EQUAL(format_max.fmt.sliced.service_lines[0][9], ???);
				//CU_ASSERT_EQUAL(format_max.fmt.sliced.service_lines[0][10], ???);
				//CU_ASSERT_EQUAL(format_max.fmt.sliced.service_lines[0][11], ???);
				//CU_ASSERT_EQUAL(format_max.fmt.sliced.service_lines[0][12], ???);
				//CU_ASSERT_EQUAL(format_max.fmt.sliced.service_lines[0][13], ???);
				//CU_ASSERT_EQUAL(format_max.fmt.sliced.service_lines[0][14], ???);
				//CU_ASSERT_EQUAL(format_max.fmt.sliced.service_lines[0][15], ???);
				//CU_ASSERT_EQUAL(format_max.fmt.sliced.service_lines[0][16], ???);
				//CU_ASSERT_EQUAL(format_max.fmt.sliced.service_lines[0][17], ???);
				//CU_ASSERT_EQUAL(format_max.fmt.sliced.service_lines[0][18], ???);
				//CU_ASSERT_EQUAL(format_max.fmt.sliced.service_lines[0][19], ???);
				//CU_ASSERT_EQUAL(format_max.fmt.sliced.service_lines[0][20], ???);
				//CU_ASSERT_EQUAL(format_max.fmt.sliced.service_lines[0][21], ???);
				//CU_ASSERT_EQUAL(format_max.fmt.sliced.service_lines[0][22], ???);
				//CU_ASSERT_EQUAL(format_max.fmt.sliced.service_lines[0][23], ???);
				CU_ASSERT_EQUAL(format_max.fmt.sliced.
						service_lines[1][0], 0);
				//CU_ASSERT_EQUAL(format_max.fmt.sliced.service_lines[1][1], ???);
				//CU_ASSERT_EQUAL(format_max.fmt.sliced.service_lines[1][2], ???);
				//CU_ASSERT_EQUAL(format_max.fmt.sliced.service_lines[1][3], ???);
				//CU_ASSERT_EQUAL(format_max.fmt.sliced.service_lines[1][4], ???);
				//CU_ASSERT_EQUAL(format_max.fmt.sliced.service_lines[1][5], ???);
				//CU_ASSERT_EQUAL(format_max.fmt.sliced.service_lines[1][6], ???);
				//CU_ASSERT_EQUAL(format_max.fmt.sliced.service_lines[1][7], ???);
				//CU_ASSERT_EQUAL(format_max.fmt.sliced.service_lines[1][8], ???);
				//CU_ASSERT_EQUAL(format_max.fmt.sliced.service_lines[1][9], ???);
				//CU_ASSERT_EQUAL(format_max.fmt.sliced.service_lines[1][10], ???);
				//CU_ASSERT_EQUAL(format_max.fmt.sliced.service_lines[1][11], ???);
				//CU_ASSERT_EQUAL(format_max.fmt.sliced.service_lines[1][12], ???);
				//CU_ASSERT_EQUAL(format_max.fmt.sliced.service_lines[1][13], ???);
				//CU_ASSERT_EQUAL(format_max.fmt.sliced.service_lines[1][14], ???);
				//CU_ASSERT_EQUAL(format_max.fmt.sliced.service_lines[1][15], ???);
				//CU_ASSERT_EQUAL(format_max.fmt.sliced.service_lines[1][16], ???);
				//CU_ASSERT_EQUAL(format_max.fmt.sliced.service_lines[1][17], ???);
				//CU_ASSERT_EQUAL(format_max.fmt.sliced.service_lines[1][18], ???);
				//CU_ASSERT_EQUAL(format_max.fmt.sliced.service_lines[1][19], ???);
				//CU_ASSERT_EQUAL(format_max.fmt.sliced.service_lines[1][20], ???);
				//CU_ASSERT_EQUAL(format_max.fmt.sliced.service_lines[1][21], ???);
				//CU_ASSERT_EQUAL(format_max.fmt.sliced.service_lines[1][22], ???);
				//CU_ASSERT_EQUAL(format_max.fmt.sliced.service_lines[1][23], ???);
				//CU_ASSERT_EQUAL(format_max.fmt.sliced.io_size, ???);
				CU_ASSERT_EQUAL(format_max.fmt.sliced.
						reserved[0], 0);
				CU_ASSERT_EQUAL(format_max.fmt.sliced.
						reserved[1], 0);

			} else {
				CU_ASSERT_EQUAL(ret_max, -1);
				CU_ASSERT_EQUAL(errno_max, EINVAL);
			}

			memset(&format_min, 0, sizeof(format_min));
			format_min.type = type;
			format_min.fmt.pix.pixelformat = fmtdesc.pixelformat;

			ret_min =
			    ioctl(get_video_fd(), VIDIOC_S_FMT, &format_min);
			errno_min = errno;

			dprintf
			    ("\t%s:%u: VIDIOC_S_FMT, type=%i, ret_min=%i, errno_min=%i\n",
			     __FILE__, __LINE__, type, ret_min, errno_min);

			if (ret_min == 0) {
				CU_ASSERT_EQUAL(ret_enum, 0);
				dprintf("\tformat_min = {.type=0x%X, "
					".fmt.sliced = { .service_set = 0x%X, "
					".service_lines = { ... }, "
					".io_size = %u, "
					".reserved[0] = 0x%X, "
					".reserved[1] = 0x%X "
					"} }\n",
					format_min.type,
					format_min.fmt.sliced.service_set,
					//format_min.fmt.sliced.service_lines[][],
					format_min.fmt.sliced.io_size,
					format_min.fmt.sliced.reserved[0],
					format_min.fmt.sliced.reserved[1]
				    );
				if (sizeof(format_min.fmt.sliced) <
				    sizeof(format_min.fmt.raw_data)) {
					dprintf1
					    ("\tformat_min = { ..., .fmt.raw_data[] = { ...");
					for (j = sizeof(format_min.fmt.sliced);
					     j <
					     sizeof(format_min.fmt.raw_data);
					     j++) {
						dprintf(", 0x%x",
							format_min.fmt.
							raw_data[j]);
					}
					dprintf1(" }}\n");
				}

				/* TODO: check the different fields */
				//CU_ASSERT_EQUAL(format_min.fmt.sliced.service_set, ???);
				CU_ASSERT_EQUAL(format_min.fmt.sliced.
						service_lines[0][0], 0);
				//CU_ASSERT_EQUAL(format_min.fmt.sliced.service_lines[0][1], ???);
				//CU_ASSERT_EQUAL(format_min.fmt.sliced.service_lines[0][2], ???);
				//CU_ASSERT_EQUAL(format_min.fmt.sliced.service_lines[0][3], ???);
				//CU_ASSERT_EQUAL(format_min.fmt.sliced.service_lines[0][4], ???);
				//CU_ASSERT_EQUAL(format_min.fmt.sliced.service_lines[0][5], ???);
				//CU_ASSERT_EQUAL(format_min.fmt.sliced.service_lines[0][6], ???);
				//CU_ASSERT_EQUAL(format_min.fmt.sliced.service_lines[0][7], ???);
				//CU_ASSERT_EQUAL(format_min.fmt.sliced.service_lines[0][8], ???);
				//CU_ASSERT_EQUAL(format_min.fmt.sliced.service_lines[0][9], ???);
				//CU_ASSERT_EQUAL(format_min.fmt.sliced.service_lines[0][10], ???);
				//CU_ASSERT_EQUAL(format_min.fmt.sliced.service_lines[0][11], ???);
				//CU_ASSERT_EQUAL(format_min.fmt.sliced.service_lines[0][12], ???);
				//CU_ASSERT_EQUAL(format_min.fmt.sliced.service_lines[0][13], ???);
				//CU_ASSERT_EQUAL(format_min.fmt.sliced.service_lines[0][14], ???);
				//CU_ASSERT_EQUAL(format_min.fmt.sliced.service_lines[0][15], ???);
				//CU_ASSERT_EQUAL(format_min.fmt.sliced.service_lines[0][16], ???);
				//CU_ASSERT_EQUAL(format_min.fmt.sliced.service_lines[0][17], ???);
				//CU_ASSERT_EQUAL(format_min.fmt.sliced.service_lines[0][18], ???);
				//CU_ASSERT_EQUAL(format_min.fmt.sliced.service_lines[0][19], ???);
				//CU_ASSERT_EQUAL(format_min.fmt.sliced.service_lines[0][20], ???);
				//CU_ASSERT_EQUAL(format_min.fmt.sliced.service_lines[0][21], ???);
				//CU_ASSERT_EQUAL(format_min.fmt.sliced.service_lines[0][22], ???);
				//CU_ASSERT_EQUAL(format_min.fmt.sliced.service_lines[0][23], ???);
				CU_ASSERT_EQUAL(format_min.fmt.sliced.
						service_lines[1][0], 0);
				//CU_ASSERT_EQUAL(format_min.fmt.sliced.service_lines[1][1], ???);
				//CU_ASSERT_EQUAL(format_min.fmt.sliced.service_lines[1][2], ???);
				//CU_ASSERT_EQUAL(format_min.fmt.sliced.service_lines[1][3], ???);
				//CU_ASSERT_EQUAL(format_min.fmt.sliced.service_lines[1][4], ???);
				//CU_ASSERT_EQUAL(format_min.fmt.sliced.service_lines[1][5], ???);
				//CU_ASSERT_EQUAL(format_min.fmt.sliced.service_lines[1][6], ???);
				//CU_ASSERT_EQUAL(format_min.fmt.sliced.service_lines[1][7], ???);
				//CU_ASSERT_EQUAL(format_min.fmt.sliced.service_lines[1][8], ???);
				//CU_ASSERT_EQUAL(format_min.fmt.sliced.service_lines[1][9], ???);
				//CU_ASSERT_EQUAL(format_min.fmt.sliced.service_lines[1][10], ???);
				//CU_ASSERT_EQUAL(format_min.fmt.sliced.service_lines[1][11], ???);
				//CU_ASSERT_EQUAL(format_min.fmt.sliced.service_lines[1][12], ???);
				//CU_ASSERT_EQUAL(format_min.fmt.sliced.service_lines[1][13], ???);
				//CU_ASSERT_EQUAL(format_min.fmt.sliced.service_lines[1][14], ???);
				//CU_ASSERT_EQUAL(format_min.fmt.sliced.service_lines[1][15], ???);
				//CU_ASSERT_EQUAL(format_min.fmt.sliced.service_lines[1][16], ???);
				//CU_ASSERT_EQUAL(format_min.fmt.sliced.service_lines[1][17], ???);
				//CU_ASSERT_EQUAL(format_min.fmt.sliced.service_lines[1][18], ???);
				//CU_ASSERT_EQUAL(format_min.fmt.sliced.service_lines[1][19], ???);
				//CU_ASSERT_EQUAL(format_min.fmt.sliced.service_lines[1][20], ???);
				//CU_ASSERT_EQUAL(format_min.fmt.sliced.service_lines[1][21], ???);
				//CU_ASSERT_EQUAL(format_min.fmt.sliced.service_lines[1][22], ???);
				//CU_ASSERT_EQUAL(format_min.fmt.sliced.service_lines[1][23], ???);
				//CU_ASSERT_EQUAL(format_min.fmt.sliced.io_size, ???);
				CU_ASSERT_EQUAL(format_min.fmt.sliced.
						reserved[0], 0);
				CU_ASSERT_EQUAL(format_min.fmt.sliced.
						reserved[1], 0);

			} else {
				CU_ASSERT_EQUAL(ret_min, -1);
				CU_ASSERT_EQUAL(errno_min, EINVAL);
			}

			if (ret_max == -1 && ret_min == -1) {
				CU_ASSERT_EQUAL(ret_enum, -1);
				CU_ASSERT_EQUAL(errno_enum, EINVAL);
			}
			break;

		case V4L2_BUF_TYPE_PRIVATE:
			memset(&format_max, 0xff, sizeof(format_max));
			format_max.type = type;

			ret_max =
			    ioctl(get_video_fd(), VIDIOC_S_FMT, &format_max);
			errno_max = errno;

			dprintf
			    ("\t%s:%u: VIDIOC_S_FMT, type=%i, ret_max=%i, errno_max=%i\n",
			     __FILE__, __LINE__, type, ret_max, errno_max);

			if (ret_max == 0) {
				CU_ASSERT_EQUAL(ret_enum, 0);
				dprintf1
				    ("\tformat_max = { ..., .fmt.raw_data[] = { ");
				for (j = 0; j < sizeof(format_max.fmt.raw_data);
				     j++) {
					dprintf("0x%x",
						format_max.fmt.raw_data[j]);
					if (j < sizeof(format_max.fmt.raw_data)) {
						dprintf1(", ");
					}
				}
				dprintf1(" }}\n");

				/* TODO: check the different fields */

			} else {
				CU_ASSERT_EQUAL(ret_max, -1);
				CU_ASSERT_EQUAL(errno_max, EINVAL);
			}

			memset(&format_min, 0, sizeof(format_min));
			format_min.type = type;
			format_min.fmt.pix.pixelformat = fmtdesc.pixelformat;

			ret_min =
			    ioctl(get_video_fd(), VIDIOC_S_FMT, &format_min);
			errno_min = errno;

			dprintf
			    ("\t%s:%u: VIDIOC_S_FMT, type=%i, ret_min=%i, errno_min=%i\n",
			     __FILE__, __LINE__, type, ret_min, errno_min);

			if (ret_min == 0) {
				CU_ASSERT_EQUAL(ret_enum, 0);
				dprintf1
				    ("\tformat_min = { ..., .fmt.raw_data[] = { ");
				for (j = 0; j < sizeof(format_min.fmt.raw_data);
				     j++) {
					dprintf("0x%x",
						format_min.fmt.raw_data[j]);
					if (j < sizeof(format_min.fmt.raw_data)) {
						dprintf1(", ");
					}
				}
				dprintf1(" }}\n");

				/* TODO: check the different fields */

			} else {
				CU_ASSERT_EQUAL(ret_min, -1);
				CU_ASSERT_EQUAL(errno_min, EINVAL);
			}

			if (ret_max == -1 && ret_min == -1) {
				CU_ASSERT_EQUAL(ret_enum, -1);
				CU_ASSERT_EQUAL(errno_enum, EINVAL);
			}
			break;
		}

		if (ret_enum == 0) {
			CU_ASSERT_EQUAL(ret_enum, 0);
		} else {
			CU_ASSERT_EQUAL(ret_enum, -1);
			CU_ASSERT_EQUAL(errno_enum, EINVAL);
		}

		i++;
	} while (ret_enum == 0 && i != 0);

	memset(&format_set, 0xff, sizeof(format_set));
	format_set = format_orig;

	ret_set = ioctl(get_video_fd(), VIDIOC_S_FMT, &format_set);
	errno_set = errno;

	dprintf("\t%s:%u: VIDIOC_S_FMT, type=%i, ret_set=%i, errno_set=%i\n",
		__FILE__, __LINE__, format_orig.type, ret_set, errno_set);

	if (ret_get == 0) {
		CU_ASSERT_EQUAL(ret_get, 0);
		CU_ASSERT_EQUAL(ret_set, 0);

		CU_ASSERT_EQUAL(format_orig.type, type);
		CU_ASSERT_EQUAL(format_set.type, type);
	} else {
		CU_ASSERT_EQUAL(ret_get, -1);
		CU_ASSERT_EQUAL(errno_get, EINVAL);
		CU_ASSERT_EQUAL(ret_set, -1);
		CU_ASSERT_EQUAL(errno_set, EINVAL);
	}

}

void test_VIDIOC_S_FMT_enum()
{
	do_set_formats_enum(V4L2_BUF_TYPE_VIDEO_CAPTURE);
	do_set_formats_enum(V4L2_BUF_TYPE_VIDEO_OUTPUT);
	do_set_formats_enum(V4L2_BUF_TYPE_VIDEO_OVERLAY);
	do_set_formats_enum(V4L2_BUF_TYPE_VBI_CAPTURE);
	do_set_formats_enum(V4L2_BUF_TYPE_VBI_OUTPUT);
	do_set_formats_enum(V4L2_BUF_TYPE_SLICED_VBI_CAPTURE);
	do_set_formats_enum(V4L2_BUF_TYPE_SLICED_VBI_OUTPUT);
	do_set_formats_enum(V4L2_BUF_TYPE_VIDEO_OUTPUT_OVERLAY);
	do_set_formats_enum(V4L2_BUF_TYPE_PRIVATE);
}

static void do_set_formats_type(enum v4l2_buf_type type)
{
	int ret_set, errno_set;
	struct v4l2_format format;
	struct v4l2_format format2;

	memset(&format, 0, sizeof(format));
	format.type = type;

	ret_set = ioctl(get_video_fd(), VIDIOC_S_FMT, &format);
	errno_set = errno;

	dprintf("\t%s:%u: VIDIOC_S_FMT, type=%i, ret_set=%i, errno_set=%i\n",
		__FILE__, __LINE__, type, ret_set, errno_set);

	CU_ASSERT_EQUAL(ret_set, -1);
	CU_ASSERT_EQUAL(errno_set, EINVAL);

	/* Check whether the format structure is untouched */
	memset(&format2, 0, sizeof(format2));
	format2.type = type;
	CU_ASSERT_EQUAL(memcmp(&format, &format2, sizeof(format)), 0);

}

void test_VIDIOC_S_FMT_type()
{
	do_set_formats_type(0);
	do_set_formats_type(9);
	do_set_formats_type(V4L2_BUF_TYPE_PRIVATE - 1);
	do_set_formats_type(S16_MIN);
	do_set_formats_type(S16_MAX);
	do_set_formats_type(S32_MAX);
}

/* TODO: test cases for VIDIOC_TRY_FMT */

/*
   TODO: test case for VIDIOC_TRY_FMT with invalid type

   TODO: test case for VIDIOC_S_FMT with type=V4L2_BUF_TYPE_VIDEO_CAPTURE and
    V4L2_BUF_TYPE_VIDEO_OUTPUT
        - with different field settings
*/
