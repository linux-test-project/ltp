/*
 * v4l-test: Test environment for Video For Linux Two API
 *
 * 18 Apr 2009  0.4  More cleanup in frequency scan test case
 * 28 Mar 2009  0.3  Clean up ret and errno variable names and dprintf() output
 *  1 Feb 2009  0.2  Added test cases for VIDIOC_S_FREQUENCY
 * 31 Jan 2009  0.1  First release
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

#include "test_VIDIOC_FREQUENCY.h"

void test_VIDIOC_G_FREQUENCY()
{
	int ret_get, errno_get;
	__u32 tuner;
	struct v4l2_frequency freq;

	tuner = 0;

	memset(&freq, 0xff, sizeof(freq));
	freq.tuner = tuner;
	ret_get = ioctl(get_video_fd(), VIDIOC_G_FREQUENCY, &freq);
	errno_get = errno;

	dprintf("\t%s:%u: VIDIOC_G_FREQUENCY, ret_get=%i, errno_get=%i\n",
		__FILE__, __LINE__, ret_get, errno_get);

	if (ret_get == 0) {
		CU_ASSERT_EQUAL(ret_get, 0);

		CU_ASSERT_EQUAL(freq.tuner, tuner);

		//CU_ASSERT(freq.type, ???);
		//CU_ASSERT_EQUAL(freq.frequency, ???);

		CU_ASSERT_EQUAL(freq.reserved[0], 0);
		CU_ASSERT_EQUAL(freq.reserved[1], 0);
		CU_ASSERT_EQUAL(freq.reserved[2], 0);
		CU_ASSERT_EQUAL(freq.reserved[3], 0);
		CU_ASSERT_EQUAL(freq.reserved[4], 0);
		CU_ASSERT_EQUAL(freq.reserved[5], 0);
		CU_ASSERT_EQUAL(freq.reserved[6], 0);
		CU_ASSERT_EQUAL(freq.reserved[7], 0);

		dprintf("\tfreq = { "
			".tuner = %u, "
			".type = 0x%X, "
			".frequency = %u "
			".reserved[]={ 0x%X, 0x%X, 0x%X, 0x%X, 0x%X, 0x%X, 0x%X, 0x%X } }\n",
			freq.tuner,
			freq.type,
			freq.frequency,
			freq.reserved[0],
			freq.reserved[1],
			freq.reserved[2],
			freq.reserved[3],
			freq.reserved[4],
			freq.reserved[5], freq.reserved[6], freq.reserved[7]
		    );
	} else {
		CU_ASSERT_EQUAL(ret_get, -1);
		CU_ASSERT_EQUAL(errno_get, EINVAL);
	}

}

void test_VIDIOC_G_FREQUENCY_S32_MAX()
{
	int ret_get, errno_get;
	__u32 tuner;
	struct v4l2_frequency freq;

	tuner = (__u32) S32_MAX;

	memset(&tuner, 0xff, sizeof(tuner));
	freq.tuner = tuner;
	ret_get = ioctl(get_video_fd(), VIDIOC_G_FREQUENCY, &freq);
	errno_get = errno;

	dprintf("\t%s:%u: VIDIOC_G_FREQUENCY, ret_get=%i, errno_get=%i\n",
		__FILE__, __LINE__, ret_get, errno_get);

	CU_ASSERT_EQUAL(ret_get, -1);
	CU_ASSERT_EQUAL(errno_get, EINVAL);
}

void test_VIDIOC_G_FREQUENCY_S32_MAX_1()
{
	int ret_get, errno_get;
	__u32 tuner;
	struct v4l2_frequency freq;

	tuner = (__u32) S32_MAX + 1;

	memset(&tuner, 0xff, sizeof(tuner));
	freq.tuner = tuner;
	ret_get = ioctl(get_video_fd(), VIDIOC_G_FREQUENCY, &freq);
	errno_get = errno;

	dprintf("\t%s:%u: VIDIOC_G_FREQUENCY, ret_get=%i, errno_get=%i\n",
		__FILE__, __LINE__, ret_get, errno_get);

	CU_ASSERT_EQUAL(ret_get, -1);
	CU_ASSERT_EQUAL(errno_get, EINVAL);
}

void test_VIDIOC_G_FREQUENCY_U32_MAX()
{
	int ret_get, errno_get;
	__u32 tuner;
	struct v4l2_frequency freq;

	tuner = U32_MAX;

	memset(&tuner, 0xff, sizeof(tuner));
	freq.tuner = tuner;
	ret_get = ioctl(get_video_fd(), VIDIOC_G_FREQUENCY, &freq);
	errno_get = errno;

	dprintf("\t%s:%u: VIDIOC_G_FREQUENCY, ret_get=%i, errno_get=%i\n",
		__FILE__, __LINE__, ret_get, errno_get);

	CU_ASSERT_EQUAL(ret_get, -1);
	CU_ASSERT_EQUAL(errno_get, EINVAL);
}

void test_VIDIOC_G_FREQUENCY_NULL()
{
	int ret_get, errno_get;
	int ret_null, errno_null;
	struct v4l2_frequency freq;
	__u32 tuner;

	tuner = 0;

	memset(&freq, 0xff, sizeof(freq));
	freq.tuner = tuner;
	freq.type = V4L2_TUNER_ANALOG_TV;
	ret_get = ioctl(get_video_fd(), VIDIOC_G_FREQUENCY, &freq);
	errno_get = errno;

	dprintf("\t%s:%u: VIDIOC_G_FREQUENCY, ret_get=%i, errno_get=%i\n",
		__FILE__, __LINE__, ret_get, errno_get);

	ret_null = ioctl(get_video_fd(), VIDIOC_G_FREQUENCY, NULL);
	errno_null = errno;

	dprintf("\t%s:%u: VIDIOC_G_FREQUENCY, ret_null=%i, errno_null=%i\n",
		__FILE__, __LINE__, ret_null, errno_null);

	/* check if VIDIOC_G_FREQUENCY is supported at all or not */
	if (ret_get == 0) {
		/* VIDIOC_G_FREQUENCY is supported, the parameter should be checked */
		CU_ASSERT_EQUAL(ret_null, -1);
		CU_ASSERT_EQUAL(errno_null, EFAULT);
	} else {
		/* VIDIOC_G_FREQUENCY not supported at all, the parameter should not be evaluated */
		CU_ASSERT_EQUAL(ret_null, -1);
		CU_ASSERT_EQUAL(errno_null, EINVAL);
	}
}

