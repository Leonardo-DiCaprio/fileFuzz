/*
********************************************************************************
File: cmdarg.c

Tab size:           4
Max line length:    80
Programmer:         Volker Kuhlmann


wav2cdr 2.3.4 Copyright (C) 1997, 1998, 1999, 2000, 2006 Volker Kuhlmann
This program is free software under the terms of the GNU General Public License
version 2 (or later, at your option).
See the file COPYING for details about license terms and warranty.
<VolkerKuhlmann@gmx.de>


DESCRIPTION:

Scans and checks the command line arguments.


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
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <assert.h>
#include <time.h>
#include <math.h>

#include "chelp.h"

#include "wav2cdr.h"

#ifdef HAS_GNUGETOPT
#include "getopt.h"
#else
#include "mygetopt.h"
#endif


/*
	Global + local variables
*/
extern FILE
	*msgfile;	/* where to display cmd args and progress */

#ifdef DEBUG
extern FILE
	*dbgfile;	/* where to display debug output */
#endif

static cmdarg_t
	cmdarg = CMDARG_DEFAULT;	/* cmd args, set to default values */

/* size of input data in bytes, not counting headers
	size not available when == -1 (therefore must be a signed type)
	Can not make this int because that may be only 16 bit.
	Can not make this size_t because that is usually unsigned int.
	Should be the same type as get_file_size().
*/
static signed long input_size;



/*
	Local function prototypes
*/
static signed long get_num_unit (const string *s);
#ifdef GET_UINT16
static UINT16 get_uint16_bytes (const char *s);
static UINT16 get_uint16_blocks (const char *s);
#endif
static void check_monotonous_cuts (void);
static signed long get_neg_cut (signed long cut);



