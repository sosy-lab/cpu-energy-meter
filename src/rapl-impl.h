/*
Copyright (c) 2012, Intel Corporation
Copyright (c) 2015-2018, Dirk Beyer

Redistribution and use in source and binary forms, with or without modification, are permitted
provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions
and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of
conditions and the following disclaimer in the documentation and/or other materials provided with
the distribution.
    * Neither the name of Intel Corporation nor the names of its contributors may be used to endorse
or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/* Replacement for pow() where the base is 2 and the power is unsigned and less than 31 (will get
 * invalid numbers if 31 or greater) */
#define B2POW(e) (((e) == 0) ? 1 : (2 << ((e)-1)))
#define RAW_UNIT_TO_DOUBLE(e) (1.0 / (double)(B2POW(e)))

/* General */
#define MSR_RAPL_POWER_UNIT 0x606 /* Unit Multiplier used in RAPL Interfaces (R/O) */

/* Package RAPL Domain */
#define MSR_RAPL_PKG_ENERGY_STATUS 0x611 /* PKG Energy Status (R/O) */
#define MSR_RAPL_PKG_POWER_INFO 0x614    /* PKG RAPL Parameters (R/O) */

/* DRAM RAPL Domain */
#define MSR_RAPL_DRAM_ENERGY_STATUS 0x619 /* DRAM Energy Status (R/O) */

/* PP0 RAPL Domain */
#define MSR_RAPL_PP0_ENERGY_STATUS 0x639 /* PP0 Energy Status (R/O) */

/* PP1 RAPL Domain */
#define MSR_RAPL_PP1_ENERGY_STATUS 0x641 /* PP1 Energy Status (R/O) */

/* PSYS RAPL Domain */
#define MSR_RAPL_PLATFORM_ENERGY_STATUS 0x64d /* PSYS Energy Status */

/* Common MSR Structures */

/* General */
typedef union {
  uint64_t as_uint64_t;
  struct rapl_unit_multiplier_msr_t {
    uint64_t power : 4;
    uint64_t : 4;
    uint64_t energy : 5;
    uint64_t : 3;
    uint64_t time : 4;
    uint64_t : 32;
    uint64_t : 12;
  } fields;
} rapl_unit_multiplier_msr_t;

/* Updated every ~1ms. Wraparound time of 60s under load. */
typedef union {
  uint64_t as_uint64_t;
  struct energy_status_msr_t {
    uint64_t total_energy_consumed : 32;
    uint64_t : 32;
  } fields;
} energy_status_msr_t;

/* PKG domain */
typedef union {
  uint64_t as_uint64_t;
  struct rapl_parameters_msr_t {
    unsigned int thermal_spec_power : 15;
    unsigned int : 1;
    unsigned int minimum_power : 15;
    unsigned int : 1;
    unsigned int maximum_power : 15;
    unsigned int : 1;
    unsigned int maximum_limit_time_window : 6;
    unsigned int : 10;
  } fields;
} rapl_parameters_msr_t;

extern double RAPL_TIME_UNIT;
extern double RAPL_ENERGY_UNIT;
extern double RAPL_DRAM_ENERGY_UNIT;
extern double RAPL_POWER_UNIT;

void config_msr_table();

/**
 * Check if MSR is supported on this machine.
 */
int is_supported_msr(off_t msr);

int get_total_energy_consumed_via_msr(
    int node, off_t msr_address, double *total_energy_consumed_joules);

/**
 * Get the maximum power that the given node can consume in watts.
 */
double get_max_power(int node);

int read_rapl_units(uint32_t processor_signature);
