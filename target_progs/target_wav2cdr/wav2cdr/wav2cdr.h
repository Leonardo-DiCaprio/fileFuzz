/*
********************************************************************************
File: wav2cdr.h

Tab size:           4
Max line length:    80
Programmer:         Volker Kuhlmann


wav2cdr 2.3.4 Copyright (C) 1997, 1998, 1999, 2000, 2006 Volker Kuhlmann
This program is free software under the terms of the GNU General Public License
version 2 (or later, at your option).
See the file COPYING for details about license terms and warranty.
<VolkerKuhlmann@gmx.de>


DESCRIPTION:

Header for the wav2cdr program. See wav2cdr.c for more.


CONDITIONALS:
	see wav2cdr.c


HISTORY:
	see wav2cdr.c

********************************************************************************
*/



#include <limits.h>
#include <stddef.h>
#include <stdio.h>

#include "chelp.h"



/*
	cdr file format
	Audio-CDs (CD-DA) and Data CDs
	size of a sector, and the whole CD, in bytes
*/
#define CDSECTORSPERSEC 75
#define CDSIZEINSECTORS (74L * 60 * CDSECTORSPERSEC)
/* sizes in bytes: */
#define CDAUDIOSECTORSIZE 2352
#define CDDATASECTORSIZE 2048
#define CDAUDIOSIZE (CDSIZEINSECTORS * CDAUDIOSECTORSIZE)
#define CDDATASIZE (CDSIZEINSECTORS * CDDATASECTORSIZE)
#define CDAUDIOBYTESPERSEC (CDSECTORSPERSEC * CDAUDIOSECTORSIZE)
/* sampling rate in samples per second:
      CDSECTORSPERSEC        75
	* CDAUDIOSECTORSIZE    * 2352
	/ channels             / 2
	/ bytes-per-channel    / 2     */
#define CDAUDIOSAMPLINGRATE 44100L


/*
	wav file format
*/
typedef struct {
	char id[4];					/* watch char order! */
	UINT32 size;				/* size of the chunk of stuff following, excl
									the 8 bytes of this chunk header */
} wav_chunkheader_t;

typedef struct {
	/* RIFF header: */
	wav_chunkheader_t RIFF;		/* "RIFF", size is that of fmt header + chunk,
									 + header of data chunk */
	char wavename[4];			/* "WAVE" */
	/* Format header: */
	wav_chunkheader_t Format;	/* "fmt ", size is that of fmt chunk only */
	UINT16 FormatTag;			/* 1 (PCM) */
	UINT16 channels;			/* 1, 2, 4 */
	UINT32 SamplingRate;
	UINT32 AvgBytesPerSec;
	UINT16 BlockAlignment;		/* 1, 2, 4 */
	UINT16 BitsPerSample;		/* 8, 16 */
	/* Sampla data header: */
	wav_chunkheader_t Data;		/* "data", size is that of the data */
	/*unsigned char bytes[44];*/
} wav_header_t;


/* debug output */
#ifdef DEBUG
  extern FILE *dbgfile;
  #define DBGPRINTF0(fmt) fprintf (dbgfile, fmt)
  #define DBGPRINTF1(fmt,v1) fprintf (dbgfile, fmt, v1)
  #define DBGPRINTF2(fmt,v1,v2) fprintf (dbgfile, fmt, v1, v2)
  #define DBGPRINTF3(fmt,v1,v2,v3) fprintf (dbgfile, fmt, v1, v2, v3)
  #define STRNULL(charptr) ((charptr) == NULL ? "(NULL)" : (charptr))
#else
  #define DBGPRINTF0(fmt)
  #define DBGPRINTF1(fmt,v1)
  #define DBGPRINTF2(fmt,v1,v2)
  #define DBGPRINTF3(fmt,v1,v2,v3)
#endif


/*
	Expressions and functions for byte-swapping, implemented as macros
*/
#define BYTES2SWAPPED(val) (\
		  (((val) >> 8) BITAND 0x00ff) \
	BITOR ((val) << 8) \
	)
#define BYTES4SWAPPED(val) (\
	      (((val) >> 24) BITAND 0x000000ffUL) \
	BITOR (((val) >>  8) BITAND 0x0000ff00UL) \
	BITOR (((val) <<  8) BITAND 0x00ff0000UL) \
	BITOR ((val) << 24) \
	)
#define WORDS2SWAPPED(val) (\
		  (((val) >> 16) BITAND 0x0000ffffUL) \
	BITOR ((val) << 16)\
	)
#define SWAP2BYTES(arg) (arg) = BYTES2SWAPPED ((arg))
#define SWAP4BYTES(arg) (arg) = BYTES4SWAPPED ((arg))
#define SWAP2WORDS(arg) (arg) = WORDS2SWAPPED ((arg))


/* turn a pointer into a pointer to a certain sized signed(!) integer */
#define AS_S16(bufptr) ((SINT16 *) bufptr)
#define AS_S32(bufptr) ((SINT32 *) bufptr)


