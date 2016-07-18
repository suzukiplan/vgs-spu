all:
	make all-`uname`

all-Darwin:
	gcc -O2 -I./src -c ./src/vgsspu_al.c
	ar ruv vgsspu.a vgsspu_al.o

all-Linux:
	gcc -O2 -I./src -c ./src/vgsspu_alsa.c
	ar ruv vgsspu.a vgsspu_alsa.o

