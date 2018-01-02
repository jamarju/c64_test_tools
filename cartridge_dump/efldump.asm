.const	BGCOLOR = $d020
.const	FGCOLOR	= $d021
.const	SETLFS	= $ffba
.const	SETNAM	= $ffbd
.const	SAVE	= $ffd8
.const	IO1		= $de00
.const	SADDR	= $8000
.const	PAGES	= $20
.const	TADDR	= $6000
.const	VS		= $fb
.const	VT		= $fd

:BasicUpstart2(start)
		
        * = $810
start:        
        
		lda #<SADDR
		sta VS
		lda #>SADDR
		sta VS+1
		lda #<TADDR
		sta VT
		lda #>TADDR
		sta VT+1
		ldx #PAGES

		// Bank EFL in. A few reads are needed to discharge the /EXROM cap,
		// let's say 20.
		sei
		ldy #20
bankin:
		lda IO1
		dey
		bne bankin

copy_page:		
		lda (VS),y
		sta (VT),y
		iny
		bne copy_page
		inc VS+1
		inc VT+1
		dex
		bne copy_page 
		
		cli
		
		lda #file_end-file // filename size
        ldx #<file
        ldy #>file
        jsr SETNAM

        lda #1 // logical file
        ldx #8 // drive
        ldy #1 // secondary addr
		jsr SETLFS

        lda #<TADDR
        sta VT
        lda #>TADDR
        sta VT+1
        
        lda #VT
        ldx #<(TADDR + PAGES * 256)
        ldy #>(TADDR + PAGES * 256)
        jsr SAVE
        
        rts
file:	.text "CARTDUMP,W,S"
file_end:
