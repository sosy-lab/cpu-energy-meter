// This file is part of CPU Energy Meter,
// a tool for measuring energy consumption of Intel CPUs:
// https://github.com/sosy-lab/cpu-energy-meter
//
// SPDX-FileCopyrightText: 2012 Intel Corporation
// SPDX-FileCopyrightText: 2015-2018 Dirk Beyer
//
// SPDX-License-Identifier: BSD-3-Clause

#ifndef _h_msr_t
#define _h_msr_t

#include <stdint.h>
#include <sys/types.h>

/*
 * For documentation see: "Intel64 and IA-32 Architectures Software Developer's Manual" Volume 4:
 * Model-Specific registers.
 * (Link: https://software.intel.com/sites/default/files/managed/22/0d/335592-sdm-vol-4.pdf)
 * In a nutshell, search the manual and find the MSR number that you wish to read. Then use the
 * read_msr_t function to get the info you need.
 */

/**
 * Open and store file descriptors in an array for as often as specified in the num_nodes param.
 *
 * @return 0 on success and -1 if at least one node fails to open
 */
int open_msr_fd(int num_nodes, int (*node_to_core)(int));

/**
 * Read the given MSR on the given node.
 *
 * @return 0 on success and -1 on failure
 */
int read_msr(int node, off_t address, uint64_t *val);

/**
 * Close each file descriptor and free the allocated array memory.
 */
void close_msr_fd();
#endif
