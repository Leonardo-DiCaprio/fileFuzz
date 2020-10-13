#include <stdio.h>
#include <stdlib.h>     // atoi()
#include <string.h>
#include <sys/stat.h>   //  lstat()
#include <sys/file.h>
#include <unistd.h>     //  access()
#include "alloc-inl.h"  //  ck_alloc()
#include "types.h"
#include "cJSON/cJSON.h"
#include "uthash.h"
#include "list.h"

#define TYPE_POS_PAIR 1
#define TYPE_BOUNDRAY 2

u8 trace_bits[MAP_SIZE];
u8  virgin_bits[MAP_SIZE];

struct l2a_hentry {
    int loc;                   /* key */
    char src[11];
    char tgt[11];
    UT_hash_handle hh;         /* makes this structure hashable */
};

struct bb_hentry {
    char addr[11];
    char lchild[11];
    int lloc;
    char rchild[11];
    int rloc;
    UT_hash_handle hh;
};

#define data_of(ptr, type, pmember, dmember) ({               \
        type * _cptr = container_of(ptr, type, pmember); \
        _cptr->dmember;})

struct pp_lentry {
    int start;
    int end;
    struct list_head list;
};

struct i2p_hentry {
    char addr[11];
    struct pp_lentry *pos_pair;
    UT_hash_handle hh;
};

struct boundary_entry {
    char tuple[22];
    int hit_cnt;
    struct list_head list;
};

struct l2a_hentry *l2a_htab = NULL;
struct bb_hentry *bb_htab = NULL;
struct i2p_hentry *i2p_htab = NULL;

int boundary_cnt = 0;

void add_loc_entry(int loc, char *src, char *tgt) {
    struct l2a_hentry *s;

    HASH_FIND_INT(l2a_htab, &loc, s);  /* id already in the hash? */
    if (s == NULL) {
        s = (struct l2a_hentry *)malloc(sizeof *s);
        s->loc = loc;
        HASH_ADD_INT(l2a_htab, loc, s);  /* param2: name of key field */
    }
    strcpy(s->src, src);
    strcpy(s->tgt, tgt);
}

void add_bb_entry(char *addr, char *child, int loc) {
    struct bb_hentry *s;

    HASH_FIND_STR(bb_htab, addr, s);  /* id already in the hash? */
    if (s == NULL) {
        s = (struct bb_hentry *)malloc(sizeof *s);
        strcpy(s->addr, addr);
        s->lchild[0] = '\0';
        s->lloc = 0;
        HASH_ADD_STR(bb_htab, addr, s);  /* param2: name of key field */
    }
    if (!s->lloc) {
        strcpy(s->lchild, child);
        s->lloc = loc;
        s->rloc = 0;
    } else {
        strcpy(s->rchild, child);
        s->rloc = loc;        
    }
}

// void *ck_alloc(int size) {

//     void *mem = malloc(size);
    
//     if (!mem) {
//         printf("E: malloc failed\n");
//         exit(-1);
//     }

//     return memset(mem, 0, size);
// }

void add_i2p_entry(char *addr, struct pp_lentry *pos_pair) {

    struct i2p_hentry *s;
    struct pp_lentry *first;

    HASH_FIND_STR(i2p_htab, addr, s);  /* id already in the hash? */
    if (s == NULL) {
        s = (struct i2p_hentry *)malloc(sizeof *s);
        strcpy(s->addr, addr);
        HASH_ADD_STR(i2p_htab, addr, s);  /* param2: name of key field */
    }
    
    s->pos_pair = pos_pair;

    // first = container_of(s->pos_pair->list.next, struct pp_lentry, list);
    // printf("-- add %s, %d to htab\n", s->addr, first->start);

}

void delete_i2p_htab(struct i2p_hentry *htab) {
    
    struct i2p_hentry *current, *tmp;
    struct pp_lentry *pp;
    struct list_head *pos, *next;

    HASH_ITER(hh, htab, current, tmp) {
        
        HASH_DEL(htab, current);  /* delete; users advances to next */

        list_for_each_safe(pos, next, &current->pos_pair->list) {

            pp = container_of(pos, struct pp_lentry, list);
            list_del_init(pos); 
            
            ck_free(pp); 
        }
        free(current);            /* optional- if you want to free  */
    }
}

void delete_loc_htab(struct l2a_hentry *htab) {
  struct l2a_hentry *current, *tmp;

  HASH_ITER(hh, htab, current, tmp) {
    HASH_DEL(htab, current);  /* delete; users advances to next */
    free(current);            /* optional- if you want to free  */
  }
}

void delete_bb_htab(struct bb_hentry *htab) {
    struct bb_hentry *current, *tmp;

    HASH_ITER(hh, htab, current, tmp) {
        HASH_DEL(htab, current);  /* delete; users advances to next */
        free(current);            /* optional- if you want to free  */
    }
}

