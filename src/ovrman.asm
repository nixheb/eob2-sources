;
; Eye of the Beholder II - START.EXE
; Borland's overlay manager, its kernel (OVRMAN of OVERLAY.LIB): the
; revision linked in START.EXE (Ghidra 2B7A, from 2B7A:000F;
; the segment starts at 2B7A:000F), found in no library we have: TC++ 1.01, BC++ 2.0, 3.0 and 3.1
; differ (2.0 reads the open mode of the file in a variable, rounds a
; size...). Written back from the EXE by tools/ovrmandata.py: the code from
; the listing, the symbols from the fixups of BC++ 2.0's OVRMAN.OBJ; the
; segments and groups are the ones of that module. The other modules of
; OVERLAY.LIB 2.0 are the ones of START.EXE, byte for byte.
;
; Assembled with
;     tasm /ml ovrman.asm
; [asm] = identical to START.EXE byte for byte (tools/asm.py).
;
; Borland assembled it with TASM in one pass: a forward jmp keeps the NOP
; of its third byte.
		LOCALS

_TEXT		SEGMENT	BYTE PUBLIC 'CODE'
_TEXT		ENDS
_DATA		SEGMENT	WORD PUBLIC 'DATA'
		EXTRN	__psp:WORD, __OvrSize:WORD, __envseg:WORD
_DATA		ENDS
_OVRTEXT_	SEGMENT	BYTE PUBLIC 'CODE'
_OVRTEXT_	ENDS
_OVRDATA_	SEGMENT	PARA PUBLIC 'OVRINFO'
		EXTRN	__CALLEXTEMSSWAP:WORD, __CALLEXTEXIT:WORD, __CALLEMSEXIT:WORD
		EXTRN	__OVRHOOK__:WORD, __OVRFILENAME:BYTE
OvrSave		dw	0
OvrVector	dd	ovr_04F5		; the overlay interrupt (INT 3Fh) swapped with the old one
OvrOpenMode	db	0
_OVRDATA_	ENDS
_EXEINFO_	SEGMENT	PARA PUBLIC 'OVRINFO'
		EXTRN	__EXENAME__:BYTE, __EXEDATE__:WORD
_EXEINFO_	ENDS
_STUB_		SEGMENT	PARA PUBLIC 'OVRINFO'
		EXTRN	__OVRTRAP__:WORD, __OvrFileBase:WORD, __OvrRetrySize:WORD, __OvrMinHeapSize:WORD
		EXTRN	__OvrTrapCount:WORD, __OvrHeapPtr:WORD, __OvrCodeList:WORD, __OvrHeapOrg:WORD
		EXTRN	__OvrHeapEnd:WORD, __OvrDosHandle:WORD, __OvrLoadCount:WORD, __OvrLoadList:WORD
		EXTRN	__OvrMemorySwap:WORD, __SEGTABLE__:WORD, __SEGTABEND__:WORD
_STUB_		ENDS
_EXTSEG_	SEGMENT	PARA PUBLIC 'OVRINFO'
_EXTSEG_	ENDS
_EMSSEG_	SEGMENT	PARA PUBLIC 'OVRINFO'
_EMSSEG_	ENDS
_VDISKSEG_	SEGMENT	PARA PUBLIC 'OVRINFO'
_VDISKSEG_	ENDS
DGROUP		GROUP	_DATA
_OVRGROUP_	GROUP	_VDISKSEG_, _EMSSEG_, _EXTSEG_, _STUB_, _OVRDATA_, _EXEINFO_
		EXTRN	__OverlayHalt:FAR

		PUBLIC	__OVRGROUP, __DGROUP

_OVRTEXT_	SEGMENT	BYTE PUBLIC 'CODE'
		ASSUME	CS:_OVRTEXT_, DS:_OVRGROUP_
__OVRGROUP	dw	_OVRGROUP_		; 2B7A:000F
__DGROUP	dw	DGROUP
T_0013		db	"PATH="
T_0018		db	000h, 000h

; 2B7A:001A
		PUBLIC	__OVRINIT
__OVRINIT	PROC	FAR
		push	bp
		mov	bp,sp
		sub	sp,14h
		push	ds
		mov	ds,word ptr cs:__OVRGROUP
		push	si
		push	di
		cld
		cmp	word ptr [__OVRTRAP__],0
		jnz	L_0033
		jmp	L_013F
L_0033:
		call	ovr_01C9
		jnc	L_0064
		mov	ax,word ptr [bp+6]
		or	ax,word ptr [bp+8]
		jnz	L_004A
		mov	word ptr [bp+6],OFFSET __EXENAME__
		mov	word ptr [bp+8],SEG __EXENAME__
