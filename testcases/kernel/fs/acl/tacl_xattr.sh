#!/bin/bash
##############################################################
#
#  Copyright (c) International Business Machines  Corp., 2003
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
#  Foundation, 
#
#  FILE        : tacl_xattr.sh
#  USAGE       : ./tacl_xattr.sh
#
#  DESCRIPTION : A script that will test ACL and Extend Attribute on Linux system.
#  REQUIREMENTS:
#                1) Kernel with loop device support
#                2) A spare (scratch) disk partition of 100MB or larger.
#                3) Kernel with ACL and Extend Attribute function support
#
#  HISTORY     :
#      10/23/2003 Kai Zhao (ltcd3@cn.ibm.com)
#
#  CODE COVERAGE: 
#                 76.3% - fs/posix_acl.c 
#                 80.9% - xattr_acl.c
#                 73.0% - xattr.c
#
##############################################################

CUR_PATH=""
CONTENT=""
RES=""
USER_PERMISSION=""
GROUP_PERMISSION=""
OTHER_PERMISSION=""
ITEM_OWNER=""
ITEM_GROUP=""

################################################################
#
# Make sure that uid=root is running this script. 
# Make sure that loop device is built into the kernel 
# Make sure that ACL(Access Control List) and Extended Attribute are 
#     built into the kernel
#
################################################################

if [ $UID != 0 ]
then
	echo "FAILED: Must have root access to execute this script"
	exit 1
fi

#################################################################
#
# Prepare Ext2 file system for ACL and Extended Attribute test
# Make some directory , file and symlink for the test
# Add three users for the test
#
#################################################################

if [ ! -e tacl ]
then
	mkdir -m 777 tacl
else 
	echo "FAILED: Directory tacl are exist"
	exit 1
fi

dd if=/dev/zero of=tacl/blkext2 bs=1k count=10240
chmod 777 tacl/blkext2

losetup /dev/loop0 tacl/blkext2 2>&1 > /dev/null
if [ $? != 0 ]
then
	echo "FAILED: [ losetup ] Must have loop device support by kernel to execute this script"
	exit 1 
fi

mkfs -t ext2 /dev/loop0
mkdir  -m 777 tacl/mount-ext2

mount -t ext2 -o defaults,acl,user_xattr /dev/loop0 tacl/mount-ext2 
if [ $? != 0 ]
then
	echo "FAILED: [ mount ] Make sure that ACL(Access Control List) and Extended Attribute are built into the kernel"
	echo "Can not mount ext2 file system with acl and user_xattr options"
	exit 1
fi

chmod 777 tacl/mount-ext2

adduser tacluser1
adduser tacluser2
adduser tacluser3
adduser tacluser4

if [ ! -e tacl/mount-ext2/shared ]
then
	mkdir -p -m 777 tacl/mount-ext2/shared    
fi

CUR_PATH=`pwd`
echo "$CUR_PATH"

su - tacluser1 << TACL_USER1

	mkdir $CUR_PATH/tacl/mount-ext2/shared/team1
	touch $CUR_PATH/tacl/mount-ext2/shared/team1/file1
	
	cd $CUR_PATH/tacl/mount-ext2/shared/team1
	ln -sf file1 symlinkfile1
	cd $CUR_PATH

	cd $CUR_PATH/tacl/mount-ext2/shared
	ln -sf team1 symlinkdir1
	cd $CUR_PATH

TACL_USER1

su - tacluser2 << TACL_USER2

	mkdir $CUR_PATH/tacl/mount-ext2/shared/team2
	touch $CUR_PATH/tacl/mount-ext2/shared/team2/file1
	
	cd $CUR_PATH/tacl/mount-ext2/shared/team2
	ln -sf file1 symlinkfile1
	cd $CUR_PATH

	cd $CUR_PATH/tacl/mount-ext2/shared
	ln -sf team2 symlinkdir2
	cd $CUR_PATH
	
TACL_USER2

#############################################################################################
#
#  The permissions bit limit user's act
#  lrwxrwxrwx    1 tacluser1 tacluser1        5 10月 21 09:13 symlinkdir1 -> team1
#  lrwxrwxrwx    1 tacluser2 tacluser2        5 10月 21 09:13 symlinkdir2 -> team2
#  dr-x------    2 tacluser1 tacluser1     1024 10月 21 09:13 team1
#  drwxr-xr-x    2 tacluser2 tacluser2     1024 10月 21 09:13 team2
#
#############################################################################################

