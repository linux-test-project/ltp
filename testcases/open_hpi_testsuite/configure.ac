dnl
dnl autoconf and automake for openhpi
dnl 	initial file by Andrea Brugger <andrea.l.brugger@intel.com> 1/03
dnl   various hacks by Sean Dague <http://dague.net/sean> 4/23/03

AC_PREREQ(2.57)
AC_INIT(openhpi.spec.in)
AM_INIT_AUTOMAKE(openhpi, 0.5.0)

AM_CONFIG_HEADER(config.h)

dnl AM_CONFIG_HEADER(./config.h)

AC_PROG_CC
AC_LIBTOOL_DLOPEN
AM_PROG_LIBTOOL

HPI_LIB_VERSION=3:0:3
HPI_PKG=openhpi
AC_SUBST(HPI_PKG)
AC_SUBST(HPI_LIB_VERSION)
AC_SUBST(ac_configure_args)

dnl Checks for programs
AC_PROG_CC
AC_PROG_CXX
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_RANLIB

dnl Check fo libraries
have_uuid=no
have_openipmi=no
have_sysfs=no

dnl Check for GLIB

AC_CHECK_PROG([found_pkg_config],[pkg-config],[yes])

if test "x$found_pkg_config" != "xyes"; then
    OH_CHECK_FAIL(pkg-config,pkg-config)
fi

GLIB_REQUIRED_VERSION=1.2.0