L_004A:
		call	ovr_01BD
		jnc	L_0064
		call	ovr_0209
		jnc	L_0064
		mov	ax,0FFFEh
		jmp	L_0141
L_005A:
		mov	ah,3Eh
		int	21h
		mov	ax,0FFFFh
		jmp	L_0141
L_0064:
		mov	bx,ax
		mov	word ptr [__OvrDosHandle],bx
		mov	cx,14h
		call	ovr_028C
		mov	cx,0FFFDh
		jc	L_005A
		xor	ax,ax
		xor	dx,dx
		cmp	word ptr [bp-14h],5A4Dh
		jz	L_0085
		mov	cx,0FFFCh
		jmp	short L_005A
L_0085:
		mov	ax,word ptr [bp-10h]
		mov	cx,word ptr [bp-12h]
		jcxz	L_008E
		dec	ax
L_008E:
		mov	dx,200h
		mul	dx
		add	ax,cx
		add	ax,0Fh
		adc	dx,0
		and	ax,0FFF0h
L_009E:
		push	dx
		push	ax
		mov	cx,dx
		mov	dx,ax
		mov	ax,4200h
		int	21h
		mov	cx,10h
		call	ovr_028C
		pop	ax
		pop	dx
		mov	cx,0FFFDh
		jc	L_005A
		add	ax,10h
		adc	dx,0
		cmp	word ptr [bp-14h],4246h
		mov	cx,0FFFCh
		jz	L_00C8
		jmp	short L_005A
L_00C8:
		cmp	word ptr [bp-12h],564Fh
		jz	L_00D7
		add	ax,word ptr [bp-10h]
		adc	dx,word ptr [bp-0Eh]
		jmp	short L_009E
L_00D7:
		mov	[__OvrFileBase],ax
		mov	word ptr [__OvrFileBase+2],dx
		mov	es,word ptr cs:__DGROUP
		mov	ax,word ptr [bp-10h]
		ASSUME	DS:DGROUP
		mov	es:[__OvrSize],ax
		ASSUME	DS:_OVRGROUP_
		mov	ax,word ptr [bp-0Eh]
		ASSUME	DS:DGROUP
		mov	es:[__OvrSize+2],ax
		ASSUME	DS:_OVRGROUP_
		mov	ah,3Eh
		int	21h
		mov	word ptr [__OvrDosHandle],0
		NOSMART		; a real far call (TASM would push cs; call near)
		call	far ptr ovr_toggle_file
		SMART
		mov	ax,word ptr [bp+0Ch]
		inc	ax
		mov	[__OvrHeapOrg],ax
		mov	[__OvrHeapPtr],ax
		mov	bx,word ptr [bp+0Ah]
		mov	word ptr [__OvrHeapEnd],bx
		call	__InitModules
		mov	bx,word ptr [__OvrHeapEnd]
		sub	bx,word ptr [__OvrHeapOrg]
		cmp	bx,word ptr [__OvrMinHeapSize]
		jnc	L_0128
		mov	cx,0FFFBh
		jmp	L_005A
L_0128:
		shr	bx,1
		shr	bx,1
		mov	word ptr [__OvrRetrySize],bx
		call	ovr_031C
		jnc	L_013F
		mov	bx,word ptr [__OvrDosHandle]
		mov	cx,0FFFFh
		jmp	short L_0141
		db	090h
L_013F:
		xor	ax,ax
L_0141:
		pop	di
		pop	si
		pop	ds
		mov	sp,bp
		pop	bp
		retf	8
__OVRINIT	ENDP

; 2B7A:014A
ovr_toggle_file	PROC	FAR
		push	bp
		mov	bp,sp
		push	ds
		mov	ds,word ptr cs:__OVRGROUP
		mov	al,byte ptr [__OVRTRAP__+1]
		mov	ah,35h
		int	21h
		push	es
		push	bx
		push	ds
		mov	al,byte ptr [__OVRTRAP__+1]
		mov	dx,word ptr [OvrVector]
		mov	ds,word ptr [OvrVector+2]
		mov	ah,25h
		int	21h
		pop	ds
		pop	word ptr [OvrVector]
		pop	word ptr [OvrVector+2]
		cmp	word ptr [__OvrDosHandle],0
		jz	L_018C
		mov	bx,word ptr [__OvrDosHandle]
		mov	ah,3Eh
		int	21h
		mov	word ptr [__OvrDosHandle],0
		jmp	short L_0198
L_018C:
		NOSMART		; lea kept (TASM would write mov reg,OFFSET)
		lea	dx,[__OVRFILENAME]
		SMART
		mov	ax,3D00h
		int	21h
		mov	[__OvrDosHandle],ax
