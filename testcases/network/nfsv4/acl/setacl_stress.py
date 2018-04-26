#!/usr/bin/env python3
'''
	Access Control Lists stressing script
	To lauch on the first client
	Aurelien Charbon - Bull SA
'''

from random_gen import *
from optparse import OptionParser
import subprocess
import os
import random

alphabet='abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ123456789_-()'
t_alphabet=len(alphabet)

test = RandomGen()

parser = OptionParser()
parser.set_defaults(nbfiles=5,nbusers=5,nbgroups=5,nloop=100 )
parser.add_option("-n","--nloop", dest="nloop",type="int", help="number of loop to do in the test")
parser.add_option("-p", "--path", dest="path",help="path on which the test is executed")
parser.add_option("-f", "--nbfiles", dest="nbfiles",type="int",help="nb of files to do the test (default=5)")
parser.add_option("-u", "--nbusers", dest="nbusers",type="int",help="nb of users (default=5)")
parser.add_option("-g", "--nbgrp", dest="nbgroups",type="int",help="nb of groups (default=5)")
(options, args) = parser.parse_args()

test.createFile(options.path,options.nbfiles)
test.getNUserList(options.nbusers)
test.getNGroupList(options.nbgroups)
for i in range (options.nloop):
	test.randomOp(options.path)
