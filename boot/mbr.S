; Main Boot Record
SECTION MBR vstart=0x7c00
    ; ======== initialize all registers
    mov ax, cs
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax

    mov sp, 0x7c00
    ; ========

    ; ======= clear out the screen
    mov ax, 0x0600
    mov bx, 0x0700
    mov cx, 0
    mov dx, 0x184f

    int 0x10
    ; =======

    ; ======= set the cursor to (0, 0)
    mov ax, 0x02
    mov bh, 0
    mov dx, 0

    int 0x10
    ; =======

    ; ======= print the logo
    mov bp, logo

    mov cx, 0x6
    mov ax, 0x1301
    mov bx, 0x000f

    int 0x10
    ; =======

    ; ======= make program stop here
    jmp $
    ; =======

    ; ======= some data
    logo db "HoloOS"

    times 510 - ($ - $$) db 0
    db 0x55, 0xaa
