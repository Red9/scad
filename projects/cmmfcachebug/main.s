	.text
.Ltext0
	.section	.text._Z8functionPcS_i,"ax",@progbits
	.global	__Z8functionPcS_i
__Z8functionPcS_i
.LFB0
	.file 1 "main.cpp"
	.loc 1 3 0
.LVL0
.LBB2
	.loc 1 4 0
	mov	r7, #0
	.loc 1 3 0
	add	r1, r2
.LVL1
	fcache #(.L5-.L4)
	.compress off

.L4
	jmp	#__LMM_FCACHE_START+(.L2-.L4)
.LVL2
.L3
	xmov	r5,r0 add r5,r7
	.loc 1 7 0
	add	r7, #1
.LVL3
	wrbyte	r6, r5
.LVL4
.L2
	.loc 1 3 0 discriminator 1
	xmov	r6,r1 add r6,r7
	.loc 1 6 0 discriminator 1
	rdbyte	r6, r6
	mov	r5, r6
	sub	r5, #48
	and	r5,#255
	cmp	r5, #9 wz,wc
	IF_BE	jmp	#__LMM_FCACHE_START+(.L3-.L4)
	jmp	__LMM_RET
	.compress default
.L5
.LBE2
	.loc 1 13 0
	mov	r0, r7
.LVL5
	lret
.LFE0
	.data
	.balign	4
.LC0
	.ascii "Hello World! i == %i\0"
	.section	.text.startup.main,"ax",@progbits
	.global	_main
_main
.LFB1
	.loc 1 17 0
	lpushm	#16+15
.LCFI0
	sub	sp, #28
.LCFI1
.LBB3
	.loc 1 19 0
	mvi	r6,#829187400
	leasp r1,#8
	wrlong	r6, r1
	mvi	r6,#7302192
	.loc 1 20 0
	mov	r2, #3
	.loc 1 19 0
	leasp r7,#12
	wrlong	r6, r7
	.loc 1 20 0
	leasp r0,#16
	lcall	#__Z8functionPcS_i
.LVL6
	.loc 1 22 0
	mviw	r7,#.LC0
	wrlong	r7, sp
	leasp r7,#4
	wrlong	r0, r7
	lcall	#_printf
.LVL7
.LBE3
	.loc 1 24 0
	mov	r0, #0
	add	sp, #28
	lpopret	#16+15
.LFE1
	.section	.debug_frame,"",@progbits
.Lframe0
	long	.LECIE0-.LSCIE0
.LSCIE0
	long	0xffffffff
	byte	0x1
	.ascii "\0"
	.uleb128 0x1
	.sleb128 -4
	byte	0x15
	byte	0xc
	.uleb128 0x10
	.uleb128 0
	byte	0x9
	.uleb128 0x15
	.uleb128 0xf
	.balign	4
.LECIE0
.LSFDE0
	long	.LEFDE0-.LASFDE0
.LASFDE0
	long	.Lframe0
	long	.LFB0
	long	.LFE0-.LFB0
	.balign	4
.LEFDE0
.LSFDE2
	long	.LEFDE2-.LASFDE2
.LASFDE2
	long	.Lframe0
	long	.LFB1
	long	.LFE1-.LFB1
	byte	0x4
	long	.LCFI1-.LFB1
	byte	0xe
	.uleb128 0x1c
	.balign	4
.LEFDE2
	.text
.Letext0
	.section	.debug_info,"",@progbits
.Ldebug_info0
	long	0x1c5
	word	0x2
	long	.Ldebug_abbrev0
	byte	0x4
	.uleb128 0x1
	.ascii "GNU C++ 4.6.1\0"
	byte	0x4
	.ascii "main.cpp\0"
	.ascii "/home/clewis/propgcc/projects/cmmfcachebug\0"
	long	0
	long	0
	long	.Ldebug_ranges0+0
	long	.Ldebug_line0
	.uleb128 0x2
	byte	0x4
	byte	0x7
	.ascii "unsigned int\0"
	.uleb128 0x2
	byte	0x1
	byte	0x8
	.ascii "char\0"
	.uleb128 0x2
	byte	0x4
	byte	0x5
	.ascii "int\0"
	.uleb128 0x2
	byte	0x1
	byte	0x8
	.ascii "unsigned char\0"
	.uleb128 0x2
	byte	0x4
	byte	0x5
	.ascii "long int\0"
	.uleb128 0x2
	byte	0x4
	byte	0x7
	.ascii "long unsigned int\0"
	.uleb128 0x3
	byte	0x4
	long	0x6f
	.uleb128 0x2
	byte	0x2
	byte	0x5
	.ascii "short int\0"
	.uleb128 0x2
	byte	0x2
	byte	0x7
	.ascii "short unsigned int\0"
	.uleb128 0x4
	byte	0x1
	.ascii "function\0"
	byte	0x1
	byte	0x3
	.ascii "_Z8functionPcS_i\0"
	long	0x77
	long	.LFB0
	long	.LFE0
	byte	0x2
	byte	0x80
	.sleb128 0
	long	0x153
	.uleb128 0x5
	.ascii "A\0"
	byte	0x1
	byte	0x3
	long	0xb0
	long	.LLST0
	.uleb128 0x5
	.ascii "B\0"
	byte	0x1
	byte	0x3
	long	0xb0
	long	.LLST1
	.uleb128 0x5
	.ascii "stringIndex\0"
	byte	0x1
	byte	0x3
	long	0x77
	long	.LLST2
	.uleb128 0x6
	long	.LBB2
	long	.LBE2
	.uleb128 0x7
	.ascii "i\0"
	byte	0x1
	byte	0x4
	long	0x77
	long	.LLST3
	byte	0
	byte	0
	.uleb128 0x8
	byte	0x1
	.ascii "main\0"
	byte	0x1
	byte	0x10
	long	0x77
	long	.LFB1
	long	.LFE1
	long	.LLST4
	long	0x1ac
	.uleb128 0x6
	long	.LBB3
	long	.LBE3
	.uleb128 0x9
	.ascii "bufferA\0"
	byte	0x1
	byte	0x12
	long	0x1ac
	byte	0x2
	byte	0x91
	.sleb128 -16
	.uleb128 0x9
	.ascii "bufferB\0"
	byte	0x1
	byte	0x13
	long	0x1bc
	byte	0x2
	byte	0x91
	.sleb128 -24
	.uleb128 0x7
	.ascii "i\0"
	byte	0x1
	byte	0x14
	long	0x77
	long	.LLST5
	byte	0
	byte	0
	.uleb128 0xa
	long	0x6f
	long	0x1bc
	.uleb128 0xb
	long	0x5f
	byte	0x9
	byte	0
	.uleb128 0xc
	long	0x6f
	.uleb128 0xb
	long	0x5f
	byte	0x7
	byte	0
	byte	0
	.section	.debug_abbrev,"",@progbits
