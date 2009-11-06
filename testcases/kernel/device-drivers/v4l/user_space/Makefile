#
# v4l-test: Test environment for Video For Linux Two API
#
# 16 Jul 2009  0.19 test_VIDIOC_JPEGCOMP.c and
#                   test_VIDIOC_ENUM_FRAMESIZES.c added
#  5 Jul 2009  0.18 v4l2_foreach.c added
# 19 May 2009  0.17 test_VIDIOC_EXT_CTRLS.c added
#  7 May 2009  0.16 v4l2_show.c added
#  5 May 2009  0.15 test_VIDIOC_QUERYBUF.c added
# 25 Apr 2009  0.14 test_VIDIOC_REQBUFS.c added
#  5 Apr 2009  0.13 test_VIDIOC_QUERYMENU.c added
#  4 Apr 2009  0.12 test_VIDIOC_FMT.c added
# 22 Mar 2009  0.11 test_VIDIOC_OUTPUT.c added,
#                   test_VIDIOC_G_SLICED_VBI_CAP.c added
# 18 Mar 2009  0.10 test_VIDIOC_PARM.c added
# 19 Feb 2009  0.9  test_VIDIOC_CTRL.c added
#  7 Feb 2009  0.8  test_VIDIOC_CROP.c added
#  3 Feb 2009  0.7  test_VIDIOC_AUDIO.c added,
#                   test_VIDIOC_AUDOUT.c added,
#  2 Feb 2009  0.6  test_VIDIOC_MODULATOR.c added,
#                   test_VIDIOC_PRIORITY.c added
# 31 Jan 2009  0.5  test_VIDIOC_TUNER.c added,
#                   test_VIDIOC_FREQUENCY.c added,
#                   test_VIDIOC_QUERYSTD.c added,
#                   v4l2_validator.c added
#  1 Jan 2009  0.4  test_VIDIOC_ENUMOUTPUT.c added,
#                   test_VIDIOC_QUERYCTRL.c added
# 23 Dec 2008  0.3  test_VIDIOC_LOG_STATUS.c added
# 22 Dec 2008  0.2  test_VIDIOC_QUERYCAP.c added
# 18 Dec 2008  0.1  First release
#
# Written by Márton Németh <nm127@freemail.hu>
# Released under GPL
#

CFLAGS = -O2 -Wall -W
#CFLAGS += -g
LDFLAGS = -lcunit

# TODO: handle dependencies

OBJS = dev_video.o \
       v4l2_validator.o \
       v4l2_foreach.o \
       test_VIDIOC_QUERYCAP.o \
       test_VIDIOC_ENUMAUDIO.o \
       test_VIDIOC_ENUMAUDOUT.o \
       test_VIDIOC_ENUM_FMT.o \
       test_VIDIOC_JPEGCOMP.o \
       test_VIDIOC_ENUMINPUT.o \
       test_VIDIOC_ENUMOUTPUT.o \
       test_VIDIOC_ENUMSTD.o \
       test_VIDIOC_ENUM_FRAMESIZES.o \
       test_VIDIOC_LOG_STATUS.o \
       test_VIDIOC_CROPCAP.o \
       test_VIDIOC_G_SLICED_VBI_CAP.o \
       test_VIDIOC_CROP.o \
       test_VIDIOC_QUERYCTRL.o \
       test_VIDIOC_QUERYMENU.o \
       test_VIDIOC_QUERYSTD.o \
       test_VIDIOC_STD.o \
       test_VIDIOC_INPUT.o \
       test_VIDIOC_OUTPUT.o \
       test_VIDIOC_TUNER.o \
       test_VIDIOC_MODULATOR.o \
       test_VIDIOC_FREQUENCY.o \
       test_VIDIOC_PRIORITY.o \
       test_VIDIOC_AUDIO.o \
       test_VIDIOC_AUDOUT.o \
       test_VIDIOC_CTRL.o \
       test_VIDIOC_EXT_CTRLS.o \
       test_VIDIOC_PARM.o \
       test_VIDIOC_FMT.o \
       test_VIDIOC_REQBUFS.o \
       test_VIDIOC_QUERYBUF.o \
       test_invalid_ioctl.o \
       v4l2_show.o \
       v4l2_test.o

all: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJS) -o v4l-test

clean:
	rm -f $(OBJS)
	rm -f v4l-test
