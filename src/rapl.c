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

/*
 * For reference, see
 * http://software.intel.com/en-us/articles/power-gov
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include "rapl.h"
#include "cpuinfo.h"
#include "intel-family.h"
#include "msr.h"
#include "rapl-impl.h"
#include "util.h"

#include <assert.h>
#include <err.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#ifdef TEST // don't print the error-msg when unit-testing
#define warnx(...)
#endif

static const int FALLBACK_THERMAL_SPEC_POWER =
    200.0; // maximum power in watts that we assume if we cannot read it
static const int MIN_THERMAL_SPEC_POWER =
    1.0e-03; // minimum power in watts that we assume as a legal value

const char *const RAPL_DOMAIN_STRINGS[RAPL_NR_DOMAIN] = {
    "package", "core", "uncore", "dram", "psys"};
const char *const RAPL_DOMAIN_FORMATTED_STRINGS[RAPL_NR_DOMAIN] = {
    "Package", "Core", "Uncore", "DRAM", "PSYS"};

/* rapl msr availablility */
#define MSR_SUPPORT_MASK 0xff
unsigned char *msr_support_table;

/* Global Variables */
double RAPL_TIME_UNIT;
double RAPL_ENERGY_UNIT;
double RAPL_DRAM_ENERGY_UNIT;
double RAPL_POWER_UNIT;

static int num_nodes = 0;

static int *pkg_map; // node-to-cpu mapping

static unsigned int umax(unsigned int a, unsigned int b) {
  return a > b ? a : b;
}

// For documentation, see:
// http://software.intel.com/en-us/articles/intel-64-architecture-processor-topology-enumeration
static int build_topology() {
  assert(num_nodes == 0);
  assert(pkg_map == NULL);

  const long os_cpu_count = sysconf(_SC_NPROCESSORS_CONF);
  assert(os_cpu_count < INT_MAX);
  int max_pkg = 0;

  // Construct an os map: os_map[APIC_ID ... APIC_ID]
  APIC_ID_t os_map[os_cpu_count];
  for (int i = 0; i < os_cpu_count; i++) {

    if (get_core_information(i, &(os_map[i])) != 0) {
      return -1;
    }

    if (os_map[i].pkg_id > max_pkg) {
      max_pkg = os_map[i].pkg_id;
    }
  }

  num_nodes = max_pkg + 1;

  // Construct a pkg map: pkg_map[pkg id] = (os_id of first thread on pkg)
  pkg_map = (int *)malloc(num_nodes * sizeof(int));

  for (int i = 0; i < os_cpu_count; i++) {
    int p = os_map[i].pkg_id;
    assert(p < num_nodes);
    if (os_map[i].smt_id == 0 && os_map[i].core_id == 0) {
      pkg_map[p] = i;
    }
  }

  return 0;
}

static void set_value_in_msr_table(off_t address) {
  const int cpu = 0;
  uint64_t msr;
  msr_support_table[address & MSR_SUPPORT_MASK] = !read_msr(cpu, address, &msr);
}

void config_msr_table() {
  assert(msr_support_table == NULL);
  msr_support_table = (unsigned char *)calloc(MSR_SUPPORT_MASK, sizeof(unsigned char));

  // read every MSR once and set flag in msr_support_table accordingly
  set_value_in_msr_table(MSR_RAPL_POWER_UNIT);
  set_value_in_msr_table(MSR_RAPL_PKG_ENERGY_STATUS);
  set_value_in_msr_table(MSR_RAPL_PKG_POWER_INFO);
  set_value_in_msr_table(MSR_RAPL_DRAM_ENERGY_STATUS);
  set_value_in_msr_table(MSR_RAPL_PP0_ENERGY_STATUS);
  set_value_in_msr_table(MSR_RAPL_PP1_ENERGY_STATUS);
  set_value_in_msr_table(MSR_RAPL_PLATFORM_ENERGY_STATUS);
  if (is_debug_enabled()) {
    for (int domain = 0; domain < RAPL_NR_DOMAIN; domain++) {
      DEBUG(
          "Domain %s is %ssupported.",
          RAPL_DOMAIN_FORMATTED_STRINGS[domain],
          is_supported_domain(domain) ? "" : "NOT ");
    }
  }
}

int get_cpu_from_node(int node) {
#ifndef TEST
  return pkg_map[node];
#else // simply return value 0 when unit-testing, as the pkg_map isn't initialized then
  return 0;
#endif
}

