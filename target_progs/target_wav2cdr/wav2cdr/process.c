/*
********************************************************************************
File: process.c

Tab size:           4
Max line length:    80
Programmer:         Volker Kuhlmann


wav2cdr 2.3.4 Copyright (C) 1997, 1998, 1999, 2000, 2006 Volker Kuhlmann
This program is free software under the terms of the GNU General Public License
version 2 (or later, at your option).
See the file COPYING for details about license terms and warranty.
<VolkerKuhlmann@gmx.de>


DESCRIPTION:

This module handles all number crunching.
Operations are performed on blocks of data.
Functions in here are stand-alone and do not depend on anything else in wav2cdr
(except for perhaps wav2cdr.h).

As the amount of data returned from processing does not have to be the same as
the amount which goes in, process_t has a variable for the number of bytes
which go out. This is not necessarily set by all functions, but it is by those
which return a differing amount, and by process_sector(). Functions may write to
the buffer area provided, but not return any data.

These functions all take a buffer pointer and a pointer to an initialised
processing info struct (process_t) as arguments. Not all values in the info
struct will be used by all functions.

Functions process_swap_... will take care of the byte order in the input data,
and produce the requested byte ordering in the output data.
Functions process_... assume the byte ordering of the input data is that of the
local machine; the byte ordering is unchanged on return.


CONDITIONALS:
	see wav2cdr.c


HISTORY:
	see wav2cdr.c

********************************************************************************
*/



#include <stddef.h>
#include <limits.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <string.h>
#ifdef DEBUG
#include <stdio.h>
#endif

#include "wav2cdr.h"
#include "chelp.h"
#ifdef VKCLIB					
  #include "vkclib.h"
#endif



/* local function prototypes */
static BOOL is_silent_sector (void * buf, const process_t *pinfo);



/*
	Saturation of numbers.
	Take a long and see whether it fits in a short, if not set to
	short min/max.
	Expression, implemented as macro because it's mission critical :-)
	(Although we could play around with inlining.)
	In: long signed int
	Out: ---
	Return: short signed int
*/
#define SATURATE_SSHORT(argshort) (\
	((argshort) > (signed long) SHRT_MAX) ? \
		SHRT_MAX \
	  :	((argshort) < (signed long) SHRT_MIN) ?\
			SHRT_MIN \
		  : (signed short) (argshort) \
	)



/*
	Swap consecutive bytes with each other.
	The last byte will be ignored if the number of bytes in the buffer is not
	even (unless assert is used).
	This will work with any byte order (in and out)... (of course)
	In: buffer, buffer size
	Out: correct byte order
	Return: ---
*/
void process_swap_bytes (void * buf, process_t *pinfo)
{
size_t i;

	#ifndef NO_ASSERTMANY
	assert ((pinfo->n_in BITAND 0x1) == 0);
	#endif

	for (i = 0; i < (pinfo->n_in / 2); i++)
		SWAP2BYTES (AS_S16(buf)[i]);

} /* process_swap_bytes() */



/*
	Swap consecutive 16-bit values.
	The last up to 3 bytes will be ignored if the number of bytes in the buffer
	is not even (unless assert is used).
	This will work with any byte order (in and out).
	In: buffer, buffer size
	Out: correct byte order
	Return: ---
*/
void process_swap_words (void * buf, process_t *pinfo)
{
size_t i;

	#ifndef NO_ASSERTMANY
	assert ((pinfo->n_in BITAND 0x3) == 0);
	#endif

	for (i = 0; i < (pinfo->n_in / 4); i++)
		SWAP2WORDS (AS_S32(buf)[i]);

} /* process_swap_words() */



/*
	Ensure byte order is that of local machine. Swap if necessary.
	Assuming 16-bit values.
	The last byte will be ignored if the number of bytes in the buffer is not
	even (unless assert is used).
	In: buffer, relevant fields in *pinfo
	Out: correct byte order
	Return: ---
*/
void process_swap_tolocal (void * buf, process_t *pinfo)
{
size_t i;

	#ifndef NO_ASSERTMANY
	assert ((pinfo->n_in BITAND 0x1) == 0);
	#endif

	if (pinfo->from_little != pinfo->little_host)
		for (i = 0; i < (pinfo->n_in / 2); i++)
			SWAP2BYTES (AS_S16(buf)[i]);

} /* process_swap_tolocal() */



