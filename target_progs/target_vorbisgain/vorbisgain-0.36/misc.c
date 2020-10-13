/*
 * Misc utility functions
 *
 * This program is distributed under the GNU General Public License, version 
 * 2.1. A copy of this license is included with this source.
 *
 * Copyright (C) 2002, 2004 Gian-Carlo Pascutto and Magnus Holmgren
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vorbis/codec.h>
#include "i18n.h"
#include "misc.h"

#ifdef WIN32
#include <windows.h>
#else /* WIN32 */
#include <errno.h>
#include <ctype.h>

#ifndef DISABLE_WINSIZE

#if HAVE_TERMIOS_H
#include <termios.h>
#endif

#if GWINSZ_IN_SYS_IOCTL
#include <sys/ioctl.h>
#endif

#endif /* DISABLE_WINSIZE */
#endif /* WIN32 */


/**
 * \brief Display a file (or other I/O) error message.
 *
 * Display a file (or other I/O) error message. First a message is formatted
 * and printed, followed by the error text, terminated by a line feed.
 *
 * \param message  message format to display.
 * \param ...      printf-arguments used to format the message.
 */
void file_error(const char* message, ...)
{
    int err_num = errno;
    va_list args;

    va_start(args, message);
    vfprintf(stderr, message, args);
    va_end(args);

    fprintf(stderr, strerror(err_num));
    fprintf(stderr, "\n");
}


/**
 * \brief Display a Vorbis error message.
 *
 * Display a Vorbis error message. First a message is formatted and printed, 
 * followed by the error text, terminated by a line feed.
 *
 * \param vorbis_error  the vorbis error to display.
 * \param message       message format to display.
 * \param ...           printf-arguments used to format the message.
 */
void vorbis_error(int vorbis_error, const char* message, ...)
{
    va_list args;

    va_start(args, message);
    vfprintf(stderr, message, args);
    va_end(args);

    switch (vorbis_error)
    {
    case OV_EREAD:
        fprintf(stderr, _("Read error\n"));
        break;

    case OV_ENOTVORBIS:
        fprintf(stderr, _("File does not contain Vorbis data\n"));
        break;

    case OV_EVERSION:
        fprintf(stderr, _("Vorbis version mismatch\n"));
        break;

    case OV_EBADHEADER:
        fprintf(stderr, _("Invalid Vorbis bitstream header\n"));
        break;

    case OV_EFAULT:
        fprintf(stderr, _("Internal Vorbis error\n"));
        break;

    case OV_EINVAL:
        fprintf(stderr, _("Invalid argument value\n"));
        break;

    case OV_EBADLINK:
        fprintf(stderr, _("Invalid stream section or requested link corrupt"));
        break;

    case OV_ENOSEEK:
        fprintf(stderr, _("File is not seekable\n"));
        break;

    case OV_EIMPL:
        fprintf(stderr, _("Function not yet implemented\n"));
        break;

    default:
        fprintf(stderr, _("An unknown error occured (%d)\n"), vorbis_error);
        break;
    }
}


/**
 * \brief Compare two tag names for equality.
 *
 * Compare two tag names for equality. The comparison ends when one string 
 * ends or an equal sign ("=") is encountered, whichever comes first.
 *
 * \param s1  first string to compare.
 * \param s2  second string to compare.
 * \return  0 if the tag names matches, otherwise the difference between the 
 *          two characters (after uppercase conversion).
 */
int tag_compare(const char *s1, const char *s2)
{
    int c;
    int n = strlen(s2);
    
    for (c = 0; c < n; c++)
    {
        if ((s1[c] == '=') || (s1[c] == 0) || (s2[c] == '=') || (s2[c] == 0))
        {
            break;
        }

        if (toupper(s1[c]) != toupper(s2[c]))
        {
            return toupper(s1[c]) - toupper(s2[c]);
        }
    }

    if (((s1[c] != '=') && (s1[c] != 0)) || ((s2[c] != '=') && (s2[c] != 0)))
    {
        /* Tag names not properly ended */
        return toupper(s1[c]) - toupper(s2[c]);
    }

    return 0;
}


/**
 * \breif Get the last component in a path. 
 *
 * Get the last component in a path. If no directory separator is found, 
 * return the path as is.
 *
 * \param path  path to get last component of.
 * \return  the last path component, or path.
 */    
char* last_path(const char* path)
{
    int i;

    for (i = strlen(path) - 1; i >= 0; i--)
    {
#ifdef WIN32
        if ((path[i] == '\\') || (path[i] == ':'))
#else
        if (path[i] == '/')
#endif
        {
            return (char*) &path[i + 1];
        }
    }

    return (char*) path;
}

#ifdef WIN32
/**
 * \breif Get the size - in columns and rows - of the output console.
 *
 * Get the size - in columns and rows - of the output console, if available.
 *
 * \param get_err  if TRUE, get the size of the stderr console, otherwise get 
 *                 the size of the stdout console.
 * \param columns  output argument for number of columns.
 * \param rows     output argument for number of rows.
 * \return  0 for success and -1 for failure.
 */    
int get_console_size(int get_err, unsigned int* columns, unsigned int* rows)
{
    CONSOLE_SCREEN_BUFFER_INFO info;
    HANDLE handle;

    handle = GetStdHandle(get_err ? STD_ERROR_HANDLE : STD_OUTPUT_HANDLE);
    
    if (INVALID_HANDLE_VALUE != handle)
    {
        if (GetConsoleScreenBufferInfo(handle, &info))
        {
            *columns = info.srWindow.Right - info.srWindow.Left + 1;
            *rows = info.srWindow.Bottom - info.srWindow.Top + 1;
            return 0;
        }
    }

    return -1;
}
#else /* WIN32 */
/**
 * \brief Get the value of an environment variable, assuming it is numeric.
 *
 * \param name   environment variable to get.
 * \param value  where to store the result value. Only set if 0 is returned.
 * \return  0 if a numeric value was found, -1 otherwise.
 */
static int getenv_num(const char* name, int* value)
{
    char* env = getenv(name);
    
    if (NULL != env)
    {
        char* end = NULL;
        int val = strtol(env, &end, 10);

        if (env != end)
        {
            /* Something was found. */
            *value = val;
            return 0;
        }
    }
    
    return -1;
}

/**
 * \breif Get the size - in columns and rows - of the output console.
 *
 * Get the size - in columns and rows - of the output console, if available.
 *
 * \param get_err  if TRUE, get the size of the stderr console, otherwise get 
 *                 the size of the stdout console.
 * \param columns  output argument for number of columns.
 * \param rows     output argument for number of rows.
 * \return  0 for success and -1 for failure.
 */    
int get_console_size(int get_err, unsigned int* columns, unsigned int* rows)
{
    int fd = fileno(get_err ? stderr : stdout);

    if (isatty(fd))
    {
        int c;
        int r;
        
        c = r = 0;

        getenv_num("COLUMNS", &c);
        getenv_num("LINES", &r);

        if ((0 < c) && (0 < r))
        {
            *columns = c;
            *rows = r;
            return 0;
        }
#ifndef DISABLE_WINSIZE
        else
        {
            struct winsize size;

            if (!ioctl(fd, TIOCGWINSZ, &size))
            {
                *columns = size.ws_col;
                *rows = size.ws_row;
                return 0;
            }
        }
#endif /* DISABLE_WINSIZE */
    }

    return -1;
}
#endif /* WIN32 */
