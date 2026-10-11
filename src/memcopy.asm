;
; Eye of the Beholder II - START.EXE
; Far pointers: a copy of any length, and the normalisation of a pointer
; (offset 0-15); module 2374 (Ghidra 3374), written in assembler.
;
; Assembled with
;     tasm /ml memcopy.asm
; [asm] = identical to START.EXE byte for byte (tools/asm.py).
;
		.MODEL	LARGE, C
		LOCALS
		.CODE

; 3374:0004 Copies n bytes from src to dst (both normalised first, then
; again every 64K)
		PUBLIC	mem_copy
mem_copy	PROC	FAR USES ax bx cx di dx ds es si, src:DWORD, dst:DWORD, n:DWORD
		cld
		mov	si, word ptr src
		mov	ds, word ptr src+2
		mov	di, word ptr dst
		mov	es, word ptr dst+2
		mov	ax, di
		mov	dx, es
		push	cs
		call	near ptr normalize_dxax
		mov	di, ax
		mov	es, dx
		mov	ax, si
		mov	dx, ds
		push	cs
		call	near ptr normalize_dxax
		mov	si, ax
		mov	ds, dx
		mov	cx, word ptr n
		mov	bx, word ptr n+2
		shr	cx, 1
		jz	@@odd
		rep	movsw
@@odd:		jnc	@@blocks
		movsb
@@blocks:	or	bx, bx
		jz	@@done
@@block:	mov	ax, di
		mov	dx, es
		push	cs
		call	near ptr normalize_dxax
		mov	di, ax
		mov	es, dx
		mov	ax, si
		mov	dx, ds
		push	cs
		call	near ptr normalize_dxax
		mov	si, ax
		mov	ds, dx
		mov	cx, 8000h
		rep	movsw
		dec	bx
		jnz	@@block
@@done:		ret
mem_copy	ENDP

; 3374:0071 p normalised
		PUBLIC	normalize
normalize	PROC	FAR p:DWORD
		mov	ax, word ptr p
		mov	dx, word ptr p+2
		push	cs
		call	near ptr normalize_dxax
		ret
normalize	ENDP

; 3374:0080 (337C:0000) DX:AX normalised: the paragraphs of the offset go
; to the segment
		PUBLIC	normalize_dxax
normalize_dxax	PROC	FAR
		push	ax
		shr	ax, 1
		shr	ax, 1
		shr	ax, 1
		shr	ax, 1
		add	dx, ax
		pop	ax
		and	ax, 0Fh
		ret
normalize_dxax	ENDP

		END
