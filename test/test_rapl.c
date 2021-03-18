// This file is part of CPU Energy Meter,
// a tool for measuring energy consumption of Intel CPUs:
// https://github.com/sosy-lab/cpu-energy-meter
//
// SPDX-FileCopyrightText: 2018-2021 Dirk Beyer <https://www.sosy-lab.org>
//
// SPDX-License-Identifier: BSD-3-Clause

#include <sched.h>
#include <stdbool.h>
#include <stdlib.h>

#include "unity.h" // needs to be placed before all the other custom h-files
#include "intel-family.h"
#include "mock_cpuinfo.h"
#include "mock_msr.h"
#include "mock_util.h"
#include "rapl.h"
#include "rapl-impl.h"

const uint32_t INTEL_SIG = 526057;

void setUp(void) {
  is_debug_enabled_IgnoreAndReturn(0);
  bind_cpu_IgnoreAndReturn(0);
  bind_context_IgnoreAndReturn(0);
  read_msr_IgnoreAndReturn(0); // make each msr available in the table
  close_msr_fd_Ignore();

  config_msr_table();
}

void tearDown(void) {
  terminate_rapl();
}

void test_ConfigMsrTable_SuccessWhenSetUpCorrectly(void) {
  // Test only a selection of registers (one from each domain)
  TEST_ASSERT_TRUE(is_supported_msr(MSR_RAPL_POWER_UNIT));

  TEST_ASSERT_TRUE(is_supported_msr(MSR_RAPL_PKG_POWER_INFO));
  TEST_ASSERT_TRUE(is_supported_msr(MSR_RAPL_DRAM_ENERGY_STATUS));
  TEST_ASSERT_TRUE(is_supported_msr(MSR_RAPL_PP0_ENERGY_STATUS));
  TEST_ASSERT_TRUE(is_supported_msr(MSR_RAPL_PP1_ENERGY_STATUS));
  TEST_ASSERT_TRUE(is_supported_msr(MSR_RAPL_PLATFORM_ENERGY_STATUS));
}

void test_ConfigMsrTable_should_DisableMsrWhenNotAvailable(void) {
  int cpu = 0;
  int read_msr_retval = 1;

  // Disable the msr-table for the following 4 register:
  read_msr_ExpectAndReturn(cpu, MSR_RAPL_POWER_UNIT, NULL, read_msr_retval);
  read_msr_ExpectAndReturn(cpu, MSR_RAPL_PKG_ENERGY_STATUS, NULL, read_msr_retval);
  read_msr_ExpectAndReturn(cpu, MSR_RAPL_PKG_POWER_INFO, NULL, read_msr_retval);

  // Enable the msr-table for everything else
  read_msr_IgnoreAndReturn(0);

  // Test the above config
  terminate_rapl();
  config_msr_table();
  TEST_ASSERT_FALSE(is_supported_msr(MSR_RAPL_POWER_UNIT));
  TEST_ASSERT_FALSE(is_supported_msr(MSR_RAPL_PKG_ENERGY_STATUS));
  TEST_ASSERT_FALSE(is_supported_msr(MSR_RAPL_PKG_POWER_INFO));

  TEST_ASSERT_TRUE(is_supported_msr(MSR_RAPL_DRAM_ENERGY_STATUS));
  TEST_ASSERT_TRUE(is_supported_msr(MSR_RAPL_PP0_ENERGY_STATUS));
  TEST_ASSERT_TRUE(is_supported_msr(MSR_RAPL_PP1_ENERGY_STATUS));
  TEST_ASSERT_TRUE(is_supported_msr(MSR_RAPL_PLATFORM_ENERGY_STATUS));
}

void test_InitRapl_should_ReturnErrWhenNoIntelSig(void) {
  is_intel_processor_IgnoreAndReturn(false);
  get_vendor_name_Ignore();

  int retval = init_rapl();
  TEST_ASSERT_EQUAL(-1, retval);
}

