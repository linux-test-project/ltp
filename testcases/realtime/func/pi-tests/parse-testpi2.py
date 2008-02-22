from scripts.parser import *
import re
class TestPi2(Log):
	def __init__(self,filename):
		Log.__init__(self,filename)

	def eval(self):
		exp1= re.compile("pthread pol 2 pri 10")
		exp2= re.compile(r'^Noise Thread')
		exp3=re.compile("[1-9]\d{2,3}")
		prev_line="temp"
		count=0
		flag=True
		for line in self.read():
			 if exp1.search(line) and  exp2.search(prev_line)and exp3.search(prev_line):
        	  	       list=prev_line.split(" ")
	               	       if int(list[4])<= 9900:
			       		count+=1
					flag=True
			       elif count == 0:
					return False



			 prev_line=line
		if count>=2:
			return True

def main():
	if len(sys.argv) < 2:
		sys.exit("Usage : ./%s <logname>" % sys.argv[0])
	else:
		log_file=sys.argv[1]
	log = TestPi2(log_file)
	sys.exit("Result: %s " % (["FAIL", "PASS"][log.eval()]))

if __name__ == "__main__":
    main()
