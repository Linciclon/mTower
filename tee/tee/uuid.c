// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) 2016, Linaro Limited
 */

#include <stdio.h>
#include <string.h>
#include <tee/uuid.h>
#include <util.h>

#ifndef PLATFORM_LPC55S69
#include "printf.h"
#endif

void tee_uuid_to_octets(uint8_t *d, const TEE_UUID *s)
{
	d[0] = s->timeLow >> 24;
	d[1] = s->timeLow >> 16;
	d[2] = s->timeLow >> 8;
	d[3] = s->timeLow;
	d[4] = s->timeMid >> 8;
	d[5] = s->timeMid;
	d[6] = s->timeHiAndVersion >> 8;
	d[7] = s->timeHiAndVersion;
	memcpy(d + 8, s->clockSeqAndNode, sizeof(s->clockSeqAndNode));
}

void tee_uuid_from_octets(TEE_UUID *d, const uint8_t *s)
{
	d->timeLow = SHIFT_U32(s[0], 24) | SHIFT_U32(s[1], 16) |
		     SHIFT_U32(s[2], 8) | s[3];
	d->timeMid = SHIFT_U32(s[4], 8) | s[5];
	d->timeHiAndVersion = SHIFT_U32(s[6], 8) | s[7];
	memcpy(d->clockSeqAndNode, s + 8, sizeof(d->clockSeqAndNode));
}

void uuid_print(uint8_t *d)
{
//  printf("UUID = %08x\n",
//      ((uint32_t) (d[0] << 24)) | (uint32_t) (d[1] << 16)
//          | (uint32_t) (d[2] << 8) | (uint32_t) d[3]);

  TEE_UUID s;
  int i;
  s.timeLow = ((uint32_t)d[0] << 24);
  s.timeLow |= ((uint32_t)d[1] << 16);
  s.timeLow |= ((uint32_t)d[2] << 8);
  s.timeLow |= ((uint32_t)d[3]);
  s.timeMid = ((uint16_t)d[4] << 8);
  s.timeMid |= ((uint16_t)d[5]);
  s.timeHiAndVersion = ((uint16_t)d[6] << 8);
  s.timeHiAndVersion |= ((uint16_t)d[7]);
  memcpy(s.clockSeqAndNode, d + 8, sizeof(s.clockSeqAndNode));

  printf("UUID = %08x-%04x-%04x", s.timeLow, s.timeMid, s.timeHiAndVersion);
  for (i = 0; i < 8; ++i) {
    printf("-%02x",s.clockSeqAndNode[i]);
  }
  printf("\n");
}