void print_ins2pos() {

    struct i2p_hentry *s;
    struct pp_lentry *pp;
    struct list_head *pos, *head;

    // printf("---%d\n", HASH_COUNT(i2p_htab));
    for (s = i2p_htab; s != NULL; s = (struct i2p_hentry*)(s->hh.next)) {
        
    // printf("---\n");
        printf("%s: [", s->addr);
        head = &(s->pos_pair->list); 
        list_for_each(pos, head) {
            pp = container_of(pos, struct pp_lentry, list);
            printf(" (%d, %d)", pp->start, pp->end);

        }
        printf("]\n");
    }
}

void print_pp_list(struct list_head *head) {

    struct pp_lentry *pp;
    struct list_head *pos;

    list_for_each(pos, head) {
        pp = container_of(pos, struct pp_lentry, list);
        printf(" (%d, %d)", pp->start, pp->end);        
    }

    printf("\n");
}

struct list_head *get_list_tail(struct list_head *cur) {

    while (cur != NULL && cur->next != NULL) 
        cur = cur->next; 
    return cur; 
}

void read_bitmap(u8* fname, u8* tname) {

//   printf("-\n");
  s32 fd = open(fname, O_RDONLY);

  if (fd < 0) PFATAL("Unable to open '%s'", fname);

//   printf("--\n");
  ck_read(fd, virgin_bits, MAP_SIZE, fname);
  close(fd);

//   printf("---\n");
  fd = open(tname, O_RDONLY);
  ck_read(fd, trace_bits, MAP_SIZE, tname);
  close(fd);

  printf("I: Done reading bitmap files.\n");
}

struct list_head *partition(struct list_head *head, struct list_head *end, 
                       struct list_head **new_head, struct list_head **new_end,
                            int container_type) {
    
    int cur_data, pivot_data;
    struct list_head *pivot = end; 
    struct list_head *prev = NULL, *cur = head, *tail = pivot; 

    switch (container_type) {

        case TYPE_POS_PAIR:
            pivot_data = data_of(pivot, struct pp_lentry, list, start);
            break;

        case TYPE_BOUNDRAY:
            pivot_data = data_of(pivot, struct boundary_entry, list, hit_cnt);
    }
    
    // printf("-- pivot = %d\n", pivot_data);

    // During partition, both the head and end of the list might change 
    // which is updated in the new_head and new_end variables 
    while (cur != pivot) 
    { 
        switch (container_type) {

        case TYPE_POS_PAIR:
            cur_data = data_of(cur, struct pp_lentry, list, start);
            break;

        case TYPE_BOUNDRAY:
            cur_data = data_of(cur, struct boundary_entry, list, hit_cnt);
        }  

        // printf("-- cur = %d\n", cur_data);

        if (cur_data < pivot_data) 
        { 
            // First node that has a value less than the pivot - becomes 
            // the new head 
            if ((*new_head) == NULL) (*new_head) = cur; 
  
            prev = cur;   
            cur = cur->next; 
        } else {// If cur node is greater than pivot 

            // Move cur node to next of tail, and change tail 
            if (prev) {
                prev->next = cur->next; 
                cur->next->prev = prev;
            }

            struct list_head *tmp = cur->next; 

            cur->next = NULL; 
            cur->prev = tail;
            
            tail->next = cur; 
            
            tail = cur; 
            cur = tmp; 
        } 
    } 
  
    // If the pivot data is the smallest element in the current list, 
    // pivot becomes the head 
    if ((*new_head) == NULL) (*new_head) = pivot; 
  
    // Update new_end to the current last node 
    (*new_end) = tail; 
  
    // Return the pivot node 
    return pivot; 
}

/* dont modify the contents of the list, modify the list pointer */
struct list_head *list_qsort(struct list_head *first, 
                             struct list_head *last, 
                             int container_type) {
    
    struct list_head *pivot, *new_first = NULL, *new_last = NULL;
    // base condition 
    if (!first || first == last) 
        return first; 
  
    // Partition the list, new_first and new_last will be updated 
    // by the partition function 
    
    pivot = partition(first, last, &new_first, &new_last, container_type); 
    
    // If pivot is the smallest element - no need to recur for 
    // the left part. 
    if (new_first != pivot) 
    { 
        // Set the node before the pivot node as NULL 
        struct list_head *tmp = new_first; 
        while (tmp->next != pivot) 
            tmp = tmp->next; 
        tmp->next = NULL; 
  
        // Recur for the list before pivot 
        new_first = list_qsort(new_first, tmp, container_type); 

        // Change next of last node of the left half to pivot 
        tmp = get_list_tail(new_first); 
        
        tmp->next =  pivot; 
        pivot->prev = tmp;
    } 
  
    // Recur for the list after the pivot element 
    pivot->next = list_qsort(pivot->next, new_last, container_type);     

    if (pivot->next) pivot->next->prev = pivot;

    return new_first; 
}

