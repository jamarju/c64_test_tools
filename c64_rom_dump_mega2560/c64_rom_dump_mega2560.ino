#include <stdint.h>

/*
 Wiring diagram:
 Robotdyn Mega Pro CH340G /Atmega2560
 (probably valid for an Arduino Mega but check pinout)

                   +---------------     
                   | VIN[ ][ ]VIN  \                  
                   | GND[ ][ ]GND  /                  
                   |  5V[ ][ ]5V   \                  
                   | 3V3[ ][ ]3V3  /                  
                   | RST[ ][ ]AREF \                  
               E1  |  TX[ ][ ]RX   /  E0
               E5  |  D3[ ][ ]D2   \  E4
               E3  |  D5[ ][ ]D4   /  G5
               H4  |  D7[ ][ ]D6   \  H3
               H6  |  D9[ ][ ]D8   /  H5
               B5  | D11[ ][ ]D10  \  B4
               B7  | D13[ ][ ]D12  /  B6
               J0  | D15[ ][ ]D14  \  J1
               H0  | D17[ ][ ]D16  /  H1              
               D2  | D19[ ][ ]D18  \  D3              
               D0  | D21[ ][ ]D20  /  D1              
    ROM A1 --- A1  | D23[ ][ ]D22  \  A0 --- ROM A0
    ROM A3 --- A3  | D25[ ][ ]D24  /  A2 --- ROM A2
    ROM A5 --- A5  | D27[ ][ ]D26  \  A4 --- ROM A5
    ROM A7 --- A7  | D29[ ][ ]D28  /  A6 --- ROM A7
               C6  | D31[ ][ ]D30  \  C7
                   +----------------

                    --+-----+-------+
                    \ |     |       |
                    / | USB | [RST] |
                    \ +-----+       |
                    /               |
                    \               |
    ROM  D1 --- F1  /   A1[ ][ ]A0  |  F0 --- ROM D0
    ROM  D3 --- F3  \   A3[ ][ ]A2  |  F2 --- ROM D2
    ROM  D5 --- F5  /   A5[ ][ ]A4  |  F4 --- ROM D4
    ROM  D7 --- F7  \   A7[ ][ ]A6  |  F6 --- ROM D6
                K1  /   A9[ ][ ]A8  |  K0
                K3  \  A11[ ][ ]A10 |  K2
                K5  /  A13[ ][ ]A12 |  K4
                K7  \  A15[ ][ ]A14 |  K6
    ROM A11 --- C4  /  D33[ ][ ]D32 |  C5 --- ROM A12
    ROM  A9 --- C2  \  D35[ ][ ]D34 |  C3 --- ROM A10
    ROM  A8 --- C0  /  D37[ ][ ]D36 |  C1 --- ROM A9
                G2  \  D39[ ][ ]D38 |  D7
    ROM /CE --- G0  /  D41[ ][ ]D40 |  G1
                L6  \  D43[ ][ ]D42 |  L7
                L4  /  D45[ ][ ]D44 |  L5
                L2  \  D47[ ][ ]D46 |  L3
                     ---------------+

  Remaining ROM pins:

  - pin 24 (Vcc) -> +5v
  - pin 12 (GND) -> GND

  For 2332 (4 KB) ROM (CHAROM):

  - pin 21 (CE2) -> +5v

  For 2364 (8 KB) ROMS (BASIC, KERNAL, cartridges):

  - pin 21 (A12) -> as in diagram


  How to diagnose:

  1. Test electrical condition of the chip **FIRST**:

  - Check for shorted Vcc
  - Inputs Ax and /CE must be high-Z. If shorted to 0v/5v, it's broken.

  2. Run this test. Choose /CE high or low. Expected output:

  - /CE low: a ROM dump in hex format. Use xxd -r to convert to binary and compare to a known good dump, etc.
  - /CE high: all FF. Otherwise the ROM FAILS to obey chip enable.

  Common failures of 2332/2364 chips:

  - Chip not obeying /CE (dump with /CE high produces the same results as /CE low).
  - Outputs shorted to GND regardless of /CE.
 */


