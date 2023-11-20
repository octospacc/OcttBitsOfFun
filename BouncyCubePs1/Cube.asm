; PSX 'Bare Metal' Cube CPU Demo by krom (Peter Lemon):
; Joypad Control:
; Up, Down: -Y, +Y Translation
; Left, Right: -X, +X Translation
; L1, L2: -Z, +Z Translation
; Triangle, X: -X, +X Rotation
; Circle, Square: -Y, +Y Rotation
; R1, R2: -Z, +Z Rotation
.psx
.create "Cube.bin", 0x80010000

.include "LIB/PSX.INC" ; Include PSX Definitions
.include "LIB/PSX_GPU.INC" ; Include PSX GPU Definitions & Macros
.include "LIB/PSX_INPUT.INC" ; Include PSX Input Definitions & Macros
.include "LIB/3D.INC" ; Include 3D Definitions & Macros

.org 0x80010000 ; Entry Point Of Code

InitJoy PadBuffer ; Initialise Joypads & Setup VSync Wait Routine Using BIOS: Buffer Address

la a0,IO_BASE ; A0 = I/O Port Base Address ($1F80XXXX)

; Setup Screen Mode
WRGP1 GPURESET,0  ; Write GP1 Command Word (Reset GPU)
WRGP1 GPUDISPEN,0 ; Write GP1 Command Word (Enable Display)
WRGP1 GPUDISPM,HRES320+VRES240+BPP15+VNTSC ; Write GP1 Command Word (Set Display Mode: 320x240, 15BPP, NTSC)
WRGP1 GPUDISPH,0xC60260 ; Write GP1 Command Word (Horizontal Display Range 608..3168)
WRGP1 GPUDISPV,0x042018 ; Write GP1 Command Word (Vertical Display Range 24..264)

; Setup Drawing Area
WRGP0 GPUDRAWM,0x000508   ; Write GP0 Command Word (Drawing To Display Area Allowed Bit 10, Texture Page Colors = 15BPP Bit 7..8, Texture Page Y Base = 0 Bit 4, Texture Page X Base = 512 Bit 0..3)
WRGP0 GPUDRAWATL,0x000000 ; Write GP0 Command Word (Set Drawing Area Top Left X1=0, Y1=0)
WRGP0 GPUDRAWABR,0x03BD3F ; Write GP0 Command Word (Set Drawing Area Bottom Right X2=319, Y2=239)
WRGP0 GPUDRAWOFS,0x000000 ; Write GP0 Command Word (Set Drawing Offset X=0, Y=0)

.macro CopyTextureVram, TEXTURE, X,Y
	CopyRectCPU X,Y, 256,256 ; Copy Rectangle (CPU To VRAM): X,Y, Width,Height
	li t0,32767 ; T0 = Data Copy Word Count
	la a1,TEXTURE ; A1 = Texture RAM Offset
	CopyTextureThis:
		lw t1,0(a1) ; T1 = DATA Word
		addiu a1,4  ; A1 += 4 (Delay Slot)
		sw t1,GP0(a0) ; Write GP0 Packet Word
		bnez t0,CopyTextureThis ; IF (T0 != 0) Copy Texture A
		subiu t0,1 ; T0-- (Delay Slot)
.endmacro

.macro CubePositionMoveCheck, joykey,position,branch
	IsJoyDown joykey,PadData ; Is Joypad Digital Button Pressed Down: Input, Input Data Address
	beqz t0,branch
	nop ; Delay Slot
	la a1,position
	lw t0,0(a1)
	nop ; Delay Slot
.endmacro

CopyTextureVram TextureA, 512,0
CopyTextureVram TextureB, 768,0
;CopyTextureVram TextureC, 0,256
CopyTextureVram TextureD, 256,256
CopyTextureVram TextureE, 512,256
CopyTextureVram TextureF, 768,256

ReverseCubeAccelY:
	la a2,YAccel                ; Load cube acceleration
	lw t1,0(a2)                 ; in t1

	nop
	subu t1,$0,t1               ; Reverse acceleration
	sw t1,0(a2)                 ; ...

	la a1,YPos                  ; Load cube Y position
	lw t0,0(a1)                 ; ...

	nop
	add t0,t1                   ; Accelerate cube
	sw t0,0(a1)                 ; Commit cube Y position

	b PRESSEND