/*
	Parse command line and do some checks for consistency.
	In: argc, argv
	Out: ---
	Return: ---
*/
void scan_cmd_args (int argc, char **argv)
{
int lidx
#ifdef DEBUG
 		 = -99
#endif
	;
int r = 1;
enum {  /* all must be != 0, and != any of the short option letters */
	o_in = 1, o_out, o_iscale, o_fscale, o_ssil, o_esil,
	o_indflt, o_inwav, o_incdr, o_inraw,
	o_todflt, o_toraw, o_tocdr, o_towav, 
	o_swapwords0, o_swapwords1, o_silthresh, o_sildelay, o_fadein, o_fadeout
	};
const string shortoptions[] = "hIiOour:Vw:";
const struct option longoptions[] = {
	{"h",				no_argument, (int *) &cmdarg.usage, 'u'},
	{"u",				no_argument, (int *) &cmdarg.usage, 'u'},
	{"usage",			no_argument, (int *) &cmdarg.usage, 'u'},
	{"help",			no_argument, (int *) &cmdarg.help, 'H'},
	{"tocdr",			no_argument, NULL, o_tocdr},
	{"towav",			no_argument, NULL, o_towav},
	{"toraw",			no_argument, NULL, o_toraw},
	{"inraw",			no_argument, NULL, o_inraw},
	{"inwav",			no_argument, NULL, o_inwav},
	{"incdr",			no_argument, NULL, o_incdr},
	#if 0
	{"ifmt",			required_argument, NULL, o_iformat},
	{"ofmt",			required_argument, NULL, o_oformat},
	#endif
	{"monostereo",		no_argument, (int *) &cmdarg.monostereo, TRUE},
	{"inbig",			no_argument, NULL, 'I'},
	{"I",				no_argument, NULL, 'I'},
	{"inlittle",		no_argument, NULL, 'i'},
	{"i",				no_argument, NULL, 'i'},
	{"outbig",			no_argument, NULL, 'O'},
	{"O",				no_argument, NULL, 'O'},
	{"outlittle",		no_argument, NULL, 'o'},
	{"o",				no_argument, NULL, 'o'},
	{"swapchannels",	no_argument, NULL, o_swapwords1},
	/*{"swch",			no_argument, NULL, o_swapwords1},*/
	{"noswapchannels",	no_argument, NULL, o_swapwords0},
	/*{"noswch",			no_argument, NULL, o_swapwords0},*/
	{"infile",			required_argument, NULL, o_in},
	/*{"if",				required_argument, NULL, o_in},*/
	{"outfile",			required_argument, NULL, o_out},
	/*{"of",				required_argument, NULL, o_out},*/
	{"iscale",			required_argument, NULL, o_iscale},
	{"fscale",			required_argument, NULL, o_fscale},
	{"cut",				no_argument, NULL, 'c'},
	{"startsilence",	required_argument, NULL, o_ssil},
	{"ss",				required_argument, NULL, o_ssil},
	{"endsilence",		required_argument, NULL, o_esil},
	{"es",				required_argument, NULL, o_esil},
	{"silencecuts", 	no_argument, (int *) &cmdarg.silencecuts, TRUE},
	{"silenceinfo",	 	no_argument, (int *) &cmdarg.silenceinfo, TRUE},
	{"silencethresh", 	required_argument, NULL, o_silthresh},
	{"silencedelay", 	required_argument, NULL, o_sildelay},
	{"fadein",		 	required_argument, NULL, o_fadein},
	{"fadeout",		 	required_argument, NULL, o_fadeout},
	{"quiet", 			no_argument, (int *) &cmdarg.quiet, TRUE},
	{"version", 		no_argument, NULL, 'V'},
	{"verbose", 		no_argument, (int *) &cmdarg.verbose, TRUE},
	{NULL,				no_argument, NULL, 0}
	};

	/* assignments by getopt will assign a size of int into boolean fields
		of cmdarg */
	assert (sizeof(BOOL)==sizeof(int));
	
	#ifdef DEBUG
	dbgfile = stderr;
	DBGPRINTF1 ("long options: %i\n", 
					sizeof(longoptions) / sizeof(struct option));
	#endif
	optind = 0;  /* tell getopt to init */
	while (r != -1) {
		r = getopt_long (argc, argv, shortoptions, longoptions, &lidx);
		DBGPRINTF3 ("getopt() ret: x%02x '%c', optind: %i", 
					r!=-1 ? r : 0xff, ((r>=32) AND (r<127)) ? r : ' ', optind);
		DBGPRINTF3 (", optopt '%c' x%02x, lidx: %i",
						optopt, optopt, lidx);
		DBGPRINTF1 (", optarg: %s\n", optarg != NULL ? optarg : "(NULL)");
		switch (r) {
		case 'h':
		case 'u':
			cmdarg.usage = TRUE;
			break;
		case 'V':
			cmdarg.version = TRUE;
			break;
		case o_swapwords1:
			cmdarg.swapwords = TRUE;
			break;
		case o_swapwords0:
			cmdarg.swapwords = FALSE;
			break;
		case o_inraw:
			cmdarg.informat = AF_raw;
			cmdarg.little_input = RAW_FORMAT_DEFAULT_IS_LITTLE;
			break;
		case o_inwav:
			cmdarg.informat = AF_wav;
			cmdarg.little_input = TRUE;
			break;
		case o_incdr:
			cmdarg.informat = AF_cdr;
			cmdarg.little_input = FALSE;
			break;
		case o_tocdr:
			cmdarg.outformat = AF_cdr;
			cmdarg.little_output = FALSE;
			break;
		case o_towav:
			cmdarg.outformat = AF_wav;
			cmdarg.little_output = TRUE;
			break;
		case o_toraw:
			cmdarg.outformat = AF_raw;
			cmdarg.little_output = RAW_FORMAT_DEFAULT_IS_LITTLE;
			break;
		case 'I':
			cmdarg.little_input = FALSE;
			if (cmdarg.informat == AF_wav)
				exit_error (ERR_CMDARG, "can't read wav in big endian", NULL);
			break;
		case 'i':
			cmdarg.little_input = TRUE;
			if (cmdarg.informat == AF_cdr)
				exit_error (ERR_CMDARG, "can't read cdr in little endian", NULL);
			break;
		case 'O':
			cmdarg.little_output = FALSE;
			if (cmdarg.outformat == AF_wav)
				exit_error (ERR_CMDARG, "must write wav in little endian", NULL);
			break;
		case 'o':
			cmdarg.little_output = TRUE;
			if (cmdarg.outformat == AF_cdr)
				exit_error (ERR_CMDARG, "must write cdr in big endian", NULL);
			break;
		case o_in:
		case 'r':
			cmdarg.infilename = optarg;
			break;
		case o_out:
		case 'w':
			cmdarg.outfilename = optarg;
			break;
		case o_iscale:
			r = sscanf (optarg, "%li", &cmdarg.iscale);
			if (r != 1)
				exit_error (ERR_CMDARG,
					"with number (syntax): ", optarg);
			break;
		case o_fscale:
			r = sscanf (optarg, "%f", &cmdarg.fscale);
			if (r != 1)
				exit_error (ERR_CMDARG, 
					"with floating point number (syntax): ", optarg);
			break;
		case 'c':
			cmdarg.numcuts = 1;  /* set marker that the option was there */
			break;
		case o_ssil:
			cmdarg.startsilence = get_silence_value (optarg);
			break;
		case o_esil:
			cmdarg.endsilence = get_silence_value (optarg);
			break;
		case o_silthresh:
			cmdarg.silence_thresh = get_num_unit (optarg) / CDAUDIOSECTORSIZE;
			break;
		case o_sildelay:
			cmdarg.silence_delay = get_num_unit (optarg) / CDAUDIOSECTORSIZE;
			break;
		case o_fadein:
			cmdarg.fadein = get_num_unit (optarg);
			break;
		case o_fadeout:
			cmdarg.fadeout = get_num_unit (optarg);
			break;
		case 0:
		case -1:
			break;
		case '?':
			/* some error, optopt knows more */
			/* this test falls over for long options with option->val == 0 */
			if (optopt == 0x00)
				exit_error (ERR_CMDARG, "unknown option: ", argv[optind - 1]);
			if ((strchr (shortoptions, optopt) != NULL)
				OR NOT isprint (optopt))
				exit_error (ERR_CMDARG, argv[optind - 1], " requires an argument");
			else {
				string s[2];
				s[0] = isprint (optopt) ? optopt : ' '; s[1] = '\0';
				exit_error (ERR_CMDARG, "unknown short option: ", s);
			}
		default:
			/* internal error */
			{
			string buf[20];
			sprintf (buf, "%i", r);
			exit_error (ERR_INTERNAL, "can't handle getopt() return val ", buf);
			}
		}
	}
	#ifdef DEBUG
	DBGPRINTF2 ("End scan. optind: %i   argc = %i\n", optind, argc);
	{ int i; for (i = 0; i < argc; i++)
		DBGPRINTF2 ("  argv %i %s\n", i, argv[i]);
	}
	#endif

	/* use remaining options to fill up filenames or cut numbers
		(but not both) */
	while (optind < argc) {
		#if 0
		/* (if not starting with '-' or exactly "-") */
		if ((argv[optind][0] == '-')  AND  (argv[optind][1] != '\0'))
			exit_error (ERR_CMDARG, "unknown option ", argv[optind]);
		#endif
		if (cmdarg.numcuts) {
			cmdarg.numcuts = argc - optind;
			cmdarg.cutstarts = &argv[optind];
			break; /* remaining args are cut numbers */
		}
		if (cmdarg.infilename == NULL)
			cmdarg.infilename = argv[optind];
		else {
			if (cmdarg.outfilename == NULL)
				cmdarg.outfilename = argv[optind];
			else
				exit_error (ERR_CMDARG, "can not handle ", argv[optind]);
		}
		optind++;
	}

	/* treat name "-" the same as stdin/stdout */
	/* this must be before main() calls check_cmd_args() */
	if ((cmdarg.infilename != NULL) 
		AND (strcmp (cmdarg.infilename, "-") == 0))
		cmdarg.infilename = NULL;
	if ((cmdarg.outfilename != NULL) 
		AND (strcmp (cmdarg.outfilename, "-") == 0))
		cmdarg.outfilename = NULL;

	/* when finding silence, set output format to raw and byte order to local */
	/* this should really go into the big switch() above */
	if (cmdarg.silenceinfo)
		cmdarg.silencecuts = TRUE;
	if (cmdarg.silencecuts) {
		cmdarg.outformat = AF_raw;
		cmdarg.little_output = is_localhost_little ();
	}

} /* scan_cmd_args() */