void test_VIDIOC_S_FREQUENCY()
{
	int ret_get, errno_get;
	int ret_set, errno_set;
	__u32 tuner;
	struct v4l2_frequency orig_freq;
	struct v4l2_frequency freq;
	struct v4l2_frequency new_freq;

	tuner = 0;

	/* fetch the current frequency setting */
	memset(&orig_freq, 0xff, sizeof(orig_freq));
	orig_freq.tuner = tuner;
	ret_get = ioctl(get_video_fd(), VIDIOC_G_FREQUENCY, &orig_freq);
	errno_get = errno;

	dprintf("\t%s:%u: VIDIOC_G_FREQUENCY, ret_get=%i, errno_get=%i\n",
		__FILE__, __LINE__, ret_get, errno_get);

	if (ret_get == 0) {
		CU_ASSERT_EQUAL(orig_freq.tuner, tuner);

		/* try to set the frequency again to the actual value */
		memset(&freq, 0xff, sizeof(freq));
		freq.tuner = tuner;
		freq.frequency = orig_freq.frequency;
		freq.type = V4L2_TUNER_ANALOG_TV;
		ret_set = ioctl(get_video_fd(), VIDIOC_S_FREQUENCY, &orig_freq);
		errno_set = errno;

		dprintf
		    ("\t%s:%u: VIDIOC_S_FREQUENCY, ret_set=%i, errno_set=%i\n",
		     __FILE__, __LINE__, ret_set, errno_set);

		CU_ASSERT_EQUAL(ret_set, 0);
		if (ret_set == 0) {

			/* check wheteher the frequency has not been changed */
			memset(&new_freq, 0xff, sizeof(new_freq));
			new_freq.tuner = tuner;
			ret_get =
			    ioctl(get_video_fd(), VIDIOC_G_FREQUENCY,
				  &new_freq);
			errno_get = errno;

			dprintf
			    ("\t%s:%u: VIDIOC_G_FREQUENCY, ret_get=%i, errno_get=%i\n",
			     __FILE__, __LINE__, ret_get, errno_get);

			CU_ASSERT_EQUAL(ret_get, 0);
			if (ret_get == 0) {
				dprintf
				    ("\t%s:%u: current frequency=%u (expected %u)\n",
				     __FILE__, __LINE__, new_freq.frequency,
				     orig_freq.frequency);
				CU_ASSERT_EQUAL(new_freq.frequency,
						orig_freq.frequency);
			}

		}

	} else {
		CU_ASSERT_EQUAL(ret_get, -1);
		CU_ASSERT_EQUAL(errno_get, EINVAL);

		/* VIDIOC_G_FREQUENCY not supported, so shall be VIDIOC_S_FREQUENCY */

		memset(&freq, 0, sizeof(freq));
		freq.tuner = tuner;
		freq.type = V4L2_TUNER_ANALOG_TV;
		freq.frequency = 0;
		ret_set = ioctl(get_video_fd(), VIDIOC_S_FREQUENCY, &freq);
		errno_set = errno;

		dprintf
		    ("\t%s:%u: VIDIOC_S_FREQUENCY, ret_set=%i, errno_set=%i\n",
		     __FILE__, __LINE__, ret_set, errno_set);

		CU_ASSERT_EQUAL(ret_set, -1);
		CU_ASSERT_EQUAL(errno_set, EINVAL);

	}

}

