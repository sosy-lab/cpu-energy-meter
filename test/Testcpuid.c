#include "unity.h"
#include "cpuid.h"

void test_GetVendorName_should_ReturnGenuineIntel(void) {
  char actual[20];
  get_vendor_name(actual);
  
  TEST_ASSERT_EQUAL_STRING("GenuineIntel", actual);
}

void test_GetProcessorSignature_should_ReturnFamily6(void) {
  unsigned int family;
  uint32_t processor_signature = get_processor_signature();
  family = (processor_signature >> 8) & 0xf;

  TEST_ASSERT_EQUAL_UINT(0x6, family);
}

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_GetVendorName_should_ReturnGenuineIntel);
  RUN_TEST(test_GetProcessorSignature_should_ReturnFamily6);
  return UNITY_END();
}

