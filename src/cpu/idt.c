#include <stdint.h>
#include "cpu/idt.h"
#include "cpu/pic.h"
#include "cpu/io.h"
#include "drivers/vga.h"
#include "drivers/keyboard.h"

static struct idt_entry idt[IDT_ENTRIES];
static struct idt_ptr   idtp;

#define KERNEL_CS    0x08
#define IDT_INT_GATE 0x8E   /* P=1, DPL=00, type=1110 (64-bit interrupt gate) */

#define DECL_ISR(n) extern void isr##n(void)
DECL_ISR(0);  DECL_ISR(1);  DECL_ISR(2);  DECL_ISR(3);
DECL_ISR(4);  DECL_ISR(5);  DECL_ISR(6);  DECL_ISR(7);
DECL_ISR(8);  DECL_ISR(9);  DECL_ISR(10); DECL_ISR(11);
DECL_ISR(12); DECL_ISR(13); DECL_ISR(14); DECL_ISR(15);
DECL_ISR(16); DECL_ISR(17); DECL_ISR(18); DECL_ISR(19);
DECL_ISR(20); DECL_ISR(21); DECL_ISR(22); DECL_ISR(23);
DECL_ISR(24); DECL_ISR(25); DECL_ISR(26); DECL_ISR(27);
DECL_ISR(28); DECL_ISR(29); DECL_ISR(30); DECL_ISR(31);

#define DECL_IRQ(n) extern void irq##n(void)
DECL_IRQ(0);  DECL_IRQ(1);  DECL_IRQ(2);  DECL_IRQ(3);
DECL_IRQ(4);  DECL_IRQ(5);  DECL_IRQ(6);  DECL_IRQ(7);
DECL_IRQ(8);  DECL_IRQ(9);  DECL_IRQ(10); DECL_IRQ(11);
DECL_IRQ(12); DECL_IRQ(13); DECL_IRQ(14); DECL_IRQ(15);

static void idt_set_gate(int num, uint64_t handler, uint16_t sel, uint8_t flags) {
    idt[num].offset_low  = (uint16_t)(handler & 0xFFFF);
    idt[num].selector    = sel;
    idt[num].ist         = 0;
    idt[num].type_attr   = flags;
    idt[num].offset_mid  = (uint16_t)((handler >> 16) & 0xFFFF);
    idt[num].offset_high = (uint32_t)((handler >> 32) & 0xFFFFFFFF);
    idt[num].reserved    = 0;
}

