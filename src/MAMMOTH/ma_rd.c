/*/*//*//*//*//*//*//*//*//*//*//*//*//*//*/*/
/*    MA_RD                                 *
*                                           *
*     -reads pe files                       *
*                                           */
// *   ---     ---     ---     ---     ---  *
/*    LIBRE SOFTWARE                        *
*                                           *
*     Licenced under GNU GPL3               *
*     be a bro and inherit                  *
*                                           */
// *   ---     ---     ---     ---     ---  *
/* CONTRIBUTORS                             *
*     lyeb,                                 *
*                                           */
/*/*//*//*//*//*//*//*//*//*//*//*//*//*//*/*/

#include "KVRNEL/MEM/kvr_str.h"
#include "KVRNEL/MEM/kvr_bin.h"

#include "ma_cntx.h"
#include "ma_trans.h"
#include "ma_ins.h"

#include <stdio.h>
#include <string.h>
#include <math.h>

//   ---     ---     ---     ---     ---

static CTOK* ctok;

//   ---     ---     ---     ---     ---

void CALCUS_COLLAPSE(void)                  {

    switch(rd_cast) {

        case 0x03: {
            schar* r = (schar*) rd_lhand;
            schar* v = (schar*) rd_value;
            CALCUS_OPSWITCH;

        } case 0x07: {
            uchar* r = (uchar*) rd_lhand;
            uchar* v = (uchar*) rd_value;
            CALCUS_OPSWITCH;

//   ---     ---     ---     ---     ---

        } case 0x04: {
            sshort* r = (sshort*) rd_lhand;
            sshort* v = (sshort*) rd_value;
            CALCUS_OPSWITCH;

        } case 0x08: {
            ushort* r = (ushort*) rd_lhand;
            ushort* v = (ushort*) rd_value;
            CALCUS_OPSWITCH;

//   ---     ---     ---     ---     ---

        } case 0x05: {
            sint* r = (sint*) rd_lhand;
            sint* v = (sint*) rd_value;
            CALCUS_OPSWITCH;

        } case 0x09: {
            uint* r = (uint*) rd_lhand;
            uint* v = (uint*) rd_value;
            CALCUS_OPSWITCH;

//   ---     ---     ---     ---     ---

        } case 0x06: {
            slong* r = (slong*) rd_lhand;
            slong* v = (slong*) rd_value;
            CALCUS_OPSWITCH;

        } case 0x0A: {
            ulong* r = (ulong*) rd_lhand;
            ulong* v = (ulong*) rd_value;
            CALCUS_OPSWITCH;

//   ---     ---     ---     ---     ---

        } default: {
            float* r = (float*) rd_lhand;
            float* v = (float*) rd_value;
            CALCUS_OPSWITCH;

        };
    };                                                                                      };

//   ---     ---     ---     ---     ---

void TRNVAL(uint len)                       {

    int  evil  = 0;
    uint vlen  = len-1;

                                            // set OP_RADIX
    rd_flags |=                             (strstr(typedata.base, "float") != NULL) << 1;

//   ---     ---     ---     ---     ---


    if( (0x30 <= rd_rawv[0])
    &&  (0x39 >= rd_rawv[0]) ) {

        if(len>1) {                         // not a single digit

            if(strstr(rd_rawv, ".")
            || rd_flags&OP_RADIX  ) {       // is float
                goto RD_ASFLTP;

            }

            elif(rd_rawv[0]==0x30) {        // starts with a zero and is not 0.****

                if  (rd_rawv[1]==0x78) {    // is hexlit
                    TRHEXVAL                (rd_rawv+vlen, rd_value              );
                }

                elif(rd_rawv[1]==0x62) {    // is bitlit
                    TRBITVAL                (rd_rawv+vlen, rd_value              );

                };
            }

//   ---     ---     ---     ---     ---

            else {                          // string -> decimal
                TRDECVAL                    (rd_rawv, rd_value                   );

            }
        }

//   ---     ---     ---     ---     ---

        elif(len==1) {                      // boring corner case: single decimal digit
            if(rd_flags&OP_RADIX) {
                goto RD_ASFLTP;

            }; rd_value[0]=rd_rawv[0]-0x30;

        };
    }

//   ---     ---     ---     ---     ---

    elif(rd_rawv[0]==0x2E) {                // cool corner case: floats

                                            // catch incorrect data size
        RD_ASFLTP: MAMMCTCH                 (NOOVERSZ(rd_size, sizeof(float)     ),
                                             evil, MAMMIT_EV_VSIZ, typedata.base );

        rd_flags|=OP_RADIX;

        TRFLTVAL                            (rd_rawv, rd_value                   );
    }

//   ---     ---     ---     ---     ---

    else {

        void* nulmy     = NULL;             // dummy for getter/valid check
        uchar szdata[3] = {0,0,0};          // stor for typedata

                                            // fetch
        STR_HASHGET                         (LNAMES_HASH, rd_rawv, nulmy, 0      );

        if(nulmy!=NULL) {                   // get type and decode typedata
            uchar* type =                   ((ADDR*) nulmy)->id.type;            \
            VALSIZ                          (type, szdata                        );

//   ---     ---     ---     ---     ---

            if(szdata[2]) {                 // indexing required!
                if(!(rd_tkx<rd_tki)) {
                    NO_IDEX_OP:
                    CALOUT(E, "Symbol %s requires indexing operation\n", rd_rawv);

                    return;

                }

                elif(tokens[rd_tkx+1][0]==0x40) {
                    mammi->state |= MAMMIT_SF_PFET;
                    mammi->vaddr  = (uintptr_t) &(((ADDR*) nulmy)->box);
                    mammi->vtype  = szdata[0] | (szdata[1]<<8) | (szdata[2]<<16);

                } else {
                    goto NO_IDEX_OP;

                };

//   ---     ---     ---     ---     ---

            } else {                        // read as it is
                uint*  var  = ADDRFET       (uint, nulmy                        );

                for(uint i=0;i<szdata[0];i++) {
                    if(i>rd_size) { break; } rd_value[i] = var[i];

                };
            }

        } else {
            CALOUT(E, "Can't fetch key %s\n", rd_rawv);

        };
    };

//   ---     ---     ---     ---     ---

    BOT:

    if(rd_flags&OP_MINUS) {                 // if negative, do the bit flipping

        if(rd_flags&OP_RADIX) {             // floats
            *rd_value |= 0x80000000LL << (rd_cbyte*8);

        }

//   ---     ---     ---     ---     ---

        else {                              // everything else

            for(uint x=0, carry=0;
                x<rd_step; x++      ) {     // take two's
                rd_value[x]=(~rd_value[x]); // flip bits
                if(!x || carry) {
                    if(rd_value[x]==0xFFFFFFFFFFFFFFFFLL) {
                        rd_value[x] += 1;   // overflows. add one and set carry
                        carry        = 1;
                    }

                    else {
                        rd_value[x] += 1;   // won't overflow, so add and no carry
                        carry        = 0;

                    };
                };
            };
        };

    };                                                                                      };

