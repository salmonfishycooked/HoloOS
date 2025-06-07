%include "boot.inc"

LOADER_STACK_TOP equ LOADER_BASE_ADDR

section LOADER vstart=LOADER_BASE_ADDR
    jmp loaderStart

    ; --------- define GDT -----------
    GDT_BASE:       dd 0x00000000
                    dd 0x00000000
    CODE_DESC:      dd 0x0000FFFF
                    dd DESC_CODE_HIGH4
    DATA_DESC:      dd 0x0000FFFF
                    dd DESC_DATA_HIGH4
    VIDEO_DESC:     dd 0x80000003          ; (0xbffff - 0xb8000 + 0x1) / 4k - 1 = 0x3
                    dd DESC_VIDEO_HIGH4
    times 60 dq 0                          ; reserved space that can contain 60 gdt

    GDT_SIZE        equ $ - GDT_BASE
    GDT_LIMIT       equ GDT_SIZE - 1

    SELECTOR_CODE   equ (0x0001 << 3) + TI_GDT + RPL0
    SELECTOR_DATA   equ (0x0002 << 3) + TI_GDT + RPL0
    SELECTOR_VIDEO  equ (0x0003 << 3) + TI_GDT + RPL0

    gdtPtr:         dw GDT_LIMIT
                    dd GDT_BASE

    logoLoader:     db "HoloOS Loader"

    loaderStart:
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

        ; ======= turn on protection mode
        ; 1. turn on A20
        in al, 0x92
        or al, 00000010b
        out 0x92, al

        ; 2. load GDT
        lgdt [gdtPtr]

        ; 3. set 0th of cr0 to 1
        mov eax, cr0
        or eax, 0x00000001
        mov cr0, eax

        ; flush workflow
        jmp dword SELECTOR_CODE:pStart

    [bits 32]
    pStart:
        cli
        mov ax, SELECTOR_DATA
        mov ds, ax
        mov es, ax
        mov ss, ax
        mov esp, LOADER_STACK_TOP

        mov ax, SELECTOR_VIDEO
        mov gs, ax

        mov byte [gs:320], 'P'
        mov byte [gs:321], 0x0f

        jmp $
