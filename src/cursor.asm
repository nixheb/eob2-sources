;
; Eye of the Beholder II - START.EXE
; The mouse cursor: hidden (the screen under it put back), shown (the
; screen saved, the shape drawn), and the DGROUP state of the mouse;
; module 238B (Ghidra 338B, from 338B:000A), written in assembler.
;
; Assembled with
;     tasm /ml cursor.asm
; [asm] = identical to START.EXE byte for byte (tools/asm.py).
;
		.MODEL	LARGE, C
		LOCALS
		INCLUDE	procs.inc

		.DATA
		INCLUDE	eob2.inc
		.CODE

; 338B:000A Hides the cursor: the screen under it is put back (counted)
		PUBLIC	cursor_hide
cursor_hide	PROC	FAR
@@000A:
		push	bp
		mov	bp,sp
		push	ax
		cmp	byte ptr InventoryMode,0
		jz	@@0018
@@0015:
		jmp	@@00F2
@@0018:
		cmp	word ptr MouseHidden,0
		jnz	@@007D
		cmp	word ptr CursorBack+2,0
		jz	@@0015
		mov	ax,CursorDrawX
		sub	ax,word ptr CursorHotX
		mov	Var_6656,ax
		mov	ax,CursorDrawY
		sub	ax,word ptr CursorHotY
		mov	Var_6658,ax
		mov	ax,CursorH
		mov	Var_6662,ax
		neg	ax
		mov	Var_6664,ax
		mov	ax,CursorW
		mov	Var_6666,ax
		neg	ax
		mov	Var_6668,ax
		mov	ax,Var_6656
		sar	ax,1
		sar	ax,1
		sar	ax,1
		cmp	ax,word ptr Var_6668
		jle	@@007D
		cmp	ax,28h
		jge	@@007D
		mov	Var_6656,ax
		add	ax,word ptr Var_6666
		mov	Var_665A,ax
		cmp	word ptr Var_6656,0
		jge	@@007F
		mov	word ptr Var_6656,0
		jmp	short @@008C
@@007D:
		jmp	short @@00E8
@@007F:
		cmp	word ptr Var_665A,28h
		jle	@@008C
		mov	word ptr Var_665A,28h
@@008C:
		mov	ax,Var_6658
		cmp	ax,word ptr Var_6664
		jle	@@00E8
		cmp	ax,0C8h
		jge	@@00E8
		mov	Var_6658,ax
		add	ax,word ptr Var_6662
		mov	Var_665C,ax
		cmp	word ptr Var_6658,0
		jge	@@00B3
		mov	word ptr Var_6658,0
		jmp	short @@00C1
@@00B3:
		cmp	word ptr Var_665C,0C8h
		jle	@@00C1
		mov	word ptr Var_665C,0C8h
@@00C1:
		push	word ptr CursorBack+2
		push	word ptr CursorBack
		mov	ax,Var_665C
		sub	ax,word ptr Var_6658
		push	ax
		mov	ax,Var_665A
		sub	ax,word ptr Var_6656
		push	ax
		mov	ax,Var_6658
		push	ax
		push	word ptr Var_6656
		call	Drv_6546
		add	sp,0Ch
@@00E8:
		inc	word ptr MouseHidden
		jnz	@@00F2
		dec	word ptr MouseHidden
@@00F2:
		pop	ax
		pop	bp
		retf
cursor_hide	ENDP

; 338B:00F5 Shows the cursor (when the count comes back to 0): the screen
; under it saved, the shape drawn
		PUBLIC	cursor_show
cursor_show	PROC	FAR
@@00F5:
		push	bp
		mov	bp,sp
		push	ax
		cmp	byte ptr InventoryMode,0
		jnz	@@0163
		cmp	word ptr MouseHidden,0
		jz	@@0163
		dec	word ptr MouseHidden
		cmp	word ptr MouseHidden,0
		jnz	@@0163
		cmp	word ptr CursorBack+2,0
		jz	@@0163
		mov	ax,word ptr ClickDown
		mov	CursorDrawX,ax
		sub	ax,word ptr CursorHotX
		mov	Var_6656,ax
		mov	Var_665E,ax
		mov	ax,word ptr ClickDown+2
		mov	CursorDrawY,ax
		sub	ax,word ptr CursorHotY
		mov	Var_6658,ax
		mov	Var_6660,ax
		mov	ax,CursorH
		mov	Var_6662,ax
		neg	ax
		mov	Var_6664,ax
		mov	ax,CursorW
		mov	Var_6666,ax
		neg	ax
		mov	Var_6668,ax
		mov	ax,Var_6656
		sar	ax,1
		sar	ax,1
		sar	ax,1
		cmp	ax,word ptr Var_6668
		jle	@@0163
		cmp	ax,28h
		jl	@@0166
