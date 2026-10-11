;
; Eye of the Beholder II - START.EXE
; The geometry of the 3D view: the tiles of the view composed into the work
; page (for each video mode), the direction and the distance between two
; blocks, the colours of the walls; module 1A68 (Ghidra 2A68), written in
; assembler. Its tables come first: the segment starts at 2A68:0002.
;
; Assembled with
;     tasm /ml geometry.asm
; [asm] = identical to START.EXE byte for byte (tools/asm.py).
;
		.MODEL	LARGE, C
		LOCALS
		INCLUDE	procs.inc

		.DATA
		INCLUDE	eob2.inc
; Initialised data of the module (DS:6180)
OctantDir	db	0FFh, 6, 2, 0FFh, 4, 5, 3, 0FFh, 0, 7, 1, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh	; 6180 octant: the sign bits -> a direction
Neighbours	dw	-33, -32, -31, -1, 1, 31, 32, 33		; 6190 the 8 blocks around
FarAhead	dw	-64, -96, 2, 3, 64, 96, -2, -3		; 61A0 per direction: 2 and 3 blocks away

; The code is in a segment of its own: the default one of .MODEL stays
; empty (the overlay manager's segment table lists both)
		.CODE	GEOM_TEXT
		db	000h, 080h, 040h, 0C0h, 020h, 0A0h, 060h, 0E0h, 010h, 090h, 050h, 0D0h, 030h, 0B0h, 070h, 0F0h
		db	008h, 088h, 048h, 0C8h, 028h, 0A8h, 068h, 0E8h, 018h, 098h, 058h, 0D8h, 038h, 0B8h, 078h, 0F8h
		db	004h, 084h, 044h, 0C4h, 024h, 0A4h, 064h, 0E4h, 014h, 094h, 054h, 0D4h, 034h, 0B4h, 074h, 0F4h
		db	00Ch, 08Ch, 04Ch, 0CCh, 02Ch, 0ACh, 06Ch, 0ECh, 01Ch, 09Ch, 05Ch, 0DCh, 03Ch, 0BCh, 07Ch, 0FCh
		db	002h, 082h, 042h, 0C2h, 022h, 0A2h, 062h, 0E2h, 012h, 092h, 052h, 0D2h, 032h, 0B2h, 072h, 0F2h
		db	00Ah, 08Ah, 04Ah, 0CAh, 02Ah, 0AAh, 06Ah, 0EAh, 01Ah, 09Ah, 05Ah, 0DAh, 03Ah, 0BAh, 07Ah, 0FAh
		db	006h, 086h, 046h, 0C6h, 026h, 0A6h, 066h, 0E6h, 016h, 096h, 056h, 0D6h, 036h, 0B6h, 076h, 0F6h
		db	00Eh, 08Eh, 04Eh, 0CEh, 02Eh, 0AEh, 06Eh, 0EEh, 01Eh, 09Eh, 05Eh, 0DEh, 03Eh, 0BEh, 07Eh, 0FEh
		db	001h, 081h, 041h, 0C1h, 021h, 0A1h, 061h, 0E1h, 011h, 091h, 051h, 0D1h, 031h, 0B1h, 071h, 0F1h
		db	009h, 089h, 049h, 0C9h, 029h, 0A9h, 069h, 0E9h, 019h, 099h, 059h, 0D9h, 039h, 0B9h, 079h, 0F9h
		db	005h, 085h, 045h, 0C5h, 025h, 0A5h, 065h, 0E5h, 015h, 095h, 055h, 0D5h, 035h, 0B5h, 075h, 0F5h
		db	00Dh, 08Dh, 04Dh, 0CDh, 02Dh, 0ADh, 06Dh, 0EDh, 01Dh, 09Dh, 05Dh, 0DDh, 03Dh, 0BDh, 07Dh, 0FDh
		db	003h, 083h, 043h, 0C3h, 023h, 0A3h, 063h, 0E3h, 013h, 093h, 053h, 0D3h, 033h, 0B3h, 073h, 0F3h
		db	00Bh, 08Bh, 04Bh, 0CBh, 02Bh, 0ABh, 06Bh, 0EBh, 01Bh, 09Bh, 05Bh, 0DBh, 03Bh, 0BBh, 07Bh, 0FBh
		db	007h, 087h, 047h, 0C7h, 027h, 0A7h, 067h, 0E7h, 017h, 097h, 057h, 0D7h, 037h, 0B7h, 077h, 0F7h
		db	00Fh, 08Fh, 04Fh, 0CFh, 02Fh, 0AFh, 06Fh, 0EFh, 01Fh, 09Fh, 05Fh, 0DFh, 03Fh, 0BFh, 07Fh, 0FFh
T_0102		db	000h, 040h, 080h, 0C0h, 010h, 050h, 090h, 0D0h, 020h, 060h, 0A0h, 0E0h, 030h, 070h, 0B0h, 0F0h
		db	004h, 044h, 084h, 0C4h, 014h, 054h, 094h, 0D4h, 024h, 064h, 0A4h, 0E4h, 034h, 074h, 0B4h, 0F4h
		db	008h, 048h, 088h, 0C8h, 018h, 058h, 098h, 0D8h, 028h, 068h, 0A8h, 0E8h, 038h, 078h, 0B8h, 0F8h
		db	00Ch, 04Ch, 08Ch, 0CCh, 01Ch, 05Ch, 09Ch, 0DCh, 02Ch, 06Ch, 0ACh, 0ECh, 03Ch, 07Ch, 0BCh, 0FCh
		db	001h, 041h, 081h, 0C1h, 011h, 051h, 091h, 0D1h, 021h, 061h, 0A1h, 0E1h, 031h, 071h, 0B1h, 0F1h
		db	005h, 045h, 085h, 0C5h, 015h, 055h, 095h, 0D5h, 025h, 065h, 0A5h, 0E5h, 035h, 075h, 0B5h, 0F5h
		db	009h, 049h, 089h, 0C9h, 019h, 059h, 099h, 0D9h, 029h, 069h, 0A9h, 0E9h, 039h, 079h, 0B9h, 0F9h
		db	00Dh, 04Dh, 08Dh, 0CDh, 01Dh, 05Dh, 09Dh, 0DDh, 02Dh, 06Dh, 0ADh, 0EDh, 03Dh, 07Dh, 0BDh, 0FDh
		db	002h, 042h, 082h, 0C2h, 012h, 052h, 092h, 0D2h, 022h, 062h, 0A2h, 0E2h, 032h, 072h, 0B2h, 0F2h
		db	006h, 046h, 086h, 0C6h, 016h, 056h, 096h, 0D6h, 026h, 066h, 0A6h, 0E6h, 036h, 076h, 0B6h, 0F6h
		db	00Ah, 04Ah, 08Ah, 0CAh, 01Ah, 05Ah, 09Ah, 0DAh, 02Ah, 06Ah, 0AAh, 0EAh, 03Ah, 07Ah, 0BAh, 0FAh
		db	00Eh, 04Eh, 08Eh, 0CEh, 01Eh, 05Eh, 09Eh, 0DEh, 02Eh, 06Eh, 0AEh, 0EEh, 03Eh, 07Eh, 0BEh, 0FEh
		db	003h, 043h, 083h, 0C3h, 013h, 053h, 093h, 0D3h, 023h, 063h, 0A3h, 0E3h, 033h, 073h, 0B3h, 0F3h
		db	007h, 047h, 087h, 0C7h, 017h, 057h, 097h, 0D7h, 027h, 067h, 0A7h, 0E7h, 037h, 077h, 0B7h, 0F7h
		db	00Bh, 04Bh, 08Bh, 0CBh, 01Bh, 05Bh, 09Bh, 0DBh, 02Bh, 06Bh, 0ABh, 0EBh, 03Bh, 07Bh, 0BBh, 0FBh
		db	00Fh, 04Fh, 08Fh, 0CFh, 01Fh, 05Fh, 09Fh, 0DFh, 02Fh, 06Fh, 0AFh, 0EFh, 03Fh, 07Fh, 0BFh, 0FFh
T_0202		db	000h, 010h, 020h, 030h, 040h, 050h, 060h, 070h, 080h, 090h, 0A0h, 0B0h, 0C0h, 0D0h, 0E0h, 0F0h
		db	001h, 011h, 021h, 031h, 041h, 051h, 061h, 071h, 081h, 091h, 0A1h, 0B1h, 0C1h, 0D1h, 0E1h, 0F1h
		db	002h, 012h, 022h, 032h, 042h, 052h, 062h, 072h, 082h, 092h, 0A2h, 0B2h, 0C2h, 0D2h, 0E2h, 0F2h
		db	003h, 013h, 023h, 033h, 043h, 053h, 063h, 073h, 083h, 093h, 0A3h, 0B3h, 0C3h, 0D3h, 0E3h, 0F3h
		db	004h, 014h, 024h, 034h, 044h, 054h, 064h, 074h, 084h, 094h, 0A4h, 0B4h, 0C4h, 0D4h, 0E4h, 0F4h
		db	005h, 015h, 025h, 035h, 045h, 055h, 065h, 075h, 085h, 095h, 0A5h, 0B5h, 0C5h, 0D5h, 0E5h, 0F5h
		db	006h, 016h, 026h, 036h, 046h, 056h, 066h, 076h, 086h, 096h, 0A6h, 0B6h, 0C6h, 0D6h, 0E6h, 0F6h
		db	007h, 017h, 027h, 037h, 047h, 057h, 067h, 077h, 087h, 097h, 0A7h, 0B7h, 0C7h, 0D7h, 0E7h, 0F7h
		db	008h, 018h, 028h, 038h, 048h, 058h, 068h, 078h, 088h, 098h, 0A8h, 0B8h, 0C8h, 0D8h, 0E8h, 0F8h
		db	009h, 019h, 029h, 039h, 049h, 059h, 069h, 079h, 089h, 099h, 0A9h, 0B9h, 0C9h, 0D9h, 0E9h, 0F9h
		db	00Ah, 01Ah, 02Ah, 03Ah, 04Ah, 05Ah, 06Ah, 07Ah, 08Ah, 09Ah, 0AAh, 0BAh, 0CAh, 0DAh, 0EAh, 0FAh
		db	00Bh, 01Bh, 02Bh, 03Bh, 04Bh, 05Bh, 06Bh, 07Bh, 08Bh, 09Bh, 0ABh, 0BBh, 0CBh, 0DBh, 0EBh, 0FBh
		db	00Ch, 01Ch, 02Ch, 03Ch, 04Ch, 05Ch, 06Ch, 07Ch, 08Ch, 09Ch, 0ACh, 0BCh, 0CCh, 0DCh, 0ECh, 0FCh
		db	00Dh, 01Dh, 02Dh, 03Dh, 04Dh, 05Dh, 06Dh, 07Dh, 08Dh, 09Dh, 0ADh, 0BDh, 0CDh, 0DDh, 0EDh, 0FDh
		db	00Eh, 01Eh, 02Eh, 03Eh, 04Eh, 05Eh, 06Eh, 07Eh, 08Eh, 09Eh, 0AEh, 0BEh, 0CEh, 0DEh, 0EEh, 0FEh
		db	00Fh, 01Fh, 02Fh, 03Fh, 04Fh, 05Fh, 06Fh, 07Fh, 08Fh, 09Fh, 0AFh, 0BFh, 0CFh, 0DFh, 0EFh, 0FFh
T_0302		db	000h, 000h
T_0304		db	000h, 000h, 000h, 000h, 000h
T_0309		db	000h, 000h, 000h
T_030C		db	000h, 000h, 000h, 000h, 000h
T_0311		db	000h
T_0312		db	000h, 000h, 000h, 000h

; 2A68:0316 The direction from block a to block b in eighths (0 north, 1
; north-east...), -1 the same block
		PUBLIC	octant
octant	PROC	FAR
		push	bp
		mov	bp,sp
		push	bx
		push	cx
		push	dx
		push	di
		push	si
		mov	ax,word ptr [bp+6]
		mov	dl,20h
		div	dl
		mov	bx,ax
		mov	ax,word ptr [bp+8]
		div	dl
		xor	cx,cx
		xor	dx,dx
		mov	cl,ah
		mov	dl,al
		xor	ax,ax
		mov	al,bh
		xor	bh,bh
		mov	si,0
		sub	bx,dx
		mov	di,bx
		jns	@@0345
		neg	di
@@0345:
		sub	cx,ax
		mov	ax,cx
		jns	@@034D
		neg	ax
@@034D:
		mov	dx,bx
		shl	dx,1
		cmp	dx,ax
		jl	@@0358
		or	si,8
@@0358:
		neg	dx
		cmp	dx,ax
		jl	@@0361
		or	si,4
@@0361:
		mov	dx,cx
		shl	dx,1
		cmp	dx,di
		jl	@@036C
		or	si,2
@@036C:
		neg	dx
		cmp	dx,di
		jl	@@0373
		inc	si
@@0373:
		xor	ax,ax
		mov	al,byte ptr OctantDir[si]
		pop	si
		pop	di
		pop	dx
		pop	cx
		pop	bx
		pop	bp
		retf
octant	ENDP

; 2A68:0380 The 16 colours of the walls and of the background (copied here)
		PUBLIC	set_vcn_colours
set_vcn_colours	PROC	FAR
		push	bp
		mov	bp,sp
		push	cx
		push	di
		push	si
		push	ds
		push	es
		mov	si,word ptr [bp+6]
		mov	ax,word ptr [bp+8]
		mov	ds,ax
		push	cs
		pop	es
		mov	di,OFFSET T_03B1
		mov	cx,8
		rep	movsw
		mov	si,word ptr [bp+0Ah]
		mov	ax,word ptr [bp+0Ch]
		mov	ds,ax
		mov	di,OFFSET T_03C1
		mov	cx,8
		rep	movsw
		pop	es
		pop	ds
		pop	si
		pop	di
		pop	cx
		pop	bp
		retf
T_03B1		db	000h, 001h, 002h, 003h, 004h, 005h, 006h, 007h, 008h, 009h, 00Ah, 00Bh, 00Ch, 00Dh, 00Eh, 00Fh
T_03C1		db	000h, 001h, 002h, 003h, 004h, 005h, 006h, 007h, 008h, 009h, 00Ah, 00Bh, 00Ch, 00Dh, 00Eh, 00Fh
T_03D1		db	000h, 000h
T_03D3		db	000h, 000h
T_03D5		db	000h, 000h
T_03D7		db	000h, 000h
T_03D9		db	000h, 000h
T_03DB		db	000h, 000h
T_03DD		db	000h, 000h
T_03DF		db	000h, 000h
T_03E1		db	000h
T_03E2		db	000h
T_03E3		db	000h, 000h
T_03E5		db	000h, 000h
T_03E7		db	000h, 000h
T_03E9		db	000h, 000h
T_03EB		db	000h, 000h
set_vcn_colours	ENDP

; 2A68:03ED The 3D view built from ViewTiles and the VCN blocks into the
; work page, in the mode of the video card
		PUBLIC	compose_view
compose_view	PROC	FAR
		push	bp
		mov	bp,sp
		push	bx
		push	cx
		push	dx
		push	di
		push	si
		push	ds
		push	es
		mov	si,word ptr [bp+6]
		mov	dx,word ptr [bp+8]
		mov	word ptr cs:T_03E5,si
		mov	word ptr cs:T_03E7,dx
		mov	bx,OFFSET ViewTiles
		mov	cx,SEG ViewTiles
		mov	bp,cx
		cmp	word ptr VideoMode,5
		jz	@@041F
		mov	di,word ptr WorkPage
		mov	ax,word ptr WorkPage+2
		jmp	short @@0425
@@041F:
		mov	ax,0A400h
		mov	di,0
@@0425:
		mov	es,ax
		mov	word ptr cs:T_03D5,0Fh
		cmp	word ptr VideoMode,5
		jz	@@044C
		cmp	word ptr VideoMode,0
		jz	@@0449
		cmp	word ptr VideoMode,1
		jz	@@0446
		jmp	@@0B14
@@0446:
		jmp	@@0890
@@0449:
		jmp	@@0678
@@044C:
		call	far ptr get_page_ptr
		mov	word ptr cs:T_0302,ax
		mov	word ptr cs:T_0304,dx
		push	cs
		mov	ax,306h
		push	ax
		call	far ptr set_page_ptr
		add	sp,4
		mov	dx,3CEh
		mov	al,5
		cli
		mov	cs:T_030C,al
		out	dx,al
		sti
@@0473:
		mov	cx,16h
@@0476:
		mov	word ptr cs:T_03D1,0
		mov	ds,bp
		mov	ax,word ptr [bx]
		cmp	ax,8000h
		jc	@@0489
		jmp	@@0572
@@0489:
		test	ax,4000h
		jz	@@0498
		mov	word ptr cs:T_03D1,1
		and	ax,3FFFh
@@0498:
		cmp	ax,0
		jnz	@@04B0
		mov	ax,word ptr [bx+294h]
		test	ax,4000h
		jz	@@04B0
		mov	word ptr cs:T_03D1,1
		and	ax,3FFFh
@@04B0:
		add	bx,2
		mov	si,0
		add	ax,0A800h
		mov	ds,ax
		cmp	word ptr cs:T_03D1,1
		jnz	@@050E
		mov	dx,3CFh
		xor	ax,ax
		cli
		mov	cs:T_0312,al
		out	dx,al
		sti
		dec	dx
		mov	al,4
		cli
		mov	cs:T_030C,al
		out	dx,al
		sti
		push	bx
		push	cx
		xor	bx,bx
		mov	word ptr cs:T_03D9,8
@@04E4:
		xor	cl,cl
		call	view_0F28
		inc	cl
		call	view_0F28
		inc	cl
		call	view_0F28
		inc	cl
		call	view_0F28
		add	di,50h
		add	si,2
		dec	word ptr cs:T_03D9
		jnz	@@04E4
		pop	cx
		pop	bx
		sub	di,27Eh
		jmp	@@065D
@@050E:
		mov	dx,3CFh
		mov	al,1
		cli
		mov	cs:T_0312,al
		out	dx,al
		sti
		push	cx
		mov	cx,8
@@051E:
		movsb
		movsb
		add	di,4Eh
		loop	@@051E
		sub	di,27Eh
		pop	cx
@@052A:
		dec	cx
		jz	@@0530
		jmp	@@0476
@@0530:
		add	di,254h
		dec	word ptr cs:T_03D5
		jz	@@053E
		jmp	@@0473
@@053E:
		mov	dx,3CEh
		mov	ax,205h
		cli
		mov	cs:T_030C,al
		out	dx,al
		inc	dx
		mov	al,ah
		mov	cs:T_0312,al
		out	dx,al
		sti
		dec	dx
		mov	al,8
		cli
		mov	cs:T_030C,al
		out	dx,al
		sti
		push	word ptr cs:T_0304
		push	word ptr cs:T_0302
		call	far ptr set_page_ptr
		add	sp,4
		jmp	@@0D51
@@0572:
		sub	ax,8000h
		test	ax,4000h
		jz	@@0584
		and	ax,3FFFh
		mov	word ptr cs:T_03D1,1
@@0584:
		push	ax
		mov	si,0
		add	ax,0A800h
		mov	word ptr cs:T_03DB,si
		mov	word ptr cs:T_03DD,ax
		pop	ax
		add	ax,word ptr cs:T_03E7
		mov	word ptr cs:T_03DF,ax
		mov	word ptr cs:T_03D3,0
		mov	ax,word ptr [bx+294h]
		test	ax,4000h
		jz	@@05B8
		sub	ax,4000h
		mov	word ptr cs:T_03D3,1
@@05B8:
		add	bx,2
		mov	si,0
		add	ax,0A800h
		mov	word ptr cs:T_03E3,ax
		push	cx
		push	bx
		mov	bx,si
		mov	si,word ptr cs:T_03DB
		mov	ds,word ptr cs:T_03DD
		mov	word ptr cs:T_03D9,8
@@05DA:
		push	ds
		mov	ax,word ptr cs:T_03DF
		mov	ds,ax
		mov	ax,word ptr [si]
		not	ax
		pop	ds
		cmp	word ptr cs:T_03D1,1
		jnz	@@0602
		push	bx
		xor	bx,bx
		xchg	al,ah
		mov	bl,ah
		mov	ah,byte ptr cs:T_0102[bx]
		mov	bl,al
		mov	al,byte ptr cs:T_0102[bx]
		pop	bx
@@0602:
		mov	cs:T_03E2,al
		mov	byte ptr cs:T_03E1,ah
		mov	dx,3CEh
		cli
		mov	al,5
		mov	cs:T_030C,al
		out	dx,al
		inc	dx
		xor	al,al
		mov	cs:T_0312,al
		out	dx,al
		sti
		dec	dx
		mov	al,4
		cli
		mov	cs:T_030C,al
		out	dx,al
		sti
		mov	cl,0
		call	view_0F5E
		mov	cl,1
		call	view_0F5E
		mov	cl,2
		call	view_0F5E
		mov	cl,3
		call	view_0F5E
		mov	dx,3CFh
		mov	al,0
		cli
		mov	cs:T_0311,al
		out	dx,al
		sti
		add	di,50h
		inc	si
		inc	si
		inc	bx
		inc	bx
		dec	word ptr cs:T_03D9
		jnz	@@05DA
		sub	di,27Eh
		pop	bx
		pop	cx
@@065D:
		mov	dx,3C5h
		mov	al,0Fh
		cli
		mov	cs:T_0309,al
		out	dx,al
		sti
		mov	dx,3CEh
		mov	al,5
		cli
		mov	cs:T_030C,al
		out	dx,al
		sti
		jmp	@@052A
@@0678:
		mov	cx,16h
@@067B:
		mov	word ptr cs:T_03D1,0
		mov	ds,bp
		mov	ax,word ptr [bx]
		cmp	ah,80h
		jc	@@0691
		sub	ah,80h
		jmp	@@0801
@@0691:
		test	ah,40h
		jz	@@06A0
		mov	word ptr cs:T_03D1,1
		and	ah,3Fh
@@06A0:
		cmp	ax,0
		jnz	@@06B8
		mov	ax,word ptr [bx+294h]
		test	ah,40h
		jz	@@06B8
		mov	word ptr cs:T_03D1,1
		and	ah,3Fh
@@06B8:
		add	bx,2
		push	si
		shl	ax,1
		add	ax,dx
		mov	ds,ax
		cmp	word ptr cs:T_03D1,1
		jnz	@@06CC
		jmp	short @@0710
@@06CC:
		movsb
		inc	si
		movsb
		inc	si
		add	di,1FFEh
		movsb
		inc	si
		movsb
		inc	si
		sub	di,1FB2h
		movsb
		inc	si
		movsb
		inc	si
		add	di,1FFEh
		movsb
		inc	si
		movsb
		inc	si
		sub	di,1FB2h
		movsb
		inc	si
		movsb
		inc	si
		add	di,1FFEh
		movsb
		inc	si
		movsb
		inc	si
		sub	di,1FB2h
		movsb
		inc	si
		movsb
		inc	si
		add	di,1FFEh
		movsb
		inc	si
		movsb
		inc	si
		sub	di,20F0h
		pop	si
		jmp	@@07E5
@@0710:
		push	bx
		xor	bx,bx
		add	si,2
		mov	bl,byte ptr [si]
		mov	al,byte ptr cs:T_0102[bx]
		stosb
		sub	si,2
		mov	bl,byte ptr [si]
		mov	al,byte ptr cs:T_0102[bx]
		stosb
		add	si,6
		add	di,1FFEh
		mov	bl,byte ptr [si]
		mov	al,byte ptr cs:T_0102[bx]
		stosb
		sub	si,2
		mov	bl,byte ptr [si]
		mov	al,byte ptr cs:T_0102[bx]
		stosb
		add	si,6
		sub	di,1FB2h
		mov	bl,byte ptr [si]
		mov	al,byte ptr cs:T_0102[bx]
		stosb
		sub	si,2
		mov	bl,byte ptr [si]
		mov	al,byte ptr cs:T_0102[bx]
		stosb
		add	si,6
		add	di,1FFEh
		mov	bl,byte ptr [si]
		mov	al,byte ptr cs:T_0102[bx]
		stosb
		sub	si,2
		mov	bl,byte ptr [si]
		mov	al,byte ptr cs:T_0102[bx]
		stosb
		add	si,6
		sub	di,1FB2h
		mov	bl,byte ptr [si]
		mov	al,byte ptr cs:T_0102[bx]
		stosb
		sub	si,2
		mov	bl,byte ptr [si]
		mov	al,byte ptr cs:T_0102[bx]
		stosb
		add	si,6
		add	di,1FFEh
		mov	bl,byte ptr [si]
		mov	al,byte ptr cs:T_0102[bx]
		stosb
		sub	si,2
		mov	bl,byte ptr [si]
		mov	al,byte ptr cs:T_0102[bx]
		stosb
		add	si,6
		sub	di,1FB2h
		mov	bl,byte ptr [si]
		mov	al,byte ptr cs:T_0102[bx]
		stosb
		sub	si,2
		mov	bl,byte ptr [si]
		mov	al,byte ptr cs:T_0102[bx]
		stosb
		add	si,6
		add	di,1FFEh
		mov	bl,byte ptr [si]
		mov	al,byte ptr cs:T_0102[bx]
		stosb
		sub	si,2
		mov	bl,byte ptr [si]
		mov	al,byte ptr cs:T_0102[bx]
		stosb
		sub	di,20F0h
		pop	bx
		pop	si
@@07E5:
		dec	cx
		jz	@@07EB
		jmp	@@067B
@@07EB:
		add	di,114h
		mov	ax,SEG ViewTiles
		mov	ds,ax
		dec	word ptr cs:T_03D5
		jz	@@07FE
		jmp	@@0678
@@07FE:
		jmp	@@0D51
@@0801:
		test	ax,4000h
		jz	@@0810
		and	ax,3FFFh
		mov	word ptr cs:T_03D1,1
@@0810:
		shl	ax,1
		add	ax,word ptr cs:T_03E7
		mov	word ptr cs:T_03DD,ax
		mov	si,word ptr cs:T_03E5
		push	si
		mov	word ptr cs:T_03D3,0
		mov	ax,word ptr [bx+294h]
		test	ax,4000h
		jz	@@083B
		and	ax,3FFFh
		mov	word ptr cs:T_03D3,1
@@083B:
		add	bx,2
		push	bx
		mov	bx,word ptr cs:T_03E5
		shl	ax,1
		add	ax,word ptr cs:T_03E7
		mov	word ptr cs:T_03E3,ax
		push	bp
		call	view_0E8D
		add	di,1FFEh
		call	view_0E8D
		sub	di,1FB2h
		call	view_0E8D
		add	di,1FFEh
		call	view_0E8D
		sub	di,1FB2h
		call	view_0E8D
		add	di,1FFEh
		call	view_0E8D
		sub	di,1FB2h
		call	view_0E8D
		add	di,1FFEh
		call	view_0E8D
		sub	di,20F0h
		pop	bp
		pop	bx
		pop	si
		xor	ch,ch
		jmp	@@07E5
@@0890:
		mov	cx,16h
@@0893:
		mov	ds,bp
		mov	ax,word ptr [bx]
		mov	word ptr cs:T_03D1,0
		cmp	ax,8000h
		jc	@@08A9
		sub	ax,8000h
		jmp	@@0A89
@@08A9:
		test	ax,4000h
		jz	@@08B8
		mov	word ptr cs:T_03D1,1
		and	ax,3FFFh
@@08B8:
		cmp	ax,0
		jnz	@@08D0
		mov	ax,word ptr [bx+294h]
		test	ax,4000h
		jz	@@08D0
		mov	word ptr cs:T_03D1,1
		and	ax,3FFFh
@@08D0:
		add	bx,2
		push	si
		shl	ax,1
		add	ax,dx
		mov	ds,ax
		cmp	word ptr cs:T_03D1,1
		jnz	@@08E4
		jmp	short @@0918
@@08E4:
		movsw
		movsw
		add	di,1FFCh
		movsw
		movsw
		add	di,1FFCh
		movsw
		movsw
		add	di,1FFCh
		movsw
		movsw
		sub	di,5F64h
		movsw
		movsw
		add	di,1FFCh
		movsw
		movsw
		add	di,1FFCh
		movsw
		movsw
		add	di,1FFCh
		movsw
		movsw
		sub	di,60A0h
		pop	si
		jmp	@@0A6D
@@0918:
		push	bx
		xor	bx,bx
		mov	bl,byte ptr [si+3]
		mov	al,byte ptr cs:T_0202[bx]
		stosb
		mov	bl,byte ptr [si+2]
		mov	al,byte ptr cs:T_0202[bx]
		stosb
		mov	bl,byte ptr [si+1]
		mov	al,byte ptr cs:T_0202[bx]
		stosb
		mov	bl,byte ptr [si]
		mov	al,byte ptr cs:T_0202[bx]
		stosb
		add	si,4
		add	di,1FFCh
		mov	bl,byte ptr [si+3]
		mov	al,byte ptr cs:T_0202[bx]
		stosb
		mov	bl,byte ptr [si+2]
		mov	al,byte ptr cs:T_0202[bx]
		stosb
		mov	bl,byte ptr [si+1]
		mov	al,byte ptr cs:T_0202[bx]
		stosb
		mov	bl,byte ptr [si]
		mov	al,byte ptr cs:T_0202[bx]
		stosb
		add	si,4
		add	di,1FFCh
		mov	bl,byte ptr [si+3]
		mov	al,byte ptr cs:T_0202[bx]
		stosb
		mov	bl,byte ptr [si+2]
		mov	al,byte ptr cs:T_0202[bx]
		stosb
		mov	bl,byte ptr [si+1]
		mov	al,byte ptr cs:T_0202[bx]
		stosb
		mov	bl,byte ptr [si]
		mov	al,byte ptr cs:T_0202[bx]
		stosb
		add	si,4
		add	di,1FFCh
		mov	bl,byte ptr [si+3]
		mov	al,byte ptr cs:T_0202[bx]
		stosb
		mov	bl,byte ptr [si+2]
		mov	al,byte ptr cs:T_0202[bx]
		stosb
		mov	bl,byte ptr [si+1]
		mov	al,byte ptr cs:T_0202[bx]
		stosb
		mov	bl,byte ptr [si]
		mov	al,byte ptr cs:T_0202[bx]
		stosb
		add	si,4
		sub	di,5F64h
		mov	bl,byte ptr [si+3]
		mov	al,byte ptr cs:T_0202[bx]
		stosb
		mov	bl,byte ptr [si+2]
		mov	al,byte ptr cs:T_0202[bx]
		stosb
		mov	bl,byte ptr [si+1]
		mov	al,byte ptr cs:T_0202[bx]
		stosb
		mov	bl,byte ptr [si]
		mov	al,byte ptr cs:T_0202[bx]
		stosb
		add	si,4
		add	di,1FFCh
		mov	bl,byte ptr [si+3]
		mov	al,byte ptr cs:T_0202[bx]
		stosb
		mov	bl,byte ptr [si+2]
		mov	al,byte ptr cs:T_0202[bx]
		stosb
		mov	bl,byte ptr [si+1]
		mov	al,byte ptr cs:T_0202[bx]
		stosb
		mov	bl,byte ptr [si]
		mov	al,byte ptr cs:T_0202[bx]
		stosb
		add	si,4
		add	di,1FFCh
		mov	bl,byte ptr [si+3]
		mov	al,byte ptr cs:T_0202[bx]
		stosb
		mov	bl,byte ptr [si+2]
		mov	al,byte ptr cs:T_0202[bx]
		stosb
		mov	bl,byte ptr [si+1]
		mov	al,byte ptr cs:T_0202[bx]
		stosb
		mov	bl,byte ptr [si]
		mov	al,byte ptr cs:T_0202[bx]
		stosb
		add	si,4
		add	di,1FFCh
		mov	bl,byte ptr [si+3]
		mov	al,byte ptr cs:T_0202[bx]
		stosb
		mov	bl,byte ptr [si+2]
		mov	al,byte ptr cs:T_0202[bx]
		stosb
		mov	bl,byte ptr [si+1]
		mov	al,byte ptr cs:T_0202[bx]
		stosb
		mov	bl,byte ptr [si]
		mov	al,byte ptr cs:T_0202[bx]
		stosb
		add	si,4
		sub	di,60A0h
		pop	bx
		pop	si
@@0A6D:
		dec	cx
		jz	@@0A73
		jmp	@@0893
@@0A73:
		add	di,0E8h
		mov	ax,SEG ViewTiles
		mov	ds,ax
		dec	word ptr cs:T_03D5
		jz	@@0A86
		jmp	@@0890
@@0A86:
		jmp	@@0D51
@@0A89:
		test	ax,4000h
		jz	@@0A98
		and	ax,3FFFh
		mov	word ptr cs:T_03D1,1
@@0A98:
		shl	ax,1
		add	ax,word ptr cs:T_03E7
		mov	word ptr cs:T_03DD,ax
		mov	si,word ptr cs:T_03E5
		push	si
		mov	word ptr cs:T_03D3,0
		mov	ax,word ptr [bx+294h]
		test	ax,4000h
		jz	@@0AC3
		and	ax,3FFFh
		mov	word ptr cs:T_03D3,1
@@0AC3:
		add	bx,2
		push	bx
		mov	bx,word ptr cs:T_03E5
		shl	ax,1
		add	ax,word ptr cs:T_03E7
		mov	word ptr cs:T_03E3,ax
		call	view_0D5A
		add	di,1FFCh
		call	view_0D5A
		add	di,1FFCh
		call	view_0D5A
		add	di,1FFCh
		call	view_0D5A
		sub	di,5F64h
		call	view_0D5A
		add	di,1FFCh
		call	view_0D5A
		add	di,1FFCh
		call	view_0D5A
		add	di,1FFCh
		call	view_0D5A
		sub	di,60A0h
		pop	bx
		pop	si
		jmp	@@0A6D
@@0B14:
		mov	word ptr cs:T_03D7,16h
@@0B1B:
		mov	word ptr cs:T_03E9,0
		mov	ds,bp
		mov	ax,word ptr [bx]
		mov	word ptr cs:T_03D1,0
		mov	word ptr cs:T_03EB,0
		cmp	ax,8000h
		jc	@@0B43
		sub	ax,8000h
		mov	word ptr cs:T_03EB,ax
		mov	ax,0
@@0B43:
		test	ax,4000h
		jz	@@0B52
		mov	word ptr cs:T_03D1,1
		and	ax,3FFFh
@@0B52:
		cmp	ax,0
		jnz	@@0B71
		mov	word ptr cs:T_03E9,1
		mov	ax,word ptr [bx+294h]
		test	ax,4000h
		jz	@@0B71
		mov	word ptr cs:T_03D1,1
		and	ax,3FFFh
@@0B71:
		add	bx,2
		shl	ax,1
		add	ax,word ptr cs:T_03E7
		mov	ds,ax
		mov	si,word ptr cs:T_03E5
		cmp	word ptr cs:T_03D1,1
		jz	@@0BE4
		push	bx
		xor	bx,bx
		mov	dx,8
@@0B90:
		mov	cx,4
@@0B93:
		mov	bl,byte ptr [si]
		inc	si
		mov	al,bl
		and	al,0F0h
		shr	al,1
		shr	al,1
		shr	al,1
		shr	al,1
		mov	ah,bl
		and	ah,0Fh
		mov	bl,ah
		cmp	word ptr cs:T_03E9,1
		jnz	@@0BB8
		mov	ah,byte ptr cs:T_03B1[bx]
		jmp	short @@0BBD
@@0BB8:
		mov	ah,byte ptr cs:T_03C1[bx]
@@0BBD:
		mov	bl,al
		cmp	word ptr cs:T_03E9,1
		jnz	@@0BCE
		mov	al,byte ptr cs:T_03B1[bx]
		jmp	short @@0BD3
@@0BCE:
		mov	al,byte ptr cs:T_03C1[bx]
@@0BD3:
		stosw
		loop	@@0B93
		add	di,138h
		dec	dx
		jnz	@@0B90
		sub	di,9F8h
		pop	bx
		jmp	short @@0C44
@@0BE4:
		push	bx
		xor	bx,bx
		mov	dx,8
@@0BEA:
		mov	cx,4
		add	si,3
@@0BF0:
		mov	bl,byte ptr [si]
		dec	si
		mov	ah,bl
		and	ah,0F0h
		shr	ah,1
		shr	ah,1
		shr	ah,1
		shr	ah,1
		mov	al,bl
		and	al,0Fh
		mov	bl,ah
		cmp	word ptr cs:T_03E9,1
		jnz	@@0C15
		mov	ah,byte ptr cs:T_03B1[bx]
		jmp	short @@0C1A
@@0C15:
		mov	ah,byte ptr cs:T_03C1[bx]
@@0C1A:
		mov	bl,al
		cmp	word ptr cs:T_03E9,1
		jnz	@@0C2B
		mov	al,byte ptr cs:T_03B1[bx]
		jmp	short @@0C30
@@0C2B:
		mov	al,byte ptr cs:T_03C1[bx]
@@0C30:
		stosw
		loop	@@0BF0
		add	si,5
		add	di,138h
		dec	dx
		jnz	@@0BEA
		sub	di,9F8h
		pop	bx
		jmp	short @@0C44
@@0C44:
		cmp	word ptr cs:T_03EB,0
		jnz	@@0C6C
		dec	word ptr cs:T_03D7
		jz	@@0C56
		jmp	@@0B1B
@@0C56:
		add	di,950h
		mov	ax,SEG ViewTiles
		mov	ds,ax
		dec	word ptr cs:T_03D5
		jz	@@0C69
		jmp	@@0B14
@@0C69:
		jmp	@@0D51
@@0C6C:
		sub	di,8
		mov	word ptr cs:T_03D1,0
		mov	ax,word ptr cs:T_03EB
		test	ax,4000h
		jz	@@0C89
		and	ax,3FFFh
		mov	word ptr cs:T_03D1,1
@@0C89:
		shl	ax,1
		add	ax,word ptr cs:T_03E7
		mov	ds,ax
		mov	si,word ptr cs:T_03E5
		cmp	word ptr cs:T_03D1,1
		jz	@@0CF5
		push	bx
		xor	bx,bx
		mov	dx,8
@@0CA5:
		mov	cx,4
@@0CA8:
		mov	bl,byte ptr [si]
		inc	si
		mov	ah,bl
		and	ah,0F0h
		shr	ah,1
		shr	ah,1
		shr	ah,1
		shr	ah,1
		mov	al,bl
		and	al,0Fh
		mov	bl,ah
		mov	ah,byte ptr cs:T_03C1[bx]
		mov	bl,al
		mov	al,byte ptr cs:T_03C1[bx]
		cmp	ah,0
		jz	@@0CD2
		mov	byte ptr es:[di],ah
@@0CD2:
		inc	di
		cmp	al,0
		jz	@@0CDA
		mov	byte ptr es:[di],al
@@0CDA:
		inc	di
		xor	bx,bx
		loop	@@0CA8
		add	di,138h
		dec	dx
		jnz	@@0CA5
		sub	di,9F8h
		pop	bx
		mov	word ptr cs:T_03EB,0
		jmp	@@0C44
@@0CF5:
		push	bx
		xor	bx,bx
		mov	dx,8
@@0CFB:
		mov	cx,4
		add	si,3
@@0D01:
		mov	bl,byte ptr [si]
		dec	si
		mov	al,bl
		and	al,0F0h
		shr	al,1
		shr	al,1
		shr	al,1
		shr	al,1
		mov	ah,bl
		and	ah,0Fh
		mov	bl,ah
		mov	ah,byte ptr cs:T_03C1[bx]
		mov	bl,al
		mov	al,byte ptr cs:T_03C1[bx]
		cmp	ah,0
		jz	@@0D2B
		mov	byte ptr es:[di],ah
@@0D2B:
		inc	di
		cmp	al,0
		jz	@@0D33
		mov	byte ptr es:[di],al
@@0D33:
		inc	di
		xor	bx,bx
		loop	@@0D01
		add	si,5
		add	di,138h
		dec	dx
		jnz	@@0CFB
		sub	di,9F8h
		pop	bx
		mov	word ptr cs:T_03EB,0
		jmp	@@0C44
@@0D51:
		pop	es
		pop	ds
		pop	si
		pop	di
		pop	dx
		pop	cx
		pop	bx
		pop	bp
		retf
compose_view	ENDP

; 2A68:0D5A
view_0D5A	PROC	NEAR
@@0D5A:
		push	dx
		mov	dx,bx
		mov	ds,word ptr cs:T_03E3
		cmp	word ptr cs:T_03D3,1
		jnz	@@0D78
		mov	ah,byte ptr [bx+3]
		xor	bx,bx
		mov	bl,ah
		mov	ah,byte ptr cs:T_0202[bx]
		jmp	short @@0D7B
@@0D78:
		mov	ah,byte ptr [bx]
		inc	dx
@@0D7B:
		mov	ds,word ptr cs:T_03DD
		cmp	word ptr cs:T_03D1,1
		jnz	@@0D94
		xor	bx,bx
		mov	bl,byte ptr [si+3]
		mov	al,byte ptr cs:T_0202[bx]
		jmp	short @@0D95
@@0D94:
		lodsb
@@0D95:
		test	al,0F0h
		jz	@@0D9C
		and	ah,0Fh
@@0D9C:
		test	al,0Fh
		jz	@@0DA3
		and	ah,0F0h
@@0DA3:
		or	al,ah
		stosb
		mov	bx,dx
		mov	ds,word ptr cs:T_03E3
		cmp	word ptr cs:T_03D3,1
		jnz	@@0DC3
		mov	ah,byte ptr [bx+2]
		xor	bx,bx
		mov	bl,ah
		mov	ah,byte ptr cs:T_0202[bx]
		jmp	short @@0DC6
@@0DC3:
		mov	ah,byte ptr [bx]
		inc	dx
@@0DC6:
		mov	ds,word ptr cs:T_03DD
		cmp	word ptr cs:T_03D1,1
		jnz	@@0DDF
		xor	bx,bx
		mov	bl,byte ptr [si+2]
		mov	al,byte ptr cs:T_0202[bx]
		jmp	short @@0DE0
@@0DDF:
		lodsb
@@0DE0:
		test	al,0F0h
		jz	@@0DE7
		and	ah,0Fh
@@0DE7:
		test	al,0Fh
		jz	@@0DEE
		and	ah,0F0h
@@0DEE:
		or	al,ah
		stosb
		mov	bx,dx
		mov	ds,word ptr cs:T_03E3
		cmp	word ptr cs:T_03D3,1
		jnz	@@0E0E
		mov	ah,byte ptr [bx+1]
		xor	bx,bx
		mov	bl,ah
		mov	ah,byte ptr cs:T_0202[bx]
		jmp	short @@0E11
@@0E0E:
		mov	ah,byte ptr [bx]
		inc	dx
@@0E11:
		mov	ds,word ptr cs:T_03DD
		cmp	word ptr cs:T_03D1,1
		jnz	@@0E2A
		xor	bx,bx
		mov	bl,byte ptr [si+1]
		mov	al,byte ptr cs:T_0202[bx]
		jmp	short @@0E2B
@@0E2A:
		lodsb
@@0E2B:
		test	al,0F0h
		jz	@@0E32
		and	ah,0Fh
@@0E32:
		test	al,0Fh
		jz	@@0E39
		and	ah,0F0h
@@0E39:
		or	al,ah
		stosb
		mov	bx,dx
		mov	ds,word ptr cs:T_03E3
		cmp	word ptr cs:T_03D3,1
		jnz	@@0E5B
		mov	ah,byte ptr [bx]
		add	dx,4
		xor	bx,bx
		mov	bl,ah
		mov	ah,byte ptr cs:T_0202[bx]
		jmp	short @@0E5E
@@0E5B:
		mov	ah,byte ptr [bx]
		inc	dx
@@0E5E:
		mov	ds,word ptr cs:T_03DD
		cmp	word ptr cs:T_03D1,1
		jnz	@@0E77
		mov	bl,byte ptr [si]
		add	si,4
		mov	al,byte ptr cs:T_0202[bx]
		jmp	short @@0E78
@@0E77:
		lodsb
@@0E78:
		test	al,0F0h
		jz	@@0E7F
		and	ah,0Fh
@@0E7F:
		test	al,0Fh
		jz	@@0E86
		and	ah,0F0h
@@0E86:
		or	al,ah
		stosb
		mov	bx,dx
		pop	dx
		ret
view_0D5A	ENDP

; 2A68:0E8D
view_0E8D	PROC	NEAR
@@0E8D:
		mov	ds,word ptr cs:T_03DD
		cmp	word ptr cs:T_03D1,1
		jnz	@@0EB5
		add	si,2
		lodsw
		sub	si,4
		push	bx
		xor	bx,bx
		mov	bl,ah
		mov	ah,byte ptr cs:T_0102[bx]
		mov	bl,al
		mov	al,byte ptr cs:T_0102[bx]
		pop	bx
		jmp	short @@0EB6
@@0EB5:
		lodsw
@@0EB6:
		mov	ds,word ptr cs:T_03E3
		cmp	word ptr cs:T_03D3,1
		jnz	@@0ED3
		mov	ch,byte ptr [bx+2]
		push	bx
		xor	bx,bx
		mov	bl,ch
		mov	ch,byte ptr cs:T_0102[bx]
		pop	bx
		jmp	short @@0ED8
@@0ED3:
		mov	ch,byte ptr [bx]
		add	bx,2
@@0ED8:
		and	ah,ch
		or	al,ah
		stosb
		cmp	word ptr cs:T_03D3,1
		jnz	@@0EF7
		mov	ch,byte ptr [bx]
		add	bx,4
		push	bx
		xor	bx,bx
		mov	bl,ch
		mov	ch,byte ptr cs:T_0102[bx]
		pop	bx
		jmp	short @@0EFC
@@0EF7:
		mov	ch,byte ptr [bx]
		add	bx,2
@@0EFC:
		mov	ds,word ptr cs:T_03DD
		cmp	word ptr cs:T_03D1,1
		jnz	@@0F21
		lodsw
		add	si,2
		push	bx
		xor	bx,bx
		mov	bl,ah
		mov	ah,byte ptr cs:T_0102[bx]
		mov	bl,al
		mov	al,byte ptr cs:T_0102[bx]
		pop	bx
		jmp	short @@0F22
@@0F21:
		lodsw
@@0F22:
		and	ah,ch
		or	al,ah
		stosb
		ret
view_0E8D	ENDP

; 2A68:0F28
view_0F28	PROC	NEAR
@@0F28:
		mov	dx,3CFh
		mov	al,cl
		cli
		mov	cs:T_0311,al
		out	dx,al
		sti
		mov	dx,3C5h
		mov	al,1
		shl	al,cl
		cli
		mov	cs:T_0309,al
		out	dx,al
		sti
		inc	di
		lodsb
		mov	bl,al
		mov	al,byte ptr cs:T_0102[bx]
		xchg	byte ptr es:[di],al
		dec	di
		lodsb
		mov	bl,al
		mov	al,byte ptr cs:T_0102[bx]
		xchg	byte ptr es:[di],al
		sub	si,2
		ret
view_0F28	ENDP

; 2A68:0F5E
view_0F5E	PROC	NEAR
@@0F5E:
		mov	dx,3CFh
		mov	al,cl
		cli
		mov	cs:T_0311,al
		out	dx,al
		sti
		mov	dl,0C5h
		mov	al,1
		shl	al,cl
		cli
		mov	cs:T_0309,al
		out	dx,al
		sti
		mov	dx,bx
		mov	ds,word ptr cs:T_03E3
		mov	al,byte ptr [bx]
		mov	ah,byte ptr [bx+1]
		cmp	word ptr cs:T_03D3,1
		xor	bx,bx
		jnz	@@0F9D
		xchg	al,ah
		mov	bl,al
		mov	al,byte ptr cs:T_0102[bx]
		mov	bl,ah
		mov	ah,byte ptr cs:T_0102[bx]
@@0F9D:
		and	al,byte ptr cs:T_03E2
		and	ah,byte ptr cs:T_03E1
		mov	ds,word ptr cs:T_03DD
		mov	cl,byte ptr [si]
		mov	ch,byte ptr [si+1]
		cmp	word ptr cs:T_03D1,1
		jnz	@@0FC9
		xchg	cl,ch
		mov	bl,cl
		mov	cl,byte ptr cs:T_0102[bx]
		mov	bl,ch
		mov	ch,byte ptr cs:T_0102[bx]
@@0FC9:
		or	al,cl
		or	ah,ch
		xchg	byte ptr es:[di],al
		inc	di
		xchg	byte ptr es:[di],ah
		dec	di
		mov	bx,dx
		ret
view_0F5E	ENDP

; 2A68:0FD8 The distance in blocks: the longest axis + half the shortest
		PUBLIC	distance
distance	PROC	FAR
		push	bp
		mov	bp,sp
		push	bx
		push	cx
		push	dx
		mov	ax,word ptr [bp+6]
		mov	bx,word ptr [bp+8]
		mov	cl,3
		mov	dl,al
		and	dl,1Fh
		shl	ax,cl
		mov	dh,ah
		mov	al,bl
		and	al,1Fh
		shl	bx,cl
		mov	ah,bh
		sub	ah,dh
		jns	@@0FFD
		neg	ah
@@0FFD:
		sub	al,dl
		jns	@@1003
		neg	al
@@1003:
		cmp	al,ah
		jl	@@1009
		xchg	al,ah
@@1009:
		shr	al,1
		add	al,ah
		xor	ah,ah
		pop	dx
		pop	cx
		pop	bx
		pop	bp
		retf
distance	ENDP

; 2A68:1014
		PUBLIC	view_1014
view_1014	PROC	FAR
		push	bp
		mov	bp,sp
		push	bx
		push	cx
		push	dx
		push	di
		push	si
		push	ds
		push	es
		mov	si,word ptr [bp+6]
		mov	di,word ptr [bp+8]
		mov	dx,word ptr [bp+0Ah]
		mov	cx,8
		mov	bx,0
@@102D:
		mov	ax,si
		add	ax,word ptr Neighbours[bx]
		and	ax,3FFh
		cmp	ax,di
		jz	@@105D
		add	bx,2
		loop	@@102D
		mov	cx,2
		mov	bx,dx
		shl	bx,1
		shl	bx,1
@@1048:
		mov	ax,si
		add	ax,word ptr FarAhead[bx]
		and	ax,3FFh
		cmp	ax,di
		jz	@@105D
		add	bx,2
		loop	@@1048
		mov	ax,0FFFFh
@@105D:
		pop	es
		pop	ds
		pop	si
		pop	di
		pop	dx
		pop	cx
		pop	bx
		pop	bp
		retf
T_1066		db	000h, 000h
T_1068		db	000h, 000h
view_1014	ENDP

; 2A68:106A
		PUBLIC	view_106A
view_106A	PROC	FAR
		push	bp
		mov	bp,sp
		push	bx
		push	cx
		push	dx
		push	di
		push	si
		push	ds
		push	es
		mov	si,word ptr [bp+6]
		mov	es,word ptr [bp+8]
		mov	cx,6590h
		xor	dx,dx
		mov	bx,word ptr Var_65F6
		add	bx,190h
		mov	word ptr cs:T_1066,bx
		inc	bh
		mov	word ptr cs:T_1068,bx
		mov	ds,word ptr Var_65F8
@@1097:
		mov	bx,word ptr cs:T_1066
@@109C:
		mov	dl,2
@@109E:
		mov	al,byte ptr es:[si]
		xlatb
		shl	al,1
		shl	al,1
		shl	al,1
		shl	al,1
		mov	ah,al
		mov	al,byte ptr es:[si+1]
		xlatb
		or	al,ah
		xchg	byte ptr es:[si],al
		inc	si
		xor	ah,ah
		test	al,0F0h
		jz	@@10C0
		or	ah,0C0h
@@10C0:
		test	al,0Fh
		jz	@@10C7
		or	ah,30h
@@10C7:
		mov	al,byte ptr es:[si]
		test	al,0F0h
		jz	@@10D1
		or	ah,0Ch
@@10D1:
		test	al,0Fh
		jz	@@10D8
		or	ah,3
@@10D8:
		not	ah
		mov	byte ptr es:[si],ah
		inc	si
		dec	dl
		jz	@@10E6
		loop	@@109E
		jmp	short @@10F5
@@10E6:
		dec	cx
		jz	@@10F5
		xor	dh,1
		jz	@@1097
		mov	bx,word ptr cs:T_1068
		jmp	short @@109C
@@10F5:
		pop	es
		pop	ds
		pop	si
		pop	di
		pop	dx
		pop	cx
		pop	bx
		pop	bp
		retf
view_106A	ENDP

; 2A68:10FE shape recoloured through table
		PUBLIC	remap_shape
remap_shape	PROC	FAR
		push	bp
		mov	bp,sp
		push	ax
		push	bx
		push	cx
		push	es
		push	di
		push	ds
		mov	di,word ptr [bp+6]
		mov	es,word ptr [bp+8]
		mov	bx,word ptr [bp+0Ah]
		mov	ds,word ptr [bp+0Ch]
		mov	al,byte ptr es:[di]
		cmp	al,1
		jnz	@@1127
		add	di,4
		mov	cx,10h
@@1120:
		mov	al,byte ptr es:[di]
		xlatb
		stosb
		loop	@@1120
@@1127:
		pop	ds
		pop	di
		pop	es
		pop	cx
		pop	bx
		pop	ax
		pop	bp
		retf
remap_shape	ENDP


		END