.Ldebug_abbrev0
	.uleb128 0x1
	.uleb128 0x11
	byte	0x1
	.uleb128 0x25
	.uleb128 0x8
	.uleb128 0x13
	.uleb128 0xb
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0x1b
	.uleb128 0x8
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x52
	.uleb128 0x1
	.uleb128 0x55
	.uleb128 0x6
	.uleb128 0x10
	.uleb128 0x6
	byte	0
	byte	0
	.uleb128 0x2
	.uleb128 0x24
	byte	0
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x3e
	.uleb128 0xb
	.uleb128 0x3
	.uleb128 0x8
	byte	0
	byte	0
	.uleb128 0x3
	.uleb128 0xf
	byte	0
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	byte	0
	byte	0
	.uleb128 0x4
	.uleb128 0x2e
	byte	0x1
	.uleb128 0x3f
	.uleb128 0xc
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x2007
	.uleb128 0x8
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x1
	.uleb128 0x40
	.uleb128 0xa
	.uleb128 0x1
	.uleb128 0x13
	byte	0
	byte	0
	.uleb128 0x5
	.uleb128 0x5
	byte	0
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2
	.uleb128 0x6
	byte	0
	byte	0
	.uleb128 0x6
	.uleb128 0xb
	byte	0x1
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x1
	byte	0
	byte	0
	.uleb128 0x7
	.uleb128 0x34
	byte	0
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2
	.uleb128 0x6
	byte	0
	byte	0
	.uleb128 0x8
	.uleb128 0x2e
	byte	0x1
	.uleb128 0x3f
	.uleb128 0xc
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x1
	.uleb128 0x40
	.uleb128 0x6
	.uleb128 0x1
	.uleb128 0x13
	byte	0
	byte	0
	.uleb128 0x9
	.uleb128 0x34
	byte	0
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2
	.uleb128 0xa
	byte	0
	byte	0
	.uleb128 0xa
	.uleb128 0x1
	byte	0x1
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x1
	.uleb128 0x13
	byte	0
	byte	0
	.uleb128 0xb
	.uleb128 0x21
	byte	0
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2f
	.uleb128 0xb
	byte	0
	byte	0
	.uleb128 0xc
	.uleb128 0x1
	byte	0x1
	.uleb128 0x49
	.uleb128 0x13
	byte	0
	byte	0
	byte	0
	.section	.debug_loc,"",@progbits
.Ldebug_loc0
.LLST0
	long	.LVL0
	long	.LVL5
	word	0x1
	byte	0x50
	long	0
	long	0
.LLST1
	long	.LVL0
	long	.LVL1
	word	0x1
	byte	0x51
	long	0
	long	0
.LLST2
	long	.LVL0
	long	.LVL2
	word	0x1
	byte	0x52
	long	0
	long	0
.LLST3
	long	.LVL0
	long	.LVL2
	word	0x2
	byte	0x30
	byte	0x9f
	long	.LVL2
	long	.LFE0
	word	0x1
	byte	0x57
	long	0
	long	0
.LLST4
	long	.LFB1
	long	.LCFI1
	word	0x2
	byte	0x80
	.sleb128 0
	long	.LCFI1
	long	.LFE1
	word	0x2
	byte	0x80
	.sleb128 28
	long	0
	long	0
.LLST5
	long	.LVL6
	long	.LVL7-1
	word	0x1
	byte	0x50
	long	0
	long	0
	.section	.debug_aranges,"",@progbits
	long	0x24
	word	0x2
	long	.Ldebug_info0
	byte	0x4
	byte	0
	word	0
	word	0
	long	.LFB0
	long	.LFE0-.LFB0
	long	.LFB1
	long	.LFE1-.LFB1
	long	0
	long	0
	.section	.debug_ranges,"",@progbits
.Ldebug_ranges0
	long	.LFB0
	long	.LFE0
	long	.LFB1
	long	.LFE1
	long	0
	long	0
	.section	.debug_line,"",@progbits
.Ldebug_line0
