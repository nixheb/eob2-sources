;
; Eye of the Beholder II - START.EXE
; The music of the PC speaker or of the Tandy / PCjr sound chip (port C0h,
; 3 voices): a handler on INT 8 (the timer, set to 4 times its speed)
; plays the notes; module 1CC7 (Ghidra 2CC7: the segment starts at 2CC7:0005),
; written in
; assembler, with a data segment of its own (Ghidra 41BE:0004).
;
; Assembled with
;     tasm /ml music.asm
; [asm] = identical to START.EXE byte for byte (tools/asm.py).
;
		LOCALS
; no .MODEL: its empty default segments would move the code (the C names
; get their underscore by hand)

MPLAY_TEXT	SEGMENT	BYTE PUBLIC 'CODE'
		ASSUME	CS:MPLAY_TEXT, DS:MPLAY_DATA
T_0005		db	000h, 000h
T_0007		db	000h, 000h

; 2CC7:0009
music_tick	PROC	FAR
		push	ds
		push	ax
		mov	ax,SEG D_0004
		mov	ds,ax
		mov	ax,word ptr D_01C6
		inc	ax
		cmp	ax,123h
		jle	@@0029
		mov	al,D_01C8
		inc	al
		cmp	al,3Bh
		jle	@@0024
		xor	al,al
@@0024:
		mov	D_01C8,al
		xor	ax,ax
@@0029:
		mov	word ptr D_01C6,ax
		call	music_step
		mov	ax,word ptr D_0006
		or	al,al
		jnz	@@0043
		mov	ax,word ptr D_0004
		mov	word ptr D_0006,ax
		pop	ax
		pop	ds
		jmp	dword ptr cs:T_0005
@@0043:
		dec	al
		mov	word ptr D_0006,ax
		mov	al,20h
		out	20h,al
		pop	ax
		pop	ds
		iret
music_tick	ENDP

; 2CC7:004F
music_step	PROC	NEAR
@@004F:
		cmp	byte ptr D_019C,0
		jz	@@00AF
		mov	byte ptr D_01C4,1
		mov	byte ptr D_019B,1
		mov	byte ptr D_019C,0
		mov	byte ptr D_019D,0
		mov	ax,word ptr D_0014
		mov	word ptr D_0008,ax
		mov	ax,word ptr D_0016
		mov	word ptr D_000A,ax
		push	di
		xor	di,di
		mov	al,3
@@007B:
		mov	byte ptr [di+D_01A2],1
		mov	byte ptr [di+D_019E],0
		mov	byte ptr [di+D_01A6],0
		mov	byte ptr [di+D_01B6],1
		shl	di,1
		mov	word ptr [di+D_000C],0
		mov	word ptr [di+D_01AA],0
		shr	di,1
		inc	di
		dec	al
		jnz	@@007B
		pop	di
		mov	byte ptr D_01C2,23h
		mov	byte ptr D_01C3,46h
@@00AF:
		cmp	byte ptr D_019B,0
		jnz	@@00B9
		jmp	@@02C1
@@00B9:
		push	es
		push	si
		push	di
		push	bp
		push	bx
		push	dx
		xor	si,si
@@00C1:
		mov	di,si
		shl	di,1
		mov	bp,word ptr [di+D_000C]
		cmp	si,1
		jnz	@@00DC
		cmp	byte ptr D_01C2,0
		jz	@@00EF
		dec	byte ptr D_01C2
		jmp	@@0287
@@00DC:
		cmp	si,2
		jnz	@@00EF
		cmp	byte ptr D_01C3,0
		jz	@@00EF
		dec	byte ptr D_01C3
		jmp	@@0287
@@00EF:
		cmp	byte ptr [si+D_01A2],0
		jnz	@@00F9
		jmp	@@0253
@@00F9:
		mov	es,word ptr D_000A
		mov	bx,word ptr D_0008
		add	bx,bp
@@0103:
		mov	al,byte ptr es:[bx]
		or	al,al
		js	@@0156
		cmp	al,5Fh
		jle	@@0111
		jmp	@@027A
@@0111:
		push	di
		xor	ah,ah
		shl	ax,1
		mov	di,ax
		cmp	byte ptr _Var_6C45,1
		jz	@@0125
		mov	ax,word ptr [di+D_0018]
		jmp	short @@0129
@@0125:
		mov	ax,word ptr [di+D_00D8]
@@0129:
		pop	di
