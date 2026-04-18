; Synthax X - GDT loader (long mode)
; Loads the GDT pointer passed in RDI and reloads all segment registers.
;
; void gdt_flush(struct gdt_ptr *gp);   ; SysV AMD64: gp -> RDI

bits 64

section .text
global gdt_flush

gdt_flush:
    lgdt [rdi]

    mov ax, 0x10            ; kernel data selector
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; Far return to reload CS with the kernel code selector (0x08).
    push qword 0x08
    lea  rax, [rel .reload_cs]
    push rax
    retfq

.reload_cs:
    ret