@@0163:
		jmp	@@0203
@@0166:
		mov	Var_6656,ax
		add	ax,word ptr Var_6666
		mov	Var_665A,ax
		cmp	word ptr Var_6656,0
		jge	@@017F
		mov	word ptr Var_6656,0
		jmp	short @@018C
@@017F:
		cmp	word ptr Var_665A,28h
		jle	@@018C
		mov	word ptr Var_665A,28h
@@018C:
		mov	ax,Var_6658
		cmp	ax,word ptr Var_6664
		jle	@@0163
		cmp	ax,0C8h
		jge	@@0163
		mov	Var_6658,ax
		add	ax,word ptr Var_6662
		mov	Var_665C,ax
		cmp	word ptr Var_6658,0
		jge	@@01B3
		mov	word ptr Var_6658,0
		jmp	short @@01C1
@@01B3:
		cmp	word ptr Var_665C,0C8h
		jle	@@01C1
		mov	word ptr Var_665C,0C8h
@@01C1:
		push	word ptr CursorBack+2
		push	word ptr CursorBack
		mov	ax,Var_665C
		sub	ax,word ptr Var_6658
		push	ax
		mov	ax,Var_665A
		sub	ax,word ptr Var_6656
		push	ax
		mov	ax,Var_6658
		push	ax
		push	word ptr Var_6656
		call	Drv_65D2
		add	sp,0Ch
		push	word ptr Var_6660
		push	word ptr Var_665E
		push	word ptr CursorShape+2
		push	word ptr CursorShape
		mov	ax,0
		push	ax
		call	Drv_656A
		add	sp,0Ah
@@0203:
		pop	ax
		pop	bp
		retf
cursor_show	ENDP

; 338B:0206 Hides the cursor while it is in (x1, y1)-(x2, y2)
		PUBLIC	mouse_hide_rect
mouse_hide_rect	PROC	FAR
		push	bp
		mov	bp,sp
		push	ax
		push	bx
		push	cx
		push	di
		push	dx
		push	si
		mov	cx,word ptr [bp+6]
		mov	dx,word ptr [bp+8]
		mov	si,word ptr [bp+0Ah]
		mov	di,word ptr [bp+0Ch]
@@021B:
		cmp	word ptr MouseBusy,0
		jnz	@@021B
		inc	word ptr MouseBusy
		test	word ptr MouseFlags,8000h
		jz	@@023C
		mov	ax,MouseFlags
		inc	al
		jnz	@@0237
		dec	al
@@0237:
		mov	MouseFlags,ax
		jmp	short @@02A7
@@023C:
		mov	word ptr HideX1,cx
		mov	word ptr HideY1,dx
		mov	word ptr HideX2,si
		mov	word ptr HideY2,di
		mov	ax,CursorDrawX
		sub	ax,word ptr CursorHotX
		and	ax,0FFF8h
		mov	Var_6656,ax
		mov	bx,word ptr CursorW
		shl	bx,1
		shl	bx,1
		shl	bx,1
		sub	word ptr HideX1,bx
		add	ax,bx
		mov	Var_665A,ax
		mov	ax,CursorDrawY
		sub	ax,word ptr CursorHotY
		mov	Var_6658,ax
		mov	bx,word ptr CursorH
		sub	word ptr HideY1,bx
		add	ax,bx
		mov	Var_665C,ax
		mov	word ptr MouseFlags,8000h
		cmp	word ptr Var_665A,cx
		jl	@@02BE
		cmp	word ptr Var_6656,si
		jg	@@02BE
		cmp	word ptr Var_665C,dx
		jle	@@02BE
		cmp	word ptr Var_6658,di
		jg	@@02BE
		or	word ptr MouseFlags,4000h
