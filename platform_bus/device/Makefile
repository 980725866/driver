
DIR = /work/linux-3.5/
obj-m := device.o

all:
	make -C $(DIR) M=`pwd` modules
	
clean:
	make -C $(DIR) M=`pwd` modules clean
	