chmod 500 tacl/mount-ext2/shared/team1

su - tacluser1 << TACL_USER1

	touch $CUR_PATH/tacl/mount-ext2/shared/team1/newfil1 2> /dev/null
	if [ -e $CUR_PATH/tacl/mount-ext2/shared/team1/newfile1 ]
	then
		echo "FAILED: [ touch ] Create file must be denied by file permission bits [ Physical Directory ]"
	else
		echo "SUCCESS: Create file denied by file permission bits [ Physical directory ]"
	fi
	
	touch $CUR_PATH/tacl/mount-ext2/shared/symlinkdir1/newfil2 2> /dev/null
	if [ -e $CUR_PATH/tacl/mount-ext2/shared/team1/newfile1 ]
	then
		echo "FAILED: [ touch ] Create file must be denied by file permission bits [ Symlink Directory ]"
	else
		echo "SUCCESS: Create file denied by file permission bits [ Symlink directory ]"
	fi
	
TACL_USER1

#################################################################
#
# ACL_USER_OBJ are a superset of the permissions specified 
#   by the file permission bits. 
# The effective user ID of the process matches the user ID of 
#   the file object owner.
# Owner's act are based ACL_USER_OBJ
# 
#################################################################

setfacl -m u::rx tacl/mount-ext2/shared/team1
su - tacluser1 << TACL_USER1

	cd $CUR_PATH/tacl/mount-ext2/shared/team1/ 2> /dev/null
	if [ $? != 0 ]
	then
		echo "FAILED: [ touch ] ACL_USER_OBJ  entry already contains the owner execute permissions, but operation failed [ Physical Directory ]"
	else
		echo "SUCCESS: ACL_USER_OBJ  entry contains the owner execute permissions, operation success [ Physical Directory ]"
	fi

	cd $CUR_PATH/tacl/mount-ext2/shared/symlinkdir1/ 2> /dev/null
	if [ $? != 0 ]
	then
		echo "FAILED: [ touch ] ACL_USER_OBJ  entry already contains the owner execute permissions, but operation failed [ Symlink Directory ]"
	else
		echo "SUCCESS: ACL_USER_OBJ  entry contains the owner execute permissions, operation success [ Symlink Directory ]"
	fi

TACL_USER1

setfacl -m u::rwx tacl/mount-ext2/shared/team1

su - tacluser1 << TACL_USER1

	touch $CUR_PATH/tacl/mount-ext2/shared/team1/newfil1 2> /dev/null
	if [ -e $CUR_PATH/tacl/mount-ext2/shared/team1/newfile1 ]
	then
		echo "FAILED: [ touch ] ACL_USER_OBJ  entry already contains the owner write permissions, but operation failed [ Physical Directory ]"
	else
		echo "SUCCESS: ACL_USER_OBJ  entry contains the owner write permissions, operation success [ Physical Directory ]"
	fi

	touch $CUR_PATH/tacl/mount-ext2/shared/symlinkdir1/newfil2 2> /dev/null
	if [ -e $CUR_PATH/tacl/mount-ext2/shared/team1/newfile2 ]
	then
		echo "FAILED: [ touch ] ACL_USER_OBJ  entry already contains the owner write permissions, but operation failed [ Symlink Directory ]"
	else
		echo "SUCCESS: ACL_USER_OBJ  entry contains the owner write permissions, operation success [ Symlink Directory ]"
	fi

TACL_USER1

#################################################################
#
# The effective user ID of the process matches the qualifier of 
#   any entry of type ACL_USER
# IF  the  matching  ACL_USER entry and the ACL_MASK 
#   entry contain the requested permissions,#  access is granted, 
#  ELSE access is denied.
#
#################################################################

setfacl -m u:tacluser3:rwx tacl/mount-ext2/shared/team1

