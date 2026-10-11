;
; Eye of the Beholder II - START.EXE
; The mouse moved by the program; module 23C1 (Ghidra 33C1, from
; 33C1:0006: the segment starts at 33C1:0006), written in assembler.
;
; Assembled with
;     tasm /ml mousemov.asm
; [asm] = identical to START.EXE byte for byte (tools/asm.py).
;
		.MODEL	LARGE, C
		LOCALS
		INCLUDE	procs.inc

		.DATA
		INCLUDE	eob2.inc
		.CODE

; 33C1:0006 CX, DX clamped into the mouse window become the position; the
; driver and the cursor follow
mouse_clamp	PROC	NEAR
@@0006:
		cmp	cx,word ptr MouseMinX
		jl	@@0018
		cmp	cx,word ptr MouseMaxX
		jle	@@001C
		mov	cx,word ptr MouseMaxX
		jmp	short @@001C
@@0018:
		mov	cx,word ptr MouseMinX
@@001C:
		mov	word ptr ClickDown,cx
		cmp	dx,word ptr MouseMinY
		jl	@@0032
		cmp	dx,word ptr MouseMaxY
		jle	@@0036
		mov	dx,word ptr MouseMaxY
		jmp	short @@0036
@@0032:
		mov	dx,word ptr MouseMinY
@@0036:
		mov	word ptr ClickDown+2,dx
		cmp	byte ptr MousePresent,0
		jz	@@004F
		cmp	word ptr MouseXDouble,1
		jnz	@@004A
		shl	cx,1
@@004A:
		mov	ax,4
		int	33h
@@004F:
		mov	ax,word ptr ClickDown
		cmp	word ptr CursorDrawX,ax
		jnz	@@0061
		mov	ax,word ptr ClickDown+2
		cmp	word ptr CursorDrawY,ax
		jz	@@007E
@@0061:
		push	word ptr PagePtr+2
		push	word ptr PagePtr
		call	Drv_65C6
		add	sp,4
		call	far ptr cursor_hide
		call	far ptr cursor_show
		call	Drv_65B6
@@007E:
		ret
mouse_clamp	ENDP

; 33C1:007F The mouse goes to (x, y)
		PUBLIC	mouse_move_to
mouse_move_to	PROC	FAR
		push	bp
		mov	bp,sp
		push	cx
		push	dx
		mov	cx,word ptr [bp+6]
		mov	dx,word ptr [bp+8]
@@008A:
		cmp	word ptr MouseBusy,0
		jnz	@@008A
		inc	word ptr MouseBusy
		call	mouse_clamp
		dec	word ptr MouseBusy
		pop	dx
		pop	cx
		pop	bp
		retf
mouse_move_to	ENDP

; 33C1:00A0 The mouse moves by (dx, dy) (the keyboard mode)
		PUBLIC	mouse_move_by
mouse_move_by	PROC	FAR
		push	bp
		mov	bp,sp
		push	cx
		push	dx
		mov	cx,word ptr [bp+6]
		mov	dx,word ptr [bp+8]
@@00AB:
		cmp	word ptr MouseBusy,0
		jnz	@@00AB
		inc	word ptr MouseBusy
		add	cx,word ptr ClickDown
		add	dx,word ptr ClickDown+2
		call	mouse_clamp
		dec	word ptr MouseBusy
		pop	dx
		pop	cx
		pop	bp
		retf
mouse_move_by	ENDP


		END
