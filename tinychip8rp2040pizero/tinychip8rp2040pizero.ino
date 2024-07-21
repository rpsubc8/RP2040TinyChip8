//Ackerman RP2040 chip8 VGA, HDMI, PS/2
//Spittie chip8 sdl x86 port ESP32 and RP2040 by ackerman
//Board Waveshare RP2040 pizero
//Arduino IDE 1.8.11
//VGA GPIO [18 (red) ,19 (green), 20 (blue), 16(HSync), 17 (VSync)]
//VGA 640x500  Hsync 33.3Khz   Vsync 63.5Hz
//HDMI
// sm_tmds [0, 1, 2]
// pins_tmds [26, 24, 22]
// pins_clk [28]
//PS2 keyboard GPIO [4 (CLK), 5 (Data)]
//USB2PS/2 PIO USB GPIO [6 (CLK), 7 (DATA)]
//HDMI library Waveshare (adafruit picodvi hdmi mod)
//VGA library (Hunter Adams and San Tarcisio mod)
//PS2 library (ps2kbdlib michalhol mod)
//Sound GPIO 21 (square polling)
//No overclock voltage HDMI
#include <Arduino.h>
#include "gbConfig.h"
#ifdef use_lib_hdmi
 #include "pico/stdlib.h"
 #include "dvi.h"
 #include "dviserialiser.h"
 #include "hardware/clocks.h"
 #include "hardware/vreg.h"
 #include "dvicommondvipinconfigs.h"
#else
 #ifdef use_lib_vga
  //Funciona GPIO 18(Rojo), GPIO 19(Verde), 20 (Azul)
  #include "vgagraphics.h"  // VGA graphics library
 #endif
#endif 

#include "dataFlash/gbrom.h"
#include "gbsdlfont8x8.h"

#include "PS2Kbd.h"

//Resetear
#define AIRCR_Register (*((volatile uint32_t*)(PPB_BASE + 0x0ED0C)))  

//Colores Indices en CHIP8
#define ID_COLOR_BLACK 0 //Negro
#define ID_COLOR_WHITE 0xFF //Blanco

//Teclado
//#define KEY_F1 0x05
//#define KEY_F2 0x06
//#define KEY_F3 0x04
//#define KEY_F4 0x0C
//#define KEY_F5 0x03
//#define KEY_F6 0x0B
//#define KEY_F7 0x83
//#define KEY_F8 0x0A
//#define KEY_F9 0x01
//#define KEY_F10 0x09
//#define KEY_F11 0x78
//#define KEY_F12 0x07
//#define KEY_ESC 0x76
//#define KEY_CURSOR_LEFT 0x6B
//#define KEY_CURSOR_DOWN 0x72
//#define KEY_CURSOR_RIGHT 0x74
//#define KEY_CURSOR_UP 0x75
//#define KEY_ALT_GR 0x11
//#define KEY_ENTER 0x5A
//#define KEY_HOME 0xE06C
//#define KEY_END 0xE069
//#define KEY_PAGE_UP 0xE07D
//#define KEY_PAGE_DOWN 0xE07A
//#define KEY_PAUSE 0xE11477E1F014E077
//#define KEY_BACKSPACE 0x66
//#define KEY_DELETE 0x71

//#define PS2_KC_KP0 0x70
//#define PS2_KC_KP_DOT 0x71

//#define PS2_KC_1 0X16
//#define PS2_KC_2 0X1E
//#define PS2_KC_3 0X26
//#define PS2_KC_4 0X25
//#define PS2_KC_Q 0X15
//#define PS2_KC_W 0X1D
//#define PS2_KC_E 0X24
//#define PS2_KC_R 0X2D
//#define PS2_KC_A 0X1C
//#define PS2_KC_S 0X1B
//#define PS2_KC_D 0X23
//#define PS2_KC_F 0X2B
//#define PS2_KC_Z 0X1A
//#define PS2_KC_X 0X22
//#define PS2_KC_C 0X21
//#define PS2_KC_V 0X2A

//unsigned char keymap[256];

//OSD identificadores
#define id_menu_main 0
#define id_menu_load_rom 1
#define id_menu_speed 2
#define id_menu_screen_adjust 3
#define id_menu_remap_keys 4
#define id_menu_keyboard_virtual 5
#define id_menu_sound 6
#define id_menu_reset 7
#define id_menu_help 8
#define id_menu_ram 9
#define id_menu_about 10


unsigned char gb_use_tiled=0; //Tiled pixels
unsigned char gb_use_tveffect=1; //efecto nieve tv OSD
unsigned char gb_use_silence=0;
unsigned char gb_sound_pulse_duration_cont=1; //500 hz duration pulse Hz
unsigned int gb_ramfree_ini=0;
unsigned int gb_ramfree_setupEnd=0;

unsigned char gb_show_osd_main_menu=0;
unsigned char gb_id_menu_cur=id_menu_main;

short int gb_osd_ventana_ini=0;
short int gb_osd_ventana_fin=15;

unsigned short int gb_vga_ancho= 320;
unsigned short int gb_vga_alto= 240;

//boolean checkAndCleanKey(uint8_t scancode) 
//{
// return false;
//}


//unsigned int tiempoHDMI_cur=0;
//unsigned int tiempoHDMI_before=0;
unsigned int gb_cont_fps=0;
unsigned int gb_fps_time_cur=0;
unsigned int gb_fps_time_ini=0;
long unsigned int gb_dump_time=0;

unsigned int gb_time_cur_key_virtual=0;
unsigned int gb_time_ini_key_virtual=0;


#ifdef use_lib_hdmi
 // DVDD 1.2V (1.1V seems ok too)
 //#define VREG_VSEL VREG_VOLTAGE_1_25
 //No hago overclock voltage
 //#define VREG_VSEL VREG_VOLTAGE_1_10
 //#define VREG_VSEL VREG_VOLTAGE_1_05
 #define FRAME_WIDTH 320
 #define FRAME_HEIGHT 240
 #define DVI_TIMING dvi_timing_640x480p_60hz

 struct dvi_inst dvi0;
#endif

//uint8_t framebuf[320*200];
//unsigned char __attribute__((aligned(4))) scanline_framebuffer[320*240*2];
unsigned char color=0;
//unsigned char aleatorio[4]={4,2,9,15};

#ifdef use_lib_hdmi
 unsigned char __attribute__((aligned(4))) oneline[320*2];
 unsigned char __attribute__((aligned(4))) onelineBlack[320*2];
 //unsigned char __attribute__((aligned(4))) oneline[320*240*2];

 //unsigned char __attribute__((aligned(4))) dibujo[320*16];
 //unsigned char hdmi_data_array[320*240];
 unsigned char * hdmi_data_array; //76800 bytes
#endif 



//CHIP8 fuentes
//Chars
const static unsigned char chip8_fontset[80] =
{ 
 0xF0,0x90,0x90,0x90,0xF0, //0
 0x20,0x60,0x20,0x20,0x70, //1
 0xF0,0x10,0xF0,0x80,0xF0, //2
 0xF0,0x10,0xF0,0x10,0xF0, //3
 0x90,0x90,0xF0,0x10,0x10, //4
 0xF0,0x80,0xF0,0x10,0xF0, //5
 0xF0,0x80,0xF0,0x90,0xF0, //6
 0xF0,0x10,0x20,0x40,0x40, //7
 0xF0,0x90,0xF0,0x90,0xF0, //8
 0xF0,0x90,0xF0,0x10,0xF0, //9
 0xF0,0x90,0xF0,0x90,0x90, //A
 0xE0,0x90,0xE0,0x90,0xE0, //B
 0xF0,0x80,0x80,0x80,0xF0, //C
 0xE0,0x90,0x90,0x90,0xE0, //D
 0xF0,0x80,0xF0,0x80,0xF0, //E
 0xF0,0x80,0xF0,0x80,0x80  //F
};

//CPU Stuff
static unsigned char V[16]; //uint8_t V[16];
static unsigned short int I; //uint16_t I;
static unsigned short int PC; //uint16_t PC;
static unsigned short int SP; //uint16_t SP;
static unsigned short int stack[16]; //uint16_t stack[16];
static unsigned char delay_t; //uint8_t delay_t;
static unsigned char sound_t; //uint8_t sound_t;


//Key
static unsigned char key[16]; //uint8_t key[16];

//Ram
static unsigned char ram[4096]; //uint8_t ram[4096];

// GFX
//static unsigned char gfx[64][32]; //uint8_t gfx[64][32];
#define max_gfx_row 64
#define max_gfx_col 64
//static unsigned char gfx[64][64]; //Para evitar over
static unsigned char gfx[max_gfx_row][max_gfx_col]; //Para evitar over
static unsigned char draw; //uint8_t draw;

static unsigned short int opcode; //uint16_t opcode;

unsigned char quit = 0;

unsigned char gb_auto_delay_cpu = 1; //auto delay
unsigned char gb_run_emulacion = 1; //Ejecuta la emulacion
unsigned char gb_current_ms_poll_sound = gb_ms_sound;
unsigned char gb_screen_xOffset=0;
static unsigned long gb_time_ini_espera=0;
static unsigned long gb_currentTime=0;
static unsigned long gb_sdl_time_sound_before=0;
static unsigned long gb_keyboardTime=0;
static unsigned long gb_time_ini_beep=0;
unsigned char gb_current_sel_rom=0;

//unsigned char gb_show_osd_main_menu=0;


unsigned char gb_show_key4x4=0;
unsigned char gb_show_key_virtual=0; //Muestra teclado virtual
unsigned char gb_cur_key_virtual=0; //Tecla inicial seleccionada
unsigned char gb_before_key_virtual=0;
signed char gb_cur_col_key_virtual=0;
signed char gb_cur_row_key_virtual=0;

unsigned char gb_key_cur_virtual_left=0;
unsigned char gb_key_before_virtual_left=0;
unsigned char gb_key_cur_virtual_right=0;
unsigned char gb_key_before_virtual_right=0;
unsigned char gb_key_cur_virtual_up=0;
unsigned char gb_key_before_virtual_up=0;
unsigned char gb_key_cur_virtual_down=0;
unsigned char gb_key_before_virtual_down=0;
unsigned char gb_key_cur_virtual_a=0;
unsigned char gb_key_before_virtual_a=0;

//unsigned char gb_key_cur_virtual[6];//L R U D A B
//unsigned char gb_key_before_virtual[6];

char gb_keyRemap[16];
unsigned char gb_id_key_left=0x04;  //Q
unsigned char gb_id_key_right=0x06; //E
unsigned char gb_id_key_up=0x02;    //2
unsigned char gb_id_key_down=0x08;  //S
unsigned char gb_id_key_a=0x05;     //W
unsigned char gb_id_key_b=0x0F;     //V

unsigned char gb_car_key_left=46;
unsigned char gb_car_key_right=46;
unsigned char gb_car_key_up=46;
unsigned char gb_car_key_down=46;
unsigned char gb_car_key_a=46;
unsigned char gb_car_key_b=46;


unsigned char gb_current_delay_emulate_ms= gb_delay_emulate_ms;
unsigned char gb_current_ms_poll_keyboard = gb_ms_keyboard;
unsigned char gb_delay_t=0;
unsigned char gb_delay_sound=0;

static unsigned char gbVolMixer=0; //Beep
static unsigned char gbCont=0;


//Funciones
void ResetEmu(void);
void Setup(void);
void CPU_init(void);
void CPU_loop(void);
void SDL_DumpVGA(void);
void Loadrom2Flash(unsigned char id);
void SDL_keys_poll(void);
void do_tinyOSD(void);
void Beep_poll(void);
void OSDClear(void);
void SDLClear(void);
void SDLprintText(const char *cad,unsigned int x, unsigned int y, unsigned char color,unsigned char backcolor);
void SDLprintCharOSD(char car,unsigned int x,unsigned int y,unsigned char color,unsigned char backcolor);
inline void jj_fast_putpixel(unsigned int x,unsigned int y,unsigned char c);
#ifdef use_lib_hdmi
 void DumpVideoHDMI(void);
#else
 #ifdef use_lib_vga
  void DumpVideoVGA(void);
 #endif
#endif 
void ShowKey4x4(void);
void ShowVirtualKeyboard(void);
void SDL_keys_virtual_poll(void);
void showFPS(void);
unsigned char ShowTinyMenu(char *cadTitle,char **ptrValue,unsigned char idSel,unsigned char aMax);



//***********************************************************
void SDLClear()
{
 /*
 unsigned int a32= gb_const_colorNormal[0];
 a32= a32|(a32<<8)|(a32<<16)|(a32<<24);
 for (int y=0; y<gb_topeY; y++){
  for (int x=0; x<gb_topeX_div4; x++)
  {   
   #ifdef use_lib_cvbs_bitluni
    gb_buffer_cvbs32[y][x]= a32;
   #else
    gb_buffer_vga32[y][x]= a32;
   #endif 
  }
 }
 */
 #ifdef use_lib_hdmi 
  memset(hdmi_data_array,0,76800); //memset(hdmi_data_array,0,sizeof(hdmi_data_array));
 #else  
  #ifdef use_lib_vga
   memset(vga_data_array,0,76800); //(640*240)DIV 2
   //Pendiente optimizar
//   for (unsigned int y=0;y<200;y++)
//   {
//    for (unsigned int x=0;x<640;x++)    
//    {
//     drawPixel(x,y,0);     
//    }
//   }
  #endif
 #endif 
}

//***********************************************************
void OSDClear()
{
 SDLClear();
}

//***********************************************************
inline void jj_fast_putpixel(unsigned int x,unsigned int y,unsigned char c)
{
 //gb_buffer_vga[y][x^2]= gb_const_colorNormal[c];
 //unsigned int auxOffs= (y<<8)+(y<<6)+x;
 //dibujo[auxOffs]= (c==0)?0:0xFF;

 #ifdef use_lib_hdmi
  unsigned int auxOffs= (y<<8)+(y<<6)+x;
  hdmi_data_array[auxOffs]= c;
 #else
  #ifdef use_lib_vga
   //VGA es 640x480, doble 320x240
   //x=(x<<1);
   //drawPixel(x,y,(c&0x07));
   //drawPixel((x+1),y,(c&0x07));
   unsigned int offsetVGA= (y<<8)+(y<<6)+x; //(destY*320)+destX doble x
   c= (c&0x07);
   unsigned char aux= (c<<3)|c;   
   vga_data_array[offsetVGA]= aux;   //2 pixels   
  #endif
 #endif 
 //unsigned int auxOffs= (y*320*2)+x;
 //oneline[auxOffs]= c;
 //oneline[auxOffs+1]= c;
}

//*************************************************************************************
void SDLprintCharOSD(char car,unsigned int x,unsigned int y,unsigned char color,unsigned char backcolor)
{ 
// unsigned char aux = gb_sdl_font_6x8[(car-64)];
 unsigned int auxId = car << 3; //*8
 unsigned char aux;
 unsigned char auxBit,auxColor;
 for (unsigned char j=0;j<8;j++)
 {
  aux = gb_sdl_font_8x8[auxId + j];
  for (unsigned int i=0;i<8;i++)
  {
   auxColor= ((aux>>i) & 0x01);
   //SDLputpixel(surface,x+(6-i),y+j,(auxColor==1)?color:backcolor);
   jj_fast_putpixel(x+(6-i),y+j,(auxColor==1)?color:backcolor);
  }
 }
}

//***************************************************************************
void SDLprintText(const char *cad,unsigned int x, unsigned int y, unsigned char color,unsigned char backcolor)
{
//SDL_Surface *surface,
// gb_sdl_font_6x8
 int auxLen= strlen(cad);
 if (auxLen>50)
  auxLen=50;
 for (int i=0;i<auxLen;i++)
 {
  SDLprintCharOSD(cad[i],x,y,color,backcolor);
  x+=7;
 }
}


#define max_gb_osd_key_4x4 17
const char * gb_osd_key_4x4[max_gb_osd_key_4x4]={
 "1",
 "2",
 "3",
 "4",
 "Q",
 "W",
 "E",
 "R",
 "A",
 "S",
 "D",
 "F",
 "Z",
 "X",
 "C",
 "V",
 "Exit Menu"
};

#define max_gb_osd_gamepad 7
const char * gb_osd_gamepad[max_gb_osd_gamepad]={
 "Left",
 "Right",      
 "Up",
 "Down",
 "A",
 "B",
 "Exit Menu"
};

