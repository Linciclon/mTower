/* SPDX-License-Identifier: BSD-2-Clause */
#ifndef MTOWER_TEE_BENCHMARK_H
#define MTOWER_TEE_BENCHMARK_H

#include <stdint.h>

#define TEE_BENCHMARK_MAGIC       0x5442454eUL /* "TBEN" */
#define TEE_BENCHMARK_MAX_INVOKES 16U

/* Keep the benchmark record away from the ioctl payload at the start of the
 * 64 KiB TEE shared-memory window. */
#define TEE_BENCHMARK_ADDRESS     0x2000ff00UL

/* CTIMER0 is one physical free-running counter.  A native secure mTower uses
 * the Secure peripheral alias; a virtualized mTower receives the Non-secure
 * alias mapped by pervmtee-m_marcobenchmark. */
#ifdef RUN_INSIDE_VM
#define TEE_BENCHMARK_TIMER_COUNT 0x40008008UL
#else
#define TEE_BENCHMARK_TIMER_COUNT 0x50008008UL
#endif

struct tee_benchmark_record {
	uint32_t magic;
	uint32_t hotp_start;
	uint32_t hotp_end;
	uint32_t open_start;
	uint32_t open_gateway;
	uint32_t open_handler;
	uint32_t invoke_count;
	uint32_t invoke_start[TEE_BENCHMARK_MAX_INVOKES];
	uint32_t invoke_gateway[TEE_BENCHMARK_MAX_INVOKES];
	uint32_t invoke_handler[TEE_BENCHMARK_MAX_INVOKES];
	uint32_t close_start;
	uint32_t close_gateway;
	uint32_t close_handler;
};

#define TEE_BENCHMARK_RECORD \
	((volatile struct tee_benchmark_record *)TEE_BENCHMARK_ADDRESS)

static inline uint32_t tee_benchmark_timer_count(void)
{
	uint32_t count;

	__asm volatile("" ::: "memory");
	count = *(volatile uint32_t *)TEE_BENCHMARK_TIMER_COUNT;
	__asm volatile("" ::: "memory");
	return count;
}

#endif /* MTOWER_TEE_BENCHMARK_H */
