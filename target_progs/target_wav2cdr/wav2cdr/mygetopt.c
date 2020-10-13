/*
********************************************************************************
File: mygetopt.c

Tab size:           4
Max line length:    80
Programmer:         Volker Kuhlmann


mygetopt 1.1 Copyright (C) 1997 Volker Kuhlmann
This program is free software under the terms of the GNU General Public License
version 2 (or later, at your option).
See the file COPYING for details about license terms and warranty.
<VolkerKuhlmann@gmx.de>


DESCRIPTION:

Function for command line argument scanning.
The function arguments are the same as for GNU getopt_long() and
getopt_long_only(), this is meant to be a (less functional!) replacement for
when the GNU functions are not available.

This module and its header work stand-alone and can be linked with any other
program.

Written with strict ANSI conformance.


CONDITIONALS:
	DEBUG			Generates some debugging output on stderr.


HISTORY:

1.1   08Dec97	Commented. Example.
1.1   07Dec97	Added scanning of short options, made more GNU compliant.
1.0   04Dec97	Created

********************************************************************************
*/



#include <string.h>

#ifdef DEBUG
 #include <stdio.h>
 #define STRNULL(charptr) ((charptr) == NULL ? "(NULL)" : (charptr))
#endif

#include "mygetopt.h"

#ifndef AND
 #define AND     &&
#endif



/* global variables (exported) */
int optind = 1;
char *optarg = NULL;
int optopt = '?';
int opterr = 1;



/*
	My (quick) version of the GNU C library getopt_long() function.
	Arguments are the same, though functionality is far less extensive.

	No sorting of argv[] is done, scanning is terminated with the first
	unrecognised option.
	Short options can not be grouped together in a single word.
	Options to both short and long options must be separated by a space (i.e.
	be the next word on the command line). Options with optional options are
	treated as if they required an option.
	No abbreviations are recognised.
	
	When returning -1, option scanning is finished and optind points to the
	first command line word following the last option.
	Between calls, optind shows whatever (the GNU version jumps all over the
	place).
	If options->flag is != NULL, it gets the value of options->val and 0 is
	returned, otherwise options->val is returned.
	Returns '?' when a short option is not recognised (optopt contains short
	option), or when a long option is not recognised (optopt contains 0 and
	optind points to the option + 1), or when a required argument is missing
	(optopt contains the short option resp. options->val and optind points to
	option + 1 i.e. NULL for long options).

	Side: if the last recognised option requires an argument, it is returned in
			optarg; updates optind, and optopt when returning '?'
	In: see function arg list
	Out: index into options array if option was recognised
	Return: -1, 0, '?', or value
*/
int getopt_long (
		int argc, 
		char *const argv[],
		const char *shortoptions,
		const struct option *options, 
		int *lidx)
{
int opt = 0;	/* counter for option array */
char *thisarg;

	#ifdef DEBUG
	 fprintf (stderr, "my getopt_long(), optind: %i\n", optind);
	#endif
	if (optind == 0)	/* init */
		optind = 1;
	else
		optind++;	/* advance to index the currently analysed arg */

	if (optind >= argc) /* finish if we run out of arguments */
		return -1;

	/* finish if this argument does not start with '-' and therefore can be no
		option */
	if ((thisarg = argv[optind])[0] != '-')
		return -1;
	thisarg++;	/* advance thisarg till after '-' */
	#ifdef DEBUG
	 fprintf (stderr, " thisarg: %s\n", thisarg);
	#endif
	if (thisarg[0] == '\0')
		return -1;	/* current arg is exactly "-", can't sort -> finished */
	
	/* deal with short options */
	if (thisarg[0] != '-') {
		char *pc;

		pc = strchr (shortoptions, thisarg[0]);
		#ifdef DEBUG
		 fprintf (stderr, " thisarg: <%s>, strchr: <%s>\n", 
		 					thisarg, STRNULL(pc));
		#endif
		if (pc == NULL) {
			optopt = thisarg[0];	/* illegal short option */
			return '?';
		}
		if (pc[1] == ':') {
			optind++;
			optarg = argv[optind];
			if (optind >= argc)
				return optopt = pc[0], '?';  /* requires arg and none left */
		}
		return *pc;
	} /* end scan short option */

	if (*++thisarg == '\0') { /* if arg was -- stop opt processing */
		optind++;
		return -1;
	}

	/* scan long option */
	#ifdef DEBUG
	 fprintf (stderr, " optind: %i, thisarg: %s\n", optind, thisarg);
	#endif
	while (options[opt].name != NULL) {
		if (strcmp (thisarg, options[opt].name) == 0) {
			/* have recognised option */
			*lidx = opt;
			if (options[opt].has_arg != no_argument) {
				/* update *lidx, optind, optarg */
				optind++;
				optarg = argv[optind];
				if ((options[opt].has_arg == required_argument)
					&& (optind >= argc))
					/* requires arg and there is none left */
					return optopt = options[opt].val, '?';
			}
			/* update flag or return val */
			if (options[opt].flag == NULL)
				return options[opt].val;
			else {
				*options[opt].flag = options[opt].val;
				return 0;
			}
		}
		opt++;
	}

	optopt = 0x00;
	return -1;	/* unrecognised option, can't sort therefore must finish */
	
} /* getopt_long() */



#if 0

This is an example showing how to use it.

/*
	Parse command line and do some checks for consistency.
	In: argc, argv
	Out: ---
	Return: ---
*/
void scan_cmd_args (int argc, char **argv)
{
int lidx;
int r = 1;
enum {
	o_whatever = 1,
	o_swapwords0, o_swapwords1
	};
const string shortoptions[] = "hi:o:";
const struct option longoptions[] = {
	{"help",		no_argument, NULL, 'h'},
	{"usage",		no_argument, cmdarg.usage, TRUE},
	{"whatever",	no_argument, NULL, o_whatever},
	{"swapwords",	no_argument, NULL, o_swapwords1},
	{"noswapwords",	no_argument, NULL, o_swapwords0},
	{"in",			required_argument, NULL, 'i'},
	{"out",			required_argument, NULL, 'o'},
	{NULL,			no_argument, NULL, 0}
	};
	
	optind = 0;  /* tell getopt to init */
	while (r != -1) {
		r = getopt_long (argc, argv, shortoptions, longoptions, &lidx);
		switch (r) {
		case 'h':
			cmdarg.help = TRUE;
			break;
		case o_swapwords1:
			cmdarg.swapwords = TRUE;
			break;
		case o_swapwords0:
			cmdarg.swapwords = FALSE;
			break;
		case o_whatever:
			cmdarg.whatever = whatever;
			break;
		case 'i':
			cmdarg.infilename = optarg;
			break;
		case 'o':
			cmdarg.outfilename = optarg;
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
				exit_error (ERR_CMDARG, 
							argv[optind - 1], " requires an argument");
			else {
				string s[2];
				s[0] = isprint (optopt) ? optopt : ' '; s[1] = '\0';
				exit_error (ERR_CMDARG, "unknown short option: ", s);
			}
		default:
			/* internal error */
		}
	}

	/* remaining arguments (if any) parameters to be scanned by the user */
	remaining = argc - optind;

} /* scan_cmd_args() */



#endif  /* example */



/* EOF mygetopt.c */
/******************************************************************************/
