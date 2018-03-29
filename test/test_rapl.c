#include <sched.h>

#include "unity.h"
#include "rapl.h"

#include "rapl.h"
#include "mock_cpuid.h"
#include "mock_msr.h"
#include "mock_util.h"


const uint32_t INTEL_SIG = 526057;
const cpuid_info_t CPUID = { 22, 1970169159, 1818588270, 1231384169 };


void setUp(void) {
  read_msr_IgnoreAndReturn(0); // make each msr available in the table
  config_msr_table();
}

void tearDown(void) {
  close_msr_fd_Ignore();
  terminate_rapl();
}


void test_ConfigMsrTable_SuccessWhenSetUpCorrectly(void) {
  // Test only a selection of registers (one from each domain)
  TEST_ASSERT_TRUE(is_supported_msr(MSR_RAPL_POWER_UNIT));

  TEST_ASSERT_TRUE(is_supported_msr(MSR_RAPL_PKG_POWER_LIMIT));
  TEST_ASSERT_TRUE(is_supported_msr(MSR_RAPL_DRAM_POWER_LIMIT));
  TEST_ASSERT_TRUE(is_supported_msr(MSR_RAPL_PP0_POWER_LIMIT));
  TEST_ASSERT_TRUE(is_supported_msr(MSR_RAPL_PP1_POWER_LIMIT));
  TEST_ASSERT_TRUE(is_supported_msr(MSR_RAPL_PLATFORM_ENERGY_STATUS));
}

void test_ConfigMsrTable_should_DisableMsrWhenNotAvailable(void) {
  int cpu = 0;
  uint64_t msr = 0;

  int read_msr_retval = 1;

  // Disable the msr-table for the following 4 register:
  read_msr_ExpectAndReturn(cpu, MSR_RAPL_POWER_UNIT, &msr, read_msr_retval);
  read_msr_ExpectAndReturn(cpu, MSR_RAPL_PKG_POWER_LIMIT, &msr, read_msr_retval);
  read_msr_ExpectAndReturn(cpu, MSR_RAPL_PKG_ENERGY_STATUS, &msr, read_msr_retval);
  read_msr_ExpectAndReturn(cpu, MSR_RAPL_PKG_POWER_INFO, &msr, read_msr_retval);
  
  // Enable the msr-table for everything else
  read_msr_IgnoreAndReturn(0);

  // Test the above config
  config_msr_table();
  TEST_ASSERT_FALSE(is_supported_msr(MSR_RAPL_POWER_UNIT));
  TEST_ASSERT_FALSE(is_supported_msr(MSR_RAPL_PKG_POWER_LIMIT));
  TEST_ASSERT_FALSE(is_supported_msr(MSR_RAPL_PKG_ENERGY_STATUS));
  TEST_ASSERT_FALSE(is_supported_msr(MSR_RAPL_PKG_POWER_INFO));

  TEST_ASSERT_TRUE(is_supported_msr(MSR_RAPL_DRAM_POWER_LIMIT));
  TEST_ASSERT_TRUE(is_supported_msr(MSR_RAPL_PP0_POWER_LIMIT));
  TEST_ASSERT_TRUE(is_supported_msr(MSR_RAPL_PP1_POWER_LIMIT));
  TEST_ASSERT_TRUE(is_supported_msr(MSR_RAPL_PLATFORM_ENERGY_STATUS));
}

void test_InitRapl_should_ReturnErrWhenNoIntelSig(void) {
  // some arbitrary values that don't translate to 'GenuineIntel'
  cpuid_info_t wrong_cpuid = { 0, 0, 0, 0 };

  get_vendor_signature_IgnoreAndReturn(wrong_cpuid);
  get_vendor_name_Ignore();

  int retval = init_rapl();
  TEST_ASSERT_EQUAL(MY_ERROR, retval);
}

void test_InitRapl_SuccessWhenFam6CPU(void) {
  get_vendor_signature_CMockIgnoreAndReturn(0, CPUID);
  get_vendor_name_CMockIgnore();
  get_processor_signature_IgnoreAndReturn(INTEL_SIG);

  unsigned int exp_family = 6;

  uint32_t processor_signature = get_processor_signature();
  unsigned int act_family = (processor_signature >> 8) & 0xf;


  TEST_ASSERT_EQUAL(exp_family, act_family);
}

void test_InitRapl_ReturnErrWhenNoFam6CPU(void) {
  get_vendor_signature_CMockIgnoreAndReturn(0, CPUID);
  get_vendor_name_CMockIgnore();

  uint32_t wrongSig = 0; // some arbitrary value
  get_processor_signature_IgnoreAndReturn(wrongSig);

  unsigned int exp_family = 6;

  uint32_t processor_signature = get_processor_signature();
  unsigned int act_family = (processor_signature >> 8) & 0xf;

  int retval = init_rapl();
  TEST_ASSERT_FALSE(exp_family == act_family);
  TEST_ASSERT_EQUAL(MY_ERROR, retval);
}

