/*
    FLVMeta - FLV Metadata Editor

    Copyright (C) 2007-2016 Marc Noirot <marc.noirot AT gmail.com>

    This file is part of FLVMeta.

    FLVMeta is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    FLVMeta is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with FLVMeta; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
*/
#ifndef __UTIL_H__
#define __UTIL_H__

#include <stdio.h>

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* determine whether two paths physically point to the same file */
int flvmeta_same_file(const char * file1, const char * file2);

#ifdef WIN32
/*
    Returns a descriptor to a temporary file.
    This function is meant as a Windows replacement
    to the broken standard tmpfile() function.
*/
FILE * flvmeta_tmpfile(void);

#else /* WIN32 */
# define flvmeta_tmpfile tmpfile
#endif /* WIN32 */

/*
    File size (LFS compatible).
    Returns a non-zero value if successful, zero otherwise.
*/
int flvmeta_filesize(const char * filename, file_offset_t * filesize);

#ifndef HAVE_ISFINITE
/*
    Check whether a double is finite (not infinity or NaN)
*/
int flvmeta_isfinite(double d);
#endif /* HAVE_ISFINITE */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __UTIL_H__ */
