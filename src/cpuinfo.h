// This file is part of CPU Energy Meter,
// a tool for measuring energy consumption of Intel CPUs:
// https://github.com/sosy-lab/cpu-energy-meter
//
// SPDX-FileCopyrightText: 2012 Intel Corporation
// SPDX-FileCopyrightText: 2015-2018 Dirk Beyer
//
// SPDX-License-Identifier: BSD-3-Clause

#ifndef _h_cpuinfo
#define _h_cpuinfo

#include <stdint.h>

typedef struct {
  int smt_id;
  int core_id;
  int pkg_id;
} APIC_ID_t;

/**
 * Check if system has an Intel processor.
 */
int is_intel_processor();

/**
 * Get processor signature (vendor-specific).
 */
uint32_t get_processor_signature();

/**
 * Read information about physical topology for an OS core.
 *
 * Returns 0 on success and -1 on failure.
 */
int get_core_information(int os_cpu, APIC_ID_t *result);

/**
 * Read string with vendor name from processor.
 * Needs to be passed an array of length VENDOR_LENGTH.
 */
void get_vendor_name(char *vendor);
// Vendor name has 12 characters, plus trailing \0
#define VENDOR_LENGTH 13

#endif
