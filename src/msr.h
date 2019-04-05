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
