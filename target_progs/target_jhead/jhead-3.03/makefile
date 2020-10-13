#--------------------------------
# jhead makefile for Unix
#--------------------------------
OBJ=.
SRC=.
CC=/home/wws/Fuzz_oggvideotools/cmp/afl-2.52b/afl-gcc
CFLAGS:= $(CFLAGS) -O3 -Wall -g

all: jhead

objs = $(OBJ)/jhead.o $(OBJ)/jpgfile.o $(OBJ)/jpgqguess.o $(OBJ)/paths.o \
	$(OBJ)/exif.o $(OBJ)/iptc.o $(OBJ)/gpsinfo.o $(OBJ)/makernote.o 

$(OBJ)/%.o:$(SRC)/%.c
	${CC} $(CFLAGS) -c $< -o $@

jhead: $(objs) jhead.h
	${CC} $(LDFLAGS) -o jhead $(objs) -lm

clean:
	rm -f $(objs) jhead

install:
	cp jhead ../install
