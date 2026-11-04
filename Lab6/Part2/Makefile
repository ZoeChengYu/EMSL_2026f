obj-m := demo.o

KDIR ?= /lib/modules/$(shell uname -r)/build

.PHONY: all modules clean

all: modules test

modules:
	$(MAKE) -C $(KDIR) M=$(CURDIR) modules

test: test.c demo_ioctl.h
	$(CC) -Wall -Wextra -O2 -o $@ test.c

clean:
	$(MAKE) -C $(KDIR) M=$(CURDIR) clean
	$(RM) test
