#
# v4l-test: Test environment for Video For Linux Two API
#
#  1 Jan 2009  0.4  test_VIDIOC_ENUMOUTPUT.c added,
#                   test_VIDIOC_QUERYCTRL.c added
# 23 Dec 2008  0.3  test_VIDIOC_LOG_STATUS.c added
# 22 Dec 2008  0.2  test_VIDIOC_QUERYCAP.c added
# 18 Dec 2008  0.1  First release
#
# Written by Márton Németh <nm127@freemail.hu>
# Released under GPL
#

CFLAGS = -O2 -Wall -Wextra
#CFLAGS += -g
LDFLAGS = -lcunit

# TODO: handle dependencies

OBJS = dev_video.o \
       test_VIDIOC_QUERYCAP.o \
       test_VIDIOC_ENUMAUDIO.o \
       test_VIDIOC_ENUMAUDOUT.o \
       test_VIDIOC_ENUM_FMT.o \
       test_VIDIOC_ENUMINPUT.o \
       test_VIDIOC_ENUMOUTPUT.o \
       test_VIDIOC_ENUMSTD.o \
       test_VIDIOC_LOG_STATUS.o \
       test_VIDIOC_CROPCAP.o \
       test_VIDIOC_QUERYCTRL.o \
       test_VIDIOC_STD.o \
       test_VIDIOC_INPUT.o \
       test_invalid_ioctl.o \
       v4l2_test.o

all: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJS) -o v4l-test

clean:
	rm -f $(OBJS)
	rm -f v4l-test
