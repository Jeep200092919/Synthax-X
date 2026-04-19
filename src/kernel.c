#include <stdint.h>
#include "kernel.h"
#include "drivers/vga.h"
#include "cpu/gdt.h"
#include "cpu/idt.h"
#include "mem/pmm.h"
#include "mem/vmm.h"
#include "mem/heap.h"
#include "shell/shell.h"

#define MB2_BOOTLOADER_MAGIC 0x36d76289u
#define HEAP_VIRT_BASE       0x40000000ULL            /* 1 GiB */
#define HEAP_BYTES           (16ULL * 1024 * 1024)    /* 16 MiB */

static void kheap_setup(void) {
    for (uintptr_t v = HEAP_VIRT_BASE; v < HEAP_VIRT_BASE + HEAP_BYTES; v += 4096) {
        void *frame = pmm_alloc_block();
        if (!frame) {
            vga_write("[fatal] pmm out of memory during heap setup\n");
            for (;;) __asm__ volatile ("cli; hlt");
        }
        if (vmm_map_page(v, (uintptr_t)frame, VMM_FLAGS_KRW) != 0) {
            vga_write("[fatal] vmm_map_page failed\n");
            for (;;) __asm__ volatile ("cli; hlt");
        }
    }
    heap_init(HEAP_VIRT_BASE, HEAP_BYTES);
}

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
    vga_write("Memory Detected: ");
    vga_write_dec(pmm_total_bytes() / (1024ULL * 1024ULL));
    vga_write(" MB\n");

    vmm_init();
    vga_write("[ok] vmm initialized\n");

    kheap_setup();
    vga_write("[ok] heap ready\n");

    shell_init();
    shell_run();
}
