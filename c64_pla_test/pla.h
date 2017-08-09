#ifndef __PLA_H__
#define __PLA_H__

#include <stdint.h>

/** Credit:
 *  http://www.zimmers.net/anonftp/pub/cbm/firmware/computers/c64/pla.c
 */

uint16_t addr = 0;

#define I(b) (!!((addr) & (1 << b)))

// INPUTS
#define CAS_    I(0)
#define LORAM_  I(1)
#define HIRAM_  I(2)
#define CHAREN_ I(3)
#define VA14_   I(4)
#define A15     I(5)
#define A14     I(6)
#define A13     I(7)
#define A12     I(8)
#define BA      I(9)
#define AEC_    I(10)
#define R_W_    I(11)
#define EXROM_  I(12)
#define GAME_   I(13)
#define VA13    I(14)
#define VA12    I(15)


// OUTPUTS
/* CASRAM_ */
#define F0  ((LORAM_ && HIRAM_ && A15 && !A14 && A13 &&       \
      !AEC_ && R_W_ && GAME_) ||          \
     (HIRAM_ && A15 && A14 && A13 &&        \
      !AEC_ && R_W_ && GAME_) ||          \
     (HIRAM_ && A15 && A14 && A13 &&        \
      !AEC_ && R_W_ && !EXROM_ && !GAME_) ||      \
     (HIRAM_ && !CHAREN_ && A15 && A14 && !A13 &&       \
      A12 && !AEC_ && R_W_ && GAME_) ||       \
     (LORAM_ && !CHAREN_ && A15 && A14 && !A13 &&       \
      A12 && !AEC_ && R_W_ && GAME_) ||       \
     (HIRAM_ && !CHAREN_ && A15 && A14 && !A13 &&       \
      A12 && !AEC_ && R_W_ && !EXROM_ && !GAME_) ||     \
     (VA14_ && AEC_ && GAME_ && !VA13 && VA12) ||     \
     (VA14_ && AEC_ && !EXROM_ && !GAME_ && !VA13 && VA12) || \
     (HIRAM_ && CHAREN_ && A15 && A14 && !A13 &&      \
      A12 && BA && !AEC_ && R_W_ && GAME_) ||     \
     (HIRAM_ && CHAREN_ && A15 && A14 && !A13 &&      \
      A12 && !AEC_ && !R_W_ && GAME_) ||        \
     (LORAM_ && CHAREN_ && A15 && A14 && !A13 &&      \
      A12 && BA && !AEC_ && R_W_ && GAME_) ||     \
     (LORAM_ && CHAREN_ && A15 && A14 && !A13 &&      \
      A12 && !AEC_ && !R_W_ && GAME_) ||        \
     (HIRAM_ && CHAREN_ && A15 && A14 && !A13 &&      \
      A12 && BA && !AEC_ && R_W_ && !EXROM_ && !GAME_) ||   \
     (HIRAM_ && CHAREN_ && A15 && A14 && !A13 &&      \
      A12 && !AEC_ && !R_W_ && !EXROM_ && !GAME_) ||    \
     (LORAM_ && CHAREN_ && A15 && A14 && !A13 &&      \
      A12 && BA && !AEC_ && R_W_ && !EXROM_ && !GAME_) ||   \
     (LORAM_ && CHAREN_ && A15 && A14 && !A13 &&      \
      A12 && !AEC_ && !R_W_ && !EXROM_ && !GAME_) ||    \
     (A15 && A14 && !A13 && A12 && BA &&        \
      !AEC_ && R_W_ && EXROM_ && !GAME_) ||       \
     (A15 && A14 && !A13 && A12 &&          \
      !AEC_ && !R_W_ && EXROM_ && !GAME_) ||      \
     (LORAM_ && HIRAM_ && A15 && !A14 && !A13 &&      \
      !AEC_ && R_W_ && !EXROM_) ||          \
     (A15 && !A14 && !A13 && !AEC_ && EXROM_ && !GAME_) ||    \
     (HIRAM_ && A15 && !A14 && A13 && !AEC_ &&      \
      R_W_ && !EXROM_ && !GAME_) ||         \
     (A15 && A14 && A13 && !AEC_ && EXROM_ && !GAME_) ||    \
     (AEC_ && EXROM_ && !GAME_ && VA13 && VA12) ||      \
     (!A15 && !A14 && A12 && EXROM_ && !GAME_) ||     \
     (!A15 && !A14 && A13 && EXROM_ && !GAME_) ||     \
     (!A15 && A14 && EXROM_ && !GAME_) ||       \
     (A15 && !A14 && A13 && EXROM_ && !GAME_) ||      \
     (A15 && A14 && !A13 && !A12 && EXROM_ && !GAME_) ||    \
     CAS_)
