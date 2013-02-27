GAS LISTING /tmp/ccctTnIs.s 			page 1


   1              	# 1 "../toggle.S"
   1              	#
   1              	...
   0              	
   0              	
   2              	# PASM toggle demo
   3              	#
   4              	# Copyright (c) 2011 Parallax, Inc.
   5              	# MIT Licensed.
   6              	#
   7              	# +--------------------------------------------------------------------
   8              	# ¦  TERMS OF USE: MIT License
   9              	# +--------------------------------------------------------------------
  10              	# Permission is hereby granted, free of charge, to any person obtaining
  11              	# a copy of this software and associated documentation files
  12              	# (the "Software"), to deal in the Software without restriction,
  13              	# including without limitation the rights to use, copy, modify, merge,
  14              	# publish, distribute, sublicense, and/or sell copies of the Software,
  15              	# and to permit persons to whom the Software is furnished to do so,
  16              	# subject to the following conditions:
  17              	#
  18              	# The above copyright notice and this permission notice shall be
  19              	# included in all copies or substantial portions of the Software.
  20              	#
  21              	# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
  22              	# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
  23              	# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
  24              	# IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
  25              	# CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
  26              	# TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
  27              	# SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
  28              	# +--------------------------------------------------------------------
  29              	
  30              			''
  31              			'' Any section starting or ending in ".cog" is special to
  32              			'' the linker: it knows that these need to be relocated to
  33              			'' COG memory but loaded into hub. It also creates
  34              			'' special names like _load_start_cogtoggle to tell where
  35              			'' the code is in memory (so the main code knows how to
  36              			'' load it into the cog).
  37              			''
  38              			'' The "ax" flag says the section will contain executable
  39              			'' code. The assembler should figure this out on its own,
  40              			'' but it never hurts to be explicit!
  41              			''
  42              			.section .cogtoggle, "ax"
  43              			.cog_ram
  44              		
  45              			'' load the pins and wait delay from the C
  46              			'' variables "pins" and "wait_time" respectively
  47              			'' note that C variables have an _ prepended
  48              			'' also note that LMM code tends to be big,
  49              			'' so a direct rdlong waitdelay, #_wait_time is
  50              			'' probably not going to work (_wait_time is bigger than 511)
  51              	
  52 0000 0000BC08 	                rdlong  waitdelay, wait_addr ' read from hub to get
  53 0004 0000BC08 	                rdlong  pins, pins_addr      ' the user's clkfreq delay and pins
GAS LISTING /tmp/ccctTnIs.s 			page 2


  54 0008 0000BCA0 	                mov     dira, pins          ' set pins to output
  55 000c 0000BCA0 			mov	outa, pins
  56              		
  57 0010 0000BCA0 	                mov     nextcnt, waitdelay
  58 0014 0000BC80 	                add     nextcnt, cnt        ' best to add cnt last
  59              	.loop
  60 0018 0000BC6C 	                xor     outa, pins          ' toggle pins
  61 001c 0000BCF8 	                waitcnt nextcnt, waitdelay  ' wait for half second
  62 0020 0000BC08 			rdlong  waitdelay, wait_addr ' update wait delay
  63 0024 00007C5C 	                jmp     #.loop
  64              	
  65 0028 00000000 	pins            long    0
  66 002c 00000000 	waitdelay       long    0                   ' read from hub to int
  67 0030 00000000 	nextcnt         long    0
  68              	
  69              			'' addresses of C variables
  70 0034 00000000 	wait_addr	long	_wait_time
  71 0038 00000000 	pins_addr	long	_pins
