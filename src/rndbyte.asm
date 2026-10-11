;
; Eye of the Beholder II - START.EXE
; A random byte: a 24-bit shift register (the seed in DGROUP); module 23DC
; (Ghidra 33DC), written in assembler.
;
; Assembled with
;     tasm /ml rndbyte.asm
; [asm] = identical to START.EXE byte for byte (tools/asm.py).
;
		.MODEL	LARGE, C
		.DATA			; Initialised data of the module (DS:6676)
Seed		dd	98761234h	; 6676 the shift register (3 bytes used)

		.CODE

; 33DC:0004 Returns a random byte: the register is shifted by one bit, the
; new bit made from bits 1 and 0; the result is its low byte xor the next
		PUBLIC	random_byte
random_byte	PROC	FAR
		push	bp
		mov	bp, sp
		push	es
		push	si
		mov	ax, SEG Seed
		mov	es, ax
		mov	si, OFFSET Seed
		xor	ah, ah
		mov	al, es:[si]
		shr	al, 1
		shr	al, 1
		rcl	byte ptr es:[si+2], 1
		rcl	byte ptr es:[si+1], 1
		cmc
		sbb	al, es:[si]
		shr	al, 1
		rcr	byte ptr es:[si], 1
		mov	al, es:[si]
		xor	al, es:[si+1]
		pop	si
		pop	es
		pop	bp
		ret
random_byte	ENDP

		END
