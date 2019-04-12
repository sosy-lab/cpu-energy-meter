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

#include <stdint.h>

/* Power Domains */
enum RAPL_DOMAIN {
  RAPL_PKG,
  RAPL_PP0,
  RAPL_PP1,
  RAPL_DRAM,
  /**
   * According to Intel Software Devlopers Manual Volume 4, Table 2-38, the consumed energy is the
   * total energy consumed by all devices in the plattform that receive power from integrated power
   * delivery mechanism. Included plattform devices are processor cores, SOC, memory, add-on or
   * peripheral devies that get powered directly from the platform power delivery means.
   */
  RAPL_PSYS,
};
#define RAPL_NR_DOMAIN 5 /* Number of power domains */

const char *const RAPL_DOMAIN_STRINGS[RAPL_NR_DOMAIN];
const char *const RAPL_DOMAIN_FORMATTED_STRINGS[RAPL_NR_DOMAIN];

/*!
 * This function must be called before calling any other function from this module.
 * Returns 0 on success, 1 on failure.
 * To free resources, call terminate_rapl() in the end.
 */
int init_rapl();

/**
 * Call this function function to cleanup resources.
 */
void terminate_rapl();

// Wraparound value for the total energy consumed. It is computed within init_rapl().
double MAX_ENERGY_STATUS_JOULES; /* default: 65536 */

int get_num_rapl_nodes();

int is_supported_domain(enum RAPL_DOMAIN power_domain);

/**
 * Read the energy consumed in joules for the given power domain of the given node
 * since the last machine reboot (or energy-register wraparound).
 *
 * Returns 0 on success, -1 otherwise
 */
int get_total_energy_consumed(
    int node, enum RAPL_DOMAIN power_domain, double *total_energy_consumed_joules);

/**
 * Read measurements for all nodes and domains and write them to current_measurements.
 * If cum_energy_J is not NULL, read previous measurements from current_measurements
 * and accumulate delta in cum_energy_J.
 */
int get_total_energy_consumed_for_nodes(
    int num_node,
    double current_measurements[num_node][RAPL_NR_DOMAIN],
    double cum_energy_J[num_node][RAPL_NR_DOMAIN]);

/**
 * Calculate how often the RAPL values need to be read such that overflows can be detected reliably.
 * The goal is to measure as rarely as possible, but often enough so that no overflow will be
 * missed out.
 *
 * Returns the number of seconds that should be waited between reads at most.
 */
long get_maximum_read_interval();

#endif
