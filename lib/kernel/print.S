SELECTOR_VIDEO       equ (0x0003 << 3)
CHAR_ATTR_DEFAULT    equ 0x0f

[bits 32]
section .text

; ------------------------------------------
; putchar puts 1 character to the position of the cursor
;
; prototype: putchar(uint8 ch)
; no ret
; ------------------------------------------
global putchar
putchar:
    pushad

    mov ax, SELECTOR_VIDEO
    mov gs, ax

    ; ==== get the position of the cursor
    mov dx, 0x3d4
    mov al, 0x0e
    out dx, al
    mov dx, 0x3d5
    in al, dx
    mov ah, al

    mov dx, 0x3d4
    mov al, 0x0f
    out dx, al
    mov dx, 0x3d5
    in al, dx

    mov bx, ax              ; store into bx

    ; ==== get the args, and show it up the screen
    mov ecx, [esp + 36]

    cmp cl, 0xa
    jz .isLineFeed
    cmp cl, 0xd
    jz .isCarriageReturn
    cmp cl, 0x8
    jz .isBackspace

    jmp .putIt

    ; ==== encounter Backspace
    .isBackspace:
        dec bx
        shl bx, 1

        mov byte [gs:bx], 0
        mov byte [gs:bx + 1], CHAR_ATTR_DEFAULT

        shr bx, 1
        jmp .setCursor

    ; ==== put the character
    .putIt:
        shl bx, 1

        mov [gs:bx], cl
        mov byte [gs:bx + 1], CHAR_ATTR_DEFAULT

        shr bx, 1
        inc bx

        cmp bx, 2000
        jl .setCursor

    ; ==== encounter CR or LF
    .isCarriageReturn:
    .isLineFeed:
        xor dx, dx
        mov ax, bx

        mov cx, 80
        div cx
        sub bx, dx

        add bx, 80
        cmp bx, 2000
        jl .setCursor

    ; ==== scroll up the screen
    .scrollUp:
        ; ==== clear the top line, and move below lines up a line.
        mov edi, 0xc000b800
        mov esi, 0xc000b8a0
        mov ecx, 960
        rep movsd

        ; ==== set the bottom-most line to blank
        mov eax, 3840
        mov ecx, 80
        .clearBottomLine:
            mov byte [gs:eax], 0
            add eax, 2
            loop .clearBottomLine
        mov bx, 1920

    ; ==== update the latest position of the cursor into graphic processor
    .setCursor:
        mov dx, 0x3d4
        mov al, 0x0e
        out dx, al
        mov dx, 0x3d5
        mov al, bh
        out dx, al

        mov dx, 0x3d4
        mov al, 0x0f
        out dx, al
        mov dx, 0x3d5
        mov al, bl
        out dx, al

    .putDone:
        popad
        ret

; putchar ENDS
; -------------------------------------

; -------------------------------------
; puts will input a string onto screen
;
; prototype: void puts(char *str);
; no ret
; -------------------------------------
global puts
puts:
    ; ==== get the address of string
    mov esi, [esp + 4]

    .outChar:
        mov cl, [esi]
        cmp cl, 0
        jz .putsOver

        push ecx
        call putchar
        add esp, 4

        inc esi
        jmp .outChar

    .putsOver:
    ret

; puts ENDS
; ------------------------------------
