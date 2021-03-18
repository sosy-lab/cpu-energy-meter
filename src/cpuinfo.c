// This file is part of CPU Energy Meter,
// a tool for measuring energy consumption of Intel CPUs:
// https://github.com/sosy-lab/cpu-energy-meter
//
// SPDX-FileCopyrightText: 2012 Intel Corporation
// SPDX-FileCopyrightText: 2015-2021 Dirk Beyer <https://www.sosy-lab.org>
//
// SPDX-License-Identifier: BSD-3-Clause

#include "cpuinfo.h"
#include "util.h"

#include <assert.h>
#include <cpuid.h>
#include <limits.h>
#include <stdio.h>

typedef struct cpuid_info_t {
  uint32_t eax;
  uint32_t ebx;
  uint32_t ecx;
  uint32_t edx;
} cpuid_info_t;

static void cpuid(uint32_t eax_in, uint32_t ecx_in, cpuid_info_t *ci) {
  assert(ci != NULL);
#ifndef TEST
  __cpuid_count(eax_in, ecx_in, ci->eax, ci->ebx, ci->ecx, ci->edx);
#else // when unit-testing, compile with -DTest for using preset values instead of executing the
      // above
  ci->eax = 0x806e9;
  ci->ebx = 0x756e6547;
  ci->ecx = 0x6c65746e;
  ci->edx = 0x49656e69;
#endif
}

int is_intel_processor() {
  cpuid_info_t sig;
  cpuid(0, 0, &sig); // get vendor signature

  const uint32_t exp_ebx = 0x756e6547; // translates to "Genu"
  const uint32_t exp_ecx = 0x6c65746e; // translates to "ineI"
  const uint32_t exp_edx = 0x49656e69; // translates to "ntel"
  return sig.ebx == exp_ebx && sig.ecx == exp_ecx && sig.edx == exp_edx;
}

uint32_t get_processor_signature() {
  cpuid_info_t info;
  cpuid(0x1, 0, &info);
  return info.eax;
}

int get_core_information(int os_cpu, APIC_ID_t *result) {
  assert(result != NULL);
  cpu_set_t prev_context;
  if (bind_cpu(os_cpu, &prev_context) == -1) {
    return -1;
  }

  cpuid_info_t info_l0;
  cpuid_info_t info_l1;
  cpuid(0xb, 0, &info_l0);
  cpuid(0xb, 1, &info_l1);

  if (bind_context(&prev_context, NULL) == -1) {
    return -1;
  }

  // Parse the x2APIC_ID_t into SMT, core and package ID.
  // http://software.intel.com/en-us/articles/intel-64-architecture-processor-topology-enumeration

  // Get the SMT ID
  const uint32_t smt_mask_width = info_l0.eax & 0x1f;  // max value 31
  const uint32_t smt_mask = (1 << smt_mask_width) - 1; // max value 0x7fffffff
  const uint32_t smt_id = info_l0.edx & smt_mask;

  // Get the core ID
  const uint32_t core_mask_width = info_l1.eax & 0x1f;                // max value 31
  const uint32_t core_mask = ((1 << core_mask_width) - 1) ^ smt_mask; // max value 0x7fffffff
  const uint32_t core_id = (info_l1.edx & core_mask) >> smt_mask_width;

  // Get the package ID
  const uint32_t pkg_mask = (~(1 << core_mask_width)) + 1; // max value 0x80000000
  const uint32_t pkg_id = (info_l1.edx & pkg_mask) >> core_mask_width;

  // all values have at most 31 bits and fit into an int
  assert(result->smt_id < INT_MAX);
  assert(result->core_id < INT_MAX);
  assert(result->pkg_id < INT_MAX);

  result->smt_id = smt_id;
  result->core_id = core_id;
  result->pkg_id = pkg_id;

  return 0;
}

static void cast_uint_to_str(char *out, uint32_t in) {
  uint32_t mask = 0x000000ff;
  for (int i = 0; i < 4; i++) {
    out[i] = (char)((in & (mask << (i * 8))) >> (i * 8));
  }
}

void get_vendor_name(char *vendor) {
  assert(vendor != NULL);
  cpuid_info_t c;
  cpuid(0, 0, &c);
  cast_uint_to_str(&(vendor[0]), c.ebx);
  cast_uint_to_str(&(vendor[4]), c.edx);
  cast_uint_to_str(&(vendor[8]), c.ecx);
  vendor[VENDOR_LENGTH - 1] = 0;
}
