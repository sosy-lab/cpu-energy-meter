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

#include <stdio.h>

#include "cpuid.h"

void cpuid(uint32_t eax_in, uint32_t ecx_in, cpuid_info_t *ci) {
#ifndef TEST
  asm(
#if defined(__LP64__)       /* 64-bit architecture */
      "cpuid;"              /* execute the cpuid instruction */
      "movl %%ebx, %[ebx];" /* save ebx output */
#else                       /* 32-bit architecture */
      "pushl %%ebx;"        /* save ebx */
      "cpuid;"              /* execute the cpuid instruction */
      "movl %%ebx, %[ebx];" /* save ebx output */
      "popl %%ebx;"         /* restore ebx */
#endif
      : "=a"(ci->eax), [ebx] "=r"(ci->ebx), "=c"(ci->ecx), "=d"(ci->edx)
      : "a"(eax_in), "c"(ecx_in));

#else // when unit-testing, compile with -DTest for using preset values instead of executing the
      // above
  ci->eax = 0x806e9;
  ci->ebx = 0x756e6547;
  ci->ecx = 0x6c65746e;
  ci->edx = 0x49656e69;
#endif
}

cpuid_info_t get_vendor_signature() {
  cpuid_info_t info;
  cpuid(0, 0, &info);
  return info;
}

uint32_t get_processor_signature() {
  cpuid_info_t info;
  cpuid(0x1, 0, &info);
  return info.eax;
}

cpuid_info_t get_processor_topology(uint32_t level) {
  cpuid_info_t info;
  cpuid(0xb, level, &info);
  return info;
}

void cast_uint_to_str(char *out, uint32_t in) {
  int i;
  uint32_t mask = 0x000000ff;
  for (i = 0; i < 4; i++) {
    out[i] = (char)((in & (mask << (i * 8))) >> (i * 8));
  }
  out[4] = '\0';
}

void get_vendor_name(char *vendor) {
  cpuid_info_t c;
  char vendor1[5];
  char vendor2[5];
  char vendor3[5];
  cpuid(0, 0, &c);
  cast_uint_to_str(vendor1, c.ebx);
  cast_uint_to_str(vendor2, c.edx);
  cast_uint_to_str(vendor3, c.ecx);
  sprintf(vendor, "%s%s%s", vendor1, vendor2, vendor3);
}
