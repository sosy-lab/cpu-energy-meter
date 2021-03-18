// This file is part of CPU Energy Meter,
// a tool for measuring energy consumption of Intel CPUs:
// https://github.com/sosy-lab/cpu-energy-meter
//
// SPDX-FileCopyrightText: 2003 O'Reilly (Taken from "Secure Programming Cookbook for C and C++")
// SPDX-FileCopyrightText: 2015-2021 Dirk Beyer <https://www.sosy-lab.org>
//
// SPDX-License-Identifier: BSD-3-Clause

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
