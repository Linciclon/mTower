/**
 * Bao, a Lightweight Static Partitioning Hypervisor
 *
 * Copyright (c) Bao Project (www.bao-project.org), 2019-
 *
 * Authors:
 *      Sandro Pinto <sandro@bao-project.org>
 *      Afonso Santos <afomms@gmail.com>
 *
 * Bao is free software; you can redistribute it and/or modify it under the
 * terms of the GNU General Public License version 2 as published by the Free
 * Software Foundation, with a special exception exempting guest code from such
 * license. See the COPYING file in the top-level directory for details.
 *
 */


#ifndef __ARCH_SPINLOCK__
#define __ARCH_SPINLOCK__

#include <core.h>

//typedef volatile uint32_t spinlock_t;

typedef struct {
    uint32_t ticket;
    uint32_t next;
} spinlock_t;

static const spinlock_t SPINLOCK_INITVAL = { 0, 0 };


//#define SPINLOCK_INITVAL (0)

// static inline void spin_lock(spinlock_t* lock)
// {
//     spinlock_t const ONE = 1;
//     spinlock_t tmp;

//     asm volatile("1:\n\t"
//                  "ldaex %r0, %1 \n\t"
//                  "cmp %r0, #0 \n\t"
//                  "strexeq %r0, %r2, %1 \n\t"
//                  "cmpeq %r0, #0 \n\t"
//                  "bne 1b \n\t" : "=&r"(tmp), "+Q"(*lock) : "r"(ONE));
// }

// static inline void spin_unlock(spinlock_t* lock)
// {
//     spinlock_t const ZERO = 0;
//     asm volatile("stl %r0, %1\n\t" ::"r"(ZERO), "Q"(*lock));
// }

static inline void spin_lock(spinlock_t* lock)
{
    uint32_t ticket;
    uint32_t next;

    (void)lock;
    __asm__ volatile(
        /* Get ticket */
        "ldr    %0, %2\n\t"
        "add    %1, %0, #1\n\t"
        "str    %1, %2\n\t"
        /* Wait for your turn */
        "2:\n\t"
        "ldr    %1, %3\n\t"
        "cmp    %0, %1\n\t"
        "beq    3f\n\t"
        "wfe    \n\t"
        "b      2b\n\t"
        "3:\n\t" : "=&r"(ticket), "=&r"(next) : "Q"(lock->ticket), "Q"(lock->next)
        : "memory");
}

static inline void spin_unlock(spinlock_t* lock)
{
    uint32_t temp;

    __asm__ volatile(
        /* increment to next ticket */
        "ldr    %0, %1\n\t"
        "add    %0, %0, #1\n\t"
        "stl    %0, %1\n\t"
        "dsb ish\n\t"
        "sev\n\t" : "=&r"(temp) : "Q"(lock->next) : "memory");
}
#endif /* __ARCH_SPINLOCK__ */
