fs.img: mkfs/mkfs README.md
	mkfs/mkfs fs.img README.md

mkfs/mkfs: mkfs/mkfs.c
	gcc -Werror -Wall -I. -o mkfs/mkfs mkfs/mkfs.c
