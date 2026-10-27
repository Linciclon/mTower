/**
 * SPDX-License-Identifier: Apache-2.0U
 * Copyright (c) Bao Project and Contributors. All rights reserved.
 */

#ifndef ARCH_CPU_H
#define ARCH_CPU_H

#include <core.h>
#include <sysregs.h>

static inline unsigned long get_cpuid(){
    return scb->cpuid;
}

static bool cpu_is_master() {
    return true;
}

#endif
