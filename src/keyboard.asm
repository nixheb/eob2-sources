;
; Eye of the Beholder II - START.EXE
; The keyboard: its handler (INT 9) fills a buffer of keys in the code
; segment, translated by the tables there; the input can also be recorded
; and played back (Var_6600); the mouse buttons come as keys; module 2295
; (Ghidra 3295), written in assembler. Its tables come first: the
; segment starts at 3295:0000 (code segment).
;
; Assembled with
;     tasm /ml keyboard.asm
; [asm] = identical to START.EXE byte for byte (tools/asm.py).
;
		.MODEL	LARGE, C
		LOCALS
		INCLUDE	procs.inc
		EXTRN	read_file:FAR, write_file:FAR

		.DATA
		INCLUDE	eob2.inc

; The code is in a segment of its own: the default one of .MODEL stays
; empty (the overlay manager's segment table lists both)
		.CODE	KBD_TEXT
T_0000		db	001h, 002h, 004h, 008h, 010h, 020h, 040h, 080h, 020h, 002h, 020h, 003h, 00Ch, 006h, 00Dh, 007h
		db	06Ah, 006h, 069h, 006h, 030h, 002h, 030h, 003h, 07Dh, 000h, 07Dh, 001h, 05Ah, 002h, 05Ah, 003h
		db	000h, 002h, 008h, 000h, 008h, 000h, 010h, 000h, 010h, 000h, 010h, 000h, 010h, 000h, 020h, 000h
		db	020h, 000h, 040h, 000h, 040h, 000h, 080h, 000h, 080h, 000h, 000h, 001h
T_003C		db	000h, 000h
T_003E		db	000h, 000h, 038h, 01Dh, 052h, 053h, 04Bh, 047h, 04Fh, 048h, 050h, 049h, 051h, 04Dh, 035h, 01Ch
		db	037h, 046h, 03Eh, 040h, 04Bh, 04Ch, 04Fh, 050h, 051h, 053h, 054h, 055h, 056h, 059h, 05Fh, 06Ch
		db	07Ch, 000h, 03Ch, 03Ah, 063h, 068h, 05Ch, 05Bh, 05Dh, 060h, 062h, 065h, 067h, 066h, 037h, 02Bh
		db	07Ch, 000h
T_0070		db	000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h
		db	000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h
		db	000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h
		db	000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h
		db	000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h
		db	000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h
		db	000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h
		db	000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h
		db	000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h
		db	000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h
		db	000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h
		db	000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h
		db	000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h
		db	000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h
		db	000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h
		db	000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h
T_0170		db	000h, 000h
T_0172		db	000h, 000h
T_0174		db	010h, 001h
T_0176		db	000h, 000h
T_0178		db	000h, 000h
T_017A		db	07Fh, 06Eh, 002h, 003h, 004h, 005h, 006h, 007h, 008h, 009h, 00Ah, 00Bh, 00Ch, 00Dh, 00Fh, 010h
		db	011h, 012h, 013h, 014h, 015h, 016h, 017h, 018h, 019h, 01Ah, 01Bh, 01Ch, 02Bh, 03Ah, 01Fh, 020h
		db	021h, 022h, 023h, 024h, 025h, 026h, 027h, 028h, 029h, 001h, 02Ch, 01Dh, 02Eh, 02Fh, 030h, 031h
		db	032h, 033h, 034h, 035h, 036h, 037h, 039h, 064h, 03Ch, 03Dh, 01Eh, 070h, 071h, 072h, 073h, 074h
		db	075h, 076h, 077h, 078h, 079h, 05Ah, 07Dh, 05Bh, 060h, 065h, 069h, 05Ch, 061h, 066h, 06Ah, 05Dh
		db	062h, 067h, 063h, 068h, 07Fh, 07Fh, 07Fh, 07Ah, 07Bh
T_01D3		db	000h, 000h, 0FEh, 087h, 0FFh, 0C0h, 01Fh, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h
T_01E3		db	000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 038h, 0EFh, 001h, 000h, 000h
T_01F3		db	000h, 000h, 000h, 000h, 000h
T_01F8		db	000h, 000h
T_01FA		db	000h
T_01FB		db	000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h
T_0203		db	000h
T_0204		db	000h
T_0205		db	000h, 000h
T_0207		db	000h, 000h
T_0209		db	000h, 000h
T_020B		db	000h, 000h, 01Eh, 02Ch, 039h, 03Ah, 03Ch, 03Eh, 040h, 05Ah, 080h
T_0216		db	000h

; 3295:0217
kbd_0217	PROC	NEAR
@@0217:
		push	bx
		push	cx
		push	di
		push	ds
		mov	bx,cs
		mov	ds,bx
		cmp	al,6Eh
		jnz	@@0226
		jmp	@@02E8
@@0226:
		cmp	al,3Eh
		jnc	@@0280
		mov	bx,ax
		and	bx,3Fh
		test	ah,1
		jnz	@@0252
		test	word ptr cs:T_0178,1
		jnz	@@024B
		mov	al,byte ptr cs:T_02EF[bx]
		jmp	short @@0260
@@0244:
		mov	al,byte ptr cs:T_036B[bx]
		jmp	short @@0260
@@024B:
		mov	al,byte ptr cs:T_03A9[bx]
		jmp	short @@0260
@@0252:
		test	word ptr cs:T_0178,1
		jnz	@@0244
		mov	al,byte ptr cs:T_032D[bx]
@@0260:
		test	ah,2
		jz	@@027E
		mov	di,bx
		and	di,7
		mov	cl,byte ptr cs:T_0000[di]
		shr	bx,1
		shr	bx,1
		shr	bx,1
		test	byte ptr cs:T_01D3[bx],cl
		jz	@@027E
		and	al,1Fh
@@027E:
		jmp	short @@02EA
@@0280:
		cmp	al,4Bh
		jc	@@02DC
		cmp	al,6Eh
		jnc	@@02B2
		xor	bh,bh
		mov	bl,al
		sub	bl,4Bh
		test	word ptr cs:T_0174,200h
		jnz	@@02A1
		test	word ptr cs:T_0178,2
		jnz	@@02A8
@@02A1:
		mov	al,byte ptr cs:T_03E7[bx]
		jmp	short @@02EA
@@02A8:
		sub	bl,0Fh
		mov	al,byte ptr cs:T_040A[bx]
		jmp	short @@02EA
@@02B2:
		cmp	al,70h
		jc	@@02E4
		cmp	al,79h
		ja	@@02E4
		mov	bl,al
		sub	bl,70h
		mov	bh,0C5h
		test	ah,7
		jz	@@02D6
		mov	bh,98h
		test	ah,4
		jnz	@@02D6
		mov	bh,0A2h
		test	ah,2
		jnz	@@02D6
		mov	bh,0ACh
@@02D6:
		sub	bh,bl
		mov	al,bh
		jmp	short @@02EA
@@02DC:
		cmp	al,41h
		jc	@@02E4
		add	al,85h
		jmp	short @@02EA
@@02E4:
		or	al,80h
		jmp	short @@02EA
@@02E8:
		mov	al,1Bh
@@02EA:
		pop	ds
		pop	di
		pop	cx
		pop	bx
		retn
T_02EF		db	000h, 060h, 031h, 032h, 033h, 034h, 035h, 036h, 037h, 038h, 039h, 030h, 02Dh, 03Dh, 000h, 008h
		db	009h, 071h, 077h, 065h, 072h, 074h, 079h, 075h, 069h, 06Fh, 070h, 05Bh, 05Dh, 05Ch, 000h, 061h
		db	073h, 064h, 066h, 067h, 068h, 06Ah, 06Bh, 06Ch, 03Bh, 027h, 000h, 00Dh, 000h, 000h, 07Ah, 078h
		db	063h, 076h, 062h, 06Eh, 06Dh, 02Ch, 02Eh, 02Fh, 000h, 000h, 000h, 000h, 000h, 020h
T_032D		db	000h, 07Eh, 021h, 040h, 023h, 024h, 025h, 05Eh, 026h, 02Ah, 028h, 029h, 05Fh, 02Bh, 000h, 008h
		db	009h, 051h, 057h, 045h, 052h, 054h, 059h, 055h, 049h, 04Fh, 050h, 07Bh, 07Dh, 07Ch, 000h, 041h
		db	053h, 044h, 046h, 047h, 048h, 04Ah, 04Bh, 04Ch, 03Ah, 022h, 000h, 00Dh, 000h, 000h, 05Ah, 058h
		db	043h, 056h, 042h, 04Eh, 04Dh, 03Ch, 03Eh, 03Fh, 000h, 000h, 000h, 000h, 000h, 020h
T_036B		db	000h, 07Eh, 021h, 040h, 023h, 024h, 025h, 05Eh, 026h, 02Ah, 028h, 029h, 05Fh, 02Bh, 000h, 008h
		db	009h, 071h, 077h, 065h, 072h, 074h, 079h, 075h, 069h, 06Fh, 070h, 07Bh, 07Dh, 07Ch, 000h, 061h
		db	073h, 064h, 066h, 067h, 068h, 06Ah, 06Bh, 06Ch, 03Ah, 022h, 000h, 00Dh, 000h, 000h, 07Ah, 078h
		db	063h, 076h, 062h, 06Eh, 06Dh, 03Ch, 03Eh, 03Fh, 000h, 000h, 000h, 000h, 000h, 020h
T_03A9		db	000h, 060h, 031h, 032h, 033h, 034h, 035h, 036h, 037h, 038h, 039h, 030h, 02Dh, 03Dh, 000h, 008h
		db	009h, 051h, 057h, 045h, 052h, 054h, 059h, 055h, 049h, 04Fh, 050h, 05Bh, 05Dh, 05Ch, 000h, 041h
		db	053h, 044h, 046h, 047h, 048h, 04Ah, 04Bh, 04Ch, 03Bh, 027h, 000h, 00Dh, 000h, 000h, 05Ah, 058h
		db	043h, 056h, 042h, 04Eh, 04Dh, 02Ch, 02Eh, 02Fh, 000h, 000h, 000h, 000h, 000h, 020h
T_03E7		db	0AEh, 0ADh, 000h, 000h, 0B5h, 0B9h, 0B1h, 000h, 0B8h, 0B0h, 0B7h, 0AFh, 000h, 000h, 0B3h, 000h
		db	0B9h, 0B5h, 0B1h, 000h, 02Fh, 0B8h, 0B4h, 0B0h, 0AEh, 02Ah, 0B7h, 0B3h, 0AFh, 0ADh, 02Dh, 02Bh
		db	000h, 00Dh, 000h
T_040A		db	000h, 037h, 034h, 031h, 000h, 02Fh, 038h, 035h, 032h, 030h, 02Ah, 039h, 036h, 033h, 02Eh, 02Dh
		db	02Bh, 000h, 00Dh, 000h
kbd_0217	ENDP

; 3295:041E
		PUBLIC	key_pressed
key_pressed	PROC	FAR
		call	kbd_0DC1
		push	cx
		push	di
		push	si
		push	es
		cld
		mov	ax,cs
		mov	es,ax
@@042A:
		cli
		mov	ax,word ptr cs:T_0170
		cmp	byte ptr Var_6600,2
		jz	@@043D
		cmp	ax,word ptr cs:T_0172
		jz	@@047E
@@043D:
		mov	si,ax
		mov	ax,word ptr cs:T_0070[si]
		cmp	byte ptr Var_6600,2
		jnz	@@044F
		or	ax,ax
		jz	@@0480
@@044F:
		mov	di,20Dh
		mov	cx,9
		repne	scasb
		or	cx,cx
		jnz	@@0464
		test	ah,8
		jnz	@@0464
		cmp	al,7Ah
		jc	@@0480
@@0464:
		cmp	al,41h
		jc	@@046F
		cmp	al,44h
		ja	@@046F
		add	si,4
@@046F:
		add	si,2
		and	si,0FFh
		mov	word ptr cs:T_0170,si
		sti
		jmp	short @@042A
@@047E:
		xor	ax,ax
@@0480:
		mov	cx,word ptr cs:T_0176
		mov	word ptr cs:T_0178,cx
		sti
		or	ax,ax
		jz	@@0494
		call	kbd_0217
		xor	ah,ah
@@0494:
		sti
		pop	es
		pop	si
		pop	di
		pop	cx
		retf
key_pressed	ENDP

; 3295:049A
		PUBLIC	key_ready
key_ready	PROC	FAR
		call	kbd_0DC1
		push	cx
		push	di
		push	si
		push	es
		cld
		mov	ax,cs
		mov	es,ax
@@04A6:
		cli
		mov	ax,word ptr cs:T_0170
		cmp	byte ptr Var_6600,2
		jz	@@04B9
		cmp	ax,word ptr cs:T_0172
		jz	@@04EE
@@04B9:
		mov	si,ax
		mov	ax,word ptr cs:T_0070[si]
		cmp	byte ptr Var_6600,2
		jnz	@@04CB
		or	ax,ax
		jz	@@04F0
@@04CB:
		mov	di,20Dh
		mov	cx,9
		repne	scasb
		or	cx,cx
		jnz	@@04DF
		or	al,al
		jz	@@04DF
		cmp	al,7Ah
		jc	@@04F0
@@04DF:
		add	si,2
		and	si,0FFh
		mov	word ptr cs:T_0170,si
		sti
		jmp	short @@04A6
@@04EE:
		xor	ax,ax
@@04F0:
		mov	cx,word ptr cs:T_0176
		mov	word ptr cs:T_0178,cx
		sti
		or	ax,ax
		jz	@@0502
		call	kbd_0217
@@0502:
		pop	es
		pop	si
		pop	di
		pop	cx
		retf
key_ready	ENDP

; 3295:0507
		PUBLIC	key_waiting
key_waiting	PROC	FAR
		cli
		mov	ax,word ptr cs:T_0170
		xor	ax,word ptr cs:T_0172
		sti
		call	kbd_0DC1
		retf
key_waiting	ENDP

; 3295:0516
		PUBLIC	flush_keys
flush_keys	PROC	FAR
		push	ax
		cli
		mov	ax,word ptr cs:T_0170
		mov	word ptr cs:T_0172,ax
		sti
		pop	ax
		retf
flush_keys	ENDP

; 3295:0523
kbd_0523	PROC	NEAR
@@0523:
		push	ds
		push	di
		mov	di,ax
		mov	ax,DGROUP
		mov	ds,ax
		mov	ax,word ptr cs:T_0070[di]
		cmp	byte ptr Var_6600,2
		jnz	@@053B
		mov	ax,Var_6603
@@053B:
		mov	Var_6603,ax
		add	di,2
		and	di,0FFh
		cmp	al,41h
		jc	@@05C1
		cmp	al,42h
		ja	@@0589
		push	ax
		mov	ax,word ptr cs:T_0070[di]
		cmp	byte ptr Var_6600,2
		jnz	@@055D
		mov	ax,Var_6607
@@055D:
		mov	Var_6607,ax
		add	di,2
		and	di,0FFh
		mov	word ptr ClickUp,ax
		mov	ax,word ptr cs:T_0070[di]
		cmp	byte ptr Var_6600,2
		jnz	@@0579
		mov	ax,Var_6609
@@0579:
		mov	Var_6609,ax
		add	di,2
		and	di,0FFh
		mov	word ptr ClickUp+2,ax
		pop	ax
		jmp	short @@05C1
@@0589:
		cmp	al,44h
		ja	@@05C1
		push	ax
		mov	ax,word ptr cs:T_0070[di]
		cmp	byte ptr Var_6600,2
		jnz	@@059D
		mov	ax,Var_6607
@@059D:
		mov	Var_6607,ax
		add	di,2
		and	di,0FFh
		mov	ax,word ptr cs:T_0070[di]
		cmp	byte ptr Var_6600,2
		jnz	@@05B6
		mov	ax,Var_6609
@@05B6:
		mov	Var_6609,ax
		add	di,2
		and	di,0FFh
		pop	ax
@@05C1:
		cmp	byte ptr Var_6600,2
		jz	@@05CD
		mov	word ptr cs:T_0170,di
@@05CD:
		pop	di
		pop	ds
		retn
kbd_0523	ENDP

; 3295:05D0
		PUBLIC	get_key
get_key	PROC	FAR
		push	bx
		push	cx
		push	di
		push	ds
		push	es
		cld
		mov	ax,cs
		mov	es,ax
@@05DA:
		cli
		cmp	byte ptr Var_6600,2
		jz	@@05F0
		mov	ax,word ptr cs:T_0170
		cmp	ax,word ptr cs:T_0172
		jnz	@@05F0
		sti
		jmp	short @@05DA
@@05F0:
		call	kbd_0523
		mov	cx,word ptr cs:T_0176
		mov	word ptr cs:T_0178,cx
		sti
		mov	di,20Dh
		mov	cx,9
		repne	scasb
		or	cx,cx
		jnz	@@05DA
		test	ah,8
		jnz	@@05DA
		cmp	al,7Ah
		jnc	@@05DA
		call	kbd_0217
		pop	es
		pop	ds
		pop	di
		pop	cx
		pop	bx
		call	kbd_0CB2
		xor	ah,ah
		retf
get_key	ENDP

; 3295:0621
		PUBLIC	get_ascii
get_ascii	PROC	FAR
		push	bx
		push	cx
		push	di
		push	ds
		push	es
		cld
		mov	ax,cs
		mov	es,ax
		mov	ds,ax
@@062D:
		cli
		cmp	byte ptr Var_6600,2
		jz	@@0643
		mov	ax,word ptr cs:T_0170
		cmp	ax,word ptr cs:T_0172
		jnz	@@0643
		sti
		jmp	short @@062D
@@0643:
		call	kbd_0523
		mov	cx,word ptr cs:T_0176
		mov	word ptr cs:T_0178,cx
		sti
		mov	di,20Dh
		mov	cx,9
		repne	scasb
		or	cx,cx
		jnz	@@062D
		cmp	al,7Ah
		jnc	@@062D
		call	kbd_0217
		pop	es
		pop	ds
		pop	di
		pop	cx
		pop	bx
		call	kbd_0CB2
		retf
get_ascii	ENDP

; 3295:066D
		PUBLIC	read_key
read_key	PROC	FAR
@@066D:
		cli
		cmp	byte ptr Var_6600,2
		jz	@@0683
		mov	ax,word ptr cs:T_0170
		cmp	ax,word ptr cs:T_0172
		jnz	@@0683
		sti
		jmp	short @@066D
@@0683:
		call	kbd_0523
		sti
		call	kbd_0CB2
		retf
read_key	ENDP

; 3295:068B
		PUBLIC	mouse_down
mouse_down	PROC	FAR
		push	bp
		mov	bp,sp
		mov	ax,word ptr [bp+6]
		pop	bp
		push	di
		push	bx
		mov	byte ptr cs:T_0708,0
		push	ds
		push	ax
		mov	ax,DGROUP
		mov	ds,ax
		mov	ax,Var_65FE
		mov	word ptr cs:T_0174,ax
		pop	ax
		pop	ds
		test	word ptr cs:T_0174,2
		jnz	@@06E5
		mov	di,0Eh
@@06B6:
		dec	di
		jl	@@06E5
		cmp	al,byte ptr cs:T_0711[di]
		jnz	@@06B6
		push	ax
		mov	al,byte ptr cs:T_071F[di]
		xor	ah,ah
		mov	di,ax
		shr	di,1
		shr	di,1
		shr	di,1
		mov	bx,ax
		and	bx,7
		mov	al,byte ptr cs:T_0709[bx]
		cli
		and	al,byte ptr cs:T_01F3[di]
		mov	cs:T_0708,al
		pop	ax
@@06E5:
		xor	ah,ah
		mov	di,ax
		shr	di,1
		shr	di,1
		shr	di,1
		mov	bx,ax
		and	bx,7
		mov	al,byte ptr cs:T_0709[bx]
		cli
		and	al,byte ptr cs:T_01F3[di]
		or	al,byte ptr cs:T_0708
		sti
		pop	bx
		pop	di
		retf
T_0708		db	000h
T_0709		db	001h, 002h, 004h, 008h, 010h, 020h, 040h, 080h
T_0711		db	03Ch, 03Ah, 063h, 068h, 05Ch, 05Bh, 05Dh, 060h, 062h, 065h, 067h, 066h, 037h, 02Bh
T_071F		db	03Eh, 040h, 04Bh, 04Ch, 04Fh, 050h, 051h, 053h, 054h, 055h, 056h, 059h, 05Fh, 06Ch, 053h, 051h
		db	057h, 08Bh, 0D8h, 00Eh, 0E8h, 055h, 0FFh, 074h, 074h, 08Bh, 0C3h, 032h, 0E4h, 08Bh, 0F8h, 0D1h
		db	0EFh, 0D1h, 0EFh, 0D1h, 0EFh, 0B5h, 001h, 08Ah, 0C8h, 080h, 0E1h, 007h, 0D2h, 0E5h, 02Eh, 0F6h
		db	006h, 0FAh, 001h, 004h, 075h, 008h, 02Eh, 0F6h, 006h, 0FBh, 001h, 001h, 074h, 003h, 080h, 0CCh
		db	002h, 02Eh, 0F6h, 006h, 0FAh, 001h, 050h, 074h, 003h, 080h, 0CCh, 004h, 02Eh, 0F6h, 006h, 0F8h
		db	001h, 010h, 075h, 008h, 02Eh, 0F6h, 006h, 0FAh, 001h, 002h, 074h, 003h, 080h, 0CCh, 001h, 02Eh
		db	0F7h, 006h, 076h, 001h, 001h, 000h, 074h, 00Ah, 02Eh, 084h, 0ADh, 0D3h, 001h, 074h, 003h, 080h
		db	0F4h, 001h, 02Eh, 0F7h, 006h, 076h, 001h, 002h, 000h, 074h, 00Ah, 02Eh, 084h, 0ADh, 0E3h, 001h
		db	074h, 003h, 080h, 0F4h, 001h, 0B0h, 0FFh, 03Ah, 0E7h, 074h, 002h, 033h, 0C0h, 05Fh, 059h, 05Bh
		db	00Bh, 0C0h, 0CBh
mouse_down	ENDP

; 3295:07B2
		PUBLIC	keyboard_install
keyboard_install	PROC	FAR
		push	ax
		push	bx
		push	ds
		push	dx
		push	es
		mov	ax,40h
		mov	es,ax
		mov	al,es:[17h]
		test	al,40h
		jz	@@07CA
		or	word ptr cs:T_0176,1
@@07CA:
		test	al,20h
		jz	@@07D4
		or	word ptr cs:T_0176,2
@@07D4:
		test	al,2
		jz	@@07DE
		or	byte ptr cs:T_01F8,10h
@@07DE:
		and	al,1
		shl	al,1
		mov	ah,al
		mov	al,es:[18h]
		shl	al,1
		shl	al,1
		and	al,0Ch
		or	ah,al
		mov	al,es:[96h]
		test	al,8
		jz	@@07FB
		or	ah,40h
@@07FB:
		mov	byte ptr cs:T_01FA,ah
		test	al,4
		jz	@@080A
		or	byte ptr cs:T_01FB,1
@@080A:
		test	al,2
		jz	@@0814
		mov	byte ptr cs:T_0203,1
@@0814:
		test	al,1
		jz	@@081E
		mov	byte ptr cs:T_0204,2
@@081E:
		mov	ax,3509h
		int	21h
		mov	word ptr cs:T_0205,bx
		mov	word ptr cs:T_0207,es
		mov	bx,cs
		mov	ds,bx
		mov	dx,OFFSET keyboard_handler
		mov	ax,2509h
		int	21h
		mov	ax,3523h
		int	21h
		mov	word ptr cs:T_0209,bx
		mov	word ptr cs:T_020B,es
		mov	bx,cs
		mov	ds,bx
		mov	dx,OFFSET break_handler
		mov	ax,2523h
		int	21h
		pop	es
		pop	dx
		pop	ds
		pop	bx
		pop	ax
		retf
keyboard_install	ENDP

; 3295:085A
		PUBLIC	keyboard_handler
keyboard_handler	PROC	FAR
		push	ax
		push	bx
		push	cx
		push	di
		push	ds
		push	dx
		push	es
		push	si
		cld
		and	word ptr cs:T_0176,0FFFCh
		mov	ax,40h
		mov	es,ax
		test	byte ptr es:[17h],40h
		jz	@@087C
		or	word ptr cs:T_0176,1
@@087C:
		test	byte ptr es:[17h],20h
		jz	@@088A
		or	word ptr cs:T_0176,2
@@088A:
		mov	ax,DGROUP
		mov	ds,ax
		mov	ax,Var_65FE
		mov	word ptr cs:T_0174,ax
		mov	dx,word ptr cs:T_0174
		mov	ax,cs
		mov	ds,ax
		mov	es,ax
		cld
		xor	ah,ah
		mov	bx,101h
		in	al,60h
		cmp	al,0E1h
		jnz	@@08B3
		mov	byte ptr cs:T_0204,3
@@08B3:
		cmp	byte ptr cs:T_0204,0
		jz	@@08C9
		dec	byte ptr cs:T_0204
		test	dx,80h
		jnz	@@08D3
		jmp	@@0A65
@@08C9:
		cmp	al,0E0h
		jnz	@@08D6
		mov	byte ptr cs:T_0203,1
@@08D3:
		jmp	@@0A58
@@08D6:
		test	al,80h
		jz	@@08DE
		xor	bl,bl
		and	al,7Fh
@@08DE:
		cmp	byte ptr cs:T_0203,0
		jz	@@090A
		mov	byte ptr cs:T_0203,0
		mov	byte ptr cs:T_0A8D,1
		mov	di,40h
		mov	cx,11h
		repne	scasb
		mov	al,byte ptr [di+0Fh]
		or	cx,cx
		jnz	@@0911
		mov	byte ptr cs:T_0A8D,0
		jmp	@@0A65
@@090A:
		mov	di,ax
		mov	al,byte ptr cs:T_017A[di]
@@0911:
		mov	word ptr cs:T_0A8E,di
		mov	di,ax
		shr	di,1
		shr	di,1
		shr	di,1
		mov	cl,al
		and	cl,7
		shl	bl,cl
		shl	bh,cl
		not	bh
		mov	cl,byte ptr cs:T_01F3[di]
		and	cl,bl
		and	byte ptr cs:T_01F3[di],bh
		or	byte ptr cs:T_01F3[di],bl
		test	byte ptr cs:T_01FA,4
		jnz	@@094A
		test	byte ptr cs:T_01FB,1
		jz	@@094D
@@094A:
		or	ah,2
@@094D:
		test	byte ptr cs:T_01FA,50h
		jz	@@0958
		or	ah,4
@@0958:
		cmp	byte ptr cs:T_0A8D,0
		jz	@@0979
		mov	byte ptr cs:T_0A8D,0
		test	dx,2
		jnz	@@0979
		mov	si,word ptr cs:T_0A8E
		mov	al,byte ptr [si+1Fh]
		cmp	si,21h
		jge	@@09B2
@@0979:
		test	byte ptr cs:T_01F8,10h
		jnz	@@0989
		test	byte ptr cs:T_01FA,2
		jz	@@098C
@@0989:
		or	ah,1
@@098C:
		test	word ptr cs:T_0176,1
		jz	@@099F
		test	byte ptr cs:T_01D3[di],ch
		jz	@@099F
		xor	ah,1
@@099F:
		test	word ptr cs:T_0176,2
		jz	@@09B2
		test	byte ptr cs:T_01E3[di],ch
		jz	@@09B2
		xor	ah,1
@@09B2:
		mov	byte ptr cs:T_0216,ah
		test	dx,1
		jnz	@@09C4
		or	cl,cl
		jz	@@09C4
		jmp	@@0A65
@@09C4:
		cmp	ax,668h
		jz	@@09D2
		cmp	ax,64Ch
		jz	@@09D2
		or	bl,bl
		jnz	@@09D5
@@09D2:
		jmp	@@0A58
@@09D5:
		mov	di,word ptr cs:T_0172
		mov	si,di
		add	di,2
		and	di,0FFh
		cmp	di,word ptr cs:T_0170
		jz	@@09F4
		mov	word ptr cs:T_0070[si],ax
		mov	word ptr cs:T_0172,di
@@09F4:
		mov	di,20Dh
		mov	cx,9
		repne	scasb
		or	cx,cx
		jnz	@@0A58
		cmp	ax,420h
		jz	@@0A0C
		cmp	ax,434h
		jz	@@0A1C
		jnz	@@0A2A
@@0A0C:
		push	ds
		push	ax
		mov	ax,DGROUP
		mov	ds,ax
		xor	word ptr SoundEnabled,1
		pop	ax
		pop	ds
		jmp	short @@0A2A
@@0A1C:
		push	ds
		push	ax
		mov	ax,DGROUP
		mov	ds,ax
		xor	word ptr Var_65FC,1
		pop	ax
		pop	ds
@@0A2A:
		cmp	al,6Eh
		jnz	@@0A3B
		cmp	word ptr cs:T_003E,0
		jz	@@0A3B
		call	dword ptr cs:T_003C
@@0A3B:
		mov	di,8
		mov	cx,1Ch
		shr	cx,1
		repne	scasw
		or	cx,cx
		jz	@@0A52
		mov	bx,word ptr [di+18h]
		and	bx,dx
		jnz	@@0A58
		jmp	short @@0A65
@@0A52:
		test	dx,4
		jz	@@0A65
@@0A58:
		pop	si
		pop	es
		pop	dx
		pop	ds
		pop	di
		pop	cx
		pop	bx
		pop	ax
		jmp	dword ptr cs:T_0205
@@0A65:
		in	al,61h
		mov	ah,al
		or	al,80h
		out	61h,al
		xchg	ah,al
		out	61h,al
		mov	ax,40h
		mov	es,ax
		mov	al,es:[96h]
		and	al,0FDh
		mov	es:[96h],al
		mov	al,20h
		out	20h,al
		pop	si
		pop	es
		pop	dx
		pop	ds
		pop	di
		pop	cx
		pop	bx
		pop	ax
		iret
T_0A8D		db	000h
T_0A8E		db	000h, 000h
break_handler:	iret			; INT 23h (ctrl-break): ignored
keyboard_handler	ENDP

; 3295:0A91
		PUBLIC	keyboard_remove
keyboard_remove	PROC	FAR
		push	ax
		push	ds
		push	dx
		mov	dx,word ptr cs:T_020B
		mov	ds,dx
		mov	dx,word ptr cs:T_0209
		mov	ax,2523h
		int	21h
		mov	dx,word ptr cs:T_0207
		mov	ds,dx
		mov	dx,word ptr cs:T_0205
		mov	ax,2509h
		int	21h
		pop	dx
		pop	ds
		pop	ax
		retf
keyboard_remove	ENDP

; 3295:0ABA
		PUBLIC	kbd_0ABA
kbd_0ABA	PROC	FAR
		push	bp
		mov	bp,sp
		push	di
		push	si
		push	ds
		push	es
		mov	ax,DGROUP
		mov	es,ax
		mov	ax,cs
		mov	ds,ax
		mov	ax,word ptr [bp+6]
		and	ax,0FFFh
		cli
		or	al,al
		jnz	@@0AD8
@@0AD5:
		jmp	@@0BA0
@@0AD8:
		cmp	al,41h
		jnc	@@0ADF
		jmp	@@0B7F
@@0ADF:
		cmp	al,42h
		ja	@@0B33
		mov	di,word ptr cs:T_0172
		mov	si,di
		add	di,6
		and	di,0FFh
		cmp	si,di
		ja	@@0AFC
		cmp	si,word ptr cs:T_0170
		jnc	@@0B03
@@0AFC:
		cmp	di,word ptr cs:T_0170
		jnc	@@0AD5
@@0B03:
		mov	word ptr cs:T_0070[si],ax
		mov	bp,ax
		mov	ax,es:word ptr ClickDown
		add	si,2
		and	si,0FFh
		mov	word ptr cs:T_0070[si],ax
		mov	ax,es:word ptr ClickDown+2
		add	si,2
		and	si,0FFh
		mov	word ptr cs:T_0070[si],ax
		mov	word ptr cs:T_0172,di
		mov	ax,bp
		jmp	short @@0BA2
@@0B33:
		cmp	al,44h
		ja	@@0B7F
		mov	di,word ptr cs:T_0172
		mov	si,di
		add	di,2
		and	di,0FFh
		cmp	si,di
		ja	@@0B50
		cmp	si,word ptr cs:T_0170
		jnc	@@0B57
@@0B50:
		cmp	di,word ptr cs:T_0170
		jnc	@@0BA0
@@0B57:
		mov	word ptr cs:T_0070[si],ax
		mov	bp,ax
		add	si,2
		and	si,0FFh
		mov	word ptr cs:T_0070[si],ax
		add	si,2
		and	si,0FFh
		mov	word ptr cs:T_0070[si],ax
		mov	word ptr cs:T_0172,di
		mov	ax,bp
		jmp	short @@0BA2
@@0B7F:
		mov	di,word ptr cs:T_0172
		mov	si,di
		add	di,2
		and	di,0FFh
		cmp	di,word ptr cs:T_0170
		jz	@@0BA0
		mov	word ptr cs:T_0070[si],ax
		mov	word ptr cs:T_0172,di
		jmp	short @@0BA2
@@0BA0:
		xor	ax,ax
@@0BA2:
		sti
		pop	es
		pop	ds
		pop	si
		pop	di
		pop	bp
		retf
kbd_0ABA	ENDP

; 3295:0BA9
		PUBLIC	mouse_button_key
mouse_button_key	PROC	FAR
		push	bp
		mov	bp,sp
		push	bx
		push	cx
		push	dx
		push	di
		push	si
		push	ds
		push	es
		mov	dl,byte ptr [bp+6]
		mov	ch,byte ptr cs:T_0CB1
		xor	ch,dl
		mov	ax,ds
		mov	es,ax
		mov	ax,cs
		mov	ds,ax
		xor	ax,ax
		test	ch,1
		jz	@@0BF9
		mov	bx,101h
		mov	ax,41h
		test	dl,1
		jnz	@@0BD9
		xor	bl,bl
@@0BD9:
		call	kbd_0C8C
		mov	ah,byte ptr cs:T_0216
		test	word ptr cs:T_0176,1
		jz	@@0BED
		or	ah,1
@@0BED:
		or	bl,bl
		jnz	@@0BF4
		or	ah,8
@@0BF4:
		call	kbd_0C37
		jz	@@0C29
@@0BF9:
		test	ch,2
		jz	@@0C29
		mov	bx,101h
		mov	ax,42h
		test	dl,2
		jnz	@@0C0B
		xor	bl,bl
@@0C0B:
		call	kbd_0C8C
		mov	ah,byte ptr cs:T_0216
		test	word ptr cs:T_0176,1
		jz	@@0C1F
		or	ah,1
@@0C1F:
		or	bl,bl
		jnz	@@0C26
		or	ah,8
@@0C26:
		call	kbd_0C37
@@0C29:
		mov	byte ptr cs:T_0CB1,dl
		pop	es
		pop	ds
		pop	si
		pop	di
		pop	dx
		pop	cx
		pop	bx
		pop	bp
		retf
mouse_button_key	ENDP

; 3295:0C37
kbd_0C37	PROC	NEAR
@@0C37:
		mov	di,word ptr cs:T_0172
		mov	si,di
		add	di,6
		and	di,0FFh
		cmp	si,di
		ja	@@0C50
		cmp	si,word ptr cs:T_0170
		jnc	@@0C57
@@0C50:
		cmp	di,word ptr cs:T_0170
		jnc	@@0C87
@@0C57:
		mov	word ptr cs:T_0070[si],ax
		mov	bp,ax
		mov	ax,es:MouseRawX
		add	si,2
		and	si,0FFh
		mov	word ptr cs:T_0070[si],ax
		mov	ax,es:MouseRawY
		add	si,2
		and	si,0FFh
		mov	word ptr cs:T_0070[si],ax
		mov	word ptr cs:T_0172,di
		mov	ax,bp
		jmp	short @@0C89
@@0C87:
		xor	ax,ax
@@0C89:
		or	ax,ax
		retn
kbd_0C37	ENDP

; 3295:0C8C
kbd_0C8C	PROC	NEAR
@@0C8C:
		mov	di,ax
		shr	di,1
		shr	di,1
		shr	di,1
		mov	cl,al
		and	cl,7
		shl	bl,cl
		shl	bh,cl
		not	bh
		mov	cl,byte ptr cs:T_01F3[di]
		and	cl,bl
		and	byte ptr cs:T_01F3[di],bh
		or	byte ptr cs:T_01F3[di],bl
		retn
T_0CB1		db	000h
kbd_0C8C	ENDP

; 3295:0CB2
kbd_0CB2	PROC	NEAR
@@0CB2:
		push	ax
		cmp	byte ptr Var_6600,0
		jnz	@@0CBD
		jmp	@@0DB3
@@0CBD:
		mov	ax,Var_6603
		cmp	byte ptr Var_6600,1
		jnz	@@0D2C
		mov	word ptr cs:T_0DB9,ax
		mov	ax,Ticks16
		mov	word ptr cs:T_0DBB,ax
		push	word ptr cs:T_0DB7
		push	word ptr cs:T_0DB5
		push	cs
		mov	ax,0DB9h
		push	ax
		push	word ptr Var_6601
		call	far ptr write_file
		add	sp,0Ah
		mov	ax,word ptr cs:T_0DB9
		cmp	ax,41h
		jc	@@0D23
		cmp	al,44h
		ja	@@0D23
		mov	ax,Var_6607
		mov	word ptr cs:T_0DB9,ax
		mov	ax,Var_6609
		mov	word ptr cs:T_0DBB,ax
		push	word ptr cs:T_0DB7
		push	word ptr cs:T_0DB5
		push	cs
		mov	ax,0DB9h
		push	ax
		push	word ptr Var_6601
		call	far ptr write_file
		add	sp,0Ah
@@0D23:
		mov	word ptr Ticks16,0
@@0D29:
		jmp	@@0DB3
@@0D2C:
		cmp	byte ptr Var_6600,2
		jnz	@@0D29
		push	word ptr cs:T_0DB7
		push	word ptr cs:T_0DB5
		push	cs
		mov	ax,0DB9h
		push	ax
		push	word ptr Var_6601
		call	far ptr read_file
		add	sp,0Ah
		or	ax,ax
		jz	@@0DAD
		mov	ax,word ptr cs:T_0DBB
		mov	Var_6605,ax
		mov	ax,word ptr cs:T_0DB9
		mov	Var_6603,ax
		cmp	ax,41h
		jc	@@0DA5
		cmp	al,44h
		ja	@@0DA5
		push	word ptr cs:T_0DB7
		push	word ptr cs:T_0DB5
		push	cs
		mov	ax,0DB9h
		add	ax,4
		push	ax
		push	word ptr Var_6601
		call	far ptr read_file
		add	sp,0Ah
		or	ax,ax
		jz	@@0DAD
		mov	ax,word ptr cs:T_0DBD
		mov	Var_6607,ax
		mov	word ptr ClickDown,ax
		mov	ax,word ptr cs:T_0DBF
		mov	Var_6609,ax
		mov	word ptr ClickDown+2,ax
		mov	word ptr Var_6618,1
@@0DA5:
		mov	word ptr Ticks16,0
		jmp	short @@0DB3
@@0DAD:
		mov	word ptr Var_660B,1
@@0DB3:
		pop	ax
		retn
T_0DB5		db	004h, 000h
T_0DB7		db	000h, 000h
T_0DB9		db	000h, 000h
T_0DBB		db	000h, 000h
T_0DBD		db	000h, 000h
T_0DBF		db	000h, 000h
kbd_0CB2	ENDP

; 3295:0DC1
kbd_0DC1	PROC	NEAR
@@0DC1:
		cmp	byte ptr Var_6600,0
		jz	@@0DF3
		cmp	byte ptr Var_6600,1
		jz	@@0DF3
		cmp	word ptr Var_660B,0
		jnz	@@0DE4
		mov	ax,Var_6605
		cmp	word ptr Ticks16,ax
		jc	@@0DE4
		mov	ax,Var_6603
		jmp	short @@0DE7
@@0DE4:
		mov	ax,0
@@0DE7:
		push	di
		mov	di,word ptr cs:T_0170
		mov	word ptr cs:T_0070[di],ax
		pop	di
@@0DF3:
		retn
kbd_0DC1	ENDP


		.DATA			; Initialised data of the module (DS:65FE)
Var_65FE	dw	110h
Var_6600	db	0
Var_6601	dw	0
Var_6603	dw	0
Var_6605	dw	0
Var_6607	dw	0
Var_6609	dw	0
Var_660B	dw	0
		db	0

		END
