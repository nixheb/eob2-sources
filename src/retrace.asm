;
; Eye of the Beholder II - START.EXE
; The vertical retrace of the VGA (input status 3DAh, bit 3); module 2404
; (Ghidra 3404, from 3404:000C), written in assembler.
;
; Assembled with
;     tasm /ml retrace.asm
; [asm] = identical to START.EXE byte for byte (tools/asm.py).
;
		.MODEL	LARGE, C
		LOCALS
		.CODE

; 3404:000C Non-zero during the vertical retrace
		PUBLIC	vretrace
vretrace	PROC	FAR
		push	bp
		mov	bp, sp
		mov	dx, 3DAh
		in	al, dx
		and	al, 8
		xor	ah, ah
		pop	bp
		ret
vretrace	ENDP

; 3404:0019 Waits for a retrace: mode 1, for its end; else for the start
; of the next one
		PUBLIC	wait_retrace
wait_retrace	PROC	FAR USES ax bx dx, mode:WORD
		mov	bl, byte ptr mode
		mov	dx, 3DAh
		cmp	bl, 1
		jz	@@end
@@out:		in	al, dx
		test	al, 8
		jz	@@out
@@in:		in	al, dx
		test	al, 8
		jnz	@@in
		jz	@@done
@@end:		in	al, dx
		test	al, 8
		jnz	@@end
@@wait:		in	al, dx
		test	al, 8
		jz	@@wait
@@done:		ret
wait_retrace	ENDP

		END