su - tacluser3 << TACL_USER3

	touch $CUR_PATH/tacl/mount-ext2/shared/team1/newfile3 2> /dev/null
	if [ -e $CUR_PATH/tacl/mount-ext2/shared/team1/newfile3 ]
	then
		echo "SUCCESS: ACL_USER entry contains the user permissions , operation success [ Physcial Directory ]"
	else
		echo "FAILED: ACL_USER entry contains the user permissions , but operation denied [ Physcial Directory ]"
	fi
	
	touch $CUR_PATH/tacl/mount-ext2/shared/symlinkdir1/newfile4 2> /dev/null
	if [ -e $CUR_PATH/tacl/mount-ext2/shared/symlinkdir1/newfile4 ]
	then
		echo "SUCCESS: ACL_USER entry contains the user permissions , operation success [ Symlink Directory ]"
	else
		echo "FAILED: ACL_USER entry contains the user permissions , but operation denied [ Symlink Directory ]"
	fi

TACL_USER3

setfacl -m mask:--- tacl/mount-ext2/shared/team1

su - tacluser3 << TACL_USER3

	touch $CUR_PATH/tacl/mount-ext2/shared/team1/newfile5 2> /dev/null
	if [ -e $CUR_PATH/tacl/mount-ext2/shared/team1/newfile5 ]
	then
		echo "FAILED: [ touch ]ACL_USER entry contains the user permissions buf ACL_MASK are set --- , operation must be denied [ Physcial Directory ]"
	else
		echo "SUCCESS: ACL_USER entry contains the user permissions , but ACL_MASK are set ___ , operation success [ Physcial Directory ]"
	fi
	
	touch $CUR_PATH/tacl/mount-ext2/shared/symlinkdir1/newfile6 2> /dev/null
	if [ -e $CUR_PATH/tacl/mount-ext2/shared/symlinkdir1/newfile6 ]
	then
		echo "FAILED: [ touch ]ACL_USER entry contains the user permissions buf ACL_MASK are set --- , operation must be denied [ Symlink Directory ]"
	else
		echo "SUCCESS: ACL_USER entry contains the user permissions , but ACL_MASK are set ___ , operation success [ Symlink Directory ]"
	fi

TACL_USER3

###########################################################################################
#
# The effective group ID or any of the supplementary group IDs of the process match the 
#  qualifier of the entry of type ACL_GROUP_OBJ, or the qualifier of any entry of type 
#  ACL_GROUP
#
# IF the ACL contains an ACL_MASK entry, THEN                                                                                                              
#                 if  the ACL_MASK entry and any of the matching ACL_GROUP_OBJ
#                 or ACL_GROUP  entries  contain  the  requested  permissions,
#                 access is granted,
#                                                                                                               
#                 else access is denied.
#                                                                                                               
# ELSE  (note  that  there  can be no ACL_GROUP entries without an ACL_MASK entry)                                                                                                               
#                 if the ACL_GROUP_OBJ entry contains  the  requested  permis-
#                 sions, access is granted,
#                                                                                                              
#                 else access is denied.
#
###########################################################################################

setfacl -m g:tacluser2:rwx tacl/mount-ext2/shared/team1

su - tacluser2 << TACL_USER2
	touch $CUR_PATH/tacl/mount-ext2/shared/team1/newfile7 2> /dev/null
	if [ -e $CUR_PATH/tacl/mount-ext2/shared/team1/newfile7 ]
	then
		echo "SUCCESS: ACL_GROUP entry contains the group premissions , option success [ Physcial Directory ]"
	else
		echo "FAILED: [ touch ] ACL_GROUP entry already contains the group premissions , but option success [ Physcial Directory ]"
	fi
	
	touch $CUR_PATH/tacl/mount-ext2/shared/symlinkdir1/newfile8 2> /dev/null
	if [ -e $CUR_PATH/tacl/mount-ext2/shared/symlinkdir1/newfile8 ]
	then
		echo "SUCCESS: ACL_GROUP entry contains the group premissions , option success [ Symlink Directory ]"
	else
		echo "FAILED: [ touch ] ACL_GROUP entry already contains the group premissions , but option success [ Symlink Directory ]"
	fi

TACL_USER2

setfacl -m mask:--- tacl/mount-ext2/shared/team1