/* max num of characters in a path name, Unix systems have this in limits.h? */
#ifndef PATH_MAX
#define PATH_MAX 1024
#endif


/* strings for std io names */
#define STRSTDIN(ptr) ((ptr) == NULL ? "(stdin)" : (ptr))
#define STRSTDOUT(ptr) ((ptr) == NULL ? "(stdout)" : (ptr))
#define STRSTDERR(ptr) ((ptr) == NULL ? "(stderr)" : (ptr))


/* Borland C 3.1 under MSDOS does not allow to have large variables (2k or
	so) local to functions. Declare those static. */
#ifdef MSDOS_BC
#define BORCRAP_largevar static
#else
#define BORCRAP_largevar
#endif


/* I/O buffer */
/* functions in here assume:
	BUFSIZE is divisable by 4!
	BUFSIZE >= WAVHEADERSIZE!
	BUFSIZE >= CDAUDIOSECTORSIZE!
*/
/*#define BUFSIZE (CDAUDIOSECTORSIZE)
  #if BUFSIZE != (4*(BUFSIZE/4))
  #error BUFSIZE must be divisable by 4!
  #endif
  #if BUFSIZE < WAVHEADERSIZE
  #error BUFSIZE must be >= WAVHEADERSIZE!
  #endif
  #if BUFSIZE < CDAUDIOSECTORSIZE
  #error BUFSIZE must be >= CDAUDIOSECTORSIZE!
  #endif*/
typedef union {
	UINT8  i8 [CDAUDIOSECTORSIZE];
	SINT16 i16[CDAUDIOSECTORSIZE/2];
	UINT32 i32[CDAUDIOSECTORSIZE/4];
} audiosect_t;


/* program exit codes */
typedef enum {
	ERR_OK = 0,		/* successful completion */
	ERR_USAGE,		/* usage or help requested + displayed */
	ERR_CMDARG,		/* error with a cmd arg, syntax, or missing parameter */
	ERR_IO,			/* I/O error */
	ERR_NOMEM,		/* memory allocation failed */
	ERR_INTERNAL	/* internal error (shouldn't happen ;-) )*/
} exit_t;


/* max scale factor (more than 16 bit makes little sense; also limited by
	process_t->iscale) */
#define SCALE_MAX SHRT_MAX
#define SCALE_MIN SHRT_MIN


/* the result of command line parameter scanning */

typedef enum {
	AF_default, AF_raw, AF_cdr, AF_wav, AF_err
} audioformat_t;

#define AF_NAMEIDX {"dflt", "raw", "cdr", "wav", "???"}

typedef struct {
	BOOL little_input;		/* input data is little endian */
	BOOL little_output;		/* output data as little endian */
	signed long iscale;		/* scale factor for signal level,
								integer arithmetic, 100 = no scaling
								(big size for range checking, allow negative) */
	float fscale;			/* scale factor for input data, 
								floating point arithmetic, 1.0 = no scaling */
	BOOL swapwords;			/* swap halves of a 32-bit word, i.e. channels */
	BOOL monostereo;		/* convert to mono then to stereo */
	BOOL silencecuts;		/* find cut numbers for silent intervals */
	BOOL silenceinfo;		/* as silencecuts, but print more info */
	long silence_thresh; 	/* silence cuts: see process_t */
	long silence_delay; 	/* silence cuts: see process_t */
	unsigned long fadein;	/* fade in at the start, in bytes */
	unsigned long fadeout;	/* fade in out the end, in bytes */
	audioformat_t informat;	/* input file format */
	audioformat_t outformat;/* output file format */
	unsigned long startsilence;	/* silence to add at start, in bytes */
	unsigned long endsilence;	/* silence to add at end, in bytes */
	string *infilename;		/* input filename */
	string *outfilename;	/* output filename */
	int numcuts;			/* number of cut positions */
	string **cutstarts;		/* array of the cut positions */
	BOOL version;			/* version requested */
	BOOL usage;				/* usage requested */
	BOOL help;				/* help requested */
	BOOL quiet; 			/* no messages */
	BOOL verbose; 			/* more messages / more output */
} cmdarg_t;

/* default values and initialisation: */
#define CMDARG_DEFAULT \
	{\
		TRUE, FALSE,\
		100, 1.0,\
		FALSE, FALSE, FALSE, FALSE, 10, 30, 0, 0,\
		AF_wav, AF_cdr, 0, 0,\
		NULL, NULL,\
		0, NULL,\
		FALSE, FALSE, FALSE, FALSE, FALSE\
	}
#define RAW_FORMAT_DEFAULT_IS_LITTLE FALSE


/*
	Operations which can be done on a sector/block of the data

	Buffer sizes are declared with type size_t; on systems where that is only
	16 bit (32 k bytes - 1) will be the largets sector size which can be
	processed. System library functions (e.g. string.h) can not handle more
	than size_t.
	
	This is kind of extendible, by adding more values at the end of the
	structure. Making it extendible without having to change the struct every
	time would require function pointers. In that case - how to pass args? (e.g.
	threshold to silencecuts?)
*/

