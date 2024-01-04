# LTP Network Tests

## Single Host Configuration

It's the default configuration (if the `RHOST` environment variable is not
defined). LTP adds `ltp_ns` network namespace and auto-configure `veth` pair
according to LTP network environment variables.

## Two Host Configuration

This setup requires the `RHOST` environment variable to be set properly and
configured SSH access to a remote host.

The `RHOST` variable must be set to the hostname of the server (test management
link) and public key setup or login without password is required.

SSH server needs to be configured to allow root login and use Public Key
Authentication (`PermitRootLogin yes` and `PubkeyAuthentication yes` in
`/etc/ssh/sshd_config`).

Some of the network stress tests which hasn't been ported to network API were
designed to be tested with `rsh` via `LTP_RSH` environment variable. Now it's
by default used `ssh`, for details see `testcases/network/stress/README`.

## Server Services Configuration
Tests have various external dependencies, exit with `TCONF` when not installed.
Some tests require additional setup.

### FTP and telnet setup
FTP stress tests and telnet server tests require environment variables `RHOST`
(remote machine), `RUSER` (remote user) and `PASSWD` (remote password). NOTE:
`RHOST` will imply two host configuration for other tests.

If `RUSER` is set to `root`, either of these steps is required:

* In `/etc/ftpusers` (or `/etc/vsftpd.ftpusers`), comment the line containing
"root" string. This file lists all those users who are not given access to do ftp
on the current system.

* If you don’t want to do the previous step, put following entry into `/root/.netrc`:
```
machine <remote_server_name>
login root
password <remote_root_password>
```

### HTTP setup
HTTP stress tests require configured and running web server (Apache2, Nginx, etc.).

### NFS setup
NFS tests require running NFS server, enable and start `nfs-server.service`
(Debian/Ubuntu and openSUSE/SLES: `nfs-kernel-server` package, others:
`nfs-server` package).

There is no detection whether service is running, test will simply fail without
warning.

### TI-RPC / Sun RPC setup
TI-RPC (or glibc legacy Sun RPC) tests require running rpcbind (or portmap on
old distributions), enable and start `rpcbind.service`.

## LTP setup
Install LTP testsuite (see INSTALL). In case of two hosts configuration, LTP
needs to be installed the same exact location and `LTPROOT` and `PATH`
environment variables set on *both* client and server machines. This is
required because some tests expect to find server files in certain locations.

Example for the default prefix `/opt/ltp`:

```sh
export LTPROOT="/opt/ltp"; export PATH="$LTPROOT/testcases/bin:$PATH"
```

## Running the tests
The network tests are executed by running the network.sh script:

```sh
TEST_VARS ./network.sh OPTIONS
```
Where
* `TEST_VARS` - non-default network parameters
* `OPTIONS` - test group(s), use `-h` to see available ones.

Default values for all LTP network parameters are set in `testcases/lib/tst_net.sh`.
Network stress parameters are documented in `testcases/network/stress/README`.

Tests which use `tst_netload_compare()` test also performance. They can fail on
overloaded SUT.  To ignore performance failure and test only the network functionality,
set `LTP_NET_FEATURES_IGNORE_PERFORMANCE_FAILURE=1` environment variable.

## Debugging
Both single and two host configurations support debugging via
`TST_NET_RHOST_RUN_DEBUG=1` environment variable.
