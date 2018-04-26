#!/usr/bin/env python3
# This script aims to help to run locktests with several clients.
#
# Report bugs to Vincent ROQUETA : vincent.roqueta@ext.bull.net

import encodings
import shutil
import os, sys
import getopt, sys
import string
import socket
from stat import *
from sys import *
from os import *

NFS4_PATH="/mnt/nfsv4"
NFS4_SERVER=""
TEST_HOME="/home/vincent/locks/"
testfile=NFS4_PATH+"/testfile"

app="locktests"
SRC="locktests.tar.gz"
SRC_PATH="deploy"
install="'tar xzf "+SRC+"; cd locks;  make `"
user="root"





class Machine:

    def mkdir(self,dir):
        self.command="mkdir -p "+dir
        self.do()
    def rmdir(self,dir):
        self.command="rm -rf "+dir
        self.do()

    def printc(self):
        print("->"+self.command)
        print("\n")

class Client(Machine):

    def __init__(self, machine):
        self.command=""
        self.machine=machine
        self.mountPath=NFS4_PATH

    def do(self):
        self.command="ssh "+user+"@"+self.machine+" "+self.command
        os.system(self.command)

    def isomount(self, dir):
        export=NFS4_SERVER
        mntpoint=NFS4_PATH
        self.command="'mkdir -p "+mntpoint+"; mount -t nfs4 "+export+" "+mntpoint+"'"
        self.do()

    def umount(self, dir):
        mntpoint=self.mountPath+"/"+dir
        self.command="umount "+mntpoint
        self.do()
    def install(self, path):
        self.command="'cd "+path+"; tar xzf "+SRC+"; cd locks;  make'"
        self.do()

    def run(self, appli):
        self.command=appli
        self.do()
    def cp(self, fichier, path):
        command="scp "+fichier+" "+user+"@"+self.machine+":"+path
        os.system(command)


class Serveur(Machine):

    def __init__(self, ip, exportPath):
        self.SERVEUR=ip
        self.exportPath=exportPath

    def do(self):
        self.command="ssh "+self.SERVEUR+" "+self.command
        os.system(self.command)

    def configure(self, dir):
        exportDir=self.exportPath+'/'+dir
        self. mkdir(exportDir)
#self.printc()
        self.export(exportDir)
#self.printc()
    def clean(self, dir):
        unexportDir=self.exportPath+'/'+dir
        self.unexport(unexportDir)
        self.rmdir(unexportDir)
def usage():
		print("\n")
		print("usage:")
		print("locktests.py <-n process -f testfile ><--setup -s fs_server> -c host1, host2, host3 ... ")
		print("--setup : setup the configuration, deploy test on other test machines; This option also requires -c and -s")
		print("-c <machine>     : host list to deploy/run/clean the test")
		print("-s <machine>     : NFS server to use to setup the test")
		print("-n <num>         : number of processes each test machine will lauch to perform the test")
		print("-f <file>        : test file. This must be the same on each machine")
		print(" ")
		print("Example :")
		print("=========")
		print("*Setup machines for testing")
		print("./locktests.py --setup -c testmachine1 testmachine2 testmachine3 -s my_nfs_server:/")
		print("\n")
		print("*Run test on testmachine1,testmachine2 with 50 process on each machine using /mnt/nfsv4/testfile")
		print("./locktests.py -n 50 -f /mnt/nfsv4/testfile -c testmachine1 testmachine2")
		print("\n")
		print("_________________________________")
		print("Vincent ROQUETA - Bull SA - 2005\n")

		return 0



def setup():
    path=os.path.abspath(".")
    fichier=SRC_PATH+"/"+SRC
    commande=""
    for i in clients:
        print("Setting up machine "+i)
        c=Client(i)
        c.mkdir(path)
        c.cp(fichier, path)
        c.install(path)
        c.isomount(NFS4_PATH)
    #Setup localhost
    print("Setting up localhost")
    commande="make; mkdir -p "+NFS4_PATH+" ; mount -t nfs4 "+NFS4_SERVER+" "+NFS4_PATH+" &"
    os.system(commande)


def run():
    path=os.path.abspath(".")
    nbreClients=len(clients)
    hostname=socket.gethostname()
    # Lancement du serveur en local
    # Launch the server locally
    commande=path+"/"+app+" -n "+nbreProcess+" -f "+filename+" -c "+str(nbreClients)+" &"
    os.system(commande)
    commande=path+"/locks/"+app+" --server "+hostname
    for i in clients:
        c=Client(i)
        c.run(commande)

def clean():
   for i in clients:
    client.umount(NFS4_PATH)






args=sys.argv[1:]
rge=list(range(len(args)))
a=""
r=True
s=False
nfsServer=False
c=False
f=False
n=False
clients=[]
for i in rge:
    if args[i] in ("--install", "-i", "--setup"):
        r=False
        s=True
        continue
    if args[i] in ("-s", "--server"):
        a="nfsServer"
        nfsServer=True
        continue
    if args[i] in ("-h", "--help"):
        usage()
        sys.exit(1)
    if args[i] in ("--clients", "-c"):
        a="clients"
        c=True
        continue
    if args[i] == "-n":
        a="nbre"
        n=True
        continue
    if args[i] == "-f":
        a="file"
        f=True
        continue

    if a=="clients":
       clients.append(args[i])
       continue
    if a=="file":
       filename=args[i]
       continue
    if a=="nbre":
       nbreProcess=args[i]
       continue
    if a=="nfsServer":
        NFS4_SERVER=args[i]
        continue


    usage()
# For ...
if s:
    if (not c) or (not nfsServer):
        usage()
        sys.exit(1)
    print("Setup")
    print(NFS4_SERVER)
    setup()
    print("Setup complete")

if r:
    if (not c) or (not f) or (not n):
        usage()
        sys.exit(1)

    print("Running test")
    run()









