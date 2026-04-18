#ifndef SYNTHAX_CPU_GDT_H
#define SYNTHAX_CPU_GDT_H

#include <stdint.h>

#define GDT_ENTRIES 5

#define GDT_SEL_KERNEL_CODE 0x08
#define GDT_SEL_KERNEL_DATA 0x10
#define GDT_SEL_USER_CODE   0x18
#define GDT_SEL_USER_DATA   0x20

struct gdt_entry {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t  base_mid;
    uint8_t  access;
    uint8_t  flags_limit_high;
    uint8_t  base_high;
} __attribute__((packed));

struct gdt_ptr {
    uint16_t limit;
    uint64_t base;
} __attribute__((packed));

void gdt_init(void);

extern void gdt_flush(struct gdt_ptr *gp);

#endif
