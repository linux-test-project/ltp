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

DEBUG_CFLAGS=-O
DEBUG_LDFLAGS=-s
CFLAGS+=-I../../include -g -Wall
LDLIBS+=
EXCLUDE_DIR=epoll

#
# Commented this out since there are directories here we don't want built by default
#
SUBDIR = `ls */Makefile | sed "s/Makefile//g" | grep -v epoll` 
UCLINUX_SUBDIR = `ls */Makefile | sed "s/Makefile//g" | grep -vE "^fork|epoll|capget|capset|chmod|chown|llseek|nftw|clone|profil|getcontext"`

all:
	@set -e; for i in lib $(SUBDIR); do \
		$(MAKE) -C $$i  all; \
	done

install:
	@set -e; for i in lib $(SUBDIR); do \
		$(MAKE) -C $$i  install; \
	done

uclinux:  
	@set -e; for i in lib $(UCLINUX_SUBDIR); do \
		$(MAKE) -C $$i UCLINUX=1 all; \
	done

uclinux_install:
	@set -e; for i in lib $(UCLINUX_SUBDIR); do \
		$(MAKE) -C $$i UCLINUX=1 all; \
	done

clean:
	@set -e; for i in lib $(SUBDIR); do \
		$(MAKE) -C $$i  clean; \
	done

