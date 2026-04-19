#include <stdint.h>
#include <stddef.h>
#include "mem/heap.h"

/* Singly-linked first-fit allocator. Each block carries a header; payloads are
 * 16-byte aligned. kfree coalesces with the following block when possible. */

#define HEAP_ALIGN      16
#define HEAP_MIN_SPLIT  16

struct block {
    size_t        size;    /* payload bytes */
    uint32_t      free;
    uint32_t      magic;
    struct block *next;
};

#define HEAP_MAGIC 0x534E5458u   /* 'SNTX' */

static struct block *head;

static inline size_t align_up(size_t s, size_t a) {
    return (s + (a - 1)) & ~(a - 1);
}

void heap_init(uintptr_t virt_base, size_t size) {
    head = (struct block *)virt_base;
    head->size  = size - sizeof(struct block);
    head->free  = 1;
    head->magic = HEAP_MAGIC;
    head->next  = NULL;
}

void *kmalloc(size_t size) {
    if (size == 0 || !head) return NULL;
    size = align_up(size, HEAP_ALIGN);

    for (struct block *b = head; b; b = b->next) {
        if (!b->free || b->size < size) continue;

        /* Split if the tail has room for another header + usable payload. */
        if (b->size >= size + sizeof(struct block) + HEAP_MIN_SPLIT) {
            struct block *tail = (struct block *)((uint8_t *)b + sizeof(struct block) + size);
            tail->size  = b->size - size - sizeof(struct block);
            tail->free  = 1;
            tail->magic = HEAP_MAGIC;
            tail->next  = b->next;
            b->next     = tail;
            b->size     = size;
        }
        b->free = 0;
        return (void *)((uint8_t *)b + sizeof(struct block));
    }
    return NULL;
}

void kfree(void *ptr) {
    if (!ptr) return;
    struct block *b = (struct block *)((uint8_t *)ptr - sizeof(struct block));
    if (b->magic != HEAP_MAGIC) return;
    b->free = 1;

    /* Coalesce with the next block if it is also free. */
    if (b->next && b->next->free && b->next->magic == HEAP_MAGIC) {
        b->size += sizeof(struct block) + b->next->size;
        b->next  = b->next->next;
    }
}
