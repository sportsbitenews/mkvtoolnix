/*
  mkvmerge -- utility for splicing together matroska files
      from component media subtypes

  aac_common.cpp

  Written by Moritz Bunkus <moritz@bunkus.org>

  Distributed under the GPL
  see the file COPYING for details
  or visit http://www.gnu.org/copyleft/gpl.html
*/

/*!
    \file
    \version \$Id: aac_common.cpp,v 1.5 2003/05/19 20:51:12 mosu Exp $
    \brief helper function for AAC data
    \author Moritz Bunkus <moritz@bunkus.org>
*/

#include <stdio.h>
#include <string.h>

#include "common.h"
#include "aac_common.h"

static const int sample_rates[16] = {96000, 88200, 64000, 48000, 44100, 32000,
                                     24000, 22050, 16000, 12000, 11025,  8000,
                                     0, 0, 0, 0}; // filling

int parse_aac_adif_header(unsigned char *buf, int size,
                          aac_header_t *aac_header) {
  int i, k;
  bool b, eob;
  unsigned int bits;
  int nprogram_conf_e, bitstream_type, profile, sfreq_index;
  int nfront_c_e, nside_c_e, nback_c_e, nlfe_c_e;
  int nassoc_data_e, nvalid_cc_e, comment_field_bytes;
  int channels;
  bit_cursor_c bc(buf, size);

  bc.get_bits(32, bits);
  if (bits != FOURCC('A', 'D', 'I', 'F'))
    return 0;
  bc.get_bit(b);                // copyright_id_present
  if (b) {
    for (i = 0; i < 3; i++)
      eob = bc.get_bits(24, bits); // copyright_id
  }
  bc.get_bit(b);                // original_copy
  bc.get_bit(b);                // home
  bc.get_bits(1, bitstream_type);
  bc.get_bits(23, bits);        // bitrate
  bc.get_bits(4, nprogram_conf_e);
  for (i = 0; i <= nprogram_conf_e; i++) {
    channels = 0;
    if (bitstream_type == 0)
      bc.get_bits(20, bits);
    bc.get_bits(4, bits);       // element_instance_tag
    bc.get_bits(2, profile);
    bc.get_bits(4, sfreq_index);
    bc.get_bits(4, nfront_c_e);
    bc.get_bits(4, nside_c_e);
    bc.get_bits(4, nback_c_e);
    bc.get_bits(2, nlfe_c_e);
    bc.get_bits(3, nassoc_data_e);
    bc.get_bits(4, nvalid_cc_e);
    bc.get_bit(b);              // mono_mixdown_present
    if (b)
      bc.get_bits(4, bits);     // mono_mixdown_el_num
    bc.get_bit(b);              // stereo_mixdown_present
    if (b)
      bc.get_bits(4, bits);     // stereo_mixdown_el_num
    bc.get_bit(b);              // matrix_mixdown_idx_present
    if (b) {
      bc.get_bits(2, bits);     // matrix_mixdown_idx
      eob = bc.get_bits(1, bits); // pseudo_surround_enable
    }
    channels = nfront_c_e + nside_c_e + nback_c_e;
    for (k = 0; k < (nfront_c_e + nside_c_e + nback_c_e); k++) {
      bc.get_bit(b);            // *_element_is_cpe
      if (b)
        channels++;
      bc.get_bits(4, bits);     // *_element_tag_select
    }
    channels += nlfe_c_e;
    for (k = 0; k < (nlfe_c_e + nassoc_data_e); k++)
      bc.get_bits(4, bits);     // *_element_tag_select
    for (k = 0; k < nvalid_cc_e; k++) {
      bc.get_bits(1, bits);     // cc_e_is_ind_sw
      bc.get_bits(4, bits);     // valid_cc_e_tag_select
    }
    bc.byte_align();
    eob = bc.get_bits(8, bits);
    for (k = 0; k < comment_field_bytes; k++)
      eob = bc.get_bits(8, bits);
  }

  if (eob)
    return 0;

  aac_header->sample_rate = sample_rates[sfreq_index];
  aac_header->id = 0;           // MPEG-4
  aac_header->profile = profile;
  aac_header->bytes = 0;
  aac_header->channels = channels > 6 ? 2 : channels;
  aac_header->bit_rate = 1024;
  aac_header->header_bit_size = bc.get_bit_position();
  aac_header->header_byte_size = (aac_header->header_bit_size + 7) / 8;

  return 1;
}

static int is_adts_header(unsigned char *buf, int size, int bpos,
                          aac_header_t *aac_header) {
  int id, profile, sfreq_index, channels, frame_length;
  bool eob, protection_absent, b;
  unsigned int bits;
  bit_cursor_c bc(buf, size);

  bc.get_bits(12, bits);
  if (bits != 0xfff)            // ADTS header
    return 0;

  bc.get_bits(1, id);           // ID: 0 = MPEG-4, 1 = MPEG-2
  bc.get_bits(2, bits);         // layer: = 0 !!
  if (bits != 0)
    return 0;
  bc.get_bit(protection_absent);
  bc.get_bits(2, profile);
  bc.get_bits(4, sfreq_index);
  bc.get_bit(b);                // private
  bc.get_bits(3, channels);
  bc.get_bit(b);                // original/copy
  bc.get_bit(b);                // home
  if (id == 0)
    bc.get_bits(2, bits);       // emphasis, MPEG-4 only
  bc.get_bit(b);                // copyright_id_bit
  bc.get_bit(b);                // copyright_id_start
  bc.get_bits(13, frame_length);
  bc.get_bits(11, bits);        // adts_buffer_fullness
  eob = bc.get_bits(2, bits);   // no_raw_blocks_in_frame
  if (!protection_absent)
    eob = bc.get_bits(16, bits);

  if (eob)
    return 0;

  aac_header->sample_rate = sample_rates[sfreq_index];
  aac_header->id = id;
  aac_header->profile = profile;
  aac_header->bytes = frame_length;
  aac_header->channels = channels > 6 ? 2 : channels;
  aac_header->bit_rate = 1024;
  if (id == 0)                // MPEG-4
    aac_header->header_bit_size = 58;
  else
    aac_header->header_bit_size = 56;
  if (!protection_absent)
    aac_header->header_bit_size += 16;
  aac_header->header_byte_size = (aac_header->header_bit_size + 7) / 8;
  aac_header->data_byte_size = aac_header->bytes -
    aac_header->header_bit_size / 8;

  return 1;
}

int find_aac_header(unsigned char *buf, int size, aac_header_t *aac_header) {
  int bpos;

  bpos = 0;
  while (bpos < size) {
    if (is_adts_header(buf, size, bpos, aac_header))
      return bpos;
    bpos++;
  }

  return -1;
}
