#
#  (C) Copyright IBM Corp. 2001, 2003
#
#  This program is free software;  you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY;  without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
#  the GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program;  if not, write to the Free Software
#  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
#

###########################################################################
# name of file	: Makefile						  #
###########################################################################

top_srcdir	?= ../../..

include $(top_srcdir)/include/mk/testcases.mk

CPPFLAGS	+= -I$(abs_srcdir)/../include -I$(abs_srcdir)/../testlib -DLTP

LDFLAGS		+= $(addprefix -L$(abs_builddir)/../,lib testlib)

LDLIBS		+= -lsctputil -lsctp -lpthread

V4_TARGETS	:= $(patsubst $(abs_srcdir)/%.c,%,$(wildcard $(abs_srcdir)/*.c))

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
