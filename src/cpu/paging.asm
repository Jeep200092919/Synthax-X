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
; Builds a 4-level identity map covering the first 64 MiB using 2 MiB pages.
; This is more than enough to contain the kernel image, the PMM bitmap
; (~1 MiB for 32 GiB of RAM), and any multiboot2 info GRUB leaves below.
;
;   PML4[0]  -> PDPT
;   PDPT[0]  -> PD
;   PD[0..31] -> 2 MiB pages at phys 0..64 MiB
setup_page_tables:
    mov eax, pdpt
    or  eax, 0x3
    mov [pml4], eax
    mov dword [pml4 + 4], 0

    mov eax, pd
    or  eax, 0x3
    mov [pdpt], eax
    mov dword [pdpt + 4], 0

    mov edi, pd
    mov eax, 0x83                   ; present | writable | 2 MiB page, base 0
    mov ecx, 32                     ; 32 * 2 MiB = 64 MiB
.map_loop:
    mov [edi], eax
    mov dword [edi + 4], 0
    add eax, 0x200000               ; next 2 MiB frame
    add edi, 8
    loop .map_loop

    ret
