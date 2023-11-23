.extern DrawSync
.extern FntFlush
.extern FntLoad
.extern FntOpen
.extern FntPrint
.extern PutDispEnv
.extern PutDrawEnv
.extern ResetGraph
.extern SetDefDispEnv
.extern SetDefDrawEnv
.extern SetDispMask
.extern VSync


.macro FlipByteVariable address
	la $t0,\address
	#lb $t1,0($t0)
	#beq $t1,$0,__FlipByteVariable1__
	beq $t0,$0,__FlipByteVariable1__
	__FlipByteVariable0__:
		li $t1,0
	__FlipByteVariable1__:
		li $t1,1
	sb $t1,($t0)
.endm


.macro CallSetDefDispDrawEnv function,address,y
	la $a0,\address
	li $a1,0
	li $a2,\y
	li $a3,320
	li $t0,240
	sw $t0,16($sp)
	jal \function
.endm


.macro SetDisplayBackgroundRgb address,r,g,b
	li $t1,\r
	la $t0,\address+25
	sb $t1,0($t0)

	li $t1,\g
	la $t0,\address+26
	sb $t1,0($t0)

	li $t1,\b
	la $t0,\address+27
	sb $t1,0($t0)

	la $t0,\address+24
	li $t1,1
	sb $t1,0($t0)
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

	SetDisplayBackgroundRgb drawEnvs+0, 32,32,32
	SetDisplayBackgroundRgb drawEnvs+92,32,32,32

	la $a0,dispEnvs+0
	jal PutDispEnv

	la $a0,drawEnvs+0
	jal PutDrawEnv

	li $a0,960
	li $a1,0
	jal FntLoad

	li $a0,32
	li $a1,32
	li $a2,320-32
	li $a3,8*7
	li $t0,0
	sw $t0,16($sp)
	li $t1,280
	sw $t1,20($sp)
	jal FntOpen

	addi $sp,$sp,4*6


MainLoop:
	jal DrawSync
	jal VSync

	la $a0,dispEnvs
	la $t0,displayIndex
	lb $t0,($t0)
	li $t1,20
	mult $t0,$t1
	mflo $t0
	add $a0,$a0,$t0
	jal PutDispEnv

	la $a0,drawEnvs
	la $t0,displayIndex
	lb $t0,($t0)
	li $t1,92
	mult $t0,$t1
	mflo $t0
	add $a0,$a0,$t0
	jal PutDrawEnv
	
	FlipByteVariable displayIndex
	
	la $a0,helloStr
	jal FntPrint
	jal FntFlush

	j MainLoop


.data
	helloStr:
		.asciiz "Hello testin'!"

	displayIndex:
		.byte 0

	# libgpu.h > DISPENV
	dispEnvs:
		.align 2
		.space 2*20

	# libgpu.h > DRAWENV
	drawEnvs:
		.align 2
		.space 2*92
