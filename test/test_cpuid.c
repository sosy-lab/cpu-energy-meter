#include "unity.h"
#include "cpuid.h"

void test_GetVendorName_should_ReturnGenuineIntel(void) {
  char expected[20] = "GenuineIntel";

  char actual[20];
  get_vendor_name(actual);
  
  TEST_ASSERT_EQUAL_STRING(expected, actual);
}

void test_GetVendorSignature_should_ReturnCorrectCpuIdValues(void) {
  uint32_t exp_ebx = 0x756e6547; // translates to "Genu"
  uint32_t exp_ecx = 0x6c65746e; // translates to "ineI"
  uint32_t exp_edx = 0x49656e69; // translates to "ntel"

  cpuid_info_t sig;
  sig = get_vendor_signature();

  TEST_ASSERT_EQUAL_UINT32(exp_ebx, sig.ebx);
  TEST_ASSERT_EQUAL_UINT32(exp_ecx, sig.ecx);
  TEST_ASSERT_EQUAL_UINT32(exp_edx, sig.edx);
}

void test_GetProcessorSignature_should_ReturnFamily6(void) {
  unsigned int expected = 0x6;

  unsigned int family;
  uint32_t processor_signature = get_processor_signature();
  family = (processor_signature >> 8) & 0xf;

  TEST_ASSERT_EQUAL_UINT(expected, family);
}
