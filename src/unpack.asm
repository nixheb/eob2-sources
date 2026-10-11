;
; Eye of the Beholder II - START.EXE
; The decompressors of Westwood's files: LCW ("format 80", the CPS
; pictures) and format 3 (runs, the pages of the WSA-like files); module
; 23EC (Ghidra 33EC, from 33EC:0004), written in assembler.
;
; Assembled with
;     tasm /ml unpack.asm
; [asm] = identical to START.EXE byte for byte (tools/asm.py).
;
		.MODEL	LARGE, C
		LOCALS
		EXTRN	normalize_dxax:FAR
		.CODE

; 33EC:0004 Unpacks src (LCW) into dst, at most max bytes; returns the
; size unpacked. Commands:
;   0cccpppp pppppppp      copy ccc + 3 bytes from dst - p
;   10cccccc               copy c bytes from src (80h: the end)
;   11cccccc pppp          copy c + 3 bytes from dst start + p
;   FE cccc vv             fill c bytes with v
;   FF cccc pppp           copy c bytes from dst start + p
		PUBLIC	unpack_lcw
unpack_lcw	PROC	FAR USES bx cx di ds dx es si, src:DWORD, dst:DWORD, max:WORD
		mov	ax, word ptr src
		mov	dx, word ptr src+2
		call	far ptr normalize_dxax
		mov	si, ax
		mov	ds, dx
		mov	ax, word ptr dst
		mov	dx, word ptr dst+2
		call	far ptr normalize_dxax
		mov	di, ax
		mov	es, dx
		mov	dx, max
		mov	cs:DstStart, di
		add	dx, di
		mov	cs:DstEnd, dx
		cld
		mov	bx, si
		mov	dx, ds
		mov	bp, es
@@loop:		mov	ax, cs:DstEnd
		sub	ax, di
		jz	@@end
		mov	cs:Left, ax
		mov	si, bx
		mov	ds, dx
		lodsb
		or	al, al
		js	@@high
		mov	ah, al			; 0cccpppp pppppppp
		and	ah, 0Fh
		mov	ch, al
		mov	cl, 4
		shr	ch, cl
		mov	cl, ch
		xor	ch, ch
		add	cx, 3
		cmp	cx, cs:Left
		jbe	@@back
		mov	cx, cs:Left
@@back:		lodsb
		mov	bx, si
		mov	si, di
		sub	si, ax
		mov	ds, bp
		rep	movsb
		jmp	@@loop
@@high:		test	al, 40h
		jnz	@@c0
		cmp	al, 80h			; 10cccccc
		jz	@@end
		mov	cl, al
		and	cx, 3Fh
		cmp	cx, cs:Left
		jbe	@@copy
		mov	cx, cs:Left
@@copy:		rep	movsb
		mov	bx, si
		jmp	@@loop
@@end:		mov	ax, di
		sub	ax, cs:DstStart
		jmp	short @@exit
@@c0:		mov	cl, al			; 11cccccc
		and	cx, 3Fh
		add	cx, 3
		cmp	al, 0FEh
		jnz	@@notfill
		lodsw				; FE: fill
		mov	cx, ax
		lodsb
		mov	bx, si
		cmp	cx, cs:Left
		jbe	@@fill
		mov	cx, cs:Left
@@fill:		rep	stosb
		jmp	@@loop
@@notfill:	cmp	al, 0FFh
		jnz	@@offset
		lodsw				; FF: a long copy
		mov	cx, ax
@@offset:	lodsw
		mov	bx, si
		add	ax, cs:DstStart
		mov	si, ax
		mov	ds, bp
		cmp	cx, cs:Left
		jbe	@@far
		mov	cx, cs:Left
@@far:		rep	movsb
		jmp	@@loop
@@exit:		ret
unpack_lcw	ENDP

DstStart	dw	0		; 33EC:00FA the start of dst
Left		dw	0		; 33EC:00FC the room left in dst
DstEnd		dw	0		; 33EC:00FE its end
		dw	0, 0		; 33EC:0100 unused

; 33EC:0104 Unpacks format 3: srcseg:0000 (a header: the size unpacked at
; +2, the length of a part to skip at +6) into dstseg:0000; a byte c,
; then c bytes to copy (c > 0), a fill of -c (c < 0), or a word count of
; bytes to copy (c = 0); returns the count of bytes copied
		PUBLIC	unpack_3
unpack_3	PROC	FAR
		push	bp
		mov	bp, sp
		push	bx
		push	cx
		push	di
		push	ds
		push	dx
		push	es
		push	si
		cld
		mov	cs:Count, 0
		mov	es, word ptr [bp+8]
		mov	ds, word ptr [bp+6]
		xor	cx, cx
		xor	di, di
		xor	si, si
		add	si, 2
		mov	bx, [si]		; DX:BX the size left
		mov	dx, [si+2]
		add	si, 4
		lodsw
		add	si, ax
@@next:		mov	cl, [si]
		inc	si
		or	cl, cl
		jz	@@long
		js	@@run
		add	cs:Count, cx
		call	@@advance
		rep	movsb
		jmp	short @@more
@@long:		mov	cx, [si]		; big-endian
		xchg	cl, ch
		add	si, 2
		jmp	short @@fill
@@run:		neg	cl
@@fill:		call	@@advance
		lodsb
		add	cs:Count, cx
		rep	stosb
@@more:		mov	ax, bx
		or	ax, dx
		jnz	@@next
		jmp	short @@exit
@@advance:	sub	bx, cx			; the size left, ES:DI normalised
		sbb	dx, 0
		push	di
		shr	di, 1
		shr	di, 1
		shr	di, 1
		shr	di, 1
		mov	ax, es
		add	ax, di
		mov	es, ax
		pop	di
		and	di, 0Fh
		retn
Count		dw	0		; 33EC:017C
@@exit:		pop	si
		pop	es
		pop	dx
		pop	ds
		pop	di
		pop	cx
		pop	bx
		mov	ax, cs:Count
		pop	bp
		ret
unpack_3	ENDP

		END
