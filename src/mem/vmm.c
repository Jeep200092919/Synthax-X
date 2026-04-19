#include <stdint.h>
#include <stddef.h>
#include "mem/vmm.h"
#include "mem/pmm.h"

/* The PML4 set up by paging.asm at boot. */
extern uint64_t pml4[];

#define PT_ADDR_MASK   0x000FFFFFFFFFF000ULL
#define PTE_FLAGS_MASK 0xFFFULL

static inline size_t idx_pml4(uintptr_t v) { return (v >> 39) & 0x1FF; }
static inline size_t idx_pdpt(uintptr_t v) { return (v >> 30) & 0x1FF; }
static inline size_t idx_pd  (uintptr_t v) { return (v >> 21) & 0x1FF; }
static inline size_t idx_pt  (uintptr_t v) { return (v >> 12) & 0x1FF; }

/* Page tables live within the first 64 MiB identity map, so phys == virt for them. */
static inline uint64_t *table_from_entry(uint64_t e) {
    return (uint64_t *)(uintptr_t)(e & PT_ADDR_MASK);
}

static void zero_table(uint64_t *t) {
    for (int i = 0; i < 512; i++) t[i] = 0;
}

/* Returns a pointer to the next-level table, allocating a fresh 4 KiB frame if
 * the parent entry is not present. Non-leaf entries are always marked present
 * and writable; individual mappings later provide the real permissions. */
static uint64_t *walk_or_alloc(uint64_t *table, size_t idx) {
    uint64_t e = table[idx];
    if (e & VMM_PAGE_PRESENT) {
        return table_from_entry(e);
    }
    void *frame = pmm_alloc_block();
    if (!frame) return NULL;
    uint64_t *nt = (uint64_t *)frame;
    zero_table(nt);
    table[idx] = ((uintptr_t)frame & PT_ADDR_MASK) | VMM_PAGE_PRESENT | VMM_PAGE_WRITE;
    return nt;
}

void vmm_init(void) {
    /* Kernel tables were built by boot.asm/paging.asm. Nothing to do here beyond
     * acknowledging the current PML4 is now under C-side management. */
}

int vmm_map_page(uintptr_t virt, uintptr_t phys, uint64_t flags) {
    uint64_t *pdpt = walk_or_alloc(pml4, idx_pml4(virt));
    if (!pdpt) return -1;

    uint64_t *pd = walk_or_alloc(pdpt, idx_pdpt(virt));
    if (!pd) return -1;

    /* Refuse to punch a 4 KiB mapping through an existing 2 MiB page. */
    if (pd[idx_pd(virt)] & VMM_PAGE_SIZE_2M) return -1;

    uint64_t *pt = walk_or_alloc(pd, idx_pd(virt));
    if (!pt) return -1;

    pt[idx_pt(virt)] = (phys & PT_ADDR_MASK) | (flags & PTE_FLAGS_MASK) | VMM_PAGE_PRESENT;

    __asm__ volatile ("invlpg (%0)" : : "r"(virt) : "memory");
    return 0;
}

void vmm_unmap_page(uintptr_t virt) {
    if (!(pml4[idx_pml4(virt)] & VMM_PAGE_PRESENT)) return;
    uint64_t *pdpt = table_from_entry(pml4[idx_pml4(virt)]);

    if (!(pdpt[idx_pdpt(virt)] & VMM_PAGE_PRESENT)) return;
    uint64_t *pd = table_from_entry(pdpt[idx_pdpt(virt)]);

    if (!(pd[idx_pd(virt)] & VMM_PAGE_PRESENT)) return;
    if (pd[idx_pd(virt)] & VMM_PAGE_SIZE_2M) return;
    uint64_t *pt = table_from_entry(pd[idx_pd(virt)]);

    pt[idx_pt(virt)] = 0;
    __asm__ volatile ("invlpg (%0)" : : "r"(virt) : "memory");
}
