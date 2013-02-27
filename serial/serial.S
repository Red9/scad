		.section .serial.cog, "ax"
		.equ	Buf_len, $100
		.equ	Min_half_period, $56
		.equ	Xon, $11
		.equ	Xoff, $13
		.compress off
..start
		.org	0
Fds_entry
		neg	PHSA, #1
		mov	CTRA, Ctra_val
		mov	DIRA, Masktx
		mov	CTRB, Ctrb_val
		mov	FRQB, #2
		rdlong	Step_clocks, Period_ptr
		wrlong	OUTB, PAR
		mov	Timestamp, Step_clocks
		add	Timestamp, CNT
Rx_cleanup
		mov	PHSB, #0
Rx_main
		jmpret	Lockstep_ret, Tx_jump
		mov	Bits_in, PHSB	wz
	if_z 	jmp	#Rx_main
		jmpret	Lockstep_ret, Tx_jump
		cmp	Bits_in, Step_clocks	wz, wc
	if_b 	jmpret	Lockstep_ret, Tx_jump
		mov	Bits_in, #8
Rx_main.get_bit
		jmpret	Lockstep_ret, Tx_jump
		test	Maskrx, INA	wc
		rcr	Data_in, #1
		jmpret	Lockstep_ret, Tx_jump
		djnz	Bits_in, #Rx_main.get_bit
		shr	Data_in, #($20 - 8)
		jmpret	Lockstep_ret, Tx_jump
		test	Maskrx, INA	wc
	if_nc 	jmp	#Rx_cleanup
		add	Rx_ptr, Rx_head_ptr
		wrbyte	Data_in, Rx_ptr
		add	Rx_ptr, #1
		jmpret	Lockstep_ret, Tx_jump
		cmpsub	Rx_ptr, Rx_end_ptr	wc
	if_nc 	sub	Rx_ptr, Rx_head_ptr
		wrword	Rx_ptr, Update_head_ptr
		tjz	PHSB, #Rx_main	wr
Tx_jump
		jmp	#Tx_main
Lockstep
		waitcnt	Timestamp, Step_clocks
		ror	PHSA, Write_bit
Lockstep_ret
		jmp	#(0 - 0)
Tx_main
		rdlong	Bytes_out, PAR
		jmpret	Tx_jump, #Lockstep
		rdlong	Step_clocks, Period_ptr
		jmpret	Tx_jump, #Lockstep
		tjz	Bytes_out, #Tx_main
		mov	Write_ptr, Bytes_out
		shr	Bytes_out, #$10
		add	Bytes_out, #1
		jmpret	Tx_jump, #Lockstep
Tx_main.tx_byte
		rdbyte	Tmp, Write_ptr
		add	Write_ptr, #1
		or	Tmp, #$100
		jmpret	Tx_jump, #Lockstep
		shl	Tmp, #($20 - 9)
		sar	Tmp, #($20 - $a)
		mov	PHSA, Tmp
		mov	Half_bits_out, #$14
Tx_main.tx_bit
		jmpret	Tx_jump, #Lockstep
		xor	Write_bit, #1
		djnz	Half_bits_out, #Tx_main.tx_bit
		jmpret	Tx_jump, #Lockstep
		djnz	Bytes_out, #Tx_main.tx_byte
		wrlong	Bytes_out, PAR
		jmpret	Tx_jump, #Lockstep
		jmp	#(Tx_main + 1)
Period_ptr
		.long	0
Maskrx
		.long	0
Masktx
		.long	0
Rx_head_ptr
		.long	0
Rx_end_ptr
		.long	0
Rx_ptr
		.long	0
Ctra_val
		.long	0
Ctrb_val
		.long	0
Write_bit
		.long	0
Update_head_ptr
		.long	0
Tmp
		.res	1
Step_clocks
		.res	1
Bits_in
		.res	1
Half_bits_out
		.res	1
Bytes_out
		.res	1
Data_in
		.res	1
Timestamp
		.res	1
Write_ptr
		.res	1
		.fit	$1f1
	    .compress default
