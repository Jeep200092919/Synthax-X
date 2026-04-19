#include <stdint.h>
#include "drivers/keyboard.h"
#include "cpu/io.h"

#define KBD_DATA 0x60

#define RING_SIZE 256
#define RING_MASK (RING_SIZE - 1)

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

static volatile char     ring[RING_SIZE];
static volatile uint32_t ring_head;
static volatile uint32_t ring_tail;

void keyboard_handle_irq(void) {
    uint8_t sc = inb(KBD_DATA);

    if (sc == 0x2A || sc == 0x36) { shift_held = 1; return; }
    if (sc == 0xAA || sc == 0xB6) { shift_held = 0; return; }
    if (sc & 0x80) return;                  /* ignore other key releases */

    char c = shift_held ? kbd_us_upper[sc] : kbd_us_lower[sc];
    if (!c) return;

    uint32_t next = (ring_head + 1) & RING_MASK;
    if (next != ring_tail) {                /* drop char on overflow */
        ring[ring_head] = c;
        ring_head = next;
    }
}

char keyboard_getchar(void) {
    for (;;) {
        __asm__ volatile ("cli");
        if (ring_head != ring_tail) {
            char c = ring[ring_tail];
            ring_tail = (ring_tail + 1) & RING_MASK;
            __asm__ volatile ("sti");
            return c;
        }
        /* sti+hlt is atomic on x86: IRQ is serviced at the hlt boundary. */
        __asm__ volatile ("sti; hlt");
    }
}
