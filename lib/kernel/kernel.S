[bits 32]

extern puts
extern idtTable

section .data
intrStr: db "interrupt occured!", 0xa, 0

global intrEntryTable
intrEntryTable:

%macro VECTOR 2
section .text
intr%1entry:
    %2

    ; save context
    push ds
    push es
    push fs
    push gs
    pushad

    ; if trap into intr by slave PIC, besides sending EOI to master pic,
    ; we also needs to send EOI to slave pic.
    mov al, 0x20                ; EOI
    out 0xa0, al                ; send to slave
    out 0x20, al                ; send to master

    push %1
    call [idtTable + %1 * 4]
    add esp, 4

    jmp intrExit

section .data
    dd intr%1entry
%endmacro

section .text
global intrExit
intrExit:
    popad
    pop gs
    pop fs
    pop es
    pop ds
    add esp, 4                  ; ignore ERROR_CODE

    iretd

VECTOR 0x00, push 0             ; push 0 for intrrupts not having ERROR_CODE
VECTOR 0x01, push 0
VECTOR 0x02, push 0
VECTOR 0x03, push 0
VECTOR 0x04, push 0
VECTOR 0x05, push 0
VECTOR 0x06, push 0
VECTOR 0x07, push 0
VECTOR 0x08, nop
VECTOR 0x09, push 0
VECTOR 0x0a, nop
VECTOR 0x0b, nop
VECTOR 0x0c, push 0
VECTOR 0x0d, nop
VECTOR 0x0e, nop
VECTOR 0x0f, push 0
VECTOR 0x10, push 0
VECTOR 0x11, nop
VECTOR 0x12, push 0
VECTOR 0x13, push 0
VECTOR 0x14, push 0
VECTOR 0x15, push 0
VECTOR 0x16, push 0
VECTOR 0x17, push 0
VECTOR 0x18, nop
VECTOR 0x19, push 0
VECTOR 0x1a, nop
VECTOR 0x1b, nop
VECTOR 0x1c, push 0
VECTOR 0x1d, nop
VECTOR 0x1e, nop                ; nop because 0x1e has ERROR_CODE
VECTOR 0x1f, push 0
VECTOR 0x20, push 0
VECTOR 0x21, push 0
VECTOR 0x22, push 0
VECTOR 0x23, push 0
VECTOR 0x24, push 0
VECTOR 0x25, push 0
VECTOR 0x26, push 0
VECTOR 0x27, push 0
VECTOR 0x28, push 0
VECTOR 0x29, push 0
VECTOR 0x2a, push 0
VECTOR 0x2b, push 0
VECTOR 0x2c, push 0
VECTOR 0x2d, push 0
VECTOR 0x2e, push 0
VECTOR 0x2f, push 0


[bits 32]
extern syscallTable
section .text
global syscallHandler
syscallHandler:
    push 0                      ; for unified format

    push ds
    push es
    push fs
    push gs

    pushad

    ; push 0x80                   ; for unified format

    push edx                    ; arg3
    push ecx                    ; arg2
    push ebx                    ; arg1

    call [syscallTable + eax * 4]
    add esp, 12                 ; release stack space occupied by arguments.

    mov [esp + 7 * 4], eax      ; override the value of eax in pushad when enter interrupt.
    jmp intrExit