//   ---     ---     ---     ---     ---

void MAEXPS(void)                           {

    TOP:                                    // if operator chars in token, eval and pop them
    switch(rd_rawv[0]) {

        default: break;

//   ---     ---     ---     ---     ---

        case 0x26:
            rd_flags |= OP_AMPER;

            goto POP_OPSTOP;

        case 0x80:
            rd_flags |= OP_DAMPR;
            MAMMIT_LVLB_NXT;

            goto POP_OPSTOP;

        case 0x8A:
            rd_flags |= OP_EAMPR;
            MAMMIT_LVLB_NXT;

            goto POP_OPSTOP;

//   ---     ---     ---     ---     ---

        case 0x7C:
            rd_flags |= OP_PIPE;

            goto POP_OPSTOP;

        case 0x86:
            rd_flags |= OP_DPIPE;
            MAMMIT_LVLB_NXT;

            goto POP_OPSTOP;

        case 0x92:
            rd_flags |= OP_EPIPE;
            MAMMIT_LVLB_NXT;

            goto POP_OPSTOP;

//   ---     ---     ---     ---     ---

        case 0x24:
            rd_flags |= OP_MONEY;

            goto POP_OPSTOP;

        case 0x88:
            rd_flags |= OP_EMONY;
            MAMMIT_LVLB_NXT;

            goto POP_OPSTOP;

        case 0x25:
            rd_flags |= OP_MODUS;

            goto POP_OPSTOP;

        case 0x89:
            rd_flags |= OP_EMODU;
            MAMMIT_LVLB_NXT;

            goto POP_OPSTOP;

//   ---     ---     ---     ---     ---

        case 0x5E:
            rd_flags |= OP_XORUS;

            goto POP_OPSTOP;

        case 0x91:
            rd_flags |= OP_EXOR;
            MAMMIT_LVLB_NXT;

            goto POP_OPSTOP;

//   ---     ---     ---     ---     ---

        case 0x2B:
            rd_flags |= OP_PLUS;
            MAMMIT_LVLB_NXT;

            goto POP_OPSTOP;

        case 0x81:
            rd_flags |= OP_PPLUS;
            MAMMIT_LVLB_NXT;

            goto POP_OPSTOP;

        case 0x8C:
            rd_flags |= OP_EPLUS;
            MAMMIT_LVLB_NXT;

            goto POP_OPSTOP;

//   ---     ---     ---     ---     ---

        case 0x2D:
            rd_flags |= OP_MINUS;

            goto POP_OPSTOP;

        case 0x82:
            rd_flags |= OP_MMINU;
            MAMMIT_LVLB_NXT;

            goto POP_OPSTOP;

        case 0x8D:
            rd_flags |= OP_EMINU;
            MAMMIT_LVLB_NXT;

            goto POP_OPSTOP;

//   ---     ---     ---     ---     ---

        case 0x2A:
            rd_flags |= OP_MUL;
            rd_flags &=~OP_DIV;

            goto POP_OPSTOP;

        case 0x8B:
            rd_flags |= OP_EMUL;
            MAMMIT_LVLB_NXT;

            goto POP_OPSTOP;

//   ---     ---     ---     ---     ---

        case 0x2F:
            rd_flags &=~OP_MUL;
            rd_flags |= OP_DIV;

            goto POP_OPSTOP;

        case 0x8E:
            rd_flags |= OP_EDIV;
            MAMMIT_LVLB_NXT;

            goto POP_OPSTOP;

//   ---     ---     ---     ---     ---

        case 0x21: MAMMIT_LVLB_NXT;
            rd_flags  = mammi->lvlb_stack[mammi->lvlb-1]&OP_MINUS;
            mammi->lvlb_stack[mammi->lvlb-1] &=~OP_MINUS;
            rd_flags |= OP_BANG;

            goto POP_OPSTOP;

        case 0x87:
            rd_flags |= OP_EBANG;
            MAMMIT_LVLB_NXT;

            goto POP_OPSTOP;

        case 0x7E: MAMMIT_LVLB_NXT;
            rd_flags  = mammi->lvlb_stack[mammi->lvlb-1]&OP_MINUS;
            mammi->lvlb_stack[mammi->lvlb-1] &=~OP_MINUS;
            rd_flags |= OP_TILDE;

            goto POP_OPSTOP;

//   ---     ---     ---     ---     ---

        case 0x3D:
            rd_flags |= OP_EQUAL;
            MAMMIT_LVLB_NXT;

            goto POP_OPSTOP;

        case 0x84:
            rd_flags |= OP_ECOOL;
            MAMMIT_LVLB_NXT;

            goto POP_OPSTOP;

//   ---     ---     ---     ---     ---

        case 0x28: MAMMIT_LVLB_NXT;
            goto POP_OPSTOP;

        case 0x40:

            if(!(mammi->state&MAMMIT_SF_PFET)) {
                CALOUT(E, "Using '@' operator without fetch-from\n");
                return;

            };

            rd_flags |= OP_AT;
            MAMMIT_LVLB_NXT;

            goto POP_OPSTOP;

        case 0x3A:

            if(mammi->state&MAMMIT_SF_PFET) {
                MAMMIT_LVLB_PRV;
                CALCUS_COLLAPSE();
                MAMMIT_LVLB_PRV;
                CALCUS_COLLAPSE();

                goto POP_OPSTOP;

            }; MAMMIT_LVLB_NXT;
            goto POP_OPSTOP;

//   ---     ---     ---     ---     ---

        case 0x3C:
            rd_flags |= OP_LT;
            MAMMIT_LVLB_NXT;

            goto POP_OPSTOP;

        case 0x83:
            rd_flags |= OP_LSHFT;
            MAMMIT_LVLB_NXT;

            goto POP_OPSTOP;

        case 0x8F:
            rd_flags |= OP_ELT;
            MAMMIT_LVLB_NXT;

            goto POP_OPSTOP;

//   ---     ---     ---     ---     ---

        case 0x3E:
            rd_flags |= OP_GT;
            MAMMIT_LVLB_NXT;

            goto POP_OPSTOP;

        case 0x85:
            rd_flags |= OP_RSHFT;
            MAMMIT_LVLB_NXT;

            goto POP_OPSTOP;

        case 0x90:
            rd_flags |= OP_RSHFT;
            MAMMIT_LVLB_NXT;

            goto POP_OPSTOP;

//   ---     ---     ---     ---     ---

        case 0x93:
        case 0x94:
            CALOUT(E, "Arrow/walkback not implemented (%s @%u)\n", __func__, __LINE__);

//   ---     ---     ---     ---     ---

        POP_OPSTOP:
            rd_rawv++; goto TOP;

    };

//   ---     ---     ---     ---     ---

    uint len = strlen(rd_rawv);
    if(!len) { goto END; }

    POP_TERMINATORS:                        // same as oppies, but at end of token
    switch(rd_rawv[len-1]) {

        default: break;

        case 0x29:

            if(mammi->lvlb) {
                MAMMIT_LVLB_PRV;            // lonely parens >;

            }; mammi->state &=~MAMMIT_SF_PSEC;

        POP_TESTOP:
            rd_rawv[len-1]=0x00;
            len--; if(!len) { goto END; }
            goto POP_TERMINATORS;

    };

//   ---     ---     ---     ---     ---

    TRNVAL(len);                            // translate string into value
    END: ;                                                                                  };

