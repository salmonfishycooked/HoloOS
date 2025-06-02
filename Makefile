MBR_SRC = boot/mbr.S
MBR_BIN = $(patsubst %.S, %.bin, $(MBR_SRC))

.PHONY: build run clean

build: $(MBR_BIN) hd60M.img
	dd if=boot/mbr.bin of=hd60M.img bs=512 count=1 conv=notrunc

run: build
	bochs -f bochsrc.disk

clean:
	rm -f boot/*.bin hd60M.img

boot/%.bin: boot/%.S
	nasm -f bin -o $@ $<

hd60M.img:
	bximage -hd=60M -func=create -imgmode="flat" -q hd60M.img
