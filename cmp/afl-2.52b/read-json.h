/*
   Read JSON mapping files into memories
   ------------------------------------------------------------------

   Written and maintained by Liang Cheng <cgnail@qq.com>

   Copyright 2018 TCA Lab, ISCAS. All rights reserved.

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at:

     http://www.apache.org/licenses/LICENSE-2.0

   This allocator is not designed to resist malicious attackers (the canaries
   are small and predictable), but provides a robust and portable way to detect
   use-after-free, off-by-one writes, stale pointers, and so on.

 */
#include <stdio.h>
#include <stdlib.h>     // atoi()
#include <string.h>
#include <sys/stat.h>   //  lstat()
#include <sys/file.h>
#include <unistd.h>     //  access()
#include <limits.h>

#include "alloc-inl.h"  //  ck_alloc()
#include "types.h"
#include "cJSON/cJSON.h"
#include "uthash.h"
#include "list.h"

struct cmp_addr_lentry {
    char addr[19];
    struct list_head list;
};

struct l2a_hentry {
    int loc;                   /* key */
    char src[19];
    char tgt[19];
    struct cmp_addr_lentry *caddr_list;
    UT_hash_handle hh;         /* makes this structure hashable */
};

struct bb_hentry {
    char addr[19];
    char lchild[19];
    int lloc;
    char rchild[19];
    int rloc;
    UT_hash_handle hh;
};

struct pp_lentry {
    int start;
    int end;
    struct list_head list;
};

struct i2p_hentry {
    char addr[19];
    struct pp_lentry *pos_pair;
    UT_hash_handle hh;
};

struct boundary_entry {
    char tuple[38];
    int hit_cnt;
    struct list_head list;
};

struct i2k_hash_entry {
    char addr[19];
    char keyoff_word[256][256];
    UT_hash_handle hh;
};


extern struct l2a_hentry *l2a_htab;
extern struct bb_hentry *bb_htab;
extern struct i2p_hentry *i2p_htab;
extern struct i2k_hash_entry *i2k_hash_tab;

void delete_loc_htab(struct l2a_hentry *htab);
void delete_bb_htab(struct bb_hentry *htab);
void delete_i2p_htab(struct i2p_hentry *htab);
void delete_i2k_htab(struct i2k_hash_entry *htab);

int read_ins2pos(char *fname);
void read_loc2cmp(u8* bname);
void read_loc2addr(u8* bname);
void find_loc2cmp(int bzdqsmmz, size_t M, size_t N, char ret[M][N]);
void read_ins2keyoff(char *fname);
int find_cmp_addr_in_ins2keyoff(size_t S, char (*cmp_addr)[S], size_t N, size_t M, char ret[N][M]);
int find_mutate_loc_in_outbuf(u8* out_buf, int len, size_t N, char keyoff_word[][N], size_t M, int ret[M]);
int find_cmp_addr_in_outbuf(u8* out_buf, int len, size_t S, char (*cmp_addr)[S], size_t N, size_t M, u8 ret[N][M]);