//   ---     ---     ---     ---     ---

void SECEXPS(void)                          {

    rd_oldval = rd_value+rd_step;

    for(uint x=0; x<rd_step; x++) {
        rd_oldval[x]=rd_value[x];

    }; mammi->state |= MAMMIT_SF_PSEC;

    MAEXPS();                               // expand token (parse operators, fetch values)

//   ---     ---     ---     ---     ---

    /*
    @(sec) syntax:

        $ lower bound                           x
        * ptr                                   x
        & upper bound

        ptr<N decrease ptr by N                 x
        ptr>N increase ptr by N                 x

        *>> move ptr to upper bound             x
        *<< move ptr to lower bound             x

        ptr#v exchange values betwen ptr && v
        ptr=v set *ptr to v
        =v    flood fill $ to Ç with v          x
        =     blank out $ to Ç                  x
        :     separate expressions              x

        */

//   ---     ---     ---     ---     ---

    MEMUNIT  sflags_i = 0;
    MEMUNIT* sflags   = mammi->lvlb_stack+0;
    MEMUNIT  sec_val  = ((*rd_value)>>(rd_cbyte*8))&szmask_a;

    for(uint x=0; x<rd_step; x++) {
        rd_value[x]=rd_oldval[x];

    }; MEMUNIT bstep = (!sec_val) ? 1 : sec_val;

//   ---     ---     ---     ---     ---

    SECTOP:

        switch(sflags[sflags_i]) {
        case OP_GT | OP_MONEY: sflags[sflags_i] &=~ (OP_GT | OP_MONEY);
            MOVBLK(&sec_beg, bstep); break;

        case OP_LT | OP_MONEY: sflags[sflags_i] &=~ (OP_LT | OP_MONEY);
            MOVBLK(&sec_beg,-bstep); break;

//   ---     ---     ---     ---     ---

        case OP_GT | OP_AMPER: sflags[sflags_i] &=~ (OP_GT | OP_AMPER);
            MOVBLK(&sec_end, bstep); break;

        case OP_LT | OP_AMPER: sflags[sflags_i] &=~ (OP_LT | OP_AMPER);
            MOVBLK(&sec_end,-bstep); break;

//   ---     ---     ---     ---     ---

        case OP_GT | OP_MUL: sflags[sflags_i] &=~ (OP_GT | OP_MUL);
            MOVBLK(&sec_cur, bstep); break;

        case OP_LT | OP_MUL: sflags[sflags_i] &=~ (OP_LT | OP_MUL);
            MOVBLK(&sec_cur,-bstep); break;

//   ---     ---     ---     ---     ---

        case OP_LSHFT | OP_MUL: sflags[sflags_i] &=~ (OP_LSHFT | OP_MUL);
            sec_cur.base  = sec_beg.base;
            sec_cur.cbyte = sec_beg.cbyte; break;

        case OP_RSHFT | OP_MUL: sflags[sflags_i] &=~ (OP_RSHFT | OP_MUL);
            sec_cur.base  = sec_end.base;
            sec_cur.cbyte = sec_end.cbyte; break;

//   ---     ---     ---     ---     ---

        case OP_EQUAL | OP_MUL: { sflags[sflags_i] &=~ (OP_EQUAL | OP_MUL);
            MEMUNIT* addr = ((MEMUNIT*) memlng->buff)+sec_cur.base;
            *addr        &=~(szmask_a<<(sec_cur.cbyte*8));
            *addr        |= (sec_val)<<(sec_cur.cbyte*8); break;
        }

        case OP_EMONY: { sflags[sflags_i] &=~ OP_EMONY;
            MEMUNIT* addr = ((MEMUNIT*) memlng->buff)+sec_beg.base;
            *addr        &=~(szmask_a<<(sec_beg.cbyte*8));
            *addr        |= (sec_val)<<(sec_beg.cbyte*8); break;
        }

        case OP_EAMPR: { sflags[sflags_i] &=~ OP_EAMPR;
            if(sec_end.cbyte<0) { break; }

            MEMUNIT* addr = ((MEMUNIT*) memlng->buff)+sec_end.base;
            *addr        &=~(szmask_a<<((sec_end.cbyte-rd_size)*8));
            *addr        |= (sec_val)<<((sec_end.cbyte-rd_size)*8); break;
        }

//   ---     ---     ---     ---     ---

        case OP_EQUAL: { sflags[sflags_i] &=~ OP_EQUAL;
            MEMUNIT* addr  = ((MEMUNIT*) memlng->buff)+0;
            uint old_base  = sec_beg.base;
            sint old_cbyte = sec_beg.cbyte;

            while(sec_beg.base!=sec_end.base) {
                addr[sec_beg.base] &=~(szmask_a<<(sec_beg.cbyte*8));
                addr[sec_beg.base] |= (sec_val)<<(sec_beg.cbyte*8);
                MOVBLK(&sec_beg, 1);

            };

            sec_beg.base  = old_base;
            sec_beg.cbyte = old_cbyte;

            break;
        }

        default: break;

//   ---     ---     ---     ---     ---

    }; sflags_i++; if(sflags_i<=mammi->lvlb) { goto SECTOP; }

    rd_lhand    = ((MEMUNIT*) memlng->buff)+(sec_cur.base);

    rd_result   = rd_lhand;
    rd_value    = rd_lhand+rd_step;

    mammi->lvlb = 0;                                                                        };

