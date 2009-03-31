dnl
dnl LTP_CHECK_CRYPTO
dnl ----------------------------
dnl
AC_DEFUN([LTP_CHECK_CRYPTO],
[dnl
AC_CHECK_LIB([crypto],[SHA1_Init],[CRYPTO_LIB=-lcrypto],[CRYPTO_LIB=])
AC_SUBST(CRYPTO_LIB)
])
