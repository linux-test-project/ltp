#
#    pan Makefile.
#
#    Copyright (C) 2009, Cisco Systems Inc.
#
#    This program is free software; you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation; either version 2 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License along
#    with this program; if not, write to the Free Software Foundation, Inc.,
#    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
#
# Ngie Cooper, July 2009
#

top_srcdir		?= ..

include $(top_srcdir)/include/mk/env_pre.mk
include $(top_srcdir)/include/mk/functions.mk

# XXX (garrcoop): some versions of flex/bison generate crap code that doesn't
# check the return code from fwrite(3).
CPPFLAGS		+= -Wno-error

CPPFLAGS		+= -I$(abs_srcdir)

LDLIBS			+= -lm

LFLAGS			+= -l

INSTALL_DIR		:= bin

MAKE_TARGETS		:= ltp-bump ltp-pan

ltp-bump: ltp-bump.o zoolib.o

ltp-pan: ltp-pan.o zoolib.o splitstr.o

# flex does some whacky junk when it generates files on the fly, so let's make
# sure gcc doesn't get lost...
vpath %.c $(abs_srcdir):$(abs_builddir)))
vpath %.l $(abs_srcdir)

include $(top_srcdir)/include/mk/generic_leaf_target.mk