L_0198:
		pop	ds
		pop	bp
		retf
ovr_toggle_file	ENDP

; 2B7A:019B
		PUBLIC	__OVREXIT
__OVREXIT	PROC	FAR
		push	bp
		mov	bp,sp
		push	ds
		mov	ds,word ptr cs:__OVRGROUP
		cmp	word ptr [__OvrDosHandle],0
		jz	L_01B0
		NOSMART		; a real far call (TASM would push cs; call near)
		call	far ptr ovr_toggle_file
		SMART
L_01B0:
		push	cs
		call	word ptr [__CALLEMSEXIT]
		push	cs
		call	word ptr [__CALLEXTEXIT]
		pop	ds
		pop	bp
		retf
__OVREXIT	ENDP

; 2B7A:01BD
ovr_01BD	PROC	NEAR
L_01BD:
		push	ds
		NOSMART		; lea kept (TASM would write mov reg,OFFSET)
		lea	di,[__OVRFILENAME]
		SMART
		push	ds
		pop	es
		call	ovr_0266
		pop	ds
		retn
ovr_01BD	ENDP

; 2B7A:01C9
ovr_01C9	PROC	NEAR
L_01C9:
		mov	ah,30h
		int	21h
		cmp	al,3
		jc	L_0208
		push	ds
		mov	ax,DGROUP
		mov	ds,ax
		ASSUME	DS:DGROUP
		mov	ds,word ptr [__envseg]
		ASSUME	DS:_OVRGROUP_
		xor	si,si
		cld
L_01DE:
		lodsb
		or	al,al
		jnz	L_01DE
		lodsb
		or	al,al
		jnz	L_01DE
		lodsw
		NOSMART		; lea kept (TASM would write mov reg,OFFSET)
		lea	di,[__OVRFILENAME]
		SMART
		mov	ax,_OVRGROUP_
		mov	es,ax
		mov	bx,di
L_01F4:
		lodsb
		stosb
		or	al,al
		jz	L_0202
		cmp	al,5Ch
		jnz	L_01F4
		mov	bx,di
		jmp	short L_01F4
L_0202:
		mov	di,bx
		call	ovr_0266
		pop	ds
L_0208:
		retn
ovr_01C9	ENDP

; 2B7A:0209
ovr_0209	PROC	NEAR
L_0209:
		push	ds
		mov	ax,DGROUP
		mov	ds,ax
		ASSUME	DS:DGROUP
		mov	ds,word ptr [__envseg]
		ASSUME	DS:_OVRGROUP_
		xor	si,si
L_0215:
		mov	di,OFFSET T_0013
		push	cs
		pop	es
		mov	cx,5
		cld
		repe	cmpsb
		jz	L_022F
		dec	si
L_0223:
		lodsb
		or	al,al
		jnz	L_0223
		cmp	al,byte ptr [si]
		jnz	L_0215
L_022C:
		pop	ds
		stc
		retn
L_022F:
		cmp	byte ptr [si],0
		jz	L_022C
		NOSMART		; lea kept (TASM would write mov reg,OFFSET)
		lea	di,[__OVRFILENAME]
		SMART
		mov	ax,_OVRGROUP_
		mov	es,ax
		xor	al,al
L_023F:
		mov	ah,al
		lodsb
		or	al,al
		jz	L_024D
		cmp	al,3Bh
		jz	L_024E
		stosb
		jmp	short L_023F
L_024D:
		dec	si
L_024E:
		cmp	ah,3Ah
		jz	L_025B
		cmp	ah,5Ch
		jz	L_025B
		mov	al,5Ch
		stosb
L_025B:
		push	ds
		push	si
		call	ovr_0266
		pop	si
		pop	ds
		jc	L_022F
		pop	ds
		retn
ovr_0209	ENDP

; 2B7A:0266
ovr_0266	PROC	NEAR
L_0266:
		lds	si,[bp+6]
		mov	ax,ds
		or	ax,si
		jz	L_027D
		mov	cx,0Ch
L_0272:
		lodsb
		stosb
		or	al,al
		jz	L_027D
		loop	L_0272
		sub	al,al
		stosb
L_027D:
		NOSMART		; lea kept (TASM would write mov reg,OFFSET)
		lea	dx,[__OVRFILENAME]
		SMART
		mov	ax,_OVRGROUP_
		mov	ds,ax
		mov	ax,3D00h
		int	21h
		retn
ovr_0266	ENDP