AC_ARG_WITH([glib],
    [[  --with-glib=VERSION     use glib version >= VERSION [default=1.2.0]]],
    [GLIB_REQUIRED_VERSION=$withval],
    [if pkg-config --atleast-version 1.2.0 glib; then
         GLIB_REQUIRED_VERSION=1.2.0;
     elif pkg-config --atleast-version 2.0.0 glib-2.0; then
         GLIB_REQUIRED_VERSION=2.0.0;
     else
         AC_MSG_ERROR([
*** GLIB is required. The latest version of
*** GLIB is always available from ftp://ftp.gtk.org/.])
        fi])

case "$GLIB_REQUIRED_VERSION" in
    1*)
        GLIB=glib;
        ;;
    2*)
        GLIB=glib-2.0;
        AC_MSG_WARN([glib 2.0 is not recommended, please use glib 1.2])
        ;;
    *)
        AC_MSG_ERROR([
*** unsupported GLIB $GLIB_REQUIRED_VERSION. The latest version of
*** GLIB is always available from ftp://ftp.gtk.org/.])
esac

if pkg-config --atleast-version $GLIB_REQUIRED_VERSION $GLIB; then
   :
else
   AC_MSG_ERROR([
*** GLIB >= $GLIB_REQUIRED_VERSION is required. The latest version of
*** GLIB is always available from ftp://ftp.gtk.org/.])
fi

exact_version=`pkg-config --modversion $GLIB`;
GLIB_CFLAGS=`pkg-config --cflags $GLIB`
GLIB_LIBS=`pkg-config --libs $GLIB`

AC_MSG_CHECKING(for GLIB - version >= $GLIB_REQUIRED_VERSION)
AC_MSG_RESULT(yes (version $exact_version))

dnl if the glib is installed on a non standard place
dnl like /opt/gnome/lib AC_CHECK_LIB will find the library.
old_cflags=$CFLAGS
old_libs=$LIBS

CFLAGS="$CFLAGS $GLIB_CFLAGS"
LIBS="$LIBS $GLIB_LIBS"

AC_CHECK_LIB([$GLIB], [g_slist_alloc], [],
[
OH_CHECK_FAIL(glib,glib-devel)
])

CFLAGS=$old_cflags
LIBS=$old_libs


AC_CHECK_LIB([ltdl], [lt_dlopen], [], [
    OH_CHECK_FAIL(libltdl,libltdl-devel,,
        this is required for plugin loading)
    ])

AC_CHECK_LIB([OpenIPMI], [ipmi_smi_setup_con], [have_openipmi=yes])

AC_CHECK_LIB([sysfs], [sysfs_get_mnt_path], [have_sysfs=yes])

OH_CHECK_NETSNMP

AC_CHECK_LIB([uuid], [uuid_generate], [have_uuid=yes], [
    OH_CHECK_FAIL(libuuid,e2fsprogs-devel)
    ])

dnl ssl is used for md2/md5 authentification in ipmidirect
AC_CHECK_LIB([ssl],[MD5_Init],[SSL_LIB=-lssl],[SSL_LIB=])
AC_SUBST(SSL_LIB)
AC_CHECK_HEADERS([openssl/md2.h openssl/md5.h])

dnl Extra goodies enablement stanzas

AC_ARG_ENABLE([thread],
    [  --enable-thread       make implementation thread-safe [default=yes]],
    [if test "x$enableval" = "xyes"; then
	THREAD_SAFE="yes"
    fi],
    [THREAD_SAFE="yes"])

if test "x$THREAD_SAFE" = "xyes"; then
        AC_DEFINE([HAVE_THREAD_SAFE], [1], [Define to 1 if you want thread-safe implementation])
        LIST_RESOURCES_THREADED=list_resources_threaded
fi
AC_SUBST(LIST_RESOURCES_THREADED)

AC_ARG_ENABLE([examples],
    [  --enable-examples       build HPI examples [default=yes]],
    [if test "x$enableval" = "xyes"; then
	ENABLED_DIRS="$ENABLED_DIRS examples"
    fi],
    [ENABLED_DIRS="$ENABLED_DIRS examples"])

AC_ARG_ENABLE([utilities],
    [  --enable-utilities      build HPI utilities [default=yes]],
    [if test "x$enableval" = "xyes"; then
	ENABLED_DIRS="$ENABLED_DIRS util"
    fi],
    [ENABLED_DIRS="$ENABLED_DIRS util"])

AC_SUBST(ENABLED_DIRS)

dnl Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([fcntl.h stdlib.h string.h sys/time.h unistd.h])

dnl Checks for library functions.
AC_FUNC_ERROR_AT_LINE
AC_FUNC_VPRINTF
AC_CHECK_FUNCS([gettimeofday memset])

dnl Checks for other helper programs

AC_PATH_PROGS(RPM, rpmbuild)

dnl plugin enablement stanzas

dnl list of static plugins
STATIC_PLUGIN_LIST=
STATIC_PLUGIN_EXTRA_LIBS=

AC_ARG_ENABLE([snmp_client],
    [  --enable-snmp_client          build snmp_client plugin [default=yes]],
    [if test "x$enableval" = "xyes"; then
        AC_SUBST(SNMP_CLIENT,snmp_client)
     elif test "x$enableval" = "xstatic"; then
        AC_SUBST(SNMP_CLIENT,snmp_client)
        STATIC_PLUGIN_LIST="$STATIC_PLUGIN_LIST snmp_client"
        STATIC_PLUGIN_EXTRA_LIBS="$STATIC_PLUGIN_EXTRA_LIBS -luuid"
    fi],
    [if test "x$have_netsnmp" = "xyes"; then
        AC_SUBST(SNMP_CLIENT,snmp_client)
     fi])

AC_ARG_ENABLE([dummy],
    [  --enable-dummy          build dummy plugin [default=yes]],
    [if test "x$enableval" = "xyes"; then
        AC_SUBST(DUMMY,dummy)
     elif test "x$enableval" = "xstatic"; then
        AC_SUBST(DUMMY,dummy)
        STATIC_PLUGIN_LIST="$STATIC_PLUGIN_LIST dummy"
        STATIC_PLUGIN_EXTRA_LIBS="$STATIC_PLUGIN_EXTRA_LIBS -luuid"
    fi],
    [AC_SUBST(DUMMY,dummy)])
    
AC_ARG_ENABLE([watchdog],
    [  --enable-watchdog       build watchdog plugin [default=yes]],
    [if test "x$enableval" = "xyes"; then
        AC_SUBST(WATCHDOG,watchdog)
    else
        AC_SUBST(WATCHDOG,"")
    fi],
    [AC_SUBST(WATCHDOG,watchdog)])

dnl
dnl We really need to make ipmi enablement be contigent on OpenIPMI
dnl

AC_ARG_ENABLE([ipmi],
    [  --enable-ipmi           build openipmi plugin [default=yes if OpenIPMI exists]],
    [if test "x$enableval" = "xyes"; then
        if test "x$have_openipmi" = "xyes"; then
            AC_SUBST(IPMI,ipmi)
        else
            OH_CHECK_FAIL(OpenIPMI,libOpenIPMI-devel,http://openipmi.sf.net,
                         Can not build IPMI support without OpenIPMI)
        fi
    fi],
    [if test "x$have_openipmi" = "xyes"; then
            AC_SUBST(IPMI,ipmi)
     fi
    ])

AC_ARG_ENABLE([sysfs],
    [  --enable-sysfs          build sysfs plugin [default=yes if libsysfs exists]],
    [if test "x$enableval" = "xyes"; then
        if test "x$have_sysfs" = "xyes"; then
            AC_SUBST(SYSFS,sysfs)
        else
            OH_CHECK_FAIL(libsysfs,,http://linux-diag.sf.net,
                Can not build sysfs support without libsysfs)
        fi
    fi],
    [if test "x$have_sysfs" = "xyes"; then
            AC_SUBST(SYSFS,sysfs)
     fi
    ])

AC_ARG_ENABLE([snmp_bc],
    [  --enable-snmp_bc        build IBM Blade Center plugin [default=yes if net-snmp is installed]],
    [if test "x$enableval" = "xyes"; then
        if test "x$have_netsnmp" = "xyes"; then
            AC_SUBST(SNMP_BC,snmp_bc)
#            CFLAGS="$CFLAGS" $SNMPFLAGS"
        else
            OH_CHECK_FAIL(net-snmp,net-snmp-devel,http://net-snmp.sourceforge.net/,
                Can not build IBM Blade Center support without net-snmp)
        fi
    fi],
    [if test "x$have_netsnmp" = "xyes"; then
            AC_SUBST(SNMP_BC,snmp_bc)
#            CFLAGS="$CFLAGS $SNMPFLAGS"
    fi]
    )

AC_ARG_ENABLE([ipmidirect],
    [  --enable-ipmidirect     build direct ipmi plugin [default=yes]],
    [if test "x$enableval" = "xyes"; then
            AC_SUBST(IPMIDIRECT,ipmidirect)
     elif test "x$enableval" = "xstatic"; then
            AC_SUBST(IPMIDIRECT,ipmidirect)
            STATIC_PLUGIN_LIST="$STATIC_PLUGIN_LIST ipmidirect"
            STATIC_PLUGIN_EXTRA_LIBS="$STATIC_PLUGIN_EXTRA_LIBS $SSL_LIB -luuid -lm -lstdc++"
     fi],
    [AC_SUBST(IPMIDIRECT,ipmidirect)])

dnl ipmidirect needs posix threads
if test "x$IPMIDIRECT" = "xipmidirect"; then
        if test "x$THREAD_SAFE" != "xyes" ; then
                AC_MSG_ERROR("ipmidirect plugin needs thread support (--enable-thread) !") 
        fi
fi

AC_ARG_ENABLE([simulator],
    [  --enable-simulator       build simulator plugin [default=yes]],
    [if test "x$enableval" = "xyes"; then
        AC_SUBST(SIMULATOR,simulator)
    else
        AC_SUBST(SIMULATOR,"")
    fi],
    [AC_SUBST(SIMULATOR,simulator)])


dnl
dnl create static plugin config
dnl

STATIC_PLUGIN=
STATIC_PLUGIN_EXTERN=
STATIC_PLUGIN_LIBS=

for name in $STATIC_PLUGIN_LIST; do
        STATIC_PLUGIN="$STATIC_PLUGIN { \"$name\", ${name}_get_interface },";
        STATIC_PLUGIN_EXTERN="${STATIC_PLUGIN_EXTERN} extern int ${name}_get_interface(void **pp, const uuid_t uuid);";
        STATIC_PLUGIN_LIBS="$STATIC_PLUGIN_LIBS \$(top_builddir)/src/plugins/$name/.libs/lib${name}.a";
done

dnl use src/plugin_static.c.in
AC_SUBST(STATIC_PLUGIN_EXTERN)
AC_SUBST(STATIC_PLUGIN)

dnl additional libs
AC_SUBST(STATIC_PLUGIN_LIBS)
AC_SUBST(STATIC_PLUGIN_EXTRA_LIBS)

dnl setup SNMPFLAGS
AC_SUBST(SNMPFLAGS)
AC_SUBST(SNMPLIBS)
AC_SUBST(SNMPDIR)

dnl defines OH_DEFAULT_UID_MAP and VARPATH
dnl set VARPATH only if unset or null.
VARPATH=${VARPATH:=/var/lib/openhpi}

AC_ARG_WITH(varpath,
    [[  --with-varpath=PATH     use direcory PATH to store openhpi specific data [default=/var/lib/openhpi]]],
    [VARPATH=$withval])
dnl substitute @VARPATH@ in all Makefiles
AC_SUBST(VARPATH)
dnl create the #define VARPATH and #define OH_DEFAULT_UID_MAP in config.h
AC_DEFINE_UNQUOTED(VARPATH,"$VARPATH",[openhpi specific data path])
AC_DEFINE_UNQUOTED(OH_DEFAULT_UID_MAP,"$VARPATH/uid_map",[uid map])

dnl 
dnl This code was borrowed from linux-ha configure.in file
dnl and is used for creating the proper substitutions
dnl in the spec file. Without it, the spec file will
dnl have ${exec_prefix}, etc. instead of the actual
dnl directory.
dnl
dnl

prefix=`eval echo "$prefix"`

case $prefix in
  NONE) prefix=/usr/local;;
esac
var() {
  case $1 in
    *'${'*)     res=`eval echo "$1"`;;
    *)          res="$1";;
  esac
  case "$res" in
    ""|NONE)    echo "$2";;
    *)          echo "$res";;
  esac
}