void test_VIDIOC_S_FREQUENCY_boundaries()
{
	int ret_g_tuner = 0;
	int ret_g_freq = 0;
	int errno_g_tuner = 0;
	int errno_g_freq = 0;
	int ret;
	__u32 index;
	struct v4l2_frequency orig_freq;
	struct v4l2_frequency freq;
	struct v4l2_frequency new_freq;
	struct v4l2_tuner tuner;

	/* this test case depends on working VIDIOC_G_TUNER and VIDIOC_G_FREQUENCY commands */

	index = 0;

	/* fetch the minimum (tuner.rangelow) and maximum (tuner.rangehigh) frequency */
	memset(&tuner, 0xff, sizeof(tuner));
	tuner.index = index;
	ret_g_tuner = ioctl(get_video_fd(), VIDIOC_G_TUNER, &tuner);
	errno_g_tuner = errno;

	dprintf
	    ("\t%s:%u: VIDIOC_G_TUNER, ret=%i, tuner.rangelow=%u, tuner.rangehigh=%u\n",
	     __FILE__, __LINE__, ret_g_tuner, tuner.rangelow, tuner.rangehigh);
	CU_ASSERT_EQUAL(tuner.index, index);

	/* fetch the current frequency setting */
	memset(&orig_freq, 0xff, sizeof(orig_freq));
	orig_freq.tuner = index;
	ret_g_freq = ioctl(get_video_fd(), VIDIOC_G_FREQUENCY, &orig_freq);
	errno_g_freq = errno;

	dprintf
	    ("\t%s:%u: VIDIOC_G_FREQUENCY, ret_g_freq=%i, orig_freq.frequency=%u\n",
	     __FILE__, __LINE__, ret_g_freq, orig_freq.frequency);
	CU_ASSERT_EQUAL(orig_freq.tuner, index);

	if (ret_g_tuner == 0 && ret_g_freq == 0) {
		CU_ASSERT_EQUAL(orig_freq.tuner, index);

		/* try to set the frequency to zero */
		memset(&freq, 0xff, sizeof(freq));
		freq.tuner = index;
		freq.type = orig_freq.type;
		freq.frequency = 0;
		ret = ioctl(get_video_fd(), VIDIOC_S_FREQUENCY, &freq);
		dprintf("\t%s:%u: set to %u: VIDIOC_S_FREQUENCY, ret=%i\n",
			__FILE__, __LINE__, 0, ret);

		CU_ASSERT_EQUAL(ret, 0);
		if (ret == 0) {

			/* check wheteher the frequency has been changed to the lowest
			 * possible value
			 */
			memset(&new_freq, 0xff, sizeof(new_freq));
			new_freq.tuner = index;
			ret =
			    ioctl(get_video_fd(), VIDIOC_G_FREQUENCY,
				  &new_freq);

			dprintf
			    ("\t%s:%u: VIDIOC_G_FREQUENCY, ret=%i, new_freq.frequency=%u\n",
			     __FILE__, __LINE__, ret, new_freq.frequency);

			CU_ASSERT_EQUAL(ret, 0);
			if (ret == 0) {
				CU_ASSERT_EQUAL(new_freq.frequency,
						tuner.rangelow);
			}
		}

		/* try to set the frequency to tuner.rangelow-1, if applicable */
		if (0 < tuner.rangelow) {
			memset(&freq, 0xff, sizeof(freq));
			freq.tuner = index;
			freq.type = orig_freq.type;
			freq.frequency = tuner.rangelow - 1;
			ret = ioctl(get_video_fd(), VIDIOC_S_FREQUENCY, &freq);

			dprintf
			    ("\t%s:%u: set to %u: VIDIOC_S_FREQUENCY, ret=%i\n",
			     __FILE__, __LINE__, tuner.rangelow - 1, ret);

			CU_ASSERT_EQUAL(ret, 0);
			if (ret == 0) {

				/* check wheteher the frequency has been changed to the lowest
				 * possible value
				 */
				memset(&new_freq, 0xff, sizeof(new_freq));
				new_freq.tuner = index;
				ret =
				    ioctl(get_video_fd(), VIDIOC_G_FREQUENCY,
					  &new_freq);

				dprintf
				    ("\t%s:%u: VIDIOC_G_FREQUENCY, ret=%i, new_freq.frequency=%u\n",
				     __FILE__, __LINE__, ret,
				     new_freq.frequency);
				CU_ASSERT_EQUAL(ret, 0);
				if (ret == 0) {
					dprintf
					    ("\t%s:%u: current frequency=%u (expected %u)\n",
					     __FILE__, __LINE__,
					     new_freq.frequency,
					     tuner.rangelow);
					CU_ASSERT_EQUAL(new_freq.frequency,
							tuner.rangelow);
				}
			}
		}

		/* try to set the frequency to tuner.rangelow */
		memset(&freq, 0xff, sizeof(freq));
		freq.tuner = index;
		freq.type = orig_freq.type;
		freq.frequency = tuner.rangelow;
		ret = ioctl(get_video_fd(), VIDIOC_S_FREQUENCY, &freq);

		dprintf("\t%s:%u: set to %u: VIDIOC_S_FREQUENCY, ret=%i\n",
			__FILE__, __LINE__, tuner.rangelow, ret);

		CU_ASSERT_EQUAL(ret, 0);
		if (ret == 0) {

			/* check wheteher the frequency has been changed to the lowest
			 * possible value
			 */
			memset(&new_freq, 0xff, sizeof(new_freq));
			new_freq.tuner = index;
			ret =
			    ioctl(get_video_fd(), VIDIOC_G_FREQUENCY,
				  &new_freq);

			dprintf
			    ("\t%s:%u: VIDIOC_G_FREQUENCY, ret=%i, new_freq.frequency=%u\n",
			     __FILE__, __LINE__, ret, new_freq.frequency);

			CU_ASSERT_EQUAL(ret, 0);
			if (ret == 0) {
				dprintf
				    ("\t%s:%u: current frequency=%u (expected %u)\n",
				     __FILE__, __LINE__, new_freq.frequency,
				     tuner.rangelow);
				CU_ASSERT_EQUAL(new_freq.frequency,
						tuner.rangelow);
			}
		}

		/* try to set the frequency to tuner.rangehigh */
		memset(&freq, 0xff, sizeof(freq));
		freq.tuner = index;
		freq.type = orig_freq.type;
		freq.frequency = tuner.rangehigh;
		ret = ioctl(get_video_fd(), VIDIOC_S_FREQUENCY, &freq);

		dprintf("\t%s:%u: set to %u: VIDIOC_S_FREQUENCY, ret=%i\n",
			__FILE__, __LINE__, tuner.rangehigh, ret);

		CU_ASSERT_EQUAL(ret, 0);
		if (ret == 0) {

			/* check wheteher the frequency has been changed to the highest
			 * possible value
			 */
			memset(&new_freq, 0xff, sizeof(new_freq));
			new_freq.tuner = index;
			ret =
			    ioctl(get_video_fd(), VIDIOC_G_FREQUENCY,
				  &new_freq);

			dprintf
			    ("\t%s:%u: VIDIOC_G_FREQUENCY, ret=%i, new_freq.frequency=%u\n",
			     __FILE__, __LINE__, ret, new_freq.frequency);

			CU_ASSERT_EQUAL(ret, 0);
			if (ret == 0) {
				dprintf
				    ("\t%s:%u: current frequency=%u (expected %u)\n",
				     __FILE__, __LINE__, new_freq.frequency,
				     tuner.rangehigh);
				CU_ASSERT_EQUAL(new_freq.frequency,
						tuner.rangehigh);
			}
		}

		/* try to set the frequency to tuner.rangehigh+1, if applicable */
		if (tuner.rangehigh < U32_MAX) {
			memset(&freq, 0xff, sizeof(freq));
			freq.tuner = index;
			freq.type = orig_freq.type;
			freq.frequency = tuner.rangehigh + 1;
			ret = ioctl(get_video_fd(), VIDIOC_S_FREQUENCY, &freq);

			dprintf
			    ("\t%s:%u: set to %u: VIDIOC_S_FREQUENCY, ret=%i\n",
			     __FILE__, __LINE__, tuner.rangehigh + 1, ret);

			CU_ASSERT_EQUAL(ret, 0);
			if (ret == 0) {

				/* check wheteher the frequency has been changed to the highest
				 * possible value
				 */
				memset(&new_freq, 0xff, sizeof(new_freq));
				new_freq.tuner = index;
				ret =
				    ioctl(get_video_fd(), VIDIOC_G_FREQUENCY,
					  &new_freq);

				dprintf
				    ("\t%s:%u: VIDIOC_G_FREQUENCY, ret=%i, new_freq.frequency=%u\n",
				     __FILE__, __LINE__, ret,
				     new_freq.frequency);

				CU_ASSERT_EQUAL(ret, 0);
				if (ret == 0) {
					dprintf
					    ("\t%s:%u: current frequency=%u (expected %u)\n",
					     __FILE__, __LINE__,
					     new_freq.frequency,
					     tuner.rangehigh);
					CU_ASSERT_EQUAL(new_freq.frequency,
							tuner.rangehigh);
				}
			}
		}

		/* try to set the frequency to U32_MAX */
		memset(&freq, 0xff, sizeof(freq));
		freq.tuner = index;
		freq.type = orig_freq.type;
		freq.frequency = U32_MAX;
		ret = ioctl(get_video_fd(), VIDIOC_S_FREQUENCY, &freq);

		dprintf("\t%s:%u: set to %u: VIDIOC_S_FREQUENCY, ret=%i\n",
			__FILE__, __LINE__, U32_MAX, ret);
		CU_ASSERT_EQUAL(ret, 0);
		if (ret == 0) {

			/* check wheteher the frequency has been changed to the highest
			 * possible value
			 */
			memset(&new_freq, 0xff, sizeof(new_freq));
			new_freq.tuner = index;
			ret =
			    ioctl(get_video_fd(), VIDIOC_G_FREQUENCY,
				  &new_freq);

			dprintf
			    ("\t%s:%u: VIDIOC_G_FREQUENCY, ret=%i, new_freq.frequency=%u\n",
			     __FILE__, __LINE__, ret, new_freq.frequency);

			CU_ASSERT_EQUAL(ret, 0);
			if (ret == 0) {
				dprintf
				    ("\t%s:%u: current frequency=%u (expected %u)\n",
				     __FILE__, __LINE__, new_freq.frequency,
				     tuner.rangehigh);
				CU_ASSERT_EQUAL(new_freq.frequency,
						tuner.rangehigh);
			}
		}

		/* try restore the original frequency settings */
		memset(&freq, 0xff, sizeof(freq));
		freq.tuner = index;
		freq.type = orig_freq.type;
		freq.frequency = orig_freq.frequency;
		ret = ioctl(get_video_fd(), VIDIOC_S_FREQUENCY, &freq);

		dprintf("\t%s:%u: set to %u: VIDIOC_S_FREQUENCY, ret=%i\n",
			__FILE__, __LINE__, orig_freq.frequency, ret);

		CU_ASSERT_EQUAL(ret, 0);
		if (ret == 0) {

			/* check wheteher the frequency has been restored */
			memset(&new_freq, 0xff, sizeof(new_freq));
			new_freq.tuner = index;
			ret =
			    ioctl(get_video_fd(), VIDIOC_G_FREQUENCY,
				  &new_freq);

			dprintf
			    ("\t%s:%u: VIDIOC_G_FREQUENCY, ret=%i, new_freq.frequency=%u\n",
			     __FILE__, __LINE__, ret, new_freq.frequency);

			CU_ASSERT_EQUAL(ret, 0);
			if (ret == 0) {
				dprintf
				    ("\t%s:%u: current frequency=%u (expected %u)\n",
				     __FILE__, __LINE__, new_freq.frequency,
				     orig_freq.frequency);
				CU_ASSERT_EQUAL(new_freq.frequency,
						orig_freq.frequency);
			}
		}
	}

	if (ret_g_freq != 0) {
		dprintf("\t%s:%u: ret_g_freq=%d (expected %d)\n", __FILE__,
			__LINE__, ret_g_freq, -1);
		dprintf("\t%s:%u: errno_g_freq=%d (expected %d)\n", __FILE__,
			__LINE__, errno_g_freq, EINVAL);
		CU_ASSERT_EQUAL(ret_g_freq, -1);
		CU_ASSERT_EQUAL(errno, EINVAL);
	}

	if (ret_g_tuner != 0) {
		dprintf("\t%s:%u: ret_g_tuner=%d (expected %d)\n", __FILE__,
			__LINE__, ret_g_tuner, -1);
		dprintf("\t%s:%u: errno_g_tuner=%d (expected %d)\n", __FILE__,
			__LINE__, errno_g_tuner, EINVAL);
		CU_ASSERT_EQUAL(ret_g_tuner, -1);
		CU_ASSERT_EQUAL(errno, EINVAL);
	}

}

