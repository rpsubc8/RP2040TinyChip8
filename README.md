# RP2040TinyChip8
<center><img src='https://raw.githubusercontent.com/rpsubc8/RP2040TinyChip8/main/preview/chip8previewgif.gif'></center>
Very preliminary version: Chip8 emulator port to RP2040
<center><img src='https://raw.githubusercontent.com/rpsubc8/RP2040TinyChip8/main/preview/rp2040pizero.jpg'></center>

Using use_lib_vga in gbConfig.h:<br>

| VGA GPIO  | Description  |
| --------- | ------------ |
| 18        | RED          | 
| 19        | GREEN        |
| 20        | BLUE         |
| 16        | HSync        |
| 17        | VSync        |

If a 10-hole non-soldering DB15 VGA connector, such as the one attached, is used:
<center><img src='https://raw.githubusercontent.com/rpsubc8/RP2040TinyChip8/main/preview/vgasinsoldar01.jpg'></center>
<center><img src='https://raw.githubusercontent.com/rpsubc8/RP2040TinyChip8/main/preview/vgasinsoldar02.jpg'></center>
<center><img src='https://raw.githubusercontent.com/rpsubc8/RP2040TinyChip8/main/preview/vgasinsoldar03.jpg'></center>
In this connector we have:

| VGA         | Description  |
| ----------- | ------------ |
| 1           | RED          | 
| 2           | GREEN        |
| 3           | BLUE         |
| 13          | HSYNC        |
| 14          | VSYNC        |
| 5,6,7,8,11  | GROUND       |

Complete circuit:<br>
<center><img src='https://raw.githubusercontent.com/rpsubc8/RP2040TinyChip8/main/preview/circuitovgaunwelded.gif'></center>

Simple circuit:<br>
<center><img src='https://raw.githubusercontent.com/rpsubc8/RP2040TinyChip8/main/preview/simplevgaunwelded.gif'></center>

<br>
For very old monitors (CRT VGA), the following scheme may be required:

Complete circuit:<br>
<center><img src='https://raw.githubusercontent.com/rpsubc8/RP2040TinyChip8/main/preview/circuitovga.gif'></center>

Simple circuit:<br>
<center><img src='https://raw.githubusercontent.com/rpsubc8/RP2040TinyChip8/main/preview/simplevga.gif'></center>

Using use_lib_hdmi in gbConfig.h (hdmi connector on waveshare board):<br>

| HDMI      | GPIO        |
| --------- | ------------|
| sm_tmds   | 0, 1, 2     |
| pins_tmds | 26, 24, 22  |
| pins_clk  | 28          |

<br><br>
<h1>Sound</h1>
In the gbConfig.h in the SPEAKER_PIN, GPIO 21 is specified. A basic polling system is used to generate a 500 Hz square wave, to save the use of interrupts or timers.<br><br>

| GPIO    | Description      |
| ------- | -----------------|
| 21      | square (polling) |

A filter similar to fabgl's filter can be used with ESP32:<br>
<center><img src='https://raw.githubusercontent.com/rpsubc8/RP2040TinyChip8/main/preview/filtroSonido.gif'></center>
Although a simple 100K logarithmic potentiometer would be sufficient.<br>
<center><img src='https://raw.githubusercontent.com/rpsubc8/RP2040TinyChip8/main/preview/simpleSoundpote.gif'></center>
<br><br>


<h1>PS/2 keyboard</h1>
Using the use_lib_keyboard_ps2 option in gbConfig.h we will be able to use an external PS/2 keyboard or a USB to PS/2, if it supports the internal PS/2 protocol, using GPIO 4 and 5. The power supply must be 3.3v. If we want to power at 5V, without reducing the voltage, we will burn the inputs of the RP2040.<br>
If you don't understand any of this, just don't go ahead.<br><br>
<center><img src='https://raw.githubusercontent.com/rpsubc8/RP2040TinyChip8/main/preview/ps2.gif'></center>

Simple circuit:
<center><img src='https://raw.githubusercontent.com/rpsubc8/RP2040TinyChip8/main/preview/minimalkeyboard.gif'></center>

| PS/2 GPIO | Description  |
| --------- | ------------ |
|  4        | CLK (pin 5)  |
|  5        | Data (pin 1) |

<br><br>
<h1>PS/2 keyboard USB C</h1>
If we have a Waveshare PiZero board, we can use the PIO-USB connector, i.e. the central one, which is located between the HDMI and the power supply.<br>
From this USB C connector, we can use a USB to USB C converter.<br>
<center><img src='https://raw.githubusercontent.com/rpsubc8/RP2040TinyChip8/main/preview/usb2usbc.jpg'></center>
Then we will use a passive PS/2 to USB converter.
<center><img src='https://raw.githubusercontent.com/rpsubc8/RP2040TinyChip8/main/preview/ps2usb.jpg'></center>
Finally, connect the PS/2 keyboard to the converter.
<center><img src='https://raw.githubusercontent.com/rpsubc8/RP2040TinyChip8/main/preview/boardps2usbc.jpg'></center>
In the gbConfig.h we must choose the option use_lib_keyboard_ps2usb, which will use GPIO 6 and 7.