//   ---     ---     ---     ---     ---

void RDEXP(void)                            {

    uint   parsed     = 0;                  // how many *expressions* have been read
    uint ex_f         = rd_tkx+1;           // idex to first token in expression

    RSTPTRS();

//   ---     ---     ---     ---     ---

    EVAL_EXP: rd_tkx++;                     // read next token in expression

    if( !(rd_tkx<rd_tki   ) \
    ||   (parsed>=rd_elems) ) {             // ... or jump to end if all tokens read
        goto RESULT;

    }; rd_flags     = 0;                    // values defined above MAMMIT_OPSWITCH
       rd_rawv      = tokens[rd_tkx];       // current token

    if(mammi->state&MAMMIT_SF_PSEC) {
        goto SECEVAL;

    };

    rd_value        = rd_lhand+rd_step;     // put next *evaluated* token here
    CLMEM2(rd_value, rd_size);

//   ---     ---     ---     ---     ---

    if( (mammi->state&MAMMIT_SF_CPRC \
         &&  rd_rawv[0]==0x40        \
         &&  rd_rawv[1]==0x28        )

    ||  (mammi->state&MAMMIT_SF_CREG \
         &&  rd_rawv[0]==0x2C        ) ) {  // ,(?) || @(?) sec check

        while(mammi->lvlb) {                // collapse stack leftovers
            MAMMIT_LVLB_PRV;
            CALCUS_COLLAPSE();

        }; parsed++;                        // consider last value solved

        if(ex_f<rd_tkx) {                   // advance if sec is not first token
            BYTESTEP();
        };

//   ---     ---     ---     ---     ---

        rd_rawv++;
        if(rd_rawv[0]==0x28) {              // eval sec expression
            RSTSEC(); SECEVAL: SECEXPS();

        }; goto EVAL_EXP;
    }; MAEXPS();                            // no @(sec) so just expand...

//   ---     ---     ---     ---     ---

                                            // collapse arithmetic-wise
    SOLVE:
        //CALOUT(E, "l%d\t 0x%" PRIX32 "\t v%d\t -> ", *rd_lhand, rd_flags, *rd_value);
        CALCUS_COLLAPSE();
        //CALOUT(E, "%d\n", *rd_lhand);

    goto EVAL_EXP;

//   ---     ---     ---     ---     ---

    RESULT: if(mammi->lvlb>0) {             // collapse expression if unresolved
        MAMMIT_LVLB_PRV;                    // this doesn't account for unclosed () parens
        goto SOLVE;                         // so beware! PE$O will not care for that mistake

    };                                                                                      };

//   ---     ---     ---     ---     ---

void REGTP(void)                            {

    uchar* type       = typedata.base;      // fetch
    rd_tkx++;                               // move to next

    uchar* name       = tokens[rd_tkx];     // fetch, stay put

    rd_size           = 4;                  // ensure elem count is always a power of 2

//   ---     ---     ---     ---     ---

    switch(rd_cast) {                       // for 'dynamic' type-casting
                                            // we set size to sizeof x C type!
                                            // wait, you dunno whats a ulong??
                                            // look at KVRNEL/zjc_CommonTypes.h

        case 0x00: rd_size=sizeof(void* ); break;

        case 0x01:
        case 0x02: rd_size=sizeof(STARK ); break;

//   ---     ---     ---     ---     ---

        case 0x03:
        case 0x07: rd_size=sizeof(uchar ); break;

        case 0x04:
        case 0x08: rd_size=sizeof(ushort); break;

        case 0x05:
        case 0x09: rd_size=sizeof(uint  ); break;

        case 0x06:
        case 0x0A: rd_size=sizeof(ulong ); break;

//   ---     ---     ---     ---     ---

        default  : rd_size=sizeof(float ); break;

    }; szmask_a       = SIZMASK(rd_size);

//   ---     ---     ---     ---     ---

                                            // make it a pow2%UNITSZ
    rd_elems          = GTUNITCNT           (rd_size, typedata.arrsize                  );

    rd_cbyte          = 0;
    rd_step           = rd_size/UNITSZ;

    if(!rd_step) {
        rd_step       = 1;

    }; rd_units       = (rd_elems*rd_size)/UNITSZ;

//   ---     ---     ---     ---     ---

                                            // no redeclaration
    int    evil       = 0; MAMMCTCH         (NOREDCL(name), evil, MAMMIT_EV_DECL, name  );
    CALOUT                                  (K, ">%s %s[%u]\n", type, name, rd_elems    );

    //ctok              = MEMBUFF(memlng, CTOK, 8192);
    lngptr            = 0;
    RSTSEC();

                                            // solve expression and store result
    RDEXP                                   (                                           );
    VALNEW                                  (name, ((MEMUNIT*) memlng->buff)+0, rd_units);  };

