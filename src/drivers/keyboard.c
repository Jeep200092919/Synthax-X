#include <stdint.h>
#include "drivers/keyboard.h"
#include "drivers/vga.h"
#include "cpu/io.h"

#define KBD_DATA 0x60

/* US QWERTY scancode set 1 -> ASCII. 0 means non-printable. */
static const char kbd_us_lower[128] = {
    0,   27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t','q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0,   'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'','`',
    0,   '\\','z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/',
    0,   '*',
    0,   ' ',
};

static const char kbd_us_upper[128] = {
    0,   27, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
    '\t','Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
    0,   'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~',
    0,   '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?',
    0,   '*',
    0,   ' ',
};

static int shift_held = 0;

void keyboard_handle_irq(void) {
    uint8_t sc = inb(KBD_DATA);

    /* Left shift press/release = 0x2A / 0xAA, right shift = 0x36 / 0xB6. */
    if (sc == 0x2A || sc == 0x36) { shift_held = 1; return; }
    if (sc == 0xAA || sc == 0xB6) { shift_held = 0; return; }

    /* Ignore other key releases (bit 7 set). */
    if (sc & 0x80) return;

    char c = shift_held ? kbd_us_upper[sc] : kbd_us_lower[sc];
    if (c) vga_putc(c);
}