void test_VIDIOC_S_FREQUENCY_scan()
{
	int ret_g_tuner, errno_g_tuner;
	int ret_g_freq, errno_g_freq;
	int ret_get, errno_get;
	int ret_set, errno_set;
	__u32 index;
	struct v4l2_frequency orig_freq;
	struct v4l2_frequency freq;
	struct v4l2_frequency new_freq;
	struct v4l2_frequency prev_freq;
	struct v4l2_tuner tuner;
	__u32 i;

	/* this test case depends on working VIDIOC_G_FREQUENCY command */

	index = 0;

	/* fetch the minimum (tuner.rangelow) and maximum (tuner.rangehigh) frequency */
	memset(&tuner, 0xff, sizeof(tuner));
	tuner.index = index;
	ret_g_tuner = ioctl(get_video_fd(), VIDIOC_G_TUNER, &tuner);
	errno_g_tuner = errno;

	dprintf("\t%s:%u: VIDIOC_G_TUNER, ret_g_tuner=%i, errno_g_tuner=%i\n",
		__FILE__, __LINE__, ret_g_tuner, errno_g_tuner);
	CU_ASSERT_EQUAL(tuner.index, index);

	/* fetch the current frequency setting */
	memset(&orig_freq, 0xff, sizeof(orig_freq));
	orig_freq.tuner = index;
	ret_g_freq = ioctl(get_video_fd(), VIDIOC_G_FREQUENCY, &orig_freq);
	errno_g_freq = errno;

	dprintf
	    ("\t%s:%u: VIDIOC_G_FREQUENCY, ret_g_freq=%i, errno_g_freq=%i, orig_freq.frequency=%u\n",
	     __FILE__, __LINE__, ret_g_freq, errno_g_freq, orig_freq.frequency);
	CU_ASSERT_EQUAL(orig_freq.tuner, index);

	if (ret_g_freq == 0) {
		CU_ASSERT_EQUAL(orig_freq.tuner, index);

		dprintf("\t%s:%u: tuner.rangelow=%u, tuner.rangehigh=%u\n",
			__FILE__, __LINE__, tuner.rangelow, tuner.rangehigh);

		i = tuner.rangelow;
		prev_freq.frequency = 0;
		do {
			/* try to set the frequency */
			memset(&freq, 0xff, sizeof(freq));
			freq.tuner = index;
			freq.type = orig_freq.type;

			freq.frequency = i;
			ret_set =
			    ioctl(get_video_fd(), VIDIOC_S_FREQUENCY, &freq);
			errno_set = errno;
			dprintf
			    ("\t%s:%u: VIDIOC_S_FREQUENCY, ret_set=%i, errno_set=%i, freq.frequency=%u\n",
			     __FILE__, __LINE__, ret_set, errno_set, i);

			CU_ASSERT_EQUAL(ret_set, 0);
			if (ret_set == 0) {

				memset(&new_freq, 0xff, sizeof(new_freq));
				new_freq.tuner = index;
				ret_get =
				    ioctl(get_video_fd(), VIDIOC_G_FREQUENCY,
					  &new_freq);
				errno_get = errno;

				dprintf
				    ("\t%s:%u: VIDIOC_G_FREQUENCY, ret_get=%i, errno_get=%i, new_freq.frequency=%u\n",
				     __FILE__, __LINE__, ret_get, errno_get,
				     new_freq.frequency);

				CU_ASSERT_EQUAL(ret_get, 0);
				if (ret_get == 0) {
					CU_ASSERT(prev_freq.frequency <=
						  new_freq.frequency);
					CU_ASSERT(tuner.rangelow <=
						  new_freq.frequency);
					CU_ASSERT(new_freq.frequency <=
						  tuner.rangehigh);
					prev_freq = new_freq;
				}
			} else {
				printf("\tError %i while setting to %u\n",
				       errno_set, i);
			}
			i++;
		} while (i <= tuner.rangehigh);

		/* try restore the original frequency settings */
		memset(&freq, 0xff, sizeof(freq));
		freq.tuner = index;
		freq.type = orig_freq.type;
		freq.frequency = orig_freq.frequency;
		ret_set = ioctl(get_video_fd(), VIDIOC_S_FREQUENCY, &freq);
		errno_set = errno;

		dprintf
		    ("\t%s:%u: VIDIOC_S_FREQUENCY, ret_set=%i, errno_set=%i\n",
		     __FILE__, __LINE__, ret_set, errno_set);

		CU_ASSERT_EQUAL(ret_set, 0);
		if (ret_set == 0) {

			/* check wheteher the frequency has been restored */
			memset(&new_freq, 0xff, sizeof(new_freq));
			new_freq.tuner = index;
			ret_get =
			    ioctl(get_video_fd(), VIDIOC_G_FREQUENCY,
				  &new_freq);
			errno_get = errno;

			dprintf
			    ("\t%s:%u: VIDIOC_G_FREQUENCY, ret_get=%i, errno_get=%i, new_freq.frequency=%u\n",
			     __FILE__, __LINE__, ret_get, errno_get,
			     new_freq.frequency);

			CU_ASSERT_EQUAL(ret_get, 0);
			if (ret_get == 0) {
				CU_ASSERT_EQUAL(new_freq.frequency,
						orig_freq.frequency);
			}
		}
	}

	if (ret_g_freq != 0) {
		CU_ASSERT_EQUAL(ret_g_freq, -1);
		CU_ASSERT_EQUAL(errno, EINVAL);
	}

	if (ret_g_tuner != 0) {
		CU_ASSERT_EQUAL(ret_g_tuner, -1);
		CU_ASSERT_EQUAL(errno, EINVAL);
	}

}