#define max_gb_main_menu 11
const char * gb_main_menu[max_gb_main_menu]={
 "Load ROM",
 "Speed",
 "Screen Adjust",
 "Gamepad Remap",
 "Keyboard virtual",
 "Sound",
 "Reset",
 "Help",
 "RAM free",
 "About",
 "Exit Menu"
};

#define max_gb_enable_menu 3
const char * gb_enable_menu[max_gb_enable_menu]={
 "Enable",
 "Disable",
 "Exit Menu"
};

#define max_gb_speed_sound_menu 8
const char * gb_speed_sound_menu[max_gb_speed_sound_menu]={
 "0",
 "1",
 "2",
 "4",
 "8",
 "16",
 "32",
 "Exit Menu"
};

#define max_gb_osd_delay_instructions 3
const char * gb_osd_delay_instructions[max_gb_osd_delay_instructions]={
 "AUTO (16 ms)",
 "0 (fast)",
 "Exit Menu"
};

#define max_gb_speed_videoaudio_options_menu 5
const char * gb_speed_videoaudio_options_menu[max_gb_speed_videoaudio_options_menu]={
 "Audio poll",
 "Video delay",
 "Keyboard poll",
 "CPU delay",
 "Exit Menu"
};

#define max_gb_osd_screen 4
const char * gb_osd_screen[max_gb_osd_screen]={
 "Pixels Left",
 "Tiles",
 "TV Effect",
 "Exit Menu"
};

#define max_gb_osd_screen_values 6
const char * gb_osd_screen_values[max_gb_osd_screen_values]={
 "0",
 "2",
 "4", 
 "8", 
 "16",
 "Exit Menu"
};

#define max_gb_reset_menu 3
const char * gb_reset_menu[max_gb_reset_menu]={
 "Soft",
 "Hard",
 "Exit Menu" 
};

#define max_gb_sound_menu 3
const char *gb_sound_menu[max_gb_sound_menu]={
 "BEEP ON/OFF",
 "Frequency Hz",
 "Exit Menu"
};

#define max_gb_sound_hz_menu 5
const char *gb_sound_hz_menu[max_gb_sound_hz_menu]={
 "500 Hz",
 "250 Hz",
 "166 Hz",
 "125 Hz",
 "Exit Menu"
};

#define max_gb_help_menu 14
const char * gb_help_menu[max_gb_help_menu]={
 "F1 show OSD",
 "Key",
 " 1,2,3,4",
 " q,w,e,r",
 " a,s,d,f",
 " z,x,c,v",
 "",
 "Gamepad",
 " Left:       q",
 " Right:      e",
 " Up:         2",
 " Down:       s",
 " 0 (Fire A): w",
 " . (Fire B): v"
};

#define max_gb_about_menu 14
const char * gb_about_menu[max_gb_about_menu]={
 "Date Published: 2024/07/21",
 "", 
 "Port of the Chip8 emulator by ackerman,",
 "based on x86 Spittie.",
 "", 
 "Waveshare RP2040 pizero",
 "VGA 3 bpp (Hunter Adams and San Tarcisio",
 "mod)",
 " GPIO 18,19,20 - red,green,blue",
 " GPIO 16,17 - hsync,vsync",
 "HDMI (adafruit picodvi hdmi mod)",
 "PS/2 keyboard (ps2kbdlib michalhol mod)",
 " PS/2 GPIO (4 CLK, 5 DATA)", 
 " USB2PS/2 PIO USB GPIO (6 CLK, 7 DATA)"
};


#ifdef use_lib_200x150
 #define gb_pos_x_menu 50
 #define gb_pos_y_menu 20
 //#define gb_pos_x_menu 2
 //#define gb_pos_y_menu 8 
#else
 #define gb_pos_x_menu 100
 #define gb_pos_y_menu 40
#endif

#define gb_osd_max_rows 10


void OSDMenuRowsDisplayScroll(const char **ptrValue,unsigned char currentId,unsigned char aMax)
{//Dibuja varias lineas
 //JJ vga.setTextColor(WHITE,BLACK);
 for (int i=0;i<gb_osd_max_rows;i++)
 {
  //JJ vga.setCursor(gb_pos_x_menu, gb_pos_y_menu+8+(i<<3));
  //JJ vga.print("                    ");
  SDLprintText("                    ",gb_pos_x_menu,gb_pos_y_menu+8+(i<<3),ID_COLOR_BLACK,ID_COLOR_BLACK);
 }
 
 for (int i=0;i<gb_osd_max_rows;i++)
 {
  if (currentId >= aMax)
   break;   
  //JJ if (i == 0)
  //JJ  vga.setTextColor(CYAN,BLUE);
  //JJ else
  //JJ  vga.setTextColor(WHITE,BLACK);
  //JJ vga.setCursor(gb_pos_x_menu, gb_pos_y_menu+8+(i<<3));
  //JJ vga.print(ptrValue[currentId]);
  SDLprintText(ptrValue[currentId],gb_pos_x_menu,gb_pos_y_menu+8+(i<<3),((i==0)?ID_COLOR_BLACK:ID_COLOR_WHITE),((i==0)?ID_COLOR_WHITE:ID_COLOR_BLACK));
  currentId++;
 }     
}

//***********************************************************************************************
void OSDdrawRectFill(unsigned short int x,unsigned short int y,unsigned short int aWidth, unsigned short int aHeight, unsigned char aColor)
{
 //unsigned short int destX; 
 #ifdef use_lib_hdmi
   unsigned int offsetHDMI;
   offsetHDMI= (y<<8)+(y<<6)+x;
   for (unsigned short int j=0;j<aHeight;j++)
   {
    memset(&hdmi_data_array[offsetHDMI],aColor,aWidth); //2 pixels    
    offsetHDMI+= 320; //nueva linea 320 pixels 320 bytes
   }
 #else
  #ifdef use_lib_vga
   //destX= (x+i)<<1;
   //drawPixel(destX,(y+j),aColor);
   //destX++;
   //drawPixel(destX,(y+j),aColor);
   unsigned int offsetVGA;
   aColor= (aColor & 0x07);
   aColor= (aColor<<3)|(aColor &0x07);
   offsetVGA= (y<<8)+(y<<6)+x;
   for (unsigned short int j=0;j<aHeight;j++)
   {
    memset(&vga_data_array[offsetVGA],aColor,aWidth); //2 pixels    
    offsetVGA+= 320; //nueva linea 640 pixels 320 bytes
   }
  #endif 
 #endif
 
//jj_fast_putpixel(gb_osd_sdl_surface,(x+i),(y+j),aColor);
//gb_buffer_vga[(y+j)][(x+i)^2]= gb_const_colorNormal[aColor];
}

//********************************************************************************************
void OSDprintChar(char car,unsigned short int x,unsigned short int y,unsigned char color,unsigned char backcolor,unsigned char isDouble,unsigned char isVertical)
{
// unsigned char aux = gb_sdl_font_6x8[(car-64)];
 unsigned int auxId = car << 3; //x8
 unsigned char aux;
 unsigned char auxBit,auxColor;
 unsigned short int xDest=0;
 unsigned short int yDest=0;
 
 #ifdef use_lib_hdmi
  unsigned char colorArray[2];
  colorArray[0]= (backcolor==0)?0:0xFF;
  colorArray[1]= (color==0)?0:0xFF;
 #else
  #ifdef use_lib_vga
   unsigned int offsetVGA;
   unsigned char colorArray[2];
   color= color &0x07;
   color= (color<<3)|color;
   backcolor= backcolor &0x07;
   backcolor= (backcolor<<3)|backcolor;
   colorArray[0]= backcolor;
   colorArray[1]= color;
  #endif
 #endif 

 
 if (isVertical==0)
 {//Caracter en horizontal
  for (unsigned char j=0;j<8;j++)
  {
   aux = gb_sdl_font_8x8[auxId + j]; //aux = gb_sdl_font_6x8[auxId + j];
   for (int i=0;i<8;i++)
   {
    auxColor= ((aux>>i) & 0x01);    
    //jj_fast_putpixel(surface,x+(6-i),y+j,(auxColor==1)?color:backcolor);
//    gb_buffer_vga[(y+j)][(x+(6-i))^2]= (auxColor==1)?gb_const_colorNormal[color]:gb_const_colorNormal[backcolor];
    #ifdef use_lib_hdmi
     jj_fast_putpixel((x+(6-i)),(y+j),colorArray[auxColor]);
    #else
     #ifdef use_lib_vga
//      xDest= x<<1; //Sin optimizar Es el doble 640, no 320
//      drawPixel((xDest+((6-i)<<1)),(y+j),((auxColor==1)?color:backcolor));
//      xDest++;
//      drawPixel((xDest+((6-i)<<1)),(y+j),((auxColor==1)?color:backcolor));

      offsetVGA= ((y+j)<<8)+((y+j)<<6)+x+(6-i); //(destY*320)+destX doble x
      //vga_data_array[offsetVGA]= ((auxColor==1)?color:backcolor); //2 pixels
      vga_data_array[offsetVGA]= colorArray[auxColor]; //2 pixels
     #endif 
    #endif 
   }
  }
 } //00001111
 else
 {//Caracter en vertical
  if (isDouble==1)
  {
   unsigned short int aux_j=0;
   for (unsigned char j=0;j<16;j++)
   {
    aux_j= j>>1; //DIV 2
    aux = gb_sdl_font_8x8[auxId + aux_j]; //aux = gb_sdl_font_6x8[auxId + aux_j];
    for (int i=0;i<8;i++)
    {
     auxColor= ((aux>>i) & 0x01);
     xDest= x+j;
     yDest= y+i;
     //jj_fast_putpixel(surface,xDest,yDest,(auxColor==1)?color:backcolor);
//     gb_buffer_vga[yDest][xDest^2]= (auxColor==1)?gb_const_colorNormal[color]:gb_const_colorNormal[backcolor];
     #ifdef use_lib_hdmi
      jj_fast_putpixel(xDest,yDest,colorArray[auxColor]);
     #else
      #ifdef use_lib_vga
//       xDest= xDest<<1;
//       drawPixel(xDest,yDest,((auxColor==1)?color:backcolor));
//       xDest++;
//       drawPixel(xDest,yDest,((auxColor==1)?color:backcolor));

       offsetVGA= (yDest<<8)+(yDest<<6)+xDest; //(destY*320)+destX doble x
       //vga_data_array[offsetVGA]= ((auxColor==1)?color:backcolor); //2 pixels
       vga_data_array[offsetVGA]= colorArray[auxColor]; //2 pixels
      #endif
     #endif 
    }                  
   }
  }
  else
  {
   for (unsigned char j=0;j<8;j++)
   {
    aux = gb_sdl_font_8x8[auxId + j]; //aux = gb_sdl_font_6x8[auxId + j];
    for (int i=0;i<8;i++)
    {
     auxColor= ((aux>>i) & 0x01);
     xDest= x+j;
     yDest= y+i;
     //jj_fast_putpixel(surface,xDest,yDest,(auxColor==1)?color:backcolor);
//     gb_buffer_vga[yDest][xDest^2]= (auxColor==1)?gb_const_colorNormal[color]:gb_const_colorNormal[backcolor];
//     drawPixel(xDest,yDest,((auxColor==1)?color:backcolor));
     #ifdef use_lib_hdmi
      jj_fast_putpixel(xDest,yDest,colorArray[auxColor]);
     #else
      #ifdef use_lib_vga     
       offsetVGA= (yDest<<8)+(yDest<<6)+xDest; //(destY*320)+destX doble x
       //vga_data_array[offsetVGA]= ((auxColor==1)?color:backcolor); //2 pixels
       vga_data_array[offsetVGA]= colorArray[auxColor]; //2 pixels
      #endif
     #endif  
    }
   }//fin cuando es normal
  }     
 }     
}

//***********************************************************************************************
void OSDprintText(char *cad,unsigned char x, unsigned char y, unsigned char color,unsigned char backcolor,unsigned char isDouble, unsigned char isVertical)
{//Coordenadas en 8x8
 unsigned short int xDest= ((unsigned short int)x)<<3; 
 unsigned short int yDest= ((unsigned short int)y)<<3;
 
 int auxLen= strlen(cad);
 if ((auxLen>50)&&(isDouble==1))
 {
  auxLen=50;
 }
 
 for (int i=0;i<auxLen;i++)
 {
  OSDprintChar(cad[i],xDest,yDest,color,backcolor,isDouble,isVertical);
  if (isVertical==0)
  {
   xDest+=6; //xDest+=8;
  }
  else
  {
   yDest-=6;//yDest-=8;
  }  
 }  
}

#ifdef use_lib_hdmi
 //***********************************************************************************************
 unsigned short int x_text_ini_hdmi= 5; //34;
 unsigned short int xTextUpDown_hdmi= 2;
 unsigned short int yTextUp_hdmi= 4; //32/8
 unsigned short int yTextDown_hdmi= 19; //152/8
 unsigned short int y_text_ini_hdmi= 36;
 unsigned char ESP32_BLACK_hdmi=0x00;
 unsigned char ESP32_WHITE_hdmi=0xFF;

 unsigned char colorArray_hdmi[2];

 //************************************************************************************************************************************
 void OSDprintCharHDMI(char car,unsigned short int x,unsigned short int y,unsigned char color,unsigned char backcolor)
 {
  unsigned int auxId = car << 3; //x8
  unsigned char aux;
  unsigned char auxBit,auxColor;
  unsigned short int xDest=0;
  unsigned short int yDest=0;  

  colorArray_hdmi[0]= (backcolor==0)?0:0xFF;
  colorArray_hdmi[1]= (color==0)?0:0xFF;

  unsigned int auxOffs= ((unsigned int)y<<8)+((unsigned int)y<<6)+(unsigned int)x;
  
  for (unsigned char j=0;j<8;j++)
  {
   aux = gb_sdl_font_8x8[auxId + (unsigned int)j]; //aux = gb_sdl_font_6x8[auxId + j];
   //6 bits fuente 6x8
   for (int i=6;i>=1;i--)
   {
    auxColor= ((aux>>i) & 0x01);
    //jj_fast_putpixel((x+(6-i)),(y+j),colorArray_hdmi[auxColor]);
    hdmi_data_array[auxOffs++]= colorArray_hdmi[auxColor];
   }
   auxOffs+=314; //320-6 nueva linea
  }
 }

 //********************************************************************************************
 void DrawNewOSDMenuHDMI(char **ptr_cad, unsigned char idSel, unsigned char maximo)
 {
  unsigned char maxCar=44;
  unsigned char idForeCol= ESP32_WHITE_hdmi;
  unsigned char idBGCol= ESP32_BLACK_hdmi;
  
  char auxCad[64];

  short int contRow=gb_osd_ventana_ini;
 
  unsigned char scr_up= 0;
  unsigned char scr_down= 0;

  scr_up= (gb_osd_ventana_ini>0)? 1:0;
  scr_down= (gb_osd_ventana_fin<(maximo-1))? 1:0;
 
  if (scr_up==1) { OSDprintText((char *)">",xTextUpDown_hdmi,yTextUp_hdmi,ESP32_BLACK_hdmi,ESP32_WHITE_hdmi,1,1); }
  else { OSDprintText((char *)" ",xTextUpDown_hdmi,yTextUp_hdmi,ESP32_BLACK_hdmi,ESP32_WHITE_hdmi,1,1); } 
  if (scr_down==1){ OSDprintText((char *)"<",xTextUpDown_hdmi,yTextDown_hdmi,ESP32_BLACK_hdmi,ESP32_WHITE_hdmi,1,1); }
  else { OSDprintText((char *)" ",xTextUpDown_hdmi,yTextDown_hdmi,ESP32_BLACK_hdmi,ESP32_WHITE_hdmi,1,1); } 

 
  short int contRowDest=0;
  //unsigned int contRow=0;
  for (unsigned short int i=gb_osd_ventana_ini;i<=gb_osd_ventana_fin;i++)
  {
   idForeCol= (contRow==idSel)? ESP32_BLACK_hdmi : ESP32_WHITE_hdmi;
   idBGCol= (contRow==idSel)? ESP32_WHITE_hdmi : ESP32_BLACK_hdmi;     
   if (i<maximo)
   {
//    strcpy(auxCad," ");
//    strcat(auxCad,ptr_cad[contRow]);
//    unsigned char len= strlen(auxCad);  
//    //for (unsigned char j=len;j<44;j++)  
//    for (unsigned char j=len;j<maxCar;j++)
//    {
//     auxCad[j]=' ';   
//    }
//    //auxCad[44]='\0'; //Maximo 44 caracteres
//    auxCad[maxCar]='\0'; //Maximo 44 caracteres
//    //OSDprintTextHDMI(auxCad,x_text_ini_hdmi,(y_text_ini_hdmi+(contRowDest<<3))>>3,idForeCol,idBGCol,0,0); 
//
    unsigned short int xDest= ((unsigned short int)x_text_ini_hdmi)<<3;    
    unsigned short int yDest= ((unsigned short int)((y_text_ini_hdmi+(contRowDest<<3))>>3))<<3;    
    //for (int kk=0;kk<44;kk++)
    for (int kk=0;kk<maxCar;kk++)
    {
//     OSDprintCharHDMI(auxCad[kk],xDest,yDest,idForeCol,idBGCol);
     if (ptr_cad[contRow][kk]=='\0')
     {
      if (kk<maxCar)
      {//Palabra mas corta que linea       
       unsigned short int ancho= (maxCar-kk); //x6 = (x4 + x2)
       ancho= (ancho<<2)+(ancho<<1);
       OSDdrawRectFill(xDest,yDest, ancho, 8,idBGCol);
      }
      break;
     }
     OSDprintCharHDMI(ptr_cad[contRow][kk],xDest,yDest,idForeCol,idBGCol);
     xDest+= 6;
    }
   }
   else
   {   
    //Lineas vacias sin rellenar
    OSDdrawRectFill((x_text_ini_hdmi<<3),((((y_text_ini_hdmi+(contRowDest<<3))>>3))<<3),96, 8,0); //(16x6)=96
   }
   
   contRowDest++;
   contRow++;
  }
 
 }
