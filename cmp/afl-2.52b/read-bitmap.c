#include <stdio.h>
#include <stdlib.h>     // atoi()
#include <string.h>
#include <sys/stat.h>   //  lstat()
#include <sys/file.h>
#include <unistd.h>     //  access()
#include "alloc-inl.h"  //  ck_alloc()
#include "types.h"

u8 trace_bits[MAP_SIZE];
u8  virgin_bits[MAP_SIZE];

void read_bitmap(u8* vname, u8* tname) {

  s32 fd = open(vname, O_RDONLY);

  if (fd < 0) PFATAL("Unable to open '%s'", vname);

  ck_read(fd, virgin_bits, MAP_SIZE, vname);
  close(fd);

  fd = open(tname, O_RDONLY);

  if (fd < 0) PFATAL("Unable to open '%s'", tname);

  ck_read(fd, trace_bits, MAP_SIZE, tname);
  close(fd);

}

int main(int argc, char* argv[]) {

    u32 i;

    if (argc != 3) {
        printf("E: Need virgin_bits and trace_bits files!\n");
        exit(-1);
    }

    read_bitmap(argv[1], argv[2]);

    for (i = 0; i < MAP_SIZE; i++) {

        if (virgin_bits[i] != 0xff) 
            printf("%d: trace: %d, virgin: %d \n", i, trace_bits[i], virgin_bits[i]);
    }

    return 0;

}