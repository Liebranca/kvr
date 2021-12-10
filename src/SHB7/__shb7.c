//**************************
// __SHB7                  *
// launches the kvr shell  *
//                         *
// LIBRE SOFTWARE          *
// Licenced under GNU GPL3 *
// be a bro and inherit    *
//                         *
// CONTRIBUTORS            *
// lyeb,                   *
//**************************



#include "__shb7.h"

#include "KVRNEL/MEM/sh_map.h"

#include "CHASM/chMNG.h"

#include "SIN/__sin.h"
#include "SIN/sin_texture.h"
#include "SIN/REND/sin_canvas.h"

#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <unistd.h>
#include <signal.h>

#include <sys/wait.h>
#include <limits.h>
#include <errno.h>

//   ---     ---     ---     ---     ---
// #:0x0;>

static uint* shbout=NULL;

static uint shbout_mx=0;
static uint shbout_my=0;

static uint shbout_x=0;
static uint shbout_y=0;

static int shbout_flg=0x00;

//   ---     ---     ---     ---     ---

char* pshout(
  uchar* str,
  uchar col_id,
  char flg) {

  uint ox=shbout_x;
  uint oy=shbout_y;

  uint pos=shbout_x+(shbout_y*shbout_mx);

  do {

    flg|=(
     (*str<0x20 && shbout[2]<=0x20)
     *(0x04*(!(flg&0x02)))

    );if(flg&0x04 || !*str) {
      break;

    };

    if(*str!=0x08) {
      shbout[pos]=(*str)|(col_id<<8);
      pos++;shbout_x++;

    } else {
      shbout[pos]=0x0020;
      pos--;shbout_x-=1*(pos>=2);

      if(shbout_x>=shbout_mx) {
        shbout_x=shbout_mx-1;
        shbout_y--;

        shbout_y*=0+(shbout_y<shbout_my);
        pos=shbout_x+(shbout_y*shbout_mx);

      };continue;
    };

    if(shbout_x>=shbout_mx
    || *str==0x0A) {
      shbout_x=0;
      shbout_y++;

      pos=shbout_x+(shbout_y*shbout_mx);

    };

    if(shbout_y>=shbout_my) {
      shbout_x=0;
      shbout_y=0;
      pos=0;break;

    };

  } while(*str++);

  shbout_flg|=0x01;
  if(flg&0x01) {
    shbout_x=ox;
    shbout_y=oy;

  };if(!(flg&0x02)) {
    pos=(pos<2) ? 2 : pos;
    shbout[pos]=0xE2|(6<<8);

  };

  return str++;

};

void oncont(int sig) {
  return;

};

//   ---     ---     ---     ---     ---