/*
	Determine how to use stdout and stderr for messages, output, and
	debugging output
*/
void set_message_output (void)
{
	/* determine where messages and debugging output go to */
	msgfile = stdout;
	if (cmdarg.outfilename == NULL)
		msgfile = stderr;
	#ifdef DEBUG
	dbgfile = msgfile;
	#endif
	if (cmdarg.quiet)
		msgfile = open_message_file ();
} /* set_message_output() */



/*
	Check consistency of command line arguments
*/
void check_cmd_args (void)
{
int hs;

	#if 0
	if (cmdarg.fscale < 0.0)
		exit_error (ERR_CMDARG, "negative scale factor makes no sense",
					 NULL);
	if (cmdarg.iscale < 0)
		exit_error (ERR_CMDARG, "negative scale factor makes no sense",
					 NULL);
	#endif
	if ((cmdarg.iscale < SCALE_MIN)
		OR (cmdarg.iscale > SCALE_MAX)
		OR ((signed long) cmdarg.fscale < SCALE_MIN)
		OR ((signed long) cmdarg.fscale > SCALE_MAX))
		exit_error (ERR_CMDARG, "scale factor out of range", NULL);
	if (cmdarg.iscale < 0  OR  cmdarg.fscale < 0.0)
		fprintf (msgfile, "Scale factor is negative. "
			"Will proceed and assume you know what you are doing.\n");

	/* initialise input data size */
	input_size = get_file_size (cmdarg.infilename, stdin);
	/* Adjust the input data size to the amount of actual data only,
		excluding any headers.  FIXME: this is dodgy... */
	hs = get_af_header_size (cmdarg.informat);
	if (input_size != -1)
		input_size -= (input_size >= (signed long) hs) ? hs : input_size;

	if (cmdarg.numcuts > 0) {
		if ((cmdarg.numcuts > 2) AND (cmdarg.outfilename == NULL))
			exit_error (ERR_CMDARG, 
							"can not cut more than 1 part to stdout", "");
		if (cmdarg.numcuts == 1)
			exit_error (ERR_CMDARG, "must have >= 2 cut numbers", "");
		check_monotonous_cuts ();
	}
	
	if (cmdarg.help) {
		msgfile = stdout;
		help ();
	}
	if (cmdarg.usage) {
		msgfile = stdout;
		usage ();
	}
	if (cmdarg.version) {
		msgfile = stdout;
		version ();
	}

	if ((cmdarg.iscale != 100) AND (cmdarg.fscale != 1.0)) {
		fprintf (msgfile, 
			"Both integer and float scale given - float scale ignored.\n");
		cmdarg.fscale = 1.0;
	}
	
	if (((unsigned long) cmdarg.silence_thresh > USHRT_MAX) 
		OR ((unsigned long) cmdarg.silence_delay > USHRT_MAX))
		exit_error (ERR_CMDARG, 
					"silence threshold and delay must be positive 16 bit", "");

	if (cmdarg.silence_delay == 0)
		exit_error (ERR_CMDARG,
					"silence delay must be >= 1", "");

	if (cmdarg.fadeout > 0 AND cmdarg.numcuts == 0 AND input_size == -1)
		exit_error (ERR_CMDARG,
					"can't fade out when input size can't be determined", "");
	
} /* check_cmd_args() */



