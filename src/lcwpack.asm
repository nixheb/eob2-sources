;
; Eye of the Beholder II - START.EXE
; The LCW compressor ("format 80", the opposite of unpack_lcw); module 2248
; (Ghidra 3248, from 3248:000A), written in assembler.
;
; Assembled with
;     tasm /ml lcwpack.asm
; [asm] = identical to START.EXE byte for byte (tools/asm.py).
;
		.MODEL	LARGE, C
		LOCALS
		EXTRN	normalize_dxax:FAR
		.CODE

; 3248:000A Compresses size bytes of src into dst; returns the size written.
; At each place: a run of more than 64 equal bytes (FE), else the longest
; earlier match (0cccpppp for 3-10 bytes less than 4K back, 11cccccc up to
; 64 bytes, FF beyond), else the byte goes to a literal run (10cccccc, at
; most 63)
		PUBLIC	compress_lcw
compress_lcw	PROC	FAR USES bx cx di ds dx es si, src:DWORD, dst:DWORD, size:WORD
		cld
		mov	ax,word ptr src
		mov	dx,word ptr src+2
		call	far ptr normalize_dxax
		mov	si,ax
		mov	ds,dx
		mov	ax,word ptr dst
		mov	dx,word ptr dst+2
		call	far ptr normalize_dxax
		mov	di,ax
		mov	es,dx
		mov	dx,size
		cld
		mov	bp,si
		add	bp,dx
		mov	word ptr cs:InLiteral,1
		mov	word ptr cs:DstStart,di
		mov	word ptr cs:SrcStart,si
		mov	word ptr cs:LitCount,di
		mov	al,81h
		stosb
		lodsb
		stosb
@@0056:
		mov	word ptr cs:DstPtr,di
		mov	di,es
		mov	word ptr cs:DstSeg,di
		mov	di,ds
		mov	es,di
		mov	di,word ptr cs:SrcStart
		mov	word ptr cs:BestLen,1
@@0072:
		mov	al,byte ptr [si]
		cmp	al,byte ptr [si+40h]
		jnz	@@00B9
		mov	bx,di
		mov	di,si
		mov	cx,bp
		sub	cx,di
		repe	scasb
		dec	di
		mov	cx,di
		sub	cx,si
		cmp	cx,41h
		jc	@@00B7
		mov	word ptr cs:InLiteral,0
		mov	si,di
		mov	es,word ptr cs:DstSeg
		mov	di,word ptr cs:DstPtr
		mov	ah,al
		mov	al,0FEh
		stosb
		xchg	ax,cx
		stosw
		mov	al,ch
		stosb
		mov	word ptr cs:DstPtr,di
		mov	di,ds
		mov	es,di
		mov	di,bx
		jmp	short @@0072
@@00B7:
		mov	di,bx
@@00B9:
		mov	cx,si
		sub	cx,di
		jz	@@00FB
		repne	scasb
		jnz	@@00FB
		mov	bx,word ptr cs:BestLen
		mov	ah,byte ptr [bx+si-1]
		cmp	ah,byte ptr es:[bx+di-2]
		jnz	@@00B9
		mov	dx,si
		mov	bx,di
		dec	di
		mov	cx,bp
		sub	cx,si
		repe	cmpsb
		jnz	@@00DF
		inc	di
@@00DF:
		mov	si,dx
		mov	ax,di
		sub	ax,bx
		mov	di,bx
		cmp	ax,word ptr cs:BestLen
		jc	@@0072
		mov	cs:BestLen,ax
		dec	bx
		mov	word ptr cs:BestPos,bx
		jmp	@@0072
@@00FB:
		mov	cx,word ptr cs:BestLen
		mov	es,word ptr cs:DstSeg
		mov	di,word ptr cs:DstPtr
		cmp	cx,2
		jbe	@@013F
		cmp	cx,0Ah
		ja	@@012F
		mov	ax,si
		sub	ax,word ptr cs:BestPos
		cmp	ax,0FFFh
		ja	@@012F
		mov	bl,cl
		sub	bl,3
		mov	cl,4
		shl	bl,cl
		add	ah,bl
		xchg	ah,al
		jmp	short @@0179
@@012F:
		cmp	cx,40h
		ja	@@0168
		sub	cl,3
		or	cl,0C0h
		mov	al,cl
		stosb
		jmp	short @@0170
@@013F:
		cmp	word ptr cs:InLiteral,0
		jnz	@@014F
@@0147:
		mov	word ptr cs:LitCount,di
		mov	al,80h
		stosb
@@014F:
		mov	bx,word ptr cs:LitCount
		cmp	byte ptr es:[bx],0BFh
		jz	@@0147
		inc	byte ptr es:[bx]
		lodsb
		stosb
		mov	word ptr cs:InLiteral,1
		jmp	short @@0186
@@0168:
		mov	al,0FFh
		stosb
		mov	ax,cs:BestLen
		stosw
@@0170:
		mov	ax,cs:BestPos
		sub	ax,word ptr cs:SrcStart
@@0179:
		stosw
		add	si,word ptr cs:BestLen
		mov	word ptr cs:InLiteral,0
@@0186:
		cmp	si,bp
		jnc	@@018D
		jmp	@@0056
@@018D:
		mov	ax,80h
		stosb
		mov	ax,di
		sub	ax,word ptr cs:DstStart
		ret
compress_lcw	ENDP

InLiteral	dw	0		; 3248:01A1 a literal run is open
DstStart	dw	0		; 3248:01A3
SrcStart	dw	0		; 3248:01A5
LitCount	dw	0		; 3248:01A7 the count byte of the literal run
DstSeg		dw	0		; 3248:01A9
DstPtr		dw	0		; 3248:01AB
BestLen		dw	0		; 3248:01AD the longest match
BestPos		dw	0		; 3248:01AF and where

		END