int main(int argc,char** argv) {

  char pwd[PATH_MAX+1];     // get bin folder
  char* binf;

  {
    binf = realpath(argv[0],pwd);
    int last=strlen(binf)-1;
    do {
      last--;

    } while(binf[last]!='/');
    binf[last+1]=0x00;

    char* lach=binf+(strlen(binf)-1);
    if(*lach!='/') {
      *(lach+1)='/';
      *(lach+2)=0x00;

    };
  };

//   ---     ---     ---     ---     ---

  int spwn_pid;
  void** mh;
  {

    // set outfile as an abspath

    // why? because it doubles as shared memory
    // we don't just need to know where it is
    // every child process needs to know as well

    char fpath[PATH_MAX+1];

    strcpy(fpath,binf);
    strcat(fpath,"shb7wt"); // cat bin+outfile

    // now make an mmapd file ;>
    mh=ntmap_origin(fpath,1);

    // mh key is needed solely as a brute-force way of
    // telling spwn about the shmem
    char* mh_key=encodemh(mh);

//   ---     ---     ---     ---     ---

    // we need to handle pauses
    signal(SIGCONT,oncont);

    // now make spwn
    spwn_pid=fork();
    if(!spwn_pid) {
      char* ex_argv[3];
      char ex_fpath[PATH_MAX+1];
      ex_argv[0]=ex_fpath;

      strcpy(ex_argv[0],binf);
      strcat(ex_argv[0],"spwn");

      ex_argv[1]=mh_key;
      ex_argv[2]=NULL;

      if(!access(ex_argv[0], X_OK)) {

        dup2(mapfd(mh),STDOUT_FILENO);

        execv(ex_argv[0],ex_argv);
        exit(-1);

      };CALOUT(E,"spwn not found\n");
      dlmap(mh);

      return -1;

    };
  };

  // first 4K of shmem are a stdout of sorts;
  // second 4K is for IPC

  char* c_out=(char*) *mh;

  shbout=((uint*) *mh)+0x0400;
  int* shbuf=(int*) (shbout+0x0400);

//   ---     ---     ---     ---     --- window init

  // create the window manager
  // that in turn spawns a window!

  NTCHMNG("SHB7x64",0);

  // *then* init the renderer
  // this is simply so there is an OpenGL context
  // before the system is even able to try and draw

  // honestly? you could flip them around
  // i init in this order because it makes sense to me

  NTSIN(2);

  // get charsize as a value between {0.0f,1.0f}
  // this value is relative to screen size

  // why? basis: the screen is a grid of characters
  // you want to know the percentage of the screen
  // taken up by each one of those

  float sc[2];GTCHRSZ(sc);

  // related to charsize, this gives you how many
  // characters fit in the screen on a given direction
  // x gives you the number of columns
  // y is number of rows

  uint  ws[2];GTSCRSZ(ws);

  // now multiply those, and you know how many
  // characters total a single screen can have

  uint CharCount=ws[0]*ws[1];
  shbout_mx=ws[0];shbout_my=ws[1];

  BEGPSH();pshout("$:",4,0);
  shbout_flg&=~0x02; // lock

//   ---     ---     ---     ---     --- loop head

  // the window manager will know when to exit
  while(GTRUN()) {

    // frame start is very important!

    // based on the value passed to frbeg, the manager
    // can speed up or slow down the program

    // this translates to longer or shorter sleep time
    // so mind this arg -- it controls everything

    // for now we only care if out has been written to

    int evilstate=FRBEG(
      ((shbout_flg&0x01)!=0)*4

    );if(evilstate) {
      break;

    };shbout_flg&=~0x01;

//   ---     ---     ---     ---     --- exec

    if(!(shbuf[0]&0x02)) {

      uchar* ibuff=GTIBUF();
      if(ibuff) {

        pshout(ibuff,5,0);
        if(ibuff[strlen(ibuff)-1]==0x0A) {
          shbuf[1]=shbout_x+(shbout_y*shbout_mx);
          shbuf[0]|=0x02;

          kill(spwn_pid,SIGCONT);
          pause();

          memset(shbout,0,0x0400*sizeof(uint));

          shbout_x=0;
          shbout_y=2;

          char* ic=c_out;
          while(*(ic=pshout(ic,5,0b10))) {;};
          //memset(c_out,0,0x0400*sizeof(uint));

// cp /cygdrive/c/cygwin64/bin/cygwin1.dll /cygdrive/d/lieb_git/kvr/bin/x64/cygwin1.dll

          shbout_x=0;
          shbout_y=0;

          pshout("$:",4,0);

        };

      };CLIBUF();

    };/* else if(*c_out) {


    };*/

//   ---     ---     ---     ---     --- loop tail

    // draw the screen buffer, it's done in one go!

    // how many chars you have is irrelevant, so
    // large prints are insanely cheap

    // do note that the buffer is only updated
    // when shbout is written to, and so if you
    // don't register a write then the previous
    // frame is drawn instead, repeating right
    // until you set the flag

    PSHCHR(shbout,shbout_flg&0x01);

    // these two must always go at the very bottom!
    // order matters: one gets current time, the
    // other calculates frame delta

    FREND();
    FRSLP();

  };

//   ---     ---     ---     ---     ---


  shbuf[0]|=0x01;
  kill(spwn_pid,SIGCONT);
  int wst;wait(&wst);

  ENDPSH();

  DLSIN();
  DLCHMNG();
  dlmap(mh);

  return 0;

};

//   ---     ---     ---     ---     ---
// #:0x1;>

