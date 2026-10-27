/**
 * SPDX-License-Identifier: Apache-2.0
 * Copyright (c) Bao Project and Contributors. All rights reserved.
 */

#ifndef FENCES_ARCH_H
#define FENCES_ARCH_H

#define ISB() __asm__ volatile("isb 0xF" ::: "memory");
#define DSB() __asm__ volatile("dsb 0xF" ::: "memory");
#define DMB() __asm__ volatile("dmb 0xF" ::: "memory");

static inline void fence_ord_write(void)
{
    DMB();
}

static inline void fence_ord_read(void)
{
    DMB();
}

static inline void fence_ord(void)
{
    DMB();
}

static inline void fence_sync_write(void)
{
    DSB();
}

static inline void fence_sync_read(void)
{
    DSB();
}

static inline void fence_sync(void)
{
    DSB();
}

#endif /* FENCES_ARCH_H */
