// ---   *   ---   *   ---
// JOJ
// Do it all over again
//
// LIBRE SOFTWARE
// Licensed under GNU GPL3
// be a bro and inherit
//
// CONTRIBUTORS
// lyeb,

// ---   *   ---   *   ---
// deps

  #include "ff/PNG_Wrap.hpp"

  #include "ff/Zwrap.hpp"
  #include "ff/JOJ.hpp"

// ---   *   ---   *   ---
// png to joj

JOJ::JOJ(
  std::string fpath,
  std::string src_path,

  const char* comp

)

:Bin() {

  this->defopen<JOJ::Header>(
    fpath,
    Bin::WRITE|Bin::TRUNC

  );

// ---   *   ---   *   ---

  m_enc            = ENCDEF;

  m_hed.img_sz     = 0;
  m_next_img       = 0;

  m_src_path       = src_path;
  m_next_type      = 0;

// ---   *   ---   *   ---
// count image components

  for(int i=0;i<4;i++) {

    if(comp[i]==0x7F) {break;};

    m_hed.img_comp[i]=comp[i];
    m_hed.img_comp_cnt++;

  };

};

// ---   *   ---   *   ---
// ^open

JOJ::JOJ(
  std::string fpath

)

:Bin() {

  this->defopen<JOJ::Header>(
    fpath,
    Bin::READ

  );

// ---   *   ---   *   ---

  m_enc=ENCDEF;

  // native format
  Mem<JOJ::Pixel> pixels(m_hed.img_sz_sq);
  m_pixels.copy(pixels);

  // ^to png/glbuff
  Mem<float> raw(m_hed.img_sz_sq<<2);
  m_raw.copy(raw);

  m_tiles.nit(
    JOJ::STD_TILE_SZ,

    m_hed.img_sz,
    m_hed.img_sz_sq

  );

};

// ---   *   ---   *   ---
// destroy

JOJ::~JOJ(void) {

  if(m_src_path=="NON") {
    this->rmdump();

  };

};

// ---   *   ---   *   ---
// gets generic dump id

std::string JOJ::get_dump_f(int idex) {

  std::string path=

    m_fpath
  + "_dump"
  + std::to_string(idex)
  ;

  return path;

};

// ---   *   ---   *   ---
// save/load to/from dump

void JOJ::swap_to(int idex,char mode) {

  std::string path = this->get_dump_f(idex);
  void*       src  = (void*) m_tiles.get(0,0);

  Bin out(path,mode);

  uint64_t sz=
    m_hed.img_sz_sq
  * sizeof(JOJ::Pixel)
  ;

// ---   *   ---   *   ---
// retrieve

  if(mode==Bin::READ) {

    Mem<JOJ::Tile_Desc_Packed> m(
      m_tiles.cnt_sq

    );

    out.read(&m[0],m.bytesz());
    m_tiles.from_buff(m);

    out.read(src,sz);

// ---   *   ---   *   ---
// store

  } else {

    Mem<JOJ::Tile_Desc_Packed> m=
      m_tiles.to_buff();

    out.write(&m[0],m.bytesz());
    out.write(src,sz);

  };

};

// ---   *   ---   *   ---
// get NxN chunk

void JOJ::get_tile(uint64_t offset) {

  JOJ::Pixel* dst=&m_tiles.data[0]+offset;
  JOJ::Pixel* src=&m_pixels[0];

// ---   *   ---   *   ---

  for(uint16_t y=0;y<m_tiles.sz;y++) {

    uint64_t idex=sq_idex(
      m_tiles.x,
      y+m_tiles.y,

      m_hed.img_sz

    );

// ---   *   ---   *   ---

    memcpy(

      dst,
      src+idex,

      m_tiles.sz
    * sizeof(JOJ::Pixel)

    );

    dst+=m_tiles.sz;

  };

};

// ---   *   ---   *   ---
// ^whole image

void JOJ::to_tiles(void) {

  m_tiles.x=0;
  m_tiles.y=0;

  uint64_t offset=0;

  for(uint16_t y=0;y<m_tiles.cnt;y++) {
    for(uint16_t x=0;x<m_tiles.cnt;x++) {

      this->get_tile(offset);
      offset+=m_tiles.sz_sq;

      m_tiles.x+=m_tiles.sz;

    };

    m_tiles.x=0;
    m_tiles.y+=m_tiles.sz;

  };

};