;ReverseCubeAccelY

Refresh:
	WaitVSync PadBuffer,PadData ; Wait For Vertical Retrace Period & Store XOR Pad Data: Buffer Address, Data Address
	FillRectVRAM 0xEEDDFF, 0,0, 320,240 ; Fill Rectangle In VRAM: Color, X,Y, Width,Height
	XYZPos XPos,YPos,ZPos ; Object X,Y,Z Translation: X,Y,Z
	XYZRotCalc XRot,YRot,ZRot,SinCos256 ; XYZ Rotation Calculation: X Rotation, Y Rotation, Z Rotation, Matrix Sin & Cos Pre-Calculated Table

; Move cube towards the left
	la a1,XPos  ; Load cube X position
	lw t0,0(a1) ; 
	nop         ; Chill
	subiu t0,32 ; Move cube left
	sw t0,0(a1) ; Store position

; Gravity pulling cube down, its force remaking it jump
	la a1,YPos                  ; Load cube Y position
	lw t0,0(a1)                 ; ...

	li t2,+8192-1024            ; Set compare value of ... floor Y
	li t3,-8192-1024            ;                      ... roof Y

	;beq t0,t2,ReverseCubeAccelY ; [If cube touches    ... floor
	;nop
	;beq t0,t3,ReverseCubeAccelY ;  reverse direction] ... roof

	slt t4,t0,t2                ; if cubeY >= floorY
	beq t4,$0,ReverseCubeAccelY ; then reverse it

	la a2,YAccel                ; Load cube acceleration
	lw t1,0(a2)                 ; ...

	nop
	addi t1,16                  ; Get acceleration stronger
	sw t1,0(a2)                 ; Then store it

	nop
	add t0,t1                   ; Accelerate cube
	sw t0,0(a1)                 ; Commit cube Y position

; PRESSUP:
  ; CubePositionMoveCheck JOY_UP,YPos,PRESSDOWN
  ; subiu t0,256 ; Y Position--
  ; sw t0,0(a1)
; PRESSDOWN:
  ; CubePositionMoveCheck JOY_DOWN,YPos,PRESSLEFT
  ; addiu t0,256 ; Y Position++
  ; sw t0,0(a1)
; PRESSLEFT:
  ; CubePositionMoveCheck JOY_LEFT,XPos,PRESSRIGHT
  ; subiu t0,256 ; X Position--
  ; sw t0,0(a1)
; PRESSRIGHT:
  ; CubePositionMoveCheck JOY_RIGHT,XPos,PRESSL1
  ; addiu t0,256 ; X Position++
  ; sw t0,0(a1)
; PRESSL1:
  ; IsJoyDown JOY_L1,PadData ; Is Joypad Digital Button Pressed Down: Input, Input Data Address
  ; beqz t0,PRESSL2
  ; nop ; Delay Slot
  ; la a1,ZPos ; Z Position--
  ; lw t0,0(a1)
  ; li t1,10240
  ; beq t0,t1,PRESSL2
  ; nop ; Delay Slot
  ; subiu t0,256
  ; sw t0,0(a1)
; PRESSL2:
  ; IsJoyDown JOY_L2,PadData ; Is Joypad Digital Button Pressed Down: Input, Input Data Address
  ; beqz t0,PRESST
  ; nop ; Delay Slot
  ; la a1,ZPos ; Z Position++
  ; lw t0,0(a1)
  ; li t1,25600
  ; beq t0,t1,PRESST
  ; nop ; Delay Slot
  ; addiu t0,256
  ; sw t0,0(a1)
; PRESST:
  ; IsJoyDown JOY_T,PadData ; Is Joypad Digital Button Pressed Down: Input, Input Data Address
  ; beqz t0,PRESSX
  ; nop ; Delay Slot
  ; la a1,XRot ; X Rotation--
  ; lw t0,0(a1)
  ; nop ; Delay Slot
  ; subiu t0,1
  ; andi t0,0xFF
  ; sw t0,0(a1)
; PRESSX:
  ; IsJoyDown JOY_X,PadData ; Is Joypad Digital Button Pressed Down: Input, Input Data Address
  ; beqz t0,PRESSC
  ; nop ; Delay Slot
  ; la a1,XRot ; X Rotation++
  ; lw t0,0(a1)
  ; nop ; Delay Slot
  ; addiu t0,1
  ; andi t0,0xFF
  ; sw t0,0(a1)
