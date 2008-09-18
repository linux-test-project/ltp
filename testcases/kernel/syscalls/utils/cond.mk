# cond.mk --- useful functions to write conditions
#
#  Copyright (c) International Business Machines  Corp., 2001
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
#  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
#

# NAME
#        check_header: Checking the existence of header file.
#
# SYNOPSIS
#        $(call check_header,HEADFILE)
#        $(call check_header,HEADFILE,STRING_IF_FOUND)
#        $(call check_header,HEADFILE,STRING_IF_FOUND,STRING_IF_NOT_FOUND)
#
# DESCRIPTION
#
# check_header checks whether $(CC) can found HEADFILE or not.
#
# With the first form, "yes" is returned if it is found.
# "no" is returned if it is not found.
#
# With the second form, STRING_IF_FOUND is returned if it is found.
# "no" is returned if it is not found.
#
# With the second form, STRING_IF_FOUND is returned if it is found.
# STRING_IF_NOT_FOUND is returned if it is not found.
#
# EXAMPLES
#
# (1) yes or no
#
#     include ../utils/cond.mk
#
#     ifeq ($(call check_header,foo.h),yes)
#     RULES if foo.h is available.
#     else
#     RULES IF foo.h is NOT available.
#     endif
#
#
# (2) adding CFLAGS#   CFLAGS += $(call check_header,foo.h,-DHAS_FOO_H, )
#
#     CFLAGS += $(call check_header,foo.h,-DHAS_FOO_H, )
#
#
# NOTE
#
# Spaces after `,' are not striped automatically.
#
# The value for CFLAGS is different in following assignment:
#
#   CFLAGS += $(call check_header,foo.h,-DHAS_FOO_H, )
#   CFLAGS += $(call check_header,foo.h,-DHAS_FOO_H,)
#

# If check_header refers CFLAGS directly, expression like 
#  
#   CFLAGS += $(call check_header ...)
#
# causes "Recursive variable `CFLAGS' references itself" error.
# COND_CFLAGS is introduced to avoid the error.
COND_CFLAGS := $(CFLAGS)
check_header = $(shell								\
	if [ "x$(2)" = "x" ]; then FOUND=yes; else FOUND="$(2)"; fi;		\
	if [ "x$(3)" = "x" ]; then NOTFOUND=no; else NOTFOUND="$(3)"; fi;	\
	if echo "\#include <$(1)>" | $(CPP) $(COND_CFLAGS) - > /dev/null 2>&1 ; \
	then echo "$${FOUND}" ;                                                 \
        else echo "$${NOTFOUND}" ; fi)
# TODO: CPPFLAGS should be used here. 



#COND_MK_DEBUG=yes
ifdef COND_MK_DEBUG
all:
	@echo "-DHAS_STDIO_H == $(call check_header,stdio.h,-DHAS_STDIO_H,)"
	@echo "\" \" == \"$(call check_header,foo.h,-DHAS_FOO_H, )\""
	@echo "yes == $(call check_header,stdio.h)"
	@echo "no == $(call check_header,foo.h)"
	@echo "YES == $(call check_header,stdio.h,YES)"
	@echo "no == $(call check_header,foo.h,YES)"
	@echo "YES == $(call check_header,stdio.h,YES,NO)"
	@echo "NO == $(call check_header,foo.h,YES,NO)"
endif
# cond.mk ends here.
