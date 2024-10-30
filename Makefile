all: xv6fs fs.img

xv6fs:
	cmake -S . -B build -G Ninja
	cmake --build build

fs.img: mkfs/mkfs README.md
	mkfs/mkfs fs.img README.md

mkfs/mkfs: mkfs/mkfs.c
	gcc -Wall -Werror -I. -o mkfs/mkfs mkfs/mkfs.c

clean:
	rm -rf build fs.img mkfs/mkfs
