/*
********************************************************************************
File: data.c

Tab size:           4
Max line length:    80
Programmer:         Volker Kuhlmann


wav2cdr 2.3.4 Copyright (C) 1997, 1998, 1999, 2000, 2006 Volker Kuhlmann
This program is free software under the terms of the GNU General Public License
version 2 (or later, at your option).
See the file COPYING for details about license terms and warranty.
<VolkerKuhlmann@gmx.de>


DESCRIPTION:

This module handels all data I/O: opening / closing, headers, data block I/O.


CONDITIONALS:
	see wav2cdr.c


HISTORY:
	see wav2cdr.c, and ChangeLog

********************************************************************************
*/



#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <limits.h>

#include "chelp.h"

#include "wav2cdr.h"



static cmdarg_t
	cmdinfo;			/* the cmd line args */

static process_t
	procinfo;			/* how and what processing should be done */

static unsigned long
	firstsec,			/* sector of first and last cut */
	lastsec;

static unsigned long
	total_bytes_out;	/* bytes written to all files */

static time_t
	/*instart,*/		/* start time of processing of current input file */
	outstart;			/* start time of processing of current output file */

static BOOL
	out_is_open;		/* flag whether output is open */

extern FILE
	*msgfile;			/* where to display cmd args and progress */

extern const unsigned long
	bytes_in,
	bytes_out;

extern const string
	outname[];



/* local function prototypes */
static void check_for_fadeout (unsigned long nsecs, unsigned long endsecnum);



/*
	Open output
*/
void open_out (int track)
{
	outstart = time (NULL);

	open_output_file (cmdinfo.outfilename, track);
	out_is_open = TRUE;
	DBGPRINTF1 ("open_out: %s\n", STRNULL(cmdinfo.outfilename));

	write_header ();

} /* open_out() */



/*
	Close output and print stats.
	Can be called if output not open (can e.g. be the case if there
	was no data to write, or at end of input when the last cut has been)
*/
void close_out (void)
{
time_t runtime;
const string *to, strstdout[] = "(stdout)";
string stime[20];

	/* if output not open do nothing */
	if (NOT out_is_open)
		return;

	write_trailer ();
	close_output_file ();
	out_is_open = FALSE;

	runtime = difftime (time (NULL), outstart);
	total_bytes_out += bytes_out;
	
	to = (cmdinfo.outfilename == NULL) ? strstdout : outname;
	timeprintf (stime, bytes_out);
	if (cmdinfo.outformat == AF_cdr)
		fprintf (msgfile, 
				"Wrote bytes:       %li (%li C, %s min) -> %s\n", 
				(long) bytes_out, 
				(long) bytes_out / CDAUDIOSECTORSIZE,
				stime,
				to);
	else
		fprintf (msgfile,
				"Wrote bytes:       %li -> %s\n", 
				(long) bytes_out,
				to);

	fprintf (msgfile, "Execution time:    %02li:%02li:%02li h  (%li s)\n"
						"-\n",
		runtime / 3600, (runtime / 60) % 60, runtime % 60, runtime);

} /* close_out() */



/*
	Return the size of the header of the given audio format
	In: audio format
	Out: ---
	Return: size of header, or -1
*/
int get_af_header_size (audioformat_t af)
{
	switch (af) {
	case AF_raw:
		return 0;
	case AF_cdr:
		return 0;
	case AF_wav:
		return sizeof (wav_header_t);
	default:
		return -1;
	}
} /* get_af_header_size() */



/*
	Read header of the current input file format
*/
void read_header (void)
{
wav_header_t wh;

	switch (cmdinfo.informat) {
	case AF_wav:
		read_wav_header (&wh);
		break;
	case AF_cdr:
	case AF_raw:
	default:
		;
	}
} /* read_header() */



/*
	Write header for the current output file format.
	Used: struct cmdinfo
	Side: bytes written updated
	In: ---
	Out: ---
	Return: ---
*/
void write_header (void)
{
wav_header_t wh;

	DBGPRINTF1 ("write_header: %i\n", cmdinfo.outformat);
	switch (cmdinfo.outformat) {
	case AF_wav:
		make_wav_header (&wh, 0L);
		write_wav_header (&wh);
		break;
	case AF_cdr:
	case AF_raw:
	default:
		;
	}
	
	add_silence (cmdinfo.startsilence);

} /* write_header() */