//   ---     ---     ---     ---     ---

void RDPRC(ADDR* addr)                      {

// TODO:
//  -alias
//  -wed
//  -if/eif/else
//  -for/jmp

    TPADDR(addr);

    lngptr       = 0;
    uint   cunit = 0;

    uchar* name  = tokens[rd_tkx];
    uchar* nxt   = tokens[rd_tkx+1];

    uint   cbyte = 0;

//   ---     ---     ---     ---     ---

    if(nxt[0]==0x40) {
        nxt++; rd_rawv=nxt;

        RSTPTRS();
        MAEXPS ();

        MEMUNIT* addr = ((MEMUNIT*) memlng->buff)+sec_beg.base;

        cbyte=(*rd_value)*rd_size;
        rd_tkx++;

    };

//   ---     ---     ---     ---     ---

    RSTSEC();

    CODE*     code  = (CODE*) CURLVAL;

    uint      udr   = 0;                    // write offset into code->data

                                            // numerical address of current block
    uintptr_t vaddr =                       (uintptr_t) addr;

                                            // MEMUNIT offset into addr->box

    code->loc       = 0x00;

    for(uint x=0; x<sizeof(uintptr_t); x+=UNITSZ) {
        code->data[udr]=vaddr>>(x*UNITSZ); udr++;

    };

    code->data[udr] = ((ulong) cbyte) | (((ulong) rd_units) << 32);
    udr++;

    code->data[udr] = szmask_a;
    udr++;

//   ---     ---     ---     ---     ---

    //ctok = (CTOK*) (code->data+udr);

    RDEXP();

    /*code->data[udr] = *rd_result;*/
    //PROCADD(sizeof(CODE));

//   ---     ---     ---     ---     ---

//  addr->box[FETMASK(rd_units, cunit)] &=~(szmask_a    << (cbyte*8));
//  addr->box[FETMASK(rd_units, cunit)] |= (*rd_result) << (cbyte*8);

                       };

//   ---     ---     ---     ---     ---

void NTNAMES(void)                          {

                                            // interpreter nit
    ID id = IDNEW                           ("MAMM", "I"                               );
    MEMGET                                  (MAMMIT, mammi, NAMESZ*sizeof(uint), &id   );
    MKSTK                                   (byref(mammi->slstack), NAMESZ             );

    for(int x=NAMESZ-1; x>0; x--) {         // fill stack with indices
        STACKPUSH(byref(mammi->slstack), x);

                                            // nit the hashes
    }; GNAMES_HASH    = MKHASH              (7, "gnames_hash"                          );
       LNAMES_HASH    = MKHASH              (5, "lnames_hash"                          );

                                            // cool constant block for awesome prints
    ADDR* frblk       =                     (ADDR*) (mammi->lvalues+FRBLK);

    frblk->id.full[0] = 0x03;               // type/sizing flags
    frblk->id.full[1] = 0x00;
    frblk->id.full[2] = 0x00;
    frblk->id.full[3] = 0x00;

    frblk->id.full[4] = 0x46;               // manual strings like a champ
    frblk->id.full[5] = 0x52;
    frblk->id.full[6] = 0x42;
    frblk->id.full[7] = 0x4C;
    frblk->id.full[8] = 0x4B;
    frblk->id.full[9] = 0x00;

    frblk->box[0]     =  FREE_BLOCK;        // occult hexspeak; improves quality of dumps

//   ---     ---     ---     ---     ---


    SYMBOL base_dtypes[]={                  // note how each type has it's own lazy call ;>

        SYMNEW("TYPE", "void",   REGVOI),   // funptr or rawptr

        SYMNEW("TYPE", "stark",  REGTRK),   // funptr
        SYMNEW("TYPE", "nihil",  REGNHL),   // funptr

        SYMNEW("TYPE", "char",   REGCHR),   // unit/unit
        SYMNEW("TYPE", "wide",   REGWID),   // unit/4
        SYMNEW("TYPE", "int",    REGINT),   // unit/2
        SYMNEW("TYPE", "long",   REGLNG),   // unit

        SYMNEW("TYPE", "float",  REGFLT)

    };

    for(uint x=0, y=0; x<ARRSIZE(base_dtypes); x++) {

                                            // insert basic types into table
        STACKPOP                            (byref(mammi->slstack), y                  );

        mammi->slots[y] = base_dtypes[x];   // copy data to array and insert in lkp table
        HASHSET                             (GNAMES_HASH, byref(mammi->slots[y].id)    );

    };

//   ---     ---     ---     ---     ---

    SYMBOL contexts[]={                     // names of valid block-types

        SYMNEW("CNTX", "reg",  REGMA ),
        SYMNEW("CNTX", "hed",  NULL  ),
        SYMNEW("CNTX", "src",  NULL  ),

        SYMNEW("CNTX", "defn", NULL  ),
        SYMNEW("CNTX", "decl", NULL  ),
        SYMNEW("CNTX", "clan", NULL  ),
        SYMNEW("CNTX", "proc", PROCMA)

    };

    for(uint x=0, y=0; x<ARRSIZE(contexts); x++) {

                                            // get next slot idex
        STACKPOP                            (byref(mammi->slstack), y                  );

        mammi->slots[y] = contexts[x];      // copy data to array and insert in lkp table
        HASHSET                             (GNAMES_HASH, byref(mammi->slots[y].id)    );

    };                                                                                      };

