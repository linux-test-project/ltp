# SPDX-License-Identifier: GPL-2.0-or-later
# (C) Copyright IBM Corp. 2001, 2003

top_srcdir	?= ../../..

include $(top_srcdir)/include/mk/testcases.mk

CPPFLAGS	+= -I$(abs_srcdir)/../include -I$(abs_srcdir)/../testlib -DLTP

LDFLAGS		+= $(addprefix -L$(abs_builddir)/../,lib testlib)

LDLIBS		+= -lsctputil -lsctp -lpthread

V4_TARGETS	:= $(patsubst $(abs_srcdir)/%.c,%,$(sort $(wildcard $(abs_srcdir)/*.c)))

V6_TARGETS	:= test_basic_v6 test_fragments_v6 test_getname_v6 \
		   test_inaddr_any_v6 test_peeloff_v6 \
		   test_sctp_sendrecvmsg_v6 test_sockopt_v6 \
		   test_tcp_style_v6 test_timetolive_v6

$(V6_TARGETS): CPPFLAGS += -DTEST_V6=1

%_v6.o: %.c
	$(COMPILE.c) $(OUTPUT_OPTION) $<

MAKE_TARGETS	:= $(V4_TARGETS) $(V6_TARGETS)

v4test: $(V4_TARGETS)
	@for a in $^;                           \
	do                                      \
		echo "./$$a";                   \
		if ./$$a; then                  \
			echo "$$a - passed";    \
			echo "";                \
		else                            \
			echo "$$a - failed";    \
			exit 1;                 \
		fi;                             \
	done

v6test: $(V6_TARGETS)
	@for a in $^;                           \
	do                                      \
		echo "./$$a";                   \
		if ./$$a; then                  \
			echo "$$a - passed";    \
			echo "";                \
		else                            \
			echo "$$a - failed";    \
			exit 1;                 \
		fi;                             \
	done

include $(top_srcdir)/include/mk/generic_leaf_target.mk