| PS2 GPIO  | Description |
| --------- | ----------- |
|  6        | CLK  (D+)   |
|  7        | Data (D-)   |

For some emulators, when using the native HID protocol via USB, the keyboard must be connected to the external connector, while the board is powered by the central connector (PIO-USB). But in this case, you are using the PS/2 protocol, so you must connect to the central PIO-USB and power it on the external USB.
<br><br>


<h1>Description</h1>
<ul>
 <li>Arduino IDE 1.8.11</li>
 <li>Board Waveshare RP2040 pizero</li>
 <li>HDMI library Waveshare (adafruit and Wren6991 picoDVI hdmi mod)</li>
 <li>VGA library (Hunter Adams and San Tarcisio mod)</li>
 <li>PS2 library (ps2kbdlib michalhol mod)</li>
 <li>A variety of PS/2 keyboards, as well as USB keyboards with PS/2 conversion allow 3.3v power supply. Be sure to work with 3.3v. Never power the keyboard at 5v. In case you are using the PIO-USB USB C connector, you can work with 5V. Make sure you know what you are doing, and if in doubt, just don't go ahead.</li>
 <li>Although 3 GPIO's are used for VGA, since only 1 bit of colour (black and white) is used, one of the 3 colour GPIO outputs can be used at each input of the VGA monitor.</li>
 <li>No overclock voltage HDMI (1.05v). By default RP2040 runs at 1.10v and 133 Mhz. For HDMI it is required to overclock to 1.25v and 250 Mhz. For HDMI, I have done an undervolt, to be more conservative, i.e. I have left it at 1.05v, being configurable in gbConfig.h. If it doesn't look good, you can go higher.</li>
</ul>
<br><br>


<h1>OSD</h1>
Like the ESP32TinyChip8, I have left an OSD visible when pressing the F1 key.<br>
You can navigate with the directional arrows on the keyboard, and when playing, you can use the keys:<br>

<h2>Keyboard PS/2</h2>

|  K  |  K  |  K  |  K  |
| --- | --- | --- | --- |
|  1  |  2  |  3  |  4  |
|  Q  |  W  |  E  |  R  |
|  A  |  S  |  D  |  F  |
|  Z  |  X  |  C  |  V  |

<br><br>
<h2>Keyboard CHIP8</h2>

|  K  |  K  |  K  |  K  |
| --- | --- | --- | --- |
|  1  |  2  |  3  |  C  |
|  4  |  5  |  6  |  D  |
|  7  |  8  |  9  |  E  |
|  A  |  0  |  B  |  F  |

<br><br>

| KEY CHIP8 | KEY KEYBOARD |
| --------- | ------------ |
| 1         | 1            |
| 2         | 2            |
| 3         | 3            |
| C         | 4            |
| 4         | Q            |
| 5         | W            |
| 6         | E            |
| D         | R            |
| 7         | A            |
| 8         | S            |
| 9         | D            |
| E         | F            |
| A         | Z            |
| 0         | X            |
| B         | C            |
| F         | V            |

<br><br>

| CURSOR    | KEY KEYBOARD | KEY CHIP8 |
| --------- | ------------ | --------- |
| LEFT      | Q            | 4         |
| RIGHT     | E            | 6         |
| DOWN      | S            | 8         |
| UP        | W            | 5         |
| BACKSPACE | V            | F         |

<br><br>
Gamepad Virtual

| CURSOR    | KEY KEYBOARD | KEY CHIP8 |
| --------- | ------------ | --------- |
| LEFT      | Q            | 4         |
| RIGHT     | E            | 6         |
| DOWN      | S            | 8         |
| UP        | 2            | 2         |
| FIRE A(0) | W            | 5         |
| FIRE B(.) | V            | F         |

<br><br>
KEYBOARD Virtual

| CURSOR    | KEY KEYBOARD |
| --------- | ------------ |
| LEFT      | LEFT         |
| RIGHT     | RIGH         |
| DOWN      | DOWN         |
| UP        | UP           |
| 0 (INS)   | FIRE A (0)   |

<br><br>

<h1>Arduino IDE</h1>
Version 1.8.11.<br>
Preferences board:<br>
<pre>
 https://github.com/earlephilhower/arduino-pico/releases/download/global/package_rp2040_index.json
</pre>
<center><img src='https://raw.githubusercontent.com/rpsubc8/RP2040TinyChip8/main/preview/preferencesArduinoIde.gif'></center>
Earle F. Philhower version 2.6.4:<br>
<center><img src='https://raw.githubusercontent.com/rpsubc8/RP2040TinyChip8/main/preview/earleFPhilhower264.gif'></center>
Waveshare RP2040 Plus 16MB options:<br>
<center><img src='https://raw.githubusercontent.com/rpsubc8/RP2040TinyChip8/main/preview/optionsrp2040.gif'></center>

<br><br>

<h1>Precompile version</h1>
Versions for VGA or HDMI, with direct PS/2 keyboard (ps2) or via PS/2 to USB C converter (ps2usb) are available.<br>
<a href='https://github.com/rpsubc8/RP2040TinyChip8/tree/main/precompile'>https://github.com/rpsubc8/RP2040TinyChip8/tree/main/precompile</a>