int check_if_supported_processor(uint32_t *current_processor_signature) {
  char vendor[VENDOR_LENGTH];
  get_vendor_name(vendor);
  if (!is_intel_processor()) {
    warnx(
        "The processor on the working machine is not from Intel. Found %s processor instead.\n",
        vendor);
    return -1;
  }
  DEBUG("%s processor found.", vendor);

  const uint32_t processor_signature = get_processor_signature();
  const unsigned int family = (processor_signature >> 8) & 0xf;
  DEBUG(
      "Processor is from family %d and uses model 0x%05X.",
      family,
      processor_signature & 0xfffffff0);
  if (family != 6) {
    // CPUID.family == 6 means it's anything from Pentium Pro (1995) to the latest Kaby Lake (2017)
    // except "Netburst"
    warnx(
        "The Intel processor must be from family 6, but instead a CPU from family %d was found.\n",
        family);
    return -1;
  }

  *current_processor_signature = processor_signature;
  return 0;
}

int init_rapl() {
  uint32_t processor_signature;
  if (check_if_supported_processor(&processor_signature) != 0) {
    return -1;
  }

  if (build_topology() != 0) {
    goto err;
  }

  if (open_msr_fd(num_nodes, &get_cpu_from_node) != 0) {
    goto err;
  }

  config_msr_table();

  if (read_rapl_units(processor_signature) != 0) {
    goto err;
  }

  /* 32 is the width of these fields when they are stored */
  MAX_ENERGY_STATUS_JOULES = (double)(RAPL_ENERGY_UNIT * (pow(2, 32) - 1));

  return 0;

err:
  terminate_rapl();
  return -1;
}

void terminate_rapl() {
  // This function should work correctly no matter in what state it is called.
  close_msr_fd();

  if (NULL != pkg_map) {
    free(pkg_map);
    pkg_map = NULL;
  }

  if (NULL != msr_support_table) {
    free(msr_support_table);
    msr_support_table = NULL;
  }

  num_nodes = 0;
}

int is_supported_msr(off_t msr) {
  return msr_support_table[msr & MSR_SUPPORT_MASK];
}

static off_t get_msr_for_domain(enum RAPL_DOMAIN power_domain) {
  switch (power_domain) {
  case RAPL_PKG:
    return MSR_RAPL_PKG_ENERGY_STATUS;
  case RAPL_PP0:
    return MSR_RAPL_PP0_ENERGY_STATUS;
  case RAPL_PP1:
    return MSR_RAPL_PP1_ENERGY_STATUS;
  case RAPL_DRAM:
    return MSR_RAPL_DRAM_ENERGY_STATUS;
  case RAPL_PSYS:
    return MSR_RAPL_PLATFORM_ENERGY_STATUS;
  default:
    abort();
  }
}

/*!
 * \brief Check if power domain (PKG, PP0, PP1, DRAM) is supported on this machine.
 *
 * Currently server parts support: PKG, PP0 and DRAM and client parts support PKG, PP0 and PP1.
 *
 * \return 1 if supported, 0 otherwise
 */
int is_supported_domain(enum RAPL_DOMAIN power_domain) {
  return is_supported_msr(get_msr_for_domain(power_domain));
}

/*!
 * \brief Get the number of RAPL nodes on this machine.
 *
 * Get the number of package power domains, that you can control using RAPL. This is equal to the
 * number of CPU packages in the system.
 *
 * \return number of RAPL nodes.
 */
int get_num_rapl_nodes() {
  return num_nodes;
}

int get_total_energy_consumed_via_msr(
    int node, off_t msr_address, double *total_energy_consumed_joules) {
  if (!is_supported_msr(msr_address)) {
    return -1;
  }

  uint64_t msr;
  cpu_set_t old_context;
  bind_cpu(get_cpu_from_node(node), &old_context); // improve performance on Linux
  int result = read_msr(node, msr_address, &msr);
  bind_context(&old_context, NULL);

  if (result != 0) {
    return -1;
  }
  energy_status_msr_t energy_status;
  energy_status.as_uint64_t = msr;

  const double energy_unit =
      (msr_address == MSR_RAPL_DRAM_ENERGY_STATUS) ? RAPL_DRAM_ENERGY_UNIT : RAPL_ENERGY_UNIT;
  *total_energy_consumed_joules = energy_unit * energy_status.fields.total_energy_consumed;

  return 0;
}

