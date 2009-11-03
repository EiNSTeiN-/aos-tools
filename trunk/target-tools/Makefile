
FLASH_OBJECTS=flash.o flashrw.o insmod.o rmmod.o files.o

all: flash

.c.o:
	arm-linux-uclibcgnueabi-gcc -c $*.c

flash: $(FLASH_OBJECTS)
	arm-linux-uclibcgnueabi-gcc -o flash $(FLASH_OBJECTS)

clean:
	rm -f flash *.o