@@012A:
		or	ax,ax
		jz	@@0136
		call	voice_freq
		call	voice_on
		jmp	short @@0139
@@0136:
		call	voice_off
@@0139:
		inc	bx
		xor	ah,ah
		mov	al,byte ptr es:[bx]
		inc	al
		mul	byte ptr [si+D_01B6]
		dec	ax
		mov	word ptr [di+D_01AA],ax
		inc	bx
		add	bp,2
		mov	byte ptr [si+D_01A2],0
		jmp	@@0287
@@0156:
		cmp	al,0CEh
		jnz	@@0178
		dec	byte ptr [si+D_01BA]
		jnz	@@0168
		add	bx,3
		add	bp,3
		jmp	short @@0103
@@0168:
		inc	bx
		mov	ax,word ptr es:[bx]
		inc	bp
		sub	bx,ax
		sub	bp,ax
		jno	@@0176
		jmp	@@027A
@@0176:
		jmp	short @@0103
@@0178:
		cmp	al,0CDh
		jnz	@@0191
		inc	bx
		mov	al,byte ptr es:[bx]
		or	al,al
		jnz	@@0186
		mov	al,1
@@0186:
		mov	byte ptr [si+D_01BA],al
		inc	bx
		add	bp,2
		jmp	@@0103
@@0191:
		cmp	al,0D2h
		jnz	@@01B5
		dec	byte ptr [si+D_01BE]
		jnz	@@01A4
		add	bx,3
		add	bp,3
		jmp	@@0103
@@01A4:
		inc	bx
		mov	ax,word ptr es:[bx]
		inc	bp
		sub	bx,ax
		sub	bp,ax
		jno	@@01B2
		jmp	@@027A
@@01B2:
		jmp	@@0103
@@01B5:
		cmp	al,0D1h
		jnz	@@01CE
		inc	bx
		mov	al,byte ptr es:[bx]
		or	al,al
		jnz	@@01C3
		mov	al,1
@@01C3:
		mov	byte ptr [si+D_01BE],al
		inc	bx
		add	bp,2
		jmp	@@0103
@@01CE:
		cmp	al,0E6h
		jnz	@@01FD
		inc	bx
		cmp	byte ptr _Var_6C45,1
		jz	@@01E5
		mov	dx,12h
		mov	ax,34DCh
		div	word ptr es:[bx]
		jmp	short @@01F6
@@01E5:
		mov	dx,1
		mov	ax,0B4F4h
		div	word ptr es:[bx]
		cmp	ax,400h
		jc	@@01F6
		mov	ax,3FFh
@@01F6:
		inc	bx
		add	bp,2
		jmp	@@012A
@@01FD:
		cmp	al,0E2h
		jnz	@@0216
		inc	bx
		mov	al,byte ptr es:[bx]
		or	al,al
		jnz	@@020B
		mov	al,1
@@020B:
		mov	byte ptr [si+D_01B6],al
		inc	bx
		add	bp,2
		jmp	@@0103
@@0216:
		cmp	al,0E8h
		jnz	@@0232
		inc	bx
		mov	al,byte ptr es:[bx]
		sub	byte ptr [si+D_01B6],al
		jno	@@022B
		jnz	@@022B
		mov	byte ptr [si+D_01B6],1
@@022B:
		inc	bx
		add	bp,2
		jmp	@@0103
@@0232:
		cmp	al,0E9h
		jnz	@@024C
		inc	bx
		mov	al,byte ptr es:[bx]
		add	byte ptr [si+D_01B6],al
		jno	@@0245
		mov	byte ptr [si+D_01B6],0FFh
@@0245:
		inc	bx
		add	bp,2
		jmp	@@0103
@@024C:
		mov	byte ptr [si+D_01A2],0
		jmp	short @@027A
@@0253:
		cmp	byte ptr D_019D,1
		jz	@@027A
		cmp	byte ptr [si+D_01A6],1
		jz	@@0273
		dec	word ptr [di+D_01AA]
		jns	@@0273
		cmp	word ptr [di+D_01AA],-1
		jz	@@0273
		mov	byte ptr [si+D_01A2],1
@@0273:
		cmp	byte ptr [si+D_019E],0
		jz	@@0287
@@027A:
		mov	byte ptr [si+D_019E],0
		mov	byte ptr [si+D_01A6],1
		call	voice_off
