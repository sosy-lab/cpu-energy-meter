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

/*! \file rapl.h
 *  Library header file.
 */

#ifndef _h_rapl
#define _h_rapl

#include <sched.h>
#include <stdint.h>

#define MY_ERROR -1

/* Power Domains */
#define RAPL_PKG 0       /*!< \brief Package power domain */
#define RAPL_PP0 1       /*!< \brief Core power domain */
#define RAPL_PP1 2       /*!< \brief Uncore power domain */
#define RAPL_DRAM 3      /*!< \brief DRAM power domain */
#define RAPL_PSYS 4      /*!< \brief PLATFORM power domain */
#define RAPL_NR_DOMAIN 5 /*!< \brief Number of power domains */

extern uint64_t debug_enabled;

// Visible for testing
extern double RAPL_TIME_UNIT;
extern double RAPL_ENERGY_UNIT;
extern double RAPL_DRAM_ENERGY_UNIT;
extern double RAPL_POWER_UNIT;

// Visible for testing
extern uint32_t processor_signature;

enum RAPL_DOMAIN { PKG, PP0, PP1, DRAM, PSYS };

char *RAPL_DOMAIN_STRINGS[RAPL_NR_DOMAIN];
char *RAPL_DOMAIN_FORMATTED_STRINGS[RAPL_NR_DOMAIN];

typedef struct APIC_ID_t {
  uint64_t smt_id;
  uint64_t core_id;
  uint64_t pkg_id;
  uint64_t os_id;
} APIC_ID_t;

// Visible for testing
extern APIC_ID_t **pkg_map;

void config_msr_table();

int init_rapl();
int terminate_rapl();

// Wraparound value for the total energy consumed. It is computed within init_rapl().
double MAX_ENERGY_STATUS_JOULES; /* default: 65536 */

uint64_t get_num_rapl_nodes();

uint64_t is_supported_msr(uint64_t msr);
uint64_t is_supported_domain(uint64_t power_domain);

int get_total_energy_consumed(uint64_t cpu, uint64_t msr_address,
                              double *total_energy_consumed_joules);
int get_pkg_total_energy_consumed(uint64_t node, double *total_energy_consumed);
int get_pp0_total_energy_consumed(uint64_t node, double *total_energy_consumed);
int get_pp1_total_energy_consumed(uint64_t node, double *total_energy_consumed);
int get_dram_total_energy_consumed(uint64_t node, double *total_energy_consumed);
int get_psys_total_energy_consumed(uint64_t node, double *total_energy_consumed);

/*! \brief RAPL parameters info structure, PKG domain */
typedef struct pkg_rapl_parameters_t {
  double thermal_spec_power_watts;
  double minimum_power_watts;
  double maximum_power_watts;
  double maximum_limit_time_window_seconds;
} pkg_rapl_parameters_t;
int get_pkg_rapl_parameters(unsigned int node, pkg_rapl_parameters_t *rapl_parameters);

double rapl_dram_energy_units_probe(double rapl_energy_units);
void calculate_probe_interval_time(struct timespec *signal_timelimit, double thermal_spec_power);

/* Utilities */
int read_rapl_units();

#endif
