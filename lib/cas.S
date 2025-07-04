section .text

; ----------------------------------------
; CAS32 compares value stored at addr to expected.
; if the value stored at addr equals to "expected", change it to newVal.
;
; prototype: bool compareAndSwap32(int *addr, int expected, int newVal)
; ret: old value (namely expected) if success;
;      current value if not success;
; ----------------------------------------
global CAS32
CAS32:
    mov eax, [esp + 8]
    mov edx, [esp + 12]
    mov ecx, [esp + 4]
    lock cmpxchg [ecx], edx
    ret

; CAS32 ENDS
; ----------------------------------------
