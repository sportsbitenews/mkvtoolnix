/*
   mkvmerge -- utility for splicing together matroska files
   from component media subtypes

   Distributed under the GPL v2
   see the file COPYING for details
   or visit http://www.gnu.org/copyleft/gpl.html

   DTS decoder & parser (private data)

   Written by Moritz Bunkus <moritz@bunkus.org>.
*/

#ifndef MTX_COMMON_PRIVATE_DTS_PARSER_H
#define MTX_COMMON_PRIVATE_DTS_PARSER_H

#include "common/byte_buffer.h"
#include "common/dts.h"
#include "common/dts_parser.h"

namespace mtx { namespace dts {

struct parser_c::impl_t {
public:
  debugging_option_c debug{"dts_parser"};

  bool swap_bytes{}, pack_14_16{};
  byte_buffer_c decode_buffer, swap_remainder, pack_remainder;
  header_t first_header;

public:
  virtual ~impl_t();

  void reset();
};

}}

#endif // MTX_COMMON_PRIVATE_DTS_PARSER_H
