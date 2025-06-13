/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) 2015, Linaro Limited
 */
#ifndef KERNEL_ENTRYSTD_H
#define KERNEL_ENTRYSTD_H

// #include <assert.h>
// #include <compiler.h>
#include <kernel/tee_ta_manager.h>
#include <tee_api_types.h>
// #include <user_ta_header.h>

// typedef unsigned int __uint32_t;
// typedef __uint32_t uint32_t;
// typedef __int32_t int32_t;

// struct tee_ioctl_buf_data {
// 	uint32_t buf_ptr;
// 	uint32_t buf_len;
// };

int32_t ioctl(uint32_t cmd, struct tee_ioctl_buf_data *buf_data);

#endif /* KERNEL_ENTRYSTD_H */