/*
	Check whether cut numbers are monotonous.
*/
static void check_monotonous_cuts (void)
{
int j;
unsigned long this, next;
string buf[80];

	for (j = 0, this = get_cut_value (0); j < (cmdarg.numcuts - 1); j++) {
		next = get_cut_value (j + 1);
		if (next < this) {
			sprintf (buf, "cut number (N+1) not >= (N)  (%li not >= %li)",
						next, this);
			exit_error (ERR_CMDARG, buf, NULL);
		}
		this = next;
	}
} /* check_monotonous_cuts() */



/*
	Display the current state of internal variables.
	Writes to the stream msgfile.
	In: ---
	Out: ---
	Return: ---
*/
void showcmdargs (void)
{
const string 
	yes[] = "yes", 
	no[] = "no", 
	big[] = "big", 
	little[] = "little";
const string *AF_nameidx[] = AF_NAMEIDX;
cdseccount_t firstcut = 0, lastcut = 0;
string insize[80], silence[80];
string stasil[TIMESTRSIZE], endsil[TIMESTRSIZE];
string filength[TIMESTRSIZE], folength[TIMESTRSIZE];
string stime[TIMESTRSIZE], sfirstcut[TIMESTRSIZE], slastcut[TIMESTRSIZE];

	timeprintf (stasil, cmdarg.startsilence);
	timeprintf (endsil, cmdarg.endsilence);
	timeprintf (filength, cmdarg.fadein);
	timeprintf (folength, cmdarg.fadeout);

	if (input_size == -1)
		strcpy (insize, "not sizeable/seekable (reading pipe?)");
	else {
		timeprintf (stime, (unsigned long) input_size);
		sprintf (insize, "%li b = %li C = %li s = %s min",
				(long) input_size,
				(long) input_size / CDAUDIOSECTORSIZE,
				/*(float) input_size / CDAUDIOSECTORSIZE / CDSECTORSPERSEC*/
				input_size / CDAUDIOSECTORSIZE / CDSECTORSPERSEC,
				stime
				);
	}

	if (cmdarg.numcuts > 0) {
		firstcut = get_cut_value (0);
		lastcut = get_cut_value (cmdarg.numcuts - 1);
	}
	timeprintf (sfirstcut, firstcut * CDAUDIOSECTORSIZE);
	timeprintf (slastcut, lastcut * CDAUDIOSECTORSIZE);
	if (cmdarg.silencecuts)
		sprintf (silence, "%s   (threshold %li, delay %li C = %.2f s)",
					yes, cmdarg.silence_thresh, cmdarg.silence_delay,
					(float) cmdarg.silence_delay / CDSECTORSPERSEC);
	else
		strcpy (silence, no);
	
	fprintf (msgfile,
		"Host is:            %s\n"
		"Byte order in, out: %s -> %s\n"
		"Format in, out:     %s -> %s\n"
		"Scale:              %li%%, * %f\n"
		"Swap channels:      %s\n"
		"Do ->mono->stereo:  %s\n"
		"Find silences:      %s  %s\n"
		"Add silence:        %li C = %s min, %li C = %s min\n"
		"Fade-in / -out:     %li C = %s min, %li C = %s min\n"
		"Input data size:    %s\n"
		"Number of cuts:     %i\n"
		"Skipping before:    %s min = %li C  (excl) (first cut)\n"
		"Skipping after:     %s min = %li C  (incl) (last cut)\n"
		"Input filename:     %s\n"
		"Output filename:    %s%s\n"
		"-\n",
		is_localhost_little () ? little : big,
		cmdarg.little_input ? little : big,
			cmdarg.little_output ? little : big,
		AF_nameidx[cmdarg.informat],
			AF_nameidx[cmdarg.outformat],
		cmdarg.iscale,
			cmdarg.fscale,
		cmdarg.swapwords ? yes : no,
		cmdarg.monostereo ? yes : no,
		silence,  cmdarg.silenceinfo ? "[INFO]" : "",
		cmdarg.startsilence / CDAUDIOSECTORSIZE, stasil,
			cmdarg.endsilence / CDAUDIOSECTORSIZE, endsil,
		cmdarg.fadein / CDAUDIOSECTORSIZE, filength, 
			cmdarg.fadeout / CDAUDIOSECTORSIZE, folength,
		insize,
		cmdarg.numcuts,
		sfirstcut, firstcut,
		slastcut, lastcut,
		STRSTDIN(cmdarg.infilename),
		STRSTDOUT(cmdarg.outfilename),
			cmdarg.outfilename != NULL ? ".%02d" : ""
		);

} /* showcmdargs() */



