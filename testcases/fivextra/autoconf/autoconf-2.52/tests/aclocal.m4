# actest.m4                                              -*- autoconf -*-
# Additional Autoconf macros to ease testing.

# AC_STATE_SAVE(FILE)
# ------------------
# Save the environment, except for those variables we are allowed to touch.
# This is to check no test touches the user name space.
# FIXME: There are surely better ways.  Explore for instance if
# we can ask help from AC_SUBST.  We have the right to touch what
# is AC_SUBST'ed.
# - ^ac_
#   Autoconf's shell name space.
# - prefix and exec_prefix
#   are kept undefined (NONE) until AC_OUTPUT which then sets them to
#   `/usr/local' and `${prefix}' for make.
# - CONFIG_STATUS and DEFS
#   Set by AC_OUTPUT.
# - F77_DUMMY_MAIN
#   Set by AC_F77_DUMMY_MAIN.
# - ALLOCA|NEED_SETGID|KMEM_GROUP
#   AC_FUNCs from acspecific.
# - AWK|LEX|LEXLIB|LEX_OUTPUT_ROOT|LN_S|M4|RANLIB|SET_MAKE|YACC
#   AC_PROGs from acspecific
# - _|@|.[*#?].|LINENO|OLDPWD|PIPESTATUS|RANDOM|SECONDS
#   Some variables some shells use and change.
#   `.[*#?].' catches `$#' etc. which are displayed like this:
#      | '!'=18186
#      | '#'=0
#      | '$'=6908
# - POW_LIB
#   From acfunctions.m4.
#
# Some `egrep' choke on such a big regex (e.g., SunOS 4.1.3).  In this
# case just don't pay attention to the env.  It would be great
# to keep the error message but we can't: that would break AT_CHECK.
m4_defun([AC_STATE_SAVE],
[(set) 2>&1 |
  egrep -v -e 'm4_join([|],
      [^a[cs]_],
      [^((exec_)?prefix|DEFS|CONFIG_STATUS)=],
      [^(CC|CFLAGS|CPP|GCC|CXX|CXXFLAGS|CXXCPP|GXX|F77|FFLAGS|FLIBS|G77)=],
      [^(LIBS|LIBOBJS|LDFLAGS)=],
      [^INSTALL(_(DATA|PROGRAM|SCRIPT))?=],
      [^(CYGWIN|ISC|MINGW32|MINIX|EMXOS2|XENIX|EXEEXT|OBJEXT)=],
      [^(X_(CFLAGS|(EXTRA_|PRE_)?LIBS)|x_(includes|libraries)|(have|no)_x)=],
      [^(host|build|target)(_(alias|cpu|vendor|os))?=],
      [^(cross_compiling)=],
      [^(interpval|PATH_SEPARATOR)=],
      [^(F77_DUMMY_MAIN|f77_(case|underscore))=],
      [^(ALLOCA|GETLOADAVG_LIBS|KMEM_GROUP|NEED_SETGID|POW_LIB)=],
      [^(AWK|LEX|LEXLIB|LEX_OUTPUT_ROOT|LN_S|M4|RANLIB|SET_MAKE|YACC)=],
      [^(_|@|.[*#?].|LINENO|OLDPWD|PIPESTATUS|RANDOM|SECONDS)=])' 2>/dev/null |
  # There maybe variables spread on several lines, eg IFS, remove the dead
  # lines.
  grep '^m4_defn([m4_re_word])=' >state-env.$1
test $? = 0 || rm -f state-env.$1

ls -1 | egrep -v '^(at-|state-|config\.)' | sort >state-ls.$1
])# AC_STATE_SAVE