// -------------- BEGIN CONFIG AREA -----------------

#define ROM_SIZE (8 * 1024)

// --------------- END CONFIG AREA ------------------


void setup(); // WTF arduino?
void loop();

inline uint8_t
chip_enable(uint8_t out) 
{
    PORTG = (PORTG & 0b11111110) | (out & 1);
}


inline uint8_t
peek(uint16_t a) 
{
  PORTA = a & 0b11111111;
  PORTC = (PORTC & 0b11100000) | ((a >> 8) & 0b00011111);

  asm("nop"); // 62.5 ns * 7 = 437.5 ns (2332 chip spec is 450 ns)
  asm("nop");
  asm("nop");
  asm("nop");
  asm("nop");
  asm("nop");
  asm("nop");

  return PINF;
}


void setup() {
  // Pin functions:
  
  // PA0..7 = address generator (ROM A0..7)
  // PC0..4 = address generator (ROM A8..12)
  // PF0..7 = data (ROM D0..7)
  // PG0    = /CE control
  
  // Pin direction: 0 = in/pullup, 1 = out
  DDRA     = 0b11111111;     DDRC     = 0b00011111;     DDRF    = 0b00000000;    
  // PA7 (D29) 1------- A7                              // PF7 (A7) 0------- D7
  // PA6 (D28) -1------ A6                              // PF6 (A6) -0------ D6
  // PA5 (D27) --1----- A5                              // PF5 (A5) --0----- D5
  // PA4 (D26) ---1---- A4   // PC4 (D33) ---1---- A12  // PF4 (A4) ---0---- D4
  // PA3 (D25) ----1--- A3   // PC3 (D34) ----1--- A11  // PF3 (A3) ----0--- D3
  // PA2 (D24) -----1-- A2   // PC2 (D35) -----1-- A10  // PF2 (A2) -----0-- D2
  // PA1 (D23) ------1- A1   // PC1 (D36) ------1- A9   // PF1 (A1) ------0- D1
  // PA0 (D22) -------1 A0   // PC0 (D37) -------1 A8   // PF0 (A0) -------0 D0

  // if DDRD == 0 (in)  -> 0 = no pull-up, 1 = pull-up
  // if DDRD == 1 (out) -> 0 = low output, 1 = high output
  PORTA    = 0b00000000;     PORTC    = 0b00000000;     PORTF   = 0b11111111;
  // PA7 (D29) 0------- A7                              // PF7 (A7) 1------- D7
  // PA6 (D28) -0------ A6                              // PF6 (A6) -1------ D6
  // PA5 (D27) --0----- A5                              // PF5 (A5) --1----- D5
  // PA4 (D26) ---0---- A4   // PC4 (D33) ---0---- A12  // PF4 (A4) ---1---- D4
  // PA3 (D25) ----0--- A3   // PC3 (D34) ----0--- A11  // PF3 (A3) ----1--- D3
  // PA2 (D24) -----0-- A2   // PC2 (D35) -----0-- A10  // PF2 (A2) -----1-- D2
  // PA1 (D23) ------0- A1   // PC1 (D36) ------0- A9   // PF1 (A1) ------1- D1
  // PA0 (D22) -------0 A0   // PC0 (D37) -------0 A8   // PF0 (A0) -------1 D0

  // /CE control (start as high output)
  DDRG  = 0b00000001;
  PORTG = 0b00000001;
  
  Serial.begin(115200);
}


void loop() {
  int16_t c;

  chip_enable(1); // disable chip
  
  Serial.println("\r\n/CE high or low? [h/l]");

  loop: c = Serial.read();
  switch (c) {
    case 'h': chip_enable(1); break;
    case 'l': chip_enable(0); break;
    default:  goto loop;
  }

  for (uint16_t a = 0; a < ROM_SIZE; a++) {
    uint8_t b;
    char str[40];

    b = peek(a);

    if (a % 16 == 0) {
      sprintf(str, "\r\n%08x:", a);
      Serial.print(str);
    }

    sprintf(str, " %02x", b);
    Serial.print(str);
  }
}
