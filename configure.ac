#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.67])
AC_INIT([nife], [0.44], [nife@seriane.fr])
AM_INIT_AUTOMAKE

AC_CONFIG_SRCDIR([src/tasks.c])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CC
AC_PROG_MAKE_SET

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([arpa/inet.h fcntl.h inttypes.h netdb.h stdlib.h string.h strings.h sys/socket.h sys/time.h termios.h unistd.h])
AC_CHECK_HEADERS([pthread.h])
AC_CHECK_HEADERS([comedilib.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_TYPE_PID_T

# Checks for library functions.
AC_FUNC_FORK
AC_FUNC_MALLOC
AC_FUNC_STRTOD
AC_CHECK_FUNCS([bzero dup2 floor gethostbyname gettimeofday inet_ntoa pow rint socket strchr])

AC_SEARCH_LIBS(pow,m)
AC_SEARCH_LIBS(comedi_get_cmd_src_mask,comedi)
AC_SEARCH_LIBS(socket,socket)
AC_SEARCH_LIBS(pthread_create,pthread)
AC_SEARCH_LIBS(inet_ntoa,nsl)

AC_CONFIG_FILES([Makefile src/Makefile])
AC_OUTPUT
