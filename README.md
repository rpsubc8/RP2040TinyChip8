# RP2040TinyChip8
Very preliminary version: Chip8 emulator port to RP2040
<center><img src='https://raw.githubusercontent.com/rpsubc8/RP2040TinyChip8/main/preview/rp2040pizero.jpg'></center>

| VGA   | Description |
| ----- | ------------|
| 18    | RED         | 
| 19    | GREEN       |
| 20    | BLUE        |
| 16    | HSync       |
| 17    | VSync       |

| HDMI      | GPIO        |
| --------- | ------------|
| sm_tmds   | 0, 1, 2     |
| pins_tmds | 26, 24, 22  |
| pins_clk  | 28          |

| PS2 GPIO  | Description |
| --------- | ----------- |
|  4        | CLK         |
|  5        | Data        |

| PS2 GPIO  | Description |
| --------- | ----------- |
|  6        | CLK         |
|  7        | Data        |

<center><img src='https://raw.githubusercontent.com/rpsubc8/RP2040TinyChip8/main/preview/boardps2usbc.jpg'></center>
<center><img src='https://raw.githubusercontent.com/rpsubc8/RP2040TinyChip8/main/preview/ps2usb.jpg'></center>
<center><img src='https://raw.githubusercontent.com/rpsubc8/RP2040TinyChip8/main/preview/usb2usbc.jpg'></center>

<ul>
 <li>Arduino IDE 1.8.11</li>
 <li>Board Waveshare RP2040 pizero</li>
 <li>HDMI library Waveshare (adafruit hdmi mod)</li>
 <li>VGA library (Hunter Adams and San Tarcisio mod)</li>
 <li>PS2 library (ps2kbdlib michalhol mod)</li>
 <li>A variety of PS/2 keyboards, as well as USB keyboards with PS/2 conversion allow 3.3v power supply. Be sure to work with 3.3v. Never power the keyboard at 5v.</li>
 <li>Although 3 GPIO's are used for VGA, since only 1 bit of colour (black and white) is used, one of the 3 colour GPIO outputs can be used at each input of the VGA monitor.</li>
 <li>No overclock voltage HDMI (1.05v).</li>
</ul>
<br><br>

<h1>Precompile version</h1>
<a href='https://github.com/rpsubc8/RP2040TinyChip8/tree/main/precompile'>https://github.com/rpsubc8/RP2040TinyChip8/tree/main/precompile</a>
