#ifndef GB_GLOBALS_H
 #define GB_GLOBALS_H

 #include "gbConfig.h"

 extern volatile unsigned char gb_keymap[32];
 //extern char gb_texto[10][40];

 extern unsigned char gb_use_tiled; //Tiled pixels
 extern unsigned char gb_use_tveffect; //TV Effect snow OSD
 extern unsigned char gb_use_silence;
 extern unsigned char gb_sound_pulse_duration_cont; //duration pulse hz 500 hz, 250, 166,125
 extern unsigned int gb_ramfree_ini;
 extern unsigned int gb_ramfree_setupEnd;
 
#endif