/*
	Ensure byte order is that of the target. Swap if necessary.
	Assuming 16-bit values.
	The last byte will be ignored if the number of bytes in the buffer is not
	even (unless assert is used).
	In: buffer, relevant fields in *pinfo
	Out: correct byte order
	Return: ---
*/
void process_swap_totarget (void * buf, process_t *pinfo)
{
size_t i;

	#ifndef NO_ASSERTMANY
	assert ((pinfo->n_in BITAND 0x1) == 0);
	#endif

	if (pinfo->to_little != pinfo->little_host)
		for (i = 0; i < (pinfo->n_in / 2); i++)
			SWAP2BYTES (AS_S16(buf)[i]);

} /* process_swap_totarget() */



/*
	Convert stereo samples to mono. Result is the average of left and right
	channel.
	The result has only half as many values! Stored at the start of the buffer.
	The input byte order must be that of the local machine.
	In: buffer, relevant fields in *pinfo
	Out: correct byte order
	Return: ---
*/
void process_tomono (void * buf, process_t *pinfo)
{
size_t i;

	#ifndef NO_ASSERTMANY
	assert ((pinfo->n_in BITAND 0x3) == 0);
	#endif

	for (i = 0; i < (pinfo->n_in / 4); i++)
		AS_S16(buf)[i] = ((SINT32) AS_S16(buf)[2*i] 
							+ (SINT32) AS_S16(buf)[2*i + 1]) / 2;

	pinfo->n_returned = pinfo->n_in / 2;

} /* process_tomono() */



/*
	Convert mono samples to stereo. The value for the second channel is that of
	the first.
	The given buffer size is filled up by repeating the values in the first half
	of the buffer! (The first value is repeated once, then the second, ...)
	This will work with any byte order (in and out).
	In: buffer, relevant fields in *pinfo
	Out: correct byte order
	Return: ---
*/
void process_tostereo (void * buf, process_t *pinfo)
{
size_t i;

	#ifndef NO_ASSERTMANY
	assert ((pinfo->n_in BITAND 0x3) == 0);
	#endif

	i = pinfo->n_in / 4;	/* this time we need to go downwards!!! */
	pinfo->n_returned = 0;
	if (i == 0) return;
	
	REPEAT
		--i;
		AS_S16(buf)[2*i + 1] = AS_S16(buf)[i];
		AS_S16(buf)[2*i] = AS_S16(buf)[i];
	UNTIL (i == 0);

	pinfo->n_returned = pinfo->n_in * 2;

} /* process_tostereo() */



/*
	Convert stereo samples to mono, and back to stereo.
	Result is the average of left and right channel, and both channels have the
	same value.
	The input byte order must be that of the local machine.
	In: buffer, relevant fields in *pinfo
	Out: the same (average) value for both channels
	Return: ---
*/
void process_tomonostereo (void * buf, process_t *pinfo)
{
size_t i;

	/* must assert this! */
	assert ((pinfo->n_in BITAND 0x3) == 0);

	for (i = 0; i < (pinfo->n_in / 2); i+=2)
		AS_S16(buf)[i] 
			= AS_S16(buf)[i + 1]
			= ((SINT32) AS_S16(buf)[i]
				+ (SINT32) AS_S16(buf)[i + 1]) / 2;

} /* process_tomonostereo() */