#endif

//***********************************************************************************************
void DrawNewOSDMenu(char **ptr_cad, unsigned char idSel, unsigned char maximo)
{//OSD 32x16  Muestra 16 filas
 #ifdef use_lib_hdmi
  unsigned char ESP32_WHITE=0xFF;
  unsigned char ESP32_BLACK=0x00; 
 #else 
  #ifdef use_lib_vga
   unsigned char ESP32_WHITE=(0xFF&0x07);
   unsigned char ESP32_BLACK=0x00;
  #endif 
 #endif 
 unsigned char idForeCol= ESP32_WHITE;
 unsigned char idBGCol= ESP32_BLACK;
 
 unsigned short int x_text_ini=0;
 unsigned short int xTextUpDown=0;
 unsigned short int yTextUp=0;
 unsigned short int yTextDown=0;

 if ((gb_vga_ancho==320)&&((gb_vga_alto==240)||(gb_vga_alto==200)))
 {
  x_text_ini= 5; //34;
  xTextUpDown= 2;
  yTextUp= 4; //32/8
  yTextDown= 19; //152/8                                         
 }
 else
 {
  if ((gb_vga_ancho==360)&&(gb_vga_alto==200))
  {
   x_text_ini= 5+3; //34;
   xTextUpDown= 2+3;
   yTextUp= 4;
   yTextDown= 19;                                          
  }
 }

 unsigned short int y_text_ini= 36;
 char auxCad[64];

 short int contRow=gb_osd_ventana_ini;
 
 unsigned char scr_up= 0;
 unsigned char scr_down= 0;


 scr_up= (gb_osd_ventana_ini>0)? 1:0;
 scr_down= (gb_osd_ventana_fin<(maximo-1))? 1:0;
 
 if (scr_up==1) { OSDprintText((char *)">",xTextUpDown,yTextUp,ESP32_BLACK,ESP32_WHITE,1,1); }
 else { OSDprintText((char *)" ",xTextUpDown,yTextUp,ESP32_BLACK,ESP32_WHITE,1,1); } 
 if (scr_down==1){ OSDprintText((char *)"<",xTextUpDown,yTextDown,ESP32_BLACK,ESP32_WHITE,1,1); }
 else { OSDprintText((char *)" ",xTextUpDown,yTextDown,ESP32_BLACK,ESP32_WHITE,1,1); } 

 
 short int contRowDest=0;
 //unsigned int contRow=0;
 for (unsigned short int i=gb_osd_ventana_ini;i<=gb_osd_ventana_fin;i++)
 {
  idForeCol= (contRow==idSel)? ESP32_BLACK : ESP32_WHITE;
  idBGCol= (contRow==idSel)? ESP32_WHITE : ESP32_BLACK;     
  if (i<maximo)
  {
   strcpy(auxCad," ");
   strcat(auxCad,ptr_cad[contRow]);
   unsigned char len= strlen(auxCad);
   for (unsigned char j=len;j<44;j++)
   {
    auxCad[j]=' ';   
   }
   auxCad[44]='\0'; //Maximo 44 caracteres
   OSDprintText(auxCad,x_text_ini,(y_text_ini+(contRowDest<<3))>>3,idForeCol,idBGCol,0,0);
//   OSDprintText(auxCad,x_text_ini,(y_text_ini+(contRowDest<<3))>>3,0xFF,0x00,0,0);
  }
  else
  {
   memset(auxCad,' ',44); //Lineas vacias sin rellenar
   auxCad[44]='\0';    
   OSDprintText(auxCad,x_text_ini,(y_text_ini+(contRowDest<<3))>>3,ESP32_WHITE,ESP32_BLACK,0,0);
//   OSDprintText(auxCad,x_text_ini,(y_text_ini+(contRowDest<<3))>>3,0xFF,0x00,0,0);
  }
   
  contRowDest++;
  contRow++;
 }
 
}

//***************************************
void OSDSubir(unsigned char idSel)
{//Le doy a la tecla de subir
 unsigned char idPag= idSel>>4; //idSel/16;
 if (idPag>0)
 {
  gb_osd_ventana_ini--;
  if (gb_osd_ventana_ini<0)
  {
   gb_osd_ventana_ini= 0;
  }
  gb_osd_ventana_fin= gb_osd_ventana_ini+15;
 }
 else
 {
  gb_osd_ventana_ini=0;
  gb_osd_ventana_fin= gb_osd_ventana_ini+15;
 }
}

//***********************************************************
void OSDBajar(unsigned char idSel)
{//Le doy a la tecla de bajar
 unsigned char idPag= idSel>>4;//idSel/16;
 if (idPag>0)
 {
  gb_osd_ventana_ini++;
  gb_osd_ventana_fin= gb_osd_ventana_ini+15;
 }
}



unsigned gb_aRand[16]={3,0,1,3,0,2,0,3,1,0,2,0,0,1,0,3};
unsigned char gb_contRand=0;
unsigned short int gb_line_shift=0;
unsigned char gb_cont_baja=0;
unsigned char gb_baja=1;

unsigned short int gb_alto=200;
unsigned short int gb_ancho=320;

static unsigned int g_seed=0;

//****************************************
void fast_srand(unsigned int seed)
{
 g_seed = seed;     
}

//****************************************
inline unsigned int fast_rand()
{
 g_seed = ((214013 * g_seed) + 2531011);
 return (g_seed>>16)&0x7FFF;
}


//*************************************************************************************
void drawTVeffect()
{    
 unsigned char gb_vga_8colors=1; //revisar quitar
  
 unsigned short int gb_alto= gb_vga_alto;
 unsigned short int gb_ancho= gb_vga_ancho;
 unsigned short int filtroY_0[2];
 unsigned short int filtroX_0[2];
 unsigned short int filtroY_1[2];
 unsigned short int filtroX_1[2];  


 if ((gb_vga_ancho==320)&&((gb_vga_alto==240)||(gb_vga_alto==200)))
 {
  filtroY_0[0]= 6; 
  filtroY_0[1]= 17;
  filtroX_0[0]= 12;
  filtroX_0[1]= 302;
  filtroY_1[0]= 32;
  filtroY_1[1]= 161;
  filtroX_1[0]= 12;
  filtroX_1[1]= 302;
 }
 else
 {
  if ((gb_vga_ancho==360)&&(gb_vga_alto==200))
  {
   filtroY_0[0]= 6; 
   filtroY_0[1]= 17;
   filtroX_0[0]= 12+(3*8);
   filtroX_0[1]= 302+(3*8);
   filtroY_1[0]= 32;
   filtroY_1[1]= 161;
   filtroX_1[0]= 12+(3*8);
   filtroX_1[1]= 302+(3*8);                                  
  }
 }

 //000000
 //010101
 //101010
 //111111 
 unsigned char aSnow[4];
 if (gb_vga_8colors==1)
 {//azul en colores
  aSnow[0]=0; ////gb_const_colorNormal[0];
  aSnow[1]=6; //gb_const_colorNormal[6];
  aSnow[2]=4; //gb_const_colorNormal[4];  
  aSnow[3]=7; //gb_const_colorNormal[7];  
 }
 else
 {//grises en 64 colores
  aSnow[0]=0; //gb_const_colorNormal[0];
  aSnow[1]=21; //gb_const_colorNormal[21];
  aSnow[2]=42; //gb_const_colorNormal[42];  
  aSnow[3]=63; //gb_const_colorNormal[63];     
 }

// unsigned char aSnow[4]={
//  /*
//  gb_const_colorNormal[0] //negro
//  ,gb_const_colorNormal[1] //rojo
//  ,gb_const_colorNormal[2] //verde
//  ,gb_const_colorNormal[3] //amarillo
//  //,gb_const_colorNormal[4] //azul
//  */
//
//  gb_const_colorNormal[0] //negro
//  ,gb_const_colorNormal[6] //cian
//  ,gb_const_colorNormal[4] //azul oscuro
//  ,gb_const_colorNormal[7] //blanco
// }; //unsigned char aSnow[4]={0,21,42,63};
 
 unsigned char aByte;
 unsigned char distancia;
 unsigned char shiftBit;

 unsigned char a0,a1,a2,a3;
 unsigned int a32;
 //unsigned char a[4];
 //unsigned char cont4pixels=0;
 //for (unsigned int y=0;y<200;y++)
 for (unsigned int y=0;y<gb_alto;y++)
 {
  //for (unsigned int x=0;x<320;x++)  
  
  shiftBit=0;
  distancia= y&15; //y%16;
  if ((y>=gb_line_shift)&&(y<(gb_line_shift+16)))
  {   
   switch (distancia)
   {
    case 0: case 1: case 2: case 13: case 14: case 15: shiftBit= 1; break;
    case 3: case 4: case 5: case 10: case 11: case 12: shiftBit= 2; break;
    case 6: case 7: case 8: case 9: shiftBit= 3; break;     
   }
  }
  //printf("%d %d %d\n",y,distancia,shiftBit);

  #ifdef use_lib_hdmi
  #else
   #ifdef use_lib_vga
    unsigned int offsetVGA= (y<<8)+(y<<6); //(y * 320)+X;
   #endif
  #endif 
  
  for (unsigned int x=0;x<gb_ancho;x++)
  {
   //40,10,200,10   
//   if (
//       ((y>8)&&(y<(8+12)) && (x>16)&&(x<(16+280)))
//       ||
//       ((y>34)&&(y<(34+132)) && (x>16)&&(x<(16+280)))
//      )
//    continue;

   if (
       ((y>=filtroY_0[0])&&(y<=filtroY_0[1]) && (x>=filtroX_0[0])&&(x<=filtroX_0[1]))
       ||
       ((y>=filtroY_1[0])&&(y<=filtroY_1[1]) && (x>=filtroX_1[0])&&(x<=filtroX_1[1]))
      )
   {
    #ifdef use_lib_hdmi
    #else
     #ifdef use_lib_vga
      offsetVGA++;
     #endif
    #endif
    
    continue;
   }
   
   //gb_contRand= rand()%4;
   aByte= (shiftBit==0)? aSnow[gb_aRand[gb_contRand]] : aSnow[(gb_aRand[gb_contRand]-shiftBit) & 0x03];

   //jj_fast_putpixel(gb_osd_sdl_surface,x,y,aByte);
//   gb_buffer_vga[y][x^2]= aByte;
   #ifdef use_lib_hdmi
   #else
    #ifdef use_lib_vga
     //drawPixel(x,y,aByte);
     //unsigned int offsetVGA= (y<<8)+(y<<6)+x; //(y * 320)+X;
     aByte= ((aByte<<3) | aByte);
     vga_data_array[offsetVGA]= aByte; //2 pixels
     offsetVGA++;
    #endif
   #endif
   
   //a[cont4pixels]= aByte;
   //cont4pixels++;
   //if (cont4pixels>=3)
   //{
   // cont4pixels=0;
   // a0=a[0];
   // a1=a[1];
   // a2=a[2];
   // a3=a[3];
   // a32= a2 | (a3<<8) | (a0<<16) | (a1<<24); //ESP32
   // gb_buffer_vga32[y][x>>2]= a32;
   //}

   gb_contRand++;
   if (gb_contRand>15)
   {
    //gb_contRand= gb_contRand + rand()%16;//gb_aRand[gb_contRand];
    //gb_contRand= gb_contRand + fast_rand()%16;//gb_aRand[gb_contRand];
    gb_contRand= gb_contRand + fast_rand() & 0x0F;
    gb_contRand= gb_contRand & 0x0F;
   }
  } 
 }


 gb_cont_baja++;
 //if (gb_cont_baja>=60)
 if (gb_cont_baja>=240)
 {
  gb_baja= ((fast_rand() & 0x03)>0)? 1 : 0;
  gb_cont_baja=0;
 }
 //printf("gb_baja %d\n",gb_baja);
 
 gb_line_shift= (gb_baja==1)? (gb_line_shift+1) : (gb_line_shift-1);
 if (gb_line_shift >= (gb_alto-1))
 {
  gb_line_shift=0;
 }
 
 //printf("SNOW\n");
 //fflush(stdout); 
}



//********************************************************************************************
void WaitFreeKeys(unsigned char isEffect)
{//Espera a que se liberen todas las teclas
 unsigned char auxKey= 1;
 unsigned int timeIni= millis(); //revisar millis(); //SDL_GetTicks();
 unsigned int timeCur= timeIni;

 while (auxKey!=0)
 {
  auxKey=0;

  if (checkKey(KEY_CURSOR_LEFT)==1){auxKey= auxKey | 1;}
  if (checkKey(KEY_CURSOR_RIGHT)==1){auxKey= auxKey | 2; }
  if (checkKey(KEY_CURSOR_UP)==1){auxKey= auxKey | 4; }
  if (checkKey(KEY_CURSOR_DOWN)==1){auxKey= auxKey | 8; }
  if (checkKey(KEY_ENTER)==1){ auxKey= auxKey | 16; }
  if (checkKey(KEY_ESC)==1){ auxKey= auxKey | 32; }  


  #ifdef use_lib_hdmi
   DumpVideoHDMI();
  #endif
  timeCur= millis(); //revisar millis(); //SDL_GetTicks();
  if ((timeCur-timeIni)>=20)
  {
   timeIni= timeCur;
   if ((isEffect==1)&&(gb_use_tveffect==1))
   {
    drawTVeffect();
   }
  }
 }
}


