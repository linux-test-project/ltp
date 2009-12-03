dnl
dnl autoconf and automake for openhpi
dnl     initial file by Andrea Brugger <andrea.l.brugger@intel.com> 1/03
dnl   various hacks by Sean Dague <http://dague.net/sean> 4/23/03


AC_PREREQ(2.57)
AC_INIT(openhpi, 2.14.1)
AC_CONFIG_SRCDIR(openhpi.spec.in)
AM_INIT_AUTOMAKE([1.8])

AM_CONFIG_HEADER(config.h)
AH_TOP([#ifndef __OPENHPI_CONFIG_H
#define __OPENHPI_CONFIG_H])
AH_BOTTOM([#endif /* __OPENHPI_CONFIG_H */])

#
# (major version) = (first number) - (third number) 
# (minor version) = (third number)
# (patch version) = (second number)
# example HPI_LIB_VERSION=13:4:11
# major version: 2  = 13 - 11
# minor version: 11 = 11
# patch version: 4  = 4
#
HPI_LIB_VERSION=16:1:14
HPI_PKG=$PACKAGE_NAME
HPI_UTIL_PKG=${PACKAGE_NAME}utils
HPI_CLIENT_PKG=${PACKAGE_NAME}client
AC_SUBST(HPI_CLIENT_PKG)
AC_SUBST(HPI_UTIL_PKG)
AC_SUBST(HPI_PKG)
AC_SUBST(HPI_LIB_VERSION)
AC_SUBST(ac_configure_args)

dnl Checks for programs
AC_PROG_CC
AC_LIBTOOL_DLOPEN
AC_PROG_LIBTOOL
AC_PROG_LN_S
AC_PROG_CXX
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET

enabled_non32bit="no"
AC_ARG_ENABLE([non32bit-int],
    [  --enable-non32bit-int   Allow compile on non 32bit int architectures [[default=no]]],
    [if test "x$enableval" = "xyes"; then
        enabled_non32bit="yes"
    fi])

dnl die on int != 32bits.  This is too instrumental to our code right now.
dnl AC_MSG_CHECKING(int is 4 bytes)
OH_SET_SIZES
if test "x$OH_SIZEOF_INT" != "x4"; then
    if test "x$enabled_non32bit" = "xyes"; then
        AC_MSG_WARN([
*** Size of int is not 4 bytes, it is $OH_SIZEOF_INT bytes on this platform.
*** The --enable_non32bit_int arg has been used to force compilation regardless.
*** Execution on this platform is not supported and could produce undefined
*** behavior.
        ])
    else
        AC_MSG_ERROR([
*** Size of int is not 4 bytes, it is $OH_SIZEOF_INT bytes on this platform.
*** Execution on this platform is not supported and could produce undefined
*** behavior.
*** Please contact the openhpi development team to fix this
        ])
    fi
fi

AM_CONDITIONAL(CROSS_COMPILATION, test x$cross_compiling = xyes)

dnl Check for libraries
have_uuid=no
have_openipmi=no
have_fam=no
have_sysfs=no
have_rtas_lib=no

dnl Check for GLIB

AC_CHECK_PROG([found_pkg_config],[pkg-config],[yes])

if test "x$found_pkg_config" != "xyes"; then
    OH_CHECK_FAIL(pkg-config,pkg-config)
fi
PKG_CFG_SETPATH

# check for gcc version

AC_MSG_CHECKING(gcc >= 3.2.0)
OH_CHECK_GCC(3,2,0)

dnl glib 2.x checks
GLIB_REQUIRED_VERSION=2.2.0
GLIB=glib-2.0

if pkg-config --atleast-version $GLIB_REQUIRED_VERSION $GLIB; then
   :
else
   AC_MSG_ERROR([
*** GLIB >= $GLIB_REQUIRED_VERSION is required. The latest version of
*** GLIB is always available from ftp://ftp.gtk.org/.])
fi

exact_version=`pkg-config --modversion $GLIB`;
GLIB_CFLAGS=`pkg-config --cflags $GLIB gthread-2.0`
GLIB_LIBS=`pkg-config --libs $GLIB gthread-2.0`

# On some versions of Solaris the pkg-config file for gthread-2.0 contains a
# compiler option, '-mt', that is incompatible with gcc
case $host_os in
    solaris2.10)
        GLIB_CFLAGS=`echo $GLIB_CFLAGS | sed -e 's/-mt/-D_REENTRANT/g'`
        GLIB_LIBS=`echo $GLIB_LIBS | sed -e 's/-mt//g'`
        ;;
esac

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

AC_CHECK_LIB([sysfs], [sysfs_get_mnt_path], [have_sysfs=yes])
AC_CHECK_HEADER([sysfs/libsysfs.h], [], [have_sysfs=no])

OH_CHECK_NETSNMP

AC_CHECK_LIB([uuid], [uuid_generate], [have_uuid=yes], [
    OH_CHECK_FAIL(libuuid,e2fsprogs-devel)
    ])

dnl ssl is used for md2/md5 authentification in ipmidirect
dnl and for SSL-based communication in ilo2_ribcl and oa_soap
AC_CHECK_LIB([crypto],[MD5_Init],[CRYPTO_LIB=-lcrypto],[CRYPTO_LIB=])
AC_SUBST(CRYPTO_LIB)
AC_CHECK_LIB([ssl],[SSL_library_init],[SSL_LIB=-lssl],[SSL_LIB=])
AC_SUBST(SSL_LIB)
AC_CHECK_HEADERS([openssl/md2.h openssl/md5.h openssl/bio.h openssl/ssl.h openssl/err.h],[have_openssl=yes],[have_openssl=no])

dnl xml is used for XML-based communication in ilo2_ribcl and oa_soap
AC_CHECK_LIB([xml2],[xmlParseMemory],[XML2_LIB=-lxml2],[XML2_LIB=])
AC_CHECK_HEADERS([libxml2/libxml/xmlexports.h],[XML2_INCLUDE="-I/usr/include/libxml2"],[XML2_INCLUDE])
AC_SUBST(XML2_LIB)
AC_SUBST(XML2_INCLUDE)

dnl Build daemon statically or dynamically.
openhpid_static="no"

ENABLED_DIRS="$ENABLED_DIRS openhpid"
AC_ARG_ENABLE([daemon],
	[  --enable-dynamic-daemon   build HPI daemon and client library non-statically[[default=yes]]],
        [if test "x$enableval" = "xno"; then   
        openhpid_static="yes"
    fi],
    [openhpid_static="no"])

AM_CONDITIONAL(OPENHPID_STATIC, test x$openhpid_static = xyes)

AC_ARG_ENABLE([clients],
    [  --enable-clients        build HPI client programs [[default=yes]]],
    [if test "x$enableval" = "xyes"; then
        ENABLED_DIRS="$ENABLED_DIRS clients"
    fi],
    [ENABLED_DIRS="$ENABLED_DIRS clients"])

AC_ARG_ENABLE([cpp_wrappers],
    [  --enable-cpp_wrappers   build SaHpi C++ wrapper objects [[default=no]]],
    [if test "x$enableval" = "xyes"; then
        ENABLED_DIRS="$ENABLED_DIRS cpp"
    fi])

AC_SUBST(ENABLED_DIRS)

dnl Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([fcntl.h stdlib.h string.h sys/time.h unistd.h stdarg.h netinet/in.h])

dnl Checks for library functions.
AC_FUNC_ERROR_AT_LINE
AC_FUNC_VPRINTF
AC_CHECK_FUNCS([gettimeofday memset strdup])

dnl Checks for other helper programs

AC_PATH_PROGS(RPM, rpmbuild)

dnl plugin enablement stanzas

dnl list of static plugins
STATIC_PLUGIN_LIST=
STATIC_PLUGIN_EXTRA_LIBS=

AC_CHECK_HEADER([linux/watchdog.h], [have_linux_watchdog_h=yes], [], [#include <linux/types.h>])
AC_ARG_ENABLE([watchdog],
    [  --enable-watchdog       build watchdog plugin [[default=yes if linux/watchdog.h exists]]],
    [if test "x$enableval" = "xyes"; then
        if test "x$have_linux_watchdog_h" = "xyes"; then
            AC_SUBST(WATCHDOG,watchdog)
        else
            AC_MSG_ERROR("Can not build the watchdog plugin without linux/watchdog.h.")
        fi
    else
        AC_SUBST(WATCHDOG,"")
    fi],
    [if test "x$have_linux_watchdog_h" = "xyes"; then
        AC_SUBST(WATCHDOG,watchdog)
    else
        AC_SUBST(WATCHDOG,"")
    fi])

dnl
dnl We really need to make ipmi enablement be contigent on OpenIPMI
dnl

if PKG_CONFIG_PATH=$PKG_CONFIG_PATH:/usr/local/lib/pkgconfig pkg-config --atleast-version 1.4.20 OpenIPMI; then
    have_openipmi=yes
    AC_CHECK_LIB([OpenIPMI], [ipmi_smi_setup_con], [have_openipmi=yes])
    OPENIPMI_CFLAGS=`PKG_CONFIG_PATH=$PKG_CONFIG_PATH:/usr/local/lib/pkgconfig pkg-config --cflags OpenIPMI`
    AC_SUBST(OPENIPMI_CFLAGS)
    OPENIPMI_LIBS=`PKG_CONFIG_PATH=$PKG_CONFIG_PATH:/usr/local/lib/pkgconfig pkg-config --libs OpenIPMI`
    AC_SUBST(OPENIPMI_LIBS)
fi

AC_ARG_ENABLE([ipmi],
    [  --enable-ipmi           build openipmi plugin [[default=yes if OpenIPMI exists]]],
    [if test "x$enableval" = "xyes"; then
        if test "x$have_openipmi" = "xyes"; then
            AC_SUBST(IPMI,ipmi)
	    AC_SUBST(WITH_IPMI,1)
        else
            OH_CHECK_FAIL(OpenIPMI,libOpenIPMI-devel,http://openipmi.sf.net,
                         Can not build IPMI support without OpenIPMI >= 1.4.20)
        fi
    elif test "x$enableval" = "xstatic"; then
        if test "x$have_openipmi" = "xyes"; then
            AC_SUBST(IPMI,ipmi)
	    AC_SUBST(WITH_IPMI,1)
            STATIC_PLUGIN_LIST="$STATIC_PLUGIN_LIST ipmi"
            STATIC_PLUGIN_EXTRA_LIBS="$STATIC_PLUGIN_EXTRA_LIBS"
        else
            OH_CHECK_FAIL(OpenIPMI,libOpenIPMI-devel,http://openipmi.sf.net,
                         Can not build IPMI support without OpenIPMI >= 1.4.20)
        fi
    else
	    AC_SUBST(WITH_IPMI,0)
    fi],
    [if test "x$have_openipmi" = "xyes"; then
            AC_SUBST(IPMI,ipmi)
	    AC_SUBST(WITH_IPMI,1)
     else
	    AC_SUBST(WITH_IPMI,0) 
     fi
    ])

AC_ARG_ENABLE([sysfs],
    [  --enable-sysfs          build sysfs plugin [[default=yes if libsysfs exists]]],
    [if test "x$enableval" = "xyes"; then
        if test "x$have_sysfs" = "xyes"; then
            AC_SUBST(SYSFS,sysfs)
	    AC_SUBST(WITH_SYSFS,1)
        else
            OH_CHECK_FAIL(libsysfs,,http://linux-diag.sf.net,
                Can not build sysfs support without libsysfs)
        fi
    else
	    AC_SUBST(WITH_SYSFS,0)
    fi],
    [if test "x$have_sysfs" = "xyes"; then
            AC_SUBST(SYSFS,sysfs)
	    AC_SUBST(WITH_SYSFS,1)
     else
	    AC_SUBST(WITH_SYSFS,0)
     fi
    ])

AC_ARG_ENABLE([snmp_bc],
    [  --enable-snmp_bc        build IBM Blade Center plugin [[default=yes if net-snmp is installed]]],
    [if test "x$enableval" = "xyes"; then
        if test "x$have_netsnmp" = "xyes"; then
            AC_SUBST(SNMP_BC,snmp_bc)
	    AC_SUBST(WITH_SNMP_BC,1)
            SNMPDIR=snmp
        else
            OH_CHECK_FAIL(net-snmp,net-snmp-devel,http://net-snmp.sourceforge.net/,
                Can not build IBM Blade Center support without net-snmp)
        fi
    else
	    AC_SUBST(WITH_SNMP_BC,0)
    fi],
    [if test "x$have_netsnmp" = "xyes"; then
            AC_SUBST(SNMP_BC,snmp_bc)
	    AC_SUBST(WITH_SNMP_BC,1)
            SNMPDIR=snmp
    else
	    AC_SUBST(WITH_SNMP_BC,0)
    fi]
    )

AC_ARG_ENABLE([ipmidirect],
    [  --enable-ipmidirect     build direct ipmi plugin [[default=yes]]],
    [if test "x$enableval" = "xyes"; then
            AC_SUBST(IPMIDIRECT,ipmidirect)
     elif test "x$enableval" = "xstatic"; then
            AC_SUBST(IPMIDIRECT,ipmidirect)
            STATIC_PLUGIN_LIST="$STATIC_PLUGIN_LIST ipmidirect"
            STATIC_PLUGIN_EXTRA_LIBS="$STATIC_PLUGIN_EXTRA_LIBS $SSL_LIB -luuid -lm -lstdc++"
     fi],
    [AC_SUBST(IPMIDIRECT,ipmidirect)])

dnl    [AC_SUBST(IPMIDIRECT,ipmidirect)])

OH_CHECK_RTAS

#Configure options for the RTAS plugin
AC_ARG_ENABLE([rtas],
    [  --enable-rtas           build rtas ppc plugin [[default=no]]],
    [if test "x$enableval" = "xyes"; then
        if test "x$have_rtas_lib" = "xyes"; then
            AC_SUBST(RTAS,rtas)
	    AC_SUBST(WITH_RTAS,1)
        else
            OH_CHECK_FAIL(librtas, librtas-devel, http://librtas.ozlabs.org/,
                Cannot build RTAS plugin without the librtas library)
        fi
    else
	    AC_SUBST(WITH_RTAS,0)
    fi],
    [AC_SUBST(WITH_RTAS,0)])

# configure argument for the simulator plugin
AC_ARG_ENABLE([simulator],
    [  --enable-simulator      build simulator plugin [[default=yes]]],
    [if test "x$enableval" = "xyes"; then
        AC_SUBST(SIMULATOR,simulator)
    fi],
    [AC_SUBST(SIMULATOR,simulator)])

# configure argument for the ilo2_ribcl plugin
AC_ARG_ENABLE([ilo2_ribcl],
    [  --enable-ilo2_ribcl     build ilo2_ribcl plugin [[default=no]]],
    [if test "x$enableval" = "xyes"; then
        if test "x$have_openssl" = "xyes"; then
            if test -n "$XML2_INCLUDE"; then
                AC_SUBST(ILO2_RIBCL,ilo2_ribcl)
                AC_SUBST(WITH_ILO2_RIBCL,1)
            else
                OH_CHECK_FAIL(xml2-devel,,,Can not build ilo2_ribcl without xml2 headers)
            fi
        else
            OH_CHECK_FAIL(openssl-devel,,,Can not build ilo2_ribcl without openssl headers)
        fi
    else
        AC_SUBST(WITH_ILO2_RIBCL,0)
    fi],
    [if test "x$have_openssl" = "xyes" && test -n "$XML2_INCLUDE"; then
        AC_SUBST(ILO2_RIBCL,ilo2_ribcl)
        AC_SUBST(WITH_ILO2_RIBCL,1)
     else
        AC_SUBST(WITH_ILO2_RIBCL,0)
     fi
    ])

# configure argument for the oa_soap plugin
AC_ARG_ENABLE([oa_soap],
    [  --enable-oa_soap        build oa_soap plugin [[default=no]]],
    [if test "x$enableval" = "xyes"; then
        if test "x$have_openssl" = "xyes"; then
	    if test -n "$XML2_INCLUDE"; then
                AC_SUBST(OA_SOAP,oa_soap)
                AC_SUBST(WITH_OA_SOAP,1)
	    else
	    	OH_CHECK_FAIL(xml2-devel,,,Can not build oa_soap without xml2 headers)
	    fi
	else
	    OH_CHECK_FAIL(openssl-devel,,,Can not build oa_soap without openssl headers)
	fi
    else
        AC_SUBST(WITH_OA_SOAP,0)
    fi],
    [if test "x$have_openssl" = "xyes" && test -n "$XML2_INCLUDE"; then
        AC_SUBST(OA_SOAP,oa_soap)
        AC_SUBST(WITH_OA_SOAP,1)
     else
        AC_SUBST(WITH_OA_SOAP,0)
     fi
    ])

dnl enable marshal library
AC_SUBST(MARSHALDIR,marshal)

dnl
dnl create static plugin config
dnl

STATIC_PLUGIN=
STATIC_PLUGIN_EXTERN=
STATIC_PLUGIN_LIBS=

for name in $STATIC_PLUGIN_LIST; do
        STATIC_PLUGIN="$STATIC_PLUGIN { \"$name\", ${name}_get_interface },";
        STATIC_PLUGIN_EXTERN="${STATIC_PLUGIN_EXTERN} extern int ${name}_get_interface(void **pp, const uuid_t uuid);";
        STATIC_PLUGIN_LIBS="$STATIC_PLUGIN_LIBS \$(top_builddir)/plugins/$name/lib${name}.la";
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
AC_SUBST(SSLDIR)

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
sysconfdir=`var "$sysconfdir" "$prefix/etc"`
libdir=`var "$libdir" "$exec_prefix/lib"`
includedir=`var "$includedir" "$exec_prefix/include"`
localstatedir=`var "$localstatedir" "/var"`
base_includedir="${includedir}"
AC_SUBST(base_includedir)

dnl
dnl defines CONFPATH, OH_DEFAULT_CONF and OH_CLIENT_DEFAULT_CONF
dnl

#CONFPATH=${CONFPATH:=/etc/${PACKAGE_NAME}}

#AC_ARG_WITH(confpath,
#   [[  --with-confpath=PATH    use directory PATH to store openhpi configuration file [default=/etc/openhpi]]],
#   [CONFPATH=$withval])

dnl substitute @CONFPATH@ in all Makefiles
CONFPATH=${sysconfdir}/openhpi
AC_SUBST(CONFPATH)

dnl create the #define in config.h
AC_DEFINE_UNQUOTED(CONFPATH, "$CONFPATH",
                   [openhpi specific config path])
AC_DEFINE_UNQUOTED(OH_DEFAULT_CONF,"$CONFPATH/${PACKAGE_NAME}.conf",
                   [default configfile])
AC_DEFINE_UNQUOTED(OH_CLIENT_DEFAULT_CONF,"$CONFPATH/${HPI_CLIENT_PKG}.conf",
                   [default client configfile])



dnl defines OH_DEFAULT_UID_MAP and VARPATH
dnl set VARPATH only if unset or null.
VARPATH=${VARPATH:=${localstatedir}/lib/${PACKAGE_NAME}}

AC_ARG_WITH(varpath,
    [[  --with-varpath=PATH     use directory PATH to store openhpi specific data [default=$prefix/var/lib/openhpi]]],
    [VARPATH=$withval])
dnl substitute @VARPATH@ in all Makefiles
AC_SUBST(VARPATH)
dnl create the #define VARPATH and #define OH_DEFAULT_UID_MAP in config.h
AC_DEFINE_UNQUOTED(VARPATH,"$VARPATH",[openhpi specific data path])
AC_DEFINE_UNQUOTED(OH_DEFAULT_UID_MAP,"$VARPATH/uid_map",[uid map])


dnl OH_PLUGIN_PATH - Directory where plugin libraries live.
dnl default is "$libdir/openhpir"
PLUGINPATH=${PLUGINPATH:=${libdir}/${PACKAGE_NAME}}
AC_ARG_WITH(pluginpath,
                [[  --with-pluginpath=PATH     use directory PATH to locate plugin libraries [default=LIBDIR/openhpi]]],
                    [PLUGINPATH=$withval])
dnl create #define OH_PLUGIN_PATH in config.h
AC_DEFINE_UNQUOTED(OH_PLUGIN_PATH,"$PLUGINPATH",[openhpi plugin path])

TEST_CLEAN='$(shell ls *.bb* *.da *.gcov *.gcno *.beam *.gcda *.summary)'
AC_SUBST(TEST_CLEAN)

AC_C_STRINGIZE

dnl getting CFLAGS right takes some effort
dnl we start with a good base set that has been built up
CFLAGS="$CFLAGS $GLIB_CFLAGS"

dnl next we have to do transformations to either turn on
dnl test coverage support.

AC_ARG_ENABLE([testcover],
    [  --enable-testcover      adds test coverage and profiling support [[default=no]]],
    [if test "x$enableval" = "xyes"; then
            CFLAGS=`echo $CFLAGS | sed -e 's/-O\w\?\>[ ]*//g'`
            CFLAGS="$CFLAGS -ftest-coverage -fprofile-arcs"
            CXXFLAGS=`echo $CXXFLAGS | sed -e 's/-O\w\?\>[ ]*//g'`
            CXXFLAGS="$CXXFLAGS -ftest-coverage -fprofile-arcs"
    fi]
    )

dnl This makes libopenhpi debugger friendly
dnl by eliminating optimizations and adding the -g flag

AC_ARG_ENABLE([debuggable],
    [  --enable-debuggable     adds -g flag and turns off optimization [[default=no]]],
    [if test "x$enableval" = "xyes"; then
            CFLAGS=`echo $CFLAGS | sed -e 's/-O\w\?\>[ ]*//g'`
            CFLAGS=`echo $CFLAGS | sed -e 's/^-g\w*[ ]*\| -g\w*//g'`
            CFLAGS="$CFLAGS -O0 -ggdb3"
            CXXFLAGS=`echo $CXXFLAGS | sed -e 's/-O\w\?\>[ ]*//g'`
            CXXFLAGS=`echo $CXXFLAGS | sed -e 's/^-g\w*[ ]*\| -g\w*//g'`
            CXXFLAGS="$CXXFLAGS -O0 -ggdb3"
    else
            CFLAGS=`echo $CFLAGS | sed -e 's/^-g\w*[ ]*\| -g\w*//g'`
            CXXFLAGS=`echo $CXXFLAGS | sed -e 's/^-g\w*[ ]*\| -g\w*//g'`
    fi]
    )

dnl This will let the user decide whether he wants
dnl to use dbg/trace messages.

AC_ARG_ENABLE([debug-msgs],
    [  --enable-debug-msgs     enables dynamic debug messages [[default=yes]]],
    [if test "x$enableval" = "xyes"; then
            AC_DEFINE([OH_DBG_MSGS],[1],[Enables dynamic debug messages])
    fi],
    [AC_DEFINE([OH_DBG_MSGS],[1],[Enables dynamic debug messages])]
    )

dnl Now we throw in a bunch of warnings.  C and C++ need
dnl different sets here

CC_WARNINGS="\
        -Wall \
        -Wmissing-prototypes \
        -Wmissing-declarations \
        -Wstrict-prototypes \
        -Wpointer-arith \
        -Wformat=2 \
        -Wformat-security \
        -Wformat-nonliteral \
        -Wno-format-y2k \
        -Wcast-qual \
        -Wcast-align \
        -Wno-strict-aliasing \
	-fno-strict-aliasing \
        -Wno-unused-value "

CXX_WARNINGS="\
        -Wall \
        -Wpointer-arith \
        -Wformat=2 \
        -Wformat-security \
        -Wformat-nonliteral \
        -Wcast-qual \
        -Wcast-align \
        -Wshadow \
        -Wwrite-strings \
        -Wredundant-decls \
        -Wno-strict-aliasing \
	-fno-strict-aliasing \
        -Wno-unused-value "

if test -n "`gcc --version | grep ' 3.2'`" ; then
        CC_WARNINGS=`echo $CC_WARNINGS | sed -e 's/-Wno-strict-aliasing//g'`
        CXX_WARNINGS=`echo $CC_WARNINGS | sed -e 's/-Wno-strict-aliasing//g'`
fi

case $host_os in
    solaris*)
        CC_WARNINGS=`echo $CC_WARNINGS | sed -e 's/-Wcast-qual//g'`
        CXX_WARNINGS=`echo $CXX_WARNINGS | sed -e 's/-Wwrite-strings//g'`
        ;;
esac

dnl remerge the warnings into the FLAGS
CXXFLAGS="$CFLAGS $CXX_WARNINGS"
CFLAGS="$CFLAGS $CC_WARNINGS"

dnl lastly, we add things if we are thread safe and have the openssl library
dnl (this could probably happen earlier in this file)
CFLAGS="$CFLAGS -D_GNU_SOURCE -D_REENTRANT -fexceptions"
CXXFLAGS="$CXXFLAGS -D_GNU_SOURCE -D_REENTRANT -fexceptions"
if test "x$have_openssl" = "xyes"; then
	SSLDIR=ssl
	AC_DEFINE([HAVE_OPENSSL],[1],[openssl library was found])
fi
AM_CONDITIONAL(HAVE_OPENSSL, test x$have_openssl = xyes)
OH_SSL_INCLUDES="-I\$(top_srcdir)/ssl"
AC_SUBST(OH_SSL_INCLUDES)

OPENHPI_INCLUDES="-I\$(top_srcdir)/include -I\$(top_srcdir)/utils -I\$(top_builddir)/utils"
dnl replace common includes here
AC_SUBST(OPENHPI_INCLUDES)

dnl Set up libs that will be linked to
LIBS="$LIBS $SSL_LIB $GLIB_LIBS -lm -lpthread"

AC_CHECK_PROG([found_docbook2html], [docbook2html], [yes])
AC_ARG_ENABLE([docs],
    [  --enable-docs            generate documentation [[default=yes if docbook2html exists]]],
    [if test "x$enableval" = "xyes"; then
        if test "x$found_docbook2html" = "xyes"; then
            AC_SUBST(HLD,hld)
            AC_SUBST(WITH_DOCBOOK2HTML,1)
        else
           OH_CHECK_FAIL(docbook2html,docbook-utils,http://sources.redhat.com/docbook-tools, Can not generate HTML design docs without docbook2html)
	fi
    else
        AC_SUBST(HLD,"")
        AC_SUBST(WITH_DOCBOOK2HTML,0)
    fi],
    [if test "x$found_docbook2html" = "xyes"; then
        AC_SUBST(HLD,hld)  
        AC_SUBST(WITH_DOCBOOK2HTML,1)
    else
        AC_SUBST(HLD,"")
        AC_SUBST(WITH_DOCBOOK2HTML,0)
    fi])
    
AC_CHECK_PROG([found_pod2man], [pod2man], [yes])
AC_ARG_ENABLE([man],
    [  --enable-man            generate man pages [[default=yes if pod2man exists]]],
    [if test "x$enableval" = "xyes"; then
        if test "x$found_pod2man" = "xyes"; then
            AC_SUBST(MAN,man)
        else
            OH_CHECK_FAIL(pod2man,perl,http://www.perl.org,
                          Can not generate man pages without pod2man)
        fi
    else
        AC_SUBST(MAN,"")
    fi],
    [if test "x$found_pod2man" = "xyes"; then
        AC_SUBST(MAN,man)
    else
        AC_SUBST(MAN,"")
    fi])

AC_CONFIG_FILES([openhpi.spec
        openhpi.pc
        rt-env.sh
        Makefile
        utils/openhpiutils.pc
        utils/Makefile
        utils/t/Makefile
        utils/t/epath/Makefile
        utils/t/rpt/Makefile
        utils/t/sahpi/Makefile
        utils/t/el/Makefile
        utils/t/uid/Makefile
        utils/t/ann/Makefile
        marshal/Makefile
        marshal/t/Makefile
        plugins/Makefile
        plugins/watchdog/Makefile
        plugins/ipmi/Makefile
        plugins/ipmi/t/Makefile
        plugins/snmp_bc/Makefile
        plugins/snmp_bc/t/Makefile
        plugins/sysfs/Makefile
        plugins/ipmidirect/Makefile
        plugins/ipmidirect/t/Makefile
        plugins/simulator/Makefile
        plugins/simulator/t/Makefile
        plugins/rtas/Makefile
        plugins/ilo2_ribcl/Makefile
        plugins/oa_soap/Makefile
        scripts/Makefile
        scripts/test/Makefile
        snmp/Makefile
	ssl/Makefile
        src/Makefile
        src/plugin_static.c
        src/t/Makefile
        src/t/ohpi/Makefile
        docs/Makefile
        docs/hld/Makefile
        docs/man/Makefile
        openhpid/Makefile
        openhpid/openhpid.sh
        hpi_shell/Makefile
        cpp/Makefile
        cpp/t/Makefile
        clients/Makefile])

AC_OUTPUT