void idt_init(void) {
    uint8_t *p = (uint8_t *)idt;
    for (uint32_t i = 0; i < sizeof(idt); i++) p[i] = 0;

    idt_set_gate(0,  (uint64_t)isr0,  KERNEL_CS, IDT_INT_GATE);
    idt_set_gate(1,  (uint64_t)isr1,  KERNEL_CS, IDT_INT_GATE);
    idt_set_gate(2,  (uint64_t)isr2,  KERNEL_CS, IDT_INT_GATE);
    idt_set_gate(3,  (uint64_t)isr3,  KERNEL_CS, IDT_INT_GATE);
    idt_set_gate(4,  (uint64_t)isr4,  KERNEL_CS, IDT_INT_GATE);
    idt_set_gate(5,  (uint64_t)isr5,  KERNEL_CS, IDT_INT_GATE);
    idt_set_gate(6,  (uint64_t)isr6,  KERNEL_CS, IDT_INT_GATE);
    idt_set_gate(7,  (uint64_t)isr7,  KERNEL_CS, IDT_INT_GATE);
    idt_set_gate(8,  (uint64_t)isr8,  KERNEL_CS, IDT_INT_GATE);
    idt_set_gate(9,  (uint64_t)isr9,  KERNEL_CS, IDT_INT_GATE);
    idt_set_gate(10, (uint64_t)isr10, KERNEL_CS, IDT_INT_GATE);
    idt_set_gate(11, (uint64_t)isr11, KERNEL_CS, IDT_INT_GATE);
    idt_set_gate(12, (uint64_t)isr12, KERNEL_CS, IDT_INT_GATE);
    idt_set_gate(13, (uint64_t)isr13, KERNEL_CS, IDT_INT_GATE);
    idt_set_gate(14, (uint64_t)isr14, KERNEL_CS, IDT_INT_GATE);
    idt_set_gate(15, (uint64_t)isr15, KERNEL_CS, IDT_INT_GATE);
    idt_set_gate(16, (uint64_t)isr16, KERNEL_CS, IDT_INT_GATE);
    idt_set_gate(17, (uint64_t)isr17, KERNEL_CS, IDT_INT_GATE);
    idt_set_gate(18, (uint64_t)isr18, KERNEL_CS, IDT_INT_GATE);
    idt_set_gate(19, (uint64_t)isr19, KERNEL_CS, IDT_INT_GATE);
    idt_set_gate(20, (uint64_t)isr20, KERNEL_CS, IDT_INT_GATE);
    idt_set_gate(21, (uint64_t)isr21, KERNEL_CS, IDT_INT_GATE);
    idt_set_gate(22, (uint64_t)isr22, KERNEL_CS, IDT_INT_GATE);
    idt_set_gate(23, (uint64_t)isr23, KERNEL_CS, IDT_INT_GATE);
    idt_set_gate(24, (uint64_t)isr24, KERNEL_CS, IDT_INT_GATE);
    idt_set_gate(25, (uint64_t)isr25, KERNEL_CS, IDT_INT_GATE);
    idt_set_gate(26, (uint64_t)isr26, KERNEL_CS, IDT_INT_GATE);
    idt_set_gate(27, (uint64_t)isr27, KERNEL_CS, IDT_INT_GATE);
    idt_set_gate(28, (uint64_t)isr28, KERNEL_CS, IDT_INT_GATE);
    idt_set_gate(29, (uint64_t)isr29, KERNEL_CS, IDT_INT_GATE);
    idt_set_gate(30, (uint64_t)isr30, KERNEL_CS, IDT_INT_GATE);
    idt_set_gate(31, (uint64_t)isr31, KERNEL_CS, IDT_INT_GATE);

    idt_set_gate(32, (uint64_t)irq0,  KERNEL_CS, IDT_INT_GATE);
    idt_set_gate(33, (uint64_t)irq1,  KERNEL_CS, IDT_INT_GATE);
    idt_set_gate(34, (uint64_t)irq2,  KERNEL_CS, IDT_INT_GATE);
    idt_set_gate(35, (uint64_t)irq3,  KERNEL_CS, IDT_INT_GATE);
    idt_set_gate(36, (uint64_t)irq4,  KERNEL_CS, IDT_INT_GATE);
    idt_set_gate(37, (uint64_t)irq5,  KERNEL_CS, IDT_INT_GATE);
    idt_set_gate(38, (uint64_t)irq6,  KERNEL_CS, IDT_INT_GATE);
    idt_set_gate(39, (uint64_t)irq7,  KERNEL_CS, IDT_INT_GATE);
    idt_set_gate(40, (uint64_t)irq8,  KERNEL_CS, IDT_INT_GATE);
    idt_set_gate(41, (uint64_t)irq9,  KERNEL_CS, IDT_INT_GATE);
    idt_set_gate(42, (uint64_t)irq10, KERNEL_CS, IDT_INT_GATE);
    idt_set_gate(43, (uint64_t)irq11, KERNEL_CS, IDT_INT_GATE);
    idt_set_gate(44, (uint64_t)irq12, KERNEL_CS, IDT_INT_GATE);
    idt_set_gate(45, (uint64_t)irq13, KERNEL_CS, IDT_INT_GATE);
    idt_set_gate(46, (uint64_t)irq14, KERNEL_CS, IDT_INT_GATE);
    idt_set_gate(47, (uint64_t)irq15, KERNEL_CS, IDT_INT_GATE);

    pic_remap(0x20, 0x28);
    /* Mask everything, then unmask cascade (IRQ2) and keyboard (IRQ1). */
    pic_set_mask(0xFFFF & ~((1 << 1) | (1 << 2)));

    idtp.limit = (uint16_t)(sizeof(idt) - 1);
    idtp.base  = (uint64_t)(uintptr_t)&idt;

    __asm__ volatile ("lidt (%0)" : : "r"(&idtp) : "memory");
}

void isr_handler_c(struct registers *r) {
    (void)r;
    vga_write("\n[exception] cpu fault; halting\n");
    for (;;) __asm__ volatile ("cli; hlt");
}

void irq_handler_c(struct registers *r) {
    uint8_t irq = (uint8_t)(r->int_no - 32);
    if (irq == 1) {
        keyboard_handle_irq();
    }
    pic_send_eoi(irq);
}