//***********************************************************************************************
//Maximo 256 elementos
unsigned char ShowTinyMenu(char *cadTitle,char **ptrValue,unsigned char idSel,unsigned char aMax)
{
 #ifdef use_lib_hdmi
  unsigned short int contSalta=0;
  gb_use_tveffect=0; //revisar
 #else
  #ifdef use_lib_vga
  #endif
 #endif
  
 //OSD 32x16
 unsigned char contKeyEq=0;
 unsigned char auxKey=0;
 unsigned char auxKey_before=0;
 unsigned char aReturn=0;
 unsigned char salir=0;
 unsigned char auxChange=1;

 gb_osd_ventana_ini= (idSel<16)? 0: (idSel-15);//idSel;
 gb_osd_ventana_fin= gb_osd_ventana_ini+15; 
 
 unsigned short int xRectTitle= 0;
 unsigned short int yRectTitle= 0;
 unsigned short int xTextTitle= 0;
 unsigned short int yTextTitle= 0;

 unsigned short int xRectLeftMenu= 0;
 unsigned short int yRectLeftMenu= 0;
 unsigned short int xTitleLeftMenu= 0;
 unsigned short int yTitleLeftMenu= 0;

 
 if ((gb_vga_ancho==320)&&((gb_vga_alto==240)||(gb_vga_alto==200)))
 {
  xRectTitle= 12;    
  yRectTitle= 6;    
  xTextTitle= 6;    
  yTextTitle= 1;    
                  
  xRectLeftMenu= 12; 
  yRectLeftMenu= 32; 
  xTitleLeftMenu= 2;
  yTitleLeftMenu= 16;
 } 

 #ifdef use_lib_hdmi 
  unsigned char ESP32_WHITE=0xFF; //Formato BGR 1 rojo, 2 verde, 3 azul
  unsigned char ESP32_BLACK=0x00;
  SDLClear(); //Solucion provisional, borro todo para no dejar rastros en HDMI
 #else
  #ifdef use_lib_vga
   unsigned char ESP32_WHITE=(0xFF&0x07); //Formato BGR 1 rojo, 2 verde, 3 azul
   unsigned char ESP32_BLACK=0x00;
  #endif 
 #endif

 #ifdef use_lib_hdmi
  OSDdrawRectFill(xRectTitle,(yRectTitle+8),292,12,ESP32_WHITE); //rectangulo Titulo
  OSDprintText((char *)"RP2040 Chip8 (Spittie) by Ackerman",xTextTitle,(yTextTitle+1),ESP32_BLACK,ESP32_WHITE,0,0); //Titulo           
 #else
  #ifdef use_lib_vga
   OSDdrawRectFill(xRectTitle,yRectTitle,292,12,ESP32_WHITE); //rectangulo Titulo
   OSDprintText((char *)"RP2040 Chip8 (Spittie) by Ackerman",xTextTitle,yTextTitle,ESP32_BLACK,ESP32_WHITE,0,0); //Titulo          
  #endif
 #endif  

 OSDdrawRectFill(xRectLeftMenu,yRectLeftMenu,26,130,ESP32_WHITE);   //OSDdrawRectFill(4,32,26,130,ESP32_WHITE);  
 OSDprintText(cadTitle,xTitleLeftMenu,yTitleLeftMenu,ESP32_BLACK,ESP32_WHITE,1,1);

 #ifdef use_lib_hdmi
  DrawNewOSDMenuHDMI(ptrValue, idSel, aMax);
 #else
  #ifdef use_lib_vga
   DrawNewOSDMenu(ptrValue, idSel, aMax);
  #endif 
 #endif 

 unsigned int timeKeyboard= millis(); //revisar millis(); //SDL_GetTicks();
 unsigned int timeKeyboard_before= timeKeyboard;
 unsigned int time_cur= millis(); //revisar millis(); //SDL_GetTicks();
 unsigned int time_prev= time_cur;
 while (salir == 0)
 {
  #ifdef use_lib_hdmi
   //if (contSalta==0)
   {
    DumpVideoHDMI();
   }
   contSalta++;
   contSalta= contSalta & 0x01F; //mod 
  #endif
   
  time_cur= millis(); //revisar millis(); //SDL_GetTicks();
  if ((time_cur-time_prev)<19)
  {       
   continue;
  }
  else
  {
   time_prev= time_cur;  
  }

  if (auxChange==1)
  {
   auxChange= 0;
   #ifdef use_lib_hdmi
    DrawNewOSDMenuHDMI(ptrValue, idSel, aMax);
   #else
    #ifdef use_lib_vga
     DrawNewOSDMenu(ptrValue, idSel, aMax);
    #endif 
   #endif 
  }

  if (gb_use_tveffect==1)
  {
   drawTVeffect();
  }
  
  auxKey=0;
  if (checkKey(KEY_CURSOR_LEFT)==1){auxKey= auxKey | 1;}
  if (checkKey(KEY_CURSOR_RIGHT)==1){auxKey= auxKey | 2; }
  if (checkKey(KEY_CURSOR_UP)==1){auxKey= auxKey | 4; }
  if (checkKey(KEY_CURSOR_DOWN)==1){auxKey= auxKey | 8; }
  if (checkKey(KEY_ENTER)==1){ auxKey= auxKey | 16; }
  if (checkKey(KEY_ESC)==1){ auxKey= auxKey | 32; }
      if (auxKey_before != auxKey)
  {
   if ((auxKey & 0x04)== 0x04) { if (idSel>0) {idSel--; OSDSubir(idSel); } }//UP
   if ((auxKey & 0x08)== 0x08) { if (idSel<(aMax-1)) {idSel++; OSDBajar(idSel); } }//DOWN
   if ((auxKey & 0x10)== 0x10) { salir= 1; aReturn= idSel; } //ENTER
   if ((auxKey & 0x20)== 0x20) { salir=1; aReturn= 255; } //ESC
                 
   auxKey_before= auxKey;
   contKeyEq=0;

   auxChange= 1;
  }
  else
  {
   contKeyEq++;
   if (contKeyEq>20)
   {
    if ((auxKey & 0x04)== 0x04) { if (idSel>0){ idSel--; OSDSubir(idSel); } }//UP
    if ((auxKey & 0x08)== 0x08) { if (idSel<(aMax-1)){ idSel++; OSDBajar(idSel); } }//DOWN

    if (auxKey!=0)
    {
     auxChange= 1;
    }
   }
  }           
 }

 WaitFreeKeys(1); //Espera a que se liberen todas las teclas y muestra efecto TV 
 
 return aReturn;
}

/*
unsigned char ShowTinyMenu(const char *cadTitle,const char **ptrValue,unsigned char aMax,short int aSel)
{
 unsigned char aReturn=0;
 unsigned char salir=0;
 unsigned char teclaPulsada=0;
 //JJ #ifdef use_lib_200x150
  //JJ vga.fillRect(0,0,200,150,BLACK);
  //JJ vga.fillRect(0,0,200,150,BLACK);//Repeat Fix visual defect
 //JJ #else
  //JJ vga.fillRect(0,0,320,200,BLACK);
  //JJ vga.fillRect(0,0,320,200,BLACK);//Repeat Fix visual defect    
 //jJ #endif
 //JJ vTaskDelay(2);
 //JJ vga.setTextColor(WHITE,BLACK);
 //JJ vga.setCursor((gb_pos_x_menu-(48)), gb_pos_y_menu-16);
 //JJ vga.print("Port Chip8(Spittie) by Ackerman");

 SDLClear();
 SDLprintText("Chip8(Spittie)by Ackerman",(gb_pos_x_menu-(48)), (gb_pos_y_menu-16),ID_COLOR_WHITE,ID_COLOR_BLACK);

 //JJ vga.setTextColor(BLACK,WHITE); 
 for (int i=0;i<12;i++)
 {  
  //JJ vga.setCursor((gb_pos_x_menu+(i*6)), gb_pos_y_menu);
  //JJ vga.print(" ");
  SDLprintCharOSD(' ',gb_pos_x_menu+(i<<3),gb_pos_y_menu,ID_COLOR_BLACK,ID_COLOR_WHITE);
 }
 SDLprintText(cadTitle,gb_pos_x_menu,gb_pos_y_menu,ID_COLOR_BLACK,ID_COLOR_WHITE);
  
 //JJ vga.setCursor(gb_pos_x_menu,gb_pos_y_menu);
 //JJ vga.print(cadTitle);

 aReturn = (aSel!=-1)?aSel:0;
 OSDMenuRowsDisplayScroll(ptrValue,aReturn,aMax);

 if (gb_show_key4x4 == 1)
 {
  ShowKey4x4();
 }
  
 #ifdef use_lib_not_use_ps2keyboard
  WaitClearAtariDB9();//Espera a dejar de tocar botones
 #endif 

 while (salir == 0)
 {
  #ifdef use_lib_hdmi    
   DumpVideoHDMI();       
  #endif
  
  gb_currentTime = millis();
  
  if ((gb_currentTime-gb_keyboardTime) >= gb_current_ms_poll_keyboard)
  {    
   gb_keyboardTime = gb_currentTime;
   teclaPulsada= 0;
   if (checkAndCleanKey(KEY_CURSOR_LEFT))
   {
    if (aReturn>10) aReturn-=10;
    OSDMenuRowsDisplayScroll(ptrValue,aReturn,aMax);
    teclaPulsada= 1;
   }
   if (checkAndCleanKey(KEY_CURSOR_RIGHT))
   {
    if (aReturn<(aMax-10)) aReturn+=10;
    OSDMenuRowsDisplayScroll(ptrValue,aReturn,aMax);
    teclaPulsada= 1;
   }     

   //case SDLK_UP:
   if (checkAndCleanKey(KEY_CURSOR_UP))
   {
    if (aReturn>0) aReturn--;
    OSDMenuRowsDisplayScroll(ptrValue,aReturn,aMax);
    teclaPulsada= 1;
   }
   if (checkAndCleanKey(KEY_CURSOR_DOWN))
   {
    if (aReturn < (aMax-1)) aReturn++;
    OSDMenuRowsDisplayScroll(ptrValue,aReturn,aMax);
    teclaPulsada= 1;
   }
   if (checkAndCleanKey(KEY_ENTER))
   {
    salir= 1;
    teclaPulsada= 1;
   }
   //case SDLK_KP_ENTER: case SDLK_RETURN: salir= 1;break;
   if (checkAndCleanKey(KEY_ESC))
   {
    salir=1; aReturn= 255;
    teclaPulsada= 1;
   }

   if ((gb_show_key4x4 == 1) && (teclaPulsada == 1))
   {
    ShowKey4x4();
   }   
  }//fin gb_currentTime
 }//fin salir
 gb_show_osd_main_menu= 0;

 //JJ #ifdef use_lib_200x150
  //JJ vga.fillRect(0,0,200,150,BLACK);
  //JJ vga.fillRect(0,0,200,150,BLACK);//Repeat Fix visual defect
 //JJ #else
  //JJ vga.fillRect(0,0,320,200,BLACK);
  //JJ vga.fillRect(0,0,320,200,BLACK);//Repeat Fix visual defect    
 //JJ #endif
 //JJ vTaskDelay(2);

 SDLClear();

 return aReturn;
}
*/

//Menu velocidad emulador
void ShowTinySpeedMenu()
{
 unsigned char aSelNum,aSelNumSpeed;
 aSelNum = ShowTinyMenu((char *)"SPEED OPTIONS",(char **)gb_speed_videoaudio_options_menu,0,max_gb_speed_videoaudio_options_menu);
 if ((aSelNum == 255)||(aSelNum == 4))
 {  
  gb_id_menu_cur=id_menu_main;
  return;
 }
 switch (aSelNum)
 {
  case 0: aSelNumSpeed= ShowTinyMenu((char *)"AUDIO POLL ms",(char **)gb_speed_sound_menu,0,max_gb_speed_sound_menu);
   if ((aSelNumSpeed == 255)||(aSelNumSpeed == 7))
   {
    gb_id_menu_cur=id_menu_speed;
    return;
   }
   gb_current_ms_poll_sound= (aSelNumSpeed<<1); //Multiplico x2
   break;
  case 1: aSelNumSpeed= ShowTinyMenu((char *)"VIDEO DELAY ms", (char **)gb_speed_sound_menu,0,max_gb_speed_sound_menu);
   if ((aSelNumSpeed == 255)||(aSelNumSpeed == 7))
   {
    gb_id_menu_cur=id_menu_speed;    
    return;
   }
   gb_current_delay_emulate_ms = (aSelNumSpeed<<1);
   break;
  case 2: aSelNumSpeed= ShowTinyMenu((char *)"KEY POLL ms", (char **)gb_speed_sound_menu,0,max_gb_speed_sound_menu);
   if ((aSelNumSpeed == 255)||(aSelNumSpeed == 7))
   {
    gb_id_menu_cur=id_menu_speed;
    return;
   }
   gb_current_ms_poll_keyboard= (aSelNumSpeed<<1);
   break;
  case 3: aSelNumSpeed= ShowTinyMenu((char *)"  CPU DELAY", (char **)gb_osd_delay_instructions,0,max_gb_osd_delay_instructions);
   if ((aSelNumSpeed == 255)||(aSelNumSpeed == 2))
   {
    gb_id_menu_cur=id_menu_speed;
    return;
   }
   switch (aSelNumSpeed)
   {
    case 0: 
     gb_auto_delay_cpu=1; 
     gb_show_osd_main_menu= 0;
     return;
     break;
    case 1: 
     gb_auto_delay_cpu=0; 
     gb_show_osd_main_menu= 0;
     return;
     break;
   }
   break;
  default: gb_id_menu_cur=id_menu_main; break;
 }

 //gb_show_osd_main_menu= 0;
 gb_id_menu_cur=id_menu_main;
}


//Menu ROM
void ShowTinyROMMenu()
{
 unsigned char aSelNum;   
 aSelNum = ShowTinyMenu((char *)"LOAD ROM", (char **)gb_list_rom_title,gb_current_sel_rom,max_list_rom);
 if (aSelNum==255)
 {
  gb_id_menu_cur=id_menu_main;
  return;
 }
 CPU_init();
 ResetEmu();
 gb_current_sel_rom = aSelNum;
 Loadrom2Flash(aSelNum);
 //SDLClear(screen);

 gb_show_osd_main_menu= 0;
}

//Ajustar pantalla
void ShowTinyScreenAdjustMenu()
{
 unsigned char aSelNum, auxCol; 
 aSelNum= ShowTinyMenu((char *)"SCREEN ADJUST", (char **)gb_osd_screen,0,max_gb_osd_screen);
 if ((aSelNum==255)||(aSelNum==3))
 {
  //gb_show_osd_main_menu= 0;
  gb_id_menu_cur=id_menu_main;
  return;
 }
 
 switch (aSelNum)
 {
  case 0:
   auxCol= ShowTinyMenu((char *)"   PIXELS", (char **)gb_osd_screen_values,0,max_gb_osd_screen_values);
   if ((auxCol==255)||(auxCol==5))
   {
    //gb_show_osd_main_menu= 0;
    gb_id_menu_cur=id_menu_screen_adjust;
    return; 
   }
   auxCol = auxCol<<1; //x2
   gb_screen_xOffset = auxCol;

   gb_show_osd_main_menu= 0;
   break;

  case 1: //Activar modo TILES
   auxCol= ShowTinyMenu((char *)"    TILES", (char **)gb_enable_menu,((gb_use_tiled==1)?0:1),max_gb_enable_menu);
   if ((auxCol==255)||(auxCol==2))
   {
    gb_id_menu_cur=id_menu_screen_adjust;
    return; 
   }
   gb_use_tiled= (auxCol==0)?1:0;
   gb_show_osd_main_menu= 0;
   break;

  case 2: //Activar efecto nieve TV OSD
   #ifdef use_lib_hdmi
    gb_id_menu_cur= id_menu_screen_adjust; //En HDMI no hay efecto TV
    return;
   #else
    auxCol= ShowTinyMenu((char *)"  TV EFFECT", (char **)gb_enable_menu,((gb_use_tveffect==1)?0:1),max_gb_enable_menu);
    if ((auxCol==255)||(auxCol==2))
    {
     gb_id_menu_cur=id_menu_screen_adjust;
     return; 
    }
    gb_use_tveffect= (auxCol==0)?1:0;   
    //gb_show_osd_main_menu= 0;
    SDLClear();
    gb_id_menu_cur=id_menu_screen_adjust;
    return;
   #endif 
   break;
 }

 gb_show_osd_main_menu= 0;
}



const char carKeys[16]={
 '1','2','3','4',
 'Q','W','E','R',
 'A','S','D','F',
 'Z','X','C','V'
};     

