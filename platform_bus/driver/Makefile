
DIR = /work/linux-3.5/
obj-m := driver.o

all:
	make -C $(DIR) M=`pwd` modules
	
clean:
	make -C $(DIR) M=`pwd` modules clean
	

