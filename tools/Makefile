
AOS_INFO_OBJECTS=aos-info.o files.o mpk.o
AOS_UNPACK_OBJECTS=aos-unpack.o files.o mpk.o
AOS_FLASH_OBJECTS=aos-flash.o files.o mpk.o
AOS_FIX_OBJECTS=aos-fix.o files.o mpk.o
AOS_REPACK_OBJECTS=aos-repack.o files.o mpk.o

all: aos-info aos-unpack aos-flash aos-fix aos-repack

.c.o:
	gcc -c $*.c

aos-info: $(AOS_INFO_OBJECTS)
	gcc -o aos-info $(AOS_INFO_OBJECTS) -L../libaos -laos -lcrypto

aos-unpack: $(AOS_UNPACK_OBJECTS)
	gcc -o aos-unpack $(AOS_UNPACK_OBJECTS) -L../libaos -laos -lcrypto

aos-flash: $(AOS_FLASH_OBJECTS)
	gcc -o aos-flash $(AOS_FLASH_OBJECTS) -L../libaos -laos -lcrypto

aos-fix: $(AOS_FIX_OBJECTS)
	gcc -o aos-fix $(AOS_FIX_OBJECTS) -L../libaos -laos -lcrypto

aos-repack: $(AOS_REPACK_OBJECTS)
	gcc -o aos-repack $(AOS_REPACK_OBJECTS) -L../libaos -laos -lcrypto

install:
	cp aos-info /usr/bin/
	cp aos-unpack /usr/bin/
	cp aos-flash /usr/bin/
	cp aos-fix /usr/bin/
	cp aos-repack /usr/bin/

clean:
	rm -f aos-info aos-unpack aos-flash aos-fix aos-repack *.o