su - tacluser2 << TACL_USER2
	touch $CUR_PATH/tacl/mount-ext2/shared/team1/newfile9 2> /dev/null
	if [ -e $CUR_PATH/tacl/mount-ext2/shared/team1/newfile9 ]
	then
		echo "FAILED: [ touch ] ACL_GROUP entry contains the group premissions and ACL_MASK entry are set ---, option must no be success [ Physical Directory ]"
	else
		echo "SUCCESS: ACL_GROUP entry already contains the group premissions and ACL_MASK entry are set ---, option success [ Physcial Directory ]"
	fi
	
	touch $CUR_PATH/tacl/mount-ext2/shared/symlinkdir1/newfile10 2> /dev/null
	if [ -e $CUR_PATH/tacl/mount-ext2/shared/symlinkdir1/newfile10 ]
	then
		echo "FAILED: [ touch ] ACL_GROUP entry contains the group premissions and ACL_MASK entry are set ---, option must no be success [ Symlink Directory ]"
	else
		echo "SUCCESS: ACL_GROUP entry already contains the group premissions and ACL_MASK entry are set ---, option success [ Symlink Directory ]"
	fi

TACL_USER2

setfacl -m g::rwx tacl/mount-ext2/shared/team1
usermod -g tacluser1 tacluser2

su - tacluser2 << TACL_USER2

	touch $CUR_PATH/tacl/mount-ext2/shared/team1/newfile11 2> /dev/null
	if [ -e $CUR_PATH/tacl/mount-ext2/shared/team1/newfile11 ]
	then
		echo "SUCCESS: ACL_GROUP_OBJ entry contains the group owner permission , option success [ Physcial Directory ]"
	else
		echo "FAILED: [ touch ] ACL_GROUP_OBJ entry already contains the group owner , but option denied [ Physcial Directory ]"
	fi
	
	touch $CUR_PATH/tacl/mount-ext2/shared/symlinkdir1/newfile12 2> /dev/null
	if [ -e $CUR_PATH/tacl/mount-ext2/shared/symlinkdir1/newfile12 ]
	then
		echo "SUCCESS: ACL_GROUP_OBJ entry contains the group owner permission , option success [ Symlink Directory ]"
	else
		echo "FAILED: [ touch ] ACL_GROUP_OBJ entry already contains the group owner , but option denied [ Symlink Directory ]"
	fi
TACL_USER2

setfacl -m mask:--- tacl/mount-ext2/shared/team1

su - tacluser2 << TACL_USER2
	touch $CUR_PATH/tacl/mount-ext2/shared/team1/newfile13 2> /dev/null
	if [ -e $CUR_PATH/tacl/mount-ext2/shared/team1/newfile13 ]
	then
		echo "FAILED: [ touch ] ACL_GROUP_OBJ entry contains the group owner premissions and ACL_MASK entry are set ---, option must no be success [ Physical Directory ]"
	else
		echo "SUCCESS: ACL_GROUP_OBJ entry already contains the group owner premissions and ACL_MASK entry are set ---, option success [ Physcial Directory ]"
	fi
	
	touch $CUR_PATH/tacl/mount-ext2/shared/symlinkdir1/newfile14 2> /dev/null
	if [ -e $CUR_PATH/tacl/mount-ext2/shared/symlinkdir1/newfile14 ]
	then
		echo "FAILED: [ touch ] ACL_GROUP_OBJ entry contains the group owner premissions and ACL_MASK entry are set ---, option must no be success [ Symlink Directory ]"
	else
		echo "SUCCESS: ACL_GROUP_OBJ entry already contains the group owner premissions and ACL_MASK entry are set ---, option success [ Symlink Directory ]"
	fi

TACL_USER2

usermod -g tacluser2 tacluser2

###################################################################################
#
# IF the ACL_OTHER entry contains the requested permissions, access is granted
#
###################################################################################

setfacl -m o::rwx tacl/mount-ext2/shared/team1

su - tacluser4 << TACL_USER4

	touch $CUR_PATH/tacl/mount-ext2/shared/team1/newfile15 2> /dev/null
	if [ -e $CUR_PATH/tacl/mount-ext2/shared/team1/newfile15 ]
	then
		echo "SUCCESS: ACL_OTHER entry contains the user permissions , operation success [ Physcial Directory ]"
	else
		echo "FAILED: ACL_OTHER entry contains the user permissions , but operation denied [ Physcial Directory ]"
	fi
	
	touch $CUR_PATH/tacl/mount-ext2/shared/symlinkdir1/newfile16 2> /dev/null
	if [ -e $CUR_PATH/tacl/mount-ext2/shared/symlinkdir1/newfile16 ]
	then
		echo "SUCCESS: ACL_OTHER entry contains the user permissions , operation success [ Symlink Directory ]"
	else
		echo "FAILED: ACL_OTHER entry contains the user permissions , but operation denied [ Symlink Directory ]"
	fi

