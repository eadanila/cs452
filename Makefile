ASSIGNMENT = k1.elf
BINARY = kernel.elf
SRCDIR = ./kernel
BUILDDIR = ./build

all: clean
	cd $(SRCDIR) && $(MAKE)
	cp $(BUILDDIR)/$(BINARY) ./$(ASSIGNMENT)


fatal: CFLAGS += -DVERBOSITY=1
fatal: kernel.elf

error: CFLAGS += -DVERBOSITY=2
error: kernel.elf

warn: CFLAGS += -DVERBOSITY=3
warn: kernel.elf

log: CLAGS += -DVERBOSITY=4
log: kernel.elf

debug: CFLAGS += -DVERBOSITY=5
debug: kernel.elf

install:
	@echo Installing for `whoami`.
	-touch /u/cs452/tftp/ARM/`whoami`/kernel
	-rm /u/cs452/tftp/ARM/`whoami`/kernel
	-cp $(BUILDDIR)/$(BINARY) /u/cs452/tftp/ARM/`whoami`/kernel
	-chmod o=r /u/cs452/tftp/ARM/`whoami`/kernel

clean:
	-rm -f ./$(ASSIGNMENT) ./$(BINARY) $(BUILDDIR)/$(BINARY) $(BUILDDIR)/*.s $(BUILDDIR)/*.o

