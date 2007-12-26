from scripts.parser import *
import re
class SchedFootballLog(Log):
	def __init__(self,filename):
		Log.__init__(self,filename)

	def eval(self):
		exp1= "Final ball position"
		flag=False
		for line in self.read():
			if exp1 in line:
				list=line.split(" ")
				if(list[3]=="0"):
					return True
				else:
					return False



def main():
	if len(sys.argv) < 2:
		sys.exit("Usage : ./%s <logname>" % sys.argv[0])
	else:
		log_file=sys.argv[1]

	log = SchedFootballLog(log_file)
	sys.exit("Result: %s " % (["FAIL", "PASS"][log.eval()]))

if __name__ == "__main__":
    main()


