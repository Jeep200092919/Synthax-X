#include <stdint.h>
#include "drivers/vga.h"

static volatile uint16_t *const VGA_BUFFER = (volatile uint16_t *)0xB8000;

static uint8_t cur_color = 0x0F;
static uint8_t cur_row   = 0;
static uint8_t cur_col   = 0;

static inline uint16_t vga_entry(char c, uint8_t color) {
    return ((uint16_t)color << 8) | (uint8_t)c;
}

void vga_set_color(uint8_t fg, uint8_t bg) {
    cur_color = (uint8_t)((bg & 0x0F) << 4) | (fg & 0x0F);
}

void vga_clear_screen(void) {
    const uint16_t blank = vga_entry(' ', cur_color);
    for (uint16_t i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        VGA_BUFFER[i] = blank;
    }
    cur_row = 0;
    cur_col = 0;
}

static void vga_scroll(void) {
    for (uint16_t y = 1; y < VGA_HEIGHT; y++) {
        for (uint16_t x = 0; x < VGA_WIDTH; x++) {
            VGA_BUFFER[(y - 1) * VGA_WIDTH + x] = VGA_BUFFER[y * VGA_WIDTH + x];
        }
    }
    const uint16_t blank = vga_entry(' ', cur_color);
    for (uint16_t x = 0; x < VGA_WIDTH; x++) {
        VGA_BUFFER[(VGA_HEIGHT - 1) * VGA_WIDTH + x] = blank;
    }
    cur_row = VGA_HEIGHT - 1;
}

void vga_putc(char c) {
    if (c == '\n') {
        cur_col = 0;
        cur_row++;
    } else if (c == '\r') {
        cur_col = 0;
    } else if (c == '\t') {
        cur_col = (uint8_t)((cur_col + 4) & ~3);
    } else {
        VGA_BUFFER[cur_row * VGA_WIDTH + cur_col] = vga_entry(c, cur_color);
        cur_col++;
    }
    if (cur_col >= VGA_WIDTH) {
        cur_col = 0;
        cur_row++;
    }
    if (cur_row >= VGA_HEIGHT) {
        vga_scroll();
    }
}

void vga_write(const char *s) {
    while (*s) {
        vga_putc(*s++);
    }
}

void vga_write_dec(uint64_t n) {
    char buf[21];
    int i = 0;
    if (n == 0) { vga_putc('0'); return; }
    while (n > 0) {
        buf[i++] = (char)('0' + (n % 10));
        n /= 10;
    }
    while (i--) vga_putc(buf[i]);
}

void vga_write_hex(uint64_t n) {
    static const char digits[] = "0123456789abcdef";
    vga_write("0x");
    for (int i = 60; i >= 0; i -= 4) {
        vga_putc(digits[(n >> i) & 0xF]);
    }
}