/*
	Take a number of 16-bit ints, and scale them with integer arithmetic.
	The results will not overflow but SATURATE_SSHORT.
	The numbers can be in either byte order before and after processing, and the
	algorithm will work on both big and little endian machines.
	The last byte will be ignored if the number of bytes in the buffer is not
	even (unless assert is used).
	In: buffer, scale factor, byte order of host / before / after
	Out: process numbers
	Return: ---
*/
void process_swap_iscale (void * buf, process_t *pinfo)
{
signed long si32;
size_t i;

	#ifndef NO_ASSERTMANY
	assert ((pinfo->n_in BITAND 0x1) == 0);
	#endif

	if ((pinfo->from_little != 0) BITXOR (pinfo->little_host != 0))
		/* input != host: swap first */
		if ((pinfo->to_little != 0) BITXOR (pinfo->little_host != 0))
			/* Swap -> Process -> Swap */
			for (i = 0; i < (pinfo->n_in / 2); i++) {
				si32 = (signed long) pinfo->iscale
						* (signed short) BYTES2SWAPPED (AS_S16(buf)[i]);
				si32 /= 100L;
				AS_S16(buf)[i] = SATURATE_SSHORT (si32);
				SWAP2BYTES (AS_S16(buf)[i]);
			}
		else
			/* Swap -> Process */
			for (i = 0; i < (pinfo->n_in / 2); i++) {
				si32 = (signed long) pinfo->iscale
						* (signed short) BYTES2SWAPPED (AS_S16(buf)[i]);
				si32 /= 100L;
				AS_S16(buf)[i] = SATURATE_SSHORT (si32);
			}
	else
		if ((pinfo->to_little != 0) BITXOR (pinfo->little_host != 0))
			/* Process -> Swap */
			for (i = 0; i < (pinfo->n_in / 2); i++) {
				si32 = (signed long) pinfo->iscale
						* AS_S16(buf)[i];
				si32 /= 100L;
				AS_S16(buf)[i] = SATURATE_SSHORT (si32);
				SWAP2BYTES (AS_S16(buf)[i]);
			}
		else
			/* Process */
			for (i = 0; i < (pinfo->n_in / 2); i++) {
				si32 = (signed long) pinfo->iscale
						* AS_S16(buf)[i];
				si32 /= 100L;
				AS_S16(buf)[i] = SATURATE_SSHORT (si32);
			}
} /* process_swap_iscale() */



/*
	Take a number of 16-bit ints, and scale them with floating point arithmetic.
	The results will not overflow but saturate (16 bits).
	The numbers can be in either byte order before and after processing.
	The algorithm will work on both big and little endian machines.
	The last byte will be ignored if the number of bytes in the buffer is not
	even (unless assert is used).
	In: buffer, scale factor, byte order of host / before / after
	Out: process numbers
	Return: ---
*/
void process_swap_fscale (void * buf, process_t *pinfo)
{
signed long int si32;
size_t i;

	#ifndef NO_ASSERTMANY
	assert ((pinfo->n_in BITAND 0x1) == 0);
	#endif

	if ((pinfo->from_little != 0) BITXOR (pinfo->little_host != 0))
		/* input != host: swap first */
		if ((pinfo->to_little != 0) BITXOR (pinfo->little_host != 0))
			/* Swap -> Process -> Swap */
			for (i = 0; i < (pinfo->n_in / 2); i++) {
				si32 = pinfo->fscale 
						* (signed short) BYTES2SWAPPED (AS_S16(buf)[i]);
				AS_S16(buf)[i] = SATURATE_SSHORT (si32);
				SWAP2BYTES (AS_S16(buf)[i]);
			}
		else
			/* Swap -> Process */
			for (i = 0; i < (pinfo->n_in / 2); i++) {
				si32 = pinfo->fscale 
						* (signed short) BYTES2SWAPPED (AS_S16(buf)[i]);
				AS_S16(buf)[i] = SATURATE_SSHORT (si32);
			}
	else
		if ((pinfo->to_little != 0) BITXOR (pinfo->little_host != 0))
			/* Process -> Swap */
			for (i = 0; i < (pinfo->n_in / 2); i++) {
				si32 = pinfo->fscale * AS_S16(buf)[i];
				AS_S16(buf)[i] = SATURATE_SSHORT (si32);
				SWAP2BYTES (AS_S16(buf)[i]);
			}
		else
			/* Process */
			for (i = 0; i < (pinfo->n_in / 2); i++) {
				si32 = pinfo->fscale * AS_S16(buf)[i];
				AS_S16(buf)[i] = SATURATE_SSHORT (si32);
			}
} /* process_swap_fscale() */



/*
	Take a number of 16-bit ints and swap the bytes if input and output byte
	orders are different.
	The last byte will be ignored if the number of bytes in the buffer is not
	even (unless assert is used).
	In: buffer
	Out: swapped bytes
	Return: ---
*/
void process_swap_noscale (void * buf, process_t *pinfo)
{
size_t i;

	#ifndef NO_ASSERTMANY
	assert ((pinfo->n_in BITAND 0x1) == 0);
	#endif
	
	if ((pinfo->from_little != 0) BITXOR (pinfo->to_little != 0))
		for (i = 0; i < (pinfo->n_in / 2); i++)
			SWAP2BYTES (AS_S16(buf)[i]);

} /* process_swap_noscale() */



