#ifndef __ZJC_HASH_H__
#define __ZJC_HASH_H__

#include "zjc_stack.h"

#ifdef __cplusplus
extern "C" {
#endif

//   ---     ---     ---     ---     ---

typedef struct ZJC_HASHNODE {

    void* data;                             // ptr to block, typecasted by table get
    char* key;                              // key that generated the table index

    uint  idex;                             // idex into subarray

    struct ZJC_HASHNODE* next;              // jump-to

} HNODE;

typedef struct ZJC_HASHSLOT {

    STK*   stack;                           // available indices

    HNODE* head;                            // ptr to first block
    HNODE* nodes;                           // array of key:data pairs

} HSLOT;

//   ---     ---     ---     ---     ---

typedef struct ZJC_HASHTABLE {

    MEM    m;                               // mem header

    uint   nslots;                          // number of slots; table bounds
    uint   nitems;                          // number of inserted items
    uint   jmp;                             // space between table entries, in bytes

} HASH;

//   ---     ---     ---     ---     ---

void  MKHASH(HASH* h, uint mag, char* id);  // build a new hash table
void  DLHASH(void* buff);                   // free a hash table

int   STHASH(void* data);                   // insert key:data into hash
void* GTHASH(int pop);                      // remove key:data from hash, return data

int   INHASH(HASH* h, char* key);           // return key is in table
int   NK4HSLOT();                           // pop slot idex from subarray

//   ---     ---     ---     ---     ---

#define HASHSET(h, key, data)               { int key_in_hash=INHASH(h, key); int retx=0;    \
    if(!key_in_hash) { ERRCATCH(NK4HSLOT(), retx, 72, key); h->nitems++; };                  \
    if(!retx       ) { STHASH(data);                                     };                 }

#define HASHGET(h, key, to, type, pop)      { int key_in_hash=1;                             \
    ERRCATCH(INHASH(h, key), key_in_hash, 73, key);                                          \
    if(key_in_hash) { to=(type*) GTHASH(pop); h->nitems-=pop; } else { to=NULL; };          }

//   ---     ---     ---     ---     ---

#ifdef __cplusplus
}
#endif

#endif // __ZJC_HASH_H__