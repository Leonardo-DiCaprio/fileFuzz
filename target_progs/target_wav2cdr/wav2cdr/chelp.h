/*
************************************************************************
File chelp.h


Tab size:           4     (all tabs converted to spaces)
Max line length:    72
Programmer:         Volker Kuhlmann


DESCRIPTION:

General useful definitions for the C language, like TRUE, BITAND, etc.,
or mnemonic names for basic C operators.


CONDITIONALS:
    ANSIEXT             Required for ANSI extensions (currently only
                        types XINT64, llword)

    NO_USHORTINTLONG    Inhibits typedefs for ushort, uint, ulong
    WANT_USHORTINTLONG  Forces "
                        (default is to guess)

    LINUX_GCC           Required when compiling with Linux gcc
    (SOLARIS_GCC          " with Solaris gcc)
    (SOLARIS_CC           " with Solaris cc)
    MSDOS_BC             " with MSDOS Borland C (3.1 as of now)

    various             Predefined by various compilers with various
                        cmd line options


HISTORY:

3.48 18 Jan 06	Fixed [SU]INT32 (long is 8 bytes on AMD64)
3.47 21 Feb 99  Added __alpha checks from Bart Warmerdam <bartw@xs4all.nl>.
3.46 26 May 98  HAS_DISENABLED.
3.45 02 May 98  Split up a long #define - Borland C is a little limited.
3.44 17 Feb 98  Commented.
3.43 04 Dec 97  Added EITHEROR.
3.42 30 Nov 97  Made WAIT_FOREVER require (). Scattered a few more #ifdefs.
                Commented assert() with fixed sizes.
3.4  26 Nov 97  Added typedefs UINT.., SINT..
3.31 28 Jul 97  Comments.
3.3  11 Jul 97  Renamed SET/CLRBITS to ASET/ACLRBITS, new SET/CLRBITS.
3.21 04 Jul 97  Commented more; more #defines.
3.2  02 Jul 97  Renamed SPACE to CHR_SPACE. Added more HAS_xxx. Took out
                types lint, ulint which were a bit silly.
3.12 01 Jul 97  Added HAS_BOOL
3.1  03 Feb 97  Added conditionals SOLARIS_GCC, SOLARIS_CC, MSDOS_BC,
                then ignoring them and using builtin stuff instead;
                NO_UNSIGNED
3.04 29 Jan 97  Added conditional ANSIEXT; REPEAT, UNTIL.
3.03 28 Jan 97  Changed comments.
3.02 26 Jan 97  Added lint, ulint. Slightly reorganised.
3.0  21 Jan 97  Changed name to chelp.h, because cdefs.h is used by gcc.
                Commented more.
2.12 28 Mar 96  Added conditional GCC.
2.1  09 Mar 96  Added SETBITS, CLRBITS.
2.01 01 Mar 96  Added comments to mnemonical data types.
2.0  15 Feb 96  Commented, mnemonical data types are now typedef's.
1.92 25 Feb 95  Added SPACE.
1.9  09 Feb 95  Changed AAND to ABITAND, AOR to ABITOR, AXOR to ABITXOR.
1.8  24 Aug 94  Added ENABLED / DISABLED.
1.7  11 Aug 94  Added BITINV.
1.6  09 Aug 94  Added WAITFOR.
1.4  28 Jul 94  Changed almost everything.
1.1  08 Jul 94  Modified.
1.0  06 Jul 94  Created out of a few other files. Added a few things.

************************************************************************
*/


#ifndef CHELP_H
#define CHELP_H


/* Mnemonics for logical and bit-wise operators
*/
#define NOT     !
#define AND     &&
#define OR      ||
#define LOGNOT  !
#define LOGAND  &&
#define LOGOR   ||
#define BITINV  ~
#define BITNOT  ~
#define BITAND  &
#define BITOR   |
#define BITXOR  ^
#define ABITAND &=
#define ABITOR  |=
#define ABITXOR ^=

#define EITHEROR(cond1,cond2) (((cond1) != 0) BITXOR ((cond2) != 0))
    /* evaluates arguments only once */


/*
    Shortcuts for unsigned short, int, long
    These are a hassle - it's best not to use them. There is no way
    to find out whether they are defined already, so we guess. If the
    guess fails, overide with NO_USHORTINTLONG or WANT_USHORTINTLONG.

    Linux gcc:
        always typedefs these (and defines _LINUX_TYPES_H)
    Solaris gcc 2.7.2:
        defines __GNUC__ (without -traditional),
        __STRICT_ANSI__ (with -ansi);
        unistd.h typedefs these (and defines _UNISTD_H).
    Solaris cc SC4.0 18 Oct 1995 C 4.0:
        defines __STDC__ = 1 for strict conformance;
        unistd.h typedefs these (and defines _UNISTD_H)
    Borland C:
        defines __TURBOC__, __BORLANDC__
        typedefs these?
    TI TMS320C3x:
        ?
*/
#define GUESS1__ \
              NOT (defined(LINUX_GCC) OR defined(_LINUX_TYPES_H)) \
          AND defined(__GNUC__) \
          AND defined(__STRICT_ANSI__) \
          AND NOT defined(_UNISTD_H)
#define GUESS2__ \
              (__STDC__-0 == 1) \
          AND NOT defined(__GNUC__) \
          AND NOT defined(_UNISTD_H)
