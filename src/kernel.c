#include <stdint.h>
#include "kernel.h"
#include "drivers/vga.h"
#include "cpu/gdt.h"
#include "cpu/idt.h"
#include "mem/pmm.h"

#define MB2_BOOTLOADER_MAGIC 0x36d76289u

void kmain(uint64_t magic, uintptr_t mb_info) {
    vga_set_color(VGA_WHITE, VGA_BLACK);
    vga_clear_screen();
    vga_write("Synthax X :: kernel online\n");

    gdt_init();
    vga_write("[ok] gdt loaded\n");

    idt_init();
    __asm__ volatile ("sti");
    vga_write("[ok] interrupts enabled\n");

    if ((uint32_t)magic != MB2_BOOTLOADER_MAGIC) {
        vga_write("[warn] multiboot2 magic mismatch\n");
    }

    pmm_init(mb_info);
    vga_write("[ok] pmm initialized\n");

    uint64_t mb = pmm_total_bytes() / (1024ULL * 1024ULL);
    vga_write("Memory Detected: ");
    vga_write_dec(mb);
    vga_write(" MB\n");

    vga_write("> ");

    for (;;) {
        __asm__ volatile ("hlt");
    }
}
