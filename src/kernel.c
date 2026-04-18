#include <stdint.h>
#include "kernel.h"
#include "drivers/vga.h"
#include "cpu/gdt.h"

void kmain(uint32_t magic, uint32_t mb_info) {
    (void)magic;
    (void)mb_info;

    vga_set_color(VGA_WHITE, VGA_BLACK);
    vga_clear_screen();
    vga_write("Synthax X :: kernel online\n");

    gdt_init();
    vga_write("[ok] gdt loaded\n");

    for (;;) {
        __asm__ volatile ("hlt");
    }
}
