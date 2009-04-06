# romcopy.ld
# Nancy Minderman
# May 8, 2007
# This file was originally created by Pawel Pytlak in another form
# for students in CMPE490

# This link script file is meant to be used with its sister file
# boot_romcopy.arm

# It's been butchered in the name of science by Nancy Minderman

# Making a mistake in editing this file can blow
# away Angel. If you do that you won't be able to use the
# serial port to connect to the board.
# Ask Nancy if you don't know what you're doing

CONSTANTS
{
	vt_reserve = 8 * 4 + 5 * 4
	sup_stack_reserve = 1k
	stack_reserve = 8k
	heap_reserve = 40k

# user_flash_memory_loadtime is for loadtime resolve of addresses so that
# when choosing to flash an executeable, no offsets are required
# in the "Write to Flash" Multi dialoq box.
# 0x01100000 is the base address your code should be flashed to and
# 0x01100000 is also the physical location of sector 23 on the FLASH chip
# when the std-user jumper is in the std position
	user_flash_memory_loadtime = 0x01100000
}
MEMORY
{
	sram_memory	: ORIGIN = 0x02000000,	LENGTH = 256K
	
# Note that this is the runtime location for user space
# when the std-user jumper is in the user position
	user_flash_memory_runtime	: ORIGIN = 0x01000000,	LENGTH = 1M

# This section is defined here soley for informational purposes
# Do not mess with this, at all, ever!!!
	std_flash_memory	: ORIGIN = 0x01100000,	LENGTH = 1M
}
SECTIONS
{
# RAM stuff
# These are the runtime locations for
# everything. Your code, data, and uninitialized
# data sections will eventually be copied over
# from FLASH to RAM and run from RAM
	.vt PAD (vt_reserve) : > sram_memory
	.supr_stack PAD (sup_stack_reserve) : > .
	.syscall : > .
	.intercall : > .
	.interfunc : > .
	.text : > .
	.data : > .
	.bss : > .
	.user_heap ALIGN(16)PAD (heap_reserve) : > .
	.user_stack ALIGN(16)PAD (stack_reserve) : > .
	
# FLASH stuff
# These are the loadtime and runtime locations
# of the flashed code, and data before the copy to
# RAM occurs. boot_romcopy.arm takes
# care of the copy over based on the symbols below

# .boot and .vectors can never be in RAM so it stays in FLASH
# the bootloader must also be located at runtime address 0x01000000

# The LOAD command instructs the linker to place the
# code in a location other than the runtime locations
# This makes the std-user jumper switch trivial
# ie. get the linker to do it !

# The ROM command is a command that instructs the linker
# to make an exact copy of another section. This exact
# copy will be stored in FLASH, copied to RAM and run
# from RAM by the bootloader. For the romcopy version
# of an executeable only the .text and .data sections
# have ROM copies.

# Had to add three more sections
# .syscall .intercall and .interfunc
# all are required and must have corresponding RAM sections

# Added the .fixaddr and .fixtype section. According to the Green Hills
# manual these sections only need to extist in ROM. They are read only.
# P. 299 of "Building applications for  embedded ARM.

	.boot 			LOAD	(user_flash_memory_loadtime): > user_flash_memory_runtime
	.vectors 		LOAD 	(user_flash_memory_loadtime + SIZEOF(.boot)): > .
	.romsyscall 	LOAD	(user_flash_memory_loadtime +
						SIZEOF(.boot) + SIZEOF(.vectors)) ROM (.syscall) : > .
	.romintercall	LOAD	(user_flash_memory_loadtime + SIZEOF(.boot) +
						SIZEOF(.vectors) + SIZEOF(.romsyscall)) ROM(.intercall) : > .
	.rominterfunc 	LOAD 	(user_flash_memory_loadtime + SIZEOF(.boot) +
						SIZEOF(.vectors) + SIZEOF(.romsyscall) +
						SIZEOF(.romintercall)) ROM (.interfunc) : > .
	.romtext 		LOAD 	(user_flash_memory_loadtime +
						SIZEOF(.boot) + SIZEOF(.vectors) +
						SIZEOF(.romsyscall) + SIZEOF(.rominterfunc)) ROM (.text) : > .
	.rodata 		LOAD 	(user_flash_memory_loadtime +
						SIZEOF(.boot) + SIZEOF(.vectors) +
						SIZEOF(.romsyscall) + SIZEOF(.romintercall) +
						SIZEOF(.rominterfunc) + SIZEOF(.romtext)) : > .
	.romdata 		LOAD 	(user_flash_memory_loadtime +
						SIZEOF(.boot) + SIZEOF(.vectors) +
						SIZEOF(.romsyscall) + SIZEOF(.romintercall) +
						SIZEOF(.rominterfunc)+ SIZEOF(.romtext) +
						SIZEOF(.rodata) )
						ROM (.data) : > .
	.fixaddr		LOAD	(user_flash_memory_loadtime +
						SIZEOF(.boot) + SIZEOF(.vectors) +
						SIZEOF(.romsyscall) + SIZEOF(.romintercall) +
						SIZEOF(.rominterfunc)+ SIZEOF(.romtext) +
						SIZEOF(.rodata) + SIZEOF(.data) ) : > .
	.fixtype		LOAD	(user_flash_memory_loadtime +
						SIZEOF(.boot) + SIZEOF(.vectors) +
						SIZEOF(.romsyscall) + SIZEOF(.romintercall) +
						SIZEOF(.rominterfunc)+ SIZEOF(.romtext) +
						SIZEOF(.rodata) + SIZEOF(.data) +
						SIZEOF(.fixaddr) ) : > .
						
# Symbols that are used by boot_romcopy.arm to init everything

# All of the regions in the .romdata section need to be
# copied from FLASH to RAM inside boot_romcopy.arm.
# For that to succeed, the code in boot_romcopy.arm needs to
# know where everything is.
	__romdatastart = ADDR(.romdata);
	__romdatasize = SIZEOF(.data);

# Because we are not running from FLASH but RAM we need
# to copy over all of the code too. The .romtext section
# contains all of your program's code and runtime support
#	__romtextstart = ADDR(.romtext);
	__romtextstart = ADDR(.romsyscall);
	__romtextsize = SIZEOF(.text) + SIZEOF(.intercall) +
					SIZEOF(.syscall) + SIZEOF(.interfunc);

# These are defined so that the chipselects for FLASH and RAM
# can be properly set
	__rambase = ADDR(sram_memory);
	__rombase = ADDR(.boot);

# The .bss section contains uninitialzed data.
# The only thing that boot_romcopy.arm needs to do with
# this is set all of the reqion space to zero.
	__bssstart = ADDR(.bss);
	__bssend = __bssstart + SIZEOF(.bss);

# These symbols define the locations in RAM for the data section.
# Because this is an embedded system, a copy of this needs
# to be in FLASH, but copied to RAM for runtime code to work.
# These symbols represent the destination addresses in RAM
	__datastart = ADDR(.data);
	__datasize = __romdatasize;
	__dataend = __datastart + SIZEOF(.data);

# These symbols define the location in RAM for the .text section
# All your code will eventually run from here. Your code must be copied
# from FLASH first. These symbols represent the destination addresses
# in RAM.
#	__textstart = ADDR(.text);
#	__textsize = SIZEOF(.text);
#	__textend = ADDR(.text) + __textsize;
	__textstart = ADDR(.syscall);
	__textsize = SIZEOF(.syscall) + SIZEOF(.intercall) +
				 SIZEOF(.interfunc) + SIZEOF(.text);
	__textend = ADDR(.syscall) + __textsize;

# note that both stacks increase in size towards lower addresses
	__user_stack_begin = ADDR(.user_stack) + SIZEOF(.user_stack);
	__supr_stack_begin = ADDR(.supr_stack) + SIZEOF(.supr_stack);
# Added definitions for the ghs heap so that library dynamic allocation
# will succeed
	__ghsbegin_heap = ADDR(.user_heap);
	__ghsend_heap = __ghsbegin_heap + SIZEOF(.user_heap);
}
