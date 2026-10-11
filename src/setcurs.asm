;
; Eye of the Beholder II - START.EXE
; The shape of the mouse cursor; module 237D (Ghidra 337D), written in
; assembler.
;
; Assembled with
;     tasm /ml setcurs.asm
; [asm] = identical to START.EXE byte for byte (tools/asm.py).
;
		.MODEL	LARGE, C
		LOCALS
		INCLUDE	procs.inc
		GLOBAL	mem_alloc:FAR, mem_free:FAR, mem_free_total:FAR

		.DATA
		INCLUDE	eob2.inc
		.CODE

; 337D:0000 The cursor becomes shape (its hot spot hx, hy): the buffer
; under it is reallocated when the shape is bigger; returns 1, or 0 in the
; keyboard mode or when the memory lacks
		PUBLIC	set_cursor
set_cursor	PROC	FAR
		push	bp
		mov	bp,sp
		cmp	byte ptr MousePresent,1
		jnz	@@0011
		cmp	byte ptr InventoryMode,0
		jz	@@0016
@@0011:
		xor	ax,ax
		jmp	@@00E8
@@0016:
		push	dx
		push	si
		push	es
@@0019:
		cmp	word ptr MouseBusy,0
		jnz	@@0019
		inc	word ptr MouseBusy
		call	far ptr cursor_hide
		mov	ax,word ptr [bp+6]
		mov	CursorHotX,ax
		mov	ax,word ptr [bp+8]
		mov	CursorHotY,ax
		mov	si,word ptr [bp+0Ah]
		mov	word ptr CursorShape,si
		mov	es,word ptr [bp+0Ch]
		mov	word ptr CursorShape+2,es
		xor	ah,ah
		mov	al,byte ptr es:[si+3]
		xor	dh,dh
		mov	dl,byte ptr es:[si+2]
		inc	dx
		cmp	word ptr CursorBack+2,0
		jz	@@007C
		push	ax
		push	dx
		call	Drv_65EE
		add	ax,10h
		cmp	word ptr CursorBufSize,ax
		pop	dx
		pop	ax
		jge	@@00C2
		push	ax
		push	dx
		push	word ptr CursorBack+2
		push	word ptr CursorBack
		call	far ptr mem_free
		add	sp,4
		pop	dx
		pop	ax
@@007C:
		mov	CursorH,ax
		push	ax
		mov	word ptr CursorW,dx
		push	dx
		call	Drv_65EE
		add	sp,4
		add	ax,10h
		mov	si,ax
		mov	CursorBufSize,ax
		call	far ptr mem_free_total
		or	dx,dx
		jnz	@@00A1
		cmp	ax,si
		jc	@@00DF
@@00A1:
		mov	ax,0
		push	ax
		xor	ax,ax
		push	ax
		mov	ax,si
		sub	ax,10h
		push	ax
		call	far ptr mem_alloc
		add	sp,6
		cmp	dx,0
		jz	@@00DF
		mov	word ptr CursorBack,ax
		mov	word ptr CursorBack+2,dx
@@00C2:
		push	word ptr PagePtr+2
		push	word ptr PagePtr
		call	Drv_65C6
		add	sp,4
		call	far ptr cursor_show
		call	Drv_65B6
		mov	ax,1
		jmp	short @@00E1
@@00DF:
		xor	ax,ax
@@00E1:
		dec	word ptr MouseBusy
		pop	es
		pop	si
		pop	dx
@@00E8:
		pop	bp
		retf
set_cursor	ENDP


		.DATA			; Initialised data of the module (DS:660E)
CursorBufSize	dw	0

		END