void DLNAMES(void)                          { DLMEM(LNAMES_HASH);                           \
                                              DLMEM(GNAMES_HASH); DLMEM(mammi);             };

//   ---     ---     ---     ---     ---

void CHKTKNS(void)                          {

    uchar* SEQ[MAMMIT_TK_COUNT];

    uchar* SEQN;
    uint   SEQI;

//   ---     ---     ---     ---     ---

    if(!(mammi->state&MAMMIT_SF_CNTX)) {    // no current context

        SEQN   = "CNON";
        SEQ[0] = "CNTX\x01";
        SEQI   = 2;

    }

//   ---     ---     ---     ---     ---

    elif(mammi->state&MAMMIT_SF_CREG) {     // context == reg

        SEQN   = "CREG";
        SEQ[0] = "TYPE\x01";
        SEQI   = 2;

    }

    elif(mammi->state&MAMMIT_SF_CPRC) {     // context == proc

        SEQN   = "CPRC";
        SEQ[0] = "LVAL\x01";
        SEQI   = 1;

    };

//   ---     ---     ---     ---     ---

    if(SEQI > (rd_tki)) {
        CALOUT(E, "Invalid token sequence number for context %s\n", SEQN);
        return;

    };

//   ---     ---     ---     ---     ---

    for(rd_tkx=0; rd_tkx<rd_tki; rd_tkx++) {

        uchar seq_k[5];                     // symbol-type key used as filter into table
        uchar seq_i;                        // *minimum* number of inputs taken by symbol

        for(uint y=0; y<4; y++) {           // unpack SEQ into k and i 
            seq_k[y]=SEQ[rd_tkx][y];

        }; seq_k[4]='\0';                   // put that nullterm there
        seq_i=SEQ[rd_tkx][4];               // fetch argcount

//   ---     ---     ---     ---     ---

        int       valid = 1;
        SYMBOL*   sym   = NULL;

        if(strcmp(seq_k, "NAME")) {         // names can be anything, live the danger

            void* nulmy = NULL;             // non-naming tokens can carry funcalls
                                            // they must exist within our gnames table, so
                                            // in-invoking any one of them we must fetch
                                            // strictly by key, since strings is all we have
                                            // wait, wat? embedding commands in text? in C?
                                            // well, who would've thought...

//   ---     ---     ---     ---     ---

            uchar key[MAMMIT_TK_WIDTH];     // fetch this from table

            uint  len=strlen(
                    tokens[rd_tkx]

            );                              // just so I don't have to do {cpy} while(*t++)

//   ---     ---     ---     ---     ---

            if(!strcmp(seq_k, "TYPE")) {    // corner case: type flags, arrays, pointers

                UPKTYPE(tokens[rd_tkx]);    // decompose type descriptor

                uint x;                     // like a fashion designer, take measures ;>
                len = strlen                (typedata.base                             );

                for(x=0; x<len; x++) {      // now copy
                    key[x]=typedata.base[x];// key == base typename

                }; key[x]=0x00;             // put the nullterm there...
            }

//   ---     ---     ---     ---     ---

            else { uint x;
                for(x=0; x<len; x++) {       // now copy
                    key[x]=tokens[rd_tkx][x];// key == base typename

                }; key[x]=0x00;             // put the nullterm there...
            };

//   ---     ---     ---     ---     ---


            if(!strcmp(seq_k, "LVAL")) {    // fetch from local table __by_key__ (slowest)
                STR_HASHGET                 (LNAMES_HASH, key, nulmy, 0                );

                if(nulmy!=NULL) {
                    ADDR* addr=(ADDR*) nulmy;

                    // symbol is funptr
                    if( 0<=addr->id.type[0]
                    &&  2>=addr->id.type[0] ) {
                        ;

                    } else {
                        RDPRC(addr);

                    };
                }; nulmy=NULL;

            } else {                        // ^same on global table
                STR_HASHGET                 (GNAMES_HASH, key, nulmy, 0                );

            }; valid = nulmy!=NULL;         // success on fetch means we might have to
            if(valid) {                     // modify mammit state based on symbols read
                sym = (SYMBOL*) nulmy;      // no sneaky funcalls on invalid input, please

        }};                                 // now hope you didn't make any silly mistakes

//   ---     ---     ---     ---     ---

        if(valid) {                         // debugger's falacy:
                                            // "if it prints something, it does something!"
            /*CALOUT                          (K, "%u: %s %s\n",                         \
                                             rd_tkx, seq_k, tokens[rd_tkx]             );*/

            if(sym) { if(sym->onrd) {       // this is why I want if x then y syntax
                sym->onrd();

            }};
        }

        else {                              // very much the same as above
            /*CALOUT                          (K, "%u: INVALID %s: '%s'\n",              \
                                             rd_tkx, seq_k, tokens[rd_tkx]             );*/

    }};                                                                                     };

//   ---     ---     ---     ---     ---

