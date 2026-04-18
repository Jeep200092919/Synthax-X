; Synthax X - Multiboot2 boot stub
; Assembled with NASM for x86_64

section .multiboot_header
align 8
header_start:
    dd 0xe85250d6                ; magic (multiboot2)
    dd 0                         ; architecture i386 (protected mode)
    dd header_end - header_start ; header length
    dd 0x100000000 - (0xe85250d6 + 0 + (header_end - header_start)) ; checksum

    ; end tag
    dw 0                         ; type
    dw 0                         ; flags
    dd 8                         ; size
header_end:

section .bss
align 16
stack_bottom:
    resb 16384                   ; 16 KiB stack
stack_top:

section .text
bits 32
global _start
extern kmain

_start:
    mov esp, stack_top
    push ebx                     ; multiboot info pointer
    push eax                     ; multiboot magic
    call kmain

.hang:
    cli
    hlt
    jmp .hang
