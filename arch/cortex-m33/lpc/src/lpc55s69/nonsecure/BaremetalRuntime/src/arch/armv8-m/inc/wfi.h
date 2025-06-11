/**
 * SPDX-License-Identifier: Apache-2.0U
 * Copyright (c) Bao Project and Contributors. All rights reserved.
 */

#ifndef ARCH_WFI_H
#define ARCH_WFI_H

static inline void wfi()
{
    asm volatile("wfi\n\t" ::: "memory");
}

#endif /* ARCH_WFI_H */