exec_prefix=`var "$exec_prefix" "$prefix"`
bindir=`var "$bindir" "$exec_prefix/bin"`
sbindir=`var "$sbindir" "$exec_prefix/sbin"`
libdir=`var "$libdir" "$exec_prefix/lib"`
includedir=`var "$includedir" "$exec_prefix/include"`
base_includedir="${includedir}"
AC_SUBST(base_includedir)

TEST_CLEAN='$(shell ls *.bb*) $(shell ls *.da) $(shell ls *.gcov)'
AC_SUBST(TEST_CLEAN)

AC_C_STRINGIZE

CXXFLAGS=$CFLAGS

CC_WARNINGS="-Wall -Wmissing-prototypes \
	-Wmissing-declarations \
	-Wstrict-prototypes \
	-Wpointer-arith \
	-Werror \
	-Wcast-qual \
	-Wcast-align"

CXX_WARNINGS="$CC_WARNINGS \
        -Wshadow \
        -Wwrite-strings \
        -Wredundant-decls"

CFLAGS="$CFLAGS $GLIB_CFLAGS $CC_WARNINGS"
NOCFLAGS=`echo $CFLAGS | perl -p -e 's/-O\S*//g'`
CXXFLAGS="$CXXFLAGS $GLIB_CFLAGS $CXX_WARNINGS"

