/*
********************************************************************************
File: wav2cdr.c

Tab size:           4
Max line length:    80
Programmer:         Volker Kuhlmann


wav2cdr 2.3.4 Copyright (C) 1997, 1998, 1999, 2000, 2006 Volker Kuhlmann
This program is free software under the terms of the GNU General Public License
version 2 (or later, at your option).
See the file COPYING for details about license terms and warranty.
<VolkerKuhlmann@gmx.de>


DESCRIPTION:

Conversion utility for wav-files to files which can be written on a CD.
Reads from stdin, writes to stdout or file(s).
Can also read and write some raw formats.
Can perform various byte swapping operations as well as scaling of data,
and cutting the input into tracks.
Reading from file is not yet supported.
Call with -help for more info.

Written with strict ANSI conformance.
Compiles without any problems with gcc under Solaris and Linux.
If you create Makefiles for other systems please send me a copy.


CONDITIONALS:
	NO_ASSERTMANY	Don't use so many asserts
	DEBUG			Writes some debugging output to msgfile.
	HAS_GNUGETOPT	The program will be linked with GNU getopt(). The GNU
					getopt.h must be available to the compiler. See the file
					README for details.
	MSDOS_BC		When compiling with Borland C under MSDOS.
	

HISTORY:

2.1   03May98	See ChangeLog
2.0   25Mar98	Added silence detection and a few minor things.
1.9   20Jan98	Fixed bug in write_wav_header().
1.8   11Dec97	Put under GPL.
1.7   08Dec97	Cut number units, negative cut numbers, adding silence.
1.6   06Dec97	My own getopt.
1.5   05Dec97	Converting to mono/stereo. Minor changes.
1.4   02Dec97	Restructured, split into modules. It is now more flexible and
				extendible. wav headers are written correctly. Handling of 
				system error strings fixed.
				Fixed a few bugs, probably introduced some.
1.3				You don't wanna know.
1.2   30Nov97	Writing wav files (preliminary).
1.1   29Nov97	Added execution time display. Fixed stdout bug. Simplified some
				lines.
1.0   28Nov97	First complete release.
0.3   26Nov97	Improved.
0.2   25Nov97	Commented, improved.
0.1   25Nov97	Created.

********************************************************************************
*/



#define PROGVERSION "Version 2.3.4  Copyright (C) 18 Jan 2006 by Volker Kuhlmann"
/*
#define PROGVERSION "Version 2.3.4  Copyright (C) 18 Jan 2006 by Volker Kuhlmann"
#define PROGVERSION "Version 2.3.3  Copyright (C) 27 Oct 2000 by Volker Kuhlmann"
#define PROGVERSION "Version 2.3.2  Copyright (C) 22 Aug 1999 by Volker Kuhlmann"
#define PROGVERSION "Version 2.3.1  Copyright (C) 18 Aug 1999 by Volker Kuhlmann"
#define PROGVERSION "Version 2.3  Copyright (C) 19 Jul 1999 by Volker Kuhlmann"
#define PROGVERSION "Version 2.2.2  Copyright (C) 07 Jun 1999 by Volker Kuhlmann"
#define PROGVERSION "Version 2.2.1  Copyright (C) 03 May 1999 by Volker Kuhlmann"
#define PROGVERSION "Version 2.2  Copyright (C) 21 Feb 1999 by Volker Kuhlmann"
#define PROGVERSION "Version 2.2  Copyright (C) 04 Feb 1999 by Volker Kuhlmann"
#define PROGVERSION "Version 2.1.2  Copyright (C) 03 Feb 1999 by Volker Kuhlmann"
#define PROGVERSION "Version 2.1  Copyright (C) 03 May 1998 by Volker Kuhlmann"
#define PROGVERSION "Version 2.0  Copyright (C) 27 Mar 1998 Volker Kuhlmann"
#define PROGVERSION "Version 2.0ß3  Copyright (C) 18 Mar 1998 Volker Kuhlmann"
#define PROGVERSION "Version 2.0ß2  Copyright (C) 13 Mar 1998 Volker Kuhlmann"
#define PROGVERSION "Version 2.0ß  Copyright (C) 10 Mar 1998 Volker Kuhlmann"
#define PROGVERSION "Version 1.9  Copyright (C) 20 Jan 1998 Volker Kuhlmann"
#define PROGVERSION "Version 1.8  Copyright (C) 11 Dec 1997 Volker Kuhlmann"
#define PROGVERSION "Version 1.7  (C) VK 8 Dec 1997"
#define PROGVERSION "Version 1.6  (C) VK 6 Dec 1997"
#define PROGVERSION "Version 1.5  (C) VK 5 Dec 1997"
#define PROGVERSION "Version 1.5  (C) VK 4 Dec 1997"
*/



#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <assert.h>
#include <time.h>

#include "chelp.h"

#include "wav2cdr.h"


