## --------------------------------------------------------- ##
## Fake the existence of programs that GNU maintainers use.  ##
## --------------------------------------------------------- ##

# serial 2

# AM_MISSING_PROG(NAME, PROGRAM)
# ------------------------------
AC_DEFUN([AM_MISSING_PROG],
[AC_REQUIRE([AM_MISSING_HAS_RUN])
$1=${$1-"${am_missing_run}$2"}
AC_SUBST($1)])


# AM_MISSING_INSTALL_SH
# ---------------------
# Like AM_MISSING_PROG, but only looks for install-sh.
AC_DEFUN([AM_MISSING_INSTALL_SH],
[AC_REQUIRE([AM_MISSING_HAS_RUN])
if test -z "$install_sh"; then
   for install_sh in "$ac_aux_dir/install-sh" \
                     "$ac_aux_dir/install.sh" \
                     "${am_missing_run}${ac_auxdir}/install-sh";
   do
     test -f "$install_sh" && break
   done
   # FIXME: an evil hack: we remove the SHELL invocation from
   # install_sh because automake adds it back in.  Sigh.
   install_sh=`echo $install_sh | sed -e 's/\${SHELL}//'`
fi
AC_SUBST(install_sh)])


# AM_MISSING_HAS_RUN
# ------------------
# Define MISSING if not defined so far and test if it supports --run.
# If it does, set am_missing_run to use it, otherwise, to nothing.
AC_DEFUN([AM_MISSING_HAS_RUN],
[test x"${MISSING+set}" = xset ||
  MISSING="\${SHELL} `CDPATH=:; cd $ac_aux_dir && pwd`/missing"
# Use eval to expand $SHELL
if eval "$MISSING --run true"; then
  am_missing_run="$MISSING --run "
else
  am_missing_run=
  am_backtick='`'
  AC_MSG_WARN([${am_backtick}missing' script is too old or missing])
fi
])

# AM_AUX_DIR_EXPAND

# For projects using AC_CONFIG_AUX_DIR([foo]), Autoconf sets
# $ac_aux_dir to ${srcdir}/foo.  In other projects, it is set to `.'.
# Of course, Automake must honor this variable whenever it call a tool
# from the auxiliary directory.  The problem is that $srcdir (hence
# $ac_aux_dir) can be either an absolute path or a path relative to
# $top_srcdir or absolute, this depends on how configure is run.  This
# is pretty anoying since it makes $ac_aux_dir quite unusable in
# subdirectories: on the top source directory, any form will work
# fine, but in subdirectories relative pat needs to be adapted.
# - calling $top_srcidr/$ac_aux_dir/missing would success if $srcdir is
#   relative, but fail if $srcdir is absolute
# - conversly, calling $ax_aux_dir/missing would fail if $srcdir is
#   absolute, and success on relative paths.
#
# Consequently, we define and use $am_aux_dir, the "always absolute"
# version of $ac_aux_dir.

AC_DEFUN([AM_AUX_DIR_EXPAND], [
# expand $ac_aux_dir to an absolute path
am_aux_dir=`CDPATH=:; cd $ac_aux_dir && pwd`
])
