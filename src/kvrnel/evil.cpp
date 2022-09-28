// ---   *   ---   *   ---
// EVIL
// hoh sys
//
// LIBRE SOFTWARE
// Licenced under GNU GPL3
// be a bro and inherit
//
// CONTRIBUTORS
// lyeb,

// ---   *   ---   *   ---
// deps

  #include <cstdio>
  #include <cstdarg>
  #include <cstdlib>
  #include <cstring>
  #include <cmath>

  #include "evil.hpp"

// ---   *   ---   *   ---
// fast check for known errors

void Evil::errchk(

  Dang* call,
  int*  retx,

  const short errcode,
  const char* info

) {

  if(

    (call->state==AR_FATAL)
  | (call->state==AR_ERROR)

  | (*retx && (*retx != call->state))

  ) {terminator(errcode,info);};

  *retx=call->state;
  poplog();

};

// ---   *   ---   *   ---
// shortens path

char* Evil::shpath(const char* path) {

  char* file = (char*) path;
  char  c    = '\0';

  // iter through string
  while (*path++){

    // is forward slash
    c=*path;
    if(c== '/') {

      // cut here
      if(strlen(path)<=FLEN_TGT)  {
        file=(char*) ++path;
        break;

      };

    };

  };

  return file;

};

// ---   *   ---   *   ---

Dang* Evil::geterrloc(

  const char* p,
  const char* f,

  int l

) {

  Dang* out=&(m_log[m_ptr]);

  char callbranch[66];

  callbranch[0]='\b';
  callbranch[1]='\0';

// ---   *   ---   *   ---

  if(m_depth) {

    // avoid recursion or you'll see this one
    if(m_depth > 63) {

      fprintf(

        stderr,

        "Call depth exceeded\n"
        "#:!;> %s at %s (line %i)\n",

        p,f,l

      );

      out=NULL;

    };

// ---   *   ---   *   ---
// fill callbranch so that
//
// depth 0
// \--> depth 1
// .  \--> depth 2
// .
// \--> depth 1
//
// etc.

    // branch start
    callbranch[0]='\\';

    // branch end
    callbranch[m_depth+1] = '>';
    callbranch[m_depth+2] = '\0';

    // fill between
    for(int x=1;x<m_depth+1;x++) {
      callbranch[x]='-';

    };

  } else {
    callbranch[0]='\n';
    callbranch[1]='\0';

  };

// ---   *   ---   *   ---
// assign current depth

  out->depth=m_depth++;

  // paste into logs
  snprintf(

    out->loc, 255,

    "%s%s%i:\x1b[0m %s<%s>"
    "\x1b[0m on func %s%s"
    "\x1b[0m line %s%i\e[0m\n",

    PALETTE[1],

    callbranch,m_ptr,

    PALETTE[0],p,
    PALETTE[2],f,
    PALETTE[3],l

  );

// ---   *   ---   *   ---
// move to next entry

  m_ptr++;
  return out;

};

// ---   *   ---   *   ---
// spit out entries

void Evil::printlog(int flush) {

  for(int i=0;i<m_ptr;i++) {
    fprintf(

      stderr,"%s%s",

      PALETTE[4],
      m_log[i].loc

    );

// ---   *   ---   *   ---
// 'delete' this entry ;>

    if(flush) {
      m_log[i].loc[0]='\0';

    };

// ---   *   ---   *   ---
// reset counters if need

  };

  if(flush) {m_ptr=0;};

};

// ---   *   ---   *   ---
// 'delete' last entry

void Evil::poplog(void) {

  if(m_ptr) {
    m_ptr--;

    m_log[m_ptr].loc[0]='\0';
    m_log[m_ptr].depth=0;

  };

  m_depth-=m_depth!=0;

};

// ---   *   ---   *   ---
// ill be back

void Evil::terminator (

  int errcode,
  const char* info

) {

  char mstart[256];
  const char* mbody;

  int color_code;

  if(errcode<0x40) {
    snprintf(mstart,255,"%s#:!;> ",PALETTE[6]);
    m_state=AR_FATAL;

    color_code=6;

  } else {
    snprintf(mstart,255,"%s#:!;> ",PALETTE[7]);
    m_state=AR_ERROR;

    color_code=7;

  };

// ---   *   ---   *   ---
// let the printing begin

  fprintf(

    stderr,

    "\n%3$s"

    "%1$s"
    "%4$04X "
    "%2$s",

    PALETTE[3],PALETTE[4],
    mstart,errcode

  );

// ---   *   ---   *   ---
// NOTE:
//
//  these first few are from an old version
//  and were never used. i leave them
//  because they amuse me

  switch(errcode) {

    case  0:
      mbody="Insuficcient memory %s(%s requested)";
      break;

    case  1:
      mbody="Access violation %s%s";
      break;

    case  2:
      mbody="The end times have come %s%s";
      break;

    case  3:
      mbody="You just did something illegal %s%s";
      break;

// ---   *   ---   *   ---
// these are real errmesses ;>

    case  4:
      mbody="ZLIB went PLOP %s(status %s)";
      break;

    case 64:
      mbody="Couln't open file %s<%s>";
      break;

    case 65:
      mbody="File couldn't be closed %s<%s>";
      break;

    case 66:
      mbody="Error writting to file %s<%s>";
      break;

    case 67:
      mbody="Error reading from file %s<%s>.";
      break;

    case 68:
      mbody="Inappropriate file signature %s<%s>";
      break;

    case 69:
      mbody="Error deleting file %s<%s>";
      break;

    case 70:
      mbody="Stack is full; can't push %s%s";
      break;

    case 71:
      mbody="Stack is empty; can't pop %s%s";
      break;

    case 72:
      mbody="Can't insert key %s<%s>";
      break;

    case 73:
      mbody="Key %s<%s>\x1b[0m not found";
      break;

// ---   *   ---   *   ---
// 0x010000 onwards is peso debug

    case 0x010001:
      mbody=

        "Multiple declarations "
        "of symbol %s'%s'"

      ;

      break;

    case 0x010002:
      mbody="Oversized value for type %s%s";
      break;

    case 0x010003:
      mbody=

        "Element %s%s\e[0m is not "
        "an address-of"

      ;

      break;

    case 0x010004:
      mbody="Cannot jump to address %s<%s>";
      break;

    default:

      mbody=

        "(!!) I screwed up the "
        "math somewhere...\n"
        "%s%s\n"

      ;

      break;

// ---   *   ---   *   ---
// spit out the wall of errors

  };

  fprintf(stderr,mbody,PALETTE[0],info);

  fprintf(

    stderr,

    "\n%s#:!;>%s TRACE:\n",
    PALETTE[color_code],PALETTE[4]

  );

  printlog(0);
  fprintf(stderr,"\n");

  if(m_state==AR_FATAL) {

    fprintf(
      stderr,

      "%sTERMINATED\e[0m\n\n",
      PALETTE[color_code]

    );

    exit(m_state);

  };

};

//   ---     ---     ---     ---     ---
