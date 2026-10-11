;
; Eye of the Beholder II - START.EXE
; Byte order swaps (the IFF and LBM files are big-endian); module 23DF
; (Ghidra 33DF), written in assembler.
;
; Assembled with
;     tasm /ml swap.asm
; [asm] = identical to START.EXE byte for byte (tools/asm.py).
;
		.MODEL	LARGE, C
		.CODE

; 33DF:0006 v with its 4 bytes reversed
		PUBLIC	swap_long
swap_long	PROC	FAR v:DWORD
		mov	ax, word ptr v
		mov	dx, word ptr v+2
		xchg	ah, dl
		xchg	al, dh
		ret
swap_long	ENDP

; 33DF:0015 v with its 2 bytes swapped
		PUBLIC	swap_word
swap_word	PROC	FAR v:WORD
		mov	ax, v
		xchg	ah, al
		ret
swap_word	ENDP

		END
