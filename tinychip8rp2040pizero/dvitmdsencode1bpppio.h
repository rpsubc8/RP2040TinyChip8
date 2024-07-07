#ifndef _TMDS_ENCODE_1BPP_PIO_H
 #define _TMDS_ENCODE_1BPP_PIO_H


 #include "gbConfig.h" 
 #ifdef use_lib_hdmi 
  // -------------------------------------------------- //
  // This file is autogenerated by pioasm; do not edit! //
  // -------------------------------------------------- //

  #pragma once

  #if !PICO_NO_HARDWARE
   #include "hardware/pio.h"
  #endif

  // ---------------- //
  // tmds_encode_1bpp //
  // ---------------- //

  #define tmds_encode_1bpp_wrap_target 0
  #define tmds_encode_1bpp_wrap 9

  static const uint16_t tmds_encode_1bpp_program_instructions[] = {
            //     .wrap_target
    0x6021, //  0: out    x, 1                       
    0xa049, //  1: mov    y, !x                      
    0x4041, //  2: in     y, 1                       
    0x4021, //  3: in     x, 1                       
    0xa02b, //  4: mov    x, !null                   
    0x4028, //  5: in     x, 8                       
    0x6021, //  6: out    x, 1                       
    0xa049, //  7: mov    y, !x                      
    0x4041, //  8: in     y, 1                       
    0x402d, //  9: in     x, 13                      
            //     .wrap
  };

  #if !PICO_NO_HARDWARE
   static const struct pio_program tmds_encode_1bpp_program = {
    .instructions = tmds_encode_1bpp_program_instructions,
    .length = 10,
    .origin = -1,
   };

   static inline pio_sm_config tmds_encode_1bpp_program_get_default_config(uint offset) {
    pio_sm_config c = pio_get_default_sm_config();
    sm_config_set_wrap(&c, offset + tmds_encode_1bpp_wrap_target, offset + tmds_encode_1bpp_wrap);
    return c;
   }

   static inline void tmds_encode_1bpp_init(PIO pio, uint sm) {
    uint offset = pio_add_program(pio, &tmds_encode_1bpp_program);
    pio_sm_config c = tmds_encode_1bpp_program_get_default_config(offset);
    sm_config_set_out_shift(&c, true, true, 32);
    sm_config_set_in_shift(&c, true, true, 24);
    pio_sm_init(pio, sm, offset, &c);
    pio_sm_set_enabled(pio, sm, true);
   }

  #endif

#endif
