/**
 * SPDX-License-Identifier: Apache-2.0U
 * Copyright (c) Bao Project and Contributors. All rights reserved.
 */

#ifndef ARCH_NVIC_H
#define ARCH_NVIC_H

#include <sysregs.h>

static inline void nvic_set_enable(unsigned long int_id, bool en)
{
    nvic->iser[int_id >> 5UL] |= (uint32_t)(1UL << (int_id & 0x1FUL));
}

static inline void nvic_set_prio(unsigned long int_id, uint8_t prio)
{
    nvic->ipr[int_id] = (uint8_t)((prio << (8U - NVIC_PRIO_BITS)) & (uint32_t)0xFFUL);
}

static inline bool nvic_is_pending(unsigned long int_id)
{
    return ((nvic->ispr[int_id >> 5UL] & (uint32_t)(1UL << (int_id & 0x1FUL))) != 0UL) ? true : false;
}

static inline void nvic_set_pending(unsigned long int_id, bool pending)
{
    if (pending) {
        nvic->ispr[int_id >> 5UL] |= (uint32_t)(1UL << (int_id & 0x1FUL));
    } else {
        nvic->icpr[int_id >> 5UL] |= (uint32_t)(1UL << (int_id & 0x1FUL));
    }
}

static inline bool nvic_is_active(unsigned long int_id)
{
    return ((nvic->iabr[int_id >> 5UL] & (uint32_t)(1UL << (int_id & 0x1FUL))) != 0UL) ? true : false;
}

void nvic_handle(void);

#endif /* ARCH_NVIC_H */