/*
	Generic: get size number, in bytes
	Converts a number (integer) with an optional unit,
	units:      type:
		b       int   (bytes), 
		c or C  int   (audio CD sectors, default),
		s       float (seconds of CD audio)
	In: string representation of the number
	Out: ---
	Return: number IN BYTES, always a multiple of CDAUDIOSECTORSIZE
*/
static signed long get_num_unit (const string *s)
{
signed long int i;
float sec;
char unit[] = "\0";
int r;

	r = sscanf (s, "%li%1s", &i, unit);
	if (r == 1)
		unit[0] = 'C';	/* default unit */
	if (r < 1  OR  (r == 2  AND  strchr ("bscC.", unit[0]) == NULL))
		exit_error (ERR_CMDARG, "number/unit syntax: ", s);
	if (r == 2  AND  unit[0] == '.') {
		unit[0] = '\0';
		r = sscanf (s, "%f%1s", &sec, unit);
		if (r == 2  AND  strchr ("bcC", unit[0]) != NULL)
			exit_error (ERR_CMDARG,
			 "number/unit syntax (fractions not with b, c): ", s);
		if (r < 1  OR  unit[0] != 's')
			exit_error (ERR_CMDARG, "number/unit syntax: ", s);
		unit[0] = 'S';
	}
	DBGPRINTF3 ("<< get val: %li %2.2f %c", i, sec, unit[0]);

	switch (unit[0]) {
	case 'c':
	case 'C':	/* audio CD sectors */
		i *= CDAUDIOSECTORSIZE;
		break;
	case 'b':	/* bytes */
		i = ((i + (CDAUDIOSECTORSIZE / 2)) / CDAUDIOSECTORSIZE)
				* CDAUDIOSECTORSIZE;
		break;
	case 's':	/* seconds of CD audio */
		i *= (signed long) CDSECTORSPERSEC * (signed long) CDAUDIOSECTORSIZE;
		break;
	case 'S':
		i = ((signed long) (sec * (float) CDSECTORSPERSEC + 0.5))
				* (signed long) CDAUDIOSECTORSIZE;
		break;
	default:
		exit_error (ERR_INTERNAL, "switch", NULL);
	}		
	DBGPRINTF1 ("  new val: %li >>\n", i);
	
	return i;

} /* get_num_unit */



