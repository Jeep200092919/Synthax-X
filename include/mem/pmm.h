#ifndef SYNTHAX_MEM_PMM_H
#define SYNTHAX_MEM_PMM_H

#include <stdint.h>

#define PMM_PAGE_SIZE   4096ULL
#define PMM_MAX_BYTES   (32ULL * 1024 * 1024 * 1024)      /* 32 GiB */
#define PMM_MAX_PAGES   (PMM_MAX_BYTES / PMM_PAGE_SIZE)   /* 8,388,608 */
#define PMM_BITMAP_BYTES (PMM_MAX_PAGES / 8)              /* 1 MiB */

#define PMM_NO_BLOCK    ((void *)(uintptr_t)0)

void      pmm_init(uintptr_t mb2_info_addr);
void     *pmm_alloc_block(void);
void      pmm_free_block(void *addr);

uint64_t  pmm_total_bytes(void);
uint64_t  pmm_used_bytes(void);
uint64_t  pmm_free_bytes(void);
uintptr_t pmm_bitmap_addr(void);

#endif