; 2B7A:028C
ovr_028C	PROC	NEAR
L_028C:
		push	ds
		lea	dx,[bp-14h]
		push	ss
		pop	ds
		mov	ah,3Fh
		int	21h
		pop	ds
		jc	L_029B
		cmp	ax,cx
L_029B:
		retn
ovr_028C	ENDP

; 2B7A:029C
		PUBLIC	__InitModules
__InitModules	PROC	NEAR
L_029C:
		mov	ax,DGROUP
		mov	es,ax
		ASSUME	DS:DGROUP
		mov	ax,es:[__psp]
		ASSUME	DS:_OVRGROUP_
		add	ax,10h
		mov	word ptr cs:T_0018,ax
		mov	ax,_STUB_
		mov	es,ax
		xor	bx,bx
		xor	di,di
		NOSMART		; lea kept (TASM would write mov reg,OFFSET)
		lea	si,[__SEGTABLE__]
		SMART
L_02B9:
		test	word ptr [si+4],2
		jz	L_02C6
		cmp	word ptr [si+2],0
		jnz	L_02C9
L_02C6:
		jmp	short L_0307
		db	090h
L_02C9:
		mov	ax,word ptr [si]
		push	es
		mov	es:[12h],ax
		mov	es,ax
		cmp	byte ptr es:[1Ah],0FFh
		jnz	L_02E5
		pop	es
		mov	word ptr es:[12h],0
		jmp	short L_0307
		db	090h
L_02E5:
		pop	ax
		mov	word ptr es:[18h],OFFSET __ReadOvrDisk
		mov	ax,[__OvrFileBase]
		mov	dx,word ptr [__OvrFileBase+2]
		add	word ptr es:[4],ax
		adc	word ptr es:[6],dx
		call	ovr_07AE
		cmp	bx,dx
		jnc	L_0307
		xchg	bx,dx
L_0307:
		add	si,8
		cmp	si,OFFSET _OVRGROUP_:__SEGTABEND__
		jnc	L_0312
		jmp	short L_02B9
L_0312:
		xor	ax,ax
		add	bx,2
		mov	word ptr [__OvrMinHeapSize],bx
		retn
__InitModules	ENDP

; 2B7A:031C
ovr_031C	PROC	NEAR
L_031C:
		mov	cx,word ptr [__OvrCodeList]
		mov	word ptr [__OvrLoadList],cx
		mov	bx,cx
		mov	si,word ptr [__OvrHeapOrg]
		mov	di,word ptr [__OvrHeapEnd]
		push	ds
L_032F:
		mov	ds,cx
		mov	cx,word ptr ds:[12h]
		jcxz	L_0365
		mov	es,cx
		mov	ax,es:[4]
		mov	dx,word ptr es:[6]
		sub	ax,word ptr ds:[4]
		sbb	dx,word ptr ds:[6]
		mov	cx,10h
		div	cx
		add	ax,si
		cmp	ax,di
		ja	L_0365
		mov	word ptr ds:[10h],si
		mov	si,ax
		mov	cx,es
		mov	word ptr ds:[1Ch],cx
		mov	bx,ds
		jmp	short L_032F
L_0365:
		mov	ds,bx
		mov	word ptr ds:[1Ch],0
		pop	ds
		mov	word ptr [__OvrHeapPtr],si
		mov	ax,[__OvrHeapOrg]
		sub	si,ax
		jnz	L_037C
		jmp	short L_03DA
		db	090h
L_037C:
		mov	cl,4
		rol	si,cl
		mov	di,si
		and	di,0Fh
		and	si,0FFF0h
		mov	es,word ptr [__OvrLoadList]
		mov	dx,word ptr es:[4]
		mov	cx,word ptr es:[6]
		call	ovr_03E9
		jc	L_03DB
		mov	ax,[__OvrLoadList]
		push	ds
L_039F:
		mov	es,ax
		mov	cx,word ptr es:[0Ah]
		jcxz	L_03AB
		call	ovr_0422
L_03AB:
		cmp	word ptr es:[0Ch],0
		jz	L_03B6
		call	ovr_0694
L_03B6:
		mov	ax,es:[10h]
		dec	ax
		mov	ds,ax
		mov	word ptr ds:[0Eh],es
		push	es
		mov	ax,0FFFFh
		mov	bx,ax
		mov	ds,word ptr cs:__OVRGROUP
		call	dword ptr [__OVRHOOK__]
		pop	es
		mov	ax,es:[1Ch]
		or	ax,ax
		jnz	L_039F
		pop	ds
L_03DA:
		clc
L_03DB:
		retn
ovr_031C	ENDP