@@0287:
		mov	word ptr [di+D_000C],bp
		inc	si
		cmp	word ptr D_0199,si
		jle	@@0295
		jmp	@@00C1
@@0295:
		xor	ax,ax
		dec	si
@@0298:
		cmp	byte ptr [si+D_01A6],1
		jnz	@@02A0
		inc	ax
@@02A0:
		dec	si
		cmp	si,0
		jge	@@0298
		cmp	word ptr D_0199,ax
		jnz	@@02BB
		mov	byte ptr D_019D,0
		mov	byte ptr D_019B,0
		mov	byte ptr D_01C4,0
@@02BB:
		pop	dx
		pop	bx
		pop	bp
		pop	di
		pop	si
		pop	es
@@02C1:
		ret
music_step	ENDP

; 2CC7:02C2
voice_on	PROC	NEAR
@@02C2:
		cmp	byte ptr _Var_6C45,1
		jz	@@02D0
		in	al,61h
		or	al,3
		out	61h,al
		ret
@@02D0:
		mov	ax,si
		mov	ah,byte ptr [si+D_01B2]
		xor	ah,0Fh
		shl	al,1
		inc	al
		shl	al,1
		shl	al,1
		shl	al,1
		shl	al,1
		or	al,ah
		or	al,80h
		out	0C0h,al
		ret
voice_on	ENDP

; 2CC7:02EC
voice_off	PROC	NEAR
@@02EC:
		cmp	byte ptr _Var_6C45,1
		jz	@@02FA
		in	al,61h
		and	al,0FCh
		out	61h,al
		ret
@@02FA:
		mov	ax,si
		mov	ah,0Fh
		shl	al,1
		inc	al
		shl	al,1
		shl	al,1
		shl	al,1
		shl	al,1
		or	al,ah
		or	al,80h
		out	0C0h,al
		ret
voice_off	ENDP

; 2CC7:0311
voice_freq	PROC	NEAR
@@0311:
		cmp	byte ptr _Var_6C45,1
		jz	@@031F
		out	42h,al
		mov	al,ah
		out	42h,al
		ret
@@031F:
		mov	dx,ax
		shl	dx,1
		shl	dx,1
		shl	dx,1
		shl	dx,1
		mov	dl,al
		and	dl,0Fh
		mov	ax,si
		shl	al,1
		shl	al,1
		shl	al,1
		shl	al,1
		shl	al,1
		or	al,dl
		or	al,80h
		out	0C0h,al
		mov	al,dh
		out	0C0h,al
		ret
voice_freq	ENDP

; 2CC7:0345
tandy_init	PROC	NEAR
@@0345:
		mov	word ptr D_0199,3
		xor	si,si
@@034D:
		mov	byte ptr [si+D_01A6],1
		call	voice_off
		inc	si
		cmp	si,4
		jnz	@@034D
		in	al,61h
		or	al,60h
		out	61h,al
		ret
tandy_init	ENDP

; 2CC7:0362
speaker_init	PROC	NEAR
@@0362:
		mov	word ptr D_0199,1
		in	al,61h
		mov	al,D_0198
		and	al,0FCh
		out	61h,al
		ret
speaker_init	ENDP

; 2CC7:0372
		PUBLIC	_digi_init
_digi_init	PROC	FAR
		push	bp
		mov	bp,sp
		push	di
		push	si
		push	ds
		mov	ax,SEG D_0004
		mov	ds,ax
		push	ds
		mov	al,8
		mov	ah,35h
		int	21h
		mov	word ptr cs:T_0005,bx
		mov	word ptr cs:T_0007,es
		mov	word ptr D_0004,0Fh
		mov	word ptr D_0006,0
		mov	ax,cs
		mov	ds,ax
		mov	dx,OFFSET music_tick
		mov	al,8
		mov	ah,25h
		int	21h
		mov	al,36h
		out	43h,al
		mov	ax,1000h
		out	40h,al
		mov	al,ah
		out	40h,al
		mov	al,0B6h
		out	43h,al
		pop	ds
		in	al,61h
		mov	D_0198,al
		out	61h,al
		cmp	byte ptr _Var_6C45,1
		jz	@@03CC
		call	speaker_init
		jmp	short @@03CF
@@03CC:
		call	tandy_init
@@03CF:
		pop	ds
		pop	si
		pop	di
		pop	bp
		retf
_digi_init	ENDP

; 2CC7:03D4
		PUBLIC	_digi_shutdown