GAS LISTING /tmp/ccctTnIs.s 			page 1


   1              	' GNU C++ (propellergcc_v0_3_5_1758) version 4.6.1 (propeller-elf)
   2              	'	compiled by GNU C version 4.6.3, GMP version 5.0.2, MPFR version 3.0.1, MPC version 0.9
   3              	' GGC heuristics: --param ggc-min-expand=100 --param ggc-min-heapsize=131072
   4              	' options passed:  ../toggle.cpp -g -fverbose-asm
   5              	' options enabled:  -fauto-inc-dec -fbranch-count-reg -fcommon
   6              	' -fdelete-null-pointer-checks -fearly-inlining
   7              	' -feliminate-unused-debug-types -fexceptions -fgcse-lm -fident
   8              	' -finline-functions-called-once -fira-share-save-slots
   9              	' -fira-share-spill-slots -fivopts -fkeep-static-consts
  10              	' -fleading-underscore -fmath-errno -fmerge-debug-strings
  11              	' -fmove-loop-invariants -fpeephole -fprefetch-loop-arrays
  12              	' -freg-struct-return -fsched-critical-path-heuristic
  13              	' -fsched-dep-count-heuristic -fsched-group-heuristic -fsched-interblock
  14              	' -fsched-last-insn-heuristic -fsched-rank-heuristic -fsched-spec
  15              	' -fsched-spec-insn-heuristic -fsched-stalled-insns-dep -fshow-column
  16              	' -fsigned-zeros -fsplit-ivs-in-unroller -fstrict-volatile-bitfields
  17              	' -ftrapping-math -ftree-cselim -ftree-forwprop -ftree-loop-if-convert
  18              	' -ftree-loop-im -ftree-loop-ivcanon -ftree-loop-optimize
  19              	' -ftree-parallelize-loops= -ftree-phiprop -ftree-pta -ftree-reassoc
  20              	' -ftree-scev-cprop -ftree-slp-vectorize -ftree-vect-loop-version
  21              	' -funit-at-a-time -fverbose-asm -fzero-initialized-in-bss -m64bit-doubles
  22              	' -mlmm
  23              	
  24              		.text
  25              	.Ltext0
  26              	' Compiler executable checksum: f9cb76eb85984b0aca1daa7095a7bbb0
  27              	
  28              		.balign	4
  29              		.global	__Z9start_cogv
  30              	__Z9start_cogv
  31              	.LFB0
  32              		.file 1 "../toggle.cpp"
   1:../toggle.cpp **** /**
   2:../toggle.cpp ****  * @file toggle.c
   3:../toggle.cpp ****  * This program demonstrates starting another COG running
   4:../toggle.cpp ****  * GAS code.
   5:../toggle.cpp ****  * The cog makes all IO except 30/31 toggle.
   6:../toggle.cpp ****  *
   7:../toggle.cpp ****  * Copyright (c) 2011, Parallax, Inc.
   8:../toggle.cpp ****  * MIT Licensed
   9:../toggle.cpp ****  */
  10:../toggle.cpp **** 
  11:../toggle.cpp **** #include <stdio.h>
  12:../toggle.cpp **** #include <propeller.h>
  13:../toggle.cpp **** #include <stdlib.h>
  14:../toggle.cpp **** #include <string.h>
  15:../toggle.cpp **** #include <unistd.h>
  16:../toggle.cpp **** 
  17:../toggle.cpp **** /*
  18:../toggle.cpp ****  * function to start up a new cog running the toggle
  19:../toggle.cpp ****  * code (which we've placed in the .cogtoggle section)
  20:../toggle.cpp ****  */
  21:../toggle.cpp **** void start_cog(void)
  22:../toggle.cpp **** {
  33              		.loc 1 22 0
  34 0000 0400FC84 		sub	sp, #4	',
  35              	.LCFI0
GAS LISTING /tmp/ccctTnIs.s 			page 2


  36 0004 00003C08 		wrlong	r14, sp	',
  37              	.LCFI1
  38 0008 0000BCA0 		mov	r14, sp	',
  39              	.LCFI2
  40              	.LBB2
  23:../toggle.cpp ****     extern unsigned int _load_start_cogtoggle[];
  24:../toggle.cpp **** 
  25:../toggle.cpp ****     /* now start the kernel */
  26:../toggle.cpp **** #if defined(__PROPELLER_XMMC__) || defined(__PROPELLER_XMM__)
  27:../toggle.cpp ****     unsigned int *buffer;
  28:../toggle.cpp **** 
  29:../toggle.cpp ****     // allocate a buffer in hub memory for the cog to start from
  30:../toggle.cpp ****     buffer = __builtin_alloca(2048);
  31:../toggle.cpp ****     memcpy(buffer, _load_start_cogtoggle, 2048);
  32:../toggle.cpp ****     cognew(buffer, 0);
  33:../toggle.cpp **** #else
  34:../toggle.cpp ****     cognew(_load_start_cogtoggle, 0);
  41              		.loc 1 34 0
  42 000c 00007C5C 		mvi	r7,#__load_start_cogtoggle	' _load_start_cogtoggle.6,
  42      00000000 
  43 0014 0000BCA0 		mov	r6, r7	' D.2375, _load_start_cogtoggle.6
  44 0018 0200FC2C 		shl	r6, #2	' D.2375,
  45 001c 00007C5C 		mvi	r7,#262128	' tmp31,
  45      F0FF0300 
  46 0024 0000BC60 		and	r7, r6	' D.2376, D.2375
  47 0028 0800FC68 		or	r7, #8	' D.2377,
  48 002c 0200FC0D 		coginit	r7 wc,wr	' tmp32
  49 0030 0100F0A4 		IF_B  neg	r7,#1	', tmp32,
  50              	.LBE2
  35:../toggle.cpp **** #endif
  36:../toggle.cpp **** }
  51              		.loc 1 36 0
  52 0034 0000BCA0 		mov	sp, r14	',
  53 0038 0000BC08 		rdlong	r14, sp	',
  54 003c 0400FC80 		add	sp, #4	',
  55 0040 0000BCA0 		mov	pc,lr	'
  56              	.LFE0
  57              		.global	_wait_time
  58              		.section	.hub,"aw",@progbits
  59              		.balign	4
  60              	_wait_time
  61 0000 00000000 		.zero	4
  62              		.global	_pins
  63              		.balign	4
  64              	_pins
  65 0004 00000000 		.zero	4
  66              		.data
  67              		.balign	4
  68              	.LC0
  69 0000 68656C6C 		.ascii "hello, world!\0"
  69      6F2C2077 
  69      6F726C64 
  69      2100
  70 000e 0000     		.balign	4
  71              	.LC1
  72 0010 746F6767 		.ascii "toggle cog has started\0"
  72      6C652063 
