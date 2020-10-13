CC=/home/wangws/wws_home/Fuzz_oggvideotools/origin/afl-2.52b/afl-gcc
CFLAGS=-g -O0

PNG_DIR=./libpng
LIBPNG=$(PNG_DIR)/.libs/libpng16.a
LIBS=$(LIBPNG) -lm -lz

readpng-file-input: readpng-file-input.c
	$(CC) $(CFLAGS) $^ $(LIBS) -o $@

# taint-example: do_lines.o taint-example.c 
# 	ar rcs libdo_lines.a $<
# 	$(CC) -o $@ taint-example.c -L. -ldo_lines
# 	objdump -d $@ > taint-example.asm

all: readpng-file_input
%.o: %.c
	$(CC) -c $<
clean:
	rm -f *.o
	rm -f readpng-file-input
