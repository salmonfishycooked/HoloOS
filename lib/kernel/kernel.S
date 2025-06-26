[bits 32]

extern puts;

section .data
intrStr: db "interrupt occured!", 0xa, 0

global intrEntryTable
intrEntryTable:

%macro VECTOR 2
section .text
intr%1entry:
    %2
    push intrStr
    call puts
    add esp, 4

    ; if trap into intr by slave pic, besides sending EOI to master pic,
    ; also needs to send EOI to slave pic.
    mov al, 0x20                ; EOI
    out 0xa0, al                ; send to slave
    out 0x20, al                ; send to master

    add esp, 4                  ; ignore ERROR_CODE
    iret

section .data
    dd intr%1entry
%endmacro

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
