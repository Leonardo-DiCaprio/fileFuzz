/*
********************************************************************************
File: fileio.c

Tab size:           4
Max line length:    80
Programmer:         Volker Kuhlmann


wav2cdr 2.3.4 Copyright (C) 1997, 1998, 1999, 2000, 2006 Volker Kuhlmann
This program is free software under the terms of the GNU General Public License
version 2 (or later, at your option).
See the file COPYING for details about license terms and warranty.
<VolkerKuhlmann@gmx.de>


DESCRIPTION:

This module handels all physical data I/O, and file formats.


CONDITIONALS:
	see wav2cdr.c


HISTORY:
	see wav2cdr.c, and ChangeLog

********************************************************************************
*/



#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <time.h>

#include "chelp.h"

#include "wav2cdr.h"



/* exported */

unsigned long
	bytes_in,			/* bytes written to current input/output file, set */
	bytes_out;			/* to 0 when opening file, unchanged when closing */

string
	outname[PATH_MAX];	/* output filename (if not to stdout), unchanged
							by closing file */

/* imported */

extern FILE
	*msgfile;			/* where to display cmd args and progress */

/* local */

static FILE
 	*infile = NULL,		/* file ptr for input and output data */
	*outfile = NULL;



/*
	Opens an input file.
	In: filename or null for stdin
	Out: ---
	Return: ---
*/
void open_input_file (const string *name)
{
	bytes_in = 0;

	/* do not open if stdin */
	if (name == NULL) {
		infile = stdin;
	} else {
		fprintf (msgfile, "Opening file(r):   %s\n-\n", name);

		infile = fopen (name, "rb");
		if (infile == NULL)
			exit_error (ERR_IO, "opening file ", name);
	}
} /* open_input_file() */



/*
	Close input file
*/
void close_input_file (void)
{
int r = 0;

	if (infile != stdin)
		r = fclose (infile);
	infile = NULL;
	if (r != 0)
		exit_error (ERR_IO, "closing input", NULL);

} /* close_input_file() */



/*
	Return the size of the given file (or stream if name is NULL) in bytes, or
	-1 if not obtainable.
	This function assumes that ftell() returns an offset in bytes.
	Currently, the max file size which can be handled is 2GB... (ANSI C limit).
*/
signed long get_file_size (const string *name, FILE *stream)
{
FILE *file;
fpos_t pos;
long size;
int r;

	/* in case of stdin, we can try to seek, but we must seek back and not
		close */
	if (name == NULL)
		file = stream;
	else {
		file = fopen (name, "rb");
		if (file == NULL) {
			fprintf (msgfile, "Failed to open file %s (%s)\n", 
						name, strerror (errno));
			return -1;
		}
	}

	/* file is open with no error */
	fgetpos (file, &pos);
	r = fseek (file, 0L, SEEK_END);
	DBGPRINTF1 ("get_file_size(): fseek %li ", (long) r);
	if (r == 0) {
		size = ftell (file);
		DBGPRINTF1 ("ftell(): %li", size);
		/* return value of ftell() might not be in bytes on all systems? */
		fsetpos (file, &pos);
	} else
		/* fseek() error */
		size = -1;
	DBGPRINTF0 ("\n");

	if (name != NULL)
		fclose (file);
		
	return (signed long) size;
	
} /* get_file_size() */



/*
	Opens an output file.
	Needs the cardinal number of the output file as this becomes part of the
	file name (unless writing to stdout, or 0).
	In: filename or null, cardinal number of output file (track number)
	Out: ---
	Return: ---
*/
void open_output_file (const string *name, int track)
{
size_t l;

	bytes_out = 0;

	/* do not open if stdout */
	if (name == NULL) {
		outfile = stdout;
	} else {
		strncpy (outname, name, (size_t) PATH_MAX);
		outname[PATH_MAX - 1] = '\0';
		if (track != 0) {
			outname[PATH_MAX - 14] = '\0';
			l = strlen (outname);
			sprintf (outname + l, ".%02i", track);
		}
		fprintf (msgfile, "Opening file(w):   %s\n", outname);
	
		outfile = fopen (outname, "wb");
		if (outfile == NULL)
			exit_error (ERR_IO, "opening file ", outname);
	}
} /* open_output_file() */



/*
	Close output file
*/
void close_output_file (void)
{
int r = 0;

	r = fclose (outfile);
	outfile = NULL;
	if (r != 0)
		exit_error (ERR_IO, "closing file", NULL);

} /* close_output_file() */



FILE *open_message_file (void)
{
FILE *file;

#ifdef MSDOS_BC
	file = fopen ("NUL", "wb+");
#else
	file = fopen ("/dev/null", "wb+");
#endif
	if (file == NULL)
		exit_error (ERR_IO, "opening bit bucket", NULL);
	return file;
} /* open_message_file() */



void close_message_file (void)
{
int r = 0;

	r = fclose (msgfile);
	msgfile = NULL;
	if (r != 0)
		exit_error (ERR_IO, "closing bit bucket", NULL);
	
} /* close_message_file() */



/*
	Emergency close files (for error exits).
	Can't do error checking here as this is called from the error handler!
*/
void emergency_close (void)
{
	if ((infile != NULL)  AND  (infile != stdin))
		(void) fclose (infile);
	if ((outfile != NULL)  AND  (outfile != stdout))
		(void) fclose (outfile);
	if ((msgfile != NULL)  AND  (msgfile != stdout)  AND  (msgfile != stderr))
		(void) fclose (msgfile);
}



