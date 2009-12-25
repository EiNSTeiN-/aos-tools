
CC=arm-linux-uclibcgnueabi-gcc

FLASH_OBJECTS=flash.o flashrw.o insmod.o rmmod.o files.o
MV_OBJECTS=mv.o
RM_OBJECTS=rm.o

all: flash mv rm

.c.o:
	$(CC) -c $*.c

flash: $(FLASH_OBJECTS)
	$(CC) -o flash $(FLASH_OBJECTS)

mv: $(MV_OBJECTS)
	$(CC) -o mv $(MV_OBJECTS)

rm: $(RM_OBJECTS)
	$(CC) -o rm $(RM_OBJECTS)

clean:
	rm -f flash mv rm *.o
