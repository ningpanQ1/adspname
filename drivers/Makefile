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
	@make -C /lib/modules/$(UNAME)/build M=$(PWD) modules

clean:
	@make -C /lib/modules/$(UNAME)/build M=$(PWD) clean
#	@rm -f *.o
#	@rm -f *.ko
#	@rm -f *.mod.c
#	@rm -f .*.cmd
#	@rm -rf .tmp_versions
#	@rm -rf Module.*
#	@rm -rf *~
#	@rm -rf *order
#	@rm -rf .cache*