/*
	Get a positive 16 bit value. Uses get_num_unit().
	In: string representation of the number
	Out: ---
	Return: number
*/
#ifdef GET_UINT16
static UINT16 get_uint16_bytes (const char *s)
{
signed long l;

	l = get_num_unit (s);
	if (l < 0  OR  l > USHRT_MAX)
		exit_error (ERR_CMDARG, "number must be positive 16 bit: ", s);
	return (UINT16) l;

} /* get_uint16_bytes() */
#endif



/*
	Get a positive 16 bit value, in CDAUDIOSECTORSIZE size units.
	Uses get_num_unit().
	In: string representation of the number
	Out: ---
	Return: number
*/
#ifdef GET_UINT16
static UINT16 get_uint16_blocks (const char *s)
{
signed long l;

	l = get_num_unit (s) / CDAUDIOSECTORSIZE;
	if (l < 0  OR  l > USHRT_MAX)
		exit_error (ERR_CMDARG, "number must be positive 16 bit: ", s);
	return (UINT16) l;

} /* get_uint16_blocks() */
#endif



/*
	In: ---
	Out: ---
	Return: Input size in full CD blocks, or -1 if not available
*/
signed long get_input_size_in_cd_blocks (void)
{
signed long size;

	if (input_size == -1)
		return -1;

	/* this is file format dependent!!, but input_size allows for that */
	size = (input_size / CDAUDIOSECTORSIZE);
	if ((size * CDAUDIOSECTORSIZE) < input_size)
		size++; /* measure from the true end of the cdr file */
	return size;

} /* get_input_size_in_cd_blocks() */



