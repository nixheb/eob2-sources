;
; Eye of the Beholder II - START.EXE
; The video driver (MGA.OVL...) loaded as a DOS overlay; module 23CD
; (Ghidra 33CD, from 33CD:000A), written in assembler.
;
; Assembled with
;     tasm /ml vdriver.asm
; [asm] = identical to START.EXE byte for byte (tools/asm.py).
;
		.MODEL	LARGE, C
		LOCALS
		INCLUDE	procs.inc
		GLOBAL	mem_alloc:FAR, mem_realloc:FAR, wait_tick:FAR

		.DATA
		INCLUDE	eob2.inc
		.CODE

; 33CD:000A Loads the program ds:dx (DOS 4B03h, as an overlay) into 64000 bytes
; allocated; its segment goes to es:[bx]; BX = 0, or -1
		PUBLIC	exec_overlay
exec_overlay	PROC	FAR
@@000A:
		push	bp
		mov	bp,sp
		push	dx
		push	bx
		push	ds
		push	es
		mov	ax,SEG MousePresent
		mov	ds,ax
		mov	ax,20h
		push	ax
		xor	ax,ax
		push	ax
		mov	ax,0FA00h
		push	ax
		call	far ptr mem_alloc
		add	sp,6
		pop	es
		pop	ds
		pop	bx
		or	ax,dx
		jnz	@@0036
		pop	dx
		mov	bx,0FFFFh
		jmp	short @@005B
@@0036:
		pop	dx
		mov	word ptr es:[bx],ax
		mov	SavedSP,sp
		mov	SavedSS,ss
		mov	ax,4B03h
		int	21h
		mov	ax,SEG MousePresent
		mov	ds,ax
		mov	sp,SavedSP
		mov	ss,SavedSS
		mov	bx,0FFFFh
		jc	@@005B
		xor	bx,bx
@@005B:
		pop	bp
		retf
exec_overlay	ENDP

; 33CD:005D Loads the video driver (name: its file); its entry is called with
; 21h (the function: install) and wait_tick, then with the name to its
; table; 0, or -1 (no memory, not found). The mouse driver present: the
; keyboard mode is left
		PUBLIC	load_video_driver
load_video_driver	PROC	FAR
		push	bp
		mov	bp,sp
		push	bx
		push	cx
		push	di
		push	ds
		push	dx
		push	es
		push	si
		cld
		mov	si,word ptr [bp+6]
		mov	ds,word ptr [bp+8]
		mov	dx,word ptr [si]
		add	si,4
		push	ds
		push	si
		mov	bx,OFFSET ExecBlock
		mov	ax,ds
		mov	es,ax
		push	cs
		call	near ptr exec_overlay
		cmp	bx,-1
		jz	@@00E6
		mov	ax,ExecBlock
		xor	bx,bx
		mov	word ptr DriverEntry,bx
		mov	word ptr DriverEntry+2,ax
		push	ax
		mov	bx,SEG wait_tick
		push	bx
		mov	bx,OFFSET wait_tick
		push	bx
		call	dword ptr DriverEntry
		pop	cx
		pop	cx
		pop	cx
		mov	word ptr DriverEntry,ax
		shl	bx,1
		shl	bx,1
		shl	bx,1
		shl	bx,1
		push	bx
		push	cx
		xor	bx,bx
		push	bx
		call	far ptr mem_realloc
		add	sp,6
		or	ax,dx
		jz	@@00E6
		mov	word ptr DriverEntry+2,dx
		pop	si
		pop	ds
		lodsw
		mov	bx,ax
		lodsw
		push	ax
		push	bx
		call	dword ptr DriverEntry
		pop	cx
		pop	cx
		jc	@@00E8
		mov	ax,SEG MousePresent
		mov	ds,ax
		cmp	byte ptr MousePresent,0
		jz	@@00E2
		mov	byte ptr InventoryMode,0
@@00E2:
		xor	ax,ax
		jmp	short @@00EB
@@00E6:
		pop	ds
		pop	si
@@00E8:
		mov	ax,0FFFFh
@@00EB:
		pop	si
		pop	es
		pop	dx
		pop	ds
		pop	di
		pop	cx
		pop	bx
		pop	bp
		retf
load_video_driver	ENDP


		.DATA			; Initialised data of the module (DS:666A)
ExecBlock	dw	0, 0		; the load segment, the relocation
DriverEntry	dd	0		; the entry of the driver (its segment:0)
SavedSP		dw	0
SavedSS		dw	0

		END
