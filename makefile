CC=gcc
ASMBIN=nasm

all : dirs asm cc link
	
	
asm : 
	$(ASMBIN) -o obj/koch.o -f elf koch.asm
cc :
	$(CC) -m32 -c main.c -o obj/main.o -I/usr/include/SDL2/
link :
	$(CC) -m32 obj/main.o obj/koch.o -lSDL2 -o bin/main -B/usr/lib32
clean :
	rm obj/
	rm bin/

dirs :
	mkdir -p obj
	mkdir -p bin