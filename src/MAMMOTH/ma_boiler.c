/*/*//*//*//*//*//*//*//*//*//*//*//*//*//*/*/
/*    MA_BOILER                             *
*                                           *
*     -annoying bits of rd                  *
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

#include "ma_boiler.h"
#include <string.h>
#include <math.h>

//   ---     ---     ---     ---     ---

uint GTUNITCNT(uint size, uint mag)         {

    while(size > UNITSZ) {
        mag++; size-=UNITSZ;

    }; return (uint) ((pow(2, 2+mag) + 0.5)/size);                                          };

void MOVBLK(BLK* b, int dirn)               {

    int sign       = (dirn<0) ? -1 : 1;
    for(uint x=0; x<(dirn*sign); x++) {
        b->cbyte      += ((int) (rd_size))*sign;

        if(dirn<0) {
            if(b->cbyte < 0) {
                b->base--;
                b->cbyte+=UNITSZ;

            };

        } else {
            if ( ( b->cbyte > UNITSZ )  \
            || ( !(b->cbyte%UNITSZ))  ) {
                b->base++;
                b->cbyte-=UNITSZ;

            };
        };
    }; b->base=(b->base)&(rd_units-1);                                                      };

uint GTARRSZ(uint loc)                      { uintptr_t a = mammi->jmpt[loc+0];             \
                                              uintptr_t b = mammi->jmpt[loc+1];             \
                                              return (b-a)/UNITSZ;                          };

//   ---     ---     ---     ---     ---

int NOREDCL(uchar* name)                    {

    void* nulmy = NULL;                     // check if name exists in global scope
    STR_HASHGET                             (GNAMES_HASH, name, nulmy, 0);

    if(nulmy) {                             // freak out if it does;
        return ERROR;

                                            // now check if name exists in *local* scope
    }; STR_HASHGET                          (LNAMES_HASH, name, nulmy, 0);

    if(nulmy) {                             // ... and freak out if it does
        return ERROR;

    }; return DONE;                                                                         };

int NOOVERSZ(uint sz, uint f)               { if(sz%f) { return ERROR; }; return DONE;      };

//   ---     ---     ---     ---     ---

// these funcs below? lazy nihil way to identify type
// without having to do a strcmp or str_hashget...

// it IS a little silly to hardcode it like this
// BUT: aren't base types HARDCODED to BEG with?

// the initial hash-gotten symbol can redirect
// to a single function, that function KNOWS
// how to interpret the data; this is simpler

// repetitive? yes
// does it work? yes
// optimal? depends

// its either this or using arguments
// AND yet more branches per call, mind you
// so no thanks, this mess will do

//   ---     ---     ---     ---     ---    void, nihil, stark

void REGVOI(void)                           { rd_cast = 0x00;                               };
void REGNHL(void)                           { rd_cast = 0x01;                               };
void REGTRK(void)                           { rd_cast = 0x02;                               };

//   ---     ---     ---     ---     ---    char

void REGCHR(void)                           { rd_cast = 0x03;                               };
void REGWID(void)                           { rd_cast = 0x04;                               };
void REGWRD(void)                           { rd_cast = 0x05;                               };
void REGLNG(void)                           { rd_cast = 0x06;                               };

//   ---     ---     ---     ---     ---    float

void REGFLT(void)                           { rd_cast = 0x0B;                               };

//   ---     ---     ---     ---     ---

void UPKTYPE(uchar* typeval)                {

    CLMEM2(                                 // clean-up leftovers from previous use
        (void*) &typedata,
        sizeof(TYPEDATA)

    );

    uchar* base = typedata.base+0;          // point to our static block of chars
    uint   len  = strlen(typeval);          // length of original string

    ushort i    = 0;                        // offset into original string
    ushort j    = 0;                        // offset into __target__ string

    uchar  c    = 0x00;                     // character currently being pointed at

//   ---     ---     ---     ---     ---

    TOP:                                    // read through original string and decompose

        c=typeval[i];                       // get next char

        if((c >= 0x30) && (0x39 >= c)) {    // if char is number
            typedata.arrsize*=10;           // left-shift
            typedata.arrsize+=c-0x30;       // add unit
            goto BOT;                       // skip to end

        }; if(j || i>2) { goto MID; };      // skip if char > original[0-2]

//   ---     ---     ---     ---     ---

    switch(c) {

        case 0x7A:                          // z, zzzzstatic (lots o' volts)
            typedata.flags|=0x01; goto BOT;
        case 0x71:                          // q, qqqqconstant
            typedata.flags|=0x02; goto BOT;
        case 0x75:                          // u, uuuuunsigned
            typedata.flags|=0x04; goto BOT;

        default:
            break;

    };

//   ---     ---     ---     ---     ---

    MID:

    switch(c) {

        case 0x2A:                          // ptr to ptr to ptr to ptr to ptpr to what
            typedata.indlvl++; break;

        default:                            // everything else goes
            base[j]=c; j++; break;

    }; BOT: i++; if(i<len) { goto TOP; }
    typedata.arrsize=(typedata.arrsize<2) ? 2 : typedata.arrsize;                           };

//   ---     ---     ---     ---     ---

SYMBOL SYMNEW(uchar* fam,
              uchar* key,
              NIHIL  fun)                   {

    SYMBOL sym = {0};                       // simple 'constructor', so to speak

    sym.id     = IDNEW(fam, key);           // fill out the id
    sym.onrd   = fun;                       // when !fun, you're bored and null

    return sym;                                                                             };

//   ---     ---     ---     ---     ---

void VALNEW(uchar*   name,
            MEMUNIT* val ,
            uint     size)                  {

    if(size%2) { size++; }

    MEMUNIT* data    = CURLVAL;
    mammi->lvaltop  += size;

    cur_cntx->size  += size;
    cur_cntx->elems++;

    for(uint x=0; x<size; x++) {            // copy units over
        data[x]=val[x];
                                            // send lookup data to table
    }; JMPT_INSERT                          (data, size, "VAL*", name);                     };

//   ---     ---     ---     ---     ---

uchar VALSIZ(uchar type)                    {

    if(type < 0x03) {                       // funptr
        return sizeof(STARK);

    } elif( 0x03 <= type \
      &&    0x06 >= type ) {                // num type
        return (uchar) (pow(2, type-0x03)+0.5);

    }; return sizeof(float);                                                                };

//   ---     ---     ---     ---     ---

void PROCADD(uchar* name,
             CODE*  val ,
             uint   size)                   {

    if(size%2) { size++; }

    mammi->lvaltop  += size;
    cur_cntx->size  += size;
    cur_cntx->elems++;

    JMPT_INSERT(val, size, "INS*", name);                                                   };

//   ---     ---     ---     ---     ---

void TPADDR(uchar type, uint elems)         {

                                            // sizing ints for the read
    rd_size         = VALSIZ                (type                );
    rd_elems        = elems;

    rd_step         = rd_size/UNITSZ;

//   ---     ---     ---     ---     ---

    if(!rd_step) {
        rd_step     = 1;

    }; rd_units     =                       (rd_elems*rd_size)/UNITSZ;

    if(!rd_units) {
        rd_units    = 1;

    };

//   ---     ---     ---     ---     ---

    szmask_a        = SIZMASK(rd_size);
    rd_cbyte       ^= rd_cbyte;                                                             };

//   ---     ---     ---     ---     ---

void BYTESTEP(void)                         {

    rd_cbyte      += rd_size;

    if(!(rd_cbyte%UNITSZ)) {
        rd_result += rd_step;
        rd_lhand   = rd_result;
        rd_value   = rd_lhand+rd_step;

        CLMEM2(rd_value, rd_size);

        lngptr    += rd_step;
        rd_cbyte   = 0;

    };                                                                                      };

//   ---     ---     ---     ---     ---

void RSTSEC(void)                           {

    sec_beg.base  = lngptr;
    sec_beg.cbyte = rd_cbyte;

    sec_cur.base  = lngptr;
    sec_cur.cbyte = rd_cbyte;

    sec_end.base  = ((rd_elems*rd_size)/UNITSZ)-1;
    sec_end.cbyte = UNITSZ-rd_size;

    rd_rawv++;                                                                              };

void RSTPTRS(void)                          {

    rd_result = ((MEMUNIT*) memlng->buff)+lngptr;
    rd_lhand  = rd_result;
    rd_value  = rd_lhand+rd_step;

    CLMEM2(rd_value, rd_size);
    CLMEM2(rd_lhand, rd_elems*rd_size);                                                     };

//   ---     ---     ---     ---     ---

void MAFETCH(MEMUNIT* r, MEMUNIT* v)        {

    MEMUNIT* addr = (MEMUNIT*) mammi->vaddr;

    uint size     =  mammi->vtype&0xFF;
    uint mag      = (mammi->vtype&0xFF00)>>8;

    uint elems    = GTUNITCNT(size, mag);

    uint units    = (elems*size)/UNITSZ;
    uint cbyte    = ((MEMUNIT)(*v))*size;
    uint cunit    = 0;

    uint step     = size/UNITSZ;

    if(!step) {
        step      = 1;

    };

//   ---     ---     ---     ---     ---

    cunit         = cbyte  / UNITSZ;
    cbyte        -= cunit  * UNITSZ;

    addr         += FETMASK(units, cunit);

//   ---     ---     ---     ---     ---

    (*r) = ((*addr) & (SIZMASK(size)<<(cbyte*8))) >> (cbyte*8);

    rd_flags&=~OP_AT;
    mammi->state&=~MAMMIT_SF_PFET;                                                          };

//   ---     ---     ---     ---     ---

void CHKMEMLAY(void)                        {

    uint      loc = 0;
    uintptr_t ptr = mammi->jmpt[loc];

    while(ptr && (loc < mammi->jmpt_i)) {

        ptr          = mammi->jmpt[loc];
        if(*((MEMUNIT*) ptr)==FREE_BLOCK) { break; };

        CNTX*     cx = (CNTX*) ptr;
        if(!cx->elems) { break; }

        uchar* name  = mammi->jmpt_h[loc].id.key;
        for(int x=0;x<strlen(name);x++) {
            if(name[x]>=0x61 && name[x]<=0x7B) {
                name[x]-=0x20;

            };
        };

        CALOUT(E, "\n0x%" PRIXPTR " BLOCK_START Sx%016"  PRIX64 " %s \n\n"           ,
                                                  cx, cx->state, name                );

        CALOUT(E, "0x%"   PRIXPTR " ELEMS\t%u\n", &(cx->elems), cx->elems            );
        CALOUT(E, "0x%"   PRIXPTR " USIZE\t%u\n", &(cx->size ), cx->size             );

        for(uint x=0; x<cx->elems; x++) { loc++;

            ptr           = mammi->jmpt[loc];
            if(*((MEMUNIT*) ptr)==FREE_BLOCK) { break; };

            name  = mammi->jmpt_h[loc].id.key;
            int y=0;
            for(;y<strlen(name);y++) {
                if(name[y]>=0x61 && name[y]<=0x7B) {
                    name[y]-=0x20;

                };
            };

            MEMUNIT* data = (MEMUNIT*) ptr;

                                            // unpack and print addr
            TPADDR                          (0x06, GTARRSZ(loc)                             );
            if(y<4) {
            CALOUT                          (E, "\n0x%" PRIXPTR " %s\t\t0x", ptr, name      );

            } else {
            CALOUT                          (E, "\n0x%" PRIXPTR " %s\t0x", ptr, name        );
            };

            uint px=43;
            for(uint y=0; y<rd_units; y++) {

                CALOUT                      (E, "\r\e[%uC", px                              );
                CALOUT                      (E, "%016" PRIX64 " ", data[y]                  );
                px=26;

                if(!((y+1)%2) && ((y+1)<rd_units)) {
                    CALOUT(E, "\r\e[59C\n0x%" PRIXPTR "\t\t0x", data+y+1);
                    px=43;

                };
            };
        }; loc++; CALOUT(E, "\n");
    };

//   ---     ---     ---     ---     ---

    END: CALOUT(E, "\n\nLNAMES at {%u/%u} capacity | %u units remaining\n",                 \
                   mammi->lvaltop, NAMESZ, NAMESZ - mammi->lvaltop        );                };

//   ---     ---     ---     ---     ---

void CALCUS_COLLAPSE(void)                  {

    switch(rd_cast) {

        case 0x03:
        case 0x04:
        case 0x05:
        case 0x06: {
            MEMUNIT* r = rd_lhand;
            MEMUNIT* v = rd_value;

            if(!(rd_flags&OP_AT)) {
                *v=(*v)<<(rd_cbyte*8);

            }; CALCUS_OPSWITCH;
        }

//   ---     ---     ---     ---     ---

        default: {
            float* r = (float*) rd_lhand;
            float* v = (float*) rd_value;
            CALCUS_OPSWITCH;

        };
    };                                                                                      };

//   ---     ---     ---     ---     ---

uint POPOPS(void)                           {

    uint ctok_cnt = 0;

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

        case 0x3F: MAMMIT_LVLB_NXT;
            rd_flags  = mammi->lvlb_stack[mammi->lvlb-1]&OP_MINUS;
            mammi->lvlb_stack[mammi->lvlb-1] &=~OP_MINUS;
            rd_flags |= OP_QUEST;

            goto POP_OPSTOP;

        case 0x7E: MAMMIT_LVLB_NXT;
            rd_flags  = mammi->lvlb_stack[mammi->lvlb-1]&OP_MINUS;
            mammi->lvlb_stack[mammi->lvlb-1] &=~OP_MINUS;
            rd_flags |= OP_TILDE;

            goto POP_OPSTOP;

//   ---     ---     ---     ---     ---

        case 0x23:
            rd_flags |= OP_KUSH;
            MAMMIT_LVLB_NXT;

            goto POP_OPSTOP;

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

        case 0x5B:
            rd_flags |= OP_BSUBS;
            MAMMIT_LVLB_NXT;

            goto POP_OPSTOP;

//   ---     ---     ---     ---     ---

        case 0x40:

            if(!(mammi->state&MAMMIT_SF_PFET)) {
                /*CALOUT(E, "Using '@' operator without fetch-from\n");
                return 0;*/

                goto POP_OPSTOP;

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
            if(rd_ctok) {
                rd_ctok->lops+=((MEMUNIT) (*rd_rawv))<<(ctok_cnt*8);
                ctok_cnt++;

            };

            rd_rawv++; goto TOP;

    };

