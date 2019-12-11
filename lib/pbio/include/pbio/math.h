// SPDX-License-Identifier: MIT
// Copyright (c) 2018-2019 David Lechner
// Copyright (c) 2018-2019 Laurens Valk
// Copyright (c) 2019 LEGO System A/S

#ifndef _PBIO_MATH_H_
#define _PBIO_MATH_H_

#include <stdint.h>

#include <fixmath.h>

int32_t pbio_math_div_i32_fix16(int32_t a, fix16_t b);
int32_t pbio_math_mul_i32_fix16(int32_t a, fix16_t b);
int32_t pbio_math_sqrt(int32_t n);

#endif // _PBIO_MATH_H_