// ---   *   ---   *   ---
// set NxN chunk

void JOJ::set_tile(uint64_t offset) {

  JOJ::Pixel* src=&m_tiles.data[0]+offset;
  JOJ::Pixel* dst=&m_pixels[0];

// ---   *   ---   *   ---

  for(uint16_t y=0;y<m_tiles.sz;y++) {

    uint64_t idex=sq_idex(
      m_tiles.x,
      y+m_tiles.y,

      m_hed.img_sz

    );

// ---   *   ---   *   ---

    memcpy(

      dst+idex,
      src,

      m_tiles.sz
    * sizeof(JOJ::Pixel)

    );

    src+=m_tiles.sz;

  };

};

// ---   *   ---   *   ---
// ^copies tiles array to pixel buffer

void JOJ::from_tiles(void) {

  m_tiles.x=0;
  m_tiles.y=0;

  uint64_t offset=0;

  for(uint16_t y=0;y<m_tiles.cnt;y++) {
    for(uint16_t x=0;x<m_tiles.cnt;x++) {

      this->set_tile(offset);
      offset+=m_tiles.sz_sq;

      m_tiles.x+=m_tiles.sz;

    };

    m_tiles.x=0;
    m_tiles.y+=m_tiles.sz;

  };

};

// ---   *   ---   *   ---
// tights up the buffs

void JOJ::pack(void) {

  // run encoder on list of images
  // dumps resulting buff for each
  for(

    int i=0;

    i<m_hed.img_cnt*m_hed.img_comp_cnt;
    i++

  ) {

    this->read_next_img();

    this->encoder(Frac::ENCODE);

    this->to_tiles();
    m_tiles.pack();

    this->swap_to(i,Bin::NEW);

  };

  // ^joins dumps into single file
  this->write();

};

// ---   *   ---   *   ---
// dump to disk

void JOJ::write(void) {

  this->write_header(&m_hed);

  for(

    int i=0;

    i<m_hed.img_cnt*m_hed.img_comp_cnt;
    i++

  ) {

    std::string path=this->get_dump_f(i);

    Bin src(path,Bin::READ);

    src.transfer(this);
    src.nuke();

  };

};

// ---   *   ---   *   ---
// ^retrieve

void JOJ::unpack(void) {

  uint64_t sz=

    m_tiles.cnt_sq
  * sizeof(JOJ::Tile_Desc_Packed)

  + m_hed.img_sz_sq
  * sizeof(JOJ::Pixel)

  ;

  for(

    int i=0;

    i<m_hed.img_cnt*m_hed.img_comp_cnt;
    i++

  ) {

    std::string path=this->get_dump_f(i);

    Bin dst(path,Bin::NEW);
    this->transfer(&dst,sz);

  };

  m_src_path="NON";

};

// ---   *   ---   *   ---
// ^get rid of retrieved

void JOJ::rmdump(void) {

  for(

    int i=0;

    i<m_hed.img_cnt*m_hed.img_comp_cnt;
    i++

  ) {

    std::string path=this->get_dump_f(i);

    Bin dst(path,Bin::READ);
    dst.nuke();

  };

};

// ---   *   ---   *   ---
// to avoid having the switch pasted everywhere

JOJ::SubEncoding JOJ::read_mode(char type) {

  JOJ::SubEncoding out;

  switch(type) {

  case JOJ::NVEC:
    out=m_enc.normal;
    Frac::Rounding_Mode=Frac::NVEC;

    break;

  case JOJ::YAUV:
    out=m_enc.color;
    Frac::Rounding_Mode=Frac::CORD;

    break;

  case JOJ::ORME:
    out=m_enc.shade;
    Frac::Rounding_Mode=Frac::CORD;

    break;

  // TODO: throw unrecognized type
  default:
    ;

  };

  return out;

};

// ---   *   ---   *   ---
// does/undoes frac'ing of floats

void JOJ::encoder(bool mode) {

  JOJ::Pixel* buff=&m_pixels[0];

// ---   *   ---   *   ---
// transform to and from compressed floats

  struct Frac::Bat<char> batch={

    .m_bytes  = buff[0].as_ptr(),
    .m_floats = &m_raw[0],
    .m_sz     = m_hed.img_sz_sq*4,

    .m_enc    = (char*) m_c_enc.values,
    .m_cnt    = (int*) m_c_enc.cnt,

    .m_mode   = mode,

  };

  batch.encoder();

};

