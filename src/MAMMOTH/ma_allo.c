#include "ma_allo.h"

#include <string.h>
#include <stdio.h>
#include <unistd.h>

//   ---     ---     ---     ---     ---

// not portable, just true for my machine
#define PAGE_SZ 4096
#define FMS_SZ  16

//   ---     ---     ---     ---     ---

// header of sub-block within allocation
typedef struct MAMM_BLK_HED
{

  uint64_t id;              // numerical identificator

  uint16_t fl;              // usage flags
  uint16_t cr;              // current position
  uint16_t us;              // used space
  uint16_t sz;              // total space

} FHED;

// allocation manager
typedef struct MAMM_FAST_ALLOC
{

  size_t sz;                // sum(sz) of all blocks
  size_t avail;             // sum(sz-us) of all blocks
  size_t used;              // sum(us) of all blocks

  void*  head;              // first block in chain
  void*  tail;              // last block

  FHED   slots[16][FMS_SZ]; // array of sub-block
                            // descriptors

} FALL; static FALL mfa = {0};

//   ---     ---     ---     ---     ---

void ntallo(void)
{

  for(int x=0;x<QMS_SZ;x++)
  {
    mfa.slots[x].id = FREE_BLOCK;
    mfa.slots[x].fl = 0;
    mfa.slots[x].cr = 0;
    mfa.slots[x].us = 0;
    mfa.slots[x].sz = 0;

  };

};

void dlallo(void)
{

  sbrk(mfa.sz);

};

//   ---     ---     ---     ---     ---

FHED* hshb(void* ptr)
{

  uintptr_t addr = (uintptr_t) ptr;

  int       idy  = ((addr&0x00F0)>>4);
  int       idx  = ((addr&0xFF00)>>8)&(QMS_SZ-1);

  FHED*     slot = mfa.slots[idy][idx];

  if(

  return &(mfa.slots[idy][idx]);

};

//   ---     ---     ---     ---     ---

void* fmem(int sz)
{

  int page_cnt = sz/PAGE_SZ;
  if(!page_cnt) { page_cnt++; }


  printf("BRK %i\n", sbrk(0));
  return;

};