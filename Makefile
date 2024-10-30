all: xv6fs fs.img

xv6fs: ./src ./inc
	cmake -S . -B build
	cmake --build build

fs.img: mkfs/mkfs README.md
	mkfs/mkfs fs.img README.md

mkfs/mkfs: mkfs/mkfs.c
	gcc -Wall -Werror -I. -o mkfs/mkfs mkfs/mkfs.c

run: xv6fs fs.img
	./build/bin/xv6fs xv6fs.log ./fs.img ./mnt/

clean:
	rm -rf ./build 
	rm -rf ./fs.img
	rm -rf ./mkfs/mkfs
	rm -rf ./xv6fs.log
