#ifndef __ZJC_HASH_H__
#define __ZJC_HASH_H__

#include "zjc_stack.h"

#ifdef __cplusplus
extern"C" {

#endif

//   ---     ---     ---     ---     ---
// #:0x0;>


#define ZJC_HASH_STKLEN 8

typedef struct ZJC_HASHSLOT {


// addresses to data
void* nodes[ZJC_HASH_STKLEN];
STK stack;                  // available indices

} HSLOT;

//   ---     ---     ---     ---     ---
// #:0x1;>


typedef struct ZJC_HASHTABLE {


MEM m;                      // mem header

uint nslots;                // number of
                            // slots;table bounds
uint nitems;                // number of inserted
                            // items

} HASH;

//   ---     ---     ---     ---     ---
// #:0x2;>



// build a new hash table
HASH* MKHASH(uint mag,char* name);

void DLHASH(void* buff);    // free a hash table
void CLHASH(HASH* h);       // wipe a hash clean

void STHASH(void* data);    // insert data into
                            // hash @key

size_t szhash(HASH* h);     // get capacity

// retrieve data @key
void GTHASH(void** to,int pop);

int INHASH(HASH* h,ID*);    // return key is in
                            // table

// convenience func, calls INHASH without a token
int KINHASH(HASH* h,char* key);

int NK4HSLOT(void);         // pop slot idex from
                            // subarray

void FLHASH(int flags);     // sets flags, used
                            // for internals

HASH* GWHASH(HASH* h);

//   ---     ---     ---     ---     ---
// #:0x3;>


#if KVR_DEBUG & KVR_CALLOH /* catch full bucket */
#define HASHSET(h, data) {\
  FLHASH(1); /* set mode to insertion */ \
  ID* lkp=(ID*) data;\
  int key_in_hash=INHASH(h,lkp);int retx=0;\
\
  if(!key_in_hash) {\
    ERRCATCH(NK4HSLOT(),retx,72,lkp->key);\
    h->nitems++;\
\
  };if(!retx) {STHASH(data);};\
}

#else
#define HASHSET(h, data) {\
  FLHASH(1);ID* lkp=(ID*) data;\
  int key_in_hash=INHASH(h,lkp);int retx=0;\
  if(!key_in_hash) {\
    retx=NK4HSLOT();h->nitems++;\
\
  };if(!retx) {STHASH(data);};\
}

#endif

//   ---     ---     ---     ---     ---
// #:0x4;>


#if KVR_DEBUG & KVR_CALLOH /* catch key error */

#define HASHGET(h, lkp, to, pop) {\
  FLHASH(0); /* set mode to fetch */\
  int key_in_hash=INHASH(h,lkp);\
\
  if(!key_in_hash) {\
    ERRCATCH(ERROR,key_in_hash,73,lkp->key);\
    to=NULL;\
\
  } else {\
    GTHASH((void**) &to,pop);h->nitems-=pop;\
  }\
}

#else
#define HASHGET(h, lkp, to, pop) {\
  FLHASH(0);\
  int key_in_hash=INHASH(h,lkp);\
\
  if(key_in_hash) {\
    GTHASH((void**) &to,pop);h->nitems-=pop;\
\
  } else {to=NULL;}\
}

#endif

//   ---     ---     ---     ---     ---
// #:0x5;>


#if KVR_DEBUG & KVR_CALLOH /* catch key error */

#define STR_HASHGET(h, key, to, pop) {\
  FLHASH(0);ID lkp=IDNEW( "LKP_",key);lkp.y=-2;\
  int key_in_hash=INHASH(h,&lkp);\
\
  if(!key_in_hash) {\
    ERRCATCH(ERROR,key_in_hash,73,key);\
    to=NULL;\
\
  } else {\
    GTHASH((void**) &to,pop);h->nitems-=pop;\
  }\
}

#else
#define STR_HASHGET(h, key, to, pop) {\
  FLHASH(0);ID lkp=IDNEW( "LKP_",key);lkp.y=-2;\
  int key_in_hash=INHASH(h,&lkp);\
\
  if(key_in_hash) {\
    GTHASH((void**) &to,pop);\
    h->nitems-=pop;\
\
  } else {to=NULL;}\
}

#endif

//   ---     ---     ---     ---     ---
// #:0x6;>


#ifdef __cplusplus
}
#endif

#endif // __ZJC_HASH_H__