_digi_shutdown	PROC	FAR
		push	bp
		mov	bp,sp
		push	di
		push	si
		push	ds
		mov	ax,SEG D_0004
		mov	ds,ax
		push	ds
		mov	dx,word ptr cs:T_0005
		mov	bx,word ptr cs:T_0007
		mov	ds,bx
		mov	al,8
		mov	ah,25h
		int	21h
		mov	al,36h
		out	43h,al
		mov	ax,0
		out	40h,al
		mov	al,ah
		out	40h,al
		pop	ds
		cmp	byte ptr _Var_6C45,1
		jnz	@@040A
		call	tandy_init
@@040A:
		call	speaker_init
		pop	ds
		pop	si
		pop	di
		pop	bp
		retf
_digi_shutdown	ENDP

; 2CC7:0412
		PUBLIC	_digi_play
_digi_play	PROC	FAR
		push	bp
		mov	bp,sp
		push	di
		push	si
		push	ds
		mov	ax,SEG D_0004
		mov	ds,ax
		mov	ax,word ptr [bp+6]
		mov	word ptr D_0014,ax
		mov	ax,word ptr [bp+8]
		mov	word ptr D_0016,ax
		mov	byte ptr D_019C,1
		mov	byte ptr D_019B,1
		mov	byte ptr D_01C4,1
		pop	ds
		pop	si
		pop	di
		pop	bp
		retf
_digi_play	ENDP

; 2CC7:043D
		PUBLIC	_music_stop
_music_stop	PROC	FAR
		push	bp
		mov	bp,sp
		push	di
		push	si
		push	ds
		mov	ax,SEG D_0004
		mov	ds,ax
		mov	byte ptr D_019D,1
		pop	ds
		pop	si
		pop	di
		pop	bp
		retf
_music_stop	ENDP

; 2CC7:0452
		PUBLIC	_digi_mode
_digi_mode	PROC	FAR
		push	bp
		mov	bp,sp
		push	di
		push	si
		push	ds
		mov	ax,SEG D_0004
		mov	ds,ax
		call	tandy_init
		pop	ds
		pop	si
		pop	di
		pop	bp
		retf
_digi_mode	ENDP

; 2CC7:0465
		PUBLIC	_digi_mode_off
_digi_mode_off	PROC	FAR
		push	bp
		mov	bp,sp
		push	di
		push	si
		push	ds
		mov	ax,SEG D_0004
		mov	ds,ax
		call	tandy_init
		call	speaker_init
		pop	ds
		pop	si
		pop	di
		pop	bp
		retf
_digi_mode_off	ENDP

MPLAY_TEXT	ENDS

; the data segment (41BE:0004, right after the runtime's _environ: WORD
; aligned, its labels are offsets from paragraph 41BE): the state of the
; 3 voices, the note tables
MPLAY_DATA	SEGMENT	WORD PUBLIC 'DATA'
D_0004		db	000h, 000h
D_0006		db	000h, 000h
D_0008		db	000h, 000h
D_000A		db	000h, 000h
D_000C		db	000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h
D_0014		db	000h, 000h
D_0016		db	000h, 000h
D_0018		db	000h, 000h, 0FFh, 0FDh, 0A2h, 0EFh, 041h, 0E2h, 082h, 0D5h, 098h, 0C9h, 03Dh, 0BEh, 08Ah, 0B3h
		db	07Ch, 0A9h, 0F2h, 09Fh, 0FCh, 096h, 089h, 08Eh, 083h, 086h, 0F7h, 07Eh, 0D9h, 077h, 021h, 071h
		db	0C7h, 06Ah, 0C6h, 064h, 01Fh, 05Fh, 0CAh, 059h, 0BEh, 054h, 0FDh, 04Fh, 07Eh, 04Bh, 042h, 047h
		db	042h, 043h, 07Bh, 03Fh, 0DBh, 03Bh, 08Fh, 038h, 062h, 035h, 063h, 032h, 08Fh, 02Fh, 0E4h, 02Ch
		db	05Fh, 02Ah, 0FEh, 027h, 0C0h, 025h, 0A1h, 023h, 0A1h, 021h, 0BEh, 01Fh, 0F6h, 01Dh, 048h, 01Ch
		db	0B1h, 01Ah, 032h, 019h, 0C8h, 017h, 072h, 016h, 030h, 015h, 0FFh, 013h, 0E0h, 012h, 0D1h, 011h
		db	0D1h, 010h, 0DFh, 00Fh, 0FBh, 00Eh, 024h, 00Eh, 059h, 00Dh, 099h, 00Ch, 0E4h, 00Bh, 039h, 00Bh
		db	098h, 00Ah, 000h, 00Ah, 070h, 009h, 0E8h, 008h, 068h, 008h, 0F0h, 007h, 07Eh, 007h, 012h, 007h
		db	0ACh, 006h, 04Ch, 006h, 0F2h, 005h, 09Dh, 005h, 04Ch, 005h, 000h, 005h, 0B8h, 004h, 074h, 004h
		db	034h, 004h, 0F8h, 003h, 0BFh, 003h, 082h, 003h, 056h, 003h, 026h, 003h, 0F9h, 002h, 0CEh, 002h
		db	0A6h, 002h, 080h, 002h, 05Ch, 002h, 03Ah, 002h, 01Ah, 002h, 0FCh, 001h, 0DFh, 001h, 0C4h, 001h
		db	0ABh, 001h, 093h, 001h, 07Ch, 001h, 067h, 001h, 053h, 001h, 040h, 001h, 02Eh, 001h, 01Dh, 001h
