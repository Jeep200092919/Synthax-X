#include <stdint.h>
#include "kernel.h"

static volatile uint16_t *const VGA = (uint16_t *)0xB8000;

static void vga_clear(void) {
    for (int i = 0; i < 80 * 25; i++) {
        VGA[i] = (uint16_t)0x0F00 | ' ';
    }
}

static void vga_print(const char *s) {
    for (int i = 0; s[i] != '\0' && i < 80; i++) {
        VGA[i] = (uint16_t)0x0F00 | (uint8_t)s[i];
    }
}

void kmain(uint32_t magic, uint32_t mb_info) {
    (void)magic;
    (void)mb_info;
    vga_clear();
    vga_print("Synthax X :: kernel online");
    for (;;) {
        __asm__ volatile ("hlt");
    }
}
