#!/usr/bin/env python3
import subprocess
import random
import re

alphabet = 'azertyuiopqsdfghjklmwxcvbnAZERTYUIOPQSDFGHJKLMWXCVBN123456789-_'
a_length = len(alphabet)

""" ACL support attribute """
ACL4_SUPPORT_ALLOW_ACL = 0x00000001
ACL4_SUPPORT_DENY_ACL = 0x00000002
ACL4_SUPPORT_AUDIT_ACL = 0x00000004
ACL4_SUPPORT_ALARM_ACL = 0x00000008

class RandomGen(object):


	"""  List of ACE possible who fields """
	ace_who=["OWNER@","GROUP@","EVERYONE@","ANONYMOUS@","AUTHENTICATED@"]

	""" List of GID than can be used to do the tests """
	gList=[]
	gListSize = len(gList)
	uList = []
	uListSize = len(uList)

	fList=[]
	fListSize = len(fList)

	""" Create a user in available groups to do the tests """
	def createUser(self,username):
		group = self.gList[random.randint(0,len(self.gList)-1)][0]
		opts = "-g" + group + " -p" + "1pilot" + " -m " + username
		u = subprocess.getoutput('/usr/sbin/useradd '+ opts)
		if u != "":
			print("create user " + username + "failed" + u)

	def createFile(self,path,n):
		for i in range(n):
			fName = 'file' + str(i)
			u = subprocess.getoutput('touch ' + path + '/'+ fName)
			self.fList.append(fName)

	def createGroup(self, grpname, gid):
		u = subprocess.getoutput('/usr/sbin/groupadd -g' + gid + " " + grpname)
		if u != "":
			print(u)

	def createNGroup(self, n):
		for i in range(n):
			gName = 'grp' + str(i)
			gid = str(500+i)
			self.createGroup(gName, gid)


	""" Random creation of n user """
	def createNUser(self,n):
		for i in range(n):
			userName= "user" + str(i)
			self.createUser(userName)

	""" clean all users created to do the tests """
	def cleanUsers(self):
		for name in self.uList:
			u = subprocess.getoutput('/usr/sbin/userdel -r '+ name)
		self.uList = []

	""" clean all users created to do the tests """
	def cleanGroups(self):
		for name in self.gList:
			u = subprocess.getoutput('/usr/sbin/groupdel '+ name[0])
		self.gList = []

	""" Retrieve the list of user from /etc/passwd file """
	def getUserList(self):
		f = open('/etc/passwd','r')
		lines = f.readlines()
		for line in lines:
			splitedline = line.split(':')
			userName = splitedline[0]
			gid = splitedline[3]
		# TO FIX: verify that the group is OK (in the right range)
			NameOK = re.match("user",userName)
			# We keep only usernames starting with "user"
			if NameOK != None:
				self.uList.append(userName)
		f.close()

	def getFileList(self,path):
		u = subprocess.getoutput('ls ' + path)
		tmp = u.split('\n')
		for i in range (len(tmp)-1):
			NameOK = re.match("file",tmp[i])
			if NameOK != None:
				self.fList.append(tmp[i])

	def getNUserList(self,nb):
		f = open('/etc/passwd','r')
		lines = f.readlines()
		n = 0
		for line in lines:
			splitedline = line.split(':');
			userName = splitedline[0]
			gid = splitedline[3]
		# TO FIX: verify that the group is OK (in the right range)
			NameOK = re.match("user",userName)
			# We keep only usernames starting with "user"
			if NameOK != None:
				self.uList.append(userName)
				n = n+1
			if n==nb:
				break;
		f.close()

	""" Get group list """
	def getGroupList(self):
		f = open('/etc/group','r')
		lines = f.readlines()
		for line in lines:
			splitedline = line.split(':');
			groupName = splitedline[0]
			gid = splitedline[2]
			NameOK = re.match("grp",groupName)
			if NameOK != None:
				self.gList.append([groupName,gid])
		f.close()

	""" Get a list of n group """
	def getNGroupList(self,nb):
		f = open('/etc/group','r')
		lines = f.readlines()
		n = 0
		for line in lines:
			splitedline = line.split(':');
			groupName = splitedline[0]
			gid = splitedline[2]
			NameOK = re.match("grp",groupName)
			if NameOK != None:
				self.gList.append([groupName,gid])
				n = n+1
			if n==nb:
				break;
		f.close()

	def printUserList(self):
		print(self.uList)

	def printGroupList(self):
		print(self.gList)

	""" Create a random name of random length """
	def createOneNameRandomLength(self,maxlength):
		outputString =""
		l=random.randint(0,maxlength)
		for i in range(l):
			a = random.randint(0,a_length-1)
			outputString =outputString  + alphabet[a]
		return outputString

	""" Create a random name of fixed length """
	def createOneName(self,lenght):
		outputString =""
		for i in range(length):
			a = random.randint(0,a_length-1)
			outputString = outputString + alphabet[a]
		return outputString

	""" Create Random User List with fixed length user names """
	def createRandomUserList(self,listlength,usernamelength):
		userlist = []
		for i in range(listlength):
			user = createOneName(lenght)
			userlist.append(user)
		return userlist

	""" Create Random ACE for a file and a given usr """
	def createRandomACE(self,user):
		type = ace_type[random.randint(0,len(ace_type))]
		flag = ace_flags[random.randint(0,len(ace_flags))]
		mask = ace_mask[random.randint(0,len(ace_mask))]
		who = ace_who[random.randint(0,len(ace_who))]
		return nfsace4(type, flag, mask, who)

	""" Create Random ACL for a file with a fixed number a entries """
	def createRandomACL(self,acl_size):
		acl = []
		userList = uList
		userListSize = uListSize
		for i in range(acl_size):
			n = random.randint(0,userListSize-1)
			usr = userList.pop(n)
			newace = createRandomACE(usr)
			acl.append(newace)
		return acl

	""" Return a mode string like 'xwr' or 'x' """
	def createRandomMode(self):
		out_str = ""
		while (out_str == ""):
				if random.randint(0,1) == 1:
					out_str += 'x'
				if random.randint(0,1) == 1:
					out_str += 'w'
				if random.randint(0,1) == 1:
					out_str += 'r'
		return out_str

	""" Create a random ACL operation (delete / remove / modify on user / group ) """
	def randomOp(self,path):
		a = random.randint(1,4)
		mode = self.createRandomMode()
		file = self.fList[random.randint(0,len(self.fList)-1)]
		if a == 1:	# creation/modification
			user = self.uList[random.randint(0,len(self.uList)-1)]
			u = subprocess.getoutput('setfacl -m u:' + user + ':' + mode + " " + path + "/" + file)

		if a == 2:	# with group
			group = self.gList[random.randint(0,len(self.gList)-1)][0]
			u = subprocess.getoutput('setfacl -m g:' + group + ':' + mode + " " + path + "/" + file)

		if a == 3:	# deletation
			user = self.uList[random.randint(0,len(self.uList)-1)]
			u = subprocess.getoutput('setfacl -x u:' + user + " " + path + "/" + file)

		if a == 4:	# with group
			group = self.gList[random.randint(0,len(self.gList)-1)][0]
			u = subprocess.getoutput('setfacl -x g:' + group + " " + path + "/" + file)

		# request on a unexisting group
		'''if a == 5:
			group = self.createOneNameRandomLength(16)
			print 'setfacl -x g:' + group + " " + path + "/" + file
			u = commands.getoutput('setfacl -x g:' + group + " " + path + "/" + file)
		if a == 6:
			user = self.createOneNameRandomLength(16)
			u = commands.getoutput('setfacl -x u:' + user + " " + path + "/" + file)

		if a == 7:	# creation/modification
			user = self.createOneNameRandomLength(16)
			u = commands.getoutput('setfacl -m u:' + user + ':' + mode + " " + path + "/" + file)

		if a == 8:	# with group
			group = self.createOneNameRandomLength(16)
			u = commands.getoutput('setfacl -m g:' + group + ':' + mode + " " + path + "/" + file)

		if a == 9:     	#Copying the ACL of one file to another
			file2 = self.fList[random.randint(0,len(self.fList)-1)]
              		u = commands.getoutput('getfacl ' + path + "/" + file + "| setfacl --set-file=- " + path + "/" + file2)
		if u!="":
			print u'''

