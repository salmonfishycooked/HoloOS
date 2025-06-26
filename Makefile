MBR_SRC = boot/mbr.S boot/loader.S
MBR_BIN = $(patsubst %.S, %.bin, $(MBR_SRC))

.PHONY: build run clean

build: $(MBR_BIN) kernel/kernel.bin hd60M.img
	dd if=boot/mbr.bin of=hd60M.img bs=512 count=1 seek=0 conv=notrunc
	dd if=boot/loader.bin of=hd60M.img bs=512 count=4 seek=2 conv=notrunc
	dd if=kernel/kernel.bin of=hd60M.img bs=512 count=200 seek=9 conv=notrunc

run: build
	bochs -f bochsrc.disk

clean:
	rm -f boot/*.bin hd60M.img
	rm -f kernel/*.o kernel/*.bin
	rm -f lib/kernel/*.o
	rm -f lib/kernel/c/*.o

boot/%.bin: boot/%.S
	nasm -f bin -o $@ -I boot/include/ $<

lib/kernel/%.o: lib/kernel/%.S
	nasm -f elf -o $@ $<

lib/kernel/c/%.o: lib/kernel/c/%.c
	gcc -m32 -I include/ -c -fno-builtin -o $@ $<

kernel/%.o: kernel/%.c
	gcc -m32 -I include/ -c -fno-builtin -o $@ $<

kernel/kernel.bin: kernel/main.o kernel/init.o kernel/interrupt.o lib/kernel/print.o \
					lib/kernel/c/printC.o lib/kernel/kernel.o
	ld -m elf_i386 -o $@ -Ttext 0xc0001500 -e main \
	kernel/main.o \
	lib/kernel/print.o \
	lib/kernel/c/printC.o \
	kernel/init.o \
	kernel/interrupt.o \
	lib/kernel/kernel.o

hd60M.img:
	bximage -hd=60M -func=create -imgmode="flat" -q hd60M.img
