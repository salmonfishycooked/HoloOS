%include "boot.inc"

section LOADER vstart=LOADER_BASE_ADDR
    ; ======= get the cursor position
    ; dh: row of the cursor, dl: col of the cursor
    mov ah, 3
    mov bh, 0

    int 0x10

    ; ======= print the logo at a new line
    add dh, 1
    mov dl, 0

    mov bp, logoLoader

    mov cx, 0xd
    mov ax, 0x1301
    mov bx, 0x000f

    int 0x10

    ; make program stop here
    jmp $

    ; ------- some data
    logoLoader db "HoloOS Loader"