GAS LISTING /tmp/ccctTnIs.s 			page 3


  72      6F672068 
  72      61732073 
  72      74617274 
  73 0027 00       		.text
  74              		.balign	4
  75              		.global	_main
  76              	_main
  77              	.LFB1
  37:../toggle.cpp **** 
  38:../toggle.cpp **** /* variables that we share between cogs */
  39:../toggle.cpp **** HUBDATA volatile unsigned int wait_time;
  40:../toggle.cpp **** HUBDATA volatile unsigned int pins;
  41:../toggle.cpp **** 
  42:../toggle.cpp **** 
  43:../toggle.cpp **** /*
  44:../toggle.cpp ****  * main code
  45:../toggle.cpp ****  * This is the code running in the LMM cog (cog 0).
  46:../toggle.cpp ****  * It launches another cog to actually run the 
  47:../toggle.cpp ****  * toggling code
  48:../toggle.cpp ****  */
  49:../toggle.cpp **** #define MIN_GAP 400000
  50:../toggle.cpp **** 
  51:../toggle.cpp **** int main (int argc,  char* argv[])
  52:../toggle.cpp **** {
  78              		.loc 1 52 0
  79 0044 2E00FCA0 		mov	__TMP0,#(2<<4)+14
  80 0048 0000FC5C 		call	#__LMM_PUSHM
  81              	.LCFI3
  82 004c 0000BCA0 		mov	r14, sp	',
  83              	.LCFI4
  84 0050 0800FC84 		sub	sp, #8	',
  85              	.LCFI5
  86 0054 0000BCA0 		mov	r7, r14	' tmp48,
  87 0058 0800FC84 		sub	r7, #8	' tmp48,
  88 005c 00003C08 		wrlong	r0, r7	' argc, argc
  89 0060 0000BCA0 		mov	r7, r14	' tmp35,
  90 0064 0400FC84 		sub	r7, #4	' tmp35,
  91 0068 00003C08 		wrlong	r1, r7	' argv, argv
  92              	.LBB3
  53:../toggle.cpp ****     int n;
  54:../toggle.cpp ****     int result;
  55:../toggle.cpp ****     unsigned int startTime;
  56:../toggle.cpp ****     unsigned int endTime;
  57:../toggle.cpp ****     unsigned int executionTime;
  58:../toggle.cpp ****     unsigned int rawTime;
  59:../toggle.cpp **** 
  60:../toggle.cpp ****     printf("hello, world!\n");
  93              		.loc 1 60 0
  94 006c 00007C5C 		mvi	r0,#.LC0	',
  94      00000000 
  95 0074 00007C5C 		lcall	#_puts	'
  95      00000000 
  61:../toggle.cpp **** 
  62:../toggle.cpp ****     /* set up the parameters for the other cog */
  63:../toggle.cpp ****     /* note that we have to avoid any pins being used by the XMM
  64:../toggle.cpp ****        interpreter, if we are in XMM mode!
  65:../toggle.cpp ****     */
GAS LISTING /tmp/ccctTnIs.s 			page 4


  66:../toggle.cpp **** #if defined(__PROPELLER_XMM__) || defined(__PROPELLER_XMMC__)
  67:../toggle.cpp ****     pins = (1<<15); /* just the LED on the C3 board */
  68:../toggle.cpp **** #else
  69:../toggle.cpp ****     pins = 0x3fffffff;
  96              		.loc 1 69 0
  97 007c 00007C5C 		mvi	r7,#_pins	' tmp36,
  97      00000000 
  98 0084 00007C5C 		mvi	r6,#1073741823	' tmp37,
  98      FFFFFF3F 
  99 008c 00003C08 		wrlong	r6, r7	' tmp37, pins
  70:../toggle.cpp **** #endif
  71:../toggle.cpp ****     wait_time = _clkfreq;  /* start by waiting for 1 second */
 100              		.loc 1 71 0
 101 0090 00007C5C 		mvi	r7,#__clkfreq	' tmp38,
 101      00000000 
 102 0098 0000BC08 		rdlong	r6, r7	' _clkfreq.0, _clkfreq
 103 009c 00007C5C 		mvi	r7,#_wait_time	' tmp39,
 103      00000000 
 104 00a4 00003C08 		wrlong	r6, r7	' _clkfreq.0, wait_time
  72:../toggle.cpp **** 
  73:../toggle.cpp ****     /* start the new cog */
  74:../toggle.cpp ****     start_cog();
 105              		.loc 1 74 0
 106 00a8 00007C5C 		lcall	#__Z9start_cogv	'
 106      00000000 
  75:../toggle.cpp ****     printf("toggle cog has started\n");
 107              		.loc 1 75 0
 108 00b0 00007C5C 		mvi	r0,#.LC1	',
 108      00000000 
 109 00b8 00007C5C 		lcall	#_puts	'
 109      00000000 
 110 00c0 0400FC80 		brs	#.L4	'
 111              	.L5
  76:../toggle.cpp **** 
  77:../toggle.cpp ****     /* every 2 seconds update the flashing frequency so the
  78:../toggle.cpp ****        light blinks faster and faster */
  79:../toggle.cpp ****     while(1) {
 112              		.loc 1 79 0
 113 00c4 00000000 		nop
 114              	.L4
  80:../toggle.cpp ****       sleep(2);
 115              		.loc 1 80 0
 116 00c8 0200FCA0 		mov	r0, #2	',
 117 00cc 00007C5C 		lcall	#_sleep	'
 117      00000000 
  81:../toggle.cpp ****       wait_time =  wait_time >> 1;
 118              		.loc 1 81 0
 119 00d4 00007C5C 		mvi	r7,#_wait_time	' tmp40,
 119      00000000 
 120 00dc 0000BC08 		rdlong	r7, r7	' wait_time.1, wait_time
 121 00e0 0000BCA0 		mov	r6, r7	' wait_time.2, wait_time.1
 122 00e4 0100FC28 		shr	r6, #1	' wait_time.2,
 123 00e8 00007C5C 		mvi	r7,#_wait_time	' tmp41,
 123      00000000 
 124 00f0 00003C08 		wrlong	r6, r7	' wait_time.2, wait_time
  82:../toggle.cpp ****       if (wait_time < MIN_GAP)
 125              		.loc 1 82 0
GAS LISTING /tmp/ccctTnIs.s 			page 5


 126 00f4 00007C5C 		mvi	r7,#_wait_time	' tmp42,
 126      00000000 
 127 00fc 0000BC08 		rdlong	r6, r7	' wait_time.4, wait_time
 128 0100 00007C5C 		mvi	r7,#399999	' tmp44,
 128      7F1A0600 
 129 0108 00003C87 		cmp	r6, r7 wz,wc	' wait_time.4, tmp44
 130 010c 0100F8A0 		IF_BE mov	r7,#1	', tmp45,
 131 0110 0000C4A0 		IF_A  mov	r7,#0	', tmp45,
 132 0114 FF00FC60 		and	r7,#255	' retval.3
 133 0118 00007CC3 		cmps	r7, #0 wz,wc	' retval.3,
 134 011c 5C00E884 		IF_E 	brs	#.L5	',
  83:../toggle.cpp **** 	wait_time = _clkfreq;
 135              		.loc 1 83 0
 136 0120 00007C5C 		mvi	r7,#__clkfreq	' tmp46,
 136      00000000 
 137 0128 0000BC08 		rdlong	r6, r7	' _clkfreq.5, _clkfreq
 138 012c 00007C5C 		mvi	r7,#_wait_time	' tmp47,
 138      00000000 
 139 0134 00003C08 		wrlong	r6, r7	' _clkfreq.5, wait_time
  79:../toggle.cpp ****     while(1) {
 140              		.loc 1 79 0
 141 0138 7800FC84 		brs	#.L5	'
 142              	.LBE3
 143              	.LFE1
 144              		.section	.debug_frame,"",@progbits
 145              	.Lframe0
 146 0000 10000000 		long	.LECIE0-.LSCIE0
 147              	.LSCIE0
 148 0004 FFFFFFFF 		long	0xffffffff
 149 0008 01       		byte	0x1
 150 0009 00       		.ascii "\0"
 151 000a 01       		.uleb128 0x1
 152 000b 7C       		.sleb128 -4
 153 000c 15       		byte	0x15
 154 000d 0C       		byte	0xc
 155 000e 10       		.uleb128 0x10
 156 000f 00       		.uleb128 0
 157 0010 09       		byte	0x9
 158 0011 15       		.uleb128 0x15
 159 0012 0F       		.uleb128 0xf
 160 0013 00       		.balign	4
 161              	.LECIE0
 162              	.LSFDE0
 163 0014 18000000 		long	.LEFDE0-.LASFDE0
 164              	.LASFDE0
 165 0018 00000000 		long	.Lframe0
 166 001c 00000000 		long	.LFB0
 167 0020 44000000 		long	.LFE0-.LFB0
 168 0024 44       		byte	0x4
 169              		long	.LCFI0-.LFB0
 170 0025 0E       		byte	0xe
 171 0026 04       		.uleb128 0x4
 172 0027 44       		byte	0x4
 173              		long	.LCFI1-.LCFI0
 174 0028 8E       		byte	0x8e
 175 0029 01       		.uleb128 0x1
 176 002a 44       		byte	0x4
GAS LISTING /tmp/ccctTnIs.s 			page 6


 177              		long	.LCFI2-.LCFI1
 178 002b 0D       		byte	0xd
 179 002c 0E       		.uleb128 0xe
 180 002d 000000   		.balign	4
 181              	.LEFDE0
 182              	.LSFDE2
 183 0030 18000000 		long	.LEFDE2-.LASFDE2
 184              	.LASFDE2
 185 0034 00000000 		long	.Lframe0
 186 0038 00000000 		long	.LFB1
 187 003c F8000000 		long	.LFE1-.LFB1
 188 0040 48       		byte	0x4
 189              		long	.LCFI3-.LFB1
 190 0041 0E       		byte	0xe
 191 0042 08       		.uleb128 0x8
 192 0043 8F       		byte	0x8f
 193 0044 02       		.uleb128 0x2
 194 0045 8E       		byte	0x8e
 195 0046 01       		.uleb128 0x1
 196 0047 44       		byte	0x4
 197              		long	.LCFI4-.LCFI3
 198 0048 0D       		byte	0xd
 199 0049 0E       		.uleb128 0xe
 200 004a 0000     		.balign	4
 201              	.LEFDE2
 202              		.text
 203              	.Letext0
 204              		.file 2 "/opt/parallax/lib/gcc/propeller-elf/4.6.1/../../../../propeller-elf/include/stdint.h"
 205              		.file 3 "/opt/parallax/lib/gcc/propeller-elf/4.6.1/../../../../propeller-elf/include/time.h"
 206              		.section	.debug_info,"",@progbits
 207              	.Ldebug_info0
 208 0000 8C020000 		long	0x28c
 209 0004 0200     		word	0x2
 210 0006 00000000 		long	.Ldebug_abbrev0
 211 000a 04       		byte	0x4
 212 000b 01       		.uleb128 0x1
 213 000c 474E5520 		.ascii "GNU C++ 4.6.1\0"
 213      432B2B20 
 213      342E362E 
 213      3100
 214 001a 04       		byte	0x4
 215 001b 2E2E2F74 		.ascii "../toggle.cpp\0"
 215      6F67676C 
 215      652E6370 
 215      7000
 216 0029 2F686F6D 		.ascii "/home/clewis/consulting/red9/propgcc/projects/gastoggle/temp\0"
 216      652F636C 
 216      65776973 
 216      2F636F6E 
 216      73756C74 
 217 0066 00000000 		long	.Ltext0
 218 006a 00000000 		long	.Letext0
 219 006e 00000000 		long	.Ldebug_line0
 220 0072 02       		.uleb128 0x2
 221 0073 04       		byte	0x4
 222 0074 07       		byte	0x7
 223 0075 756E7369 		.ascii "unsigned int\0"
GAS LISTING /tmp/ccctTnIs.s 			page 7


 223      676E6564 
 223      20696E74 
 223      00
 224 0082 02       		.uleb128 0x2
 225 0083 01       		byte	0x1
 226 0084 08       		byte	0x8
 227 0085 63686172 		.ascii "char\0"
 227      00
 228 008a 02       		.uleb128 0x2
 229 008b 04       		byte	0x4
 230 008c 05       		byte	0x5
 231 008d 696E7400 		.ascii "int\0"
 232 0091 02       		.uleb128 0x2
 233 0092 01       		byte	0x1
 234 0093 08       		byte	0x8
 235 0094 756E7369 		.ascii "unsigned char\0"
 235      676E6564 
 235      20636861 
 235      7200
 236 00a2 02       		.uleb128 0x2
 237 00a3 04       		byte	0x4
 238 00a4 05       		byte	0x5
 239 00a5 6C6F6E67 		.ascii "long int\0"
 239      20696E74 
 239      00
 240 00ae 02       		.uleb128 0x2
 241 00af 04       		byte	0x4
 242 00b0 07       		byte	0x7
 243 00b1 6C6F6E67 		.ascii "long unsigned int\0"
 243      20756E73 
 243      69676E65 
 243      6420696E 
 243      7400
 244 00c3 03       		.uleb128 0x3
 245 00c4 04       		byte	0x4
 246 00c5 82000000 		long	0x82
 247 00c9 02       		.uleb128 0x2
 248 00ca 02       		byte	0x2
 249 00cb 05       		byte	0x5
 250 00cc 73686F72 		.ascii "short int\0"
 250      7420696E 
 250      7400
 251 00d6 02       		.uleb128 0x2
 252 00d7 02       		byte	0x2
 253 00d8 07       		byte	0x7
 254 00d9 73686F72 		.ascii "short unsigned int\0"
 254      7420756E 
 254      7369676E 
 254      65642069 
 254      6E7400
 255 00ec 02       		.uleb128 0x2
 256 00ed 01       		byte	0x1
 257 00ee 06       		byte	0x6
 258 00ef 7369676E 		.ascii "signed char\0"
 258      65642063 
 258      68617200 
 259 00fb 04       		.uleb128 0x4
GAS LISTING /tmp/ccctTnIs.s 			page 8


 260 00fc 75696E74 		.ascii "uint32_t\0"
 260      33325F74 
 260      00
 261 0105 02       		byte	0x2
 262 0106 0D       		byte	0xd
 263 0107 72000000 		long	0x72
 264 010b 02       		.uleb128 0x2
 265 010c 08       		byte	0x8
 266 010d 07       		byte	0x7
 267 010e 6C6F6E67 		.ascii "long long unsigned int\0"
 267      206C6F6E 
 267      6720756E 
 267      7369676E 
 267      65642069 
 268 0125 02       		.uleb128 0x2
 269 0126 08       		byte	0x8
 270 0127 05       		byte	0x5
 271 0128 6C6F6E67 		.ascii "long long int\0"
 271      206C6F6E 
 271      6720696E 
 271      7400
 272 0136 04       		.uleb128 0x4
 273 0137 636C6F63 		.ascii "clock_t\0"
 273      6B5F7400 
 274 013f 03       		byte	0x3
 275 0140 08       		byte	0x8
 276 0141 72000000 		long	0x72
 277 0145 05       		.uleb128 0x5
 278 0146 01       		byte	0x1
 279 0147 73746172 		.ascii "start_cog\0"
 279      745F636F 
 279      6700
 280 0151 01       		byte	0x1
 281 0152 15       		byte	0x15
 282 0153 5F5A3973 		.ascii "_Z9start_cogv\0"
 282      74617274 
 282      5F636F67 
 282      7600
 283 0161 00000000 		long	.LFB0
 284 0165 00000000 		long	.LFE0
 285 0169 00000000 		long	.LLST0
 286 016d 9B010000 		long	0x19b
 287 0171 06       		.uleb128 0x6
 288 0172 00000000 		long	.LBB2
 289 0176 00000000 		long	.LBE2
 290 017a 07       		.uleb128 0x7
 291 017b 5F6C6F61 		.ascii "_load_start_cogtoggle\0"
 291      645F7374 
 291      6172745F 
 291      636F6774 
 291      6F67676C 
 292 0191 01       		byte	0x1
 293 0192 17       		byte	0x17
 294 0193 9B010000 		long	0x19b
 295 0197 01       		byte	0x1
 296 0198 01       		byte	0x1
 297 0199 00       		byte	0
GAS LISTING /tmp/ccctTnIs.s 			page 9


 298 019a 00       		byte	0
 299 019b 08       		.uleb128 0x8
 300 019c 72000000 		long	0x72
 301 01a0 A6010000 		long	0x1a6
 302 01a4 09       		.uleb128 0x9
 303 01a5 00       		byte	0
 304 01a6 0A       		.uleb128 0xa
 305 01a7 01       		byte	0x1
 306 01a8 6D61696E 		.ascii "main\0"
 306      00
 307 01ad 01       		byte	0x1
 308 01ae 33       		byte	0x33
 309 01af 8A000000 		long	0x8a
 310 01b3 00000000 		long	.LFB1
 311 01b7 00000000 		long	.LFE1
 312 01bb 00000000 		long	.LLST1
 313 01bf 47020000 		long	0x247
 314 01c3 0B       		.uleb128 0xb
 315 01c4 61726763 		.ascii "argc\0"
 315      00
 316 01c9 01       		byte	0x1
 317 01ca 33       		byte	0x33
 318 01cb 8A000000 		long	0x8a
 319 01cf 02       		byte	0x2
 320 01d0 91       		byte	0x91
 321 01d1 70       		.sleb128 -16
 322 01d2 0B       		.uleb128 0xb
 323 01d3 61726776 		.ascii "argv\0"
 323      00
 324 01d8 01       		byte	0x1
 325 01d9 33       		byte	0x33
 326 01da 47020000 		long	0x247
 327 01de 02       		byte	0x2
 328 01df 91       		byte	0x91
 329 01e0 74       		.sleb128 -12
 330 01e1 06       		.uleb128 0x6
 331 01e2 00000000 		long	.LBB3
 332 01e6 00000000 		long	.LBE3
 333 01ea 0C       		.uleb128 0xc
 334 01eb 6E00     		.ascii "n\0"
 335 01ed 01       		byte	0x1
 336 01ee 35       		byte	0x35
 337 01ef 8A000000 		long	0x8a
 338 01f3 0C       		.uleb128 0xc
 339 01f4 72657375 		.ascii "result\0"
 339      6C7400
 340 01fb 01       		byte	0x1
 341 01fc 36       		byte	0x36
 342 01fd 8A000000 		long	0x8a
 343 0201 0C       		.uleb128 0xc
 344 0202 73746172 		.ascii "startTime\0"
 344      7454696D 
 344      6500
 345 020c 01       		byte	0x1
 346 020d 37       		byte	0x37
 347 020e 72000000 		long	0x72
 348 0212 0C       		.uleb128 0xc
GAS LISTING /tmp/ccctTnIs.s 			page 10


 349 0213 656E6454 		.ascii "endTime\0"
 349      696D6500 
 350 021b 01       		byte	0x1
 351 021c 38       		byte	0x38
 352 021d 72000000 		long	0x72
 353 0221 0C       		.uleb128 0xc
 354 0222 65786563 		.ascii "executionTime\0"
 354      7574696F 
 354      6E54696D 
 354      6500
 355 0230 01       		byte	0x1
 356 0231 39       		byte	0x39
 357 0232 72000000 		long	0x72
 358 0236 0C       		.uleb128 0xc
 359 0237 72617754 		.ascii "rawTime\0"
 359      696D6500 
 360 023f 01       		byte	0x1
 361 0240 3A       		byte	0x3a
 362 0241 72000000 		long	0x72
 363 0245 00       		byte	0
 364 0246 00       		byte	0
 365 0247 03       		.uleb128 0x3
 366 0248 04       		byte	0x4
 367 0249 C3000000 		long	0xc3
 368 024d 07       		.uleb128 0x7
 369 024e 5F636C6B 		.ascii "_clkfreq\0"
 369      66726571 
 369      00
 370 0257 03       		byte	0x3
 371 0258 09       		byte	0x9
 372 0259 36010000 		long	0x136
 373 025d 01       		byte	0x1
 374 025e 01       		byte	0x1
 375 025f 0D       		.uleb128 0xd
 376 0260 77616974 		.ascii "wait_time\0"
 376      5F74696D 
 376      6500
 377 026a 01       		byte	0x1
 378 026b 27       		byte	0x27
 379 026c 77020000 		long	0x277
 380 0270 01       		byte	0x1
 381 0271 05       		byte	0x5
 382 0272 03       		byte	0x3
 383 0273 00000000 		long	_wait_time
 384 0277 0E       		.uleb128 0xe
 385 0278 72000000 		long	0x72
 386 027c 0D       		.uleb128 0xd
 387 027d 70696E73 		.ascii "pins\0"
 387      00
 388 0282 01       		byte	0x1
 389 0283 28       		byte	0x28
 390 0284 77020000 		long	0x277
 391 0288 01       		byte	0x1
 392 0289 05       		byte	0x5
 393 028a 03       		byte	0x3
 394 028b 00000000 		long	_pins
 395 028f 00       		byte	0
GAS LISTING /tmp/ccctTnIs.s 			page 11


 396              		.section	.debug_abbrev,"",@progbits
 397              	.Ldebug_abbrev0
 398 0000 01       		.uleb128 0x1
 399 0001 11       		.uleb128 0x11
 400 0002 01       		byte	0x1
 401 0003 25       		.uleb128 0x25
 402 0004 08       		.uleb128 0x8
 403 0005 13       		.uleb128 0x13
 404 0006 0B       		.uleb128 0xb
 405 0007 03       		.uleb128 0x3
 406 0008 08       		.uleb128 0x8
 407 0009 1B       		.uleb128 0x1b
 408 000a 08       		.uleb128 0x8
 409 000b 11       		.uleb128 0x11
 410 000c 01       		.uleb128 0x1
 411 000d 12       		.uleb128 0x12
 412 000e 01       		.uleb128 0x1
 413 000f 10       		.uleb128 0x10
 414 0010 06       		.uleb128 0x6
 415 0011 00       		byte	0
 416 0012 00       		byte	0
 417 0013 02       		.uleb128 0x2
 418 0014 24       		.uleb128 0x24
 419 0015 00       		byte	0
 420 0016 0B       		.uleb128 0xb
 421 0017 0B       		.uleb128 0xb
 422 0018 3E       		.uleb128 0x3e
 423 0019 0B       		.uleb128 0xb
 424 001a 03       		.uleb128 0x3
 425 001b 08       		.uleb128 0x8
 426 001c 00       		byte	0
 427 001d 00       		byte	0
 428 001e 03       		.uleb128 0x3
 429 001f 0F       		.uleb128 0xf
 430 0020 00       		byte	0
 431 0021 0B       		.uleb128 0xb
 432 0022 0B       		.uleb128 0xb
 433 0023 49       		.uleb128 0x49
 434 0024 13       		.uleb128 0x13
 435 0025 00       		byte	0
 436 0026 00       		byte	0
 437 0027 04       		.uleb128 0x4
 438 0028 16       		.uleb128 0x16
 439 0029 00       		byte	0
 440 002a 03       		.uleb128 0x3
 441 002b 08       		.uleb128 0x8
 442 002c 3A       		.uleb128 0x3a
 443 002d 0B       		.uleb128 0xb
 444 002e 3B       		.uleb128 0x3b
 445 002f 0B       		.uleb128 0xb
 446 0030 49       		.uleb128 0x49
 447 0031 13       		.uleb128 0x13
 448 0032 00       		byte	0
 449 0033 00       		byte	0
 450 0034 05       		.uleb128 0x5
 451 0035 2E       		.uleb128 0x2e
 452 0036 01       		byte	0x1
GAS LISTING /tmp/ccctTnIs.s 			page 12


 453 0037 3F       		.uleb128 0x3f
 454 0038 0C       		.uleb128 0xc
 455 0039 03       		.uleb128 0x3
 456 003a 08       		.uleb128 0x8
 457 003b 3A       		.uleb128 0x3a
 458 003c 0B       		.uleb128 0xb
 459 003d 3B       		.uleb128 0x3b
 460 003e 0B       		.uleb128 0xb
 461 003f 8740     		.uleb128 0x2007
 462 0041 08       		.uleb128 0x8
 463 0042 11       		.uleb128 0x11
 464 0043 01       		.uleb128 0x1
 465 0044 12       		.uleb128 0x12
 466 0045 01       		.uleb128 0x1
 467 0046 40       		.uleb128 0x40
 468 0047 06       		.uleb128 0x6
 469 0048 01       		.uleb128 0x1
 470 0049 13       		.uleb128 0x13
 471 004a 00       		byte	0
 472 004b 00       		byte	0
 473 004c 06       		.uleb128 0x6
 474 004d 0B       		.uleb128 0xb
 475 004e 01       		byte	0x1
 476 004f 11       		.uleb128 0x11
 477 0050 01       		.uleb128 0x1
 478 0051 12       		.uleb128 0x12
 479 0052 01       		.uleb128 0x1
 480 0053 00       		byte	0
 481 0054 00       		byte	0
 482 0055 07       		.uleb128 0x7
 483 0056 34       		.uleb128 0x34
 484 0057 00       		byte	0
 485 0058 03       		.uleb128 0x3
 486 0059 08       		.uleb128 0x8
 487 005a 3A       		.uleb128 0x3a
 488 005b 0B       		.uleb128 0xb
 489 005c 3B       		.uleb128 0x3b
 490 005d 0B       		.uleb128 0xb
 491 005e 49       		.uleb128 0x49
 492 005f 13       		.uleb128 0x13
 493 0060 3F       		.uleb128 0x3f
 494 0061 0C       		.uleb128 0xc
 495 0062 3C       		.uleb128 0x3c
 496 0063 0C       		.uleb128 0xc
 497 0064 00       		byte	0
 498 0065 00       		byte	0
 499 0066 08       		.uleb128 0x8
 500 0067 01       		.uleb128 0x1
 501 0068 01       		byte	0x1
 502 0069 49       		.uleb128 0x49
 503 006a 13       		.uleb128 0x13
 504 006b 01       		.uleb128 0x1
 505 006c 13       		.uleb128 0x13
 506 006d 00       		byte	0
 507 006e 00       		byte	0
 508 006f 09       		.uleb128 0x9
 509 0070 21       		.uleb128 0x21
GAS LISTING /tmp/ccctTnIs.s 			page 13


 510 0071 00       		byte	0
 511 0072 00       		byte	0
 512 0073 00       		byte	0
 513 0074 0A       		.uleb128 0xa
 514 0075 2E       		.uleb128 0x2e
 515 0076 01       		byte	0x1
 516 0077 3F       		.uleb128 0x3f
 517 0078 0C       		.uleb128 0xc
 518 0079 03       		.uleb128 0x3
 519 007a 08       		.uleb128 0x8
 520 007b 3A       		.uleb128 0x3a
 521 007c 0B       		.uleb128 0xb
 522 007d 3B       		.uleb128 0x3b
 523 007e 0B       		.uleb128 0xb
 524 007f 49       		.uleb128 0x49
 525 0080 13       		.uleb128 0x13
 526 0081 11       		.uleb128 0x11
 527 0082 01       		.uleb128 0x1
 528 0083 12       		.uleb128 0x12
 529 0084 01       		.uleb128 0x1
 530 0085 40       		.uleb128 0x40
 531 0086 06       		.uleb128 0x6
 532 0087 01       		.uleb128 0x1
 533 0088 13       		.uleb128 0x13
 534 0089 00       		byte	0
 535 008a 00       		byte	0
 536 008b 0B       		.uleb128 0xb
 537 008c 05       		.uleb128 0x5
 538 008d 00       		byte	0
 539 008e 03       		.uleb128 0x3
 540 008f 08       		.uleb128 0x8
 541 0090 3A       		.uleb128 0x3a
 542 0091 0B       		.uleb128 0xb
 543 0092 3B       		.uleb128 0x3b
 544 0093 0B       		.uleb128 0xb
 545 0094 49       		.uleb128 0x49
 546 0095 13       		.uleb128 0x13
 547 0096 02       		.uleb128 0x2
 548 0097 0A       		.uleb128 0xa
 549 0098 00       		byte	0
 550 0099 00       		byte	0
 551 009a 0C       		.uleb128 0xc
 552 009b 34       		.uleb128 0x34
 553 009c 00       		byte	0
 554 009d 03       		.uleb128 0x3
 555 009e 08       		.uleb128 0x8
 556 009f 3A       		.uleb128 0x3a
 557 00a0 0B       		.uleb128 0xb
 558 00a1 3B       		.uleb128 0x3b
 559 00a2 0B       		.uleb128 0xb
 560 00a3 49       		.uleb128 0x49
 561 00a4 13       		.uleb128 0x13
 562 00a5 00       		byte	0
 563 00a6 00       		byte	0
 564 00a7 0D       		.uleb128 0xd
 565 00a8 34       		.uleb128 0x34
 566 00a9 00       		byte	0
GAS LISTING /tmp/ccctTnIs.s 			page 14


 567 00aa 03       		.uleb128 0x3
 568 00ab 08       		.uleb128 0x8
 569 00ac 3A       		.uleb128 0x3a
 570 00ad 0B       		.uleb128 0xb
 571 00ae 3B       		.uleb128 0x3b
 572 00af 0B       		.uleb128 0xb
 573 00b0 49       		.uleb128 0x49
 574 00b1 13       		.uleb128 0x13
 575 00b2 3F       		.uleb128 0x3f
 576 00b3 0C       		.uleb128 0xc
 577 00b4 02       		.uleb128 0x2
 578 00b5 0A       		.uleb128 0xa
 579 00b6 00       		byte	0
 580 00b7 00       		byte	0
 581 00b8 0E       		.uleb128 0xe
 582 00b9 35       		.uleb128 0x35
 583 00ba 00       		byte	0
 584 00bb 49       		.uleb128 0x49
 585 00bc 13       		.uleb128 0x13
 586 00bd 00       		byte	0
 587 00be 00       		byte	0
 588 00bf 00       		byte	0
 589              		.section	.debug_loc,"",@progbits
 590              	.Ldebug_loc0
 591              	.LLST0
 592 0000 00000000 		long	.LFB0-.Ltext0
 593 0004 04000000 		long	.LCFI0-.Ltext0
 594 0008 0200     		word	0x2
 595 000a 80       		byte	0x80
 596 000b 00       		.sleb128 0
 597 000c 04000000 		long	.LCFI0-.Ltext0
 598 0010 0C000000 		long	.LCFI2-.Ltext0
 599 0014 0200     		word	0x2
 600 0016 80       		byte	0x80
 601 0017 04       		.sleb128 4
 602 0018 0C000000 		long	.LCFI2-.Ltext0
 603 001c 44000000 		long	.LFE0-.Ltext0
 604 0020 0200     		word	0x2
 605 0022 7E       		byte	0x7e
 606 0023 04       		.sleb128 4
 607 0024 00000000 		long	0
 608 0028 00000000 		long	0
 609              	.LLST1
 610 002c 44000000 		long	.LFB1-.Ltext0
 611 0030 4C000000 		long	.LCFI3-.Ltext0
 612 0034 0200     		word	0x2
 613 0036 80       		byte	0x80
 614 0037 00       		.sleb128 0
 615 0038 4C000000 		long	.LCFI3-.Ltext0
 616 003c 50000000 		long	.LCFI4-.Ltext0
 617 0040 0200     		word	0x2
 618 0042 80       		byte	0x80
 619 0043 08       		.sleb128 8
 620 0044 50000000 		long	.LCFI4-.Ltext0
 621 0048 3C010000 		long	.LFE1-.Ltext0
 622 004c 0200     		word	0x2
 623 004e 7E       		byte	0x7e
GAS LISTING /tmp/ccctTnIs.s 			page 15


 624 004f 08       		.sleb128 8
 625 0050 00000000 		long	0
 626 0054 00000000 		long	0
 627              		.section	.debug_aranges,"",@progbits
 628 0000 1C000000 		long	0x1c
 629 0004 0200     		word	0x2
 630 0006 00000000 		long	.Ldebug_info0
 631 000a 04       		byte	0x4
 632 000b 00       		byte	0
 633 000c 0000     		word	0
 634 000e 0000     		word	0
 635 0010 00000000 		long	.Ltext0
 636 0014 3C010000 		long	.Letext0-.Ltext0
 637 0018 00000000 		long	0
 638 001c 00000000 		long	0
 639              		.section	.debug_line,"",@progbits
 640 0000 BA000000 	.Ldebug_line0
 640      02008600 
 640      00000101 
 640      FB0E0D00 
 640      01010101 
