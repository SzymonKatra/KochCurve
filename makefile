CC=gcc
ASMBIN=nasm

all : 64

64  : dirs asm64 cc64 link64
86  : dirs asm86 cc86 link86
	
asm86 : 
	$(ASMBIN) -o obj/koch86.o -f elf koch86.asm
asm64 :
	$(ASMBIN) -o obj/koch64.o -f elf64 koch64.asm
    
cc86 :
	$(CC) -m32 -c main.c -o obj/main86.o
cc64:
	$(CC) -m64 -c main.c -o obj/main64.o
    
link86 :
	$(CC) -m32 obj/main86.o obj/koch86.o -lallegro -lallegro_main -lallegro_primitives -lallegro_font -lallegro_ttf -o bin/main86
link64:
	$(CC) -m64 obj/main64.o obj/koch64.o -lallegro -lallegro_main -lallegro_primitives -lallegro_font -lallegro_ttf -o bin/main64

clean :
	rm obj/
	rm bin/

dirs :
	mkdir -p obj
	mkdir -p bin
	cp consola.ttf bin/consola.ttf