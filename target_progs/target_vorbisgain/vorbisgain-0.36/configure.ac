# Process this file with autoconf to produce a configure script.

# ------------------------------------------------
# Initialization
# --------------------------------------------------

AC_INIT(vorbisgain,0.36)
AC_CONFIG_SRCDIR(vorbisgain.c)
AM_INIT_AUTOMAKE
AM_CONFIG_HEADER(config.h)

# ------------------------------------------------
# Checks for programs.
# --------------------------------------------------

# save $CFLAGS since AC_PROG_CC likes to insert "-g -O2"
# if $CLFAGS is blank
cflags_save="$CFLAGS"
AC_PROG_CC
CFLAGS="$cflags_save"

# --------------------------------------------------
# Set build flags based on environment
# --------------------------------------------------

cflags_save="$CFLAGS"
ldflags_save="$LDFLAGS"
if test -z "$GCC"; then
        case $host in
        *)
                DEBUG="-g"
                CFLAGS="-O"
                PROFILE="-g -p"
                ;;
        esac
else
        case $host in
        *)
                DEBUG="-g -Wall -fsigned-char"
                CFLAGS="-O2 -fsigned-char"
                PROFILE="-O2 -g -pg -fsigned-char"
                ;;
        esac
fi
CFLAGS="$CFLAGS $cflags_save"
DEBUG="$DEBUG $cflags_save"
PROFILE="$PROFILE $cflags_save"
LDFLAGS="$LDFLAGS $ldflags_save"

# --------------------------------------------------
# Check for libraries
# --------------------------------------------------

AM_PATH_OGG(,AC_MSG_ERROR([*** libogg not installed - please install first ***]))
AM_PATH_VORBIS(,AC_MSG_ERROR([*** libvorbis not installed - please install first ***]))

# --------------------------------------------------
# Checks for header files.
# --------------------------------------------------

AC_HEADER_DIRENT
AC_HEADER_STDC
AC_HEADER_TIOCGWINSZ
AC_CHECK_HEADERS([errno.h stdlib.h string.h unistd.h utime.h termios.h])

# --------------------------------------------------
# Checks for typedefs, structures, and compiler characteristics.
# --------------------------------------------------

AC_C_CONST
AC_C_INLINE
AC_TYPE_SIZE_T

# --------------------------------------------------
# Checks for library functions.
# --------------------------------------------------

# GCP: This does very wrong things on my Debian sid box
#AC_FUNC_MALLOC

AC_FUNC_STAT
AC_FUNC_VPRINTF
AC_CHECK_FUNCS([floor getcwd memmove memset pow strchr strdup strerror])

AC_CHECK_LIB(vorbisfile,ov_read_float,,
    AC_MSG_ERROR([*** libvorbisfile does not contain ov_read_float - please upgrade ***]),
    [$VORBIS_LIBS $OGG_LIBS])

CFLAGS_save=$CFLAGS
LIBS_save=$LIBS
CFLAGS="$CFLAGS $OGG_CFLAGS $VORBIS_CFLAGS"
LIBS="$LIBS $VORBIS_LIBS $OGG_LIBS"

AC_MSG_CHECKING(if ov_read_float() takes four arguments)
AC_TRY_COMPILE(
    [#include <vorbis/vorbisfile.h>],
    [OggVorbis_File vf;
     float **pcm;
     ov_read_float(&vf, &pcm, 1024, 0);],
    [AC_MSG_RESULT(yes)],
    [AC_MSG_RESULT(no)
     AC_DEFINE(VORBISFILE_OV_READ_FLOAT_3_ARGS,,
         [Define if ov_read_float only takes 3 args])])

CFLAGS=$CFLAGS_save
LIBS=$LIBS_save

# --------------------------------------------------
# Check for features to enable
# --------------------------------------------------

AC_ARG_ENABLE(recursive,
    AC_HELP_STRING([--enable-recursive],
        [enable recursive processing and pattern matching]),
    AC_DEFINE(ENABLE_RECURSIVE,,[Enable recursive processing and pattern matching]),)

AC_ARG_ENABLE(winsize,
    AC_HELP_STRING([--disable-winsize],
        [disable getting console window size using ioctl()]),
    AC_DEFINE(DISABLE_WINSIZE,,[Disable getting console window size using ioctl()]),)

AC_OUTPUT(Makefile)
