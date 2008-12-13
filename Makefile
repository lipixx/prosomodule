obj-m += mihuerto.o
export-objs := activar_monitoritzacio.o. 
export-objs := activar_monitoritzacio.o. 
export-objs := activar_monitoritzacio.o. 
export-objs := activar_monitoritzacio.o. 
//obj-m += elpayes.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
