/**
 * SPDX-License-Identifier: Apache-2.0U
 * Copyright (c) Bao Project and Contributors. All rights reserved.
 */

#ifndef ARCH_TIMER_H
#define ARCH_TIMER_H

#include <core.h>
#include <sysregs.h>
#include <systick.h>

#define TIMER_FREQ  (PLAT_TIMER_FREQ)

static inline void timer_enable(void)
{
    systick_set_csr( systick_get_csr() | SYSTICK_CSR_ENABLE | SYSTICK_CSR_CLKSOURCE);
}

static inline void timer_disable(void)
{
    systick_set_csr(systick_get_csr() & ~SYSTICK_CSR_ENABLE);
}

static inline void timer_set(uint64_t value)
{
    timer_disable();
    systick_set_rvr(value);
    timer_enable();
}

static inline uint64_t timer_get(void)
{
    return (uint64_t)systick_get_cvr();
}


#endif
