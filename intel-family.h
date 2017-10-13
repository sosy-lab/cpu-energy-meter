#ifndef _ASM_X86_INTEL_FAMILY_H
#define _ASM_X86_INTEL_FAMILY_H

/*
 * Intel Core Processors (Branded as Core, Xeon, etc...)
 *
 * The "_X" parts are generally the EP and EX Xeons, or the "Extreme" ones, like Broadwell-E.
 *
 * For reference, see Intel Architectures Software Developer's Manual Volume 4, Model-Specific Registers, Chapter 2, Table 2-1
 * (https://software.intel.com/sites/default/files/managed/22/0d/335592-sdm-vol-4.pdf)
 */


#define CPU_INTEL_SANDYBRIDGE		    0x206A0     // Family 6 Model 42 (0x2a)
#define CPU_INTEL_SANDYBRIDGE_X         0x206D0     // Family 6 Model 45 (0x2d)

#define CPU_INTEL_IVYBRIDGE		        0x306A0     // Family 6 Model 58 (0x3a)
#define CPU_INTEL_IVYBRIDGE_X		    0x306E0     // Family 6 Model 62 (0x3e)

#define CPU_INTEL_HASWELL_CORE		    0x306C0     // Family 6 Model 60 (0x3c)
#define CPU_INTEL_HASWELL_ULT		    0x40650     // Family 6 Model 69 (0x45)
#define CPU_INTEL_HASWELL_GT3E		    0x40660     // Family 6 Model 70 (0x46)
#define CPU_INTEL_HASWELL_X             0x306F0     // Family 6 Model 63 (0x3f)

#define CPU_INTEL_BROADWELL_CORE	    0x306D0     // Family 6 Model 61 (0x3d)
#define CPU_INTEL_BROADWELL_GT3E	    0x40670     // Family 6 Model 71 (0x47)
#define CPU_INTEL_BROADWELL_X		    0x406F0     // Family 6 Model 79 (0x4f)
#define CPU_INTEL_BROADWELL_XEON_D	    0x50660     // Family 6 Model 64 (0x56)

#define CPU_INTEL_SKYLAKE_MOBILE	    0x406E0     // Family 6 Model 78 (0x4e)
#define CPU_INTEL_SKYLAKE_DESKTOP	    0x506E0     // Family 6 Model 94 (0x5e)
#define CPU_INTEL_SKYLAKE_X             0x50650     // Family 6 Model 85 (0x55)

#define CPU_INTEL_KABYLAKE_MOBILE	    0x806E0     // Family 6 Model 142 (0x8e)
#define CPU_INTEL_KABYLAKE_DESKTOP	    0x906E0     // Family 6 Model 158 (0x9e)

#endif /* _ASM_X86_INTEL_FAMILY_H */