//   ---     ---     ---     ---     ---

    ctok_cnt = 0;
    uint len = strlen(rd_rawv);

    if(!len) { goto END; }

    POP_TERMINATORS:                        // same as oppies, but at end of token
    switch(rd_rawv[len-1]) {

        default: break;

        case 0x5D:
            rd_flags |= OP_ESUBS;
            MAMMIT_LVLB_NXT;

            goto POP_TESTOP;

        case 0x29:

            if(mammi->lvlb) {
                MAMMIT_LVLB_PRV;            // lonely parens >;

            }; mammi->state &=~MAMMIT_SF_PSEC;

        POP_TESTOP:

            if(rd_ctok) {
                rd_ctok->rops+=((MEMUNIT) (*rd_rawv))<<(ctok_cnt*8);
                ctok_cnt++;

            };

            rd_rawv[len-1]=0x00;
            len--; if(!len) { break; }
            goto POP_TERMINATORS;

    }; END: return len;                                                                     };

//   ---     ---     ---     ---     ---

void JMPT_INSERT(void*  x   ,
                 uint   size,
                 uchar* meta,
                 uchar* name)               {

    LABEL* l = (mammi->jmpt_h+mammi->jmpt_i);
    l->id    = IDNEW(meta, name);

    l->loc   = mammi->jmpt_i;
    l->p_loc = 0;

//   ---     ---     ---     ---     ---    find parent

    if(cur_cntx && mammi->jmpt_i) {
        uintptr_t paddr=(uintptr_t) cur_cntx;
        for(int y=(int) mammi->jmpt_i;
            y > -1; y--              ) {

            LABEL* pl = (mammi->jmpt_h+y);
            if(mammi->jmpt[pl->loc]==paddr) {
                l->p_loc = pl->loc; break;

            };
        };
    };

//   ---     ---     ---     ---     ---    insertion

    HASHSET(LNAMES_HASH, byref(l->id));

    mammi->jmpt[mammi->jmpt_i+0]=(uintptr_t) x;
    mammi->jmpt[mammi->jmpt_i+1]=(uintptr_t) (x+(size*UNITSZ));

    mammi->jmpt_i++;                                                                        };

//   ---     ---     ---     ---     ---