/*
*/
void process_fading (void * buf, process_t *pinfo)
{
float fade;
size_t i;
static cdseccount_t fi_len = 0, fo_len = 0;

	#ifndef NO_ASSERTMANY
	assert ((pinfo->n_in BITAND 0x1) == 0);
	#endif

	/* initialise when fading starts */
	if (fi_len == 0 AND pinfo->fadein != 0) {
		fi_len = pinfo->fadein + 1;
	}
	if (fo_len == 0 AND pinfo->fadeout != 0) {
		fo_len = pinfo->fadeout + 1;
	}
	DBGPRINTF2 ("fading in %li, out %li\n", fi_len, fo_len);
	
	/* fade-in and fade-out, possibly combined */
	if (fi_len > 0) {
		if (fo_len > 0) {
			fade = (((float) (fi_len - pinfo->fadein)) / ((float) fi_len))
					* (((float) pinfo->fadeout) / ((float) fo_len));
			if (--pinfo->fadein == 0) fi_len = 0;
			if (--pinfo->fadeout == 0) fo_len = 0;
		} else {
			fade = ((float) (fi_len - pinfo->fadein)) / ((float) fi_len);
			if (--pinfo->fadein == 0) fi_len = 0;
		}
	} else {
		if (fo_len > 0) {
			fade = ((float) pinfo->fadeout) / ((float) fo_len);
			if (--pinfo->fadeout == 0) fo_len = 0;
		} else {
			return; /* both fi_len and fo_len are 0, no fading */
		}
	}
	DBGPRINTF1 ("fading scale: %2.5f\n", fade);
	
	for (i = 0; i < (pinfo->n_in / 2); i++)
		(AS_S16(buf)[i]) = (SINT16) (((float) (AS_S16(buf)[i])) * fade);

} /* process_fading() */



