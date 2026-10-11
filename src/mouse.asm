;
; Eye of the Beholder II - START.EXE
; The mouse: its handler (INT 33h), the cursor test; and the table of
; DGROUP given to the video driver; module 2269 (Ghidra 3269, from
; 3269:0008), written in assembler.
;
; Assembled with
;     tasm /ml mouse.asm
; [asm] = identical to START.EXE byte for byte (tools/asm.py).
;
		.MODEL	LARGE, C
		LOCALS
		INCLUDE	procs.inc
		EXTRN	mem_alloc:FAR, mem_free:FAR

		.DATA
		INCLUDE	eob2.inc
		EXTRN	CurFont:WORD, PageSegs:WORD, Var_62A5:WORD
		.CODE

; 3269:0008 Installs mouse_handler (INT 33h, function 0Ch) if a driver is there;
; returns 1 if so. The cursor starts at (160, 100), hidden
		PUBLIC	mouse_init
mouse_init	PROC	FAR
		push	bp
		mov	bp,sp
		push	bx
		push	cx
		push	dx
		push	es
		mov	ax,3533h
		int	21h
		mov	word ptr ClickDown,0A0h
		mov	word ptr ClickDown+2,64h
		mov	word ptr MouseHidden,1
		mov	word ptr MouseFlags,0
		mov	word ptr MouseMaxX,13Fh
		mov	word ptr MouseMaxY,0C7h
		mov	ax,es
		or	ax,bx
		jz	@@0046
		mov	bl,byte ptr es:[bx]
		cmp	bl,0CFh
		jnz	@@0050
@@0046:
		mov	ax,0
		mov	byte ptr MousePresent,0
		jmp	short @@009F
@@0050:
		mov	ax,0
		int	33h
		cmp	ax,0FFFFh
		jnz	@@0046
		mov	word ptr MouseXDouble,0
		mov	ax,3
		int	33h
		cmp	cx,0A0h
		jz	@@0071
		mov	word ptr MouseXDouble,1
@@0071:
		mov	cx,0A0h
		mov	dx,64h
		mov	byte ptr MousePresent,1
		mov	byte ptr InventoryMode,1
		cmp	word ptr MouseXDouble,1
		jnz	@@008A
		shl	cx,1
@@008A:
		mov	ax,4
		int	33h
		mov	ax,0Ch
		mov	cx,1Fh
		push	cs
		pop	es
		mov	dx,OFFSET mouse_handler
		int	33h
		mov	ax,1
@@009F:
		pop	es
		pop	dx
		pop	cx
		pop	bx
		pop	bp
		retf
mouse_init	ENDP

; 3269:00A5 Called by the mouse driver (CX, DX the position, BX the buttons): the
; position (halved, stepped), the buttons as keys, the cursor moved
		PUBLIC	mouse_handler
mouse_handler	PROC	FAR
		push	ds
		push	ax
		push	bx
		push	cx
		push	dx
		pushf
		cld
		mov	ax,DGROUP
		mov	ds,ax
		cmp	byte ptr InventoryMode,0
		jnz	@@0115
		cmp	word ptr MouseXDouble,1
		jnz	@@00C1
		shr	cx,1
@@00C1:
		cmp	cx,13Fh
		jbe	@@00CA
		mov	cx,13Fh
@@00CA:
		mov	word ptr MouseRawX,cx
		mov	word ptr MouseRawY,dx
		mov	ax,Var_6614
		or	ax,ax
		jz	@@00E4
		test	bx,4
		jz	@@00E4
		and	bx,3
		or	bx,ax
@@00E4:
		cmp	word ptr MouseButtons,bx
		jz	@@0103
		push	bx
		call	far ptr mouse_button_key
		add	sp,2
		mov	ax,MouseButtons
		xor	ax,bx
		or	ax,word ptr Var_6616
		mov	Var_6616,ax
		mov	word ptr MouseButtons,bx
@@0103:
		cmp	bx,0
		jz	@@010E
		mov	word ptr Var_6618,1
@@010E:
		cmp	word ptr MouseBusy,0
		jz	@@0139
