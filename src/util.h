/*
Copyright (c) 2003, O'Reilly: Taken from "Secure Programming Cookbook for C and C++"
(https://resources.oreilly.com/examples/9780596003944/)
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

#ifndef _h_util
#define _h_util

#include <sched.h>
#include <stdio.h>
#include <sys/types.h>

#define DEBUG(msg, args...)                                                                        \
  if (is_debug_enabled()) {                                                                        \
    fprintf(stderr, "[DEBUG] " msg "\n", args);                                                    \
  }

void enable_debug();
int is_debug_enabled();

static const uid_t UID_NOBODY = 65534;
static const gid_t GID_NOGROUP = 65534;

/*
 * Drop all capabilities that the process is currently in possession of.
 *
 * For security, terminates process on failure.
 */
void drop_capabilities();

/**
 * Drop any extra group or user privileges.
 * Custom values can be specified for uid and gid to be taken as new id in the process.
 *
 * For security, terminates process on failure.
 */
void drop_root_privileges_by_id(uid_t uid, gid_t gid);

/**
 * Set the CPU affinity of the current thread to the given CPU.
 * If old_context is not null, store previous CPU affinity in it.
 *
 * Returns 0 on success and -1 on failure.
 */
int bind_cpu(int cpu, cpu_set_t *old_context);

/**
 * Set the CPU affinity of the current thread to the given set.
 * If old_context is not null, store previous CPU affinity in it.
 *
 * Returns 0 on success and -1 on failure.
 */
int bind_context(cpu_set_t *new_context, cpu_set_t *old_context);

#endif /* _h_util */
