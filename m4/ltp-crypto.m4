dnl
dnl LTP_CHECK_CRYPTO
dnl ----------------------------
dnl
AC_DEFUN([LTP_CHECK_CRYPTO],
[dnl
AC_CHECK_HEADERS(openssl/sha.h,[CRYPTO_LIB=-lcrypto],[CRYPTO_LIB=])
AC_SUBST(CRYPTO_LIB)
])