/*
	Decide whether this sector is silent
	In: buffer
	Out: buffer content '\0'-terminated
	Return: TRUE if sector is silent
*/
static BOOL is_silent_sector (void * buf, const process_t *pinfo)
{
#ifdef DEBUG
unsigned long sumabs = 0L;			/* sum of abs(x) */
UINT16 avgabs;
#endif
signed long sum = 0L;				/* sum of x */
SINT16 avg;
unsigned long sumdcpk = 0L; 		/* sum of abs(x - avg) */
UINT16 avgdcpk;

size_t i;
#ifdef DEBUG
static size_t secnum = 0;
#endif
BOOL is_silent;

#define ABS(arg) (((arg) < 0) ? (-(arg)) : (arg))
#define BUF(buf) ((char *) (buf) + strlen (buf))

	if ((pinfo->n_in / 2) == 0) {
		* (char *) buf = '\0';
		return TRUE;	/* nothing to do... */
	}

	/* fast case for threshold == 0: because the silence value of this sector
		can not be less than zero, we treat this as "this is a silent sector if
		all samples are 0" */
	if (pinfo->silence_thresh == 0) {
		for (i = 0; i < (pinfo->n_in / 2); i++)
			if (AS_S16(buf)[i] != 0)
				return FALSE;
		return TRUE;
	}

	/* calculate averages: (sum(x)/N), and (sum(abs(x))/N)
	*/
	for (i = 0; i < (pinfo->n_in / 2); i++) {
	#ifdef DEBUG
	#ifdef MSDOS_BC
		sumabs += ABS ((signed long) AS_S16(buf)[i]);
	#else
		sumabs += ABS (AS_S16(buf)[i]);
	#endif
	#endif
		sum += AS_S16(buf)[i];
	}
	#ifdef DEBUG
	avgabs = sumabs / (unsigned long) (pinfo->n_in / 2);
	#endif
	avg = sum / (signed long) (pinfo->n_in / 2); /* cast here is necessary
		or division stuffs up (signed/unsigned)) */
	/* calculate average of the difference between each value and the average
		of all values
	*/
	for (i = 0; i < (pinfo->n_in / 2); i++) {
	#ifdef MSDOS_BC
		sumdcpk += ABS (avg - (signed long) AS_S16(buf)[i]);
	#else
		sumdcpk += ABS (avg - AS_S16(buf)[i]);
	#endif
	}
	avgdcpk = sumdcpk / (unsigned long) (pinfo->n_in / 2);
	
	/* decide whether this is a "silent" sector:
		This is somewhat tricky. I have seen sectors with a very high DC offset
		in the signal (crappy digitizer hardware?), the DC offset equals about
		the absolutes average (sum(abs(x))/N) in non-quiet sectors.
		Let's subtract the DC offset from the absolutes average and see how far
		that goes. Note very: fails dismally in sectors with only positive
		numbers! but then that would be a rather weird audio signal.
		Let's try the difference between 
	*/
  #if 0
  	/* average of the sum of absolute values: sum(abs(x))/N
		Fails if DC offsets are present. */
	is_silent = (avgabs < pinfo->silence_thresh);
  #elif 0
	/* absolute of the difference between average (which is the DC
		offset), and the above: abs(sum(abs(x))/N - sum(x)/N)
		Fails dismally in sectors with only positive numbers!
		but then that would be a rather weird audio signal */
	is_silent = (ABS ((signed long) avgabs - (signed long) avg) 
		< pinfo->silence_thresh);
		/* without the casts, cond is always false for Borland C 3.1 */
  #else
	/* avg := sum(x)/N; sum(abs(x - avg)) */
	is_silent = (avgdcpk < pinfo->silence_thresh);
  #endif

	* (char *) buf = '\0';
	#ifdef DEBUG
	sprintf (buf,
		" %li: av %6i, +av %5i, d %3i, avdc %5i"
		", s%7li, +s %6li, +sdc %5li, n %i, =%c\n",
		(long) secnum++,
		avg, avgabs, avgabs - avg, avgdcpk, 
		sum, sumabs, sumdcpk, pinfo->n_in / 2, is_silent ? 'Y' : 'N');
		/* note silence_info() scans for this = sign */
	#endif
	/* verbose output of how silent this sector is */
	if (pinfo->silence_val) sprintf (BUF(buf), "S%hu ", avgdcpk);
	
	return is_silent;	
#undef BUF
} /* is_silent_sector() */



/*
	Return cut numbers according to the silent intervals in the input.
	These cut numbers can be fed back into --cut, then every second track
	generated (even numbers) contains a silent interval.
	The silence delay period is part of the signal interval, not the silence
	interval. If the silent interval between two signal intervals is less than
	(2 * silence delay), the silent part at the start of the second signal
	period will be shortened.
	Conditional HARDON_CUT: if set, the silence delay period is counted to the
	silent interval, not the signal interval.
	The input byte order must be that of the local machine.
	This function will produce incorrect results if called with buffer sizes not
	equal to CDAUDIOSECTORSIZE (unless the call is the last).
	In: buffer
	Out: cut numbers
	Return: ---
*/
void process_silencecuts (void * buf, process_t *pinfo)
{
static cdseccount_t secnum = 0; 	/* counter of sectors */
static BOOL sigstarted = FALSE; 	/* a non-silent interval has started */
static cdseccount_t sigstart;		/* start of signal (i.e. non-silence) */
static cdseccount_t
			 silentsectors = 0; 	/* silent sectors so far, this interval */
static cdseccount_t silencestart; 	/* first sector of silence, this interval */
#ifndef HARDON_CUT
static cdseccount_t lastsigend = 0;
#endif

#ifdef DEBUG
	if (secnum == 0) {
		DBGPRINTF1 ("cut: threshold: %i\n", pinfo->silence_thresh);
		DBGPRINTF1 ("cut: delay: %i\n", pinfo->silence_delay);
	#ifdef HARDON_CUT
		DBGPRINTF0 ("cut: HARD\n");
	#endif
	}
	/* in case is_silent_sector() wrote something: */
	#define BUF ((char *) buf + strlen (buf))
#else
	/*#define BUF buf*/
	#define BUF ((char *) buf + strlen (buf))
#endif
	
	/* is_silent_sector() will put a '\0' into the buffer returned */
	if (is_silent_sector (buf, pinfo)) {
		if ((pinfo->n_in / 2) > 0U) {
			if (silentsectors++ == 0U)
				silencestart = secnum;
			if (silentsectors == pinfo->silence_delay  AND  sigstarted) {
			#ifdef HARDON_CUT
				DBGPRINTF2 ("cut: signal: %ldC,%ldC\n", sigstart, silencestart);
				sprintf (BUF, "%ldC %ldC\n", sigstart, silencestart);
			#else
				DBGPRINTF2 ("cut: signal: %ldC,%ldC\n", sigstart, secnum + 1U);
				sprintf (BUF, "%ldC %ldC\n", sigstart, secnum + 1U);
				lastsigend = secnum + 1U;
			#endif
				sigstarted = FALSE;
			}
		}
	} else {
		if (NOT sigstarted) {
		#ifdef HARDON_CUT
			sigstart = secnum;
			if (silentsectors < pinfo->silence_delay)
				sigstart -= silentsectors;
		#else
			sigstart = secnum - (silentsectors < pinfo->silence_delay ?
									silentsectors : pinfo->silence_delay);
			if (sigstart < lastsigend)
				sigstart = lastsigend;
		#endif
			sigstarted = TRUE;
		}
		silentsectors = 0U;
	}
	
	if (pinfo->last  AND  sigstarted) {
		DBGPRINTF2 ("cut: signal: %ldC,%ldC\n", sigstart, secnum);
		sprintf (BUF, "%ldC %ldC\n", sigstart, secnum);
	}
	
	#undef BUF
	secnum++;
	pinfo->n_returned = strlen (buf);
		/* do not include \0 in number of returned bytes because it is more
			hassle than it's worth afterwards; buffer will be large enough
			to hold one more byte */
} /* process_silencecuts() */



