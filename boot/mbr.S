; Main Boot Record
%include "boot.inc"

SECTION MBR vstart=0x7c00
    ; ======== initialize all registers
    mov ax, cs
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax

    mov sp, 0x7c00

    ; ======= clear out the screen
    mov ax, 0x0600
    mov bx, 0x0700
    mov cx, 0
    mov dx, 0x184f

    int 0x10

    ; ======= set the cursor to (0, 0)
    mov ax, 0x02
    mov bh, 0
    mov dx, 0

    int 0x10

    ; ======= print the logo
    mov bp, logo

    mov cx, 0x6
    mov ax, 0x1301
    mov bx, 0x000f

    int 0x10

    ; ======= read loader from disk
    mov edi, LOADER_BASE_ADDR
    mov esi, LOADER_START_SECTOR
    mov edx, 4
    call readFromDisk

    ; ======= hand over control to loader
    jmp LOADER_BASE_ADDR + 0x2fe

    ; ----------------------------------------
    ; readFromDisk reads data of sectors in Primary Disk (using LBA form)
    ;
    ; edi: start address where you want to store the read data from disk.
    ; esi: start sector number where you want to read from the disk.
    ; edx: number of sector that you want to read. ( <= 255 )
    ; ----------------------------------------
    readFromDisk:
        ; ========= backup some registers
        mov ecx, edx

        ; ========= 1. set number of sector that will be read
        mov dx, 0x1f2
        mov al, cl
        out dx, al

        ; ========= 2. set LBA and device register
        mov eax, esi
        ; set LBA Low 8bit
        mov dx, 0x1f3
        out dx, al

        ; set LBA Mid 8bit
        shr eax, 8
        mov dx, 0x1f4
        out dx, al

        ; set LBA High 8bit
        shr eax, 8
        mov dx, 0x1f5
        out dx, al

        ; set LBA left-most 4bit and device regs
        shr eax, 8
        and al, 00001111b
        or al, 11100000b         ; set 4~7 bit of device reg to 1110 (LBA Mode)
        mov dx, 0x1f6
        out dx, al

        ; ========= 3. set read command
        mov dx, 0x1f7
        mov al, 0x20
        out dx, al

        ; ========= 4. check if ready
        .notReady:
            in al, dx
            and al, 10001000b
            cmp al, 00001000b
            jnz .notReady

        ; ========= 5. read data from disk
        mov ax, 256
        mul cx
        mov cx, ax
        mov bx, di
        mov dx, 0x1f0
        .goOnRead:
            in ax, dx
            mov [bx], ax
            add bx, 2
            loop .goOnRead
        ret
        
    ; readFromDisk END
    ; ----------------------------------------

    ; ------- some data
    logo db "HoloOS"

    times 510 - ($ - $$) db 0
    db 0x55, 0xaa