void test_IsSupportedDomain_ReturnsCorrectValues(void) {
  int cpu = 0;
  uint64_t msr = 0;
  int enable_msr = 0;
 
  // Config the msr-table such that afterwards only the test for MSR_RAPL_DRAM
  // will evaluate to true.
 
  // Disable the msr-table for everything else
  read_msr_ExpectAndReturn(cpu, MSR_RAPL_POWER_UNIT, &msr, !enable_msr);

  // Disable msr-table for PKG_POWER_LIMIT and PKG_ENERGY_STATUS:
  read_msr_ExpectAndReturn(cpu, MSR_RAPL_PKG_POWER_LIMIT, &msr, !enable_msr);
  read_msr_ExpectAndReturn(cpu, MSR_RAPL_PKG_ENERGY_STATUS, &msr, !enable_msr);
  read_msr_ExpectAndReturn(cpu, MSR_RAPL_PKG_POWER_INFO, &msr, !enable_msr);

  // Enable msr-table for DRAM_POWER_LIMIT and DRAM_ENERGY_STATUS:
  read_msr_ExpectAndReturn(cpu, MSR_RAPL_DRAM_POWER_LIMIT, &msr, enable_msr);
  read_msr_ExpectAndReturn(cpu, MSR_RAPL_DRAM_ENERGY_STATUS, &msr, enable_msr);

  // Enable msr-table for PP0_POWER_LIMIT; disable for PP0_ENERGY_STATUS:
  read_msr_ExpectAndReturn(cpu, MSR_RAPL_PP0_POWER_LIMIT, &msr, enable_msr);
  read_msr_ExpectAndReturn(cpu, MSR_RAPL_PP0_ENERGY_STATUS, &msr, !enable_msr);

  // Disable msr-table for PP1_POWER_LIMIT; enable for PP1_ENERGY_STATUS:
  read_msr_ExpectAndReturn(cpu, MSR_RAPL_PP1_POWER_LIMIT, &msr, !enable_msr);
  read_msr_ExpectAndReturn(cpu, MSR_RAPL_PP1_ENERGY_STATUS, &msr, enable_msr);

  read_msr_IgnoreAndReturn(!enable_msr);

  // Test method #is_supported_domain(uint64_t) with the above config:
  // Only RAPL_DRAM domain should evaluate to true.
  config_msr_table();

  TEST_ASSERT_FALSE(is_supported_domain(0)); // RAPL_PKG
  TEST_ASSERT_FALSE(is_supported_domain(1)); // RAPL_PP0
  TEST_ASSERT_FALSE(is_supported_domain(2)); // RAPL_PP1
  TEST_ASSERT_TRUE(is_supported_domain(3)); // RAPL_DRAM
}

void test_GetRaplUnitMultiplier_ReturnsCorrectValues(void) {
  uint64_t msr = 658947;  // exp. value for MSR_RAPL_POWER_UNIT
  rapl_unit_multiplier_msr_t unit_msr = *(rapl_unit_multiplier_msr_t *)&msr;

  int delta = 1e-14;
  double exp_time = 0.0009765625;
  double exp_energy = 6.103515625e-05;
  double exp_power = 0.125;

  double act_time = 1.0 / (double)(B2POW(unit_msr.time));
  double act_energy = 1.0 / (double)(B2POW(unit_msr.energy));
  double act_power = 1.0 / (double)(B2POW(unit_msr.power));

  TEST_ASSERT_FLOAT_WITHIN(delta, exp_time, act_time);
  TEST_ASSERT_FLOAT_WITHIN(delta, exp_energy, act_energy);
  TEST_ASSERT_FLOAT_WITHIN(delta, exp_power, act_power);
}

void test_GetPkgTotalEnergyConsumed_ComputesCorrectValue(void) {
  RAPL_ENERGY_UNIT = 6.103515625e-5;
  uint64_t msr_address = MSR_RAPL_PKG_ENERGY_STATUS;
  uint64_t msr = 0;
  int cpu = 0;

  uint64_t read_msr_ret_ptr_val = 494516256; // some arbitrary value
  read_msr_ExpectAndReturn(cpu, msr_address, &msr, 0);
  read_msr_ReturnThruPtr_val(&read_msr_ret_ptr_val);

  double consumed_energy = 0;
  int retval = get_total_energy_consumed(cpu, msr_address, &consumed_energy);
  TEST_ASSERT_EQUAL_INT(0, retval);
  TEST_ASSERT_GREATER_THAN(0, consumed_energy);

  double exp_consumed_energy = 30182.876953125;
  energy_status_msr_t domain_msr = *(energy_status_msr_t *)&read_msr_ret_ptr_val;
  double act_consumed_energy = RAPL_ENERGY_UNIT * domain_msr.total_energy_consumed;
  TEST_ASSERT_FLOAT_WITHIN(0.001, exp_consumed_energy, act_consumed_energy);

}

void test_ReadRaplUnits_ReturnsCorrectValues(void) {
  uint64_t msr = 0;
  int read_msr_retval = 0;
  uint64_t read_msr_ret_ptr_val = 658947;  // exp. value for MSR_RAPL_POWER_UNIT

  read_msr_ExpectAndReturn(0, MSR_RAPL_POWER_UNIT, &msr, read_msr_retval);
  read_msr_ReturnThruPtr_val(&read_msr_ret_ptr_val);

  int retval = read_rapl_units();
  TEST_ASSERT_TRUE(is_supported_msr(MSR_RAPL_POWER_UNIT));

  int delta = 1e-14;
  double exp_rapl_time_unit = 0.0009765625;
  double exp_rapl_energy_unit = 6.103515625e-05;
  double exp_rapl_dram_energy_unit = rapl_dram_energy_units_probe(exp_rapl_energy_unit);
  double exp_rapl_power_unit = 0.125;

  TEST_ASSERT_FLOAT_WITHIN(delta, exp_rapl_time_unit, RAPL_TIME_UNIT);
  TEST_ASSERT_FLOAT_WITHIN(delta, exp_rapl_energy_unit, RAPL_ENERGY_UNIT);
  TEST_ASSERT_FLOAT_WITHIN(delta, exp_rapl_dram_energy_unit, RAPL_DRAM_ENERGY_UNIT);
  TEST_ASSERT_FLOAT_WITHIN(delta, exp_rapl_power_unit, RAPL_POWER_UNIT);
}


