; Synthax X - multiboot2 entry + long-mode transition.
;
; Flow:
;   GRUB hands control in 32-bit protected mode.
;   Set up PAE paging (identity map first 2 MiB via setup_page_tables).
;   Enable PAE, load CR3, set EFER.LME, enable CR0.PG.
;   Load a minimal 64-bit GDT, far-jump to the 64-bit code segment.
;   In 64-bit mode, reset segments, set RSP, call kmain(magic, mb_info).

section .multiboot_header
align 8
header_start:
    dd 0xe85250d6                            ; magic (multiboot2)
    dd 0                                     ; architecture: i386 protected
    dd header_end - header_start             ; header length
    dd 0x100000000 - (0xe85250d6 + 0 + (header_end - header_start)) ; checksum
    dw 0
    dw 0
    dd 8
header_end:

section .bss
align 16
stack_bottom:
    resb 16384
stack_top:

section .rodata
align 8
gdt64:
.null: equ $ - gdt64
    dq 0
.code: equ $ - gdt64
    ; present | code/data | executable | 64-bit code
    dq (1 << 43) | (1 << 44) | (1 << 47) | (1 << 53)
.pointer:
    dw $ - gdt64 - 1
    dd gdt64                                 ; 32-bit base (loaded while in PM)

section .text
bits 32
global _start
extern kmain
extern setup_page_tables
extern pml4

_start:
    mov esp, stack_top

    ; Stash multiboot magic (eax) and info pointer (ebx) into edi/esi so that
    ; after the long-mode transition they naturally sit in rdi/rsi for the
    ; System V AMD64 call to kmain(magic, mb_info).
    mov edi, eax
    mov esi, ebx

    call setup_page_tables

    mov eax, pml4
    mov cr3, eax

    mov eax, cr4
    or  eax, 1 << 5                          ; CR4.PAE
    mov cr4, eax

    mov ecx, 0xC0000080                      ; IA32_EFER
    rdmsr
    or  eax, 1 << 8                          ; EFER.LME
    wrmsr

    mov eax, cr0
    or  eax, 1 << 31                         ; CR0.PG
    mov cr0, eax

    lgdt [gdt64.pointer]
    jmp  gdt64.code:long_mode_start

bits 64
long_mode_start:
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    mov rsp, stack_top

    call kmain

.hang:
    cli
    hlt
    jmp .hang
