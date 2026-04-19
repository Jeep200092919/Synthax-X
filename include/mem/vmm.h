#ifndef SYNTHAX_MEM_VMM_H
#define SYNTHAX_MEM_VMM_H

#include <stdint.h>

#define VMM_PAGE_PRESENT   (1ULL << 0)
#define VMM_PAGE_WRITE     (1ULL << 1)
#define VMM_PAGE_USER      (1ULL << 2)
#define VMM_PAGE_SIZE_2M   (1ULL << 7)

#define VMM_FLAGS_KRW      (VMM_PAGE_PRESENT | VMM_PAGE_WRITE)

void vmm_init(void);
int  vmm_map_page(uintptr_t virt, uintptr_t phys, uint64_t flags);
void vmm_unmap_page(uintptr_t virt);

#endif
