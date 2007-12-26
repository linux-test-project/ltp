import sys,os,string,re

class Log:
    def __init__(self,filename):
	if filename:
	    log_file=filename
	try:
	    self.__log_file = open(log_file, "r")
	except IOError, errmsg:
	    sys.exit(errmsg)

    def read(self):
	for line in self.__log_file.read().split("\n"):
	    yield line
	self.__log_file.close()

    def eval(self):
	pass