void test_VIDIOC_S_FREQUENCY_NULL()
{
	int ret_get, errno_get;
	int ret_set, errno_set;
	int ret_null, errno_null;
	struct v4l2_frequency freq_orig;
	struct v4l2_frequency freq;
	__u32 tuner;

	tuner = 0;

	memset(&freq_orig, 0, sizeof(freq_orig));
	freq_orig.tuner = tuner;
	freq_orig.type = V4L2_TUNER_ANALOG_TV;
	ret_get = ioctl(get_video_fd(), VIDIOC_G_FREQUENCY, &freq_orig);
	errno_get = errno;

	dprintf("\t%s:%u: VIDIOC_G_FREQUENCY, ret_get=%i, errno_get=%i\n",
		__FILE__, __LINE__, ret_get, errno_get);

	memset(&freq, 0, sizeof(freq));
	freq.tuner = 0;
	freq.type = V4L2_TUNER_ANALOG_TV;
	freq.frequency = freq_orig.frequency;
	ret_set = ioctl(get_video_fd(), VIDIOC_S_FREQUENCY, &freq_orig);
	errno_set = errno;

	dprintf("\t%s:%u: VIDIOC_G_FREQUENCY, ret_set=%i, errno_set=%i\n",
		__FILE__, __LINE__, ret_set, errno_set);

	ret_null = ioctl(get_video_fd(), VIDIOC_S_FREQUENCY, NULL);
	errno_null = errno;

	dprintf("\t%s:%u: VIDIOC_S_FREQUENCY, ret_null=%i, errno_null=%i\n",
		__FILE__, __LINE__, ret_null, errno_null);

	/* Check whether any of VIDIOC_S_FREQUENCY or VIDIOC_G_FREQUENCY
	 * is supported at all or not
	 */
	if (ret_get == 0 || ret_set == 0) {
		/* VIDIOC_G_FREQUENCY or VIDIOC_S_FREQUENCY is supported, so
		 * the parameter of VIDIOC_S_FREQUENCY should be checked
		 */
		CU_ASSERT_EQUAL(ret_null, -1);
		CU_ASSERT_EQUAL(errno_null, EFAULT);
	} else {
		/* none of VIDIOC_G_FREQUENCY and VIDIOC_S_FREQUENCY is supported,
		 * the parameter should not be evaluated
		 */
		CU_ASSERT_EQUAL(ret_get, -1);
		CU_ASSERT_EQUAL(errno_get, EINVAL);
		CU_ASSERT_EQUAL(ret_set, -1);
		CU_ASSERT_EQUAL(errno_set, EINVAL);
		CU_ASSERT_EQUAL(ret_null, -1);
		CU_ASSERT_EQUAL(errno_null, EINVAL);
	}

}