//************************
void ShowVirtualKeyboard()
{
 //int origenX = gb_pos_x_menu+60;
 int origenX = gb_pos_x_virtualKey;
 int destX= origenX;
 //int destY= gb_pos_y_menu+10;
 int destY= gb_pos_y_virtualKey;
 unsigned char contKey=0;

 #ifdef use_lib_hdmi
  for (unsigned char j=0;j<4;j++)
  {
   for (unsigned char i=0;i<4;i++)
   {
    if (gb_cur_key_virtual == contKey)
    {
     OSDprintCharHDMI(carKeys[contKey],destX,destY,ID_COLOR_WHITE,ID_COLOR_BLACK);                          
    }
    else
    {
     OSDprintCharHDMI(carKeys[contKey],destX,destY,ID_COLOR_BLACK,ID_COLOR_WHITE);     
    }
    contKey++;
    destX+= 7;
   }
   destX= origenX;
   destY+= 9;
  } 
 #else 
  for (unsigned char j=0;j<4;j++)
  {
   for (unsigned char i=0;i<4;i++)
   {
    if (gb_cur_key_virtual == contKey)
    {
     SDLprintCharOSD(carKeys[contKey],destX,destY,ID_COLOR_WHITE,ID_COLOR_BLACK);                          
    }
    else
    {
     SDLprintCharOSD(carKeys[contKey],destX,destY,ID_COLOR_BLACK,ID_COLOR_WHITE);
    }
    contKey++;
    destX+= 9;
   }
   destX= origenX;
   destY+= 9;
  } 
 #endif
 //SDL_Flip(gb_osd_sdl_surface);
}

//***********************************************************************************************
//Menu resetear
void ShowTinyResetMenu()
{ 
 unsigned char aSelNum;
 aSelNum= ShowTinyMenu((char *)"   RESET", (char **)gb_reset_menu,0,max_gb_reset_menu);
 if ((aSelNum==255)||(aSelNum==2))
 {  
  gb_id_menu_cur=id_menu_main;
  return;
 }
 CPU_init();
 ResetEmu();
 Loadrom2Flash(gb_current_sel_rom);
 if (aSelNum == 1)
 {
  //ESP.restart();
  //AIRCR_Register = 0x5FA0004;
  watchdog_enable(1, 1);  
  while(1);
 }

 gb_show_osd_main_menu= 0;
}

//**********************************************************
void ShowTinySoundMenu()
{ 
 unsigned char aSelNum;
 aSelNum= ShowTinyMenu((char *)"   SOUND", (char **)gb_sound_menu,0,max_gb_sound_menu);
 if ((aSelNum==255)||(aSelNum==2))
 {  
  gb_id_menu_cur=id_menu_main;
  return;
 }

 switch(aSelNum)
 {
  case 0: //Beep ON OFF
   aSelNum= ShowTinyMenu((char *)"   BEEP", (char **)gb_enable_menu,((gb_use_silence==1)?1:0),max_gb_enable_menu);
   if ((aSelNum==255)||(aSelNum==2))
   {  
    gb_id_menu_cur=id_menu_sound;
    return;
   }
   gb_use_silence= (aSelNum==0)?0:1;
   gb_show_osd_main_menu= 0;
   return;
   break;
  
  case 1: //Frecuency 500 Hz, 250 Hz, 166 Hz, 125 Hz
   unsigned char idSelSound= 1;
   switch (gb_sound_pulse_duration_cont)
   {
    case 1: idSelSound=0; break;//500 hz
    case 2: idSelSound=1; break;//250 hz
    case 3: idSelSound=2; break;//166 hz
    case 4: idSelSound=3; break;//125 hz
    default: idSelSound= 1; break;
   }
   aSelNum= ShowTinyMenu((char *)"  FREQUENCY", (char **)gb_sound_hz_menu,idSelSound,max_gb_sound_hz_menu);
   if ((aSelNum==255)||(aSelNum==4))
   {  
    gb_id_menu_cur=id_menu_sound;
    //gb_show_osd_main_menu= 0;
    return;
   }   
   switch(aSelNum)
   {
    case 0: gb_sound_pulse_duration_cont=1; break; //500 Hz
    case 1: gb_sound_pulse_duration_cont=2; break; //250 Hz
    case 2: gb_sound_pulse_duration_cont=3; break; //166 Hz
    case 3: gb_sound_pulse_duration_cont=4; break; //125 Hz    
   }
   gb_show_osd_main_menu= 0;
   return;
   break;  
 }

 gb_show_osd_main_menu= 0;
}

//************************
void ShowKey4x4()
{
 const char carKeys[16]={
  '1','2','3','4',
  'Q','W','E','R',
  'A','S','D','F',
  'Z','X','C','V'
 };

      
 //int origenX = gb_pos_x_menu+60;
 //int destX= origenX;
 //int destY= gb_pos_y_menu+10;
 int origenX = 100;
 int destX= origenX;
 int destY= 150;

 unsigned char contKey=0;
 for (unsigned char j=0;j<4;j++)
 {
  for (unsigned char i=0;i<4;i++)
  {
   SDLprintCharOSD(carKeys[contKey],destX,destY,ID_COLOR_BLACK,ID_COLOR_WHITE);
   SDLprintCharOSD(gb_keyRemap[contKey],destX+50,destY,ID_COLOR_BLACK,ID_COLOR_WHITE);
   contKey++;
   destX+= 9;
  }
  destX= origenX;
  destY+= 9;
 }
 
 //destX= origenX;
 //destY= gb_pos_y_menu+10 +(5*8);
 origenX= 200;
 destX= origenX; 
 destY= 150; 
 
 for (unsigned char j=0;j<3;j++)
 {
  for (unsigned char i=0;i<9;i++)
  {
   SDLprintCharOSD(' ',destX,destY,ID_COLOR_BLACK,ID_COLOR_WHITE);      
   destX+= 8;
  }
  destX= origenX;
  destY+= 8;
 }
  
 //destX= origenX;
 //destY= gb_pos_y_menu+10 +(5*8); 
 destX= 200;
 destY= 150; 
 SDLprintCharOSD(gb_car_key_left,destX,destY+8,ID_COLOR_BLACK,ID_COLOR_WHITE); //Izquierda
 SDLprintCharOSD(gb_car_key_right,destX+(8*2),destY+8,ID_COLOR_BLACK,ID_COLOR_WHITE); //Derecha
 SDLprintCharOSD(gb_car_key_up,destX+8,destY,ID_COLOR_BLACK,ID_COLOR_WHITE); //Arriba
 SDLprintCharOSD(gb_car_key_down,destX+8,destY+(8*2),ID_COLOR_BLACK,ID_COLOR_WHITE); //Abajo
 SDLprintCharOSD(gb_car_key_a,destX+(8*5),destY+9,ID_COLOR_BLACK,ID_COLOR_WHITE); //A
 SDLprintCharOSD(gb_car_key_b,destX+(8*7),destY+9,ID_COLOR_BLACK,ID_COLOR_WHITE); //B 
 
 //SDL_Flip(gb_osd_sdl_surface);      
} 


//************************
void ShowTinyRemapKeysMenu()
{
 //1234
 //qwer
 //asdf
 //zxcv
 unsigned char salir=0;
 unsigned char idPad,idCar;
 const unsigned char carGamePad[6]={17,16,30,31,'A','B'};
 const unsigned char traducePad[16]={1,2,3,0xC, 4,5,6,0xD, 7,8,9, 0xE, 0xA,0,0xB,0x0F};
 
 
 unsigned char aSelNum, auxCol;  
 
 while (salir==0)
 {
  aSelNum= ShowTinyMenu((char *)"REMAP GAMEPAD", (char **)gb_osd_gamepad,0,max_gb_osd_gamepad);
  if ((aSelNum==255)||(aSelNum==6))
  {   
   gb_id_menu_cur=id_menu_main;
   return;
  }
  idPad= aSelNum; 
  //Serial.printf("Remap Gamepad end\n");
  //fflush(stdout);
 

 //SDLClear(gb_osd_sdl_surface);  
 
  
 //SDLprintText(gb_osd_sdl_surface,"CHIP8(Spittie)by Ackerman",gb_pos_x_menu-(4<<3),gb_pos_y_menu-16,WHITE,BLACK,1);
 //for (int i=0;i<20;i++)
 //{
 // SDLprintChar(gb_osd_sdl_surface,' ',gb_pos_x_menu+(i<<3),gb_pos_y_menu,BLACK,WHITE,1);
 //}
 //SDLprintText(gb_osd_sdl_surface,"Remap Gamepad",gb_pos_x_menu,gb_pos_y_menu,BLACK,WHITE,1);
 //SDLprintText(gb_osd_sdl_surface,gb_osd_gamepad[0],gb_pos_x_menu+100,gb_pos_y_menu,BLACK,WHITE,1);
 
 //SDL_Flip(gb_osd_sdl_surface);

 
  aSelNum= ShowTinyMenu((char *)"  KEY 4x4", (char **)gb_osd_key_4x4,0,max_gb_osd_key_4x4);
  idCar= aSelNum;
  //Serial.printf("idCar %d\n",idCar);
  //fflush(stdout);
  if ((aSelNum!=255)&&(aSelNum!=16))
  {
   for (unsigned char i=0;i<16;i++)
   {
    if (gb_keyRemap[i] == carGamePad[idPad])
    {
     gb_keyRemap[i]= 32;
    }
   }

  
   switch (idPad)
   {
    case 0:     
     gb_id_key_left= traducePad[idCar];
     gb_car_key_left= gb_osd_key_4x4[idCar][0];
     //if (gb_id_key_left == traducePad[idPad]){ gb_id_key_left= 0; gb_car_key_left= 46; }
     if (gb_car_key_right == gb_osd_key_4x4[idCar][0]){ gb_id_key_right= 0; gb_car_key_right= 46; }
     if (gb_car_key_up == gb_osd_key_4x4[idCar][0]){ gb_id_key_up= 0; gb_car_key_up= 46;}
     if (gb_car_key_down == gb_osd_key_4x4[idCar][0]){ gb_id_key_down= 0; gb_car_key_down= 46; }
     if (gb_car_key_a == gb_osd_key_4x4[idCar][0]){ gb_id_key_a= 0; gb_car_key_a= 46; }
     if (gb_car_key_b == gb_osd_key_4x4[idCar][0]){ gb_id_key_b= 0; gb_car_key_b= 46; }
     //Serial.printf("idPad %d gb_id_key_left %d gb_car_key_left %d traducePad %d\n",idPad,gb_id_key_left,gb_car_key_left,traducePad[idPad]);
     //Serial.printf("L %d R %d U %d D %d\n",gb_id_key_left,gb_id_key_right,gb_id_key_up,gb_id_key_down);
     //fflush(stdout);
     break;
    case 1:
     gb_id_key_right= traducePad[idCar];
     gb_car_key_right= gb_osd_key_4x4[idCar][0];
     if (gb_car_key_left == gb_osd_key_4x4[idCar][0]){ gb_id_key_left= 0; gb_car_key_left= 46; }
     //if (gb_id_key_right == traducePad[idPad]){ gb_id_key_right= 0; gb_car_key_right= 46; }
     if (gb_car_key_up == gb_osd_key_4x4[idCar][0]){ gb_id_key_up= 0; gb_car_key_up= 46;}
     if (gb_car_key_down == gb_osd_key_4x4[idCar][0]){ gb_id_key_down= 0; gb_car_key_down= 46; }
     if (gb_car_key_a == gb_osd_key_4x4[idCar][0]){ gb_id_key_a= 0; gb_car_key_a= 46; }
     if (gb_car_key_b == gb_osd_key_4x4[idCar][0]){ gb_id_key_b= 0; gb_car_key_b= 46; }     
     //Serial.printf("idPad %d gb_id_key_left %d gb_car_key_left %d traducePad %d\n",idPad,gb_id_key_left,gb_car_key_left,traducePad[idPad]);
     //Serial.printf("L %d R %d U %d D %d\n",gb_id_key_left,gb_id_key_right,gb_id_key_up,gb_id_key_down);     
     break;
    case 2:
     gb_id_key_up= traducePad[idCar];
     gb_car_key_up= gb_osd_key_4x4[idCar][0];
     if (gb_car_key_left == gb_osd_key_4x4[idCar][0]){ gb_id_key_left= 0; gb_car_key_left= 46; }
     if (gb_car_key_right == gb_osd_key_4x4[idCar][0]){ gb_id_key_right= 0; gb_car_key_right= 46; }
     //if (gb_id_key_up == traducePad[idPad]){ gb_id_key_up= 0; gb_car_key_up= 46;}
     if (gb_car_key_down == gb_osd_key_4x4[idCar][0]){ gb_id_key_down= 0; gb_car_key_down= 46; }
     if (gb_car_key_a == gb_osd_key_4x4[idCar][0]){ gb_id_key_a= 0; gb_car_key_a= 46; }
     if (gb_car_key_b == gb_osd_key_4x4[idCar][0]){ gb_id_key_b= 0; gb_car_key_b= 46; }     
     //Serial.printf("idPad %d gb_id_key_left %d gb_car_key_left %d traducePad %d\n",idPad,gb_id_key_left,gb_car_key_left,traducePad[idPad]);
     //Serial.printf("L %d R %d U %d D %d\n",gb_id_key_left,gb_id_key_right,gb_id_key_up,gb_id_key_down);     
     break;
    case 3:
     gb_id_key_down= traducePad[idCar];
     gb_car_key_down= gb_osd_key_4x4[idCar][0]; 
     if (gb_car_key_left == gb_osd_key_4x4[idCar][0]){ gb_id_key_left= 0; gb_car_key_left= 46; }
     if (gb_car_key_right == gb_osd_key_4x4[idCar][0]){ gb_id_key_right= 0; gb_car_key_right= 46; }
     if (gb_car_key_up == gb_osd_key_4x4[idCar][0]){ gb_id_key_up= 0; gb_car_key_up= 46;}
     //if (gb_id_key_down == traducePad[idPad]){ gb_id_key_down= 0; gb_car_key_down= 46; }
     if (gb_car_key_a == gb_osd_key_4x4[idCar][0]){ gb_id_key_a= 0; gb_car_key_a= 46; }
     if (gb_car_key_b == gb_osd_key_4x4[idCar][0]){ gb_id_key_b= 0; gb_car_key_b= 46; }     
     //Serial.printf("idPad %d gb_id_key_left %d gb_car_key_left %d traducePad %d\n",idPad,gb_id_key_left,gb_car_key_left,traducePad[idPad]);
     //Serial.printf("L %d R %d U %d D %d\n",gb_id_key_left,gb_id_key_right,gb_id_key_up,gb_id_key_down);     
     break;
    case 4:
     gb_id_key_a= traducePad[idCar];
     gb_car_key_a= gb_osd_key_4x4[idCar][0]; 
     if (gb_car_key_left == gb_osd_key_4x4[idCar][0]){ gb_id_key_left= 0; gb_car_key_left= 46; }
     if (gb_car_key_right == gb_osd_key_4x4[idCar][0]){ gb_id_key_right= 0; gb_car_key_right= 46; }
     if (gb_car_key_up == gb_osd_key_4x4[idCar][0]){ gb_id_key_up= 0; gb_car_key_up= 46;}
     if (gb_id_key_down == traducePad[idPad]){ gb_id_key_down= 0; gb_car_key_down= 46; }
     //if (gb_car_key_a == gb_osd_key_4x4[idCar][0]){ gb_id_key_a= 0; gb_car_key_a= 46; }
     if (gb_car_key_b == gb_osd_key_4x4[idCar][0]){ gb_id_key_b= 0; gb_car_key_b= 46; }     
     //Serial.printf("idPad %d gb_id_key_left %d gb_car_key_left %d traducePad %d\n",idPad,gb_id_key_left,gb_car_key_left,traducePad[idPad]);
     //Serial.printf("L %d R %d U %d D %d\n",gb_id_key_left,gb_id_key_right,gb_id_key_up,gb_id_key_down);     
     break;     
    case 5:
     gb_id_key_b= traducePad[idCar];
     gb_car_key_b= gb_osd_key_4x4[idCar][0]; 
     if (gb_car_key_left == gb_osd_key_4x4[idCar][0]){ gb_id_key_left= 0; gb_car_key_left= 46; }
     if (gb_car_key_right == gb_osd_key_4x4[idCar][0]){ gb_id_key_right= 0; gb_car_key_right= 46; }
     if (gb_car_key_up == gb_osd_key_4x4[idCar][0]){ gb_id_key_up= 0; gb_car_key_up= 46;}
     if (gb_id_key_down == traducePad[idPad]){ gb_id_key_down= 0; gb_car_key_down= 46; }
     if (gb_car_key_a == gb_osd_key_4x4[idCar][0]){ gb_id_key_a= 0; gb_car_key_a= 46; }
     //if (gb_car_key_b == gb_osd_key_4x4[idCar][0]){ gb_id_key_b= 0; gb_car_key_b= 46; }
     //Serial.printf("idPad %d gb_id_key_left %d gb_car_key_left %d traducePad %d\n",idPad,gb_id_key_left,gb_car_key_left,traducePad[idPad]);
     //Serial.printf("L %d R %d U %d D %d\n",gb_id_key_left,gb_id_key_right,gb_id_key_up,gb_id_key_down);     
     break;     
   }   
   gb_keyRemap[aSelNum]= carGamePad[idPad];
  }
  else
  {
   salir= 1;
   gb_id_menu_cur=id_menu_remap_keys;
   return;
  }
 }
  
 //SDL_Flip(gb_osd_sdl_surface);
 
 //gb_show_osd_main_menu= 0; 
 gb_id_menu_cur=id_menu_main;
}