@@0115:
		jmp	@@01EB
@@0118:
		push	word ptr PagePtr+2
		push	word ptr PagePtr
		call	Drv_65C6
		add	sp,4
		call	far ptr cursor_hide
		call	Drv_65B6
		or	word ptr MouseFlags,4000h
		jmp	@@01E5
@@0139:
		mov	word ptr MouseBusy,1
		cmp	byte ptr Var_6600,2
		jz	@@018A
		cmp	word ptr MouseStepX,0
		jz	@@0166
		mov	ax,cx
		mov	cx,dx
		sub	ax,word ptr MouseOrgX
		mov	bx,word ptr MouseStepX
		cwd
		idiv	bx
		imul	bx
		add	ax,word ptr MouseOrgX
		mov	dx,cx
		mov	cx,ax
@@0166:
		cmp	word ptr MouseStepY,0
		jz	@@0182
		mov	ax,dx
		sub	ax,word ptr MouseOrgY
		mov	bx,word ptr MouseStepY
		cwd
		idiv	bx
		imul	bx
		add	ax,word ptr MouseOrgY
		mov	dx,ax
@@0182:
		mov	word ptr ClickDown,cx
		mov	word ptr ClickDown+2,dx
@@018A:
		mov	cx,word ptr ClickDown
		cmp	word ptr CursorDrawX,cx
		jnz	@@019E
		mov	dx,word ptr ClickDown+2
		cmp	word ptr CursorDrawY,dx
		jz	@@01E5
@@019E:
		cmp	word ptr MouseHidden,0
		jnz	@@01E5
		test	word ptr MouseFlags,8000h
		jz	@@01C8
		cmp	cx,word ptr HideX1
		jl	@@01B9
		cmp	cx,word ptr HideX2
		jg	@@01C8
@@01B9:
		cmp	dx,word ptr HideY1
		jl	@@01C8
		cmp	dx,word ptr HideY2
		jg	@@01C8
		jmp	@@0118
@@01C8:
		push	word ptr PagePtr+2
		push	word ptr PagePtr
		call	Drv_65C6
		add	sp,4
		call	far ptr cursor_hide
		call	far ptr cursor_show
		call	Drv_65B6
@@01E5:
		mov	word ptr MouseBusy,0
@@01EB:
		popf
		pop	dx
		pop	cx
		pop	bx
		pop	ax
		pop	ds
		retf
mouse_handler	ENDP

; 3269:01F2 Removes the handler
		PUBLIC	mouse_remove
mouse_remove	PROC	FAR
		push	bp
		mov	bp,sp
		cmp	byte ptr MousePresent,0
		jz	@@0211
		push	ax
		push	cx
		push	dx
		push	es
		mov	ax,0Ch
		mov	cx,0
		push	cs
		pop	es
		mov	dx,OFFSET mouse_handler
		int	33h
		pop	es
		pop	dx
		pop	cx
		pop	ax
@@0211:
		pop	bp
		retf
mouse_remove	ENDP

; 3269:0213 Non-zero if the cursor is in (x1, y1)-(x2, y2) (waits for the handler)
		PUBLIC	cursor_in_rect
cursor_in_rect	PROC	FAR
		push	bp
		mov	bp,sp
		push	cx
		push	di
		push	dx
		push	si
		mov	cx,word ptr [bp+6]
		mov	si,word ptr [bp+0Ah]
		mov	dx,word ptr [bp+8]
		mov	di,word ptr [bp+0Ch]
@@0226:
		cmp	word ptr MouseBusy,0
		jnz	@@0226
		inc	word ptr MouseBusy
		mov	ax,CursorDrawX
		mov	MouseTestX,ax
		mov	ax,CursorDrawY
		mov	MouseTestY,ax
		cmp	word ptr MouseTestX,cx
		jl	@@025A
		cmp	word ptr MouseTestX,si
		jg	@@025A
		cmp	word ptr MouseTestY,dx
		jl	@@025A
		cmp	word ptr MouseTestY,di
		jg	@@025A
		mov	ax,1
		jmp	short @@025D
@@025A:
		mov	ax,0