/*
	Read a wav header from the input file
	Will not return unless before all bytes have been read.
	In: buffer
	Out: wav header
	Return: ---
*/
void read_wav_header (wav_header_t *header)
{
size_t numread;
process_t proc;

	numread = fread (header, (size_t) 1, sizeof (wav_header_t), infile);
	if (numread != sizeof (wav_header_t)) {
		if (ferror (infile))
			exit_error (ERR_IO, "reading wav header", NULL);
		else
			exit_error (ERR_IO, "EOF while reading wav header", NULL);
	}

	/* do any byte swapping here */
	init_process_info (&proc);
	if (NOT proc.little_host) {
		SWAP4BYTES (header->RIFF.size);
		SWAP4BYTES (header->Format.size);
		SWAP2BYTES (header->FormatTag);
		SWAP2BYTES (header->channels);
		SWAP4BYTES (header->SamplingRate);
		SWAP4BYTES (header->AvgBytesPerSec);
		SWAP2BYTES (header->BlockAlignment);
		SWAP2BYTES (header->BitsPerSample);
		SWAP4BYTES (header->Data.size);
	}

	/* check that sampling rate etc are what we can handle */
	if (   (header->channels != 2)
		OR (header->SamplingRate != CDAUDIOSAMPLINGRATE)
		OR (header->BitsPerSample != 16)) {
		fprintf (msgfile,
			"Can only handle wav data with: \n"
			"  2 channels, %li Hz sampling rate, 16 bits per sample\n"
			"I will ignore that this wav data is different, but expect "
				"garbled data.\n"
			, (long) CDAUDIOSAMPLINGRATE);
	}	
	if (   (header->FormatTag != 1)) {
		fprintf (msgfile,
			"Ignoring unrecognised setting(s) in this wav header\n");
	}
} /* read_wav_header() */



/*
	Initialise a wav header
	Fixed values as for audi CDs only
	In: buffer, number of data bytes excl(!) wav header
	Out: wav struct
	Return: ---
*/
void make_wav_header (wav_header_t *header, unsigned long databytes)
{
static wav_header_t wh =
	{
		{"RIFF", 0 /* data size */
					+ sizeof (wav_header_t) - sizeof (wav_chunkheader_t)},
		"WAVE",
		{"fmt ", 16},
		1, 2, 44100L, 2 * 2 * 44100L, 4, 16,
		{"data", 0 /* data size here */}
	};

	*header = wh;
	header->RIFF.size += databytes;  /* insert size */
	header->Data.size = databytes;

} /* make_wav_header() */



/*
	Write a wav header to the output.
	Seeks to the start of the output first in case the data has already
	been written and the header needs to be updated when reaching the
	end of the file.
	Will not return unless correct number of bytes has been written.
	In: wav header
	Out: ---
	Return: ---
*/
void write_wav_header (const wav_header_t *header)
{
size_t numwritten;
int r;
wav_header_t wh;
process_t proc;

	wh = *header;
	
	/* do any byte swapping here */
	init_process_info (&proc);
	if (NOT proc.little_host) { /* thanks to rick@dgii.com (Rick Richardson) */
		SWAP4BYTES (wh.RIFF.size);  /* for pointing out the missing NOT */
		SWAP4BYTES (wh.Format.size);
		SWAP2BYTES (wh.FormatTag);
		SWAP2BYTES (wh.channels);
		SWAP4BYTES (wh.SamplingRate);
		SWAP4BYTES (wh.AvgBytesPerSec);
		SWAP2BYTES (wh.BlockAlignment);
		SWAP2BYTES (wh.BitsPerSample);
		SWAP4BYTES (wh.Data.size);
	}
	
	/* overwrite header at start of file (or write to empty file)
		fwrite() always writes at the end (appends) for files opened for
		read/write, but seems to work here prob because the file is opened for
		write only */
#if 0
	rewind (outfile); /* = fseek (outfile, 0L, SEEK_SET); clearerr (outfile) */
#else
	r = fseek (outfile, 0L, SEEK_SET);
	if (r != 0)
		exit_error (ERR_IO,
			"for writing wav format output must be seekable", NULL);
#endif
	numwritten = fwrite (&wh, (size_t) 1, sizeof (wav_header_t), outfile);
	if (numwritten != sizeof (wav_header_t)) {
		exit_error (ERR_IO, "writing wav header", NULL);
	}

	/* only update if we are not overwriting the start of the file */
	if (bytes_out < numwritten)
		bytes_out = numwritten;

} /* write_wav_header() */



/*
	Read block of data
	To test for EOF call read_eof().
	Note size_t might only be 16 bits.
	In: buffer, num of bytes to read
	Out: data
	Return: num of bytes read
*/
size_t read_block (void *buf, size_t bytes)
{
size_t numread;

	numread = fread (buf, (size_t) 1, bytes, infile);
	if (numread != bytes)
		if (ferror (infile))
			exit_error (ERR_IO, "reading ", NULL);

	bytes_in += numread;
	return numread;

} /* read_block() */



/*
	Return EOF condition of input stream
	In: ---
	Out: ---
	Return: TRUE if input stream has end-of-file condition
*/
BOOL read_eof (void)
{
	return feof (infile);
} /* read_eof() */



/*
	Write block of data
	Will not return if error while writing.
	Note size_t might only be 16 bits.
	In: buffer, num of bytes to write
	Out: ---
	Return: number of bytes written (= number requested)
*/
size_t write_block (const void *buf, size_t bytes)
{
size_t numwritten;

	numwritten = fwrite (buf, (size_t) 1, bytes, outfile);
	DBGPRINTF2 ("write_block(): %i b, writ %i\n", bytes, numwritten);
	if (numwritten != bytes)
		exit_error (ERR_IO, "writing ", NULL);

	bytes_out += numwritten;
	return numwritten;

} /* write_block() */



/* EOF fileio.c */
/******************************************************************************/
