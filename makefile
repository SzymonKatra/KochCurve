CC=gcc
ASMBIN=nasm

all : dirs asm cc link
	
	
asm : 
	$(ASMBIN) -o obj/koch.o -f elf koch.asm
cc :
	$(CC) -m32 -c main.c -o obj/main.o -I/usr/include/i386-linux-gnu
link :
	$(CC) -m32 obj/main.o obj/koch.o -lallegro -lallegro_main -lallegro_primitives -lallegro_font -lallegro_ttf -o bin/main -B/usr/lib32
clean :
	rm obj/
	rm bin/

dirs :
	mkdir -p obj
	mkdir -p bin
	cp consola.ttf bin/consola.ttf