/*
	Write trailer for the current output file format.
	This can be e.g. updating a header, or padding the last sector.
	This functions is always called at the end of an output file.
	Used: struct cmdinfo
	Side: ---
	In: ---
	Out: ---
	Return: ---
*/
void write_trailer (void)
{
wav_header_t wh;
size_t last;
BORCRAP_largevar char buffer[CDAUDIOSECTORSIZE];

	DBGPRINTF1 ("write_trailer: %i\n", cmdinfo.outformat);
	add_silence (cmdinfo.endsilence);

	switch (cmdinfo.outformat) {
	case AF_wav:
		/* update wav header */
		make_wav_header (&wh, bytes_out - sizeof (wh));
		write_wav_header (&wh);
		break;
	case AF_cdr:
		/* pad last sector */
		last = (size_t) (bytes_out % CDAUDIOSECTORSIZE);
		if (last == 0)
			break;
		last = CDAUDIOSECTORSIZE - last;
		if (last > 0) {
			memset (buffer, 0, last);
			write_block (buffer, last);
			fprintf (msgfile, "cdr: padding %li bytes\n", (long) last);
		}
		break;
	case AF_raw:
	default:
		;
	}

} /* write_trailer() */



/*
	Add silence...
	In: number of bytes to write
	Out: ---
	Return: ---
*/
void add_silence (unsigned long bytes)
{
size_t w;
char buffer[CDAUDIOSECTORSIZE];  /* note buffer must be < max size_t! */

	DBGPRINTF1 ("add_silence: %li\n", bytes);
	memset (buffer, 0, sizeof (buffer));
	do {
		w = (bytes > sizeof (buffer)) ? sizeof (buffer) : (size_t) bytes;
		w = write_block (buffer, w);
		bytes -= w;
	} while (bytes > 0);
	
} /* add_silence() */



/*
*/
static void check_for_fadeout (unsigned long nsecs, unsigned long endsecnum)
{
unsigned long fadeoutblocks;

	fadeoutblocks = cmdinfo.fadeout / CDAUDIOSECTORSIZE;
	if (nsecs + fadeoutblocks == endsecnum)
		procinfo.fadeout = fadeoutblocks;

} /* check_for_fadeout() */



/*
	Decide whether to copy the current sector to the output or discard it, and
	open/close files if necessary.
	The first file is opened when necessary, the last file remains opened.
	If cutting the input into pieces is off, all sectors are copied.
	Cutting is turned on by specifying at least 2 cut numbers.
	Cut numbers are the sector number from start of input, starting with 0.
	One "cut" goes from one cut number to the next. The start cut number
	belongs to the cut, but not the end cut number, as in [start..[end .
	The first and the last cut are discarded, in other words all sectors
	up to and not including the first cut number and after and including the
	last cut number are discarded.
	Cut numbers are assumed to be sorted in increasing order, or equal.
	No consistency checking of cut numbers is done here.
	In case adjacent cut numbers are equal, empty output is produced.
	If output is to stdout, at most 1 cut / 2 cut numbers are allowed.
	In: ---
	Out: ---
	Return: whether to copy this sector
*/
BOOL copy_sector_check_file (void)
{
static unsigned long nsecs = 0,	/* current sector of input */
				endsecnum = 0;	/* end sector num of current cut */
static int track = 1;			/* counter of cutouts */

	/* have not yet reached start sector - no show */
	if (nsecs < firstsec) {
		nsecs++; return FALSE;
	}
		
	/* open first output file to get started */
	if (nsecs == firstsec) {
		open_out (track);
		procinfo.fadein = cmdinfo.fadein / CDAUDIOSECTORSIZE;
		if (cmdinfo.numcuts != 0) {
			endsecnum = get_cut_value (track);
			DBGPRINTF1 ("endsecnum: %li\n", endsecnum);
		} else if (cmdinfo.fadeout)
			endsecnum = (unsigned long) get_input_size_in_cd_blocks ();
			/* at this point we know that input size is available */
	}
	
	/* copy all sectors if there is no cutting */
	if (cmdinfo.numcuts == 0) {
		if (cmdinfo.fadeout)
			check_for_fadeout (nsecs, endsecnum);
		nsecs++; return TRUE;
	}
	
	/* if output is to stdout we can have at most 2 cut numbers */
	if (cmdinfo.outfilename == NULL) {
		if (cmdinfo.fadeout)
			check_for_fadeout (nsecs, endsecnum);
		nsecs++; return nsecs <= lastsec;
	}

	/* when reaching current end sector: close file and open next, unless
	   this was the last */
	if (cmdinfo.fadeout)
		check_for_fadeout (nsecs, endsecnum);
	while (nsecs >= endsecnum) {
		if (track < cmdinfo.numcuts - 1) {
			close_out ();
			open_out (++track);
			procinfo.fadein = cmdinfo.fadein / CDAUDIOSECTORSIZE;
			endsecnum = get_cut_value (track);
			DBGPRINTF1 ("endsecnum: %li\n", endsecnum);
		} else {
			nsecs++; return FALSE;
		}
	}
	
	nsecs++; return TRUE;
	
} /* copy_sector_check_file() */



