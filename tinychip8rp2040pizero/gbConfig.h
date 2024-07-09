#ifndef _GB_CONFIG_H
 #define _GB_CONFIG_H

 //Keyboard (select one option)
 //Keyboard GPIO external real PS2 connector [CLK(4) DATA(5)]
 #define use_lib_keyboard_ps2
 //Keyboard usb board (waveshare) with adapter PS2 to usb, usb to usbC [CLK(6)D+ DATA(7)D-] PIO-USB
 //#define use_lib_keyboard_ps2usb

 //GPIO sound
 #define SPEAKER_PIN 21

 //Solo 1 buffer 76800 bytes (no nextframe)
 #define use_lib_vga_one_buffer

 //Use hdmi waveshare rp2040 pizero (select one option)
 //#define use_lib_hdmi
 #define use_lib_vga

 //voltage overclock HDMI warning (select one option) 1.10v is default RP2040
 #define VREG_VSEL VREG_VOLTAGE_1_05
 //#define VREG_VSEL VREG_VOLTAGE_1_10
 //#define VREG_VSEL VREG_VOLTAGE_1_20
 //#define VREG_VSEL VREG_VOLTAGE_1_25

 //Video mode
 //#define use_lib_200x150
 #define use_lib_320x200


 //View logs serial
 //#define use_lib_log_serial

 //Read keyborad poll x millis
 #define gb_ms_keyboard 20 
 //Two Buttons Joystick press 500 ms for view OSD MENU(use_lib_not_use_ps2keyboard)
 #define gb_ms_gamepad_osd_menu 500
 //First button press joystick to view OSD MENU(use_lib_not_use_ps2keyboard)
 //ATARI_DB9_UP_ID ATARI_DB9_DOWN_ID ATARI_DB9_LEFT_ID ATARI_DB9_RIGHT_ID
 //ATARI_DB9_A_ID ATARI_DB9_B_ID
 #define gb_use_gamepad_osd_menu_button0 ATARI_DB9_UP_ID
 //Second button press joystick to view OSD MENU(use_lib_not_use_ps2keyboard)
 #define gb_use_gamepad_osd_menu_button1 ATARI_DB9_A_ID
 //Read sound poll x millis
 #define gb_ms_sound 1
 //Skip x frames
 #define gb_frame_crt_skip 0
 //Delay millis iteration emulate
 #define gb_delay_emulate_ms 0 





 //Keyboard
 #ifdef use_lib_keyboard_ps2
  #define KEYBOARD_CLK 4
  #define KEYBOARD_DATA 5
 #else
  #ifdef use_lib_keyboard_ps2usb
   #define KEYBOARD_CLK 6
   #define KEYBOARD_DATA 7
  #endif 
 #endif 


 #ifdef use_lib_200x150
  #define gb_add_offset_x 0
  #define gb_add_offset_y 0
  //#define gb_topeX 200
  //#define gb_topeY 150
  //#define gb_topeX_div4 50
 #else
  #ifdef use_lib_hdmi
   #define gb_add_offset_x 60 
   #define gb_add_offset_y 40
   //#define gb_topeX 320
   //#define gb_topeY 200
   //#define gb_topeX_div4 80
  #else
   #ifdef use_lib_vga
    //(640-256) DIV 2
    #define gb_add_offset_x 192 
    #define gb_add_offset_y 40
    //#define gb_topeX 320
    //#define gb_topeY 200
    //#define gb_topeX_div4 80   
   #endif
  #endif 
 #endif

 //Posicion Teclado virtual
 #define gb_pos_x_virtualKey 4
 #define gb_pos_y_virtualKey 4

#endif
