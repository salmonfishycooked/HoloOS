[bits 32]
section .text

; ----------------------------------
; switchTo changes context between two threads.
;
; prototype: switchTo(struct taskStruct *cur, struct taskStruct *next)
; no ret.
; ----------------------------------
global switchTo
switchTo:
    push esi
    push edi
    push ebx
    push ebp

    mov eax, [esp + 20]             ; get argument: cur
    mov [eax], esp

    ; ===== swap context to next thread
    mov eax, [esp + 24]             ; get argument: next
    mov esp, [eax]                  ; restore kernel stack pointer
    pop ebp
    pop ebx
    pop edi
    pop esi

    ret
