/**
 * @file        include/mtower/err.h
 * @brief       Display a formatted error message
 *
 * @copyright   Copyright (c) 2020 Samsung Electronics Co., Ltd. All Rights Reserved.
 * @author      Taras Drozdovskyi t.drozdovsky@samsung.com
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __INCLUDE_MTOWER_ERR_H_
#define __INCLUDE_MTOWER_ERR_H_

/* Included Files */
#ifndef PLATFORM_LPC55S69
#include "printf.h"
#endif
/* Pre-processor Definitions */
/* All C pre-processor macros are defined here. */

#define  errx(i ,format, ...) printf(format "\n", ##__VA_ARGS__)

/* Public Types */
/* Any types, enumerations, structures or unions are defined here. */


/* Public Data */
/* All data declarations with global scope appear here, preceded by
 * the definition EXTERN.
 */


/* Inline Functions */
/* Any static inline functions may be defined in this grouping.
 * Each is preceded by a function header similar to the below.
 */

/* Public Function Prototypes */
/* All global functions in the file are prototyped here. The keyword
 * extern or the definition EXTERN are never used with function
 * prototypes.
 */


#endif /* __INCLUDE_MTOWER_ERR_H_ */

