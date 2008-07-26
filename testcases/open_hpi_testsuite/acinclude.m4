##################################################
#
#  OpenHPI Macros
#
#  Copyright (C) IBM Corp 2003-2006
#
#  Author(s):
#      Sean Dague <http://dague.net/sean>
#      Renier Morales <renier@openhpi.org>
#
#  This file is licensed under the same terms as OpenHPI itself.
#  See the COPYING file in the top level of OpenHPI for more info.
#
# This is a set of common macros to be used by the top
# level configure.in.
#
##################################################


AC_DEFUN([OH_SET_SIZES],
    [

    if test "x$cross_compiling" != "xno"; then
        if test "x$OH_SIZEOF_UCHAR" = x; then
            OH_SIZEOF_UCHAR=$ac_cv_sizeof_uchar
        fi
        if test "x$OH_SIZEOF_USHORT" = x; then
            OH_SIZEOF_USHORT=$ac_cv_sizeof_ushort
        fi
        if test "x$OH_SIZEOF_UINT" = x; then
            OH_SIZEOF_UINT=$ac_cv_sizeof_uint
        fi
        if test "x$OH_SIZEOF_CHAR" = x; then
            OH_SIZEOF_CHAR=$ac_cv_sizeof_char
        fi
        if test "x$OH_SIZEOF_SHORT" = x; then
            OH_SIZEOF_SHORT=$ac_cv_sizeof_short
        fi
        if test "x$OH_SIZEOF_INT" = x; then
            OH_SIZEOF_INT=$ac_cv_sizeof_int
        fi
        if test "x$OH_SIZEOF_LLONG" = x; then
            OH_SIZEOF_LLONG=$ac_cv_sizeof_longlong
        fi
        if test "x$OH_SIZEOF_FLOAT" = x; then
            OH_SIZEOF_FLOAT=$ac_cv_sizeof_float
        fi
        if test "x$OH_SIZEOF_DOUBLE" = x; then
            OH_SIZEOF_DOUBLE=$ac_cv_sizeof_double
        fi
    else
        OH_SSFILE=testsize
        OH_SSSOURCE="$OH_SSFILE.c"
        echo "#include <stdlib.h>" > $OH_SSSOURCE
        echo "#include <stdio.h>" >> $OH_SSSOURCE
        echo "int main() {" >> $OH_SSSOURCE
        # add more here if you need them
        # the lots of slashes are needed to do the processing below right
        echo "printf(\"unsigned char %d\\\\n\",sizeof(unsigned char));" >> $OH_SSSOURCE
        echo "printf(\"unsigned short %d\\\\n\",sizeof(unsigned short));" >> $OH_SSSOURCE
        echo "printf(\"unsigned int %d\\\\n\",sizeof(unsigned int));" >> $OH_SSSOURCE
        echo "printf(\"char %d\\\\n\",sizeof(char));" >> $OH_SSSOURCE
        echo "printf(\"short %d\\\\n\",sizeof(short));" >> $OH_SSSOURCE
        echo "printf(\"int %d\\\\n\",sizeof(int));" >> $OH_SSSOURCE
        echo "printf(\"long long %d\\\\n\",sizeof(long long));" >> $OH_SSSOURCE
        echo "printf(\"float %d\\\\n\",sizeof(float));" >> $OH_SSSOURCE
        echo "printf(\"double %d\\\\n\",sizeof(double));" >> $OH_SSSOURCE
        echo "return 0;" >> $OH_SSSOURCE
        echo "}" >> $OH_SSSOURCE
    
        $CC -o $OH_SSFILE $OH_SSSOURCE

        OH_TYPE_SIZES=`./$OH_SSFILE`
        # feel free to define more logic here if we need it
    
        OH_SIZEOF_UCHAR=`echo -e $OH_TYPE_SIZES | grep "^unsigned char" | awk '{print $[3]}'`
        OH_SIZEOF_USHORT=`echo -e $OH_TYPE_SIZES | grep "^unsigned short" | awk '{print $[3]}'`
        OH_SIZEOF_UINT=`echo -e $OH_TYPE_SIZES | grep "^unsigned int" | awk '{print $[3]}'`
        OH_SIZEOF_CHAR=`echo -e $OH_TYPE_SIZES | grep "^char" | awk '{print $[2]}'`
        OH_SIZEOF_SHORT=`echo -e $OH_TYPE_SIZES | grep "^short" | awk '{print $[2]}'`
        OH_SIZEOF_INT=`echo -e $OH_TYPE_SIZES | grep "^int" | awk '{print $[2]}'`
        OH_SIZEOF_LLONG=`echo -e $OH_TYPE_SIZES | grep "^long long" | awk '{print $[3]}'`
        OH_SIZEOF_FLOAT=`echo -e $OH_TYPE_SIZES | grep "^float" | awk '{print $[2]}'`
        OH_SIZEOF_DOUBLE=`echo -e $OH_TYPE_SIZES | grep "^double" | awk '{print $[2]}'`
        rm -f $OH_SSFILE $OH_SSSOURCE

    fi
    ])

