
AOS_INFO_OBJECTS=aos-info.o files.o mpk.o
AOS_EXTRACT_OBJECTS=aos-extract.o files.o mpk.o
AOS_UNPACK_OBJECTS=aos-unpack.o files.o mpk.o
AOS_FLASH_OBJECTS=aos-flash.o files.o mpk.o
AOS_RESIZE_OBJECTS=aos-resize.o files.o mpk.o
AOS_REPACK_OBJECTS=aos-repack.o files.o mpk.o

all: aos-info aos-extract aos-unpack aos-flash aos-resize aos-repack

.c.o:
	gcc -c $*.c

aos-info: $(AOS_INFO_OBJECTS)
	gcc -o aos-info $(AOS_INFO_OBJECTS) -L../libaos -laos -lcrypto

aos-extract: $(AOS_EXTRACT_OBJECTS)
	gcc -o aos-extract $(AOS_EXTRACT_OBJECTS) -L../libaos -laos -lcrypto

aos-unpack: $(AOS_UNPACK_OBJECTS)
	gcc -o aos-unpack $(AOS_UNPACK_OBJECTS) -L../libaos -laos -lcrypto

aos-flash: $(AOS_FLASH_OBJECTS)
	gcc -o aos-flash $(AOS_FLASH_OBJECTS) -L../libaos -laos -lcrypto

aos-resize: $(AOS_RESIZE_OBJECTS)
	gcc -o aos-resize $(AOS_RESIZE_OBJECTS) -L../libaos -laos -lcrypto

aos-repack: $(AOS_REPACK_OBJECTS)
	gcc -o aos-repack $(AOS_REPACK_OBJECTS) -L../libaos -laos -lcrypto

install:
	cp aos-info /usr/bin/
	cp aos-extract /usr/bin/
	cp aos-unpack /usr/bin/
	cp aos-flash /usr/bin/
	cp aos-resize /usr/bin/
	cp aos-repack /usr/bin/

clean:
	rm -f aos-info aos-extract aos-unpack aos-flash aos-resize aos-repack *.o