@@025D:
		dec	word ptr MouseBusy
		pop	si
		pop	dx
		pop	di
		pop	cx
		pop	bp
		retf
cursor_in_rect	ENDP

; 3269:0267 sti
		PUBLIC	ints_on
ints_on	PROC	FAR
		sti
		retf
ints_on	ENDP

; 3269:0269 cli
		PUBLIC	ints_off
ints_off	PROC	FAR
		cli
		retf
ints_off	ENDP

; 3269:026B The page shown: its 16-byte descriptor moves to p
		PUBLIC	set_page_ptr
set_page_ptr	PROC	FAR
		push	bp
		mov	bp,sp
		push	ax
		push	bx
		push	cx
		push	ds
		push	si
		push	es
		push	di
		pushf
		cld
		mov	ax,DGROUP
		mov	ds,ax
		mov	ax,word ptr [bp+6]
		mov	bx,word ptr [bp+8]
		cli
		mov	si,word ptr PagePtr
		mov	cx,word ptr PagePtr+2
		mov	word ptr PagePtr,ax
		mov	word ptr PagePtr+2,bx
		jcxz	@@029F
		mov	ds,cx
		mov	di,ax
		mov	es,bx
		mov	cx,10h
		rep	movsb
@@029F:
		sti
		popf
		pop	di
		pop	es
		pop	si
		pop	ds
		pop	cx
		pop	bx
		pop	ax
		pop	bp
		retf
set_page_ptr	ENDP

; 3269:02AA The descriptor of the page shown
		PUBLIC	get_page_ptr
get_page_ptr	PROC	FAR
		push	bp
		mov	bp,sp
		push	ds
		mov	ax,DGROUP
		mov	ds,ax
		cli
		mov	ax,word ptr PagePtr
		mov	dx,word ptr PagePtr+2
		sti
		pop	ds
		pop	bp
		retf
get_page_ptr	ENDP



		.DATA			; Initialised data of the module (DS:651C)
		PUBLIC	Drv_653E
		dw	5Ch		; 651C
MouseTestX	dw	0
MouseTestY	dw	0
; 6522 the table given to the video driver: variables and procedures
DriverTable	dd	DGROUP:CurFont
		dd	DGROUP:PageSegs
		dd	DGROUP:Var_62A5
		dd	mem_alloc
		dd	mem_free
		dd	get_page_ptr
		dd	set_page_ptr
; 653E the entries of the video driver (filled when MGA.OVL is loaded)
Drv_653E	dd	0
Drv_6542	dd	0
Drv_6546	dd	0
Drv_654A	dd	0
Drv_654E	dd	0
Drv_6552	dd	0
Drv_6556	dd	0
Drv_655A	dd	0
Drv_655E	dd	0
Drv_6562	dd	0
Drv_6566	dd	0
Drv_656A	dd	0
Drv_656E	dd	0
Drv_6572	dd	0
Drv_6576	dd	0
Drv_657A	dd	0
Drv_657E	dd	0
Drv_6582	dd	0
Drv_6586	dd	0
Drv_658A	dd	0
Drv_658E	dd	0
Drv_6592	dd	0
Drv_6596	dd	0
Drv_659A	dd	0
Drv_659E	dd	0
Drv_65A2	dd	0
Drv_65A6	dd	0
Drv_65AA	dd	0
Drv_65AE	dd	0
Drv_65B2	dd	0
Drv_65B6	dd	0
Drv_65BA	dd	0
Drv_65BE	dd	0
Drv_65C2	dd	0
Drv_65C6	dd	0
Drv_65CA	dd	0
Drv_65CE	dd	0
Drv_65D2	dd	0
Drv_65D6	dd	0
Drv_65DA	dd	0
Drv_65DE	dd	0
Drv_65E2	dd	0
Drv_65E6	dd	0
Drv_65EA	dd	0
Drv_65EE	dd	0
Drv_65F2	dd	0
Var_65F6	dw	0		; used by the 3D view (2A68)
Var_65F8	dw	0
SoundEnabled	dw	1
Var_65FC	dw	1		; the music


		END