; 2B7A:03DC
ovr_03DC	PROC	FAR
		mov	cl,4
		shr	ax,cl
		ror	dx,cl
		and	dx,0F000h
		or	ax,dx
		retn
ovr_03DC	ENDP

; 2B7A:03E9
ovr_03E9	PROC	NEAR
L_03E9:
		push	ax
		mov	bx,word ptr [__OvrDosHandle]
		mov	ax,4200h
		int	21h
		pop	ax
		push	ds
		mov	ds,ax
		jmp	short L_0400
L_03F9:
		mov	ax,ds
		add	ax,0FFFh
		mov	ds,ax
L_0400:
		mov	cx,0FFF0h
		or	di,di
		jnz	L_0409
		mov	cx,si
L_0409:
		xor	dx,dx
		mov	ah,3Fh
		int	21h
		jc	L_0420
		cmp	ax,cx
		jc	L_0420
		sub	si,ax
		sbb	di,0
		mov	ax,si
		or	ax,di
		jnz	L_03F9
L_0420:
		pop	ds
		retn
ovr_03E9	ENDP

; 2B7A:0422
ovr_0422	PROC	NEAR
L_0422:
		push	ds
		push	es
		mov	ax,es:[8]
		mov	si,ax
		and	si,0Fh
		shr	ax,1
		shr	ax,1
		shr	ax,1
		shr	ax,1
		mov	dx,word ptr es:[10h]
		add	ax,dx
		mov	ds,ax
		mov	es,dx
		shr	cx,1
		cld
L_0443:
		lodsw
		mov	bx,ax
		mov	di,word ptr es:[bx]
		push	ds
		mov	ax,_EXEINFO_
		mov	ds,ax
		mov	ax,di
		and	di,0FFF8h
		mov	dx,word ptr [di]
		mov	word ptr es:[bx],dx
		test	ax,1
		jz	L_0461
		call	ovr_0467
L_0461:
		pop	ds
		loop	L_0443
		pop	es
		pop	ds
		retn
ovr_0422	ENDP

; 2B7A:0467
ovr_0467	PROC	NEAR
L_0467:
		push	cx
		mov	ds,dx
		mov	ah,byte ptr es:[bx-1]
		mov	al,ah
		and	ax,0F807h
		cmp	ah,0B8h
		jnz	L_04C3
		mov	cx,ax
		mov	ah,byte ptr es:[bx+2]
		mov	al,ah
		and	ax,0F807h
		cmp	ah,50h
		jnz	L_04C3
		cmp	al,cl
		jnz	L_04C3
		mov	dx,ax
		mov	ah,byte ptr es:[bx+3]
		mov	al,ah
		and	ax,0F807h
		cmp	cx,ax
		jnz	L_04C3
		mov	ah,byte ptr es:[bx+6]
		mov	al,ah
		and	ax,0F807h
		cmp	ax,dx
		jnz	L_04C3
		mov	di,20h
		mov	cx,word ptr ds:[0Ch]
		mov	ax,word ptr es:[bx+4]
L_04B3:
		cmp	ax,word ptr [di+2]
		jz	L_04BF
		add	di,5
		loop	L_04B3
		jmp	short L_04C5
L_04BF:
		mov	word ptr es:[bx+4],di
L_04C3:
		pop	cx
		retn
L_04C5:
		pop	cx
		retn
ovr_0467	ENDP

; 2B7A:04C7
		PUBLIC	__ReadOvrDisk
__ReadOvrDisk	PROC	FAR
		mov	si,word ptr es:[8]
		xor	di,di
		add	si,word ptr es:[0Ah]
		adc	di,0
		mov	dx,word ptr es:[4]
		mov	cx,word ptr es:[6]
		mov	ax,es:[10h]
		call	ovr_03E9
		jc	L_04F4
		mov	cx,word ptr es:[0Ah]
		jcxz	L_04F3
		call	ovr_0422
L_04F3:
		clc
L_04F4:
		retn
__ReadOvrDisk	ENDP

; 2B7A:04F5
ovr_04F5	PROC	FAR
		push	bp
		mov	bp,sp
		test	bp,1
		jz	L_0503
		jmp	far ptr __OverlayHalt
L_0503:
		push	ax
		push	bx
		push	cx
		push	dx
		push	si
		push	di
		push	ds
		push	es
		mov	ax,_OVRGROUP_
		mov	ds,ax
		sti
		les	bx,[bp+2]
		push	word ptr es:[bx]
		sub	word ptr [bp+2],2
		jnz	L_0522
		call	ovr_05A5
		jmp	short L_053A
