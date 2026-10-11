;
; Eye of the Beholder II - START.EXE
; The clocks: a handler on INT 1Ch (the BIOS tick, 18.2 Hz) counts the
; ticks in DGROUP; module 23E1 (Ghidra 33E1), written in assembler.
;
; Assembled with
;     tasm /ml clock.asm
; [asm] = identical to START.EXE byte for byte (tools/asm.py).
;
		.MODEL	LARGE, C
		LOCALS

		.DATA			; Initialised data of the module (DS:667A)
		PUBLIC	Ticks16, TimerTicks, Clock6680, GameTicks, Clock6688, Countdown, ClockEnable
Ticks16		dw	0		; 667A every tick
TimerTicks	dd	0		; 667C every tick
Clock6680	dd	0		; 6680 when ClockEnable bit 0
GameTicks	dd	0		; 6684 when ClockEnable bit 1 (stops in the menus)
Clock6688	dd	0		; 6688 every tick
Countdown	dd	0		; 668C down to 0
ClockEnable	dw	0		; 6690

		.CODE
OldTick		dd	0		; 33E1:0000 the handler before ours (the code segment starts at 33E1:0000)

; 33E1:0004 Clock6688, read with the interrupts off (unused)
		PUBLIC	read_clock6688
read_clock6688	PROC	FAR
		cli
		mov	ax, word ptr Clock6688
		mov	dx, word ptr Clock6688+2
		sti
		ret
read_clock6688	ENDP

; 33E1:000E Hooks tick_handler on INT 1Ch
		PUBLIC	tick_install
tick_install	PROC	FAR
		push	bp
		mov	bp, sp
		push	ax
		push	bx
		push	ds
		push	es
		mov	ah, 35h
		mov	al, 1Ch
		int	21h
		mov	word ptr cs:OldTick+2, es
		mov	word ptr cs:OldTick, bx
		mov	ah, 25h
		mov	al, 1Ch
		push	cs
		pop	ds
		mov	dx, OFFSET tick_handler
		int	21h
		pop	es
		pop	ds
		pop	bx
		pop	ax
		pop	bp
		ret
tick_install	ENDP

; 33E1:0036 Puts the old handler back
		PUBLIC	tick_remove
tick_remove	PROC	FAR
		push	bp
		mov	bp, sp
		push	ax
		push	ds
		push	dx
		push	es
		mov	ah, 25h
		mov	al, 1Ch
		mov	ds, word ptr cs:OldTick+2
		mov	dx, word ptr cs:OldTick
		int	21h
		pop	es
		pop	dx
		pop	ds
		pop	ax
		pop	bp
		ret
tick_remove	ENDP

; 33E1:0053 The handler: counts, then chains to the old one
tick_handler	PROC	FAR
		push	ax
		push	ds
		mov	ax, DGROUP
		mov	ds, ax
		mov	ax, ClockEnable
		shr	ax, 1
		jnc	@@no6680
		add	word ptr Clock6680, 1
		adc	word ptr Clock6680+2, 0
@@no6680:	shr	ax, 1
		jnc	@@nogame
		add	word ptr GameTicks, 1
		adc	word ptr GameTicks+2, 0
@@nogame:	inc	Ticks16
		add	word ptr TimerTicks, 1
		adc	word ptr TimerTicks+2, 0
		add	word ptr Clock6688, 1
		adc	word ptr Clock6688+2, 0
		mov	ax, word ptr Countdown
		or	ax, word ptr Countdown+2
		jz	@@counted
		sub	word ptr Countdown, 1
		sbb	word ptr Countdown+2, 0
@@counted:	pop	ds
		pop	ax
		cmp	word ptr cs:OldTick+2, 0
		jz	@@none
		jmp	dword ptr cs:OldTick
@@none:		iret
tick_handler	ENDP

		END