void test_InitRapl_SuccessWhenFam6CPU(void) {
  is_intel_processor_IgnoreAndReturn(true);
  get_vendor_name_Ignore();
  get_processor_signature_IgnoreAndReturn(INTEL_SIG);

  unsigned int exp_family = 6;

  uint32_t processor_signature = get_processor_signature();
  unsigned int act_family = (processor_signature >> 8) & 0xf;

  TEST_ASSERT_EQUAL(exp_family, act_family);
}

void test_InitRapl_ReturnErrWhenNoFam6CPU(void) {
  is_intel_processor_IgnoreAndReturn(true);
  get_vendor_name_Ignore();

  uint32_t wrongSig = 0; // some arbitrary value
  get_processor_signature_IgnoreAndReturn(wrongSig);

  unsigned int exp_family = 6;

  uint32_t processor_signature = get_processor_signature();
  unsigned int act_family = (processor_signature >> 8) & 0xf;

  int retval = init_rapl();
  TEST_ASSERT_FALSE(exp_family == act_family);
  TEST_ASSERT_EQUAL(-1, retval);
}

void test_IsSupportedDomain_ReturnsCorrectValues(void) {
  int cpu = 0;
  int enable_msr = 0;

  // Config the msr-table such that afterwards only the test for MSR_RAPL_DRAM
  // will evaluate to true.

  // Disable the msr-table for POWER_UNIT
  read_msr_ExpectAndReturn(cpu, MSR_RAPL_POWER_UNIT, NULL, !enable_msr);

  // Disable msr-table for PKG_POWER_INFO and PKG_ENERGY_STATUS
  read_msr_ExpectAndReturn(cpu, MSR_RAPL_PKG_ENERGY_STATUS, NULL, !enable_msr);
  read_msr_ExpectAndReturn(cpu, MSR_RAPL_PKG_POWER_INFO, NULL, !enable_msr);

  // Enable msr-table for DRAM_ENERGY_STATUS
  read_msr_ExpectAndReturn(cpu, MSR_RAPL_DRAM_ENERGY_STATUS, NULL, enable_msr);

  // Disable msr-table for PP0_ENERGY_STATUS
  read_msr_ExpectAndReturn(cpu, MSR_RAPL_PP0_ENERGY_STATUS, NULL, !enable_msr);

  // Disable msr-table for PP1_ENERGY_STATUS
  read_msr_ExpectAndReturn(cpu, MSR_RAPL_PP1_ENERGY_STATUS, NULL, !enable_msr);

  // Disable any remaining msr's
  read_msr_IgnoreAndReturn(!enable_msr);

  // Test method #is_supported_domain(uint64_t) with the above config:
  // Only RAPL_DRAM domain should evaluate to true.
  terminate_rapl();
  config_msr_table();

  TEST_ASSERT_FALSE(is_supported_domain(0)); // RAPL_PKG
  TEST_ASSERT_FALSE(is_supported_domain(1)); // RAPL_PP0
  TEST_ASSERT_FALSE(is_supported_domain(2)); // RAPL_PP1
  TEST_ASSERT_TRUE(is_supported_domain(3));  // RAPL_DRAM
}

void test_GetRaplUnitMultiplier_ReturnsCorrectValues(void) {
  rapl_unit_multiplier_msr_t units;
  units.as_uint64_t = 658947; // arbitrary value

  int delta = 1e-14;
  // exp. values below computed by hand
  double exp_time = 0.0009765625;
  double exp_energy = 6.103515625e-05;
  double exp_power = 0.125;

  double act_time = RAW_UNIT_TO_DOUBLE(units.fields.time);
  double act_energy = RAW_UNIT_TO_DOUBLE(units.fields.energy);
  double act_power = RAW_UNIT_TO_DOUBLE(units.fields.power);

  TEST_ASSERT_FLOAT_WITHIN(delta, exp_time, act_time);
  TEST_ASSERT_FLOAT_WITHIN(delta, exp_energy, act_energy);
  TEST_ASSERT_FLOAT_WITHIN(delta, exp_power, act_power);
}

