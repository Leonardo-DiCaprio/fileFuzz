#include "read-json.h"

/*************************
 * List-based quick sort *
 *************************/
static struct list_head *get_list_tail(struct list_head *cur) {

    while (cur != NULL && cur->next != NULL) 
        cur = cur->next; 
    return cur; 
}

static struct list_head *partition(struct list_head *head, 
                                    struct list_head *end, 
                                    struct list_head **new_head, 
                                    struct list_head **new_end,
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

    /* During partition, both the head and end of the list might change 
       which is updated in the new_head and new_end variables 
     */
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
            /* First node that has a value less than the pivot - becomes 
               the new head 
             */
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
  
    /* If the pivot data is the smallest element in the current list, 
       pivot becomes the head 
     */
    if ((*new_head) == NULL) (*new_head) = pivot; 
  
    // Update new_end to the current last node 
    (*new_end) = tail; 
  
    // Return the pivot node 
    return pivot; 
}

/* dont modify the contents of the list, modify the list pointer */
static struct list_head *list_qsort(struct list_head *first, 
                                    struct list_head *last, 
                                    int container_type) {
    
    struct list_head *pivot, *new_first = NULL, *new_last = NULL;
    // base condition 
    if (!first || first == last) 
        return first; 
  
    /* Partition the list, new_first and new_last will be updated 
       by the partition function 
     */
    pivot = partition(first, last, &new_first, &new_last, container_type); 
    
    /* If pivot is the smallest element - no need to recur for 
       the left part. 
     */
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

static void sort_list(struct list_head *head, int container_type) {

    struct list_head *new_first, *new_last;

    head->next->prev = NULL;
    head->prev->next = NULL;

    new_first = list_qsort(head->next, head->prev, container_type);     
    new_last = get_list_tail(new_first);

    __list_add(head, new_last, new_first);

}

/**********************
 * Read JSON mappings *
 **********************/
void add_loc_entry(int loc, char *src, char *tgt) {

    struct l2a_hentry *s;
    struct cmp_addr_lentry *caddr_head;
    struct list_head *cl_head;

    HASH_FIND_INT(l2a_htab, &loc, s);  
    if (s == NULL) {
        s = (struct l2a_hentry *)malloc(sizeof *s);
        s->loc = loc;
        HASH_ADD_INT(l2a_htab, loc, s);  /* param2: name of key field */
    }
    strcpy(s->src, src);
    strcpy(s->tgt, tgt);

    caddr_head = (struct cmp_addr_lentry *)ck_alloc(sizeof(*caddr_head));

    sprintf(caddr_head->addr, "%s", LIST_HOLDER); 
    cl_head = &caddr_head->list;

    INIT_LIST_HEAD(cl_head);
    s->caddr_list = caddr_head;

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

void add_i2p_entry(char *addr, struct pp_lentry *pos_pair) {

    struct i2p_hentry *s;
    // struct pp_lentry *first;

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

void add_i2k_entry(char *addr, size_t S, char (*cmp_addr)[S]) {

    struct i2k_hash_entry *s;
    HASH_FIND_STR(i2k_hash_tab, addr, s);  /* id already in the hash? */
    if (s == NULL) {
        s = (struct i2k_hash_entry *)malloc(sizeof (struct i2k_hash_entry));
        strcpy(s->addr, addr);
        HASH_ADD_STR(i2k_hash_tab, addr, s);  /* param2: name of key field */
    }
    int add_i2k_entry_i = 0;
    while(strcmp(cmp_addr[add_i2k_entry_i], "end")!=0){
        strcpy(s->keyoff_word[add_i2k_entry_i], cmp_addr[add_i2k_entry_i]);
        add_i2k_entry_i++;
    }
    strcpy(s->keyoff_word[add_i2k_entry_i], "end");
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

void delete_i2k_htab(struct i2k_hash_entry *htab) {
    
    struct i2k_hash_entry *current, *tmp;

    HASH_ITER(hh, htab, current, tmp) {
        
        HASH_DEL(htab, current);  /* delete; users advances to next */

        free(current);            /* optional- if you want to free  */
    }
    htab = NULL;
}

cJSON *read_json_file(u8 *fname) {

    u8 *content;
    struct stat st;
    FILE *f;
    cJSON *json_content = NULL;
    u32 r_len = 0;
    
    if (lstat(fname, &st) || access(fname, R_OK)) {
        // ACTF("Cannot read %s!\n", fname);
        return json_content;
    }

    content = (char *)ck_alloc(st.st_size + 1);
    
    f = fopen(fname, "rb");  
    if (!f) {
      ACTF("Unable to open '%s'", fname);
      goto ret;
    }
    
    r_len = fread(content, sizeof(char), st.st_size, f);
    if (r_len != st.st_size) {
      ACTF("Incomplete reading '%s'", fname);
      goto ret;
    } 

    json_content = cJSON_Parse(content);
    
ret:
    ck_free(content);
    fclose(f);
    return json_content;
}

static void merge_pairs(struct list_head *head) {

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

int read_ins2pos(char *fname) {
    //printf("[+] parsing json file...");

    int i, ins2pos_size;
    char *addr, delm[] = ":";

    struct pp_lentry *pp, *pp_head;
    struct list_head *pl_head;

    cJSON *i2p, *pos_pair, *ins2pos = read_json_file(fname);

    if (!ins2pos) {
        //printf("Error before: [%s] when reading %s\n",cJSON_GetErrorPtr(), fname);
        return -1;
    }
    //printf("Done...");

    ins2pos_size = cJSON_GetArraySize(ins2pos);
    //printf("%d entries found.\n", ins2pos_size);
    
    for(i = 0; i < ins2pos_size; i++) {
        // printf("-- %d", i);
        i2p = cJSON_GetArrayItem(ins2pos, i);
        
        // printf("-- %s --", i2p->string);
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
                // if (strcmp(addr, "0x809154c") == 0){ 
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

            //simplify_pp_list(pl_head);
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

void find_ins2pos_2(char* cmp_addr, int* ret, size_t size) {

    int i=0;
    struct i2p_hentry *s;
    struct pp_lentry *pp;
    struct list_head *pos, *head;

    // printf("---%d\n", HASH_COUNT(i2p_htab));
    for (s = i2p_htab; s != NULL; s = (struct i2p_hentry*)(s->hh.next)) {
        
    // printf("---\n");
        if(strcmp(s->addr, cmp_addr)==0 && i<size-2){
            //printf("%s: [", s->addr);
            head = &(s->pos_pair->list); 
            list_for_each(pos, head) {
                pp = container_of(pos, struct pp_lentry, list);
                //printf(" (%d, %d)", pp->start, pp->end);        
                ret[i++]=pp->start;
                ret[i++]=pp->end;
            }
            //printf(" ]\n"); 
        }
    }
    ret[i]=0;
}

void read_loc2cmp(u8* bname) {
    
    int i, loc;
    u8 *fname, *delm = "L";

    struct cmp_addr_lentry *ca;
    struct list_head *cal_head;
    struct l2a_hentry *l2a;
    cJSON *l2c, *cmp_addr, *loc2cmp;

    ACTF("Reading the CMP addresses of the target binary...");

    fname = alloc_printf("%s_loc2cmp.json", bname);

    loc2cmp = read_json_file(fname);

    if (!loc2cmp) 
        FATAL("Error before: [%s]", cJSON_GetErrorPtr());

    for(i = 0; i < cJSON_GetArraySize(loc2cmp); i++) {

        l2c = cJSON_GetArrayItem(loc2cmp, i);
        loc = atoi(l2c->string); 

        HASH_FIND_INT(l2a_htab, &loc, l2a); 
        if (l2a == NULL) {
            printf("Cannot find %d in l2a_htab\n", loc);
            continue;
        }

        cal_head = &(l2a->caddr_list->list);

        cJSON_ArrayForEach(cmp_addr, l2c) {
            char *addr = cJSON_GetStringValue(cmp_addr);
            
            ca = (struct cmp_addr_lentry *)ck_alloc(sizeof(*ca));
            sprintf(ca->addr, "%s", strtok(addr, delm));

            list_add(&ca->list, cal_head);    
        }
    }

    cJSON_Delete(loc2cmp);
    ACTF("Done %s reading.\n", fname);
    ck_free(fname);

}

void read_loc2addr(u8* bname) {
    
    int i;
    u8* fname;
    cJSON *loc2addr = NULL;

    
    ACTF("Reading the edge hashes of the target binary...");

    fname = alloc_printf("%s_loc2tuple.json", bname);

    loc2addr = read_json_file(fname);

    if (!loc2addr) 
        FATAL("Error before: [%s]", cJSON_GetErrorPtr());

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
    ACTF("Done %s reading.\n", fname);
    ck_free(fname);

}

void find_loc2cmp(int bzdqsmmz, size_t M, size_t N, char ret[M][N]) {
    struct l2a_hentry *s;
    struct cmp_addr_lentry *ca;
    struct list_head *pos, *head;
    
    int i = 0;
    // printf("---%d\n", HASH_COUNT(i2p_htab));
    for (s = l2a_htab; s != NULL; s = (struct l2a_hentry*)(s->hh.next)) {
        if (s->loc == bzdqsmmz){
            // printf("%d: [", s->loc);
            head = &(s->caddr_list->list); 
            
            list_for_each(pos, head) {
                //ret[i] = (char *) malloc(sizeof(char)*11);
                ca = container_of(pos, struct cmp_addr_lentry, list);
                strcpy(ret[i], ca->addr);
                i ++;   
            }
            // printf(" ]\n");
        }
    }
    strcpy(ret[i], "end");
}

void read_ins2keyoff(char *fname) {
    int i, ins2keyoff_size;
    char *addr, delm[] = ":";
    cJSON *i2k, *keyoff, *ins2keyoff = read_json_file(fname);

    if (ins2keyoff && i2k_hash_tab==NULL) {
        
        ins2keyoff_size = cJSON_GetArraySize(ins2keyoff);
        //printf("%d entries found.\n", ins2pos_size);
        
        for(i = 0; i < ins2keyoff_size; i++) {
            //printf("-- %d", i);
            char keyoff_word[64][128];
            int j=0;
            memset(keyoff_word, '0', sizeof(char)*64*64);
            i2k = cJSON_GetArrayItem(ins2keyoff, i);
            
            //printf("-- %s\n", i2k->string);
            addr = strtok(i2k->string, delm);

            cJSON_ArrayForEach(keyoff, i2k) {           
                size_t value_size = 0;
                if(i2k->child){
                    cJSON *child = NULL;   
                    child = i2k->child;

                    while(child != NULL)
                    {
                        value_size++;
                        child = child->next;
                    }
                }

                if (value_size != 0) {
                    if(strlen(keyoff->valuestring) < (128-1) && j < 63){
                        strcpy(keyoff_word[j], keyoff->valuestring);
                        j++;
                    }
                }         
            }
            strcpy(keyoff_word[j], "end");
            // some addresses do not have corresponding input positions. Skip them. 
            if (strcmp(keyoff_word[0], "end")!=0) {
                add_i2k_entry(addr, 128, keyoff_word); // pl->list->next is the first pos_pair.
            }  
            // printf("\n");
        }
        
        cJSON_Delete(ins2keyoff);
    }

}


int find_cmp_addr_in_ins2keyoff(size_t S, char (*cmp_addr)[S], size_t N, size_t M, char ret[N][M]) {

    struct i2k_hash_entry *s;
    int i=0, j=0, k=0, return_flag=0;
    while(strcmp(cmp_addr[j], "end")!=0){
        HASH_FIND_STR(i2k_hash_tab, cmp_addr[j], s); 
        if (s != NULL) {
            k=0;
            while(strcmp(s->keyoff_word[k], "end")!=0) {
                if(i<N && strlen(s->keyoff_word[k])<M){
                    strcpy(ret[i], s->keyoff_word[k]); 
                    i++;
                    k++; 
                    return_flag = 1;
                }
            }
        }
        j++;
    }
    strcpy(ret[i], "end");
    return return_flag;
}

int find_loc(u8* out_buf_tmp, char* words, int length){
    int i=0, high=0, j=0;
    int word_len = strlen(words)>0?strlen(words):1;
    u8 word_int[word_len];
    if (word_len%2 == 0){
        for(i=0;i<word_len && j<word_len/2;){
            if(words[i]>='0'&&words[i]<='9') high = words[i] - '0';
            else high = words[i] - 'W';
            high = high << 4;
            if(words[i+1]>='0'&&words[i+1]<='9') word_int[j] = (words[i+1] - '0') + high;
            else word_int[j] = (words[i+1] - 'W') + high;
            j++;
            i+=2;
        }
    }
    else
        return -length;
    for (i=0;i < length-word_len/2; ++i){
        if (out_buf_tmp[i] == word_int[0]){
            for(j=0;j<word_len/2;j++){
                if (out_buf_tmp[i+j] != word_int[j])
                    break;
            }
            if (j>=(word_len/2 - 1))
                return i;
        }
    }
    return -length;
}
// find_cmp_addr_in_ins2keyoff + find_mutate_loc_in_outbuf = find_cmp_addr_in_outbuf

int find_cmp_addr_in_outbuf(u8* out_buf, int len, size_t S, char (*cmp_addr)[S], size_t N, size_t M, u8 ret[N][M]) {

    struct i2k_hash_entry *s;
    int i=0, j=0, k=0, return_count=0;
    char ins2keyoff_word_tmp[256] = {0};
    char *end = "end";
    while(strcmp(cmp_addr[j], end)!=0){
        HASH_FIND_STR(i2k_hash_tab, cmp_addr[j], s); 
        if (s != NULL) {
            k=0;
            while(strcmp(s->keyoff_word[k], end)!=0) {
                strcpy(ins2keyoff_word_tmp, s->keyoff_word[k]); 
                char* tmp;
                int left, right, loc_l= -len, loc_r= -len;
                char* word; 
                if(!strstr(ins2keyoff_word_tmp, "|")){
                    strtok(ins2keyoff_word_tmp, ":");
                    strtok(NULL, ":");
                    tmp = strtok(NULL, ":");
                    left = atoi(strtok(NULL, ":"));
                    right = atoi(strtok(NULL, ":"));
                    strtok(tmp, ",");
                    tmp = strtok(NULL, ",");
                    //printf("%s\n", strtok(NULL, ","));
                    word = strtok(tmp, ")");
                    loc_l = find_loc(out_buf, word, len);
                    loc_r = loc_l;
                }
                else{
                    strtok(ins2keyoff_word_tmp, ",");
                    tmp = strtok(NULL, "!");
                    word = strtok(tmp, ")");
                    loc_l = find_loc(out_buf, word, len);
                    tmp = strtok(NULL, "!");
                    tmp = tmp + 1;
                    left = atoi(strtok(tmp, "|"));
                    tmp = strtok(NULL, "!");
                    strtok(tmp, ",");
                    tmp = strtok(NULL, "!");
                    word = strtok(tmp, ")");
                    loc_r = find_loc(out_buf, word, len);
                    tmp = strtok(NULL, "!");
                    tmp = tmp + 1;
                    right = atoi(strtok(tmp, "!"));
                }
                if(loc_l+left>0 && loc_r+right>0 && loc_l+left<len && loc_r+right<len){
                    memset(ret+return_count, 0, M);
                    int zero_flag = 0;
                    for(i=0; i<loc_r+right-loc_l-left+1; i++){
                        zero_flag += out_buf[i+loc_l+left];
                        ret[return_count][i] = out_buf[i+loc_l+left];
                    }
                    ret[return_count][i] = 255;
                    ret[return_count][i+1] = 255;
                    ret[return_count][i+2] = 255;
                    ret[return_count][i+3] = 255;
                    if(likely(zero_flag>0))    
                        return_count++; 
                    if(return_count >= N)
                        return return_count;                   
                }                 
                k++; 
            }
        }
        j++;
    }
    //memcpy(ret[i], end, sizeof(char)*strlen(end));
    return return_count;
}

int find_mutate_loc_in_outbuf(u8* out_buf, int len, size_t N, char (*keyoff_word)[N], size_t M, int ret[M]){
    int k=0, a=0;
    char* endstr = "end";
    int sum_ret = 0;
    while (strcmp(keyoff_word[k], endstr)!=0){          
        char* tmp;
        int left, right, loc_l= -len, loc_r= -len;
        char* word; 
        if(!strstr(keyoff_word[k], "|")){
            strtok_r(keyoff_word[k], ",", &tmp);
            word = strtok_r(tmp, ")", &tmp);
            loc_l = find_loc(out_buf, word, len);
            loc_r = loc_l;
            tmp = tmp + 1;
            left = atoi(strtok_r(tmp, ":", &tmp));
            right = atoi(tmp);
        }
        else{
            strtok_r(keyoff_word[k], ",", &tmp);
            word = strtok_r(tmp, ")", &tmp);
            loc_l = find_loc(out_buf, word, len);
            tmp = tmp + 1;
            left = atoi(strtok_r(tmp, "|", &tmp));
            
            strtok_r(tmp, ",", &tmp);
            word = strtok_r(tmp, ")", &tmp);
            loc_r = find_loc(out_buf, word, len);
            tmp = tmp + 1;
            right = atoi(tmp);
        }
        if(loc_l+left>0 && loc_r+right>0 && loc_l+left<len && loc_r+right<len && a<M-1 && loc_l+left<loc_r+right){
            int count_ret;
            for(count_ret=loc_l+left;count_ret<=loc_r+right;count_ret++){
                ret[count_ret]++;
                sum_ret ++;
            }
            a += 2;
        }
        k ++;
    }
    return sum_ret;
}