void sort_list(struct list_head *head, int container_type) {

    struct list_head *new_first, *new_last;

    head->next->prev = NULL;
    head->prev->next = NULL;

    new_first = list_qsort(head->next, head->prev, container_type);     
    new_last = get_list_tail(new_first);

    __list_add(head, new_last, new_first);

}

void merge_pairs(struct list_head *head) {

    int merged;
    struct list_head *pos, *next;
    struct pp_lentry *pp, *npp;

    do {
        merged = 0;
        list_for_each_safe(pos, next, head) {
            
            pp = container_of(pos, struct pp_lentry, list);
            npp = container_of(next, struct pp_lentry, list);

            if (next != head && pp->end + 1 == npp->start) {

                merged = 1;
                npp->start = pp->start;
                // list_del_init(pos);
                __list_del_entry(pos);
                // free(pp); 
            }
        }
    } while(merged);
}

void simplify_pp_list(struct list_head *head) {

    if (!list_empty(head)) {
        // printf("I: Sorting and merging postion pairs...\n");
        sort_list(head, TYPE_POS_PAIR);
        merge_pairs(head);
    } else {
        printf("E: No position pair at all.\n");
    }
}

cJSON * read_json_file(char *fname) {

    char *content;
    struct stat st;
    FILE *f;
    cJSON *json_content = NULL;
    
    if (lstat(fname, &st) || access(fname, R_OK)) {
        printf("E: Cannot read %s!\n", fname);
        return json_content;
    }

    content = (char *)ck_alloc(st.st_size + 1);
    
    f = fopen(fname, "rb");  
    fread(content, sizeof(char), st.st_size, f);

    fclose(f);  
    // free(fname);

    json_content = cJSON_Parse(content);
    ck_free(content);

    return json_content;
}

int read_ins2pos(char *fname) {
    printf("[+] parsing json file...");

    int i, ins2pos_size;
    char *addr, delm[] = ":";

    struct pp_lentry *pp, *pp_head;
    struct list_head *pl_head;

    cJSON *i2p, *pos_pair, *ins2pos = read_json_file(fname);

    if (!ins2pos) {
        printf("Error before: [%s] when reading %s\n",cJSON_GetErrorPtr(), fname);
        return -1;
    }
    printf("Done...");

    ins2pos_size = cJSON_GetArraySize(ins2pos);
    printf("%d entries found.\n", ins2pos_size);
    
    for(i = 0; i < ins2pos_size; i++) {
        // printf("-- %d", i);
        i2p = cJSON_GetArrayItem(ins2pos, i);
        // printf("-- %s", i2p->string);
        addr = strtok(i2p->string, delm); 

        pp_head = (struct pp_lentry *)ck_alloc(sizeof(struct pp_lentry));

        pp_head->start = -1; 
        pp_head->end = -1;
        pl_head = &pp_head->list;
        // printf("-- %s", addr);

        INIT_LIST_HEAD(pl_head);

        cJSON_ArrayForEach(pos_pair, i2p) {
            if (cJSON_GetArraySize(pos_pair) != 0) {
                int start = cJSON_GetArrayItem(pos_pair, 0)->valueint;
                int end = cJSON_GetArrayItem(pos_pair, 1)->valueint;
                // printf("%s: [%d, %d] \n", addr, start, end);
                
                pp = (struct pp_lentry *)ck_alloc(sizeof(struct pp_lentry));

                pp->start = start;
                pp->end = end;

                list_add(&pp->list, pl_head);
            }         
        }
        // printf("--list assembled");

        /* some addresses do not have corresponding input positions. Skip them. */
        if (!list_empty(pl_head)) {

            simplify_pp_list(pl_head);
            // print_pp_list(pl_head);
            
            add_i2p_entry(addr, pp_head); // pl->list->next is the first pos_pair.

        } else {
            // printf("--wrong");
            ck_free(pp_head);
        }
        // printf("\n");
    }

    cJSON_Delete(ins2pos);
    return 0;

}

int read_loc2addr(char *fname) {
    
    int i;
    cJSON *loc2addr = NULL;
    
    loc2addr = read_json_file(fname);

    if (!loc2addr) {
        printf("Error before: [%s]\n",cJSON_GetErrorPtr());
        return -1;
    }

    for(i = 0; i < cJSON_GetArraySize(loc2addr); i++) {
        cJSON *l2a = cJSON_GetArrayItem(loc2addr, i);

        cJSON *element;
        cJSON_ArrayForEach(element, l2a){
            if (cJSON_GetArraySize(element) == 2) {
                int loc = atoi(l2a->string);
                char *src = cJSON_GetStringValue(cJSON_GetArrayItem(element, 0));
                char *tgt = cJSON_GetStringValue(cJSON_GetArrayItem(element, 1));
                // printf("%d: %s -> %s\n", loc, src, tgt);
                
                add_loc_entry(loc, src, tgt);
                add_bb_entry(src, tgt, loc);

            }
        }
    }

    cJSON_Delete(loc2addr);
    printf("I: Done %s reading.\n", fname);
    
    return 0;
}

