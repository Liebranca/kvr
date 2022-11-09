// ---   *   ---   *   ---
// paste-in header
// part of the JOJ class

#ifndef __24_JOJ_TILES_H__
#define __24_JOJ_TILES_H__

// ---   *   ---   *   ---
// xform/fetch helper

private:

  cx8 STD_TILE_SZ=0x10;

typedef struct {
  uint16_t src_x;
  uint16_t src_y;
  uint16_t dst_x;
  uint16_t dst_y;

} Tile_Fetch;

// ---   *   ---   *   ---
// comparison/reconstruction helper

typedef struct {

  // fetch-from
  uint16_t x;
  uint16_t y;

  // fetch-to
  uint16_t dx;
  uint16_t dy;

  uint8_t  rotated  : 2;
  uint8_t  mirrored : 2;
  uint8_t  cleared  : 3;

  std::vector<uint64_t> used_by;

} Tile_Desc;

typedef struct {

  uint16_t x;
  uint16_t y;

  uint8_t  rotated  : 2;
  uint8_t  mirrored : 2;
  uint8_t  cleared  : 3;

} Tile_Desc_Packed;

// ---   *   ---   *   ---
// another comparison helper

typedef struct {

  JOJ::Tile_Desc& td;

  JOJ::Pixel*     a;
  JOJ::Pixel*     b;

  uint16_t        x;
  uint16_t        y;

} Tile_Cmp;

// ---   *   ---   *   ---

public:

