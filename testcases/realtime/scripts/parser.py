#!/usr/bin/env python3
# -*- coding: utf-8 -*-

################################################################################
##                                                                            ##
## Copyright ©  International Business Machines  Corp., 2007, 2008            ##
##                                                                            ##
## This program is free software;  you can redistribute it and#or modify      ##
## it under the terms of the GNU General Public License as published by       ##
## the Free Software Foundation; either version 2 of the License, or          ##
## (at your option) any later version.                                        ##
##                                                                            ##
## This program is distributed in the hope that it will be useful, but        ##
## WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY ##
## or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License   ##
## for more details.                                                          ##
##                                                                            ##
## You should have received a copy of the GNU General Public License          ##
## along with this program;  if not, write to the Free Software               ##
## Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA    ##
##                                                                            ##
## NAME: parser.py                                                            ##
##                                                                            ##
## DESCRIPTION: Base class for all log parsers                                ##
##                                                                            ##
## AUTHOR: Chirag <chirag@linux.vnet.ibm.com                                  ##
##                                                                            ##
################################################################################

import sys

class Log:
	def __init__(self,filename):
		if filename:
			log_file=filename
	try:
		self.__log_file = open(log_file, "r")
	except IOError as errmsg:
		sys.exit(errmsg)

	def read(self):
		for line in self.__log_file.read().split("\n"):
			yield line
	self.__log_file.close()

	def eval(self):
		pass
