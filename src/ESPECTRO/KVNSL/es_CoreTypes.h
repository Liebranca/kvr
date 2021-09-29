#ifndef __ES_CORETYPES_H__
#define __ES_CORETYPES_H__

#include "KVRNEL/zjc_evil.h"
#include "KVRNEL/zjc_CommonTypes.h"
#include "KVRNEL/TYPES/zjc_id.h"
#include "KVRNEL/lymath.h"

#ifdef __cplusplus
extern "C" {
#endif

//   ---     ---     ---     ---     ---

typedef struct PESO_OPERATION {             // basic hashable fun ptr with descriptor

    ID    id;                               // key/lookup token
    ustr8 data;                             // some bytes describing the call
    STARK fun;                              // ptr to the actual operation

} PEOP;

//   ---     ---     ---     ---     ---

void  NTPEOP (uint  idex  ,
              char* opcode,
              uint  data  ,
              STARK fun   );                // insert fun ptr into op array

PEOP* GTPENOP(void        );                // return null operator

void  GTOP   (PEOP** dst  ,
              char*  key  );                // fetch op matching key

//   ---     ---     ---     ---     ---

typedef struct PESO_CHARSPRITE {            // animated char sequence

    char* buff;                             // characters to draw each frame
    char  frame[16];                        // draw buffer
    float current;                          // current frame
    uint  length;                           // length of buff
    ustr8 co;                               // position in chargrid

} CHRSPRT;

//   ---     ---     ---     ---     ---

CHRSPRT MKCHRSPRT(char* buff,
                  uint  co  );              // make char sprite from a buffer

void    PLCHRSPRT(CHRSPRT* sprite,
                  float dirn     );         // get next frame for animation

//   ---     ---     ---     ---     ---

#define ES_SPRITE_SLOTS 8

typedef struct PESO_CHARBUFF {              // basic constr container

    float   t[4];                           // transform; scale & startof in screen coords
    uint    chrsz;                          // base charsize multiplier

    CHRSPRT sprites[ES_SPRITE_SLOTS];       // fixed-size array for animated char sequences

} CHRBUFF;

//   ---     ---     ---     ---     ---

void NTKVR (uint chrsz  );                  // init console
void DLKVR (            );                  // del & restore

int  kbhit (void        );                  // get n chars waiting on input buff
void getch (void        );                  // read input buff

//   ---     ---     ---     ---     ---

float* GTKVRCURT(void);                     // get cursor transform
float* GTKVRCHRT(void);                     // get next char transform
ustr8* GTKVRCHRD(void);                     // get next char data

//   ---     ---     ---     ---     ---

#ifdef __cplusplus
}
#endif

#endif // __ES_CORETYPES_H__