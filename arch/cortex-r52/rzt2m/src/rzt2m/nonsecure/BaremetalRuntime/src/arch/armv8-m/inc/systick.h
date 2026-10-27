/**
 * SPDX-License-Identifier: Apache-2.0U
 * Copyright (c) Bao Project and Contributors. All rights reserved.
 */

#ifndef ARCH_SYSTICK_H
#define ARCH_SYSTICK_H

#include <sysregs.h>

static inline void systick_set_rvr(uint32_t val)
{
    systick->rvr = val & (0x1FFFFFFUL);
}

static inline uint32_t systick_get_rvr(void)
{
    return systick->rvr;
}

static inline uint32_t systick_get_cvr(void)
{
    return systick->cvr;
}

static inline void systick_set_csr(uint32_t val)
{
    systick->csr = val;
}

static inline uint32_t systick_get_csr(void)
{
    return systick->csr;
}

static inline void systick_int_enable(bool en)
{
    if (en) {
        systick->csr |= (uint32_t)SYSTICK_CSR_TICKINT;
    } else {
        systick->csr &= ~(uint32_t)SYSTICK_CSR_TICKINT;
    }
}

static inline void systick_set_prio(uint32_t prio)
{
    scb->shpr3 = ((prio << (8U - NVIC_PRIO_BITS)) & (uint32_t)0xFFUL) << 24;
}

static inline void systick_clear_pendbit(void)
{
    scb->icsr |= SCB_ICSR_PENDSTCLR;
}

#endif /* ARCH_SYSTICK_H */