void RDNXT(void)                            {

    uchar op[16]; CLMEM2(op, 16);           // operator storage ;>

    uint  opi   = 0;                        // idex into opstor
    uint  s_opi = 0;                        // special oppy idex

    TOP:

        if(gblevil) { return; }             // something went wrong last pass

        rd_prv=rd_cur;                      // remember current
        rd_cur=rd_buff[rd_pos];             // advance to next

        rd_pos++; rd_nxt=rd_buff[rd_pos];
        rd_wid = rd_nxt | (rd_cur<<8);

//   ---     ---     ---     ---     ---

        if(mammi->state&MAMMIT_SF_PLCO) {
            if(rd_cur==0x0A) {
                mammi->state &=~MAMMIT_SF_PLCO;

            } else { goto TOP; }
        }

        elif(mammi->state&MAMMIT_SF_PMCO) {
            if(rd_wid==0x2A2F) {
                mammi->state &=~MAMMIT_SF_PMCO;

            } else { goto TOP; }
        };

//   ---     ---     ---     ---     ---

    switch(rd_wid) {                        // check for special wide chars

        case 0x0000: return;                // double nullterm should never happen

        case 0x243A:                        //      $:

            mammi->state |= MAMMIT_SF_PESC;
            rd_tk[rd_tkp]=rd_cur; rd_tkp++; rd_pos++; goto TOP;

        case 0x3B3E:                        //      ;>

            mammi->state &=~MAMMIT_SF_PESC; rd_pos++; goto TOP;

        case 0x2F2A:                        //      /*

            mammi->state |= MAMMIT_SF_PMCO; rd_pos++; goto TOP;

        case 0x2F2F:                        //      //

            mammi->state |= MAMMIT_SF_PLCO; rd_pos++; goto TOP;

//   ---     ---     ---     ---     ---    // doubled operators

        case 0x2626: s_opi=0x00; goto SOPPY;// 80   &&
        case 0x2B2B: s_opi=0x01; goto SOPPY;// 81   ++
        case 0x2D2D: s_opi=0x02; goto SOPPY;// 82   --
        case 0x3C3C: s_opi=0x03; goto SOPPY;// 83   <<
        case 0x3D3D: s_opi=0x04; goto SOPPY;// 84   ==
        case 0x3E3E: s_opi=0x05; goto SOPPY;// 85   >>
        case 0x7C7C: s_opi=0x06; goto SOPPY;// 86   ||

//   ---     ---     ---     ---     ---    // op= operators

        case 0x213D: s_opi=0x07; goto SOPPY;// 87   !=
        case 0x243D: s_opi=0x08; goto SOPPY;// 88   $=
        case 0x253D: s_opi=0x09; goto SOPPY;// 89   %=
        case 0x263D: s_opi=0x0A; goto SOPPY;// 8A   &=
        case 0x2A3D: s_opi=0x0B; goto SOPPY;// 8B   *=
        case 0x2B3D: s_opi=0x0C; goto SOPPY;// 8C   +=
        case 0x2D3D: s_opi=0x0D; goto SOPPY;// 8D   -=
        case 0x2F3D: s_opi=0x0E; goto SOPPY;// 8E   /=
        case 0x3C3D: s_opi=0x0F; goto SOPPY;// 8F   <=
        case 0x3E3D: s_opi=0x10; goto SOPPY;// 90   >=
        case 0x5E3D: s_opi=0x11; goto SOPPY;// 91   ^=
        case 0x7C3D: s_opi=0x12; goto SOPPY;// 92   |=

//   ---     ---     ---     ---     ---    // very special operators

        case 0x2D3E: s_opi=0x13; goto SOPPY;// 93   ->
        case 0x3E2D: s_opi=0x14; goto SOPPY;// 94   <-

        SOPPY: rd_cur=0x80+s_opi; rd_pos++; break;

    };

    switch(rd_cur) {

//   ---     ---     ---     ---     ---    BLANKS 0-15

        case 0x00: return;                  // end on nullterm \0

        case 0x01: goto APTOK;              // skip control chars
        case 0x02: goto APTOK;              // some or all of these could be used in the future
        case 0x03: goto APTOK;              // for now they are meaningless, so skip all
        case 0x04: goto APTOK;              // keeping the cases for when the day comes
        case 0x05: goto APTOK;
        case 0x06: goto APTOK;
        case 0x07: goto APTOK;
        case 0x08: goto APTOK;
        case 0x09: goto APTOK;

        case 0x0A: rd_line++; goto APTOK;   // inc line counter on \n ;)

        case 0x0B: goto APTOK;
        case 0x0C: goto APTOK;
        case 0x0D: goto APTOK;
        case 0x0E: goto APTOK;
        case 0x0F: goto APTOK;

//   ---     ---     ---     ---     ---    BLANKS 16-31

        case 0x10: goto APTOK;              // same as above, meaningless for now
        case 0x11: goto APTOK;              // reason why they're here, above as well
        case 0x12: goto APTOK;
        case 0x13: goto APTOK;
        case 0x14: goto APTOK;
        case 0x15: goto APTOK;
        case 0x16: goto APTOK;
        case 0x17: goto APTOK;
        case 0x18: goto APTOK;
        case 0x19: goto APTOK;
        case 0x1A: goto APTOK;
        case 0x1B: goto APTOK;
        case 0x1C: goto APTOK;
        case 0x1D: goto APTOK;
        case 0x1E: goto APTOK;
        case 0x1F: goto APTOK;

//   ---     ---     ---     ---     ---    BLANK 32

        case 0x20: APTOK:                   // for space and skipped (see above x2)

            if(!rd_tkp) { break; }

            if(!(  0x01 <= rd_prv \
                && 0x20 >= rd_prv ) ) {     // if previous is not *also* control char {1,32}

                rd_tkp = 0; rd_tki++;       // reset position && advance slot
                rd_tk  = tokens[rd_tki];    // push token

            }; break;

//   ---     ---     ---     ---     ---    OPERATORS L

        case 0x3A:                          // :    colon

            if(mammi->state&MAMMIT_SF_PSEC) {
                goto APTOK;

            }; goto OP_NONSEC;

//   ---     ---     ---     ---     ---    // @(sec) operators
                                            // used for memlng hackery AND calcus

        case 0x24:                          // $    money
        case 0x26:                          // &    amper
        case 0x28:                          // (    l_brackus
        case 0x2A:                          // *    mul

        case 0x3C:                          // <    lt
        case 0x83:                          // <<   lshft

        case 0x3D:                          // =    equal
        case 0x88:                          // $=   emony
        case 0x8B:                          // *=   emul
        case 0x8A:                          // &=   eampr

        case 0x3E:                          // >    gt
        case 0x85:                          // >>   rshft

            if(mammi->state&MAMMIT_SF_PSEC) {
                goto FORCE_INSERT;

            };

//   ---     ---     ---     ---     ---    non @(sec) operators

        case 0x27:                          // '    squot
        case 0x22:                          // "    dquot
        case 0x23:                          // #    kush

        case 0x25:                          // %    modus
        case 0x89:                          // %=   emodu


        case 0x2B:                          // +    plus
        case 0x81:                          // ++   pplus
        case 0x8C:                          // +=   eplus

        case 0x2D:                          // -    minus
        case 0x82:                          // --   mminu
        case 0x8D:                          // -=   eminu

        case 0x2F:                          // /    div
        case 0x8E:                          // /=   ediv

        case 0x40:                          // @    at
        case 0x3F:                          // ?    quest
        case 0x21:                          // !    bang
        case 0x87:                          // !=   ebang

        case 0x5C:                          // \    escapus
        case 0x5B:                          // [    l_subscriptus

        case 0x5E:                          // ^    xorus
        case 0x91:                          // ^=   exor

        case 0x7C:                          // |    pipe
        case 0x92:                          // |=   epipe

        case 0x7E:                          // ~    tilde

        case 0x8F:                          // <=   elt
        case 0x90:                          // >=   egt
        case 0x84:                          // ==   ecool

        case 0x80:                          // &&   dampr
        case 0x86:                          // ||   dpipe

        case 0x93:                          // ->   arrow
        case 0x94:                          // <-   brrow

            OP_NONSEC:
            op[opi]=rd_cur; opi++;
            goto APTOK;

//   ---     ---     ---     ---     ---    OPERATORS R

        case 0x29:                          // )    r_brackus
        case 0x5D:                          // ]    r_subscriptus

            mammi->state  &=~MAMMIT_SF_PSEC;
            rd_tk[rd_tkp]  = rd_cur; rd_tkp++;
            goto APTOK;

//   ---     ---     ---     ---     ---    TERMINATORS

        case 0x2C:                          // ,    comma

            // APTOK copy-paste
            if(!(  0x01 <= rd_prv \
                && 0x20 >= rd_prv ) ) {     // if previous is not *also* control char {1,32}

                rd_tkp = 0; rd_tki++;       // reset position && advance slot
                rd_tk  = tokens[rd_tki];    // push token

            };

            rd_tk[rd_tkp] = rd_cur;         // set comma
            rd_tkp++;

            if(rd_nxt!=0x28) {              // check for special sequence ,(
                rd_tkp    = 0; rd_tki++;    // if not found, just leave the comma alone
                rd_tk     = tokens[rd_tki];

            } else {
                mammi->state |= MAMMIT_SF_PSEC;

            }; break;

        case 0x7B: MAMMIT_LVLA_NXT          // {    l_curlius
            goto PROCST;

        case 0x7D: MAMMIT_LVLA_PRV          // }    r_curlius
            goto PROCST;

        case 0x3B: PROCST:                  // ;    #EE

            if(rd_tkp) {                    // >only when theres chars left in slot
                rd_tkp = 0; rd_tki++;       // reset position && advance slot
                rd_tk  = tokens[rd_tki];    // push token

            }; if(!rd_tki) { break; }       // no tokens, no checking

            CHKTKNS();                      // process token sequence and clean up

            for(uint i=0; i<MAMMIT_TK_COUNT; i++) {
                CLMEM2(tokens[i], MAMMIT_TK_WIDTH);

            }; rd_tkp=0; rd_tki=0;
            rd_tk=tokens[rd_tki];

            // clean non-context stateflags
            // ruins multi-statement peso escapes, ill fix that later
            mammi->state &=~MAMMIT_SF_PESO;

            break;

//   ---     ---     ---     ---     ---    CHARACTERS

        default:                            // 'cat' char to cur token

            // prefix token with operator(s)
            if(opi) { uint y=0; while(op[y]) {
                rd_tk[rd_tkp]=op[y]; rd_tkp++;
                op[y]=0x00; y++;

            } opi=0; };

            FORCE_INSERT:
                rd_tk[rd_tkp]=rd_cur; rd_tkp++; break;

    }; if(rd_nxt) { goto TOP; };                                                            };

