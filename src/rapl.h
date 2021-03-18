// This file is part of CPU Energy Meter,
// a tool for measuring energy consumption of Intel CPUs:
// https://github.com/sosy-lab/cpu-energy-meter
//
// SPDX-FileCopyrightText: 2012 Intel Corporation
// SPDX-FileCopyrightText: 2015-2021 Dirk Beyer <https://www.sosy-lab.org>
//
// SPDX-License-Identifier: BSD-3-Clause

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