typedef struct FwdTiles {

  uint16_t x;
  uint16_t y;

  uint16_t sz;
  uint16_t sz_i;

  uint16_t cnt;

  uint64_t sz_sq;
  uint64_t cnt_sq;

  Mem<JOJ::Pixel> data;
  std::vector<JOJ::Tile_Desc> tab;

// ---   *   ---   *   ---
// default constructor doesn't want to work

  void nit(

    uint16_t sz,

    uint64_t img_sz,
    uint64_t img_sz_sq

  ) {

    this->x       = 0;
    this->y       = 0;

    this->sz      = sz;
    this->sz_sq   = sz*sz;
    this->sz_i    = sz-1;

    this->cnt     = img_sz/sz;
    this->cnt_sq  = this->cnt*this->cnt;

    Mem<JOJ::Pixel> data(img_sz_sq);
    this->data.copy(data);

    this->tab.resize(this->cnt_sq);

  };

// ---   *   ---   *   ---

  // write tiles info to a buffer
  Mem<JOJ::Tile_Desc_Packed> to_buff(void);

  // ^inverse
  void from_buff(
    Mem<JOJ::Tile_Desc_Packed>& src

  );

  // offsets the next table read
  void fetch_offset(
    uint64_t prev_tiles,
    uint16_t atlas_cnt

  );

  enum {
    SOLID       = 0b000,
    CLEAR_NAT   = 0b001,
    CLEAR_FETCH = 0b010,
    FAKE_SOLID  = 0b100,

  };

  // discard contents of tile
  void clear(
    uint16_t x,
    uint16_t y,

    uint8_t  flag=JOJ::Tiles::CLEAR_FETCH

  );

  // true if tile is blank
  bool is_clear(JOJ::Pixel* a);

  // number of used tiles
  uint64_t solid_cnt(void);

  // true if tiles are identical
  bool cmp(
    JOJ::Pixel* a,
    JOJ::Pixel* b

  );

  // initializes tile descriptor
  JOJ::Tile_Desc& nit_desc(
    uint16_t x,
    uint16_t y

  );

  // get handle to compared tile
  JOJ::Pixel* match_get_ref(
    uint16_t x,
    uint16_t y

  );

  // matches two tiles
  bool match_cmp(JOJ::Tile_Cmp& mat);

  enum {

    XFORM_APPLY,
    XFORM_SKIP

  };

  // attempt matching with previous tiles
  void match(
    uint16_t x,
    uint16_t y,

    bool     skip=false

  );

  enum {

    MIRROR_NONE,

    MIRROR_X,
    MIRROR_Y,
    MIRROR_XY

  };

  enum {

    ROT_0,

    ROT_90,
    ROT_180,
    ROT_240

  };

  // ^attempts mirroring
  bool match_mirror(
    JOJ::Tile_Desc& td

  );

  // ^attemps rotation
  bool match_rotate(
    JOJ::Tile_Desc& td

  );

  // walks back mirrors in descriptor
  void undo_mirror(
    JOJ::Tile_Desc& td

  );

  // walks back rotations in descriptor
  void undo_rotation(
    JOJ::Tile_Desc& td

  );

  // ^undoes failed attempts
  inline void match_undo(
    JOJ::Tile_Desc& td

  ) {

    this->undo_rotation(td);
    this->undo_mirror(td);

    td.rotated  = ROT_0;
    td.mirrored = MIRROR_NONE;

  };

  // rotates from descriptor
  void apply_rotation(
    JOJ::Tile_Desc& td

  );

  // mirrors from descriptor
  void apply_mirror(
    JOJ::Tile_Desc& td

  );

  // send duplicate of tile to buffer
  Mem<JOJ::Pixel> copy(
    JOJ::Pixel* pixel

  );

  // replace tile with another
  void mov(
    JOJ::Pixel* dst,
    JOJ::Pixel* src

  );

  // move tile to first empty one
  void reloc(JOJ::Tile_Desc& td);

  // ^runs reloc on whole image
  void reloc_all(void);

  // builds table from image
  void pack(bool no_match);

  // build original image from table
  void unpack(
    JOJ::FwdTiles& atlas,
    bool           clear_nat=false

  );

  // ^same, copy self
  void unpack(bool clear_nat=false);

  // get packed tiles for whole image
  void get_img_desc(
    JOJ::FwdTiles&         atlas,
    std::vector<uint64_t>& desc

  );

  // get tile number
  inline uint64_t tile_idex(
    uint16_t x,
    uint16_t y

  ) {return sq_idex(x,y,this->cnt);};

  // get index of tile in buffer
  inline uint64_t real_idex(
    uint16_t x,
    uint16_t y

  ) {

    return sq_idex(

      x*this->sz_sq,
      y*this->cnt,

      this->sz_sq

    );

  };

  // return ptr to tile at [x,y]
  inline JOJ::Pixel* get(
    uint16_t x,
    uint16_t y

  ) {

    JOJ::Pixel* out=&this->data[0];
    return out+this->real_idex(x,y);

  };

  // return ptr to tile N
  inline JOJ::Pixel* get(uint64_t i) {
    return &this->data[0]+i*this->sz_sq;

  };

// ---   *   ---   *   ---
// under the hood

private:

  enum {

    XFORM_ROR,
    XFORM_ROL,

    XFORM_XMIR,
    XFORM_YMIR

  };

  // generic
  void xform(
    uint16_t off_x,
    uint16_t off_y,

    uint32_t fn_idex

  );

  // 90 degrees, clockwise rotation
  void xform_ror(Tile_Fetch& co) {
    co.dst_x=this->sz_i-co.src_y;
    co.dst_y=co.src_x;

  };

  // ^anti-clockwise
  void xform_rol(Tile_Fetch& co) {
    co.dst_x=co.src_y;
    co.dst_y=this->sz_i-co.src_x;

  };

  // invert on X
  void xform_xmir(Tile_Fetch& co) {
    co.dst_x=this->sz_i-co.src_x;
    co.dst_y=co.src_y;

  };

  // ^on Y
  void xform_ymir(Tile_Fetch& co) {
    co.dst_x=co.src_x;
    co.dst_y=this->sz_i-co.src_y;

  };

// ---   *   ---   *   ---
// ^entry points

public:

  inline void ror(uint16_t x,uint16_t y) {
    this->xform(x,y,XFORM_ROR);

  };

  inline void rol(uint16_t x,uint16_t y) {
    this->xform(x,y,XFORM_ROL);

  };

  inline void xmir(uint16_t x,uint16_t y) {
    this->xform(x,y,XFORM_XMIR);

  };

  inline void ymir(uint16_t x,uint16_t y) {
    this->xform(x,y,XFORM_YMIR);

  };

// ---   *   ---   *   ---

} Tiles;

// ---   *   ---   *   ---

#endif // __24_JOJ_TILES_H__
