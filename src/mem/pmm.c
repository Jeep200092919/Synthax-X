#include <stdint.h>
#include "mem/pmm.h"

extern char _kernel_start[];
extern char _kernel_end[];

/* Multiboot2 structures (subset we need). */
struct mb2_tag_hdr {
    uint32_t type;
    uint32_t size;
};

#define MB2_TAG_END    0
#define MB2_TAG_MMAP   6

#define MB2_MMAP_AVAILABLE  1

struct mb2_mmap_tag {
    uint32_t type;
    uint32_t size;
    uint32_t entry_size;
    uint32_t entry_version;
    /* entries follow */
};

struct mb2_mmap_entry {
    uint64_t addr;
    uint64_t len;
    uint32_t type;
    uint32_t reserved;
};

static uint8_t  *bitmap;
static uint64_t  bitmap_bytes;
static uint64_t  total_mem;
static uint64_t  highest_page;

static inline void bit_set  (uint64_t i) { bitmap[i >> 3] |=  (uint8_t)(1u << (i & 7)); }
static inline void bit_clear(uint64_t i) { bitmap[i >> 3] &= (uint8_t)~(1u << (i & 7)); }
static inline int  bit_test (uint64_t i) { return bitmap[i >> 3] &  (1u << (i & 7)); }

static void mark_region_used(uint64_t addr, uint64_t len) {
    uint64_t start = addr / PMM_PAGE_SIZE;
    uint64_t end   = (addr + len + PMM_PAGE_SIZE - 1) / PMM_PAGE_SIZE;
    if (end > PMM_MAX_PAGES) end = PMM_MAX_PAGES;
    for (uint64_t i = start; i < end; i++) bit_set(i);
}

static void mark_region_free(uint64_t addr, uint64_t len) {
    uint64_t start = (addr + PMM_PAGE_SIZE - 1) / PMM_PAGE_SIZE;  /* round up */
    uint64_t end   = (addr + len) / PMM_PAGE_SIZE;                 /* round down */
    if (end > PMM_MAX_PAGES) end = PMM_MAX_PAGES;
    for (uint64_t i = start; i < end; i++) bit_clear(i);
    if (end > highest_page) highest_page = end;
}

void pmm_init(uintptr_t mb_addr) {
    /* Read the multiboot2 total size BEFORE placing the bitmap, so we can
     * be sure the bitmap doesn't land on top of the multiboot info blob. */
    uint32_t total_size = *(volatile uint32_t *)mb_addr;

    /* Bitmap goes after whichever is higher: kernel end or multiboot blob end. */
    uintptr_t k_end  = (uintptr_t)_kernel_end;
    uintptr_t mb_end = mb_addr + total_size;
    uintptr_t hi     = k_end > mb_end ? k_end : mb_end;
    uintptr_t bm     = (hi + PMM_PAGE_SIZE - 1) & ~(PMM_PAGE_SIZE - 1);

    bitmap       = (uint8_t *)bm;
    bitmap_bytes = PMM_BITMAP_BYTES;
    total_mem    = 0;
    highest_page = 0;

    /* Start with everything reserved; walk the mmap to punch holes of free RAM. */
    for (uint64_t i = 0; i < bitmap_bytes; i++) bitmap[i] = 0xFF;

    /* Walk multiboot2 tags. Layout: [total_size:u32][reserved:u32][tags...][end tag] */
    uintptr_t cursor = mb_addr + 8;
    uintptr_t limit  = mb_addr + total_size;

    while (cursor < limit) {
        struct mb2_tag_hdr *tag = (struct mb2_tag_hdr *)cursor;
        if (tag->type == MB2_TAG_END) break;

        if (tag->type == MB2_TAG_MMAP) {
            struct mb2_mmap_tag *m = (struct mb2_mmap_tag *)tag;
            uintptr_t e_ptr = cursor + sizeof(struct mb2_mmap_tag);
            uintptr_t e_end = cursor + tag->size;
            while (e_ptr + sizeof(struct mb2_mmap_entry) <= e_end) {
                struct mb2_mmap_entry *e = (struct mb2_mmap_entry *)e_ptr;
                if (e->type == MB2_MMAP_AVAILABLE) {
                    total_mem += e->len;
                    mark_region_free(e->addr, e->len);
                }
                e_ptr += m->entry_size;
            }
        }

        cursor += (tag->size + 7) & ~7ULL;   /* 8-byte align to next tag */
    }

    /* Reserve the low 1 MiB (BIOS / real-mode stubs / GRUB). */
    mark_region_used(0, 0x100000);

    /* Reserve the kernel image. */
    mark_region_used((uint64_t)(uintptr_t)_kernel_start,
                     (uint64_t)((uintptr_t)_kernel_end - (uintptr_t)_kernel_start));

    /* Reserve the bitmap storage itself. */
    mark_region_used(bm, bitmap_bytes);

    /* Reserve the multiboot2 info blob so we don't hand it back out. */
    mark_region_used(mb_addr, total_size);
}

void *pmm_alloc_block(void) {
    uint64_t scan_bytes = (highest_page + 7) >> 3;
    if (scan_bytes > bitmap_bytes) scan_bytes = bitmap_bytes;

    for (uint64_t byte = 0; byte < scan_bytes; byte++) {
        if (bitmap[byte] == 0xFF) continue;
        for (int bit = 0; bit < 8; bit++) {
            uint64_t idx = (byte << 3) + (uint64_t)bit;
            if (!bit_test(idx)) {
                bit_set(idx);
                return (void *)(uintptr_t)(idx * PMM_PAGE_SIZE);
            }
        }
    }
    return PMM_NO_BLOCK;
}

void pmm_free_block(void *addr) {
    uint64_t idx = (uintptr_t)addr / PMM_PAGE_SIZE;
    if (idx >= PMM_MAX_PAGES) return;
    bit_clear(idx);
}

uint64_t pmm_total_bytes(void) { return total_mem; }

uint64_t pmm_used_bytes(void) {
    uint64_t scan = (highest_page + 7) >> 3;
    if (scan > bitmap_bytes) scan = bitmap_bytes;
    uint64_t used = 0;
    for (uint64_t b = 0; b < scan; b++) {
        uint8_t v = bitmap[b];
        while (v) { used += (v & 1); v >>= 1; }
    }
    return used * PMM_PAGE_SIZE;
}

uint64_t pmm_free_bytes(void) {
    uint64_t total_managed = highest_page * PMM_PAGE_SIZE;
    uint64_t used = pmm_used_bytes();
    return total_managed > used ? total_managed - used : 0;
}

uintptr_t pmm_bitmap_addr(void) { return (uintptr_t)bitmap; }
