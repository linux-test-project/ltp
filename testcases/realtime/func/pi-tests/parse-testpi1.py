from scripts.parser import *
import re
class TestPi1(Log):
	def __init__(self,filename):
		Log.__init__(self,filename)

	def eval(self):
		exp1= re.compile("pthread pol 0 pri 0")
		exp2= re.compile(r'^Noise Thread')
		exp3=re.compile("[1-9]\d{2,3}")
		flag=False
		for line in self.read():
			 if exp1.search(line) and  exp2.search(prev_line)and exp3.search(prev_line):
        	  	       list=prev_line.split(" ")
	               	       if int(list[4])< 9900:
			       		flag=True
			       else:
					flag=False



			 prev_line=line
		return flag

def main():
	if len(sys.argv) < 2:
		sys.exit("Usage : ./%s <logname>" % sys.argv[0])
	else:
		log_file = sys.argv[1]
	log = TestPi1(log_file)
	sys.exit("Result: %s " % (["FAIL", "PASS"][log.eval()]))

if __name__ == "__main__":
    main()


