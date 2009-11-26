
FLASH_OBJECTS=flash.o flashrw.o insmod.o rmmod.o files.o
MV_OBJECTS=mv.o
RM_OBJECTS=rm.o

all: flash mv rm

.c.o:
	arm-linux-uclibcgnueabi-gcc -c $*.c

flash: $(FLASH_OBJECTS)
	arm-linux-uclibcgnueabi-gcc -o flash $(FLASH_OBJECTS)

mv: $(MV_OBJECTS)
	arm-linux-uclibcgnueabi-gcc -o mv $(MV_OBJECTS)

rm: $(RM_OBJECTS)
	arm-linux-uclibcgnueabi-gcc -o rm $(RM_OBJECTS)

clean:
	rm -f flash mv rm *.o