// ---   *   ---   *   ---
// minimal sanity check

void JOJ::chk_img_sz(

  std::string fpath,
  uint16_t    sz

) {

  // get mem
  if(!m_hed.img_sz) {

    m_hed.img_sz    = sz;
    m_hed.img_sz_sq = sz*sz;

    if(!m_hed.atlas_sz) {
      this->atlas_sz(sz);

    };

    // for extracting image files
    Mem<float> raw(m_hed.img_sz_sq<<2);
    m_raw.copy(raw);

    // ^converts into integer format
    Mem<JOJ::Pixel> pixels(m_hed.img_sz_sq);
    m_pixels.copy(pixels);

    m_tiles.nit(
      JOJ::STD_TILE_SZ,

      m_hed.img_sz,
      m_hed.img_sz_sq

    );

// ---   *   ---   *   ---
// throw bad size

  } else if(sz*sz!=m_hed.img_sz_sq) {

    printf(
      "Image <%s> has size different "
      "from previous in queue; *.joj "
      "requires same-size images\n",

      fpath.c_str()

    );

    exit(1);

  };

};

// ---   *   ---   *   ---
// handles color conversions

void JOJ::color(float* pixel,bool mode) {

  if(mode==Frac::ENCODE) {

    if(m_c_enc==m_enc.color) {
      rgba2yauv(pixel);

    };

  } else {

    if(m_c_enc==m_enc.color) {
      yauv2rgba(pixel);

    } else if(m_c_enc==m_enc.normal) {
      fnorm(pixel);

    };

  };

};

// ---   *   ---   *   ---
// gets raw yauv buffer from png

void JOJ::from_png(
  std::string name,
  char* type

) {

  std::string fpath=
    m_src_path

  + "/"
  + name

  + std::string(type)
  + ".png"

  ;

  // open image
  PNG im(fpath);

  // run checks
  this->chk_img_sz(fpath,im.get_sz());

  auto buff=im.read();

  // convert
  this->img_cpy_rgba2yauv(
    &m_raw[0],
    &buff[0],

    m_hed.img_sz,
    255.0f

  );

};

// ---   *   ---   *   ---
// fill out internal buffer
// give back ptr to buff

float* JOJ::read_pixels(int idex) {

  char type=m_hed.img_comp[
    idex%m_hed.img_comp_cnt

  ];

  m_c_enc=this->read_mode(type);

  // decode joj
  this->swap_to(idex,Bin::READ);

//  m_tiles.unpack();
  this->from_tiles();

  this->encoder(Frac::DECODE);

  return &m_raw[0];

};

// ---   *   ---   *   ---
// from joj format to float

Mem<float> JOJ::to_buff(

  int   idex,

  float mult,
  bool  atlas

) {

  uint64_t sz_sq=this->on_atlas_sz_sq(atlas)<<2;

  Mem<float> out(sz_sq);

  float* pixels = this->read_pixels(idex);

  this->img_cpy_yauv2rgba(
    &out[0],
    pixels,

    this->on_atlas_sz(atlas),
    mult

  );

  return Mem<float>(out);

};

// ---   *   ---   *   ---
// from joj format to png

void JOJ::to_png(

  int         idex,
  std::string name,

  bool        atlas

) {

  uint64_t sz    = this->on_atlas_sz(atlas);
  uint64_t sz_sq = this->on_atlas_sz_sq(atlas);

  // make image
  PNG im(name,sz);

  float* pixels=this->read_pixels(idex);
  Mem<uint8_t> buff(sz_sq<<2);

  this->img_cpy_yauv2rgba(
    &buff[0],
    pixels,

    sz,
    255.0f

  );

  // dump it out
  im.write(buff);

};

// ---   *   ---   *   ---
// get next "queued" read

void JOJ::read_next_img(void) {

  uint8_t comp_idex=
    m_hed.img_comp[m_next_type++];

  m_c_enc=this->read_mode(comp_idex);

  this->from_png(
    m_img_names[m_next_img],
    (char*) IMG_TYPES[comp_idex]

  );

  // move to next image set
  if(m_next_type==m_hed.img_comp_cnt) {
    m_next_type=0;
    m_next_img++;

  };

};

// ---   *   ---   *   ---