//***********************************
void ShowTinyKeyboardVirtual()
{
 unsigned char aSelNum;     
 aSelNum = ShowTinyMenu((char *)" KEY VIRTUAL", (char **)gb_enable_menu,((gb_show_key_virtual==0)?1:0),max_gb_enable_menu);
 if((aSelNum==255)||(aSelNum==2))
 {
  gb_id_menu_cur=id_menu_main;
  return;
 }
 switch (aSelNum)
 {
  case 0: gb_show_key_virtual= 1; break;
  case 1: gb_show_key_virtual= 0; break;
 } 
 
 gb_show_osd_main_menu= 0;
}

//************************************************************************************
void ShowTinyHelpMenu()
{ 
 ShowTinyMenu((char *)"    HELP", (char **)gb_help_menu,0,max_gb_help_menu);
 gb_show_osd_main_menu= 0;
}

//************************************************************************************
void ShowTinyRAM()
{
 char cad[3][32];
 char *ptrCad[3]={cad[0],cad[1],cad[2]};
 unsigned int auxFree;

 sprintf(cad[0],"INIT: %d bytes (%d KB)",gb_ramfree_ini,(gb_ramfree_ini>>10));
 sprintf(cad[1],"SETUP: %d bytes (%d KB)",gb_ramfree_setupEnd,(gb_ramfree_setupEnd>>10));
 auxFree= rp2040.getFreeHeap(); 
 sprintf(cad[2],"NOW: %d bytes (%d KB)",auxFree,(auxFree>>10));

 ShowTinyMenu((char *)"  RAM FREE",ptrCad, 0, 3);
 
 gb_id_menu_cur= id_menu_main;
 gb_show_osd_main_menu=0;  
}

//************************************************************************************
void ShowTinyAboutMenu()
{
 ShowTinyMenu((char *)"   ABOUT", (char **)gb_about_menu,0,max_gb_about_menu); 
 gb_show_osd_main_menu= 0; 
}

//***********************************
void ShowTinyMainMenu()
{
 //Load ROM
 //Speed
 //Screen Adjust
 //Gamepad Remap
 //Keyboard virtual
 //Reset
 //Help
 //RAM
 //About
 //Exit Menu
   
 unsigned char aSelNum;

 //ShowTinyMenu("MAIN MENU",gb_main_menu,max_gb_main_menu,-1); break;
 aSelNum = ShowTinyMenu((char *)"  MAIN MENU", (char **)gb_main_menu, 0 ,max_gb_main_menu);

 if ((aSelNum==255)||(aSelNum==10))
 {
  gb_show_osd_main_menu= 0;
  return;
 }

 switch (aSelNum)
 {
  case 0: gb_id_menu_cur= id_menu_load_rom; break;
  case 1: gb_id_menu_cur= id_menu_speed; break;
  case 2: gb_id_menu_cur= id_menu_screen_adjust; break;
  case 3: gb_id_menu_cur= id_menu_remap_keys; break;
  case 4: gb_id_menu_cur= id_menu_keyboard_virtual; break;
  case 5: gb_id_menu_cur= id_menu_sound; break;
  case 6: gb_id_menu_cur= id_menu_reset; break;
  case 7: gb_id_menu_cur= id_menu_help; break;
  case 8: gb_id_menu_cur= id_menu_ram; break;
  case 9: gb_id_menu_cur= id_menu_about; break;
  default: gb_show_osd_main_menu= 0; break;  
 }
}

//************************************************************************************
//Very small tiny osd
void do_tinyOSD() 
{
 unsigned char sonidoAntes;
 unsigned char aSelNum;
// #ifdef use_lib_not_use_ps2keyboard
// #else
//  if (checkAndCleanKey(KEY_F1))
//  {
//   gb_show_osd_main_menu= 1;
//   return;
//  }
// #endif 

 if (gb_show_osd_main_menu == 1)
 {
  sonidoAntes= gbVolMixer;
  gbVolMixer= 0;

  OSDClear();
  while (gb_show_osd_main_menu==1)
  {
   switch (gb_id_menu_cur)
   {
    case id_menu_main: ShowTinyMainMenu(); break;
    case id_menu_load_rom: ShowTinyROMMenu(); break;
    case id_menu_speed: ShowTinySpeedMenu(); break;
    case id_menu_screen_adjust: ShowTinyScreenAdjustMenu(); break;
    case id_menu_remap_keys:
     gb_show_key4x4= 1;
     ShowTinyRemapKeysMenu();
     gb_show_key4x4= 0;
     break;
    case id_menu_keyboard_virtual: ShowTinyKeyboardVirtual(); break;
    case id_menu_sound: ShowTinySoundMenu(); break;
    case id_menu_reset: ShowTinyResetMenu(); break;
    case id_menu_help: ShowTinyHelpMenu(); break;
    case id_menu_ram: ShowTinyRAM(); break;
    case id_menu_about: ShowTinyAboutMenu(); break;
    default: gb_show_osd_main_menu=1; break;
   }//fin switch
   
   #ifdef use_lib_hdmi
    DumpVideoHDMI();
   #endif
   
  }//fin while
 }//fin if show osd

 OSDClear();
 gb_id_menu_cur= id_menu_main;
 gb_show_osd_main_menu=0;  
 SDL_DumpVGA();
 gbVolMixer= sonidoAntes;
 
//  aSelNum = ShowTinyMenu("MAIN MENU",gb_main_menu,max_gb_main_menu,-1);
//  switch (aSelNum)
//  {
//   case 0: ShowTinyROMMenu(); break;
//   case 1: ShowTinySpeedMenu(); break;
//   case 2: ShowTinyScreenAdjustMenu(); break;
//   case 3:
//    gb_show_key4x4= 1;
//    ShowTinyRemapKeysMenu();    
//    gb_show_key4x4= 0;    
//    break;   
//   case 4:
//    ShowTinyKeyboardVirtual();
//    break;
//   case 5: ShowTinyResetMenu(); break;
//   default: break;
//  }
//  //SDLClear(screen); //Borramos pantalla
//  SDL_DumpVGA();
//  gbVolMixer= sonidoAntes;
// }
}


//****************************
void SendVirtualKey(char col, char row)
{
 unsigned char auxCar;
 const unsigned char traducePad[16]={1,2,3,0xC, 4,5,6,0xD, 7,8,9, 0xE, 0xA,0,0xB,0x0F};
 if ((row<0)||(row>3)||(col<0)||(col>3))
 {
  return;
 }
 
 auxCar= (row*4)+col;

 key[gb_before_key_virtual]= 0;
 key[traducePad[auxCar]]= 1;
 gb_before_key_virtual= traducePad[auxCar];
 
 //Serial.printf("SendVirtualKey %d %d\n",auxCar,traducePad[auxCar]);
 //fflush(stdout);
}

//********************************
void SDL_keys_virtual_poll()
{
 unsigned char movimiento=0;
 unsigned char accion=0;

 #ifdef use_lib_not_use_ps2keyboard
  gb_atari_db9 = 0;
  if (digitalRead(ATARI_DB9_UP_PIN) == LOW){ gb_atari_db9 |= 0x01; }
  if (digitalRead(ATARI_DB9_DOWN_PIN) == LOW){ gb_atari_db9 |= 0x02; }
  if (digitalRead(ATARI_DB9_LEFT_PIN) == LOW){ gb_atari_db9 |= 0x04; }
  if (digitalRead(ATARI_DB9_RIGHT_PIN) == LOW){ gb_atari_db9 |= 0x08; }
  if (digitalRead(ATARI_DB9_A_PIN) == LOW){ gb_atari_db9 |= 0x10; }
  if (digitalRead(ATARI_DB9_B_PIN) == LOW){ gb_atari_db9 |= 0x20; }
  
  //if (
  //    (((gb_atari_db9>>5) & 0x01) == 1)
  //    &&
  //    (((gb_atari_db9>>4) & 0x01) == 1)
  //   )
  if (
     (((gb_atari_db9>>gb_use_gamepad_osd_menu_button0) & 0x01) == 1)
      &&
      (((gb_atari_db9>>gb_use_gamepad_osd_menu_button1) & 0x01) == 1)
     )
  {
   if (gb_ini_osd_main_menu == 1)
   {
    if ((millis() - gb_ini_osd_main_menu_time) >= gb_ms_gamepad_osd_menu)
    {
     gb_show_osd_main_menu= 1; //500 ms A B presionado saca menu
     gb_ini_osd_main_menu = 0;
    }
   }
   else
   {
    gb_ini_osd_main_menu = 1; //Arranca timer
    gb_ini_osd_main_menu_time= millis();
   }   
  }
  else
  {
   gb_ini_osd_main_menu = 0;    
  }

  gb_key_cur_virtual_left = ((gb_atari_db9 & 0x04) == 0x04) ? 1 : 0;
  gb_key_cur_virtual_right = ((gb_atari_db9 & 0x08) == 0x08) ? 1 : 0;
  gb_key_cur_virtual_up = ((gb_atari_db9 & 0x01) == 0x01) ? 1 : 0;
  gb_key_cur_virtual_down = ((gb_atari_db9 & 0x02) == 0x02) ? 1 : 0;
  gb_key_cur_virtual_a = ((gb_atari_db9 & 0x10) == 0x10) ? 1 : 0;

 if (gb_key_cur_virtual_left != gb_key_before_virtual_left)
 {
  gb_key_before_virtual_left = gb_key_cur_virtual_left;
  if (gb_key_cur_virtual_left == 1)
  {
   gb_cur_col_key_virtual--;
   movimiento=1;
  }
 }
 if (gb_key_cur_virtual_right != gb_key_before_virtual_right)
 {
  gb_key_before_virtual_right = gb_key_cur_virtual_right;
  if (gb_key_cur_virtual_right==1)
  {
   gb_cur_col_key_virtual++; 
   movimiento=1;
  }
 }
 if (gb_key_cur_virtual_up != gb_key_before_virtual_up)
 {
  gb_key_before_virtual_up = gb_key_cur_virtual_up;
  if (gb_key_cur_virtual_up == 1)
  {
   gb_cur_row_key_virtual--;
   movimiento=1;
  }
 }
 if (gb_key_cur_virtual_down != gb_key_before_virtual_down)
 {
  gb_key_before_virtual_down = gb_key_cur_virtual_down;
  if (gb_key_cur_virtual_down == 1)
  {
   gb_cur_row_key_virtual++;
   movimiento=1;
  }
 }


 if (movimiento == 1)
 {
    //Serial.printf("row %d col %d\n",gb_cur_row_key_virtual,gb_cur_col_key_virtual);
    if (gb_cur_col_key_virtual>3){ gb_cur_col_key_virtual=3; }
    if (gb_cur_col_key_virtual<0){ gb_cur_col_key_virtual=0; }
    if (gb_cur_row_key_virtual>3){ gb_cur_row_key_virtual=3; }
    if (gb_cur_row_key_virtual<0){ gb_cur_row_key_virtual=0; }
    gb_cur_key_virtual= (gb_cur_row_key_virtual*4) +gb_cur_col_key_virtual;
    //Serial.printf("gb_cur_key_virtual %d\n",gb_cur_key_virtual);
    //fflush(stdout);      
 }
 
 if (gb_key_cur_virtual_a == 1)
 {
  SendVirtualKey(gb_cur_col_key_virtual,gb_cur_row_key_virtual);               
 }
 else
 {
   key[gb_before_key_virtual]= 0;
 } 

 #else
  gb_key_cur_virtual_left = checkKey(KEY_CURSOR_LEFT); //(keymap[KEY_CURSOR_LEFT] == 0)?1:0; //Left
  gb_key_cur_virtual_right = checkKey(KEY_CURSOR_RIGHT); //(keymap[KEY_CURSOR_RIGHT] == 0)?1:0; //Right
  gb_key_cur_virtual_up = checkKey(KEY_CURSOR_UP); //(keymap[KEY_CURSOR_UP] == 0)?1:0; //Up
  gb_key_cur_virtual_down = checkKey(KEY_CURSOR_DOWN); //(keymap[KEY_CURSOR_DOWN] == 0)?1:0; //Down
  gb_key_cur_virtual_a = checkKey(PS2_KC_KP0); //(keymap[PS2_KC_KP0] == 0)?1:0; //A 0 numerico


  if (gb_key_cur_virtual_left != gb_key_before_virtual_left)
  {
   gb_key_before_virtual_left = gb_key_cur_virtual_left;
   if (gb_key_cur_virtual_left == 1)
   {
    gb_cur_col_key_virtual--;
    movimiento=1;
   }
  }
  if (gb_key_cur_virtual_right != gb_key_before_virtual_right)
  {
   gb_key_before_virtual_right = gb_key_cur_virtual_right;
   if (gb_key_cur_virtual_right==1)
   {
    gb_cur_col_key_virtual++; 
    movimiento=1;
   }
  }
  if (gb_key_cur_virtual_up != gb_key_before_virtual_up)
  {
   gb_key_before_virtual_up = gb_key_cur_virtual_up;
   if (gb_key_cur_virtual_up == 1)
   {
    gb_cur_row_key_virtual--;
    movimiento=1;
   }
  }
  if (gb_key_cur_virtual_down != gb_key_before_virtual_down)
  {
   gb_key_before_virtual_down = gb_key_cur_virtual_down;
   if (gb_key_cur_virtual_down == 1)
   {
    gb_cur_row_key_virtual++;
    movimiento=1;
   }
  }


  if (movimiento == 1)
  {
   if (gb_cur_col_key_virtual>3){ gb_cur_col_key_virtual=3; }
   if (gb_cur_col_key_virtual<0){ gb_cur_col_key_virtual=0; }
   if (gb_cur_row_key_virtual>3){ gb_cur_row_key_virtual=3; }
   if (gb_cur_row_key_virtual<0){ gb_cur_row_key_virtual=0; }
   gb_cur_key_virtual= (gb_cur_row_key_virtual*4) +gb_cur_col_key_virtual;
   //Serial.printf("gb_cur_key_virtual %d\n",gb_cur_key_virtual);
   //fflush(stdout);      
  }
 
  if (gb_key_cur_virtual_a == 1)
  {
   //Serial.printf("SendVirtualKey %d %d\n",gb_cur_col_key_virtual,gb_cur_row_key_virtual);    
   SendVirtualKey(gb_cur_col_key_virtual,gb_cur_row_key_virtual);               
  }
  else
  {
   //Serial.printf("gb_before_key_virtual %d\n",gb_before_key_virtual);
   key[gb_before_key_virtual]= 0;
  }
 #endif   
}