@@02A7:
		push	word ptr PagePtr+2
		push	word ptr PagePtr
		call	Drv_65C6
		add	sp,4
		push	cs
		call	near ptr cursor_hide
		call	Drv_65B6
@@02BE:
		dec	word ptr MouseBusy
		pop	si
		pop	dx
		pop	di
		pop	cx
		pop	bx
		pop	ax
		pop	bp
		retf
mouse_hide_rect	ENDP

; 338B:02CA Ends mouse_hide_rect
		PUBLIC	mouse_show_rect
mouse_show_rect	PROC	FAR
		push	bp
		mov	bp,sp
		push	ax
@@02CE:
		cmp	word ptr MouseBusy,0
		jnz	@@02CE
		inc	word ptr MouseBusy
		mov	ax,MouseFlags
		cmp	al,0
		jz	@@02E6
		dec	word ptr MouseFlags
		jmp	short @@02F1
@@02E6:
		mov	word ptr MouseFlags,0
		test	ax,4000h
		jz	@@0308
@@02F1:
		push	word ptr PagePtr+2
		push	word ptr PagePtr
		call	Drv_65C6
		add	sp,4
		push	cs
		call	near ptr cursor_show
		call	Drv_65B6
@@0308:
		dec	word ptr MouseBusy
		pop	ax
		pop	bp
		retf
mouse_show_rect	ENDP

; 338B:030F Hides the cursor (with the page of the screen)
		PUBLIC	hide_mouse
hide_mouse	PROC	FAR
		push	bp
		mov	bp,sp
@@0312:
		cmp	word ptr MouseBusy,0
		jnz	@@0312
		inc	word ptr MouseBusy
		push	word ptr PagePtr+2
		push	word ptr PagePtr
		call	Drv_65C6
		add	sp,4
		push	cs
		call	near ptr cursor_hide
		call	Drv_65B6
		dec	word ptr MouseBusy
		pop	bp
		retf
hide_mouse	ENDP

; 338B:033A Shows it again
		PUBLIC	show_mouse
show_mouse	PROC	FAR
		push	bp
		mov	bp,sp
@@033D:
		cmp	word ptr MouseBusy,0
		jnz	@@033D
		inc	word ptr MouseBusy
		push	word ptr PagePtr+2
		push	word ptr PagePtr
		call	Drv_65C6
		add	sp,4
		push	cs
		call	near ptr cursor_show
		call	Drv_65B6
		dec	word ptr MouseBusy
		pop	bp
		retf
show_mouse	ENDP


		.DATA			; Initialised data of the module (DS:6610)
MouseBusy      	dw	0		; the cursor is being changed: the handler waits
MouseButtons   	dw	0		; the buttons down
Var_6614       	dw	2
Var_6616       	dw	0		; the buttons that changed
Var_6618       	dw	0
ClickDown      	dw	0, 0		; the mouse position (x, y)
MouseRawX      	dw	0
MouseRawY      	dw	0
ClickUp		dw	0, 0		; (x, y)
MouseXDouble   	dw	0		; the driver counts x in 0-639: halved
MouseHidden    	dw	0		; hide count of the cursor
MouseMinX      	dw	0
MouseMaxX      	dw	0
MouseMinY      	dw	0
MouseMaxY      	dw	0
CursorH		dw	0		; the cursor shape: height, width + 1
CursorW		dw	0
CursorHotX     	dw	0
CursorHotY     	dw	0
CursorDrawX    	dw	0		; where the cursor is drawn
CursorDrawY    	dw	0
MouseFlags     	dw	0		; 8000h: hidden inside HideRect
HideX1		dw	0
HideY1		dw	0
HideX2		dw	0
HideY2		dw	0
CursorBack     	dd	0		; the screen under the cursor
CursorShape    	dd	0
PagePtr		dd	0		; the descriptor of the page shown (16 bytes)
MousePresent   	db	0
InventoryMode  	db	0		; the keyboard moves the selection (no cursor)
Var_6656       	dw	0		; the cursor rectangle
Var_6658       	dw	0
Var_665A       	dw	0
Var_665C       	dw	0
Var_665E       	dw	0
Var_6660       	dw	0
Var_6662       	dw	0
Var_6664       	dw	0
Var_6666       	dw	0
Var_6668       	dw	0

		END
