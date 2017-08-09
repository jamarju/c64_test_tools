/*
  Wiring diagram:
                                               +-----+
                  +----[PWR]-------------------| USB |--+
                  |                            +-----+  |
                  |         GND/RST2  [ ][ ]            |
                  |       MOSI2/SCK2  [ ][ ]  A5/SCL[ ] |   C5 
                  |          5V/MISO2 [ ][ ]  A4/SDA[ ] |   C4 
                  |                             AREF[ ] |
                  |                              GND[ ] |
                  | [ ]N/C                    SCK/13[ ] |   B5
                  | [ ]IOREF                 MISO/12[ ] |   B4 --- DRAM DIN
                  | [ ]RST                   MOSI/11[ ]~|   B3 --- DRAM /WE
                  | [ ]3V3    +---+               10[ ]~|   B2 --- DRAM /RAS
                  | [ ]5v    -| A |-               9[ ]~|   B1 --- DRAM /CAS
                  | [ ]GND   -| R |-               8[ ] |   B0 --- RAM DOUT
                  | [ ]GND   -| D |-                    |
                  | [ ]Vin   -| U |-               7[ ] |   D7 --- RAM A7
                  |          -| I |-               6[ ]~|   .
              C0  | [ ]A0    -| N |-               5[ ]~|   .
              C1  | [ ]A1    -| O |-               4[ ] |   .
              C2  | [ ]A2     +---+           INT1/3[ ]~|   .
              C3  | [ ]A3                     INT0/2[ ] |   .
              C4  | [ ]A4/SDA  RST SCK MISO     TX>1[ ] |   .
              C5  | [ ]A5/SCL  [ ] [ ] [ ]      RX<0[ ] |   D0 --- RAM A0
                  |            [ ] [ ] [ ]              |
                  |  UNO_R3    GND MOSI 5V  ____________/
                  \_______________________/
      
                  http://busyducks.com/ascii-art-arduinos

  Remaining DRAM pins.

  - pin 8  (Vcc) -> +5v
  - pin 16 (Vss) -> GND

  This is for Hitachi HM4864 chips. See C64 service manual for compatible chips.

  How to diagnose:

  1. TO AVOID FRYING YOUR ARDUINO, test electrical condition of the chip **FIRST**:

  - Check for shorted Vcc
  - Inputs Ax, /CAS, /RAS, /WE, DIN must be high-Z. If shorted to 0v/5v, it's broken.
  - Output DOUT must be high-Z in the absence of RAS/CAS activity.
  - Output DOUT is push-pull AFTER the RAS/CAS read sequence (see datasheet). If it's high-Z, it's broken.

  2. Run this test:

  It will write 1s, then 0s, then two checkerboard patterns in a never-ending loop.

  RAM is automatically refreshed on row selection. As long as every row is selected once
  every 2 ms there is no need for RAS-only refresh cycle.

  TODO: a delay of <2ms could be added between the write and read loops to test data retention.

  - LED (pin 13) off -> TEST PASS
  - LED (pin 13) on  -> TEST FAIL (will turn back off at the end of the test cycle, so PAY ATTENTION)

*/

#define DOUT 0
#define nCAS 1
#define nRAS 2
#define nWE  3
#define DIN  4
#define LED  5

#define SETB(port, b)  port |= _BV(b)
#define CLRB(port, b)  port &= ~_BV(b)
#define ISSET(port, b) ((port & _BV(b)) != 0)

#define TEST_1
#define TEST_0
#define TEST_CHECKERBOARD
#define TEST_REVERSE_CHECKERBOARD

void setup() {
  DDRD    = 0b11111111; // 1 = out, 0 = in
  // PD7  (7) 1-------; A7
  // PD6  (6) -1------; A6
  // PD5  (5) --1-----; A5
  // PD4  (4) ---1----; A4
  // PD3  (3) ----1---; A3
  // PD2  (2) -----1--; A2
  // PD1  (1) ------1-; A1
  // PD0  (0) -------1; A0
  
  DDRB    = 0b11111110;
  // PB5 (13) --1----- LED
  // PB4 (12) ---1---- DIN
  // PB3 (11) ----1--- nWE
  // PB2 (10) -----1-- nRAS
  // PB1 ( 9) ------1- nCAS
  // PB0 ( 8) -------0 DOUT

  PORTD   = 0;

  PORTB   = 0b11011110;
  // PB4 (13) --0----- LED = 0
  // PB4 (12) ---1---- DIN = 1
  // PB3 (11) ----1--- nWE = read
  // PB2 (10) -----1-- nRAS = 1
  // PB1 ( 9) ------1- nCAS = 1
  // PB0 ( 8) -------0 DOUT = no pullup
}

inline void
w(uint8_t row, uint8_t col, uint8_t v) 
{
    if (v) {
      SETB(PORTB, DIN);
    } else {
      CLRB(PORTB, DIN);
    }
    PORTD = row;
    CLRB(PORTB, nRAS);
    CLRB(PORTB, nWE);
    PORTD = col;  // hold nWE on
    CLRB(PORTB, nCAS);
    asm("nop");  // > 100 ns
    SETB(PORTB, nWE);
    SETB(PORTB, nRAS);
    SETB(PORTB, nCAS);
}

inline uint8_t
r(uint8_t row, uint8_t col)
{
    // read
    PORTD = row;
    CLRB(PORTB, nRAS);
    PORTD = col;
    CLRB(PORTB, nCAS);

    asm("nop"); // > 100n
    asm("nop");
    
    uint8_t v = ISSET(PINB, DOUT);

    SETB(PORTB, nRAS);
    SETB(PORTB, nCAS);

    return v;
}

void loop() {
  uint8_t col = 0, row = 0, v = 0;

  for (;;) {

#ifdef TEST_1
    // WRITE 1s
    do {
      do {
        w(row, col, 1);
        row++;
      } while (row);
      col++;
    } while (col);

    // VERIFY
    do {
      do {
        if (! r(row, col)) SETB(PORTB, LED);
        row++;
      } while (row);
      col++;
    } while (col);
#endif

#ifdef TEST_0
    // WRITE 0s
    do {
      do {
        w(row, col, 0);
        row++;
      } while (row);
      col++;
    } while (col);

    // VERIFY
    do {
      do {
        if (r(row, col)) SETB(PORTB, LED);
        row++;
      } while (row);
      col++;
    } while (col);
#endif

#ifdef TEST_CHECKERBOARD
    // WRITE CHECKERBOARD
    v = 0;
    do {
      do {
        w(row, col, v);
        row++;
        v = !v;
      } while (row);
      col++;
      v = !v;
    } while (col);

    // VERIFY CHECKERBOARD
    v = 0;
    do {
      do {
        if (r(row, col) != v) SETB(PORTB, LED);
        row++;
        v = !v;
      } while (row);
      col++;
      v = !v;
    } while (col);
#endif

#ifdef TEST_REVERSE_CHECKERBOARD
    // WRITE REVERSE CHECKERBOARD
    v = 1;
    do {
      do {
        w(row, col, v);
        row++;
        v = !v;
      } while (row);
      col++;
      v = !v;
    } while (col);

    // VERIFY REVERSE CHECKERBOARD
    v = 1;
    do {
      do {
        if (r(row, col) != v) SETB(PORTB, LED);
        row++;
        v = !v;
      } while (row);
      col++;
      v = !v;
    } while (col);
#endif

    delay(100);
    CLRB(PORTB, LED);
  }
}