/*
	Calculates the cut position from the file size for negative cut numbers.
	Expects the input data size to be initialised.
*/
static signed long get_neg_cut (signed long cut)
{
signed long pos, size;

	if (input_size == -1)
		exit_error (ERR_CMDARG, 
			"cut numbers must not be negative when input can't be sized", 
			NULL);

	size = get_input_size_in_cd_blocks ();
	pos = size - (-cut);
	if (pos < 0) {
		string buf[100];
		sprintf (buf, 
			"cut %li C before start of input with length %li b = %li C",
			(long) cut, (long) input_size, (long) size);
		exit_error (ERR_CMDARG, buf, NULL);
	}
	return pos;
} /* get_neg_cut() */



/*
	Get the Nth cut value.
	The value returned is positive and in audio CD sectors!
	Expects the input data size to be initialised.
*/
unsigned long get_cut_value (int n)
{
signed long cut, l = 0;

	cut = get_num_unit (cmdarg.cutstarts[n]) / CDAUDIOSECTORSIZE;
	l = cut;
	if ((cut == 0L) AND (n == (cmdarg.numcuts - 1))) {
		/* a 0 as last cut number means all the rest;
		   if input size is not available it's better to use the theoretical
		   maximum file size than to fail */
		if (input_size != -1)
			l = get_neg_cut (cut);
		else
			l = ULONG_MAX / CDAUDIOSECTORSIZE;
	} else if (cut < 0) {
		/* negative cut number - calculate from file size */
		if (input_size == -1)
			exit_error (ERR_CMDARG, 
				"cut numbers must not be negative when input can't be sized", 
				NULL);
		else
			l = get_neg_cut (cut);
	}
	
	DBGPRINTF2 ("get_cut_value(%i): %li\n", n, (long) l);
	return (unsigned long) l;
	
} /* get_cut_value() */



/*
	Get a silence value.
	The value returned is in bytes.
*/
unsigned long get_silence_value (const string *s)
{
signed long l;

	l = get_num_unit (s);
	if (l < 0L)
		exit_error (ERR_CMDARG, 
					"negative silence number does not make sense", NULL);
	return (unsigned long) l;
	
} /* get_silence_value() */



/*
	Return the command arg info
	In: buffer
	Out: initialised command arg info struct
	Return: ---
*/
void get_cmdarg_info (cmdarg_t *pcmdarg)
{
	*pcmdarg = cmdarg;
}



/*
	Init a processing info struct, from cmdargs and otherwise
	In: buffer
	Out: initialised process info struct
	Return: ---
*/
void init_process_info (process_t *pinfo)
{
	pinfo->n_in = pinfo->n_returned = 0;

	pinfo->first = TRUE;
	pinfo->last = FALSE;

	pinfo->little_host = is_localhost_little ();
	pinfo->from_little = cmdarg.little_input;
	pinfo->to_little = cmdarg.little_output;
	pinfo->iscale = cmdarg.iscale;
	pinfo->fscale = cmdarg.fscale;
	pinfo->swap_channels = cmdarg.swapwords;
	pinfo->monostereo = cmdarg.monostereo ? MS_TOMONOSTEREO : MS_NONE;
	pinfo->silencecuts = cmdarg.silencecuts;
	pinfo->silence_thresh = cmdarg.silence_thresh;
	pinfo->silence_delay = cmdarg.silence_delay;
	pinfo->silence_val = cmdarg.verbose;
	pinfo->fadein = 0;
	pinfo->fadeout = 0;
	
} /* init_process_info() */



/* EOF cmdarg.c */
/******************************************************************************/
