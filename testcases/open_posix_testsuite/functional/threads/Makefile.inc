#
# Include this file in your Makefile for linking against the built NPTL 
# and GLIBC
# 
# NOTE: Before making the test suite, you need to build glibc. $LIBC_PATCH 
# stands for the path of GLIBC, please export it first.

LDFLAGS += -lpthread