D_00D8		db	000h, 000h, 0FFh, 003h, 0FFh, 003h, 0FFh, 003h, 0FFh, 003h, 0FFh, 003h, 0FFh, 003h, 0FFh, 003h
		db	0FFh, 003h, 0FFh, 003h, 0FFh, 003h, 0FFh, 003h, 0FFh, 003h, 0FFh, 003h, 0FFh, 003h, 0FFh, 003h
		db	0FFh, 003h, 0FFh, 003h, 0FFh, 003h, 0FFh, 003h, 0FFh, 003h, 0FFh, 003h, 0FFh, 003h, 0FFh, 003h
		db	0FFh, 003h, 0FFh, 003h, 0FFh, 003h, 0FFh, 003h, 0FFh, 003h, 0FFh, 003h, 0FFh, 003h, 0FFh, 003h
		db	0F9h, 003h, 0C0h, 003h, 08Ah, 003h, 057h, 003h, 027h, 003h, 0FAh, 002h, 0CFh, 002h, 0A7h, 002h
		db	081h, 002h, 05Dh, 002h, 03Bh, 002h, 01Bh, 002h, 0FCh, 001h, 0E0h, 001h, 0C5h, 001h, 0ACh, 001h
		db	094h, 001h, 07Dh, 001h, 068h, 001h, 053h, 001h, 040h, 001h, 02Eh, 001h, 01Dh, 001h, 00Dh, 001h
		db	0FEh, 000h, 0F0h, 000h, 0E2h, 000h, 0D6h, 000h, 0CAh, 000h, 0BEh, 000h, 0B4h, 000h, 0AAh, 000h
		db	0A0h, 000h, 097h, 000h, 08Fh, 000h, 087h, 000h, 07Fh, 000h, 078h, 000h, 071h, 000h, 06Bh, 000h
		db	065h, 000h, 05Fh, 000h, 05Ah, 000h, 054h, 000h, 050h, 000h, 04Ch, 000h, 047h, 000h, 043h, 000h
		db	040h, 000h, 03Ch, 000h, 039h, 000h, 035h, 000h, 032h, 000h, 030h, 000h, 02Dh, 000h, 02Ah, 000h
		db	028h, 000h, 026h, 000h, 024h, 000h, 022h, 000h, 020h, 000h, 01Eh, 000h, 01Ch, 000h, 01Bh, 000h
D_0198		db	000h
D_0199		db	000h, 000h
D_019B		db	000h
D_019C		db	000h
D_019D		db	000h
D_019E		db	000h, 000h, 000h, 000h
D_01A2		db	000h, 000h, 000h, 000h
D_01A6		db	000h, 000h, 000h, 000h
D_01AA		db	000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h
D_01B2		db	00Fh, 00Ah, 005h, 000h
D_01B6		db	000h, 000h, 000h, 000h
D_01BA		db	000h, 000h, 000h, 000h
D_01BE		db	000h, 000h, 000h, 000h
D_01C2		db	000h
D_01C3		db	000h
D_01C4		db	000h
		PUBLIC	_Var_6C45		; DS:6C45 (audio.c)
_Var_6C45		db	000h
D_01C6		db	000h, 000h
D_01C8		db	000h

MPLAY_DATA	ENDS

		END
