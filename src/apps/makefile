CC = ../../sysroot/tools/host-gcc/bin/x86_64-bestos-gcc

all: ../root/bin/sh ../root/bin/init
.DEFAULT = all

../root/bin/init: init.c
	mkdir -p $(@D)
	$(CC) -o $@ $<

../root/bin/sh: sh.c
	mkdir -p $(@D)
	$(CC) -o $@ $<