/* BASIC_ */
#define F1  (!LORAM_ || !HIRAM_ || !A15 || A14 || !A13 || \
     AEC_ || !R_W_ || !GAME_ )
/* KERNAL_ */
#define F2  ((!HIRAM_ || !A15 || !A14 || !A13 || AEC_ ||  \
      !R_W_ || !GAME_) &&       \
     (!HIRAM_ || !A15 || !A14 || !A13 || AEC_ ||  \
      !R_W_ || EXROM_ || GAME_ ))
/* CHAROM_ */
#define F3  ((!HIRAM_ || CHAREN_ || !A15 || !A14 || A13 ||  \
      !A12 || AEC_ || !R_W_ || !GAME_) &&   \
     (!LORAM_ || CHAREN_ || !A15 || !A14 || A13 ||  \
      !A12 || AEC_ || !R_W_ || !GAME_) &&   \
     (!HIRAM_ || CHAREN_ || !A15 || !A14 || A13 ||  \
      !A12 || AEC_ || !R_W_ || EXROM_ || GAME_) &&  \
     (!VA14_ || !AEC_ || !GAME_ || VA13 || !VA12) &&  \
     (!VA14_ || !AEC_ || EXROM_ || GAME_ || VA13 || !VA12))
/* GR_W_ */
#define F4  (CAS_ || !A15 || !A14 || A13 || !A12 || AEC_ || R_W_)
/* I_O_ */
#define F5  ((!HIRAM_ || !CHAREN_ || !A15 || !A14 || A13 ||   \
      !A12 || !BA || AEC_ || !R_W_ || !GAME_) &&    \
     (!HIRAM_ || !CHAREN_ || !A15 || !A14 || A13 ||   \
      !A12 || AEC_ || R_W_ || !GAME_) &&      \
     (!LORAM_ || !CHAREN_ || !A15 || !A14 || A13 ||   \
      !A12 || !BA || AEC_ || !R_W_ || !GAME_) &&    \
     (!LORAM_ || !CHAREN_ || !A15 || !A14 || A13 ||   \
      !A12 || AEC_ || R_W_ || !GAME_) &&      \
     (!HIRAM_ || !CHAREN_ || !A15 || !A14 || A13 ||   \
      !A12 || !BA || AEC_ || !R_W_ || EXROM_ ||     \
      GAME_) &&           \
     (!HIRAM_ || !CHAREN_ || !A15 || !A14 || A13 ||   \
      !A12 || AEC_ || R_W_ || EXROM_ || GAME_) &&   \
     (!LORAM_ || !CHAREN_ || !A15 || !A14 || A13 ||   \
      !A12 || !BA || AEC_ || !R_W_ || EXROM_ ||     \
      GAME_) &&           \
     (!LORAM_ || !CHAREN_ || !A15 || !A14 || A13 ||   \
      !A12 || AEC_ || R_W_ || EXROM_ || GAME_) &&   \
     (!A15 || !A14 || A13 || !A12 || !BA ||     \
      AEC_ || !R_W_ || !EXROM_ || GAME_) &&     \
     (!A15 || !A14 || A13 || !A12 || AEC_ ||    \
      R_W_ || !EXROM_ || GAME_ ))
/* ROML_ */
#define F6  ((!LORAM_ || !HIRAM_ || !A15 || A14 || A13 ||   \
      AEC_ || !R_W_ || EXROM_) &&       \
     (!A15 || A14 || A13 || AEC_ || !EXROM_ || GAME_))
/* ROMH_ */
#define F7  ((!HIRAM_ || !A15 || A14 || !A13 ||     \
      AEC_ || !R_W_ || EXROM_ || GAME_ ) &&     \
     (!A15 || !A14 || !A13 || AEC_ || !EXROM_ || GAME_) &&  \
     (!AEC_ || !EXROM_ || GAME_ || !VA13 || !VA12))


#endif