void find_boundary_edges(struct list_head *bl_head) {
    
	int i, loc, num = 0;	
    struct l2a_hentry *l;
    struct bb_hentry *b;
	struct boundary_entry *t;
    
    boundary_cnt = 0;
    
    for (i = 0; i < MAP_SIZE; i++) {
        
        if (trace_bits[i]) {

            num++;
            // printf("%d, %d\n", trace_bits[i], virgin_bits[i]);
            
            HASH_FIND_INT(l2a_htab, &i, l);
            if (l == NULL) {
                printf("E: Cannot find the edge loc %d in the cfg!\n", i);
                //exit(-1);
		        continue;
            }

            HASH_FIND_STR(bb_htab, l->src, b);
            if (b == NULL) {
                printf("E: Cannot find the basic block %s in the cfg!\n", l->src);
                exit(-1);
            }
                        
            if (i == b->lloc) loc = b->rloc; else loc = b->lloc;

            // printf("%d's sibling = %d hit %d times\n", i, loc, trace_bits[loc]);

            // boundary edge if the sibling edge has not been hit
            if (loc != 0 && virgin_bits[loc] == 0) {
                
                t = (struct boundary_entry *)malloc(sizeof *t);
                
                sprintf(t->tuple, "%s:%s", l->src, l->tgt);
                t->hit_cnt = virgin_bits[i];

                list_add(&t->list, bl_head); 
                boundary_cnt++;
            }
        }
    }
    
    printf("I: Found %d boundary edges in %d bits.\n", boundary_cnt, num);
}


// void swap_data(struct boundary_entry *i, struct boundary_entry *j) {

//     char *st;
//     int t;

//     sprintf(st, "%s", i->tuple);
//     strcpy(i->tuple, j->tuple);
//     strcpy(j->tuple, st);
//     free(st);

//     t = i->hit_cnt; 
//     i->hit_cnt = j->hit_cnt; 
//     j->hit_cnt = t;
// }

// struct boundary_entry *partition_old(struct boundary_entry *b_head, 
//                                  struct boundary_entry *b_tail) {
    
//     int x = b_tail->hit_cnt;
//     struct boundary_entry *i = b_head->prev, *j;

//     for (j = b_head; j != b_tail; j = j->next) {

//         // the hit_cnt before i are all less than x;
//         if (j->hit_cnt <= x) {

//             i = (i == NULL) ? b_head : i->next;
//             swap_data(i, j);
//         }
//     }
//     i = (i==NULL) ? b_head : i->next;
//     swap_data(i, b_tail);   // i->hit_cnt = x
    
//     return i;
// }

void sort_boundary_edges(struct list_head *head) {

    if (!list_empty(head)) {
        printf("I: Sorting boundary edges...\n");
        sort_list(head, TYPE_BOUNDRAY);
    } else {
        printf("E: No boundary edge at all.\n");
    }
}

int main(int argc, char* argv[]) {
    
    struct boundary_entry *boundary_head, *be;

    u8 *tname;
	struct list_head *bl_head, *bp;

    if (argc != 3) {
        printf("E: Need TWO file to proceed!\n");
        exit(-1);
    }
    printf("I: Reading %s ...\n", argv[1]);
    if (read_loc2addr(argv[1]) == -1) goto end;

    /* run the testcase and get trace_bits[] */
    tname = strcat(argv[2], "_t");
    printf("I: Reading fname = %s, tname = %s\n", argv[2], tname);
    read_bitmap(argv[2], tname);

    boundary_head = (struct boundary_entry *)malloc(sizeof *boundary_head);

	boundary_head->tuple = "__USELESS_LIST_HEAD__";
	boundary_head->hit_cnt = -1;
	
    bl_head = &boundary_head->list;
	INIT_LIST_HEAD(bl_head);
	
    find_boundary_edges(bl_head);
    sort_boundary_edges(bl_head);

    list_for_each(bp, bl_head) {
        be = container_of(bp, struct boundary_entry, list);
        printf("%s: %d\n", be->tuple, be->hit_cnt);

    }   


    // if (read_ins2pos(argv[1]) == -1) goto end;
    // print_ins2pos();

    // get_bit_pos();

    delete_loc_htab(l2a_htab);
    delete_bb_htab(bb_htab);
    // delete_i2p_htab(i2p_htab);


end:
    return 0;

}
