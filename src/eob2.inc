;
; Eye of the Beholder II - START.EXE
; The DGROUP variables shared by the modules written in assembler: PUBLIC
; in the module that defines them, EXTRN in the others (TASM GLOBAL).
;
		GLOBAL	ExeName:DWORD	; 1CC8 main.c "START.EXE"
		GLOBAL	VideoMode:WORD	; 628A 
		GLOBAL	MouseStepX:WORD	; 628C the mouse moves by steps (0: free)
		GLOBAL	MouseStepY:WORD	; 628E 
		GLOBAL	MouseOrgX:WORD	; 6290 
		GLOBAL	MouseOrgY:WORD	; 6292 
		GLOBAL	FileErrorHandler:DWORD	; 6294 
		GLOBAL	MouseTestX:WORD	; 651E mouse_in_rect: the cursor read once
		GLOBAL	MouseTestY:WORD	; 6520 
		GLOBAL	DriverTable:DWORD	; 6522 the table given to the video driver
		GLOBAL	Drv_6542:DWORD	; 6542 the video driver (MGA.OVL) entries
		GLOBAL	Drv_6546:DWORD	; 6546 
		GLOBAL	Drv_654A:DWORD	; 654A 
		GLOBAL	Drv_654E:DWORD	; 654E 
		GLOBAL	Drv_6552:DWORD	; 6552 
		GLOBAL	Drv_6556:DWORD	; 6556 
		GLOBAL	Drv_655A:DWORD	; 655A 
		GLOBAL	Drv_655E:DWORD	; 655E 
		GLOBAL	Drv_6562:DWORD	; 6562 
		GLOBAL	Drv_6566:DWORD	; 6566 
		GLOBAL	Drv_656A:DWORD	; 656A 
		GLOBAL	Drv_656E:DWORD	; 656E 
		GLOBAL	Drv_6572:DWORD	; 6572 
		GLOBAL	Drv_6576:DWORD	; 6576 
		GLOBAL	Drv_657A:DWORD	; 657A 
		GLOBAL	Drv_657E:DWORD	; 657E 
		GLOBAL	Drv_6582:DWORD	; 6582 
		GLOBAL	Drv_6586:DWORD	; 6586 
		GLOBAL	Drv_658A:DWORD	; 658A 
		GLOBAL	Drv_658E:DWORD	; 658E 
		GLOBAL	Drv_6592:DWORD	; 6592 
		GLOBAL	Drv_6596:DWORD	; 6596 
		GLOBAL	Drv_659A:DWORD	; 659A 
		GLOBAL	Drv_659E:DWORD	; 659E 
		GLOBAL	Drv_65A2:DWORD	; 65A2 
		GLOBAL	Drv_65A6:DWORD	; 65A6 
		GLOBAL	Drv_65AA:DWORD	; 65AA 
		GLOBAL	Drv_65AE:DWORD	; 65AE 
		GLOBAL	Drv_65B2:DWORD	; 65B2 
		GLOBAL	Drv_65B6:DWORD	; 65B6 
		GLOBAL	Drv_65BA:DWORD	; 65BA 
		GLOBAL	Drv_65BE:DWORD	; 65BE 
		GLOBAL	Drv_65C2:DWORD	; 65C2 
		GLOBAL	Drv_65C6:DWORD	; 65C6 
		GLOBAL	Drv_65CA:DWORD	; 65CA 
		GLOBAL	Drv_65CE:DWORD	; 65CE 
		GLOBAL	Drv_65D2:DWORD	; 65D2 
		GLOBAL	Drv_65D6:DWORD	; 65D6 
		GLOBAL	Drv_65DA:DWORD	; 65DA 
		GLOBAL	Drv_65DE:DWORD	; 65DE 
		GLOBAL	Drv_65E2:DWORD	; 65E2 
		GLOBAL	Drv_65E6:DWORD	; 65E6 
		GLOBAL	Drv_65EA:DWORD	; 65EA 
		GLOBAL	Drv_65EE:DWORD	; 65EE 
		GLOBAL	Drv_65F2:DWORD	; 65F2 
		GLOBAL	Var_65F6:WORD	; 65F6 the 3D view (2A68)
		GLOBAL	Var_65F8:WORD	; 65F8 
		GLOBAL	SoundEnabled:WORD	; 65FA 
		GLOBAL	Var_65FC:WORD	; 65FC the music on
		GLOBAL	Var_65FE:WORD	; 65FE 
		GLOBAL	Var_6600:BYTE	; 6600 the input: 0 normal, 1 recorded, 2 played back (?)
		GLOBAL	Var_6601:WORD	; 6601 
		GLOBAL	Var_6603:WORD	; 6603 
		GLOBAL	Var_6605:WORD	; 6605 
		GLOBAL	Var_6607:WORD	; 6607 
		GLOBAL	Var_6609:WORD	; 6609 
		GLOBAL	Var_660B:WORD	; 660B 
		GLOBAL	CursorBufSize:WORD	; 660E the size of the buffer under the cursor
		GLOBAL	MouseBusy:WORD	; 6610 the cursor is being changed: the handler waits
		GLOBAL	MouseButtons:WORD	; 6612 the buttons down
		GLOBAL	Var_6614:WORD	; 6614 
		GLOBAL	Var_6616:WORD	; 6616 the buttons that changed
		GLOBAL	Var_6618:WORD	; 6618 
		GLOBAL	ClickDown:WORD	; 661A the mouse position (x, y)
		GLOBAL	MouseRawX:WORD	; 661E 
		GLOBAL	MouseRawY:WORD	; 6620 
		GLOBAL	ClickUp:WORD	; 6622 (x, y)
		GLOBAL	MouseXDouble:WORD	; 6626 the driver counts x in 0-639: halved
		GLOBAL	MouseHidden:WORD	; 6628 hide count of the cursor
		GLOBAL	MouseMinX:WORD	; 662A 
		GLOBAL	MouseMaxX:WORD	; 662C 
		GLOBAL	MouseMinY:WORD	; 662E 
		GLOBAL	MouseMaxY:WORD	; 6630 
		GLOBAL	CursorH:WORD	; 6632 the cursor shape: height, width + 1
		GLOBAL	CursorW:WORD	; 6634 
		GLOBAL	CursorHotX:WORD	; 6636 
		GLOBAL	CursorHotY:WORD	; 6638 
		GLOBAL	CursorDrawX:WORD	; 663A where the cursor is drawn
		GLOBAL	CursorDrawY:WORD	; 663C 
		GLOBAL	MouseFlags:WORD	; 663E 8000h: hidden inside HideRect
		GLOBAL	HideX1:WORD	; 6640 
		GLOBAL	HideY1:WORD	; 6642 
		GLOBAL	HideX2:WORD	; 6644 
		GLOBAL	HideY2:WORD	; 6646 
		GLOBAL	CursorBack:DWORD	; 6648 the screen under the cursor
		GLOBAL	CursorShape:DWORD	; 664C 
		GLOBAL	PagePtr:DWORD	; 6650 the descriptor of the page shown (16 bytes)
		GLOBAL	MousePresent:BYTE	; 6654 
		GLOBAL	InventoryMode:BYTE	; 6655 the keyboard moves the selection (no cursor)
		GLOBAL	Var_6656:WORD	; 6656 the cursor rectangle
		GLOBAL	Var_6658:WORD	; 6658 
		GLOBAL	Var_665A:WORD	; 665A 
		GLOBAL	Var_665C:WORD	; 665C 
		GLOBAL	Var_665E:WORD	; 665E 
		GLOBAL	Var_6660:WORD	; 6660 
		GLOBAL	Var_6662:WORD	; 6662 
		GLOBAL	Var_6664:WORD	; 6664 
		GLOBAL	Var_6666:WORD	; 6666 
		GLOBAL	Var_6668:WORD	; 6668
		GLOBAL	Ticks16:WORD	; 667A clock.asm
		GLOBAL	ViewTiles:WORD:660	; 987E the tiles of the 3D view [2][15][22]
		GLOBAL	WorkPage:DWORD	; DF3A page 3
		GLOBAL	CurDrive:BYTE	; F002 the drive at start (BSS)
		GLOBAL	LastDrive:BYTE	; F042 the last drive (BSS)
		GLOBAL	ExePath:BYTE:80	; F108 the full name of the program (BSS)
		GLOBAL	OldInt3F:DWORD	; F1C0 the overlay interrupt before the game (BSS)
