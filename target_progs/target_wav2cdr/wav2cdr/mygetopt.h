/*
********************************************************************************
File: mygetopt.h

Tab size:           4
Max line length:    80
Programmer:         Volker Kuhlmann


mygetopt 1.1 Copyright (C) 1997 Volker Kuhlmann
This program is free software under the terms of the GNU General Public License
version 2 (or later, at your option).
See the file COPYING for details about license terms and warranty.
<VolkerKuhlmann@gmx.de>


DESCRIPTION:

Header file for mygetopt.c.


CONDITIONALS:
	none


HISTORY:

1.0   04Dec97	Created

********************************************************************************
*/



#ifndef MYGETOPT_H
#define MYGETOPT_H



/* options to options */
enum {
	no_argument = 0,		/* no option */
	required_argument = 1,	/* option is required */
	optional_argument = 2	/* option is optional */
};

struct option
{
	const char *name;		/* name of long option */
	int has_arg;			/* has this option an option? */
	int *flag;				/* address for a return value */
	int val;				/* value copied to *flag, or returned */
};



/* index of arguments scanned so far, or index to first 
	argument not an option */
extern int optind;

/* value of the option to the option, if there was one (or NULL?) */
extern char *optarg;

/* when returning '?' (short or long option not recognised), this is the
(short) option which was unrecognised */
extern int optopt;

/* store 0 here to suppress error messages from getopt */
extern int opterr;



int getopt_long (
		int argc, 
		char *const argv[],
		const char *shortoptions,
		const struct option *options, 
		int *lidx);



#endif  /* #ifndef MYGETOPT_H */



/* EOF mygetopt.h */
/******************************************************************************/
