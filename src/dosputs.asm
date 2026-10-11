;
; Eye of the Beholder II - START.EXE
; A string written to the screen through DOS (function 6, one character at
; a time); module 2246 (Ghidra 3246), written in assembler.
;
; Assembled with
;     tasm /ml dosputs.asm
; [asm] = identical to START.EXE byte for byte (tools/asm.py).
;
		.MODEL	LARGE, C
		LOCALS
		.CODE

; 3246:0006 Writes the string s (ends with 0)
		PUBLIC	dos_puts
dos_puts	PROC	FAR USES ax di ds dx si, s:DWORD
		cld
		mov	si, word ptr s
		mov	ds, word ptr s+2
@@next:		lodsb
		or	al, al
		jz	@@done
		mov	dl, al
		mov	ah, 6
		int	21h
		jmp	@@next
@@done:		ret
dos_puts	ENDP

		END
