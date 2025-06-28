OS = holoOS

BUILD_DIR = ./build

ENTRY_POINT = 0xc0001500

AS = nasm
CC = gcc
LD = ld
LIB = -I include/
ASFLAGS = -f elf
CFLAGS = -m32 -Wall $(LIB) -c -fno-builtin -W -Wstrict-prototypes \
         -Wmissing-prototypes -DNDEBUG
LDFLAGS = -m elf_i386 -Ttext $(ENTRY_POINT) -e main -Map $(BUILD_DIR)/kernel.map

OBJS = $(BUILD_DIR)/kernel/main.o \
       $(BUILD_DIR)/kernel/init.o \
       $(BUILD_DIR)/kernel/interrupt.o \
       $(BUILD_DIR)/lib/kernel/kernel.o \
       $(BUILD_DIR)/lib/kernel/printC.o \
       $(BUILD_DIR)/lib/kernel/print.o \
       $(BUILD_DIR)/device/timer.o \
       $(BUILD_DIR)/kernel/debug.o \
       $(BUILD_DIR)/lib/string.o \
       $(BUILD_DIR)/kernel/bitmap.o \
       $(BUILD_DIR)/kernel/memory.o


# ------------ C Source Code Compile (to obj) ---------------
$(BUILD_DIR)/kernel/main.o: kernel/main.c
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/kernel/init.o: kernel/init.c
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/kernel/interrupt.o: kernel/interrupt.c
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/lib/kernel/printC.o: lib/kernel/printC.c
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/device/timer.o: device/timer.c
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/kernel/debug.o: kernel/debug.c
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/lib/string.o: lib/string.c
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/kernel/bitmap.o: kernel/bitmap.c
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/kernel/memory.o: kernel/memory.c
	$(CC) $(CFLAGS) $< -o $@


# ------------ ASM Source Code Compile (to obj) ---------------
$(BUILD_DIR)/lib/kernel/kernel.o: lib/kernel/kernel.S
	$(AS) $(ASFLAGS) $< -o $@

$(BUILD_DIR)/lib/kernel/print.o: lib/kernel/print.S
	$(AS) $(ASFLAGS) $< -o $@


# ------------ MBR and BootLoader  ------------------
$(BUILD_DIR)/boot/mbr.bin: boot/mbr.S
	$(AS) -f bin -I boot/include/ $< -o $@

$(BUILD_DIR)/boot/loader.bin: boot/loader.S
	$(AS) -f bin -I boot/include/ $< -o $@


# ---------------- Linking all objects -------------------
$(BUILD_DIR)/kernel.bin: $(OBJS)
	$(LD) $(LDFLAGS) $^ -o $@


# ---------------- create OS Image -------------------
$(OS).img:
	bximage -hd=60M -func=create -imgmode="flat" -q $(OS).img


# ----------------- Constructions --------------------
.PHONY: all run clean createImg createDir

createDir:
	mkdir -p $(BUILD_DIR)/kernel
	mkdir -p $(BUILD_DIR)/lib/kernel
	mkdir -p $(BUILD_DIR)/device
	mkdir -p $(BUILD_DIR)/boot

createImg: $(OS).img
	dd if=$(BUILD_DIR)/boot/mbr.bin of=$(OS).img bs=512 count=1 seek=0 conv=notrunc
	dd if=$(BUILD_DIR)/boot/loader.bin of=$(OS).img bs=512 count=4 seek=2 conv=notrunc
	dd if=$(BUILD_DIR)/kernel.bin of=$(OS).img bs=512 count=200 seek=9 conv=notrunc

clean:
	rm -f $(OS).img
	rm -rf $(BUILD_DIR)/

build: $(BUILD_DIR)/kernel.bin $(BUILD_DIR)/boot/mbr.bin $(BUILD_DIR)/boot/loader.bin

all: createDir build createImg

run: all
	bochs -f bochsrc.disk
