MBR_SRC = boot/mbr.S boot/loader.S
MBR_BIN = $(patsubst %.S, %.bin, $(MBR_SRC))

.PHONY: build run clean

build: $(MBR_BIN) hd60M.img
	dd if=boot/mbr.bin of=hd60M.img bs=512 count=1 seek=0 conv=notrunc
	dd if=boot/loader.bin of=hd60M.img bs=512 count=4 seek=2 conv=notrunc

run: build
	bochs -f bochsrc.disk

clean:
	rm -f boot/*.bin hd60M.img

boot/%.bin: boot/%.S
	nasm -f bin -o $@ -I boot/include/ $<

hd60M.img:
	bximage -hd=60M -func=create -imgmode="flat" -q hd60M.img
