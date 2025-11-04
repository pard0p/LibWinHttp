CC=i686-w64-mingw32-gcc
CC_64=x86_64-w64-mingw32-gcc

all: bin/loader.x86.o bin/loader.x64.o

bin:
	mkdir bin

#
# x86 targets
#
bin/loader.x86.o: bin
	$(CC) -DWIN_X86 -shared -masm=intel -Wall -Wno-pointer-arith -c src/loader.c -o bin/loader.x86.o

#
# x64 targets
#
bin/loader.x64.o: bin
	$(CC_64) -DWIN_X64 -shared -masm=intel -Wall -Wno-pointer-arith -c src/loader.c -o bin/loader.x64.o

#
# Other targets
#
clean:
	rm -f bin/*
