#
#  (C) Copyright IBM Corp. 2003
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
#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
#

###########################################################################
# name of file	: Makefile						  #
# description	: make(1) description file for quota/q_mount test         #
###########################################################################

all:	q_mount.img.gz

q_mount.img.gz:	Makefile
	rm -f q_mount.img q_mount.img.gz
	dd if=/dev/zero of=q_mount.img count=1024
	rm -rf mnt
	mkdir mnt
	mke2fs -F q_mount.img
	mount q_mount.img mnt -o loop
	umount mnt
	rm -rf mnt
	gzip q_mount.img

install:	q_mount.img.gz
	ln -f q_mount.img.gz ../../../bin/ || break

uninstall:
	rm -f ../../../bin/q_mount.img.gz

clean:
	rm -rf mnt q_mount.img.gz *~