L_0522:
		add	bp,6
		mov	ax,word ptr [bp+0]
		xchg	word ptr [bp-6],ax
		mov	word ptr [bp+0],ax
		call	ovr_05A5
		mov	ax,word ptr [bp+0]
		xchg	word ptr [bp-6],ax
		mov	word ptr [bp+0],ax
L_053A:
		pop	bx
		mov	al,es:[1Ah]
		and	al,8
		and	byte ptr es:[1Ah],0F7h
		cbw
		mov	ds,word ptr cs:__OVRGROUP
		call	dword ptr [__OVRHOOK__]
		pop	es
		pop	ds
		pop	di
		pop	si
		pop	dx
		pop	cx
		pop	bx
		pop	ax
		pop	bp
		iret
ovr_04F5	ENDP

; 2B7A:055B
ovr_055B	PROC	NEAR
L_055B:
		push	es
		inc	word ptr [__OvrLoadCount]
		call	ovr_07AE
		jmp	short L_0593
L_0565:
		popf
		push	dx
		jnc	L_056C
		call	ovr_0638
L_056C:
		mov	es,word ptr [__OvrLoadList]
		mov	ax,es:[1Ch]
		mov	[__OvrLoadList],ax
		cmp	byte ptr es:[1Bh],0
		jnz	L_0587
		call	ovr_0620
		call	ovr_07A2
		jmp	short L_0592
L_0587:
		dec	byte ptr es:[1Bh]
		call	ovr_06E5
		call	ovr_0736
L_0592:
		pop	dx
L_0593:
		call	ovr_0786
		pushf
		cmp	dx,ax
		ja	L_0565
		popf
		pop	es
		mov	ax,[__OvrHeapPtr]
		mov	es:[10h],ax
		retn
ovr_055B	ENDP

; 2B7A:05A5
ovr_05A5	PROC	NEAR
L_05A5:
		inc	word ptr [__OvrTrapCount]
		cmp	word ptr es:[10h],0
		jz	L_05C0
		mov	byte ptr es:[1Bh],1
		or	byte ptr es:[1Ah],4
		jmp	short L_05DC
		db	090h
L_05C0:
		or	byte ptr es:[1Ah],8
		call	ovr_055B
		push	ds
		dec	ax
		mov	ds,ax
		mov	word ptr ds:[0Eh],es
		pop	ds
		call	word ptr es:[18h]
		jc	L_061B
		call	ovr_0736
L_05DC:
		call	ovr_0673
		mov	al,es:[1Ah]
		and	al,3
		add	byte ptr es:[1Bh],al
		push	es
		call	ovr_0786
		mov	es,word ptr [__OvrLoadList]
L_05F2:
		mov	cx,word ptr es:[1Ch]
		jcxz	L_0619
		cmp	ax,word ptr [__OvrRetrySize]
		jnc	L_0619
		push	cx
		push	ax
		cmp	byte ptr es:[1Bh],0
		jz	L_060D
		xor	ax,ax
		jmp	short L_0613
L_060D:
		call	ovr_06B2
		call	ovr_07A2
L_0613:
		pop	cx
		pop	es
		add	ax,cx
		jmp	short L_05F2
L_0619:
		pop	es
		retn
L_061B:
		jmp	far ptr __OverlayHalt
ovr_05A5	ENDP

; 2B7A:0620
ovr_0620	PROC	NEAR
L_0620:
		call	ovr_06B2
		cmp	word ptr es:[18h],OFFSET __ReadOvrDisk
		jnz	L_0630
		call	word ptr [__CALLEXTEMSSWAP]
L_0630:
		mov	word ptr es:[10h],0
		retn
ovr_0620	ENDP

; 2B7A:0638
ovr_0638	PROC	NEAR
L_0638:
		mov	ax,[__OvrLoadList]
		xor	cx,cx
L_063D:
		inc	cx
		push	ax
		mov	es,ax
		mov	ax,es:[1Ch]
		or	ax,ax
		jnz	L_063D
		mov	[__OvrLoadList],ax
		mov	ax,[__OvrHeapEnd]
		mov	[__OvrHeapPtr],ax
L_0652:
		pop	es
		push	cx
		mov	ax,[__OvrLoadList]
		mov	es:[1Ch],ax
		mov	word ptr [__OvrLoadList],es
		call	ovr_07A2
		sub	word ptr [__OvrHeapPtr],ax
		call	ovr_06E5
		pop	cx
		loop	L_0652
		mov	ax,[__OvrHeapOrg]
		mov	[__OvrHeapPtr],ax
		retn
ovr_0638	ENDP

; 2B7A:0673
ovr_0673	PROC	NEAR
L_0673:
		cmp	word ptr es:[0Ch],0
		jnz	L_067C
		retn
