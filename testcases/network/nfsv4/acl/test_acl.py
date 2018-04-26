#!/usr/bin/env python3
'''
	Access Control Lists testing based on newpynfs framework
	Aurelien Charbon - Bull SA
'''
from random_gen import *
import subprocess
import os
import threading
import time
import random

alphabet='abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ123456789_-() ~'
t_alphabet=len(alphabet)

def test_acl_default(path):

# set default acl on the test directory
	u = subprocess.getoutput('mkdir ' + path + "/" + testdir)
	u = subprocess.getoutput('getfacl ' + path + "/" + testdir)
	acl=[]
	for i in range (len(splitedresult)-1):
		splitedline = splitedresult[i].split('::')
		name = splitedline[0]
		entry = splitedline[1]
		acl.append(name,entry)
# create a file in this directory
	u = subprocess.getoutput('touch ' + path + "/" + testdir + testfile)
# get the file's ACL and verify
	u = subprocess.getoutput('getfacl ' + path + "/" + testdir + testfile)
	splitedresult = u.split('\n')
	acl2=[]
	for i in range (len(splitedresult)-1):
		splitedline = splitedresult[i].split('::')
		name = splitedline[0]
		entry = splitedline[1]
		acl2.append(name,entry)

	result_final = True
	while i < len(acl2):
		result = False
		while j < len(acl2) and result == False:
			if acl2[i] == acl[j]:
				result = True
		if result == False:
			result_final = False

''' Measuring time to add an ACE to a list regarding the number of ACE already in the list'''
''' Doing the measurement on 100 files '''
def test_acl_long():
	path = '/mnt/nfs/test-acl'
	test = RandomGen()
	test.createFile(path,100)
	test.getUserList()
	t0=time.time()
	for test_file in test.fList:
		for user in test.uList:
			mode = test.createRandomMode()
			u = subprocess.getoutput('setfacl -m u:' + user + ':' + mode + " " + path + "/" + test_file)
	t1=time.time()
	print(t1-t0)

def test_nfs_acl():
	print("test acl 10000\n")
	test = RandomGen()
	f = open('/tmp/acl-result-10000','w')
	path = '/mnt/nfs/test-acl'
	for i in range(10000):
		print("test avec " + str(i) + " ACE")
		test.getUserList()
		testfile = 'testfile' + str(i)
		u = subprocess.getoutput('touch ' + path + "/" + testfile)
		t0=time.time()
		for j in range(i):
			user = test.uList.pop()
			mode = test.createRandomMode()
			u = subprocess.getoutput('setfacl -m u:' + user + ':' + mode + " " + path + "/" + testfile)
			t1=time.time()
			f.write(str(i) + "\t" + str(t1-t0)+"\n")
			f.close()


def test_nfs_getfacl():
	# mesures sur le getfacl
	test = RandomGen()

	path = '/mnt/nfs/test-acl' # NFS mounted directory
	u = subprocess.getoutput('rm ' + path + "/*")	# clean directory
	print("test acl getfacl\n")
	f = open('/tmp/acl-result-getfacl','w')
	for i in range(37):

		test.getUserList()
		testfile = 'testfile' + str(i)

		u = subprocess.getoutput('touch ' + path + "/" + testfile)
		print("setfacl " + str(i) + " " + u)
		for j in range(i):
			user = test.uList.pop()
			mode = test.createRandomMode()
			u = subprocess.getoutput('setfacl -m u:' + user + ':' + mode + " " + path + "/" + testfile)

		t1=time.time()
		u = subprocess.getoutput('getfacl ' + path + "/" + testfile)
		print("getfacl - " + str(i) + u + "\n")
		t2=time.time()
		f.write(str(i) + "\t" + str(t2-t1)+"\n")
		f.close()


def main():
	# test getFileList
	path = '/mnt/nfs/test-acl'
	test = RandomGen()
	test.getFileList(path)
	print(test.fList)
main()





