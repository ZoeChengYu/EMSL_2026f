obj-m := demo.o

KDIR ?= /lib/modules/$(shell uname -r)/build
PWD  := $(shell pwd)

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules
	gcc -Wall -Wextra -O2 -o Lab7 Lab7.c

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
	rm -f Lab7
