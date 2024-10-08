#ifndef _TMDS_TABLE_H
 #define _TMDS_TABLE_H

 #include "gbConfig.h"
 
 #ifdef use_lib_hdmi
 
  // Generated from tmds_table_gen.py
  //
  // This table converts a 6 bit data input into a pair of TMDS data symbols
  // with data content *almost* equal (1 LSB off) to input value left shifted by
  // two. The pairs of symbols have a net DC balance of 0.
  //
  // The two symbols are concatenated in the 20 LSBs of a data word, with the
  // first symbol in least-significant position.
  //
  // Note the declaration isn't included here, just the table body. This is in
  // case you want multiple copies of the table in different SRAMs (particularly
  // scratch X/Y).
  0x7fd00u,
  0x40dfcu,
  0x41df8u,
  0x7ed04u,
  0x43df0u,
  0x7cd0cu,
  0x7dd08u,
  0x42df4u,
  0x47de0u,
  0x78d1cu,
  0x79d18u,
  0x46de4u,
  0x7bd10u,
  0x44decu,
  0x45de8u,
  0xafa41u,
  0x4fdc0u,
  0x70d3cu,
  0x71d38u,
  0x4edc4u,
  0x73d30u,
  0x4cdccu,
  0x4ddc8u,
  0xa7a61u,
  0x77d20u,
  0x48ddcu,
  0x49dd8u,
  0xa3a71u,
  0x4bdd0u,
  0xa1a79u,
  0xa0a7du,
  0x9fa81u,
  0x5fd80u,
  0x60d7cu,
  0x61d78u,
  0x5ed84u,
  0x63d70u,
  0x5cd8cu,
  0x5dd88u,
  0xb7a21u,
  0x67d60u,
  0x58d9cu,
  0x59d98u,
  0xb3a31u,
  0x5bd90u,
  0xb1a39u,
  0xb0a3du,
  0x8fac1u,
  0x6fd40u,
  0x50dbcu,
  0x51db8u,
  0xbba11u,
  0x53db0u,
  0xb9a19u,
  0xb8a1du,
  0x87ae1u,
  0x57da0u,
  0xbda09u,
  0xbca0du,
  0x83af1u,
  0xbea05u,
  0x81af9u,
  0x80afdu,
  0xbfa01u,

 #endif

#endif
