#include <stdint.h>
#include "cpu/gdt.h"

static struct gdt_entry gdt[GDT_ENTRIES];
static struct gdt_ptr   gp;

static void gdt_set_gate(int idx, uint32_t base, uint32_t limit,
                         uint8_t access, uint8_t flags) {
    gdt[idx].base_low         = (uint16_t)(base & 0xFFFF);
    gdt[idx].base_mid         = (uint8_t)((base >> 16) & 0xFF);
    gdt[idx].base_high        = (uint8_t)((base >> 24) & 0xFF);
    gdt[idx].limit_low        = (uint16_t)(limit & 0xFFFF);
    gdt[idx].flags_limit_high = (uint8_t)(((limit >> 16) & 0x0F) | (flags & 0xF0));
    gdt[idx].access           = access;
}

void gdt_init(void) {
    gp.limit = (uint16_t)(sizeof(gdt) - 1);
    gp.base  = (uint64_t)(uintptr_t)&gdt;

    /* 0: null */
    gdt_set_gate(0, 0, 0, 0x00, 0x00);
    /* 1: kernel code (64-bit, ring 0) — L=1, G=1 */
    gdt_set_gate(1, 0, 0xFFFFF, 0x9A, 0xA0);
    /* 2: kernel data (ring 0) — G=1, D/B=1 */
    gdt_set_gate(2, 0, 0xFFFFF, 0x92, 0xC0);
    /* 3: user code (64-bit, ring 3) */
    gdt_set_gate(3, 0, 0xFFFFF, 0xFA, 0xA0);
    /* 4: user data (ring 3) */
    gdt_set_gate(4, 0, 0xFFFFF, 0xF2, 0xC0);

    gdt_flush(&gp);
}
