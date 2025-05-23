/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) 2014, STMicroelectronics International N.V.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/* Based on GP TEE Internal API Specification Version 0.11 */
#ifndef TEE_API_TYPES_H
#define TEE_API_TYPES_H

#include <compiler.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <tee_api_defines.h>

/*
 * Common Definitions
 */

typedef uint32_t TEE_Result;

typedef struct {
	uint32_t timeLow;
	uint16_t timeMid;
	uint16_t timeHiAndVersion;
	uint8_t clockSeqAndNode[8];
} TEE_UUID;

/*
 * The TEE_Identity structure defines the full identity of a Client:
 * - login is one of the TEE_LOGIN_XXX constants
 * - uuid contains the client UUID or Nil if not applicable
 */
typedef struct {
	uint32_t login;
	TEE_UUID uuid;
} TEE_Identity;

/*
 * This union describes one parameter passed by the Trusted Core Framework
 * to the entry points TA_OpenSessionEntryPoint or
 * TA_InvokeCommandEntryPoint or by the TA to the functions
 * TEE_OpenTASession or TEE_InvokeTACommand.
 *
 * Which of the field value or memref to select is determined by the
 * parameter type specified in the argument paramTypes passed to the entry
 * point.
*/
typedef union {
	struct {
		void *buffer;
		uint32_t size;
		uint32_t dummy; // ????
	} memref;
	struct {
		uint32_t a;
    uint32_t b;
    uint32_t c;   //???
	} value;
} TEE_Param;

/*
 * The type of opaque handles on TA Session. These handles are returned by
 * the function TEE_OpenTASession.
 */
typedef struct __TEE_TASessionHandle *TEE_TASessionHandle;

/*
 * The type of opaque handles on property sets or enumerators. These
 * handles are either one of the pseudo handles TEE_PROPSET_XXX or are
 * returned by the function TEE_AllocatePropertyEnumerator.
*/
typedef struct __TEE_PropSetHandle *TEE_PropSetHandle;

typedef struct __TEE_ObjectHandle *TEE_ObjectHandle;
typedef struct __TEE_ObjectEnumHandle *TEE_ObjectEnumHandle;
typedef struct __TEE_OperationHandle *TEE_OperationHandle;

/*
 * Storage Definitions
 */

typedef uint32_t TEE_ObjectType;

typedef struct {
	uint32_t objectType;
	__extension__ union {
		uint32_t keySize;	/* used in 1.1 spec */
		uint32_t objectSize;	/* used in 1.1.1 spec */
	};
	__extension__ union {
		uint32_t maxKeySize;	/* used in 1.1 spec */
		uint32_t maxObjectSize;	/* used in 1.1.1 spec */
	};
	uint32_t objectUsage;
	uint32_t dataSize;
	uint32_t dataPosition;
	uint32_t handleFlags;
} TEE_ObjectInfo;

typedef enum {
	TEE_DATA_SEEK_SET = 0,
	TEE_DATA_SEEK_CUR = 1,
	TEE_DATA_SEEK_END = 2
} TEE_Whence;

typedef struct {
	uint32_t attributeID;
	union {
		struct {
			void *buffer;
			uint32_t length;
		} ref;
		struct {
			uint32_t a, b;
		} value;
	} content;
} TEE_Attribute;

/* Cryptographic Operations API */

typedef enum {
	TEE_MODE_ENCRYPT = 0,
	TEE_MODE_DECRYPT = 1,
	TEE_MODE_SIGN = 2,
	TEE_MODE_VERIFY = 3,
	TEE_MODE_MAC = 4,
	TEE_MODE_DIGEST = 5,
	TEE_MODE_DERIVE = 6
} TEE_OperationMode;

typedef struct {
	uint32_t algorithm;
	uint32_t operationClass;
	uint32_t mode;
	uint32_t digestLength;
	uint32_t maxKeySize;
	uint32_t keySize;
	uint32_t requiredKeyUsage;
	uint32_t handleState;
} TEE_OperationInfo;

typedef struct {
	uint32_t keySize;
	uint32_t requiredKeyUsage;
} TEE_OperationInfoKey;

typedef struct {
	uint32_t algorithm;
	uint32_t operationClass;
	uint32_t mode;
	uint32_t digestLength;
	uint32_t maxKeySize;
	uint32_t handleState;
	uint32_t operationState;
	uint32_t numberOfKeys;
	TEE_OperationInfoKey keyInformation[];
} TEE_OperationInfoMultiple;

/* Time & Date API */

typedef struct {
	uint32_t seconds;
	uint32_t millis;
} TEE_Time;

/* TEE Arithmetical APIs */

typedef uint32_t TEE_BigInt;

typedef uint32_t TEE_BigIntFMM;

typedef uint32_t TEE_BigIntFMMContext __aligned(__alignof__(void *));

/* Tee Secure Element APIs */

typedef struct __TEE_SEServiceHandle *TEE_SEServiceHandle;
typedef struct __TEE_SEReaderHandle *TEE_SEReaderHandle;
typedef struct __TEE_SESessionHandle *TEE_SESessionHandle;
typedef struct __TEE_SEChannelHandle *TEE_SEChannelHandle;

typedef struct {
	bool sePresent;
	bool teeOnly;
	bool selectResponseEnable;
} TEE_SEReaderProperties;

typedef struct {
	uint8_t *buffer;
	size_t bufferLen;
} TEE_SEAID;

/* Other definitions */
typedef uint32_t TEE_ErrorOrigin;
typedef void *TEE_Session;

#define TEE_MEM_INPUT   0x00000001
#define TEE_MEM_OUTPUT  0x00000002

#define TEE_MEMREF_0_USED  0x00000001
#define TEE_MEMREF_1_USED  0x00000002
#define TEE_MEMREF_2_USED  0x00000004
#define TEE_MEMREF_3_USED  0x00000008

#define TEE_SE_READER_NAME_MAX	20

#endif /* TEE_API_TYPES_H */