int get_total_energy_consumed(
    int node, enum RAPL_DOMAIN power_domain, double *total_energy_consumed_joules) {
  return get_total_energy_consumed_via_msr(
      node, get_msr_for_domain(power_domain), total_energy_consumed_joules);
}

int get_total_energy_consumed_for_nodes(
    int num_node,
    double current_measurements[num_node][RAPL_NR_DOMAIN],
    double cum_energy_J[num_node][RAPL_NR_DOMAIN]) {
  int result = 0;
  for (int i = 0; i < num_node; i++) {
    for (int domain = 0; domain < RAPL_NR_DOMAIN; ++domain) {
      if (is_supported_domain(domain)) {
        double new_sample;
        if (get_total_energy_consumed(i, domain, &new_sample) != 0) {
          warnx("Measuring domain %s of CPU %d failed.", RAPL_DOMAIN_FORMATTED_STRINGS[domain], i);
          result = 1;
          continue; // at least continue reading other domains
        }

        if (cum_energy_J != NULL) {
          double delta = new_sample - current_measurements[i][domain];

          /* Handle wraparound */
          if (delta < 0) {
            delta += MAX_ENERGY_STATUS_JOULES;
          }

          cum_energy_J[i][domain] += delta;
        }

        current_measurements[i][domain] = new_sample;
      }
    }
  }

  return result;
}

long get_maximum_read_interval() {
  // get maximum power consumption over all nodes (this will lead to the fastest overflow)
  double max_power = 1;
  for (int node = 0; node < num_nodes; node++) {
    max_power = fmax(max_power, get_max_power(node));
  }

  // get smallest energy unit over all domains (this will lead to the fastest overflow)
  double energy_unit = fmin(RAPL_ENERGY_UNIT, RAPL_DRAM_ENERGY_UNIT);

  double seconds = ((pow(2, 32) - 1) * energy_unit) / max_power;

  // divide by two to guarantee that we measure twice between overflows
  seconds = seconds / 2;
  assert(seconds >= 2);

  return floor(seconds - 1);
}

double get_max_power(int node) {
  if (!is_supported_msr(MSR_RAPL_PKG_POWER_INFO)) {
    goto err;
  }

  uint64_t msr;
  if (read_msr(node, MSR_RAPL_PKG_POWER_INFO, &msr) != 0) {
    goto err;
  }

  rapl_parameters_msr_t rapl_parameters;
  rapl_parameters.as_uint64_t = msr;
  const unsigned int max_raw_power =
      umax(rapl_parameters.fields.thermal_spec_power, rapl_parameters.fields.maximum_power);
  const double max_power_watts = max_raw_power * RAPL_POWER_UNIT;

  if (max_power_watts > MIN_THERMAL_SPEC_POWER) {
    DEBUG("Max power consumption of node %d is %fW.", node, max_power_watts);
    return max_power_watts;
  }

err:
  return FALLBACK_THERMAL_SPEC_POWER;
}

int read_rapl_units(uint32_t processor_signature) {
  if (!is_supported_msr(MSR_RAPL_POWER_UNIT)) {
    return -1;
  }

  uint64_t msr;
  if (read_msr(0, MSR_RAPL_POWER_UNIT, &msr) != 0) {
    return -1;
  }

  rapl_unit_multiplier_msr_t units;
  units.as_uint64_t = msr;
  RAPL_TIME_UNIT = RAW_UNIT_TO_DOUBLE(units.fields.time);
  RAPL_ENERGY_UNIT = RAW_UNIT_TO_DOUBLE(units.fields.energy);
  RAPL_POWER_UNIT = RAW_UNIT_TO_DOUBLE(units.fields.power);

  switch (processor_signature & 0xfffffff0) {
  case CPU_INTEL_HASWELL_X:
  case CPU_INTEL_BROADWELL_X:
  case CPU_INTEL_BROADWELL_XEON_D:
  case CPU_INTEL_SKYLAKE_X:
  case CPU_INTEL_XEON_PHI_KNL:
  case CPU_INTEL_XEON_PHI_KNM:
    RAPL_DRAM_ENERGY_UNIT = 15.3E-6;
    break;
  default:
    RAPL_DRAM_ENERGY_UNIT = RAPL_ENERGY_UNIT;
  }

  DEBUG(
      "Measured the following unit multipliers:"
      "   RAPL_ENERGY_UNIT=%0.6eJ   RAPL_DRAM_ENERGY_UNIT=%0.6eJ",
      RAPL_ENERGY_UNIT,
      RAPL_DRAM_ENERGY_UNIT);

  return 0;
}
