;
; Eye of the Beholder II - START.EXE
; The procedures of the modules in assembler called from another one: PUBLIC
; in the module that defines them, EXTRN in the others (TASM GLOBAL). Included
; before any segment (so that the calls stay far).
;
		GLOBAL	normalize_dxax:FAR	; 3374:0080
		GLOBAL	mouse_button_key:FAR	; 3295:0BA9
		GLOBAL	cursor_hide:FAR	; 338B:000A
		GLOBAL	cursor_show:FAR	; 338B:00F5
		GLOBAL	get_page_ptr:FAR	; 3269:02AA
		GLOBAL	set_page_ptr:FAR	; 3269:026B