#define USHORTINTLONG_GUESS__ ( \
          GUESS1__ \
        OR \
          GUESS2__ \
        OR \
          (defined(MSDOS_BC) OR defined(__TURBOC__)) \
        )
#if    defined (WANT_USHORTINTLONG) \
    OR (NOT defined(NO_USHORTINTLONG) AND USHORTINTLONG_GUESS__)
#define HAS_USHORTINTLONG
typedef unsigned short      ushort;
typedef unsigned int        uint;
typedef unsigned long       ulong;
#endif


/*
    Shortcut for unsigned char
    Does not seem to clash with any compiler / system
*/
#ifndef HAS_UCHAR
#define HAS_UCHAR
typedef unsigned char       uchar;
#endif


/* Shortcuts for common data types (size is compiler dependent)
*/
#ifndef HAS_STRING
#define HAS_STRING
typedef char                string;
#endif


/* Data types with fixed sizes (independent of compiler)
*/
#ifndef HAS_FIXEDSIZES
#define HAS_FIXEDSIZES
typedef unsigned char       UINT8, byte;            /*  8 bits */
typedef unsigned short      UINT16, dbyte, word;    /* 16 bits */
#if defined(__alpha) OR defined(__x86_64__)
  typedef unsigned int      UINT32,
                            qbyte, dword, lword;    /* 32 bits */
#else
  typedef unsigned long     UINT32,
                            qbyte, dword, lword;    /* 32 bits */
#endif

typedef signed char         SINT8;                  /*  8 bits signed */
typedef signed short        SINT16;                 /* 16 bits signed */
#if defined(__alpha) OR defined(__x86_64__)
  typedef signed int        SINT32;                 /* 32 bits signed */
#else
  typedef signed long       SINT32;                 /* 32 bits signed */
#endif

#ifdef ANSIEXT
#ifdef __alpha
  typedef unsigned long     UINT64, llword;         /* 64 bits */
  typedef signed long       SINT64;                 /* 64 bits signed */
#else
  typedef unsigned long long  UINT64, llword;       /* 64 bits */
  typedef signed long long    SINT64;               /* 64 bits signed */
#endif
#endif
/* check the sizes here? then we would depend on limits.h
   better to require the user to use assert():
    assert (sizeof(UINT8) == 1);
    assert (sizeof(UINT16) == 2);
    assert (sizeof(UINT32) == 4);
    assert (sizeof(SINT8) == 1);
    assert (sizeof(SINT16) == 2);
    assert (sizeof(SINT32) == 4);
    assert (sizeof(UINT64) == 8);
    assert (sizeof(SINT64) == 8);
*/
#endif


/* Mnemonical data types
    It should be possible to choose (int) for these, in case sizeof(enum) <
    sizeof(int). Essential for GNU getopt().
*/
#ifndef HAS_BOOL
#define HAS_BOOL
typedef enum {FALSE = 0, TRUE = !0}     BOOL;
    /* Note: all values != FALSE count as TRUE.
             Always compare with FALSE! */
#endif

#ifndef HAS_OFFON
#define HAS_OFFON
typedef enum {OFF, ON = NOT OFF}        OFFON;
    /* Note: all values != OFF count as ON.
             Always compare with OFF, never ever ever with ON! */
#endif

#ifndef HAS_PASSFAIL
#define HAS_PASSFAIL
typedef enum {PASS, FAIL = NOT PASS}    PASSFAIL;
    /* Note: only the values PASS and FAIL are legal in this type!!!
             Must compare with either PASS or FAIL.
             Do not assume either PASS or FAIL are 0 or 1 or -1 or anything!!
             I.e. do not compare with 0!! */
#endif

#ifndef HAS_DISENABLED
#ifndef DISABLED
#define DISABLED    0
#endif
#ifndef ENABLED
#define ENABLED     (NOT DISABLED)
#endif
#endif


/* Mnemonics for control flow
*/
#define WAIT_FOREVER()  for (;;){}
#define FOREVER         while (TRUE)
#define WAITFOR(cond)   while (NOT((cond)))
#define REPEAT          do {
#define UNTIL(cond)     } while (NOT((cond)))


/* Handling bits in hardware registers
   Bits set in the bit field will be set/cleared in the register.
*/
/* use: xxxBITS (register, bit field); */
#define SETBITS(reg,bits)   ((reg)) ABITOR ((bits))
#define CLRBITS(reg,bits)   ((reg)) ABITAND (BITINV ((bits)))

/* use: hw_register = AxxxBITS (bit field); */
#define ASETBITS(bits)      ABITOR (bits)
#define ACLRBITS(bits)      ABITAND (BITINV (bits))


/* ASCII Character names
*/
/* these (all 34; 0-32, 127) should prob go into a separate file */
#if 0
#ifndef HAS_CHARNAMES
#define HAS_CHARNAMES
#define CHR_NUL '\x00'
#define CHR_BEL '\x07'
#define CHR_BS  '\x08'
#define CHR_HT  '\x09'
#define CHR_NL  '\x0a'
#define CHR_LF  '\x0a'
#define CHR_VT  '\x0b'
#define CHR_NP  '\x0c'
#define CHR_CR  '\x0d'
#define CHR_ESC '\x1b'
#define CHR_SPC '\x20'
#define CHR_DEL '\x7f'
#define Ctrl(letter) ((char) (letter) >= 'a' ? \
                        (letter) - '`' : (letter) - '@')
#endif
#endif


#endif  /* #ifndef CHELP_H */

/* EOF chelp.h */
/**********************************************************************/
