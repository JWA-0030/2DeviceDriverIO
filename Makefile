obj-m += HW3Read.o
obj-m += HW3Write.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
	$(CC) HW3Driver.c -o test
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	rm test