; PRESSC:
  ; IsJoyDown JOY_C,PadData ; Is Joypad Digital Button Pressed Down: Input, Input Data Address
  ; beqz t0,PRESSS
  ; nop ; Delay Slot
  ; la a1,YRot ; Y Rotation--
  ; lw t0,0(a1)
  ; nop ; Delay Slot
  ; subiu t0,1
  ; andi t0,0xFF
  ; sw t0,0(a1)
; PRESSS:
  ; IsJoyDown JOY_S,PadData ; Is Joypad Digital Button Pressed Down: Input, Input Data Address
  ; beqz t0,PRESSR1
  ; nop ; Delay Slot
  ; la a1,YRot ; Y Rotation++
  ; lw t0,0(a1)
  ; nop ; Delay Slot
  ; addiu t0,1
  ; andi t0,0xFF
  ; sw t0,0(a1)
; PRESSR1:
  ; IsJoyDown JOY_R1,PadData ; Is Joypad Digital Button Pressed Down: Input, Input Data Address
  ; beqz t0,PRESSR2
  ; nop ; Delay Slot
  ; la a1,ZRot ; Z Rotation--
  ; lw t0,0(a1)
  ; nop ; Delay Slot
  ; subiu t0,1
  ; andi t0,0xFF
  ; sw t0,0(a1)
; PRESSR2:
  ; IsJoyDown JOY_R2,PadData ; Is Joypad Digital Button Pressed Down: Input, Input Data Address
  ; beqz t0,PRESSEND
  ; nop ; Delay Slot
  ; la a1,ZRot ; Z Rotation++
  ; lw t0,0(a1)
  ; nop ; Delay Slot
  ; addiu t0,1
  ; andi t0,0xFF
  ; sw t0,0(a1)
PRESSEND:
  ShadeTexQuadCullBackZSort ShadeTexCubeQuad,ShadeTexCubeQuadEnd,PolySort ; Shaded Texture Quad Back Face Cull Z Sort: Object Start Address, Object End Address, Sort Address
  b Refresh
  nop ; Delay Slot

PadBuffer:
  dw 0 ; Pad Buffer (Automatically Stored Every Frame)
PadData:
  dw 0 ; Pad Data (Read From VSync Routine)

XPos:
  dw 16384 ; X Position Word
YPos:
  dw 512 ; Y Position Word
ZPos:
  dw 25600 ; Z Position Word

YAccel:
	dw -64

XRot:
  dw 0 ; X Rotate Word (0..255)
YRot:
  dw 0 ; Y Rotate Word (0..255)
ZRot:
  dw 0 ; Z Rotate Word (0..255)

Matrix3D: ; 3D Matrix: Set To Default Identity Matrix (All Numbers Multiplied By 256 For 24.8 Fixed Point Format)
  dw 256, 0, 0, 0 ; X = 1.0, 0.0, 0.0, X Translation = 0.0
  dw 0, 256, 0, 0 ; 0.0, Y = 1.0, 0.0, Y Translation = 0.0
  dw 0, 0, 256, 0 ; 0.0, 0.0, Z = 1.0, Z Translation = 0.0

; Matrix Sin & Cos Pre-Calculated Table
  .include "sincos256.asm" ; Matrix Sin & Cos Pre-Calculated Table (256 Rotations)

; Object Data
  .include "objects.asm" ; Object Data

TextureA:
  .incbin "GFX/A.bin" ; Include 256x256 15BPP Texture Data (131072 Bytes)
TextureB:
  .incbin "GFX/B.bin" ; Include 256x256 15BPP Texture Data (131072 Bytes)
;TextureC:
;  .incbin "GFX/C.bin" ; Include 256x256 15BPP Texture Data (131072 Bytes)
TextureD:
  .incbin "GFX/D.bin" ; Include 256x256 15BPP Texture Data (131072 Bytes)
TextureE:
  .incbin "GFX/E.bin" ; Include 256x256 15BPP Texture Data (131072 Bytes)
TextureF:
  .incbin "GFX/F.bin" ; Include 256x256 15BPP Texture Data (131072 Bytes)

PolySort: ; Polygon Sorting Area

.close