AC_SUBST(NOCFLAGS)

if test "x$THREAD_SAFE" = "xyes" ; then
        CFLAGS="$CFLAGS -D_GNU_SOURCE -D_REENTRANT -fexceptions"
        CXXFLAGS="$CXXFLAGS -D_GNU_SOURCE -D_REENTRANT -fexceptions"
fi

LIBS="$LIBS $GLIB_LIBS -lm"
if test "x$THREAD_SAFE" = "xyes" ; then
        LIBS="$LIBS -lpthread"
fi


AC_CONFIG_FILES([openhpi.spec
        openhpi.pc
        rt-env.sh
        Makefile
        src/Makefile
        src/snmp/Makefile
        src/t/Makefile
	src/t/epath/Makefile
	src/t/rpt/Makefile
	src/t/sel/Makefile
        src/plugin_static.c
        src/plugins/Makefile
	src/plugins/snmp_client/Makefile
        src/plugins/dummy/Makefile
        src/plugins/watchdog/Makefile
        src/plugins/ipmi/Makefile
        src/plugins/snmp_bc/Makefile
        src/plugins/snmp_bc/t/Makefile
        src/plugins/sysfs/Makefile
        src/plugins/ipmidirect/Makefile
        src/plugins/simulator/Makefile
        docs/Makefile
        docs/hld/Makefile
        examples/Makefile
	util/Makefile])

AC_OUTPUT


