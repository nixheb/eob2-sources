;
; Eye of the Beholder II - START.EXE
; The drives of the machine (DOS); module 2263 (Ghidra 3263, from
; 3263:0002), written in assembler.
;
; Assembled with
;     tasm /ml drives.asm
; [asm] = identical to START.EXE byte for byte (tools/asm.py).
;
		.MODEL	LARGE, C
		LOCALS

		.DATA
		INCLUDE	eob2.inc
		.CODE

; 3263:0002 The current drive (CurDrive) and the last drive (LastDrive: the
; number of drives, less the phantom ones DOS 440Eh reports)
		PUBLIC	count_drives
count_drives	PROC	FAR
		push	bp
		mov	bp,sp
		push	ax
		push	bx
		push	dx
		mov	ah,19h
		int	21h
		mov	CurDrive,al
		mov	dl,al
		mov	ah,0Eh
		int	21h
		dec	al
		xor	ah,ah
		mov	dx,ax
@@001B:
		mov	bl,dl
		inc	bl
		mov	ax,440Eh
		int	21h
		jnc	@@002D
		cmp	al,0Fh
		jnz	@@002D
		dec	dx
		jmp	short @@001B
@@002D:
		mov	ax,dx
		mov	LastDrive,al
		pop	dx
		pop	bx
		pop	ax
		pop	bp
		retf
count_drives	ENDP

; 3263:0037 Non-zero if drive (0 = A:) is a real drive, not a phantom one
		PUBLIC	drive_ready
drive_ready	PROC	FAR
		push	bp
		mov	bp,sp
		push	bx
		push	dx
		mov	dl,byte ptr [bp+6]
		push	bx
		mov	bl,dl
		inc	bl
		mov	ax,440Eh
		int	21h
		pop	bx
		jnc	@@0052
		cmp	al,1
		jz	@@0061
		jmp	short @@005C
@@0052:
		cmp	al,0
		jz	@@0061
		dec	al
		cmp	al,dl
		jz	@@0061
@@005C:
		mov	ax,0
		jmp	short @@0064
@@0061:
		mov	ax,1
@@0064:
		pop	dx
		pop	bx
		pop	bp
		retf
drive_ready	ENDP


		END
