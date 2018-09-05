#
# Makefile for the Adspname device drivers.
#
# Advantech eAutomation Division
#

obj-m := adspname.o
UNAME := $(shell uname -r)
PWD := $(shell pwd)
ADVMOD := adspname

default:
	@make -C /lib/modules/$(UNAME)/build SUBDIRS=$(PWD) modules

clean:
	@rm -f *.o
	@rm -f *.ko
	@rm -f *.mod.c
	@rm -f .*.cmd
	@rm -rf .tmp_versions
	@rm -rf Module.*
	@rm -rf *~
	@rm -rf *order
	@rm -rf .cache*
