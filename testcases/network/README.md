# LTP Network Tests

## Pre-requisites
Enable all the networking services on test machine(s): rshd, nfsd, fingerd.

## Single Host Configuration

It is a default configuration ('RHOST' is not defined). LTP adds 'ltp_ns'
network namespace and auto-configure 'veth' pair according to LTP network
environment variables.

## Two Host Configuration

This setup requires 'RHOST' environment variable to be set properly and
configured SSH or RSH (default) access to a remote host.

The 'RHOST' variable name must be set to the hostname of the server
(test management link) and PASSWD should be set to the root password
of the remote server.

In order to have RSH access:
* Edit the "/root/.rhosts" file. Please note that the file may not exist,
so you must create one if it does not. You must add the fully qualified
hostname of the machine you are testing on to this file. By adding the test
machine's hostname to this file, you will be allowing the machine to rsh to itself,
as root, without the requirement of a password.

```sh
echo $client_hostname >> /root/.rhosts
```

You may need to re-label '.rhost' file to make sure rlogind will have access to it:

```sh
/sbin/restorecon -v /root/.rhosts
```

* Add rlogin, rsh, rexec into /etc/securetty file:

```sh
for i in rlogin rsh rexec; do echo $i >> /etc/securetty; done
```

## Server Services Configuration
Verify that the below daemon services are running. If not, please install
and start them:
rsh-server, telnet-server, finger-server, rdist, rsync, dhcp-server, http-server.

Note: If any of the above daemon is not running on server, the test related to
that service running from client will fail.

### FTP setup
* In “/etc/ftpusers” [or vi /etc/vsftpd.ftpusers], comment the line containing
“root” string. This file lists all those users who are not given access to do ftp
on the current system.

* If you don’t want to do the previous step, put following entry into /root/.netrc
machine <remote_server_name> login root password <remote_root_password>.
Otherwise, ftp,rlogin & telnet fails for ‘root’ user & hence needs to be
executed using ‘test’ user to get successful results.

## LTP setup
Install LTP testsuite. In case of two hosts configuration, make sure LTP is installed
on both client and server machines.

Testcases and network tools must be in PATH, e.g.:

```sh
export PATH=/opt/ltp/testcases/bin:/usr/bin:$PATH
```
Default values for all LTP network variables are set in testcases/lib/tst_net.sh.
If you need to override some parameters please export them before test run or
specify them when running ltp-pan or testscripts/network.sh.

## Running the tests
To run the test type the following:

```sh
TEST_VARS ./network.sh OPTIONS
```
Where
* TEST_VARS - non-default network parameters (see testcases/lib/tst_net.sh), they
  could be exported before test run;
* OPTIONS - test group(s), use '-h' to see available ones.

## Analyzing the results
Generally this test must be run more than 24 hours. When you want to stop the test
press CTRL+C to stop ./network.sh.

Search failed tests in LTP logfile using grep FAIL <logfile>. For any failures,
run the individual tests and then try to come to the conclusion.
