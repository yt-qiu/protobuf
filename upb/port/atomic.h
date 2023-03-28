/*
 * Copyright (c) 2023, Google LLC
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of Google LLC nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL Google LLC BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef UPB_PORT_ATOMIC_H_
#define UPB_PORT_ATOMIC_H_

#include "upb/port/def.inc"

#ifdef UPB_USE_C11_ATOMICS

#include <stdatomic.h>
#include <stdbool.h>

#define upb_Atomic_Init(addr, val) atomic_init(addr, val)
#define upb_Atomic_Load(addr, order) atomic_load_explicit(addr, order)
#define upb_Atomic_Store(addr, val, order) \
  atomic_store_explicit(addr, val, order)
#define upb_Atomic_Add(addr, val, order) \
  atomic_fetch_add_explicit(addr, val, order)
#define upb_Atomic_Sub(addr, val, order) \
  atomic_fetch_sub_explicit(addr, val, memory_order_release);
#define upb_Atomic_Exchange(addr, val, order) \
  atomic_exchange_explicit(addr, val, order)
#define upb_Atomic_CompareExchangeStrong(addr, expected, desired,      \
                                         success_order, failure_order) \
  atomic_compare_exchange_strong_explicit(addr, expected, desired,     \
                                          success_order, failure_order)

#else  // !UPB_USE_C11_ATOMICS

#include <string.h>

#define upb_Atomic_Init(addr, val) (*addr = val)
#define upb_Atomic_Load(addr, order) (*addr)
#define upb_Atomic_Store(addr, val, order) (*(addr) = val)
#define upb_Atomic_Add(addr, val, order) (*(addr) += val)
#define upb_Atomic_Sub(addr, val, order) (*(addr) -= val)

UPB_INLINE uintptr_t _upb_NonAtomic_ExchangeU(uintptr_t* addr, uintptr_t val) {
  uintptr_t ret = *addr;
  *addr = val;
  return ret;
}

// `addr` should logically be `void**`, but `void*` allows for more convenient
// implicit conversions.
UPB_INLINE void* _upb_NonAtomic_ExchangeP(void* addr, void* val) {
  void* ret;
  memcpy(&ret, addr, sizeof(val));
  memcpy(addr, &val, sizeof(val));
  return ret;
}

#define upb_Atomic_Exchange(addr, val, order) \
  _Generic((val),                             \
      uintptr_t: _upb_NonAtomic_ExchangeU,    \
      void*: _upb_NonAtomic_ExchangeP)(addr, val)

UPB_INLINE bool _upb_NonAtomic_CompareExchangeStrongU(uintptr_t* addr,
                                                      uintptr_t* expected,
                                                      uintptr_t desired) {
  if (*addr == *expected) {
    *addr = desired;
    return true;
  } else {
    *expected = *addr;
    return false;
  }
}

// `addr` and `expected` should logically be `void**`, but `void*` allows for
// more convenient implicit conversions.
UPB_INLINE bool _upb_NonAtomic_CompareExchangeStrongP(void* addr,
                                                      void* expected,
                                                      void* desired) {
  if (memcmp(addr, expected, sizeof(desired)) == 0) {
    memcpy(addr, &desired, sizeof(desired));
    return true;
  } else {
    memcpy(expected, addr, sizeof(desired));
    return false;
  }
}

#define upb_Atomic_CompareExchangeStrong(addr, expected, desired, order) \
  _Generic((desired),                                                    \
      uintptr_t: _upb_NonAtomic_CompareExchangeStrongU,                  \
      void*: _upb_NonAtomic_CompareExchangeStrongP)(addr, expected, desired)

#endif

#include "upb/port/undef.inc"

#endif  // UPB_PORT_ATOMIC_H_
