# RP2040TinyChip8
Very preliminary version: Chip8 emulator port to RP2040

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

<ul>
 <li>Arduino IDE 1.8.11</li>
 <li>Board Waveshare RP2040 pizero</li>
 <li>HDMI library Waveshare (adafruit hdmi mod)</li>
 <li>VGA library (Hunter Adams and San Tarcisio mod)</li>
 <li>PS2 library (ps2kbdlib michalhol mod)</li>
 <li>No overclock voltage HDMI (1.05v).</li>
</ul>