L_067C:
		cmp	byte ptr es:[20h],0EAh
		jz	L_06B1
		mov	cx,word ptr es:[2]
		jcxz	L_0694
		mov	ax,es:[10h]
		mov	dx,es
		call	ovr_0754
ovr_0673	ENDP

; 2B7A:0694
ovr_0694	PROC	NEAR
L_0694:
		mov	bx,word ptr es:[10h]
		mov	cx,word ptr es:[0Ch]
		mov	di,20h
		cld
L_06A2:
		mov	dx,word ptr es:[di+2]
		mov	al,0EAh
		stosb
		mov	ax,dx
		stosw
		mov	ax,bx
		stosw
		loop	L_06A2
L_06B1:
		retn
ovr_0694	ENDP

; 2B7A:06B2
ovr_06B2	PROC	NEAR
L_06B2:
		cmp	byte ptr es:[20h],0CDh
		jz	L_06E4
		mov	ax,es
		mov	dx,word ptr es:[10h]
		xor	cx,cx
		call	ovr_0754
		mov	word ptr es:[2],cx
		mov	cx,word ptr es:[0Ch]
		mov	di,20h
		cld
L_06D4:
		mov	dx,word ptr es:[di+1]
		mov	ax,[__OVRTRAP__]
		stosw
		mov	ax,dx
		stosw
		xor	al,al
		stosb
		loop	L_06D4
L_06E4:
		retn
ovr_06B2	ENDP

; 2B7A:06E5
ovr_06E5	PROC	NEAR
L_06E5:
		mov	ax,[__OvrHeapPtr]
		mov	dx,word ptr es:[10h]
		mov	es:[10h],ax
		mov	cx,word ptr es:[8]
		inc	cx
		shr	cx,1
		xor	si,si
		cld
		cmp	ax,dx
		jc	L_0706
		mov	si,cx
		dec	si
		shl	si,1
		std
L_0706:
		mov	di,si
		push	ds
		push	es
		mov	ds,dx
		mov	es,ax
		rep	movsw
		cld
		dec	ax
		mov	ds,ax
		pop	es
		mov	word ptr ds:[0Eh],es
		inc	ax
		pop	ds
		cmp	byte ptr es:[20h],0CDh
		jz	L_0735
		call	ovr_0760
		mov	cx,word ptr es:[0Ch]
		mov	di,23h
		cld
L_072F:
		stosw
		add	di,3
		loop	L_072F
L_0735:
		retn
ovr_06E5	ENDP

; 2B7A:0736
ovr_0736	PROC	NEAR
L_0736:
		call	ovr_07A2
		add	word ptr [__OvrHeapPtr],ax
		push	ds
		mov	ax,_STUB_
L_0741:
		mov	ds,ax
		mov	ax,ds:[1Ch]
		or	ax,ax
		jnz	L_0741
		mov	word ptr ds:[1Ch],es
		mov	es:[1Ch],ax
		pop	ds
		retn
ovr_0736	ENDP

; 2B7A:0754
ovr_0754	PROC	NEAR
L_0754:
		call	ovr_0760
		or	bx,bx
		jz	L_075F
		xchg	word ptr ss:[bx+2],cx
L_075F:
		retn
ovr_0754	ENDP

; 2B7A:0760
ovr_0760	PROC	NEAR
L_0760:
		xor	bx,bx
		push	cx
		push	bp
		jmp	short L_076A
L_0766:
		shl	cx,1
		mov	bp,cx
L_076A:
		mov	cx,word ptr [bp+0]
		shr	cx,1
		jz	L_0783
		jc	L_0766
		cmp	dx,word ptr [bp+4]
		jnz	L_0766
		mov	word ptr [bp+4],ax
		or	bx,bx
		jnz	L_0766
		mov	bx,bp
		jmp	short L_0766
L_0783:
		pop	bp
		pop	cx
		retn
ovr_0760	ENDP

; 2B7A:0786
ovr_0786	PROC	NEAR
L_0786:
		mov	ax,[__OvrLoadList]
		or	ax,ax
		jz	L_0799
		mov	es,ax
		mov	ax,es:[10h]
		sub	ax,word ptr [__OvrHeapPtr]
		jnc	L_07A1
L_0799:
		mov	ax,[__OvrHeapEnd]
		sub	ax,word ptr [__OvrHeapPtr]
		stc
L_07A1:
		retn
ovr_0786	ENDP

; 2B7A:07A2
ovr_07A2	PROC	NEAR
L_07A2:
		mov	ax,es:[8]
		add	ax,11h
		mov	cl,4
		shr	ax,cl
		retn