void test_GetTotalEnergyConsumed_ComputesCorrectValue(void) {
  RAPL_ENERGY_UNIT = 6.103515625e-5;
  uint64_t msr_address = MSR_RAPL_PKG_ENERGY_STATUS;
  int cpu = 0;

  // Set up the expectations to the methods and what they should return
  uint64_t read_msr_ret_ptr_val = 494516256; // some arbitrary value
  read_msr_ExpectAndReturn(cpu, msr_address, NULL, 0);
  read_msr_IgnoreArg_val();
  read_msr_ReturnThruPtr_val(&read_msr_ret_ptr_val);

  // Test that the function completes and returns the correct value
  double consumed_energy = 0;
  int retval = get_total_energy_consumed_via_msr(cpu, msr_address, &consumed_energy);
  TEST_ASSERT_EQUAL_INT(0, retval);
  TEST_ASSERT_GREATER_THAN(0, consumed_energy);

  // Test that the compution is calculated correctly
  double delta = 1e-09;
  double exp_consumed_energy = 30182.876953125;
  energy_status_msr_t energy_status;
  energy_status.as_uint64_t = read_msr_ret_ptr_val;
  double act_consumed_energy = RAPL_ENERGY_UNIT * energy_status.fields.total_energy_consumed;
  TEST_ASSERT_FLOAT_WITHIN(delta, exp_consumed_energy, act_consumed_energy);
}

void test_GetTotalEnergyConsumed_should_DifferCorrectlyBetweenDramAndDefault(void) {
  RAPL_ENERGY_UNIT = 6.103515625e-5;
  RAPL_DRAM_ENERGY_UNIT = 15.3e-6;

  int node = 0;
  int cpu = 0;
  double energy_consumed = 0;

  // Set up mocks for testing RAPL_PKG_ENERGY_STATUS
  uint64_t read_msr_ret_ptr_val = 494516256; // some arbitrary value
  read_msr_ExpectAndReturn(cpu, MSR_RAPL_PKG_ENERGY_STATUS, NULL, 0);
  read_msr_IgnoreArg_val();
  read_msr_ReturnThruPtr_val(&read_msr_ret_ptr_val);

  int exp_retval = 0;
  double delta = 1e-09;
  int act_retval;
  double exp_consumed_energy;
  double act_consumed_energy;

  // Test that for PKG_ENERGY_STATUS, the value from RAPL_ENERGY_UNIT is taken as multiplier
  act_retval = get_total_energy_consumed_via_msr(node, MSR_RAPL_PKG_ENERGY_STATUS, &energy_consumed);
  TEST_ASSERT_EQUAL(exp_retval, act_retval);

  energy_status_msr_t energy_status;
  energy_status.as_uint64_t = read_msr_ret_ptr_val;
  exp_consumed_energy = 30182.876953125; // value computed by hand
  act_consumed_energy = RAPL_ENERGY_UNIT * energy_status.fields.total_energy_consumed;
  TEST_ASSERT_FLOAT_WITHIN(delta, exp_consumed_energy, act_consumed_energy);

  // Set up mocks for testing RAPL_DRAM_ENERGY_STATUS
  read_msr_ret_ptr_val = 37908518; // some arbitrary value
  read_msr_ExpectAndReturn(cpu, MSR_RAPL_DRAM_ENERGY_STATUS, NULL, 0);
  read_msr_IgnoreArg_val();
  read_msr_ReturnThruPtr_val(&read_msr_ret_ptr_val);

  // Test that for DRAM_ENERGY_STATUS, the value from RAPL_DRAM_ENERGY_UNIT is taken as multiplier
  act_retval = get_total_energy_consumed_via_msr(node, MSR_RAPL_DRAM_ENERGY_STATUS, &energy_consumed);
  TEST_ASSERT_EQUAL(exp_retval, act_retval);

  energy_status.as_uint64_t = read_msr_ret_ptr_val;
  exp_consumed_energy = 580.0003254; // value computed by hand
  act_consumed_energy = RAPL_DRAM_ENERGY_UNIT * energy_status.fields.total_energy_consumed;
  TEST_ASSERT_FLOAT_WITHIN(delta, exp_consumed_energy, act_consumed_energy);
}

