#
# RPM Package Manager (RPM) spec file for ltp-devel
#
Summary: Linux Test Project (LTP) Software Development Kit (SDK)
Name: ltp-devel
Version: 1.0
Release: 0.0
Prefix: /opt/ltp
License: GPL
Group: Development/Libraries
URL: http://www.linuxtestproject.org
Vendor: IBM Corp
Packager: Subrata Modak <subrata.modak@@in.ibm.com>
AutoReqProv:    0
Provides:       LTP
#ExclusiveArch:  i386
ExclusiveOS:    linux
%description
This is a development package of the Linux Test Project (LTP).
It is intended to be used to build testcases using the provided API.
%files
/opt/ltp/include/test.h
/opt/ltp/include/usctest.h
/opt/ltp/include/compiler.h
/opt/ltp/lib/libltp.a
/usr/share/pkgconfig/ltp.pc
/opt/ltp/bin/pan
/opt/ltp/bin/scanner
/opt/ltp/bin/bump
/opt/ltp/share/man/man3/tst_tmpdir.3
/opt/ltp/share/man/man3/random_range_seed.3
/opt/ltp/share/man/man3/pattern.3
/opt/ltp/share/man/man3/parse_ranges.3
/opt/ltp/share/man/man3/usctest.3
/opt/ltp/share/man/man3/random_range.3
/opt/ltp/share/man/man3/forker.3
/opt/ltp/share/man/man3/rmobj.3
/opt/ltp/share/man/man3/parse_open_flags.3
/opt/ltp/share/man/man3/tst_res.3
/opt/ltp/share/man/man3/write_log.3
/opt/ltp/share/man/man3/str_to_bytes.3
/opt/ltp/share/man/man3/tst_set_error.3
/opt/ltp/share/man/man3/parse_opts.3
/opt/ltp/share/man/man3/string_to_tokens.3
/opt/ltp/share/man/man3/tst_sig.3
/opt/ltp/share/man/man3/get_attrib.3
/opt/ltp/share/man/man1/pan.1
/opt/ltp/share/man/man1/doio.1
/opt/ltp/share/man/man1/iogen.1
/opt/ltp/share/man/man1/bump.1
# Post-install stuff would go here.
#EOF