TACL_USER4

setfacl -m mask:--- tacl/mount-ext2/shared/team1

su - tacluser4 << TACL_USER4

	touch $CUR_PATH/tacl/mount-ext2/shared/team1/newfile17 2> /dev/null
	if [ -e $CUR_PATH/tacl/mount-ext2/shared/team1/newfile17 ]
	then
		echo "SUCCESS: [ touch ]ACL_OTHER do not strick by ACL_MASK [ Physcial Directory ]"
	else
		echo "FAILED: ACL_OTHER do not strick by ACL_MASK [ Physcial Directory ]"
	fi
	
	touch $CUR_PATH/tacl/mount-ext2/shared/symlinkdir1/newfile18 2> /dev/null
	if [ -e $CUR_PATH/tacl/mount-ext2/shared/symlinkdir1/newfile18 ]
	then
		echo "SUCCESS: [ touch ]ACL_OTHER do not strick by ACL_MASK [ Symlink Directory ]"
	else
		echo "FAILED: ACL_OTHER do not strick by ACL_MASK [ Symlink Directory ]"
	fi

TACL_USER4

############################################################################
#
# OBJECT CREATION AND DEFAULT ACLs
# The new object inherits the default ACL of the containing directory as its access ACL.
#
############################################################################

rm -f tacl/mount-ext2/shared/team1/newfil*

#
# Test ACL_USER_OBJ default ACLs
#
setfacl -m d:u::r -m d:g::r -m d:o::r tacl/mount-ext2/shared/team1

su - tacluser1 << TACL_USER1
	
	MASK=`umask`
	umask 0
	touch $CUR_PATH/tacl/mount-ext2/shared/team1/newfile1
	umask $MASK > /dev/null
	
TACL_USER1

CONTENT=""
CONTENT=`ls -l tacl/mount-ext2/shared/team1/newfile1`
RES=`echo $CONTENT | grep ".r--r--r--" | awk '{print $1}'`
#echo $RES
if [ $RES != "" ]
then
	echo "SUCCESS: With default ACLs set , new file permission set correct."
else
	echo "FAILED: With default ACLs set , new file permission set not correct"
fi



#
# Test ACL_USER and ACL_GROUP defaults ACLs
#
setfacl -m d:u:tacluser3:rw -m d:g:tacluser3:rw tacl/mount-ext2/shared/team1
su - tacluser3 << TACL_USER3
	
	MASK=`umask`
	umask 0
	touch $CUR_PATH/tacl/mount-ext2/shared/team1/newfile2
	umask $MASK > /dev/null
	
TACL_USER3

CONTENT=""
CONTENT=`ls -l tacl/mount-ext2/shared/team1/newfile2`
RES=`echo $CONTENT | grep ".r--rw-r--" | awk '{print $1}'`
#echo $RES
if [ $RES != "" ]
then
	echo "SUCCESS: With default ACLs set , new file permission set correct."
else
	echo "FAILED: With default ACLs set , new file permission set not correct"
fi

#
# Test ACL_GROUP default ACLs
#

setfacl -m d:u::rwx -m d:g::rwx -m d:o::rwx tacl/mount-ext2/shared/team1
su - tacluser3 << TACL_USER3
	
	MASK=`umask`
	umask 0
	touch $CUR_PATH/tacl/mount-ext2/shared/team1/newfile3
	umask $MASK > /dev/null
	
TACL_USER3

CONTENT=""
CONTENT=`ls -l tacl/mount-ext2/shared/team1/newfile3`
RES=`echo $CONTENT | grep ".rw-rw-rw-" | awk '{print \$1}'`

if [ $RES != "" ]
then
	echo "SUCCESS: With default ACLs set , new file permission set correct."
else
	echo "FAILED: With default ACLs set , new file permission set not correct"
fi


