import os
import socket

STARTER_TEXT = """
Example :
=========
*Setup machines for testing
./locktests.py --setup -c client1 client2 client3 -s nfs-server:/path/to/shareddir/filename

*Run test on client1,client2 with 50 process on each machine using /mnt/test/filetest.txt
./locktests.py -n 50 -f /mnt/test/filetest.txt -c client1 client2
_________________________________
Vincent ROQUETA - Bull SA - 2005.
"""
APP = "locktests"
SRC = "locktests.tar.gz"
SRC_PATH = "deploy"
HOSTNAME = socket.gethostname()
USER = None

class Machine:

	def __init__(self, machineIP, mntpoint, export):
		self.machineIP = machineIP
		self.mntpoint = mntpoint
		self.export = export

	def mkdir(self, loc):
		self.cmd = f"'mkdir -p {loc}'"
		self.do()

	def rmdir(self, loc):
		self.cmd = f"'rm -rf {loc}'"
		self.do()

	def run(self, cmd):
		self.cmd = "'{cmd}'"
		self.do()

	def do(self):
		ssh = f'ssh {USER}@{self.machineIP} {self.cmd}'
		print('->', ssh)
		os.system(ssh)


class Client(Machine):

	def __init__(self, machineIP, mntpoint, export=''):
		super().__init__(machineIP, mntpoint, export)

	def mount(self):
		self.mkdir(self.mntpoint)
		self.cmd = f"'mount -t nfs {self.export} {self.mntpoint}'"
		self.do()

	def umount(self):
		self.cmd = f"'umount {self.mntpoint}/'"
		self.do()

	def install(self):
		self.cmd = f"'tar xzf {SRC}; cd locks; make'"
		self.do()

	def cp(self, srcpath, dstpath=None):
		cmd = f"scp {srcpath} {USER}@{self.machineIP}:{dstpath}"
		print('->', cmd)
		os.system(cmd)


def setup(clients, server):
	mntpoint = '/mnt/nfsv4'
	app_path = f'{SRC_PATH}/{SRC}'
	for clientIP in clients:
		print('Setting up machine:', clientIP)
		client = Client(clientIP, mntpoint, server)
		client.mount()
		client.cp(app_path, f'~/{SRC}')
		client.install()
	print(f'{server} has be mounted to {mntpoint}')
	print("Setting up localhost")
	cmd = f"make; mkdir -p {mntpoint}; mount -t nfs {server} {mntpoint}"
	os.system(cmd)

def run_tests(loop_proc, file_path, clients):
	cmd = f'~/locks/{APP} -n {loop_proc} -f {file_path} -c {len(clients)} &'
	os.system(cmd)

	cmd = f'~/locks/{APP} --server {HOSTNAME}'
	for clientIP in clients:
		client = Client(clientIP, mntpoint)
		client.run(cmd)

def validArg(vargs, required=[], when=None):
	valid = []
	for arg in required:
		valid.append(arg in vargs and vargs[when] == True if when is not None else arg in vargs)
	return all(valid)



if __name__ == "__main__":
	import argparse

	parser = argparse.ArgumentParser(description='Stress NFSv4 locks', 
									 epilog=STARTER_TEXT, 
									 formatter_class=argparse.RawDescriptionHelpFormatter)
	parser.add_argument('--setup', action='store_true', help='Setup the NFS configuration; this option also requires -c and -s')
	parser.add_argument('-c', '--client-list', nargs='+', metavar='', help='Host list to deploy/run/clean the test')
	parser.add_argument('-s', '--server', type=str, metavar='', help='NFS server mount to use')
	parser.add_argument('-n', '--num-proc', type=str, metavar='', help='Number of processes will launch on each machine')
	parser.add_argument('-f', '--file-path', type=str, metavar='', help='Directory mount and filename path to test file')
	parser.add_argument('-u', '--user', type=str, metavar='', required=True, help='SSH User to remote')


	args = parser.parse_args()
	USER = args.user
	vargs = vars(args)

	if validArg(vargs, required=['client_list', 'server'], when='setup'):
		print('Setup...')
		setup(args.client_list, args.server)
		print('Setup complete!')
	elif validArg(vargs, required=['num_proc', 'file_path', 'client_list']):
		print('Running test...')
		run_tests(args.num_proc, args.file_path, args.client_list)
		print('Test complete!')
	else:
		parser.error(STARTER_TEXT)
