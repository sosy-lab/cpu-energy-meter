/*
Copyright (c) 2012, Intel Corporation

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of Intel Corporation nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/* Written by Martin Dimitrov, Carl Strickland */

#ifndef _h_msr_t
#define _h_msr_t

#include <stdint.h>

#define MY_ERROR -1

/* Replacement for pow() where the base is 2 and the power is unsigned and
 * less than 31 (will get invalid numbers if 31 or greater) */
#define B2POW(e) (((e) == 0) ? 1 : (2 << ((e) - 1)))

/* General */
#define MSR_RAPL_POWER_UNIT 0x606 /* Unit Multiplier used in RAPL Interfaces (R/O) */

/* Package RAPL Domain */
#define MSR_RAPL_PKG_POWER_LIMIT   0x610 /* PKG RAPL Power Limit Control (R/W) */
#define MSR_RAPL_PKG_ENERGY_STATUS 0x611 /* PKG Energy Status (R/O) */
//#define MSR_RAPL_PKG_PERF_STATUS   0x613 /* PKG Performance Throttling Status (R/O) */
//#define MSR_RAPL_PKG_POWER_INFO    0x614 /* PKG RAPL Parameters (R/O) */

/* DRAM RAPL Domain */
#define MSR_RAPL_DRAM_POWER_LIMIT   0x618 /* DRAM RAPL Power Limit Control (R/W) */
#define MSR_RAPL_DRAM_ENERGY_STATUS 0x619 /* DRAM Energy Status (R/O) */
//#define MSR_RAPL_DRAM_PERF_STATUS   0x61b /* DRAM Performance Throttling Status (R/O) */
//#define MSR_RAPL_DRAM_POWER_INFO    0x61c /* DRAM RAPL Parameters (R/O) */

/* PP0 RAPL Domain */
#define MSR_RAPL_PP0_POWER_LIMIT   0x638 /* PP0 RAPL Power Limit Control (R/W) */
#define MSR_RAPL_PP0_ENERGY_STATUS 0x639 /* PP0 Energy Status (R/O) */
//#define MSR_RAPL_PP0_POLICY        0x63a /* PP0 Performance Throttling Status (R/O) */
//#define MSR_RAPL_PP0_PERF_STATUS   0x63b /* PP0 Balance Policy (R/W) */

/* PP1 RAPL Domain */
#define MSR_RAPL_PP1_POWER_LIMIT   0x640 /* PP1 RAPL Power Limit Control (R/W) */
#define MSR_RAPL_PP1_ENERGY_STATUS 0x641 /* PP1 Energy Status (R/O) */
//#define MSR_RAPL_PP1_POLICY        0x642 /* PP1 Balance Policy (R/W) */

/* PSYS RAPL Domain */
#define MSR_RAPL_PLATFORM_ENERGY_STATUS 0x64d /* PSYS Energy Status */

/* Common MSR Structures */
typedef struct rapl_power_limit_control_msr_t {
    uint64_t power_limit         : 15;
    uint64_t limit_enabled       : 1;
    uint64_t clamp_enabled       : 1;
    uint64_t limit_time_window_y : 5;
    uint64_t limit_time_window_f : 2;
    uint64_t                     : 7;
    uint64_t lock_enabled        : 1;
    uint64_t                     : 32;
} rapl_power_limit_control_msr_t;

/* Updated every ~1ms. Wraparound time of 60s under load. */
typedef struct energy_status_msr_t {
    uint64_t total_energy_consumed : 32;
    uint64_t                       : 32;
} energy_status_msr_t;


/* General */
typedef struct rapl_unit_multiplier_msr_t {
    uint64_t power  : 4;
    uint64_t        : 4;
    uint64_t energy : 5;
    uint64_t        : 3;
    uint64_t time   : 4;
    uint64_t        : 32;
    uint64_t        : 12;
} rapl_unit_multiplier_msr_t;

/*
 *  For documentaion see: "Intel64 and IA-32 Architectures Software Developer's Manual" Volume 4:  Model-Specific registers.
 * https://software.intel.com/sites/default/files/managed/22/0d/335592-sdm-vol-4.pdf In a nutshell, search the manual and 
 * find the MSR number that you wish to read/write.
 * Then use the read_msr_t/write_msr_t functions and extract_bit functions to get the info you need.
 */

/**
 * Read the given MSR on the given CPU.
 *
 * @return            0 on success and MY_ERROR on failure
 */
int read_msr(int cpu, uint64_t address, uint64_t *val);

/**
 * Write the given value to the given MSR on the given CPU.
 *
 * @return            0 on success and MY_ERROR on failure
 */
int write_msr(int cpu, uint64_t address, uint64_t val);
#endif