ovr_07A2	ENDP

; 2B7A:07AE
ovr_07AE	PROC	NEAR
L_07AE:
		mov	cl,4
		mov	ax,es:[8]
		add	ax,11h
		shr	ax,cl
		mov	dx,word ptr es:[0Ah]
		add	dx,0Fh
		shr	dx,cl
		add	dx,ax
		retn
ovr_07AE	ENDP

; 2B7A:07C6
		PUBLIC	__ISOVERLAYMOD
__ISOVERLAYMOD	PROC	FAR
		push	ds
		push	es
		mov	ds,word ptr cs:__OVRGROUP
		cmp	ax,word ptr [__OvrHeapOrg]
		jc	L_07F6
		cmp	ax,word ptr [__OvrHeapEnd]
		jnc	L_07F6
		dec	ax
		mov	es,ax
		mov	es,word ptr es:[0Eh]
		inc	ax
		mov	bx,word ptr [__OVRTRAP__]
		cmp	bx,word ptr es:[0]
		jnz	L_07F6
		cmp	ax,word ptr es:[10h]
		mov	ax,es
		jz	L_07F9
L_07F6:
		xor	ax,ax
		stc
L_07F9:
		pop	es
		pop	ds
		retf
__ISOVERLAYMOD	ENDP

; 2B7A:07FC
		PUBLIC	__CHECKSTACK
__CHECKSTACK	PROC	FAR
		push	bp
		mov	bp,sp
		sub	sp,2
		mov	byte ptr [bp-2],0
		push	ds
		mov	ds,word ptr cs:__OVRGROUP
		push	si
		push	di
		mov	bx,bp
		mov	ax,[__OVRTRAP__]
		mov	dx,word ptr [__OvrHeapOrg]
		jmp	short L_081D
L_0819:
		shl	si,1
		mov	bx,si
L_081D:
		cmp	bx,cx
		jnc	L_084B
		mov	si,word ptr ss:[bx]
		shr	si,1
		jz	L_084B
		jc	L_0819
		mov	di,word ptr ss:[bx+4]
		cmp	di,dx
		jnc	L_0819
		mov	es,di
		mov	di,word ptr ss:[bx+2]
		or	di,di
		jnz	L_0819
		cmp	word ptr es:[di],ax
		jnz	L_0819
		mov	byte ptr [bp-2],1
		mov	word ptr es:[di+2],di
		jmp	short L_0819
L_084B:
		cmp	byte ptr [bp-2],1
		jz	L_0858
		jmp	short L_088D
		db	090h
L_0854:
		shl	si,1
		mov	bx,si
L_0858:
		mov	si,word ptr ss:[bx]
		shr	si,1
		jz	L_088D
		jc	L_0854
		mov	di,word ptr ss:[bx+4]
		cmp	di,dx
		jnc	L_0854
		mov	es,di
		mov	di,word ptr ss:[bx+2]
		or	di,di
		jz	L_0854
		cmp	word ptr es:[0],ax
		jnz	L_0854
		cmp	word ptr es:[2],0
		jnz	L_0854
		xchg	word ptr es:[2],di
		mov	word ptr ss:[bx+2],di
		jmp	short L_0854
L_088D:
		pop	di
		pop	si
		pop	ds
		mov	sp,bp
		pop	bp
		retf
__CHECKSTACK	ENDP

; 2B7A:0894
		PUBLIC	__CHECKOVERLAY
__CHECKOVERLAY	PROC	FAR
		push	bp
		mov	bp,sp
		push	ds
		mov	ds,word ptr cs:__OVRGROUP
		mov	es,word ptr [bp+8]
		mov	cx,word ptr [__OVRTRAP__]
		cmp	cx,word ptr es:[0]
		jnz	L_08E9
		cmp	word ptr es:[10h],0
		jz	L_08BB
		cmp	byte ptr es:[20h],0CDh
		jnz	L_08E2
L_08BB:
		mov	word ptr [bp+8],0
		push	si
		push	di
		call	ovr_05A5
		pop	di
		pop	si
		mov	bx,word ptr [bp+6]
		push	es
		mov	al,es:[1Ah]
		and	al,8
		and	byte ptr es:[1Ah],0F7h
		cbw
		mov	ds,word ptr cs:__OVRGROUP
		call	dword ptr [__OVRHOOK__]
		pop	es
L_08E2:
		mov	ax,es:[10h]
		mov	word ptr [bp+8],ax
L_08E9:
		pop	ds
		pop	bp
		retf
__CHECKOVERLAY	ENDP


_OVRTEXT_	ENDS

		END