/*
	Process one block of data. This will usually be one audio sector, but it can
	be any size divisable by 4.
	Processing depends on the setting of the variables in struct *pinfo.
	Byte swapping and scaling is handled here.
	Runs on big and little endian machines.
	Handles little and big input data, generates little or big output.
	Some care was taken to make things more efficient depending on the
	processing to be done.
	In: buffer, processing info
	Out: buffer contents changed
	Return: ---
*/
void process_sector (void *buf, process_t *pinfo)
{
BOOL order;

	/* always assert here */
	assert ((pinfo->n_in BITAND 0x3) == 0);

	/* preset bytes returned to bytes in; make sure those functions which do
		not return the same amount update n_returned */
	pinfo->n_returned = pinfo->n_in;
	
	/* do any byte swapping and scaling if necessary */
	if (pinfo->iscale != 100)
		process_swap_iscale (buf, pinfo);
	else if (pinfo->fscale != 1.0) /* is a precise float comparison ok here? */
		process_swap_fscale (buf, pinfo);
	else
		process_swap_noscale (buf, pinfo);
	/* now byte order of data is that of target audio format! */
	
	/* More processing with byte swapping to do? */
	if (pinfo->monostereo != MS_NONE
		OR pinfo->silencecuts
		OR pinfo->fadein != 0 OR pinfo->fadeout != 0) {

		/* get byte order of local architecture, swap data to it if necessary */
		order = pinfo->from_little;
		pinfo->from_little = pinfo->to_little;
		process_swap_tolocal (buf, pinfo);

		/* convert between mono and stereo */
		if (pinfo->monostereo != MS_NONE) {
			switch (pinfo->monostereo) {
			case MS_TOMONO:
				process_tomono (buf, pinfo); break;
			case MS_TOSTEREO:
				process_tostereo (buf, pinfo); break;
			case MS_TOMONOSTEREO:
				process_tomonostereo (buf, pinfo); break;
			default:
				; /* tough luck */
			}
		}
	
		/* find silent intervals, and output cut numbers accordingly */
		if (pinfo->silencecuts)
			process_silencecuts (buf, pinfo);
		
		/* process fading */
		if (pinfo->fadein != 0 OR pinfo->fadeout != 0)
			process_fading (buf, pinfo);

		/* swap data back to byte order of target audio format, if necessary */
		process_swap_totarget (buf, pinfo);
		pinfo->from_little = order;
	}

	/* swap pairs of words in buffer */
	if (pinfo->swap_channels)
		process_swap_words (buf, pinfo);
	
} /* process_sector() */



/* EOF process.c */
/******************************************************************************/
