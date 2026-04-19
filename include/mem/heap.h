#ifndef SYNTHAX_MEM_HEAP_H
#define SYNTHAX_MEM_HEAP_H

#include <stdint.h>
#include <stddef.h>

void  heap_init(uintptr_t virt_base, size_t size);
void *kmalloc(size_t size);
void  kfree(void *ptr);

#endif
