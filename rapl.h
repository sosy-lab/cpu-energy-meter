/*
Copyright (c) 2012, Intel Corporation

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of Intel Corporation nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/* Written by Martin Dimitrov, Carl Strickland */


/*! \file rapl.h
 *  Library header file.
 */

#ifndef _h_rapl
#define _h_rapl

#include <stdint.h>

#define MY_ERROR -1

/* Power Domains */
#define RAPL_PKG  0      /*!< \brief Package power domain */
#define RAPL_PP0  1      /*!< \brief Core power domain */
#define RAPL_PP1  2      /*!< \brief Uncore power domain */
#define RAPL_DRAM 3      /*!< \brief DRAM power domain */
#define RAPL_NR_DOMAIN 4 /*!< \brief Number of power domains */

enum RAPL_DOMAIN { PKG, PP0, PP1, DRAM };

typedef struct APIC_ID_t {
    uint64_t smt_id;
    uint64_t core_id;
    uint64_t pkg_id;
    uint64_t os_id;
} APIC_ID_t;

int init_rapl();
int terminate_rapl();

/* Wraparound values for total energy consumed and accumulated throttled time.
 * These values are computed within init_rapl(). */
double MAX_ENERGY_STATUS_JOULES;   /* default: 65536 */
double MAX_THROTTLED_TIME_SECONDS; /* default: 4194304 */

uint64_t get_num_rapl_nodes_pkg();
uint64_t get_num_rapl_nodes_pp0();
uint64_t get_num_rapl_nodes_pp1();
uint64_t get_num_rapl_nodes_dram();

uint64_t is_supported_msr(uint64_t msr);
uint64_t is_supported_domain(uint64_t power_domain);

/* General */

/*! \brief RAPL power limit control structure, PKG domain */
typedef struct pkg_rapl_power_limit_control_t {
    double   power_limit_watts_1;
    double   limit_time_window_seconds_1;
    uint64_t limit_enabled_1;
    uint64_t clamp_enabled_1;
    double   power_limit_watts_2;
    double   limit_time_window_seconds_2;
    uint64_t limit_enabled_2;
    uint64_t clamp_enabled_2;
    uint64_t lock_enabled;
} pkg_rapl_power_limit_control_t;

/*! \brief RAPL parameters info structure, PKG domain */
typedef struct pkg_rapl_parameters_t {
    double thermal_spec_power_watts;
    double minimum_power_watts;
    double maximum_power_watts;
    double maximum_limit_time_window_seconds;
} pkg_rapl_parameters_t;
int get_pkg_rapl_power_limit_control_t(uint64_t node, pkg_rapl_power_limit_control_t *rapl_power_limit_control);
int get_pkg_total_energy_consumed(uint64_t node, double *total_energy_consumed);
int get_pkg_rapl_parameters_t(uint64_t node, pkg_rapl_parameters_t *rapl_parameters);
int get_pkg_accumulated_throttled_time(uint64_t node, double *accumulated_throttled_time_seconds);
int set_pkg_rapl_power_limit_control_t(uint64_t node, pkg_rapl_power_limit_control_t *rapl_power_limit_control);

/*! \brief RAPL power limit control structure, DRAM domain */
typedef struct dram_rapl_power_limit_control_t {
    double   power_limit_watts;
    double   limit_time_window_seconds;
    uint64_t limit_enabled;
    uint64_t clamp_enabled;
    uint64_t lock_enabled;
} dram_rapl_power_limit_control_t;

/*! \brief RAPL parameters info structure, DRAM domain */
typedef struct dram_rapl_parameters_t {
    double thermal_spec_power_watts;
    double minimum_power_watts;
    double maximum_power_watts;
    double maximum_limit_time_window_seconds;
} dram_rapl_parameters_t;
int get_dram_rapl_power_limit_control_t(uint64_t node, dram_rapl_power_limit_control_t *rapl_power_limit_control);
int get_dram_total_energy_consumed(uint64_t node, double *total_energy_consumed);
int get_dram_rapl_parameters_t(uint64_t node, dram_rapl_parameters_t *rapl_parameters);
int get_dram_accumulated_throttled_time(uint64_t node, double *accumulated_throttled_time_seconds);
int set_dram_rapl_power_limit_control_t(uint64_t node, dram_rapl_power_limit_control_t *rapl_power_limit_control);


/*! \brief RAPL power limit control structure, PP0 domain */
typedef struct pp0_rapl_power_limit_control_t {
    double   power_limit_watts;
    double   limit_time_window_seconds;
    uint64_t limit_enabled;
    uint64_t clamp_enabled;
    uint64_t lock_enabled;
} pp0_rapl_power_limit_control_t;
int get_pp0_rapl_power_limit_control_t(uint64_t node, pp0_rapl_power_limit_control_t *rapl_power_limit_control);
int get_pp0_total_energy_consumed(uint64_t node, double *total_energy_consumed);
int get_pp0_balance_policy(uint64_t node, uint64_t *priority_level);
int get_pp0_accumulated_throttled_time(uint64_t node, double *accumulated_throttled_time_seconds);
int set_pp0_rapl_power_limit_control_t(uint64_t node, pp0_rapl_power_limit_control_t *rapl_power_limit_control);
int set_pp0_balance_policy(uint64_t node, uint64_t priority_level);


/*! \brief RAPL power limit control structure, PP1 domain */
typedef struct pp1_rapl_power_limit_control_t {
    double   power_limit_watts;
    double   limit_time_window_seconds;
    uint64_t limit_enabled;
    uint64_t clamp_enabled;
    uint64_t lock_enabled;
} pp1_rapl_power_limit_control_t;
int get_pp1_rapl_power_limit_control_t(uint64_t node, pp1_rapl_power_limit_control_t *rapl_power_limit_control);
int get_pp1_total_energy_consumed(uint64_t node, double *total_energy_consumed);
int get_pp1_balance_policy(uint64_t node, uint64_t *priority_level);
int set_pp1_rapl_power_limit_control_t(uint64_t node, pp1_rapl_power_limit_control_t *rapl_power_limit_control);
int set_pp1_balance_policy(uint64_t node, uint64_t priority_level);

/* Utilities */

/*! \brief Use the RDTSC instruction to read the time-stamp counter */
int read_tsc(uint64_t *tsc);

/* Required by Power Gadget */
int get_pp0_freq_mhz(uint64_t node, uint64_t *freq);

#endif
