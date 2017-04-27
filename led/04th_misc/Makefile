
DIR = /work/linux-3.5/
obj-m := led.o
 
#PWD       := $(shell pwd)

all:
	make -C $(DIR) M=`pwd` modules
	
clean:
	make -C $(DIR) M=`pwd` modules clean
	

