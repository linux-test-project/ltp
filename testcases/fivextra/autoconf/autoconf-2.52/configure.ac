#                                                       -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.
AC_PREREQ(2.50)
AC_INIT(GNU Autoconf, 2.52, bug-autoconf@gnu.org)
AC_CONFIG_SRCDIR(acgeneral.m4)
AC_CONFIG_AUX_DIR(config)
AM_INIT_AUTOMAKE(autoconf, 2.52)

# Initialize the test suite.
AT_CONFIG(..)
AC_PATH_PROG(EXPR, expr)

# We use a path for GNU m4 so even if users have another m4 first in
# their path, the installer can configure with a path that has GNU m4
# first and get that path embedded in the installed autoconf and
# autoheader scripts.
AC_PROG_GNU_M4
if test x"$ac_cv_prog_gnu_m4" != xyes; then
  AC_MSG_ERROR([GNU m4 1.4 is required])
fi
# This is needed because Automake does not seem to realize there is
# a AC-SUBST inside AC-PROG-GNU-M4.  Grmph!
AC_SUBST(M4)

# `autoconf' and `ifnames' use AWK.  And we need decent RE support.
AC_PROG_AWK
# Some AWK fail
if echo xfoo | $AWK '/foo|^bar$/ { print }' | grep xfoo >/dev/null; then :; else
  AC_MSG_ERROR([the regex engine of $AWK is too broken to be used
                you might want to install GNU AWK])
fi

# Generating man pages.
AM_MISSING_PROG(HELP2MAN, help2man)

# We use a path for perl so the #! line in autoscan will work.
AC_PATH_PROG(PERL, perl, no)
AC_SUBST(PERL)dnl
AC_SUBST(PERLSCRIPTS)dnl
if test "$PERL" != no; then
  PERLSCRIPTS="autoscan autoupdate"
else
  AC_MSG_WARN([autoscan and autoupdate will not be built since perl is not found])
fi

AC_PROG_INSTALL

# Automake can't see inner AC_SUBSTS (`aclocal' is bypassed), so we tag the
# AC_SUBSTS here too.
AC_SUBST(PACKAGE_NAME)
AC_SUBST(PACKAGE)
AC_SUBST(VERSION)

AC_CONFIG_FILES(Makefile m4/Makefile man/Makefile doc/Makefile config/Makefile
                tests/Makefile tests/atconfig)

AC_OUTPUT

# Report the state of this version of Autoconf if this is a beta.
case AC_PACKAGE_VERSION in
  *[[a-z]]*)
    cat <<EOF

You are about to use an experimental version of Autoconf.  Be sure to
read the relevant mailing lists, most importantly <autoconf@gnu.org>.

Below you will find information on the status of this version of Autoconf.


EOF
    sed -n '/^\* Status/,$p' $srcdir/BUGS;;
esac
