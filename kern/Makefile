#
# Makefile for busy-wait IO library
#
XDIR=/u/cs452/public/xdev
XBINDIR=$(XDIR)/bin
XLIBDIR1=$(XDIR)/arm-none-eabi/lib
XLIBDIR2=$(XDIR)/lib/gcc/arm-none-eabi/9.2.0
CC = $(XBINDIR)/arm-none-eabi-gcc
AR = $(XBINDIR)/arm-none-eabi-ar
AS = $(XBINDIR)/arm-none-eabi-as
LD = $(XBINDIR)/arm-none-eabi-ld

# -g: include debug information for gdb
# -S: only compile and emit assembly
# -fPIC: emit position-independent code
# -Wall: report all warnings
# -mcpu=arm920t: generate code for the 920t architecture
# -msoft-float: no FP co-processor
CFLAGS = -g -S -fPIC -Wall -mcpu=arm920t -msoft-float -I. -I../include
# ASFLAGS = -g -fPIC -Wall -mcpu=arm920t -msoft-float -I. -I../include

# -static: force static linking
# -e: set entry point
# -nmagic: no page alignment
# -T: use linker script
LDFLAGS = -static -e main -nmagic -T linker.ld -L ../lib -L $(XLIBDIR2)

all: kernel.elf

arm_lib.o: arm_lib.asm
	$(AS) $(ASFLAGS) -o arm_lib.o arm_lib.asm

main.s: main.c
	$(CC) -S $(CFLAGS) main.c

main.o: main.s
	$(AS) $(ASFLAGS) -o main.o main.s

syscall.s: syscall.c
	$(CC) -S $(CFLAGS) syscall.c

syscall.o: syscall.s
	$(AS) $(ASFLAGS) -o syscall.o syscall.s

# arm_lib.o: arm_lib.s
# 	$(AS) $(ASFLAGS) -o test.o arm_lib.s

kernel.elf: main.o arm_lib.o syscall.o
	$(LD) $(LDFLAGS) -o $@ main.o arm_lib.o syscall.o -lbwio -lgcc

clean:
	-rm -f kernel.elf *.s *.o
