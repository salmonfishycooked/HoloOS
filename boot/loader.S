%include "boot.inc"

LOADER_STACK_TOP equ LOADER_BASE_ADDR

section LOADER vstart=LOADER_BASE_ADDR
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

    totalMemBytes:  dd 0
    ardsBuf:        times 244 db 0
    ardsCnt:        dw 0

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

        ; ======= get available memory
        call getMemCap
        mov [totalMemBytes], eax

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

        ; ===== read kernel.bin into memory
        mov edi, KERNEL_BIN_BASE_ADDR
        mov esi, KERNEL_START_SECTOR
        mov edx, 200
        call readFromDisk64

        ; ===== setup page table
        call pageSetup

        ; ===== do sth necessary and turn on paging mechanism
        add dword [VIDEO_DESC + 4], 0xc0000000
        add esp, 0xc0000000
        add dword [gdtPtr + 2], 0xc0000000

        ; set Page Directory Table's address to cr3
        mov eax, PAGE_DIR_TABLE_ADDR
        mov cr3, eax

        ; turn on pg bit of cr0 (the 31st bit)
        mov eax, cr0
        or eax, 0x80000000
        mov cr0, eax

        ; reload gdtPtr
        lgdt [gdtPtr]

        jmp SELECTOR_CODE:enterKernel

    enterKernel:
        ; ===== init kernel.bin
        call kernelInit

        mov byte [gs:480], 'V'
        mov byte [gs:481], 0x0f
        
        ; ===== hand over control flow to kernel
        mov esp, 0xc009f000
        jmp KERNEL_ENTRY_POINT


    ; ---------------------------------
    ; kernelInit will initialize kernel.bin to physical memory
    ; no args
    ; no ret
    ; --------------------------------
    [bits 32]
    kernelInit:
        xor eax, eax
        xor ebx, ebx
        xor ecx, ecx
        xor edx, edx

        mov ebx, [KERNEL_BIN_BASE_ADDR + 28]        ; e_phoff
        mov dx, [KERNEL_BIN_BASE_ADDR + 42]        ; e_phentsize
        mov cx, [KERNEL_BIN_BASE_ADDR + 44]        ; e_phnum

        add ebx, KERNEL_BIN_BASE_ADDR               ; real address of program header
        .kernelInitCopySeg:
            cmp byte [ebx], 0
            je .kernelInitPtNull

            ; do memcpy(dst, src, size)
            push dword [ebx + 16]

            mov eax, [ebx + 4]
            add eax, KERNEL_BIN_BASE_ADDR
            push eax

            push dword [ebx + 8]
            call memcpy

            add esp, 12
            .kernelInitPtNull:
                add ebx, edx
                loop .kernelInitCopySeg

        ret

    ; kernelInit ENDS
    ; ---------------------------------


    ; ---------------------------------
    ; memcpy copys data from src to dst
    ; the number of copied data is size (unit: byte)
    ;
    ; args: (dst, src, size) (push to stack in order)
    ; no ret
    ; ---------------------------------
    memcpy:
        cld
        push ebp
        mov ebp, esp

        push ecx

        mov edi, [ebp + 8]
        mov esi, [ebp + 12]
        mov ecx, [ebp + 16]
        rep movsb

        pop ecx
        pop ebp
        ret

    ; memcpy ENDS
    ; ---------------------------------


    ; ---------------------------------
    ; pageSetup initializes things about paging mechanism
    ; no arguments
    ; no return value
    ; ---------------------------------
    [bits 32]
    pageSetup:
        ; ===== clear Page Directory Table
        mov ecx, 1024
        mov esi, 0
        .pageSetupClear:
            mov dword [PAGE_DIR_TABLE_ADDR + esi], 0
            add esi, 4
            loop .pageSetupClear

        ; ===== init Page Directory Entry
        mov eax, PAGE_DIR_TABLE_ADDR + 0x1000
        or eax, PG_US_U | PG_RW_W | PG_P
        mov [PAGE_DIR_TABLE_ADDR], eax
        mov [PAGE_DIR_TABLE_ADDR + 0x300 * 4], eax      ; the first pde of kernel

        ; ===== init Page Table Entry (lower 1MB)
        mov eax, 0
        or eax, PG_US_U | PG_RW_W | PG_P
        mov esi, 0
        mov ecx, 256
        .pageSetupInitPTE:
            mov [PAGE_DIR_TABLE_ADDR + 0x1000 + esi * 4], eax
            inc esi
            add eax, 0x1000
            loop .pageSetupInitPTE

        ; ===== init remaining Page Directory Entry of kernel (from pde idx 0x301 to 0x3fe)
        mov eax, PAGE_DIR_TABLE_ADDR + 0x2000
        or eax, PG_US_U | PG_RW_W | PG_P
        mov esi, 0x301
        mov ecx, 0xfe
        .pageSetupInitKernelPDE:
            mov [PAGE_DIR_TABLE_ADDR + esi * 4], eax
            inc esi
            add eax, 0x1000
            loop .pageSetupInitKernelPDE

        ; let the last pde point to PAGE_DIR_TABLE_ADDR
        mov eax, PAGE_DIR_TABLE_ADDR
        or eax, PG_US_U | PG_RW_W | PG_P
        mov dword [PAGE_DIR_TABLE_ADDR + 0x3ff * 4], eax

        ret

    ; pageSetup ENDS
    ; ---------------------------------


    ; ---------------------------------
    ; getMemCap will get the information of memory you can use
    ; no arguments
    ;
    ; return value: eax (stores the upper bound of memory address)
    ; ---------------------------------
    [bits 16]
    getMemCap:
        .getMemE820:
            mov ax, 0xe820
            mov ebx, 0
            mov di, ardsBuf
            mov ecx, 20
            mov edx, 0x534d4150
            .loopGetArds:
                int 0x15
                jc .getMemE801

                add di, 20
                add word [ardsCnt], 1
                mov eax, 0xe820

                cmp ebx, 0
                jnz .loopGetArds

            mov cx, [ardsCnt]
            mov bx, ardsBuf
            mov eax, 0
            .loopCalMaxMem:
                mov edx, [bx]
                add edx, [bx + 8]
                cmp eax, edx
                jae .goNextArd
                mov eax, edx

                .goNextArd:
                add bx, 20
                loop .loopCalMaxMem
            jmp .getMemOK

        .getMemE801:
            mov ax, 0xe801
            int 0x15
            jc .getMem88

            mov dx, 1024
            mul dx
            shl edx, 16
            and eax, 0x0000ffff
            or edx, eax
            add edx, 1024 * 1024
            mov esi, edx

            mov eax, 64 * 1024
            and ebx, 0x0000ffff
            mul ebx
            add esi, eax

            mov eax, esi
            jmp .getMemOK

        .getMem88:
            mov ah, 0x88
            int 0x15
            jc errorHandle

            mov ecx, 1024
            and eax, 0x0000ffff
            mul ecx
            add eax, 1024 * 1024

        .getMemOK:
            ret

    ; getMemCap ENDS
    ; ----------------------


    ; ----------------------------------------
    ; readFromDisk64 reads data of sectors in Primary Disk (using LBA form)
    ;
    ; edi: start address where you want to store the read data from disk.
    ; esi: start sector number where you want to read from the disk.
    ; edx: number of sector that you want to read. 
    ; ----------------------------------------
    [bits 32]
    readFromDisk64:
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
        mov eax, 256
        mul ecx
        mov ecx, eax
        mov dx, 0x1f0
        .goOnRead:
            in ax, dx
            mov [edi], ax
            add edi, 2
            loop .goOnRead
        ret
        
    ; readFromDisk END
    ; ----------------------------------------


    ; ----------------------
    ; error handler
    ; ----------------------
    [bits 16]
    errorHandle:
        jmp $
            

    ; ------ some data
    logoLoader: db "HoloOS Loader"
