#include <stdint.h>
#include "cpu/pic.h"
#include "cpu/io.h"

#define ICW1_INIT 0x10
#define ICW1_ICW4 0x01
#define ICW4_8086 0x01

void pic_remap(uint8_t offset1, uint8_t offset2) {
    uint8_t m1 = inb(PIC1_DATA);
    uint8_t m2 = inb(PIC2_DATA);

    outb(PIC1_CMD,  ICW1_INIT | ICW1_ICW4); io_wait();
    outb(PIC2_CMD,  ICW1_INIT | ICW1_ICW4); io_wait();
    outb(PIC1_DATA, offset1);               io_wait();
    outb(PIC2_DATA, offset2);               io_wait();
    outb(PIC1_DATA, 0x04);                  io_wait();  /* master has slave on IRQ2 */
    outb(PIC2_DATA, 0x02);                  io_wait();  /* slave cascade identity */
    outb(PIC1_DATA, ICW4_8086);             io_wait();
    outb(PIC2_DATA, ICW4_8086);             io_wait();

    outb(PIC1_DATA, m1);
    outb(PIC2_DATA, m2);
}

void pic_send_eoi(uint8_t irq) {
    if (irq >= 8) outb(PIC2_CMD, PIC_EOI);
    outb(PIC1_CMD, PIC_EOI);
}

void pic_set_mask(uint16_t mask) {
    outb(PIC1_DATA, (uint8_t)(mask & 0xFF));
    outb(PIC2_DATA, (uint8_t)((mask >> 8) & 0xFF));
}