//   ---     ---     ---     ---     ---

int main(int argc, char** argv)             {

    int   prmemlay = 0;
    int   from_bin = 0;
    char* bin_path = "";
    BIN*  bin      = NULL;

    for(int x=1; x<argc; x++) {
        char* arg=argv[x];
        if(!strcmp(arg, "-ml")) {
            prmemlay=1;

        } elif(strstr(arg, "-f")!=NULL) {
            from_bin=1; bin_path=arg+2;

        };
    };

    NTNAMES();
    MEM* s  = MKSTR("MAMM_RD", 1024, 1); CLMEM(s);
    LDLNG(ZJC_DAFPAGE); memlng = GTLNG(); CLMEM(memlng);
    rd_buff = MEMBUFF(s, uchar, 0);

//   ---     ---     ---     ---     ---

    if(from_bin) { int rb;
        BINOPEN (bin, bin_path, KVR_FMODE_RB, KVR_FTYPE_DMP, rb);
        PBINREAD(bin, rb, uchar, 1024, rd_buff                 );

    };

//   ---     ---     ---     ---     ---

    CALOUT(E, "\e[38;2;128;255;128m\n$PEIN:\n%s\n\e[0m\e[38;2;255;128;128m$OUT:", rd_buff);
    RDNXT(); CALOUT(E, "\e[0m");

    /*lmpush(MAMMIT_CNTX_FETCH(pe_proc, 0)); lmpop();*/

    if(prmemlay) { CHKMEMLAY(); };

    if(from_bin) {
        BINCLOSE(bin);
        DLMEM   (bin);

    };

//   ---     ---     ---     ---     ---

    DLMEM(memlng);
    DLMEM(s);
    DLNAMES();

    return 0;

};

//   ---     ---     ---     ---     ---