#################################################################################
#
# Chmod also change ACL_USER_OBJ ACL_GROUP_OBJ and ACL_OTHER permissions
#
#################################################################################
su - tacluser3 << TACL_USER3
	MASK=`umask`
	umask 0
	
	chmod 777 $CUR_PATH/tacl/mount-ext2/shared/team1/newfile3
	umask $MASK > /dev/null
TACL_USER3

CONTENT=""
CONTENT=`getfacl tacl/mount-ext2/shared/team1/newfile3`

USER_PERMISSION=`echo $CONTENT | awk '{print \$10}'`

GROUP_PERMISSION=`echo $CONTENT | awk '{print \$12}'`
OTHER_PERMISSION=`echo $CONTENT | awk '{print \$15}'`

if [ $USER_PERMISSION == "user::rwx" ]
then
	if [ $GROUP_PERMISSION == "group::rwx" ]
	then
		if [ $OTHER_PERMISSION == "other::rwx" ]
		then 
			echo "SUCCESS: Chmod with ACL_USER_OBJ ACL_GROUP_OBJ and ACL_OTHER are correct"
		else
			echo "FAILED: Chmod with ACL_USER_OBJ ACL_GROUP_OBJ and ACL_OTHER are not correct"
		fi
	else
		echo "FAILED: Chmod with ACL_USER_OBJ ACL_GROUP_OBJ and ACL_OTHER are not correct"
	fi
else
	echo "FAILED: Chmod with ACL_USER_OBJ ACL_GROUP_OBJ and ACL_OTHER are not correct"
fi


#####################################################################################
#
# Chown only change object owner and group
#
#####################################################################################

chown tacluser2.tacluser2 tacl/mount-ext2/shared/team1/newfile2
CONTENT=""
CONTENT=`getfacl tacl/mount-ext2/shared/team1/newfile2`

ITEM_OWNER=`echo $CONTENT | awk '{print \$6}'`
ITEM_GROUP=`echo $CONTENT | awk '{print \$9}'`

if [ $ITEM_OWNER == "tacluser2" ]
then 
	if [ $ITEM_GROUP == "tacluser2" ]
	then
		echo "SUCCESS: Chown correct"
	else
		echo "FAILED: Chown are not correct"
	fi
else
	echo "FAILED: Chown are not correct"
fi

#####################################################
#
# Test ACLs backup and restore
#
#####################################################

getfacl -RL tacl/mount-ext2/ > tacl/tmp1
setfacl -m u::--- -m g::--- -m o::--- tacl/mount-ext2/shared/team1
setfacl --restore tacl/tmp1
getfacl -RL tacl/mount-ext2/ > tacl/tmp2

if [ `diff tacl/tmp1 tacl/tmp2` ]
then 
	echo "FAILED: ACLs backup and restore are not correct"
else
	echo "SUCCESS: ACLs backup and restore are correct"
fi

echo "End ACLs Test"

#####################################################
#
# Now begin Extend Attribute test
#
#####################################################

echo
echo "Now begin Extend Attribute Test"
echo


# dir
attr -s attrname1 -V attrvalue1 tacl/mount-ext2/shared/team2

#file
attr -s attrname2 -V attrvalue2 tacl/mount-ext2/shared/team2/file1

#symlink file
attr -s attrname3 -V attrvalue3 tacl/mount-ext2/shared/team2/symlinkfile1
#setfattr -hn attrname4 -v attrvalue4 tacl/mount-ext2/shared/team2/symlinkfile1
#setfattr -n attrname5 -v attrvalue5 tacl/mount-ext2/shared/team2/symlinkfile1

getfattr -d tacl/mount-ext2/*
getfattr -dR tacl/mount-ext2/*
getfattr -h tacl/mount-ext2/shared/team2/symlinkfile1
getfattr -L tacl/mount-ext2/*
getfattr -P tacl/mount-ext2/*

attr -g attrname1 tacl/mount-ext2/shared/team2
attr -r attrname2 tacl/mount-ext2/shared/team2/file1


#################################
#
# Backup and Restore
#
#################################
getfattr -dhR -m- -e hex tacl/mount-ext2 > backup.ea
setfattr -h --restore=backup.ea




#####################################################
#
# Clean up 
#
#####################################################

userdel tacluser1
userdel tacluser2
userdel tacluser3
userdel tacluser4
umount -d tacl/mount-ext2
rm -rf tacl