//Lectura teclado
void SDL_keys_poll()
{
 #ifdef use_lib_not_use_ps2keyboard
  gb_atari_db9 = 0;
  if (digitalRead(ATARI_DB9_UP_PIN) == LOW){ gb_atari_db9 |= 0x01; }
  if (digitalRead(ATARI_DB9_DOWN_PIN) == LOW){ gb_atari_db9 |= 0x02; }
  if (digitalRead(ATARI_DB9_LEFT_PIN) == LOW){ gb_atari_db9 |= 0x04; }
  if (digitalRead(ATARI_DB9_RIGHT_PIN) == LOW){ gb_atari_db9 |= 0x08; }
  if (digitalRead(ATARI_DB9_A_PIN) == LOW){ gb_atari_db9 |= 0x10; }
  if (digitalRead(ATARI_DB9_B_PIN) == LOW){ gb_atari_db9 |= 0x20; }
  
  //if (
  //    (((gb_atari_db9>>5) & 0x01) == 1)
  //    &&
  //    (((gb_atari_db9>>4) & 0x01) == 1)
  //   )
  if (
     (((gb_atari_db9>>gb_use_gamepad_osd_menu_button0) & 0x01) == 1)
      &&
      (((gb_atari_db9>>gb_use_gamepad_osd_menu_button1) & 0x01) == 1)
     )   
  {
   if (gb_ini_osd_main_menu == 1)
   {
    if ((millis() - gb_ini_osd_main_menu_time) > 500)
    {
     gb_show_osd_main_menu= 1; //500 ms A B presionado saca menu
     gb_ini_osd_main_menu = 0;
    }
   }
   else
   {
    gb_ini_osd_main_menu = 1; //Arranca timer
    gb_ini_osd_main_menu_time= millis();
   }   
  }
  else
  {
   gb_ini_osd_main_menu = 0;    
  }
  //if (gb_atari_db9 != gb_atari_db9_prev)
  //{  
   //gb_atari_db9_prev= gb_atari_db9;
  // Serial.printf("%02X\n",gb_atari_db9);
  //}

  //Gamepad
  key[gb_id_key_left] =  ((gb_atari_db9 & 0x04) == 0x04) ? 1 : 0; //left
  key[gb_id_key_right] =  ((gb_atari_db9 & 0x08) == 0x08) ? 1 : 0; //right  
  key[gb_id_key_up] =  ((gb_atari_db9 & 0x01) == 0x01) ? 1 : 0; //up
  key[gb_id_key_down] =  ((gb_atari_db9 & 0x02) == 0x02) ? 1 : 0; //down  
  key[gb_id_key_a] =  ((gb_atari_db9 & 0x10) == 0x10) ? 1 : 0; //A
  key[gb_id_key_b] =  ((gb_atari_db9 & 0x20) == 0x20) ? 1 : 0; //B

 #else
  key[0x02] = checkKey(PS2_KC_2); //(keymap[PS2_KC_2] == 0)?1:0; //2
  key[0x01] = checkKey(PS2_KC_1); //(keymap[PS2_KC_1] == 0)?1:0; //1
  key[0x03] = checkKey(PS2_KC_3); //(keymap[PS2_KC_3] == 0)?1:0; //3
  key[0x0C] = checkKey(PS2_KC_4); //(keymap[PS2_KC_4] == 0)?1:0; //4
  key[0x04] = checkKey(PS2_KC_Q); //(keymap[PS2_KC_Q] == 0)?1:0; //q
  key[0x05] = checkKey(PS2_KC_W); //(keymap[PS2_KC_W] == 0)?1:0; //w
  key[0x06] = checkKey(PS2_KC_E); //(keymap[PS2_KC_E] == 0)?1:0; //e  
  key[0x0D] = checkKey(PS2_KC_R); //(keymap[PS2_KC_R] == 0)?1:0; //r
  key[0x07] = checkKey(PS2_KC_A); //(keymap[PS2_KC_A] == 0)?1:0; //a
  key[0x08] = checkKey(PS2_KC_S); //(keymap[PS2_KC_S] == 0)?1:0; //s
  key[0x09] = checkKey(PS2_KC_D); //(keymap[PS2_KC_D] == 0)?1:0; //d
  key[0x0E] = checkKey(PS2_KC_F); //(keymap[PS2_KC_F] == 0)?1:0; //f
  key[0x0A] = checkKey(PS2_KC_Z); //(keymap[PS2_KC_Z] == 0)?1:0; //z
  key[0x00] = checkKey(PS2_KC_X); //(keymap[PS2_KC_X] == 0)?1:0; //x
  key[0x0B] = checkKey(PS2_KC_C); //(keymap[PS2_KC_C] == 0)?1:0; //c
  key[0x0F] = checkKey(PS2_KC_V); //(keymap[PS2_KC_V] == 0)?1:0; //v  

  //if (checkKey(KEY_CURSOR_LEFT)){ key[0x04] = 1; }
  //if (checkKey(KEY_CURSOR_UP)){ key[0x05] = 1; }
  //if (checkKey(KEY_CURSOR_RIGHT)){ key[0x06] = 1; }
  //if (checkKey(KEY_CURSOR_DOWN)){ key[0x08] = 1; }
  //if (checkKey(KEY_BACKSPACE)){ key[0x0F] = 1; }  
  
  //if (keymap[KEY_CURSOR_LEFT] == 0)
  // key[0x04] = 1;
  //if (keymap[KEY_CURSOR_UP] == 0)
  // key[0x05] = 1;
  //if (keymap[KEY_CURSOR_RIGHT] == 0)
  // key[0x06] = 1;
  //if (keymap[KEY_CURSOR_DOWN] == 0)
  // key[0x08] = 1; 
  //if (keymap[KEY_BACKSPACE] == 0)
  // key[0x0F] = 1;


  //Gamepad
  if (checkKey(KEY_CURSOR_LEFT)==1){ key[gb_id_key_left]= 1; } //(keymap[KEY_CURSOR_LEFT] == 0)?1:0; //left
  if (checkKey(KEY_CURSOR_RIGHT)==1){ key[gb_id_key_right]= 1; } //(keymap[KEY_CURSOR_RIGHT] == 0)?1:0; //right  
  if (checkKey(KEY_CURSOR_UP)==1){ key[gb_id_key_up]= 1; } //(keymap[KEY_CURSOR_UP] == 0)?1:0; //up
  if (checkKey(KEY_CURSOR_DOWN)==1){ key[gb_id_key_down]= 1; } //(keymap[KEY_CURSOR_DOWN] == 0)?1:0; //down  
  if (checkKey(PS2_KC_KP0)==1){ key[gb_id_key_a]= 1; } //(keymap[PS2_KC_KP0] == 0)?1:0;   //A 0 numerico
  if (checkKey(PS2_KC_KP_DOT)==1){ key[gb_id_key_b]= 1; } //(keymap[PS2_KC_KP_DOT] == 0)?1:0; //B . numerico  
 #endif  
}


//*****************************
void ResetEmu()
{
 memset(V,0,16);
 memset(stack,0,16);
 memset(key,0,16);
 memset(ram,0,4096);
 memset(gfx,0,sizeof gfx);
 memcpy(ram,chip8_fontset,80);
}

//Init
void CPU_init()
{//Initialize 
 PC = 0x200;
 opcode = 0;
 I = 0;
 SP = 0;
 draw = 0;
 delay_t = 0;
 sound_t = 0;
}

//*********************************************
void CPU_loop()
{
 opcode = ram[PC + 0];
 opcode <<= 8;
 opcode |= ram[PC + 1];

 switch(opcode & 0xF000)
 {      
  case 0x0000:
   {
    switch(opcode & 0x00FF) 
    {
     case 0x00E0: 
      {                         
       memset(gfx,0,sizeof gfx);
       draw = 1;
       PC += 2;
      }
      break;
    
   case 0X00EE:
      {
     SP--;
     PC = stack[SP];
     PC += 2;
    }
    break;
          
   default:
      {
     #ifdef use_lib_log_serial
      Serial.printf("Unknown opcode\n");
     #endif
    }
    break;
    }
   }
   break;
      
  case 0x1000: 
   {
  PC = opcode & 0X0FFF;
   }
   break;
      
  case 0x2000:
   {
  stack[SP] = PC;
  SP++;
  PC = opcode & 0x0FFF;
   }
   break;

  case 0x3000:
   {
  if (V[(opcode & 0x0F00) >> 8] == (opcode & 0x00FF)) {
   PC += 4;
  } 
    else {
   PC += 2;
  }
   }
   break;
      
  case 0x4000:
   {
  if (V[(opcode & 0x0F00) >> 8] != ((opcode & 0x00FF))) {
   PC += 4;
  }
    else {
   PC += 2;
  }
   }
   break;
      
  case 0x5000: 
   {
  if (V[(opcode & 0x0F00) >> 8] == V[(opcode & 0x00F0) >> 4]) {
   PC += 4;
  }
    else {
   PC += 2;
  }
   }
   break;
      
  case 0x6000: 
   {
  V[(opcode & 0x0F00) >> 8] = opcode & 0x00FF;
  PC += 2;
   }
   break;
    
  case 0x7000: 
   {
  V[(opcode & 0x0F00) >> 8] += opcode & 0x00FF;
  PC += 2;
   }
   break;
      
  case 0x8000: 
   {
  switch(opcode & 0x000F) {
   case 0x0000:
      {
     V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4];
     PC += 2;
    }
    break;
          
   case 0x0001:
      {
     V[(opcode & 0x0F00) >> 8] |= V[(opcode & 0x00F0) >> 4];
     PC += 2;
    }
    break;
          
   case 0x0002:
      {
     V[(opcode & 0x0F00) >> 8] &= V[(opcode & 0x00F0) >> 4];
     PC += 2;
    }
    break;
          
   case 0x0003:
      {
     V[(opcode & 0x0F00) >> 8] ^= V[(opcode & 0x00F0) >> 4];
     PC += 2;
    }
    break;
        
   case 0x0004:
      {
     if (V[(opcode & 0x00F0) >> 4] > (0xFF - V[(opcode & 0x0F00) >> 8])) {
    V[0xF] = 1;
     }
       else {
    V[0xF] = 0;
     }
     V[(opcode & 0x0F00) >> 8] += V[(opcode & 0x00F0) >> 4];
     PC += 2;
    }
    break;
          
   case 0x0005:
      {
     if (V[(opcode & 0x00F0) >> 4] > V[(opcode & 0x0F00) >> 8]) {
    V[0xF] = 0;
     }
       else {
    V[0xF] = 1;
     }
     V[(opcode & 0x0F00) >> 8] -= V[(opcode & 0x00F0) >> 4];
     PC += 2;
      }
    break;
          
          case 0x0006: {
            V[0xF] = V[(opcode & 0x0F00) >> 8] & 0x1;
            V[(opcode & 0x0F00) >> 8] >>= 1;
            PC += 2;
          }
          break;
          
          case 0x0007: {
            if (V[(opcode & 0x0F00) >> 8] > V[(opcode & 0x00F0) >> 4]) {
              V[0xF] = 0; 
            } else {
              V[0xF] = 1;
            }
            V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4] - V[(opcode & 0x0F00) >> 8];
            PC += 2;
          }
          break;
          
          case 0x000E: {
            V[0xF] = V[(opcode & 0x0F00) >> 8] >> 7;
            V[(opcode & 0x0F00) >> 8] <<= 1;
            PC += 2;
          }
          break;
          
          default: {
            #ifdef use_lib_log_serial
             Serial.printf("Unknown opcode\n");
            #endif 
            
          }
          break;
        }
      }
      break;
      
      case 0x9000: {
        if (V[(opcode & 0x0F00) >> 8] != V[(opcode & 0x00F0) >> 4]) {
          PC += 4;
        } else {
          PC += 2;
        }
      }
      break;
      
      case 0xA000: {
        I = opcode & 0x0FFF;
        PC += 2;
      }
      break;
      
      case 0xB000: {
        PC = (opcode & 0x0FFF) + V[0x0];
      }
      break;
      
      case 0xC000: {
        V[(opcode & 0x0F00) >> 8] = (rand() % 0xFF) & (opcode & 0x00FF);
        PC += 2;
      }
      break;
      
      case 0xD000: {
        uint8_t x = V[(opcode & 0x0F00) >> 8];
        uint8_t y = V[(opcode & 0x00F0) >> 4];
        uint8_t n = opcode & 0x000F;
        uint8_t pixel;
        
        V[0xF] = 0;     
        
        for(int i=0; i < n; i++) 
            {
         pixel = ram[I + i];
         for(int j=0; j < 8; j++) 
             {
          if((pixel & (0x80 >> j)) != 0) 
              {
               if (((x+j)< max_gfx_row) && ((y+i)< max_gfx_col))
               {
           if(gfx[x+j][y+i] == 1) 
                {
            V[0xF] = 1; 
           }
           gfx[x+j][y+i] ^= 1;
               }
              }
         }
        }
        
        draw = 1;
        PC += 2;
      }
      break;
      
      case 0xE000: {
        switch(opcode & 0x00FF) {
          case 0x009E: {
            if (key[V[(opcode & 0x0F00) >> 8]] == 1) {
              key[V[(opcode & 0x0F00) >> 8]] = 0;
              PC += 4;
            } else {
              PC += 2;
            }
          }
          break;
            
          case 0x00A1 : {
            if (key[V[(opcode & 0x0F00) >> 8]] != 1) {
              PC += 4;
            } else {
              PC += 2;
            }
          }
          break;
          
          default: {
            #ifdef use_lib_log_serial
             Serial.printf("Unknown opcode\n");
            #endif 
            
          }
          break;
        }
      }
      break;
      
      
      case 0xF000: {
        switch(opcode & 0x00FF) {
          case 0x0007: {
            V[(opcode & 0x0F00) >> 8] = delay_t;
            PC += 2;
          }
          break;
          
          case 0x000A: {
            int keypress = 0;
            
            for (int i=0; i < 16; ++i) {
              if (key[i] != 0) {
                key[i] = 0;
                V[(opcode & 0x0F00) >> 8] = i;
                keypress = 1;
              }
            }
            
            if (keypress == 0) {
              break;
            }
            
            PC += 2;
          }
          break;
          
          case 0x0015: {
            delay_t = V[(opcode & 0x0F00) >> 8];
            PC += 2;
          }
          break;
          
          case 0x0018: {
            sound_t = V[(opcode & 0x0F00) >> 8];
            PC += 2;
          }
          break;
          
          case 0x001E: {
            if(I + V[(opcode & 0x0F00) >> 8] > 0xFFF) {
              V[0xF] = 1;
            } else {
              V[0xF] = 0;
            }
            I += V[(opcode & 0x0F00) >> 8];
            PC += 2;
          }
          break;
          
          case 0x0029: {
            I = V[(opcode & 0x0F00) >> 8] * 0x5;
            PC += 2;
          }
          break;
          
          case 0x0033: {
            ram[I] = V[(opcode & 0x0F00) >> 8] / 100;
            ram[I + 1] = (V[(opcode & 0x0F00) >> 8] / 10) % 10;
            ram[I + 2] = (V[(opcode & 0x0F00) >> 8] % 100) % 10;
            PC += 2;
          }
          break;
          
          case 0x0055: {
            for (int i=0; i<=((opcode & 0x0F00) >> 8); ++i) {
              ram[I + i] = V[i];
            }
            // Original interpreter?
            I += ((opcode & 0x0F00) >> 8) + 1;
            PC += 2;
          }
          break;
          
          case 0x0065: {
            for (int i=0; i<=((opcode & 0x0F00) >> 8); ++i) {
              V[i] = ram[I + i];
            }
            // Original interpreter?
            I += ((opcode & 0x0F00) >> 8) + 1;
            PC += 2;
          }
          break;
          
          default: {
            #ifdef use_lib_log_serial
             Serial.printf("Unknown opcode\n");
            #endif 
            
          }
          break;
        }
      } 
      break;
      
      default: {
        #ifdef use_lib_log_serial
         Serial.printf("Unknown opcode\n");
        #endif
        
      }
      break;
    }
}