#
# OH_CHECK_FAIL($LIBNAME,$PACKAGE_SUGGEST,$URL,$EXTRA)
#

AC_DEFUN([OH_CHECK_FAIL],
    [
    OH_MSG=`echo -e "- $1 not found!\n"`
    if test "x" != "x$4"; then
        OH_MSG=`echo -e "$OH_MSG\n- $4"`
    fi
    if test "x$2" != "x"; then
        OH_MSG=`echo -e "$OH_MSG\n- Try installing the $2 package\n"`
    fi
    if test "x$3" != "x"; then
        OH_MSG=`echo -e "$OH_MSG\n- or get the latest software from $3\n"`
    fi
    
    AC_MSG_ERROR(
!
************************************************************
$OH_MSG
************************************************************
)
    ]
)

#
#  gcc version check.
#

AC_DEFUN([OH_CHECK_GCC],
    [
    GCCVERSIONOK=`gcc -dumpversion | \
    sed 's/\./ /g' | \
    awk '{ \
        if ( $[1] > $1) { \
            print "OK"; \
        } \
        if ( $[1] == $1 ) { \
            if( $[2] > $2 ) { \
                print "OK"; \
            } \
            if( $[2] == $2 ) { \
                if( $[3] >= $3 ) { \
                    print "OK"; \
                } \
            } \
        } \
    }'` \
    
    if test "$GCCVERSIONOK" == "OK"; then
        AC_MSG_RESULT(yes)
    else
        OH_CHECK_FAIL(gcc >= $1.$2.$3 is required to build OpenHPI)
    fi
])
    

# it is worth noting that we have to strip 
# optimization from the cflags for net-snmp
# hopefully they'll fix that bug in the future

AC_DEFUN([OH_CHECK_NETSNMP],
    [
    AC_MSG_CHECKING(for net-snmp)
    AC_TRY_LINK(
    [
        #include <net-snmp/net-snmp-config.h>
        #include <net-snmp/net-snmp-includes.h>
    ],
    [
        struct snmp_session session
    ],
    [
        have_netsnmp=yes
        SNMPFLAGS=`net-snmp-config --cflags | perl -p -e 's/-O\S*//g'`
        SNMPLIBS=`net-snmp-config --libs`
        AC_MSG_RESULT(yes)
    ],
    [AC_MSG_RESULT(no.  No SNMP based plugins can be built!)])
])


AC_DEFUN([OH_CHECK_OPENIPMI],
	[
	AC_MSG_CHECKING(for OpenIPMI)

	OH_OI_FILE=ipmi_ver
	OH_OI_SRC="ipmi_ver.c"
	echo "#include <stdio.h>" > $OH_OI_SRC
	echo "#include <OpenIPMI/ipmiif.h>" >> $OH_OI_SRC
	echo "int main() {" >> $OH_OI_SRC
	echo "printf(\"%d.%d.%d\", OPENIPMI_VERSION_MAJOR, \
				   OPENIPMI_VERSION_MINOR, \
				   OPENIPMI_VERSION_RELEASE);" >> $OH_OI_SRC
	echo "return 0;}" >> $OH_OI_SRC

	gcc -o $OH_OI_FILE $CFLAGS $CPPFLAGS $LDFLAGS $OH_OI_SRC >& /dev/null

	if test -f "ipmi_ver"; then
		OPENIPMI_VERSION=`./ipmi_ver | \
		awk -F. '{ \
			if ( $[1] == $1 ) { \
				if ( $[2] == $2 ) { \
			    	if ( $[3] >= $3 ) { \
			    		print "OK"; \
					} \
				}
			}

		}'` 
	fi
		
	if test "$OPENIPMI_VERSION" == "OK"; then
		have_openipmi=yes
		AC_MSG_RESULT(yes)
	else
		AC_MSG_RESULT(no...OpenIPMI missing or wrong version IPMI plug-in can't build)
		have_openipmi=no
   	fi

	rm -rf $OH_OI_FILE $OH_OI_SRC
])

AC_DEFUN([PKG_CFG_SETPATH],
[
	if test -f "/etc/ld.so.conf"; then
		TEMP=`cat /etc/ld.so.conf | grep "/lib$"`
		TEMP=`echo $TEMP | sed -e 's/\/lib \|\/lib$/\/lib\/pkgconfig:/g'`
		PKG_CONFIG_PATH="${PKG_CONFIG_PATH}:${TEMP}"
	fi
])

AC_DEFUN([OH_CHECK_RTAS],
    [
    AC_MSG_CHECKING(for RTAS libary)
    AC_TRY_COMPILE(
        [
	    #include <stdio.h>
	    #include <librtas.h>
        ],    
        [
            rtas_activate_firmware();
        ],
        [
	    if test -f "/usr/bin/lsvpd"; then
		    have_rtas_lib=yes
        	    AC_MSG_RESULT(yes)
	    else
		    AC_MSG_RESULT(no)
	    fi
       ],
       [AC_MSG_RESULT(no)]
    )])