/*
	Scan the passed string for a silence value.
	In: pointer to pointer to string, buffer for silence number
	Out: pointer points to next, unscanned, number in string; silence number
	Return: whether a silence number was found
*/
BOOL got_silence_number (char **pstr, UINT16 *ps)
{
int numbers_scanned, chars_scanned;

/*fprintf(stderr,"SILstr: len=%d <%s>\n", strlen(*pstr), *pstr);*/
	/* scan over a possible silence value */
	numbers_scanned = sscanf (*pstr, "%nS%hu%n%*c%n",
						&chars_scanned, ps, &chars_scanned, &chars_scanned);
	*pstr += chars_scanned;
/*if (numbers_scanned>0) fprintf(stderr,"SIL: %hu, <%s>\n", sil, *pstr);*/
	return numbers_scanned >= 1;
} /* got_silence_number() */



/*
	Scan the passed string for cut numbers. At most 1 is returned, and the
	pointer to the string increased so that a subsequent call with the increased
	pointer will return the next cut number.
	In: pointer to pointer to string, buffer for cut number
	Out: pointer points to next, unscanned, number in string; cut number
	Return: whether a cut number was found
*/
BOOL got_cut_number (char **pstr, unsigned long *pi)
{
int numbers_scanned, chars_scanned;

	numbers_scanned = sscanf (*pstr, "%n%lu%n%*c%n", 
						&chars_scanned, pi, &chars_scanned, &chars_scanned);
	*pstr += chars_scanned;
	return numbers_scanned >= 1;
} /* got_cut_number() */



void sprint_cutinfo (void *buffer, unsigned long start, unsigned long end,
						BOOL is_audio_interval)
{
cdseccount_t diff;
string st1[TIMESTRSIZE], st2[TIMESTRSIZE], st3[TIMESTRSIZE];

	diff = end - start;
	timeprintf (st1, start * CDAUDIOSECTORSIZE);
	timeprintf (st2, end * CDAUDIOSECTORSIZE);
	timeprintf (st3, diff * CDAUDIOSECTORSIZE);
	sprintf ((char *) buffer + strlen (buffer),
			" %-5s %9li b, %6li C, %4li s, %s min\n"
			"  DIFF %9li b, %6li C, %4li s, %s min\n"
			"   --> %9li b, %6li C, %4li s, %s min\n\n",
			is_audio_interval ? "AUDIO" : "silnc",
			start * CDAUDIOSECTORSIZE, start,
			start / CDSECTORSPERSEC, st1,
			diff * CDAUDIOSECTORSIZE, diff,
			diff / CDSECTORSPERSEC, st3,
			end * CDAUDIOSECTORSIZE, end,
			end / CDSECTORSPERSEC, st2
			);
} /* sprint_cutinfo() */



/*
	Generate the silence information of the input file.
	Currently this function does not work if the number of cuts is not even;
	fulfilled by processing though.
*/
void silence_info (void *buffer, BOOL eof)
{
cdseccount_t cut = 0U;
static cdseccount_t lastcut = (cdseccount_t) -1, lastsigend = 0U;
static BOOL didname = FALSE;
static BOOL hadcut = FALSE;
string *cutstring, *cuts;
UINT16 silval;

#if 0
	const string *t[] = { "23", " 23", "23 ", " 4  ", " 55 ", "", "5 6 7",
							"77C", "77c", "7C 8C", "x"};
	string *s;
	int i = 0;
	while (t[i][0] != 'x') {
		s = (/*no const*/ string *) t[i];
		printf ("---+---\n");
		while (got_cut_number (&s, &cut))
			printf ("got cut number %ld\n", cut);
		i++;
	}
#endif

	if (procinfo.n_returned == 0U  AND  NOT eof) /* speed things up a bit */
		return;
	
	cutstring = cuts = malloc (procinfo.n_returned + 1);
	if (cuts == NULL)
		exit_error (ERR_NOMEM, "Could not allocate memory", NULL);
	strncpy (cuts, buffer, procinfo.n_returned);
	cuts[procinfo.n_returned] = '\0';
	#ifdef DEBUG
	cuts = (strchr (cuts, '=') != NULL) ? strchr (cuts, '=') + 2 : cuts;
	#endif
	
	#define BUF(buf) ((char *) (buf) + strlen (buf))
	* (char *) buffer = '\0';
	if (NOT didname) {
		sprintf (buffer, "%s:\n", STRSTDIN(cmdinfo.infilename));
		didname = TRUE;
	}

	while (procinfo.n_returned > 1U) {
		/* print silence value if there */
		if (got_silence_number (&cuts, &silval)  AND  cmdinfo.verbose)
			sprintf (BUF(buffer), "Silence = %i\n", silval);
		
		/* if there is a cut number, collect and print out */
		if (NOT got_cut_number (&cuts, &cut))
			break;
		if (lastcut == (cdseccount_t) -1) {
			lastcut = cut;
			continue;
		}
		hadcut = TRUE;
		if (cut == 0U)  /* last sector of file */
			cut = bytes_in / CDAUDIOSECTORSIZE;
		if (lastsigend < lastcut)
			sprint_cutinfo (BUF(buffer), lastsigend, lastcut, FALSE);
		sprint_cutinfo (BUF(buffer), lastcut, cut, TRUE);
		lastsigend = cut;
		lastcut = (cdseccount_t) -1;
	}
	if (eof  AND  lastsigend < bytes_in / CDAUDIOSECTORSIZE) {
		sprint_cutinfo (buffer, lastsigend, bytes_in / CDAUDIOSECTORSIZE, FALSE);
	}
	free (cutstring);
	procinfo.n_returned = strlen (buffer); /* ignore the '\0' here */
} /* silence_info() */



