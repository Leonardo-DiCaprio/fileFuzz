## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = foreign dist-zip

bin_PROGRAMS = vorbisgain

INCLUDES = @OGG_CFLAGS@ @VORBIS_CFLAGS@

vorbisgain_LDADD = @VORBIS_LIBS@ @OGG_LIBS@

vorbisgain_SOURCES = vorbisgain.c vorbis.c recurse.c misc.c gain_analysis.c \
                     vcedit.c getopt.c getopt1.c \
                     vorbisgain.h vorbis.h recurse.h misc.h gain_analysis.h \
                     vcedit.h getopt.h i18n.h

mandir = @mandir@

man_MANS = vorbisgain.1

EXTRA_DIST = $(man_MANS) NEWS README vorbisgain.txt

debug:
	$(MAKE) all CFLAGS="@DEBUG@"

profile:
	$(MAKE) all CFLAGS="@PROFILE@"

vorbisgain.txt: vorbisgain.1
	groff -man -Tascii $< | sed "s/\x1b\[..\?m//g" >$@
