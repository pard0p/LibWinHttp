CC_64=x86_64-w64-mingw32-gcc

all: libwinhttp.x64.zip

bin:
	mkdir bin

libwinhttp.x64.zip: bin
	$(CC_64) -DWIN_X64 -shared -masm=intel -Wall -Wno-pointer-arith -c src/HTTP.c -o bin/HTTP.x64.o
	zip -q -j libwinhttp.x64.zip bin/*.x64.o

clean:
	rm -rf bin/*.o
	rm -f libwinhttp.x64.zip