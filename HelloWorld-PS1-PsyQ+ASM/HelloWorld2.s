.extern c_init
.extern c_display

.extern DrawSync
.extern FntFlush
.extern FntPrint
.extern PutDispEnv
.extern PutDrawEnv
.extern ResetGraph
.extern SetDefDispEnv
.extern SetDefDrawEnv
.extern SetDispMask
.extern SetVideoMode
.extern VSync


.macro CallSetDefDispDrawEnv function,address,y
	la $a0,\address
	li $a1,0
	li $a2,\y
	li $a3,320
	li $t0,240
	sw $t0,16($sp)
	jal \function
.endm


.macro SetBackgroundRgb address,r,g,b
	# set params to items 26 27 28 of \address
.endm


.global main
main:
	addi $sp,$sp,-4*6

	lw $a0,0($0)
	jal ResetGraph
	
	CallSetDefDispDrawEnv SetDefDispEnv,dispEnvs+0, 0
	CallSetDefDispDrawEnv SetDefDispEnv,dispEnvs+20,240
	CallSetDefDispDrawEnv SetDefDrawEnv,drawEnvs+0, 240
	CallSetDefDispDrawEnv SetDefDrawEnv,drawEnvs+92,0

	SetBackgroundRgb drawEnvs+0, 32,32,32
	SetBackgroundRgb drawEnvs+92,32,32,32

	jal c_init
	
	addi $sp,$sp,4*6


MainLoop:
	#addi $sp,$sp,-4*5

	jal DrawSync

	jal VSync

	jal c_display
	
	la $a0,helloStr
	jal FntPrint
	# li $a0,70
	# li $a1,71
	# li $a2,72
	# li $a3,73
	# li $t0,120
	# sw $t0,16($sp)
	# jal Print12345

	jal FntFlush

	j MainLoop

	#addi $sp,$sp,4*5


.data
	helloStr:
		.asciiz "Hello testin'!"

	.global displayIndex
	displayIndex:
		.byte 0

	# libgpu.h > DISPENV
	.global disp
	disp:
	dispEnvs:
		.align 2
		.space 40

	# libgpu.h > DRAWENV
	.global draw
	draw:
	drawEnvs:
		.align 2
		.space 184