//****************************
//unsigned int gb_bw32[2]={0,0x3F3F3F3F};
unsigned char gb_bw[2]={0x00,0x3F};
unsigned char gb_bw_hdmi[2]={0x00,0xFF};
void SDL_DumpVGA()
{ 
 //unsigned int *ptrvga32= (unsigned int *)&vga_data_array[0];
 unsigned char aux;
 unsigned int ofsX,ofsY,calcOfs320;
 unsigned int destX,destY,offsetVGA,offsetHDMI; //,aux32;
 //SDLClear(screen);
 ofsY= gb_add_offset_y; 
 for (unsigned int j=0; j<32; j++)
 {
  ofsX= gb_screen_xOffset + gb_add_offset_x;
  for (unsigned int i=0; i<64; i++)  
  {
   //JJ aux= (gfx[i][j] == 1)?255:0;
   //JJ vga.dotFast((ofsX+i),(ofsY+j),aux);  //x
   //JJ vga.dotFast((ofsX+i+1),(ofsY+j),aux);//x+1
   //JJ vga.dotFast((ofsX+i),ofsY+j+1,aux);  //x, y
   //JJ vga.dotFast((ofsX+i+1),ofsY+j+1,aux);//x+1,y+1
   
//   aux= (gfx[i][j] == 1)?gb_const_colorNormal[1]:gb_const_colorNormal[0];      
   //aux= (gfx[i][j] == 1)?0xFF:0;
   aux= gb_bw_hdmi[(gfx[i][j])];
   //dibujo[((j+gb_add_offset_y)*320) + (i+gb_add_offset_x)]= aux;
   #ifdef use_lib_hdmi
    destY= (j+ofsY);    
    destX= (ofsX+i);
    offsetHDMI= (destY<<8)+(destY<<6) + destX;

    if (gb_use_tiled==1)
    {       
     hdmi_data_array[offsetHDMI]= aux;
     hdmi_data_array[(offsetHDMI + 1)]= aux;
     offsetHDMI+=320;
     hdmi_data_array[offsetHDMI]= aux;
     hdmi_data_array[(offsetHDMI + 1)]= aux;     
    }
    else
    {
     hdmi_data_array[offsetHDMI]= aux;
     hdmi_data_array[offsetHDMI+1]= aux;
     hdmi_data_array[offsetHDMI+2]= aux;
     offsetHDMI+= 320;
     hdmi_data_array[offsetHDMI]= aux;
     hdmi_data_array[offsetHDMI+1]= aux;
     hdmi_data_array[offsetHDMI+2]= aux;
     offsetHDMI+= 320;
     hdmi_data_array[offsetHDMI]= aux;
     hdmi_data_array[offsetHDMI+1]= aux;
     hdmi_data_array[offsetHDMI+2]= aux;     
    }
   
    //dibujo[((j+ofsY)*320) + (ofsX+i)]= aux;
    //dibujo[((j+ofsY)*320) + (ofsX+i+1)]= aux;
   
    //dibujo[((j+ofsY+1)*320) + (ofsX+i)]= aux;
    //dibujo[((j+ofsY+1)*320) + (ofsX+i+1)]= aux;

    ofsX+=2;
   #else
    #ifdef use_lib_vga    
     //VGA (Pendiente de optimizar, esta fatal)
     //aux= aux&0x07;
     aux= aux&0x01;
     destX= (ofsX+i);
     destY= (j+ofsY);
     //drawPixel(destX,destY,aux);
     //drawPixel((destX+1),destY,aux);
     //drawPixel((destX+2),destY,aux);
     //drawPixel((destX+3),destY,aux);

     //aux= (aux==0)?0:0xFF;
     //aux32= gb_bw32[aux];
     aux= gb_bw[aux];
     offsetVGA= (destY<<8)+(destY<<6)+destX;//(destY*320)+destX;
     //offsetVGA= (destY<<6)+(destY<<4)+(destX>>2);//(destY*80)+(destX/4);
     //offsetVGA= ((destY*320)+destX)/4;//(destY*80)+(destX/4);
     vga_data_array[offsetVGA]= aux;   //2 pixels
     vga_data_array[offsetVGA+1]= aux; //2 pixels
     if (gb_use_tiled==0){ vga_data_array[offsetVGA+2]= aux; } //2 pixels sin separador tiles
     //ptrvga32[offsetVGA]= aux32;

     //destY++;
     //drawPixel(destX,destY,aux);
     //drawPixel((destX+1),destY,aux);
     //drawPixel((destX+2),destY,aux);
     //drawPixel((destX+3),destY,aux);

     offsetVGA+= 320;
     //offsetVGA+= 80;
     vga_data_array[offsetVGA]= aux;   //2 pixels
     vga_data_array[offsetVGA+1]= aux; //2 pixels
     if (gb_use_tiled==0){ vga_data_array[offsetVGA+2]= aux; } //2 pixels sin separador tiles
     //ptrvga32[offsetVGA]= aux32;

     if (gb_use_tiled==0)
     {//Linea 3 intermedia era negra
      offsetVGA+= 320;
      vga_data_array[offsetVGA]= aux; //2 pixels
      vga_data_array[offsetVGA+1]= aux; //2 pixels
      vga_data_array[offsetVGA+2]= aux; //2 pixels      
     }
     
     ofsX+=2;
    #endif 
   #endif 
   

//    gb_buffer_vga[(ofsY+j)][(ofsX+i)^2] = aux; //No uso DMA, hay CPU de sobra
//    gb_buffer_vga[(ofsY+j)][(ofsX+i+1)^2] = aux;
//    gb_buffer_vga[(ofsY+j+1)][(ofsX+i)^2] = aux;
//    gb_buffer_vga[(ofsY+j+1)][(ofsX+i+1)^2] = aux;

   //ofsX+=2;
  }  
  ofsY+=2;  
 }
}

//*************************************
void Loadrom2Flash(unsigned char id)
{//Solo lee 256 roms
 memcpy(&ram[0x200],gb_list_rom_data[id],gb_list_rom_size[id]);
}

//**************************
void __not_in_flash_func()Beep_poll()
{//Genera tono aproximado 500 Hz (1 ms, 0.5 ms cambio pulso)
 if ((gbVolMixer == 1)&&(gb_use_silence==0))
 {
  gb_currentTime = millis();
  //if ((gb_currentTime - gb_time_ini_beep) >= 1) //500 Hz
  //if ((gb_currentTime - gb_time_ini_beep) >= 2) //250 Hz
  //if ((gb_currentTime - gb_time_ini_beep) >= 3) //166 Hz
  //if ((gb_currentTime - gb_time_ini_beep) >= 4) //125 Hz
  if ((gb_currentTime - gb_time_ini_beep) >= gb_sound_pulse_duration_cont) //125 Hz
  {
   gb_time_ini_beep = gb_currentTime;
   gbCont++;
  }

  
  //#ifdef use_lib_cvbs_bitluni
  // pinMode(SPEAKER_PIN, OUTPUT); //Obligar a que sea output y el Silencio en DAC1 REG_CLR_BIT
  //#endif 
  //if ((gbCont & 0x01)==0x01){
  // digitalWrite(SPEAKER_PIN, HIGH);
  //}
  //else{
  // digitalWrite(SPEAKER_PIN, LOW);
  //}
  digitalWriteFast(SPEAKER_PIN, (gbCont & 0x01));
 }
 else
 {
  //digitalWrite(SPEAKER_PIN, LOW);
  digitalWriteFast(SPEAKER_PIN, LOW);
 }
}













#ifdef use_lib_hdmi
 //*****************************************************************
 void core1_main() 
 {
  dvi_register_irqs_this_core(&dvi0, DMA_IRQ_0);
  while (queue_is_empty(&dvi0.q_colour_valid))
    __wfe();
  dvi_start(&dvi0);
  dvi_scanbuf_main_16bpp(&dvi0);
 }
#endif 

#ifdef use_lib_hdmi
 //******************************************************************
 void __not_in_flash_func()DumpVideoHDMI()
 {//16278
  unsigned int auxOffs= 0;//(y<<8)+(y<<6);//(y*320);
  uint16_t *scanline;
  unsigned char color=0;
  uint16_t *ptr_oneline16= (uint16_t *)&oneline[0];  
  uint16_t color16=0;

  long unsigned int ini= micros();
  for (uint y = 0; y < FRAME_HEIGHT; ++y)
  {
   
   if ((y&0x0F)==0)
   {//Cada 16 lineas polling sonido
    Beep_poll(); //Forma cutre de ahorrar timer e interrupcion
   }
      
   unsigned int dest=0;   
   //if ((y>=40)&&(y<200))
   if ((y>=40)&&(y<208))
   {    
    for (unsigned int kk=0;kk<320;kk++)
    {
//     color= dibujo[auxOffs++];
//     oneline[dest++]= color;
//     oneline[dest++]= color;             

     color= hdmi_data_array[auxOffs++];
     color16= (((uint16_t)color)<<8)|color;
     ptr_oneline16[dest++]= color16;  
    }
  
    scanline = &((uint16_t *)oneline)[0];
   }
   else
   {
    scanline = &((uint16_t *)onelineBlack)[0];
   }
      
   //uint16_t *scanline = &((uint16_t *)oneline)[0];
     
   queue_add_blocking_u32(&dvi0.q_colour_valid, &scanline);
   //while (queue_try_remove_u32(&dvi0.q_colour_free, &scanline));
   queue_try_remove_u32(&dvi0.q_colour_free, &scanline);
  }

  long unsigned int fin=micros();
  gb_dump_time= (fin-ini);
 }
#else
 #ifdef use_lib_vga
  void DumpVideoVGA()
  {
  
  }
 #endif  
#endif



//******************************************************
/*
void CambiaTexto()
{
  char cadOut[80]="";
  for (unsigned char i=10;i<20;i++)
  {
   sprintf(cadOut,"Linea %d",i);
   SDLprintText(cadOut,1,(i*8),0xFF,0);
  }  
}
*/


//*********************************************************
void EmuLoop()
{  
 if (gb_run_emulacion == 1)
 {                 
  CPU_loop();
 }

 gb_currentTime = millis();
 if ((gb_currentTime-gb_keyboardTime) >= gb_current_ms_poll_keyboard)
 {  
  gb_keyboardTime = gb_currentTime;
  if (gb_show_key_virtual == 1)
  {
   SDL_keys_virtual_poll();   
  }
  else
  {
   SDL_keys_poll();
  }

  if (checkKey(PS2_KC_F1)==1) //Sacar OSD
  {
   gb_show_osd_main_menu= 1;
   do_tinyOSD();
  }
 }

 //do_tinyOSD();

 if (draw == 1)
 {
  gb_cont_fps++;
  SDL_DumpVGA();   
  draw = 0;
   
  gb_run_emulacion= 0;
  gb_time_ini_espera = millis();
 }

 if (gb_show_key_virtual == 1)
 {
  gb_time_cur_key_virtual = millis();
  if ((gb_time_cur_key_virtual-gb_time_ini_key_virtual)>=99)
  {
   gb_time_ini_key_virtual= gb_time_cur_key_virtual;
   //ShowKey4x4();  
   ShowVirtualKeyboard();
  }
 }    


 if (gb_run_emulacion == 1)
 {
  if(delay_t > 0){    
   delay_t--;
   gb_run_emulacion=0;
   gb_delay_t = 16;
   gb_time_ini_espera = millis();
  }
  else
  {
   gb_delay_t = 0;
  }
  if(sound_t > 0) {    
   sound_t--;
   gb_run_emulacion=0;    
   gb_time_ini_espera = millis();
   gbVolMixer= 1;
   gb_time_ini_beep = gb_time_ini_espera;
  }
  else
  {    
   gbVolMixer= 0;   
  }   
 }  


 gb_currentTime = millis();
 if (gb_run_emulacion == 0)
 {  
  //if ((gb_currentTime - gb_time_ini_espera) >= (16 + gb_delay_t + gb_delay_sound + gb_current_delay_emulate_ms))
  if ((gb_currentTime - gb_time_ini_espera) >= (16 + gb_delay_sound + gb_current_delay_emulate_ms))
  {
   gb_run_emulacion = 1;  
  }
 }

 if (gb_auto_delay_cpu == 0)
 {
  gb_run_emulacion = 1;
 }
 
 Beep_poll();
}

//*****************************************************************
void ShowFPS()
{
 unsigned short int contLine=40;
 char cadOut[60];
 sprintf(cadOut,"%03d",gb_cont_fps);
 SDLprintText(cadOut,2,contLine,0xFF,0);
 contLine+=8;
 sprintf(cadOut,"%d",gb_dump_time);
 SDLprintText(cadOut,2,contLine,0xFF,0);
 contLine+=8;
 sprintf(cadOut,"%03d",delay_t);
 SDLprintText(cadOut,2,contLine,0xFF,0);
 contLine+=8;
 sprintf(cadOut,"%03d",gb_delay_t);
 SDLprintText(cadOut,2,contLine,0xFF,0);
 contLine+=8;
 sprintf(cadOut,"%01d",gb_run_emulacion);
 SDLprintText(cadOut,2,contLine,0xFF,0);
 contLine+=8;
 sprintf(cadOut,"%03d",sound_t);
 SDLprintText(cadOut,2,contLine,0xFF,0);
 contLine+=8;  
 
 gb_cont_fps=0;
}



//*****************************************************************
void setup() 
{
 gb_ramfree_ini= rp2040.getFreeHeap();
 
 pinMode(SPEAKER_PIN, OUTPUT); //Obligar a que sea output y el Silencio en DAC1 REG_CLR_BIT  
 digitalWrite(SPEAKER_PIN, LOW);

 #ifdef use_lib_hdmi
  hdmi_data_array= (unsigned char *)malloc(76800); //pointer hdmi
  memset(hdmi_data_array,0,76800);
 #else
  #ifdef use_lib_vga
   vga_data_array= (unsigned char *)malloc(76800); //pointer vga 
   memset(vga_data_array,0,76800);
  #endif
 #endif 
 
 // Initialize the Raspberry Pi Pico
 //memset(framebuf,0,sizeof(framebuf));
 //memset(scanline_framebuffer,0xFF,sizeof(scanline_framebuffer));
 //memset(scanline_framebuffer,0xFF,sizeof(scanline_framebuffer));
 #ifdef use_lib_hdmi
  memset(oneline,0xFF,sizeof(oneline));
  //memset(hdmi_data_array,0x00,sizeof(hdmi_data_array));
  memset(onelineBlack,0x00,sizeof(onelineBlack)); //Una linea negra
 #endif 

 Serial.begin(115200);
 Serial.printf("RAM init %d\r\n",gb_ramfree_ini);
 
 kb_begin();
 Serial.printf("PS2 Keyboard init\r\n");
 
 //delay(3000);
 delay(500);
 #ifdef use_lib_hdmi
  vreg_set_voltage(VREG_VSEL);
  delay(10);
 #endif 
   
 #ifdef use_lib_hdmi
  dvi0.timing = &DVI_TIMING;
  dvi0.ser_cfg = DVI_DEFAULT_SERIAL_CONFIG;
  set_sys_clock_khz(DVI_TIMING.bit_clk_khz, true);
  dvi_init(&dvi0, next_striped_spin_lock_num(), next_striped_spin_lock_num());

  // Core 1 will wait until it sees the first colour buffer, then start up the
  // DVI signalling.
  multicore_launch_core1(core1_main);
  Serial.printf("HDMI init\r\n");
 #else
  #ifdef use_lib_vga
   initVGA();
   Serial.printf("VGA init\r\n");
  #endif
 #endif




 CPU_init();
 ResetEmu();
 Loadrom2Flash(gb_current_sel_rom);
 gb_keyboardTime = gb_currentTime = gb_sdl_time_sound_before= gb_time_ini_beep = millis();
 //tiempoHDMI_before= tiempoHDMI_cur= gb_keyboardTime;
 gb_fps_time_cur= gb_fps_time_ini= gb_keyboardTime;
 gb_time_cur_key_virtual= gb_time_ini_key_virtual= gb_keyboardTime;
 //srand(time(NULL)); //Random revisar

 memset(gb_keyRemap,' ',16); 

 gb_ramfree_setupEnd= rp2040.getFreeHeap();
 Serial.printf("RAM END %d\r\n",gb_ramfree_setupEnd);
 Serial.printf("END Setup\r\n");
}


unsigned char oscila=0;
//*****************************************************************
void loop() 
{
  unsigned short int contSalta=0;
  while (true) 
  {     
   #ifdef use_lib_hdmi    
    //tiempoHDMI_cur= millis();     
    //if ((tiempoHDMI_cur-tiempoHDMI_before)>=16)
    //unsigned char forcedumpHDMI= 0;
    //if ((tiempoHDMI_cur-tiempoHDMI_before)>=30)
    //{
    // tiempoHDMI_before= tiempoHDMI_cur;
    // forcedumpHDMI= 1;
    // //DumpVideoHDMI();
    //}         
    //if ((gb_run_emulacion==0)||(contSalta==0))
    if (contSalta==0)
    {
     DumpVideoHDMI();
    }
    contSalta++;
    contSalta= contSalta & 0x1F; //mod 32
    //contSalta= contSalta & 0x3F; //mod 64
    //if (contSalta>=64)
    //{
    // contSalta=0;  
    //}     
    //sleep_ms(1);    
   #else
    #ifdef use_lib_vga
     //DumpVideoVGA();
    #endif 
   #endif   
   
   
   //CambiaTexto();
   EmuLoop();


   #ifdef use_lib_fps_show
    gb_fps_time_cur= millis();
    if ((gb_fps_time_cur-gb_fps_time_ini)>=999)
    {
     gb_fps_time_ini= gb_fps_time_cur;
     ShowFPS();    
    }
   #endif

  }
}
