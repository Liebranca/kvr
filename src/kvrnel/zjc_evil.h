#ifndef __ZJC_EVIL_H__
#define __ZJC_EVIL_H__

#include <assert.h>

#ifdef __cplusplus
extern "C" {
#endif

//   ---     ---     ---     ---     ---

                                            // logflags. enables/disables certain logmesses
#define KVR_CALOM 0x0001                    // MEM
#define KVR_CALOF 0x0002                    // BIN
#define KVR_CALOK 0x0004                    // PROC
#define KVR_CALOE 0x0008                    // ERR
#define KVR_CALOR 0x0010                    // RAW
#define KVR_CALOH 0x0020                    // HASH
#define KVR_CALOP 0x0040                    // PESO
#define KVR_CALOS 0x0080                    // SIN

#ifndef KVR_DEBUG                           /* should be created at compile-time through -D*/
#define KVR_DEBUG 0x00                      /* this is just here as a safety net           */
#endif

//   ---     ---     ---     ---     ---
// some shorthands

#define CASSERT(boo, dummy)                 /* just a prettier name for static assert      */\
    _Static_assert(boo, dummy)

#define elif else if                        /* and prettier not having that space there    */

#define DONE     0                          /* execution went 'okay', apparently           */
#define FATAL -255                          /* also prettier than return -num or some      */
#define ERROR -254                          /* other int errcode; return ERROR is cleaner  */

#define byref(x) (&(x))                     /* to pass address-of with less clutter        */

//   ---     ---     ---     ---     ---

#define KCOM_MAXVAL 32                      /* maximum number of tokens per command        */
#define KCOM_VALW   96                      /* maximum token length                        */
#define KCOM_INLEN  KCOM_MAXVAL * KCOM_VALW

#define ARRSIZE(arr) sizeof(arr) / sizeof(*arr)

//   ---     ---     ---     ---     ---

typedef struct ZJC_COM {                    // a command

    int   order;                            // SGL || VERB_FIRST || SUBJ_FIRST
    int   ids[2];                           // indices for key tokens
    int   token_count;
    char  tokens[KCOM_MAXVAL][KCOM_VALW];

} COM;

typedef struct ZJC_RET {                    // a command's return

    int   state;                            // return state of the command
    void* block;                            // handle to datablock or NULL

} RET;

typedef void (*COMMAND)(RET*  r   );        // pointer to void func taking a RET* arg
typedef void (*STARK  )(void* buff);        // pointer to void func taking a void* arg
typedef void (*NIHIL  )(void      );        // pointer to void func taking no args

//   ---     ---     ---     ---     ---
/* utils */

char* __shpath (char* path                );// fixed ;> shortens path to /src/|... onwards

void __writoa  (int x, char* buff, int rdx);// wraps itoa
int  __wrstrcmp(char* s0, char* s1        );// wraps strcmp
int  qstoi     (char* buff                );// quick string to int (100% diarrhea)

//   ---     ---     ---     ---     ---

typedef struct ZJC_DANGERCALL {             // records info for calls that __can__ fail

    char location[256];                     // line that made the call
    int  depth;                             // is call parent or child of another
    int  state;                             // whether this call was succesful or not

} DANG;

//   ---     ---     ---     ---     ---

DANG* __geterrloc (const char* p,
                   const char* f, int l);   // get error location

void __printcalreg(int flush              );// prints __registered__ calls
void __popcalreg  (                       );// pop from call register
void __terminator (int errcode, char* info);// goes back in time to save john connor

void __calout     (char fam,              \
                   char* format, ...      );// prints to KVNSLOG

//   ---     ---     ---     ---     ---
// some bug-tracking macros

#include "calout.h"                 /* CALOUT boiler macros                        */

#define ERRLOC                              /* macro paste for error locations             */\
    __shpath(__FILE__), __func__, __LINE__

#define GETLOC __geterrloc(ERRLOC)          /* spam wherever something may go wrong        */

// this one is naughty slow, wrap it in checks if used in a loop
#define ERRCATCH(func, retx, errcode, info) { DANG* cal = GETLOC;                            \
                                                                                             \
    cal->state            = func;                                                            \
    if  ( (cal->state    == FATAL        )                                                   \
        | (cal->state    == ERROR        )                                                   \
        | (retx && (retx != cal->state)) )  { __terminator(errcode, info);                  }\
                                                                                             \
    retx = cal->state; __popcalreg();                                                       }

//   ---     ---     ---     ---     ---

#ifdef __cplusplus
}
#endif

#endif // __ZJC_EVIL_H__