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
CFLAGS = -O3 -g -c -fPIC -Wall -Wextra -Wno-unused-parameter -Werror -pedantic -mcpu=arm920t -msoft-float -I. -I../include
# ASFLAGS = -g -fPIC -Wall -mcpu=arm920t -msoft-float -I. -I../include

# -static: force static linking
# -e: set entry point
# -nmagic: no page alignment
# -T: use linker script
LDFLAGS = -static -e main -nmagic -T $(SRCDIR)/linker.ld -L ../lib -L $(XLIBDIR2)
LIBFLAGS = -lbwio -lgcc

SRCDIR = .
BUILDDIR = ../build

C_SOURCES = $(wildcard $(SRCDIR)/*.c)
C_OBJECTS = $(patsubst $(SRCDIR)/%.c,$(BUILDDIR)/%.o,$(C_SOURCES))

ASM_SOURCES = $(wildcard $(SRCDIR)/*.asm)
ASM_OBJECTS = $(patsubst $(SRCDIR)/%.asm,$(BUILDDIR)/%.o,$(ASM_SOURCES))

SOURCES = $(C_SOURCES) $(ASM_SOURCES)
OBJECTS = $(C_OBJECTS) $(ASM_OBJECTS)

BINARY = kernel.elf

happy: clean all

inline:
	-mkdir -p $(BUILDDIR)
	-rm $(BUILDDIR)/*
	cp arm_lib.asm $(BUILDDIR)
	cp linker.ld $(BUILDDIR)
	python3 ./inline.py . $(BUILDDIR)
	cd $(BUILDDIR) && $(CC) $(CFLAGS) -o inline-all.o inline-all.c
	cd $(BUILDDIR) && $(AS) $(ASFLAGS) -o arm_lib.o arm_lib.asm
	cd $(BUILDDIR) && $(LD) $(LDFLAGS) -o $(BUILDDIR)/$(BINARY) inline-all.o arm_lib.o $(LIBFLAGS)
	cp $(BUILDDIR)/$(BINARY) ./$(BINARY)

all: $(BUILDDIR) $(OBJECTS) $(BINARY)

$(BUILDDIR):
	-mkdir -p $@

$(BUILDDIR)/%.o: $(SRCDIR)/%.asm
	$(AS) $(ASFLAGS) -o $@ $^

$(BUILDDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -o $@ $^

$(BINARY): $(OBJECTS)
	$(LD) $(LDFLAGS) -o $(BUILDDIR)/$@ $^ $(LIBFLAGS)
	-cp $(BUILDDIR)/$@ ./$@

fatal: CFLAGS += -DVERBOSITY=1
fatal: $(BINARY)

error: CFLAGS += -DVERBOSITY=2
error: $(BINARY)

warn: CFLAGS += -DVERBOSITY=3
warn: $(BINARY)

log: CLAGS += -DVERBOSITY=4
log: $(BINARY)

debug: CFLAGS += -DVERBOSITY=5
debug: $(BINARY)

install:
	@echo Installing for `whoami`.
	-touch /u/cs452/tftp/ARM/`whoami`/kernel
	-rm /u/cs452/tftp/ARM/`whoami`/kernel
	-cp $(BUILDDIR)/$(BINARY) /u/cs452/tftp/ARM/`whoami`/kernel
	-chmod o=r /u/cs452/tftp/ARM/`whoami`/kernel

clean:
	-rm -f ./$(BINARY) $(BUILDDIR)/*