void test_GetPkgRaplParameters_ReturnsCorrectValues(void) {
  RAPL_POWER_UNIT = 0.125;

  // Setup mocks
  int node = 0;
  uint64_t read_msr_ret_ptr_val = 120; // some arbitrary value
  read_msr_ExpectAndReturn(node, MSR_RAPL_PKG_POWER_INFO, NULL, 0);
  read_msr_IgnoreArg_val();
  read_msr_ReturnThruPtr_val(&read_msr_ret_ptr_val);

  double retval = get_max_power(node);
  TEST_ASSERT_EQUAL(120*RAPL_POWER_UNIT, retval); // exp. 15W
}

void test_CalculateProbeIntervalTime_ReturnsCorrectValues(void) {
  RAPL_ENERGY_UNIT = 6.103515625e-05; // some arbitrary value
  RAPL_DRAM_ENERGY_UNIT = 6.103515625e-05; // some arbitrary value

  long exp_result_sec = 131070; // computed by hand; value equates to roughly 36h

  long seconds = get_maximum_read_interval();

  // RAPL_ENERGY_UNIT with thermal_spec_power of 1W should lead to almost 73 hours
  // until an overflow comes to pass. We take half of this value as interval time
  // for the probes of the msr's.
  TEST_ASSERT_EQUAL_INT64(exp_result_sec, seconds);
}

static void check_ReadRaplUnits_ExpectedValues(uint32_t processor_signature, double exp_rapl_dram_energy_unit) {
  int read_msr_retval = 0;
  uint64_t read_msr_ret_ptr_val = 658947; // expected value for MSR_RAPL_POWER_UNIT

  read_msr_ExpectAndReturn(0, MSR_RAPL_POWER_UNIT, NULL, read_msr_retval);
  read_msr_IgnoreArg_val();
  read_msr_ReturnThruPtr_val(&read_msr_ret_ptr_val);

  int retval = read_rapl_units(processor_signature);
  TEST_ASSERT_EQUAL_INT(retval, 0);
  TEST_ASSERT_TRUE(is_supported_msr(MSR_RAPL_POWER_UNIT));

  int delta = 1e-14;
  double exp_rapl_time_unit = 0.0009765625;
  double exp_rapl_energy_unit = 6.103515625e-05;
  double exp_rapl_power_unit = 0.125;

  TEST_ASSERT_FLOAT_WITHIN(delta, exp_rapl_time_unit, RAPL_TIME_UNIT);
  TEST_ASSERT_FLOAT_WITHIN(delta, exp_rapl_energy_unit, RAPL_ENERGY_UNIT);
  TEST_ASSERT_FLOAT_WITHIN(delta, exp_rapl_dram_energy_unit, RAPL_DRAM_ENERGY_UNIT);
  TEST_ASSERT_FLOAT_WITHIN(delta, exp_rapl_power_unit, RAPL_POWER_UNIT);
}

void test_ReadRaplUnits_ReturnsCorrectValues(void) {
  const double exp_retval_server = 15.3E-6;
  const double exp_retval_regular = 6.103515625e-05;

  check_ReadRaplUnits_ExpectedValues(CPU_INTEL_SANDYBRIDGE, exp_retval_regular);
  check_ReadRaplUnits_ExpectedValues(CPU_INTEL_BROADWELL_CORE, exp_retval_regular);
  check_ReadRaplUnits_ExpectedValues(CPU_INTEL_SKYLAKE_DESKTOP, exp_retval_regular);

  check_ReadRaplUnits_ExpectedValues(CPU_INTEL_BROADWELL_X, exp_retval_server);
  check_ReadRaplUnits_ExpectedValues(CPU_INTEL_HASWELL_X, exp_retval_server);
  check_ReadRaplUnits_ExpectedValues(CPU_INTEL_SKYLAKE_X, exp_retval_server);
}