/*
	Global + local variables
*/
FILE
	*msgfile = NULL;	/* where to display cmd args and progress */

#ifdef DEBUG
FILE
	*dbgfile = NULL;	/* where to display debug output */
#endif
						/* init these 2 to NULL just in case */


/*
	Local function prototypes
*/
static void version_text (void);



/*
	Display usage and exit.
	In: ---
*/
static void version_text (void)
{
	fprintf (msgfile,
#include "version.-c"
	, PROGVERSION
#ifdef HAS_GNUGETOPT
	, "GNU getopt()"
#else
	, "mygetopt()"
#endif
	);
} /* version_text() */

void version (void)
{
	version_text ();
	exit_error (ERR_USAGE, NULL, NULL);
} /* version() */



/*
	Display usage and exit.
	In: ---
*/
void usage (void)
{
	version_text ();
	fprintf (msgfile,
#include "usage.-c"
	);

	showcmdargs ();
	exit_error (ERR_USAGE, NULL, NULL);

} /* usage() */



/*
	Display help and exit.
	In: ---
*/
void help (void)
{
	fprintf (msgfile,
#include "help.-c"
	, CDAUDIOSAMPLINGRATE
	, CDAUDIOSECTORSIZE
	, CDSECTORSPERSEC
	);

	exit_error (ERR_USAGE, NULL, NULL);

} /* help() */



/*
	Display error msg and exit with error code.
	Also displays the system error text if error flag set.
	Close files still open (no error checking with that one).
	In: exit code, up to 2 error strings or NULL
	Out: ---
	Return: ---
*/
void exit_error (exit_t err, const char *errtext, const char *errtext2)
{
	if (errtext != NULL) {
		fprintf (stderr, "wav2cdr: error: %s%s", 
						errtext, errtext2 == NULL ? "" : errtext2);
	}
	if (err == ERR_IO) {
		if (errno != 0)
			fprintf (stderr, " (%s)", strerror (errno));
	}
	if (errtext != NULL  OR  err == ERR_IO)
		putc ('\n', stderr);
	if (err == ERR_CMDARG)
		fprintf (stderr,
			"Use -h or -u or --usage for usage, or --help for more help\n");

	emergency_close (); /* emergency close files */
	
	exit ((int) err);

} /* exit_error() */



/*
	In: ---
	Out: ---
	Return: endianness of local host (TRUE if little)
*/
BOOL is_localhost_little (void)
{
UINT16 lclfmt = 1;
UINT8 *is_little = (UINT8 *) &lclfmt;

	/* make sure this works */
	assert (sizeof(UINT16) == 2 * sizeof(UINT8));
	
	#ifdef DEBUG
	#ifdef LITTLE_ENDIAN
	  return TRUE;
	#endif
	#ifdef BIG_ENDIAN
	  return FALSE;
	#endif
	#endif

	/* test endianness of local machine */
	return (*is_little != 0);
	
} /* is_localhost_little() */



/*
	Format a time as "MM:SS.ss".
	In: buffer, time in (seconds * CDSECTORSPERSEC * CDAUDIOSECTORSIZE)
	Out: formatted time; at most 10 bytes (TIMESTRSIZE) incl '\0'
	Return: return value of sprintf
*/
int timeprintf (string *buf, unsigned long bytes)
{
unsigned short
	m,			/* minutes */
	s,			/* seconds */
	fs; 		/* fractional second */

	s = (bytes / CDAUDIOSECTORSIZE) / CDSECTORSPERSEC;
	fs = (bytes - (unsigned long) s * CDAUDIOBYTESPERSEC)
	     * 100 / ((unsigned long) CDAUDIOBYTESPERSEC);
	m = s / 60;
	s = s % 60;

	return sprintf (buf, "%02d:%02d.%02d", m, s, fs);
	
} /* timeprintf */



int main (int argc, char *argv[])
{
	assert (sizeof(UINT8)  == 1);
	assert (sizeof(UINT16) == 2);
	assert (sizeof(UINT32) == 4);
	assert (sizeof(SINT16) == 2);
	assert (sizeof(SINT32) == 4);
	
	assert (sizeof(audiosect_t) >= sizeof(wav_header_t));
	assert (sizeof(audiosect_t) % 4 == 0);
	/* assert (sizeof(buffer_t) >= CDAUDIOSECTORSIZE); */

	/* This program assumes that a short is 2 bytes, and that SHRT_MAX is
		the max value of a signed short. (Why doesn't limit.h bloody have
		a SSHRT_MAX?) */
	assert (SHRT_MAX == USHRT_MAX/2);
	/* actually, better might be: */
	assert ((long)SHRT_MAX == 32767L);
	
	scan_cmd_args (argc, argv);
	set_message_output ();
	check_cmd_args ();
	showcmdargs ();

	do_data_io ();
	
	return ERR_OK;
	
} /* main() */



/* EOF wav2cdr.c */
/******************************************************************************/
