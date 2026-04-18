; Synthax X - paging table buffers + 4-level identity map setup (first 2 MiB).
; Called from boot.asm while still in 32-bit protected mode.

bits 32

section .bss
align 4096
global pml4
global pdpt
global pd
pml4:
    resb 4096
pdpt:
    resb 4096
pd:
    resb 4096

section .text
global setup_page_tables

; void setup_page_tables(void)
;
; Writes:
;   PML4[0]  -> PDPT        (present | writable)
;   PDPT[0]  -> PD          (present | writable)
;   PD[0]    -> 0x00000000  as a 2 MiB page (present | writable | PS)
setup_page_tables:
    mov eax, pdpt
    or  eax, 0x3
    mov [pml4], eax
    mov dword [pml4 + 4], 0

    mov eax, pd
    or  eax, 0x3
    mov [pdpt], eax
    mov dword [pdpt + 4], 0

    mov dword [pd],     0x83        ; present | writable | 2 MiB page, base 0
    mov dword [pd + 4], 0

    ret