#define PROCESS_MAX_MULTIPLY 2

typedef enum {
	MS_NONE, MS_TOMONO, MS_TOSTEREO, MS_TOMONOSTEREO
} monostereo_t;

/* type counting CD sectors; this must be >=32 bit, so we can't use size_t as
	that may only be int which may only be 16 bit (e.g. Borland C 5) */
typedef unsigned long cdseccount_t;

/*
	Information for processing 1 block of data.
	Some of the variables will be modified during processing and returned!
*/
typedef struct {
	size_t n_in;			/* number of bytes in buffer */
	size_t n_returned;		/* number of bytes returned by operation,
								this can be PROCESS_MAX_MULTIPLY times larger
								than n_in!! */
	/* Is it useful to have the buffer pointer in here too? Or 2 pointers, for
		source and destination buffer?
		Currently buffer ptr is passed as func arg, and data is written back
		to source buffer */
	BOOL first; 			/* this is the first block to process
								not yet used, but will be necessary for re-
								entrant code;
								or better use an initialisation function? */
	/*
	short id; or better: callerid_t caller_id; ?
	for shared libraries - to identify the caller? together with init function?
	*/
	BOOL last;				/* this will be the last block to process */
	BOOL little_host;		/* host is little endian */
	BOOL from_little;		/* input data is little endian */
	BOOL to_little;			/* generate little endian data */
	signed short iscale;	/* scale values to this many %; only 16 bit! */
	float fscale;			/* scale values by this factor */
	BOOL swap_channels;		/* swap first (left) and second (right) channel */
	monostereo_t monostereo;/* convert to mono/stereo */
	BOOL silencecuts;		/* find cut numbers, cutting at silent intervals */
	unsigned short silence_thresh; 	/* silence cuts: threshold */
	unsigned short silence_delay;	/* silence cuts: num of blocks required to
										be below the threshold */
	BOOL silence_val;		/* silence cuts: return silence value of sector */
	cdseccount_t fadein;	/* level of fade-in / fade-out for THIS sector */
	cdseccount_t fadeout;	/* 1..100, 0 = no fading; these variables are
							   modified during processing and must keep value
							   from one call to the next! */
} process_t;



/*
	function prototypes
*/
/* in wav2cdr.c */
void version (void);
void usage (void);
void help (void);
void exit_error (exit_t err, const char *errtext, const char *errtext2);
BOOL is_localhost_little (void);
int timeprintf (string *buf, unsigned long bytes);
#define TIMESTRSIZE (10 + 5/*safe*/)
int main (int argc, char *argv[]);

/* in cmdarg.c */
void scan_cmd_args (int argc, char **argv);
void set_message_output (void);
void check_cmd_args (void);
void showcmdargs (void);
signed long get_input_size_in_cd_blocks (void);
unsigned long get_cut_value (int n);
unsigned long get_silence_value (const string *s);
void get_cmdarg_info (cmdarg_t *pcmdarg);
void init_process_info (process_t *pinfo);

/* in data.c */
void open_out (int track);
void close_out (void);
int get_af_header_size (audioformat_t af);
void read_header (void);
void write_header (void);
void write_trailer (void);
void add_silence (unsigned long bytes);
BOOL copy_sector_check_file (void);
BOOL got_silence_number (char **pstr, UINT16 *ps);
BOOL got_cut_number (char **pstr, unsigned long *pi);
void sprint_cutinfo (void *buffer, unsigned long start, unsigned long end,
						BOOL is_audio_interval);
void silence_info (void *buffer, BOOL eof);
void handle_sectors (void);
void do_data_io (void);

/* in fileio.c */
void open_input_file (const string *name);
void close_input_file (void);
signed long get_file_size (const string *name, FILE *stream);
void open_output_file (const string *name, int track);
void close_output_file (void);
FILE *open_message_file (void);
void close_message_file (void);
void emergency_close (void);
void read_wav_header (wav_header_t *header);
void make_wav_header (wav_header_t *header, unsigned long databytes);
void write_wav_header (const wav_header_t *header);
size_t read_block (void *buf, size_t bytes);
BOOL read_eof (void);
size_t write_block (const void *buf, size_t bytes);

/* in process.c */
void process_swap_bytes (void * buf, process_t *pinfo);
void process_swap_words (void * buf, process_t *pinfo);
void process_swap_tolocal (void * buf, process_t *pinfo);
void process_swap_totarget (void * buf, process_t *pinfo);
void process_tomono (void * buf, process_t *pinfo);
void process_tostereo (void * buf, process_t *pinfo);
void process_tomonostereo (void * buf, process_t *pinfo);
void process_swap_iscale (void * buf, process_t *pinfo);
void process_swap_fscale (void * buf, process_t *pinfo);
void process_swap_noscale (void * buf, process_t *pinfo);
void process_fading (void * buf, process_t *pinfo);
void process_silencecuts (void * buf, process_t *pinfo);
void process_sector (void *buf, process_t *pinfo);



/* EOF wav2cdr.h */
/******************************************************************************/