/*
	Read + write all data sectors, but no headers etc.
	Does not open/close input or output.
	Assumes input is open, and is left open.
	Assumes output is closed and will be opened/closed by
	copy_sector_check_file(). Output will be left closed ???
	In: ---
	Out: ---
	Return: ---
*/
void handle_sectors (void)
{
size_t numtoread, numread, numtowrite;
BORCRAP_largevar
byte buffer[CDAUDIOSECTORSIZE * PROCESS_MAX_MULTIPLY];

	numtoread = CDAUDIOSECTORSIZE;
	
	do {
		numread = read_block (&buffer, numtoread);
		/* go through the loop for empty input also */

		/* can only process in multiples of 4 bytes, truncate */
		if (numread BITAND 0x03) {
			fprintf (msgfile,
				"Can only process data in multiples of 4 bytes,"
				" truncating %i bytes.\n", (int)(numread BITAND 0x03));
			numread ABITAND (size_t) -4;  /* = 0x...fffc */
		}
		
		/* handle cutting / cut numbers, and opening closing output files */
		if (NOT copy_sector_check_file ())
			continue;
		
		/* do the number crunching */
		procinfo.n_in = numread;
		procinfo.last = read_eof ();
		process_sector (&buffer, &procinfo);
		if (cmdinfo.silenceinfo)
			silence_info (buffer, procinfo.last);
		
		/* write out buffer */
		numtowrite = procinfo.n_returned;
		DBGPRINTF2 ("handle_sectors: numread %i, numtowrite %i\n",
					numread, numtowrite);
		write_block (&buffer, numtowrite);

	} while ((numread == numtoread) AND NOT read_eof ());

} /* handle_sectors() */



/*
	Deal with an input file and write to output.
	This is called after command line parsing and does the lot.
	In: ---
	Out: ---
	Return: ---
*/
void do_data_io (void)
{
time_t executionstart;
unsigned long runtime;
string stime[20];

	/* init local variables */
	total_bytes_out = 0;
	firstsec = lastsec = 0;
	outstart = (time_t) -1;
	out_is_open = FALSE;
	
	/* get current values */
	get_cmdarg_info (&cmdinfo);
	init_process_info (&procinfo);
	if (cmdinfo.numcuts > 0) {
		firstsec = get_cut_value (0);
		lastsec = get_cut_value (cmdinfo.numcuts - 1);
	}

	/* START watch */
	executionstart = time (NULL);

	/* DO it */
	open_input_file (cmdinfo.infilename);
	read_header ();
	/* open output is done by copy_sector_check_file() */
	handle_sectors ();
	close_input_file ();	
	close_out ();	/* will be still open if not cutting */

	/* STOP watch */
	runtime = difftime (time (NULL), executionstart);

	if (cmdinfo.outformat == AF_cdr) {
		timeprintf (stime, total_bytes_out);
		fprintf (msgfile, 
				"Wrote bytes total: %li (%li C, %s min)\n", 
				(long) total_bytes_out, 
				(long) total_bytes_out / CDAUDIOSECTORSIZE, stime);
	} else
		fprintf (msgfile,
				"Wrote bytes total: %li\n", 
				(long) total_bytes_out);
	fprintf (msgfile, "Exec time total:   %02li:%02li:%02li h  (%li s)\n",
		runtime / 3600, (runtime / 60) % 60, runtime % 60, runtime);
	
} /* do_data_io() */



/* EOF dataio.c */
/******************************************************************************/
